#include <memory>
#include <filesystem>
#include <cstdlib>
#include <string>

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
    if (!parse_cmdline(argc, argv, coreParameters)) return EXIT_SUCCESS;

    platform::configure_encoding();

    // Инициализация логгера
	if (ENGINE_DEBUG_BUILD) {
		Logger::getInstance().initialize(
            coreParameters.userFolder/std::filesystem::u8path("logs/ChromaForge.log")
        );
	} else {
		Logger::getInstance().initialize(
			coreParameters.userFolder/std::filesystem::u8path("logs/ChromaForge.log"),
			LogLevel::WARN,
			LogLevel::INFO
		);
	}

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
