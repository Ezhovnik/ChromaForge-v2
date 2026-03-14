#ifndef FRONTEND_MENU_MENU_H_
#define FRONTEND_MENU_MENU_H_

#include <string>

class Engine;
class LevelController;

namespace menus {
    extern void create_settings_panel(Engine* engine);
    extern void create_pause_panel(Engine* engine, LevelController* level);
    void open_world(std::string name, Engine* engine, bool confirmConvert);
    void create_version_label(Engine* engine);
    void create_menus(Engine* engine);
    void refresh_menus(Engine* engine);
}

#endif // FRONTEND_MENU_MENU_H_
