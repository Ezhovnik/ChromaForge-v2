#include <devtools/stdin_cmd_reader.h>

#include <thread>
#include <iostream>

#include <engine/Engine.h>
#include <logic/CommandsInterpreter.h>
#include <coders/json.h>
#include <debug/Logger.h>

static std::thread reader_thread;

void cmd::start_stdin_cmd_reader(Engine& engine) {
    reader_thread = std::thread([&engine]() {
        auto& interpreter = engine.getCmd();
        LOG_INFO("Reader thread started");

        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            engine.postRunnable([line, &interpreter] () {
                try {
                    auto result = interpreter.execute(line);
                    if (result.isString()) {
                        LOG_INFO("{}", result.asString());
                    } else {
                        LOG_INFO("{}", json::stringify(result, true));
                    }
                } catch (const std::exception& err) {
                    LOG_ERROR("{}", err.what());
                }
            });
        }
    });
    reader_thread.detach();
}
