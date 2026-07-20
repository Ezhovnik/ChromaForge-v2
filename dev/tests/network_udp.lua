math.randomseed(43172)
for i = 1, 15 do
    debug.info(string.format("Iteration %s", i))
    local complete = false

    local server = network.udp_open(8645 + i, function (address, port, data, srv)
        debug.info(string.format("Server received %s byte(s) from %s:%s", #data, address, port))
        srv:send(address, port, "pong")
    end)

    app.spark()
    network.udp_connect("localhost", 8645 + i, function (data)
        debug.info(string.format("Client received %s byte(s) from server", #data))
        complete = true
    end, function (socket)
        debug.info("UDP socket opened")
        start_coroutine(function()
            debug.info("UDP data-sender started")
            for k = 1, 15 do
                local payload = ""
                for j = 1, 16 do
                    payload = payload .. math.random(0, 9)
                end
                socket:send(payload)
                debug.info(string.format("Sent packet %s (%s bytes)", k, #payload))
                coroutine.yield()
            end
            app.sleep_until(function () return complete end, nil, 5)
            socket:close()
        end, "udp-data-sender")
    end)

    app.sleep_until(function () return complete end, nil, 5)
    server:close()
end
