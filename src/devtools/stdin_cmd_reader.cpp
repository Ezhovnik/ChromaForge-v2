#include <devtools/stdin_cmd_reader.h>

#include <thread>
#include <iostream>

#include <engine/Engine.h>
#include <logic/CommandsInterpreter.h>
#include <coders/json.h>
#include <debug/Logger.h>

static debug::Logger logger("stdin-reader");

static std::thread reader_thread;

void cmd::start_stdin_cmd_reader(Engine& engine) {
    reader_thread = std::thread([&engine]() {
        auto& interpreter = engine.getCmd();
        logger.info() << "Reader thread started";

        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) continue;
            engine.postRunnable([line, &interpreter] () {
                try {
                    auto result = interpreter.execute(line);
                    if (result.isString()) {
                        logger.info() << result.asString();
                    } else {
                        logger.info() << json::stringify(result, true);
                    }
                } catch (const std::exception& err) {
                    logger.error() << err.what();
                }
            });
        }
    });
    reader_thread.detach();
}
