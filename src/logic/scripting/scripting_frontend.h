#ifndef LOGIC_SCRIPTING_SCRIPTING_FRONTEND_H_
#define LOGIC_SCRIPTING_SCRIPTING_FRONTEND_H_

#include <filesystem>
#include <string>

#include "../../typedefs.h"

class Hud;

namespace scripting {
    extern Hud* hud;

    void on_frontend_init(Hud* hud);
    void on_frontend_close();

    void load_hud_script(scriptenv env, std::string packid, std::filesystem::path file);
}

#endif // LOGIC_SCRIPTING_SCRIPTING_FRONTEND_H_
