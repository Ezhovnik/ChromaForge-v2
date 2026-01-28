#ifndef FILES_ENGINE_FILES_H_
#define FILES_ENGINE_FILES_H_

#include <string>
#include <filesystem>

namespace engine_fs {
    extern std::filesystem::path get_screenshot_file(std::string ext);
    extern std::filesystem::path get_saves_folder();
    extern std::filesystem::path get_logs_file();

    extern bool is_world_name_used(std::string name);
}

#endif // FILES_ENGINE_FILES_H_
