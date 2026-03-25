#ifndef FRONTEND_MENU_MENU_H_
#define FRONTEND_MENU_MENU_H_

#include <string>
#include <memory>
#include <vector>

#include "../graphics/ui/elements/layout/Menu.h"

namespace dynamic {
    class Value;
}

class Task;
class Engine;
class UIDocument;

namespace menus {
    void create_version_label(Engine* engine);
    gui::page_loader_func create_page_loader(Engine* engine);
    UIDocument* show(
        Engine* engine, 
        const std::string& name,
        std::vector<std::unique_ptr<dynamic::Value>> args
    );
    void show_process_panel(Engine* engine, std::shared_ptr<Task> task, std::wstring text=L"");
}

#endif // FRONTEND_MENU_MENU_H_
