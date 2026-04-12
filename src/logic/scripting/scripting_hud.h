#ifndef LOGIC_SCRIPTING_SCRIPTING_HUD_H_
#define LOGIC_SCRIPTING_SCRIPTING_HUD_H_

#include <filesystem>
#include <string>

#include "../../typedefs.h"

class Hud;

namespace scripting {
    extern Hud* hud;

    void on_frontend_init(Hud* hud);
    void on_frontend_close();
    void on_frontend_render();

    void load_hud_script(
        const scriptenv& env,
        const std::string& packid,
        const std::filesystem::path& file
    );
}

#endif // LOGIC_SCRIPTING_SCRIPTING_HUD_H_
