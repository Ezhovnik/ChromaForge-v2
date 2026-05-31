-- Use only for engine development/testing
-- Must be empty in the release version
-- Cannot be modified through content packs
local server = network.tcp_open(65343, function (socket)
    print("connected client", socket.id)
    socket:send(utf8.tobytes("Hello, World!"))
    --socket:close()
end)
print("server", server.id)

local socket = network.tcp_connect('localhost', 65343, function (socket)
    print("connected", socket.id)
end)

function on_world_spark()
    print("spark")
    local result = socket:recv(128)
    if result and #result > 0 then
        print("received from server:", utf8.tostring(result))
        socket:close()
    end
end

function on_world_quit()
    server:close()
end
