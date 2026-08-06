#include <logic/scripting/lua/lua_commons.h>

#include <debug/Logger.h>

static debug::Logger logger("lua");

void lua::log_error(const std::string& text) {
    logger.error() << "Lua error: " << text;
}
