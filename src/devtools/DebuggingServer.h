#pragma once

#include <memory>
#include <string>
#include <vector>
#include <variant>

#include <typedefs.h>

namespace network {
    class Server;
    class Connection;
    class ReadableConnection;
    class Network;
}

namespace dv {
    class value;
}

class Engine;

namespace devtools {
    class ClientConnection {
    public:
        ClientConnection(network::Network& network, uint64_t connection) : network(network), connection(connection) {}
        ~ClientConnection();

        std::string read();
        void send(const dv::value& message);
        void sendResponse(const std::string& type);
    private:
        network::Network& network;
        size_t messageLength = 0;
        uint64_t connection;
    };

    enum class DebuggingEventType {
        SetBreakpoint = 1,
        RemoveBreakpoint,
        Step,
        StepIntoFunction,
        Resume,
        GetValue
    };

    struct BreakpointEventDto {
        std::string source;
        int line;
    };

    struct SignalEventDto {
    };

    using ValuePath = std::vector<std::variant<std::string, int>>;

    struct GetValueEventDto {
        int frame;
        int localIndex;
        ValuePath path;
    };

    struct DebuggingEvent {
        DebuggingEventType type;
        std::variant<BreakpointEventDto, SignalEventDto, GetValueEventDto> data;
    };

    class DebuggingServer {
    public:
        DebuggingServer(Engine& engine, const std::string& serverString);
        ~DebuggingServer();

        bool update();
        void pause(
            std::string&& reason,
            std::string&& message,
            dv::value&& stackTrace
        );

        void sendValue(dv::value&& value, int frame, int local, ValuePath&& path);

        void setClient(uint64_t client);
        std::vector<DebuggingEvent> pullEvents();
    private:
        Engine& engine;
        network::Server& server;
        std::unique_ptr<ClientConnection> connection;
        std::vector<DebuggingEvent> breakpointEvents;

        bool performCommand(const std::string& type, const dv::value& map);
    };
}
