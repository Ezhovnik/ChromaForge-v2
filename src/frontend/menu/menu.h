#ifndef FRONTEND_MENU_MENU_H_
#define FRONTEND_MENU_MENU_H_

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "../../content/ContentPack.h"

class Engine;
class LevelController;

namespace gui {
    class Panel;
}

using packconsumer = std::function<void(const ContentPack& pack)>;

namespace menus {
    extern void create_settings_panel(Engine* engine);
    extern std::shared_ptr<gui::Panel> create_packs_panel(
        const std::vector<ContentPack>& packs, 
        Engine* engine, 
        bool backbutton, 
        packconsumer callback,
        packconsumer remover
    );
    void open_world(
        std::string name, 
        Engine* engine, 
        bool confirmConvert
    );
    void create_world(
        Engine* engine, 
        const std::string& name, 
        const std::string& seedstr,
        const std::string& generatorID
    );
    void delete_world(std::string name, Engine* engine);
    void create_version_label(Engine* engine);
    void remove_packs(
        Engine* engine, 
        LevelController* controller, 
        std::vector<std::string> packs
    );
    void add_packs(
        Engine* engine,
        LevelController* controller,
        std::vector<std::string> packs
    );

    void create_menus(Engine* engine);
    void refresh_menus(Engine* engine);
}

#endif // FRONTEND_MENU_MENU_H_
