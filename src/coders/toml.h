#ifndef CODERS_TOML_H_
#define CODERS_TOML_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "../data/dynamic.h"

class SettingsHandler;

namespace toml {
    std::string stringify(SettingsHandler& handler);
    std::string stringify(dynamic::Map& root, const std::string& name="");
    dynamic::Map_sptr parse(std::string_view file, std::string_view source);

    void parse(
        SettingsHandler& handler, 
        std::string_view file,
        std::string_view source
    );
}

#endif // CODERS_TOML_H_
