#ifndef FILES_ENGINE_FILES_H_
#define FILES_ENGINE_FILES_H_

#include <string>
#include <filesystem>

#define SCREENSHOTS_FOLDER "../build/screenshots"
#define SAVES_FOLDER "../build/saves"

namespace engine_fs {
    extern std::string get_screenshot_file(std::string ext);
    extern std::string get_saves_folder();
}

#endif // FILES_ENGINE_FILES_H_
