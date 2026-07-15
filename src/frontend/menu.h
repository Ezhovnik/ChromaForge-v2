#pragma once

#include <string>
#include <memory>
#include <vector>

#include <data/dv.h>
#include <delegates.h>

class Task;
class Engine;
class UIDocument;

namespace gui {
    class GUI;
}

namespace menus {
    void create_version_label(gui::GUI& gui);

    UIDocument* show(
        Engine& engine, 
        const std::string& name,
        std::vector<dv::value> args
    );

    void show_process_panel(
        Engine& engine,
        const std::shared_ptr<Task>& task,
        const std::wstring& text=L""
    );

    void call(Engine& engine, runnable func);
}
