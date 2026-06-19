#pragma once

#include <memory>
#include <vector>
#include <mutex>

#include <typedefs.h>
#include <settings.h>
#include <util/Buffer.h>
#include <delegates.h>

namespace network {
    using OnResponse = std::function<void(std::vector<char>)>;
    using OnReject = std::function<void(const char*)>;

    class Requests {
    public:
        virtual ~Requests() {}

        virtual void get(
            const std::string& url,
            OnResponse onResponse,
            OnReject onReject=nullptr,
            long maxSize=0
        ) = 0;

        virtual void post(
            const std::string& url,
            const std::string& data,
            OnResponse onResponse,
            OnReject onReject=nullptr,
            long maxSize=0
        ) = 0;

        virtual size_t getTotalUpload() const = 0;
        virtual size_t getTotalDownload() const = 0;

        virtual void update() = 0;
    };

    enum class ConnectionState {
        Initial, Connecting, Connected, Closed
    };

    class Connection {
    public:
        virtual ~Connection() {}

        virtual void connect(runnable callback) = 0;
        virtual int recv(char* buffer, size_t length) = 0;
        virtual int send(const char* buffer, size_t length) = 0;
        virtual void close() = 0;
        virtual int available() = 0;

        virtual size_t pullUpload() = 0;
        virtual size_t pullDownload() = 0;

        virtual int getPort() const = 0;
        virtual std::string getAddress() const = 0;

        virtual ConnectionState getState() const = 0;
    };

    class TcpServer {
    public:
        virtual ~TcpServer() {}
        virtual void startListen(consumer<uint64_t> handler) = 0;
        virtual void close() = 0;
        virtual bool isOpen() = 0;
        virtual int getPort() const = 0;
    };

    class Network {
        std::unique_ptr<Requests> requests;

        std::mutex connectionsMutex {};
        std::unordered_map<uint64_t, std::shared_ptr<Connection>> connections;
        uint64_t nextConnection = 1;

        std::unordered_map<uint64_t, std::shared_ptr<TcpServer>> servers;
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
            long maxSize=0
        );

        void post(
            const std::string& url,
            const std::string& fieldsData,
            OnResponse onResponse,
            OnReject onReject = nullptr,
            long maxSize=0
        );

        [[nodiscard]] Connection* getConnection(uint64_t id);
        [[nodiscard]] TcpServer* getServer(uint64_t id) const;

        uint64_t connect(const std::string& address, int port, consumer<uint64_t> callback);;

        uint64_t openServer(int port, consumer<uint64_t> handler);

        uint64_t addConnection(const std::shared_ptr<Connection>& connection);

        size_t getTotalUpload() const;
        size_t getTotalDownload() const;

        void update();

        static std::unique_ptr<Network> create(
            const NetworkSettings& settings
        );
    };
}
