#include <devtools/DebuggingServer.h>

#include <engine/Engine.h>
#include <network/Network.h>
#include <debug/Logger.h>
#include <coders/json.h>

using namespace devtools;

ClientConnection::~ClientConnection() {
    if (auto connection = dynamic_cast<network::ReadableConnection*>(
        network.getConnection(this->connection, true)
    )) {
        connection->close();
    }
}

std::string ClientConnection::read() {
    auto connection = dynamic_cast<network::ReadableConnection*>(
        network.getConnection(this->connection, true)
    );
    if (connection == nullptr) return "";
    if (messageLength == 0) {
        if (connection->available() >= sizeof(int32_t)) {
            int32_t length = 0;
            connection->recv(reinterpret_cast<char*>(&length), sizeof(int32_t));
            if (length <= 0) {
                LOG_ERROR("Invalid message length {}", length);
            } else {
                messageLength = length;
            }
        }
    } else if (connection->available() >= messageLength) {
        std::string string(messageLength, 0);
        connection->recv(string.data(), messageLength);
        return string;
    }
    return "";
}

void ClientConnection::send(const dv::value& object) {
    auto connection = dynamic_cast<network::ReadableConnection*>(
        network.getConnection(this->connection, true)
    );
    if (connection == nullptr) return;
    auto message = json::stringify(object, false);
    int32_t length = message.length();
    connection->send(reinterpret_cast<char*>(&length), sizeof(int32_t));
    connection->send(message.data(), length);
}

void ClientConnection::sendResponse(const std::string& type) {
    send(dv::object({{"type", type}}));
}

static network::Server& create_tcp_server(
    DebuggingServer& dbgServer, Engine& engine, int port
) {
    auto& network = engine.getNetwork();
    uint64_t serverId = network.openTcpServer(
        port,
        [&network, &dbgServer](uint64_t sid, uint64_t id) {
            auto& connection = dynamic_cast<network::ReadableConnection&>(
                *network.getConnection(id, true)
            );
            connection.setPrivate(true);
            LOG_INFO("Connected client {}: {}:{}", id, connection.getAddress(), connection.getPort());
            dbgServer.setClient(id);
        }
    );
    auto& server = *network.getServer(serverId, true);
    server.setPrivate(true);

    auto& tcpServer = dynamic_cast<network::TcpServer&>(server);
    tcpServer.setMaxClientsConnected(1);

    LOG_INFO("TCP debugging server open at port {}", server.getPort());

    return tcpServer;
}

static network::Server& create_server(
    DebuggingServer& dbgServer, Engine& engine, const std::string& serverString
) {
    LOG_INFO("Starting debugging server");

    size_t sepPos = serverString.find(':');
    if (sepPos == std::string::npos) {
        THROW_ERR("Invalid debugging server configuration string");
    }
    auto transport = serverString.substr(0, sepPos);
    if (transport == "tcp") {
        int port;
        try {
            port = std::stoi(serverString.substr(sepPos + 1));
        } catch (const std::exception& err) {
            THROW_ERR("Invalid TCP port");
        }
        return create_tcp_server(dbgServer, engine, port);
    } else {
        THROW_ERR(
            "Unsupported debugging server transport '{}'", transport
        );
    }
}

DebuggingServer::DebuggingServer(
    Engine& engine, const std::string& serverString
) : engine(engine),
    server(create_server(*this, engine, serverString)),
    connection(nullptr) {}

DebuggingServer::~DebuggingServer() {
    LOG_INFO("Stopping debugging server");
    server.close();
}

bool DebuggingServer::update() {
    if (connection == nullptr) return true;
    std::string message = connection->read();
    if (message.empty()) return true;

    LOG_DEBUG("Received: {}", message);

    try {
        auto obj = json::parse(message);
        if (!obj.has("type")) {
            LOG_ERROR("Missing message type");
            return true;
        }
        const auto& type = obj["type"].asString();
        return performCommand(type, obj);
    } catch (const std::runtime_error& err) {
        LOG_ERROR("Could not to parse message: {}", err.what());
    }
    return true;
}

bool DebuggingServer::performCommand(
    const std::string& type, const dv::value& map
) {
    if (type == "terminate") {
        engine.quit();
        connection->sendResponse("success");
    } else if (type == "detach") {
        connection->sendResponse("success");
        connection.reset();
        return false;
    } else {
        LOG_ERROR("Unsupported command '{}'", type);
    }
    return true;
}

void DebuggingServer::setClient(uint64_t client) {
    this->connection = std::make_unique<ClientConnection>(engine.getNetwork(), client);
}
