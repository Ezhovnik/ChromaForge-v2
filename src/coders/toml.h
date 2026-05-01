#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <data/dv.h>

class SettingsHandler;

namespace toml {
    std::string stringify(SettingsHandler& handler);
    std::string stringify(const dv::value& root, const std::string& name = "");

    dv::value parse(std::string_view file, std::string_view source);

    void parse(
        SettingsHandler& handler, 
        std::string_view file,
        std::string_view source
    );
}
