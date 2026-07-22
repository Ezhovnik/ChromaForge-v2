#pragma once

#include <network/commons.h>

namespace network {
    class TcpConnection : public Connection {
    public:
        ~TcpConnection() override = default;

        virtual void connect(runnable callback) = 0;
        virtual int recv(char* buffer, size_t length) = 0;
        virtual int available() = 0;

        virtual void setNoDelay(bool noDelay) = 0;
        [[nodiscard]] virtual bool isNoDelay() const = 0;

        [[nodiscard]] TransportType getTransportType() const noexcept override {
            return TransportType::TCP;
        }
    };

    class UdpConnection : public Connection {
    public:
        ~UdpConnection() override = default;

        virtual void connect(ClientDatagramCallback handler) = 0;

        [[nodiscard]] TransportType getTransportType() const noexcept override {
            return TransportType::UDP;
        }
    };

    class TcpServer : public Server {
    public:
        ~TcpServer() override {}
        virtual void startListen(ConnectCallback handler) = 0;

        [[nodiscard]] TransportType getTransportType() const noexcept override {
            return TransportType::TCP;
        }

        virtual void setMaxClientsConnected(int count) = 0;
    };

    class UdpServer : public Server {
    public:
        ~UdpServer() override {}
        virtual void startListen(ServerDatagramCallback handler) = 0;

        virtual void sendTo(const std::string& addr, int port, const char* buffer, size_t length) = 0;

        [[nodiscard]] TransportType getTransportType() const noexcept override {
            return TransportType::UDP;
        }
    };

    class Network {
        std::unique_ptr<Requests> requests;

        std::mutex connectionsMutex {};
        std::unordered_map<uint64_t, std::shared_ptr<Connection>> connections;
        uint64_t nextConnection = 1;

        std::unordered_map<uint64_t, std::shared_ptr<Server>> servers;
        uint64_t nextServer = 1;

        size_t totalDownload = 0;
        size_t totalUpload = 0;
    public:
        Network(std::unique_ptr<Requests> requests);
        ~Network();

        void get(
            const std::string& url,
            OnResponse onResponse,
            OnReject onReject = nullptr,
            std::vector<std::string> headers = {},
            long maxSize=0
        );

        void post(
            const std::string& url,
            const std::string& fieldsData,
            OnResponse onResponse,
            OnReject onReject = nullptr,
            std::vector<std::string> headers = {},
            long maxSize=0
        );

        [[nodiscard]] Connection* getConnection(uint64_t id, bool includePrivate);
        [[nodiscard]] Server* getServer(uint64_t id, bool includePrivate) const;

        uint64_t connectTcp(const std::string& address, int port, consumer<uint64_t> callback);
        uint64_t connectUdp(const std::string& address, int port, const consumer<uint64_t>& callback, ClientDatagramCallback handler);

        uint64_t openTcpServer(int port, ConnectCallback handler);
        uint64_t openUdpServer(int port, const ServerDatagramCallback& handler);

        uint64_t addConnection(const std::shared_ptr<Connection>& connection);

        [[nodiscard]] size_t getTotalUpload() const;
        [[nodiscard]] size_t getTotalDownload() const;

        void update();

        static std::unique_ptr<Network> create(
            const NetworkSettings& settings
        );
    };
}
