#pragma once

#include <string>
#include <filesystem>

namespace engine_fs {
    extern std::filesystem::path get_screenshot_file(std::string ext);
    extern std::filesystem::path get_saves_folder();
    extern std::filesystem::path get_logs_file();

    extern bool is_world_name_used(std::string name);
}
