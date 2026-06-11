#include <memory>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <iostream>

#include <engine.h>
#include <files/files.h>
#include <util/platform.h>
#include <coders/toml.h>
#include <input_bindings.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include <util/command_line.h>
#include <constants.h>
#include <window/Events.h>

// Точка входа в программу
int main(int argc, char** argv) {
    CoreParameters coreParameters;
    try {
        if (!parse_cmdline(argc, argv, coreParameters)) {
            return EXIT_SUCCESS;
        }
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Инициализация логгера
    auto logPath = coreParameters.userFolder/std::filesystem::u8path("logs/ChromaForge.log");
    if (ENGINE_DEBUG_BUILD) {
		Logger::getInstance().initialize(logPath);
	} else {
		Logger::getInstance().initialize(
			logPath,
			LogLevel::WARN,
			LogLevel::INFO
		);
	}

    platform::configure_encoding();

    try {
        Engine(std::move(coreParameters)).run();
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred:\n{}", err.what());
    } catch (const std::exception& err) {
        LOG_ERROR("Uncaught exception:\n{}", err.what());
        throw;
    }

    Logger::getInstance().flush();

    return EXIT_SUCCESS;
}
