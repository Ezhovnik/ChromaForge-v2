#ifndef FRONTEND_MENU_H_
#define FRONTEND_MENU_H_

class Engine;

namespace menus {
    void create_menus(Engine* engine);
    void refresh_menus(Engine* engine);
    void create_version_label(Engine* engine);
}

#endif // FRONTEND_MENU_H_
