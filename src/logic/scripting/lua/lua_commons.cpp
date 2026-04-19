#include "lua_commons.h"

#include <debug/Logger.h>

void lua::log_error(const std::string& text) {
    LOG_ERROR("Lua error: {}", text);
}
