#include <network/Network.h>

#include <stdexcept>
#include <limits>
#include <string>
#include <cerrno>
#include <cstring>
#include <memory>
#include <queue>
#include <thread>

#include <debug/Logger.h>
#include <util/stringutil.h>

using namespace network;

namespace network {
    std::unique_ptr<Requests> create_curl_requests();

    std::shared_ptr<TcpConnection> connect_tcp(
        const std::string& address, int port, runnable callback
    );

    std::shared_ptr<TcpServer> open_tcp_server(
        uint64_t id, Network* network, int port, ConnectCallback handler
    );

    std::shared_ptr<UdpConnection> connect_udp(
        uint64_t id,
        const std::string& address,
        int port,
        ClientDatagramCallback handler,
        runnable callback
    );

    std::shared_ptr<UdpServer> open_udp_server(
        uint64_t id,
        Network* network,
        int port,
        const ServerDatagramCallback& handler
    );
}


Network::Network(std::unique_ptr<Requests> requests) : requests(std::move(requests)) {
}

Network::~Network() = default;

void Network::get(
    const std::string& url,
    OnResponse onResponse,
    OnReject onReject,
    std::vector<std::string> headers,
    long maxSize
) {
    requests->get(url, onResponse, onReject, std::move(headers), maxSize);
}

void Network::post(
    const std::string& url,
    const std::string& fieldsData,
    OnResponse onResponse,
    OnReject onReject,
    std::vector<std::string> headers,
    long maxSize
) {
    requests->post(
        url, fieldsData, onResponse, onReject, std::move(headers), maxSize
    );
}

Connection* Network::getConnection(uint64_t id, bool includePrivate) {
    std::lock_guard lock(connectionsMutex);

    const auto& found = connections.find(id);
    if (found == connections.end() || (!includePrivate && found->second->isPrivate())) {
        return nullptr;
    }
    return found->second.get();
}

Server* Network::getServer(uint64_t id, bool includePrivate) const {
    const auto& found = servers.find(id);
    if (found == servers.end() || (!includePrivate && found->second->isPrivate())) {
        return nullptr;
    }
    return found->second.get();
}

uint64_t Network::connectTcp(const std::string& address, int port, consumer<uint64_t> callback) {
    std::lock_guard lock(connectionsMutex);
    uint64_t id = nextConnection++;
    auto socket = connect_tcp(address, port, [id, callback]() {
        callback(id);
    });
    connections[id] = std::move(socket);
    return id;
}

uint64_t Network::openTcpServer(int port, ConnectCallback handler) {
    uint64_t id = nextServer++;
    auto server = open_tcp_server(id, this, port, handler);
    servers[id] = std::move(server);
    return id;
}

uint64_t Network::connectUdp(
    const std::string& address,
    int port,
    const consumer<uint64_t>& callback,
    ClientDatagramCallback handler
) {
    std::lock_guard lock(connectionsMutex);

    uint64_t id = nextConnection++;
    auto socket = connect_udp(id, address, port, std::move(handler), [id, callback]() {
        callback(id);
    });
    connections[id] = std::move(socket);
    return id;
}

uint64_t Network::openUdpServer(int port, const ServerDatagramCallback& handler) {
    uint64_t id = nextServer++;
    auto server = open_udp_server(id, this, port, handler);
    servers[id] = std::move(server);
    return id;
}

uint64_t Network::addConnection(const std::shared_ptr<Connection>& socket) {
    std::lock_guard lock(connectionsMutex);

    uint64_t id = nextConnection++;
    connections[id] = std::move(socket);
    return id;
}

size_t Network::getTotalUpload() const {
    return requests->getTotalUpload() + totalUpload;
}

size_t Network::getTotalDownload() const {
    return requests->getTotalDownload() + totalDownload;
}

void Network::update() {
    requests->update();

    {
        std::lock_guard lock(connectionsMutex);
        auto socketiter = connections.begin();
        while (socketiter != connections.end()) {
            auto socket = socketiter->second.get();
            totalDownload += socket->pullDownload();
            totalUpload += socket->pullUpload();
            if (
                (
                    socket->getTransportType() == TransportType::UDP ||
                    dynamic_cast<TcpConnection*>(socket)->available() == 0
                ) && socket->getState() == ConnectionState::Closed
            ) {
                socketiter = connections.erase(socketiter);
                continue;
            }
            ++socketiter;
        }
    }
    auto serveriter = servers.begin();
    while (serveriter != servers.end()) {
        auto server = serveriter->second.get();
        if (!server->isOpen()) {
            serveriter = servers.erase(serveriter);
            continue;
        }
        server->update();
        ++serveriter;
    }
}

std::unique_ptr<Network> Network::create(const NetworkSettings& settings) {
    return std::make_unique<Network>(network::create_curl_requests());
}
