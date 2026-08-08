#include <memory>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <csignal>
#include <iostream>

#include <engine/Engine.h>
#include <util/platform.h>
#include <coders/toml.h>
#include <input_bindings.h>
#include <core_content_defs.h>
#include <debug/Logger.h>
#include <util/command_line.h>
#include <constants.h>

static debug::Logger logger("main");

static void sigterm_handler(int signum) {
    Engine::getInstance().quit();
}

// Точка входа в программу
int main(int argc, char** argv) {
#ifdef CHROMA_BUILD_NAME
    if constexpr (CHROMA_BUILD_NAME[0]) {
        logger.info() << "Build: " << CHROMA_BUILD_NAME;
    }
#endif
    CoreParameters coreParameters;
    try {
        if (!parse_cmdline(argc, argv, coreParameters)) {
            return EXIT_SUCCESS;
        }
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::signal(SIGTERM, sigterm_handler);

    // Инициализация логгера
    auto logPath = coreParameters.userFolder/std::filesystem::u8path("logs/ChromaForge.log");
    debug::Logger::init(logPath.u8string());

    platform::configure_encoding();

    auto& engine = Engine::getInstance();
    try {
        engine.initialize(std::move(coreParameters));
        engine.run();
    } catch (const initialize_error& err) {
        debug::Logger::getInstance().critical() << "Could not initialize engine: " << err.what();
    }
#if defined(NDEBUG)
    catch (const std::exception& err) {
        debug::Logger::getInstance().error() << "Uncaught exception: " << err.what();
        debug::Logger::flush();
        throw;
    }
#endif
    Engine::terminate();
    return EXIT_SUCCESS;
}
