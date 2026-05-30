#pragma once

#include <filesystem>
#include <string>

#include <typedefs.h>

class Hud;
class WorldRenderer;

namespace scripting {
    extern Hud* hud;
    extern WorldRenderer* renderer;

    void on_frontend_init(Hud* hud, WorldRenderer* renderer);
    void on_frontend_close();
    void on_frontend_render();

    void load_hud_script(
        const scriptenv& env,
        const std::string& packid,
        const std::filesystem::path& file,
        const std::string& fileName
    );
}
