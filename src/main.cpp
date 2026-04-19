#include <memory>
#include <filesystem>
#include <cstdlib>
#include <string>

#include <engine.h>
#include <settings.h>
#include "files/files.h"
#include "util/platform.h"
#include "coders/toml.h"
#include <input_bindings.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include "util/command_line.h"
#include "files/settings_io.h"
#include "files/engine_paths.h"
#include <constants.h>
#include <window/Events.h>

// Точка входа в программу
int main(int argc, char** argv) {
    EnginePaths paths;
	if (!parse_cmdline(argc, argv, paths)) return EXIT_SUCCESS;
    std::filesystem::path userfiles = paths.getUserfiles();

    platform::configure_encoding();

    // Инициализация логгера
	if (ENGINE_DEBUG_BUILD) {
		Logger::getInstance().initialize(paths.getLogsFile().string());
	} else {
		Logger::getInstance().initialize(
			paths.getLogsFile().string(),
			LogLevel::WARN,
			LogLevel::INFO
		);
	}

    try {
        EngineSettings settings;
        SettingsHandler handler(settings);
        Engine engine(settings, handler, &paths);
        engine.mainloop();
    } catch (const initialize_error& err) {
        LOG_CRITICAL("An initialization error occurred:\n{}", err.what());
    } catch (const std::exception& err) {
        LOG_ERROR("Uncaught exception:\n{}", err.what());
        throw;
    }

    Logger::getInstance().flush();

    return EXIT_SUCCESS;
}
