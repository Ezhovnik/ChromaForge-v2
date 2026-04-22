#pragma once

#include <string>
#include <memory>
#include <vector>

#include <data/dynamic.h>
#include <graphics/ui/elements/layout/Menu.h>

class Task;
class Engine;
class UIDocument;

namespace menus {
    void create_version_label(Engine* engine);

    gui::page_loader_func create_page_loader(Engine* engine);

    UIDocument* show(
        Engine* engine, 
        const std::string& name,
        std::vector<dynamic::Value> args
    );

    void show_process_panel(
        Engine* engine,
        const std::shared_ptr<Task>& task,
        const std::wstring& text=L""
    );

    bool call(Engine* engine, runnable func);
}
