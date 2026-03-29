#include "menu.h"

#include <filesystem>
#include <random>
#include <chrono>

#include <glm/glm.hpp>

#include "../delegates.h"
#include "../engine.h"
#include "../files/engine_paths.h"
#include "../graphics/ui/elements/display/Label.h"
#include "../graphics/ui/elements/layout/Menu.h"
#include "../graphics/ui/gui_util.h"
#include "../graphics/ui/GUI.h"
#include "../logic/scripting/scripting.h"
#include "../settings.h"
#include "../util/stringutil.h"
#include "../window/Window.h"
#include "UIDocument.h"
#include "../core_content_defs.h"
#include "locale/langs.h"
#include "../interfaces/Task.h"
#include "../graphics/ui/elements/layout/Panel.h"
#include "../data/dynamic.h"
#include "../coders/commons.h"

gui::page_loader_func menus::create_page_loader(Engine* engine) {
    return [=](const std::string& query) {
        std::vector<dynamic::Value> args;

        std::string name;
        size_t index = query.find('?');
        if (index != std::string::npos) {
            auto argstr = query.substr(index+1);
            name = query.substr(0, index);

            auto map = std::make_shared<dynamic::Map>();
            BasicParser parser("Query for " + name, argstr);
            while (parser.hasNext()) {
                auto key = parser.readUntil('=');
                parser.nextChar();
                auto value = parser.readUntil('&');
                map->put(key, value);
            }
            args.push_back(map);
        } else {
            name = query;
        }
        auto file = engine->getResPaths()->find("layouts/pages/" + name + ".xml");
        auto fullname = BUILTIN_CONTENT_NAMESPACE + ":pages/" + name;
        auto document = UIDocument::read(scripting::get_root_environment(), fullname, file).release();
        engine->getAssets()->store(document, fullname);
        scripting::on_ui_open(document, std::move(args));
        return document->getRoot();
    };
}

void menus::create_version_label(Engine* engine) {
    auto gui = engine->getGUI();
    auto text = "v" + ENGINE_VERSION_STRING + " development build ";
    gui->add(guiutil::create(
        "<label z-index='1000' color='#FFFFFF80' gravity='bottom-left' margin='4'>"
        + text +
        "</label>"
    ));
}

UIDocument* menus::show(Engine* engine, const std::string& name, std::vector<dynamic::Value> args) {
    auto menu = engine->getGUI()->getMenu();
    auto file = engine->getResPaths()->find("layouts/" + name + ".xml");
    auto fullname = BUILTIN_CONTENT_NAMESPACE + ":layouts/" + name;

    auto document = UIDocument::read(scripting::get_root_environment(), fullname, file).release();
    engine->getAssets()->store(document, fullname);
    scripting::on_ui_open(document, std::move(args));
    menu->addPage(name, document->getRoot());
    menu->setPage(name);

    return document;
}

void menus::show_process_panel(Engine* engine, std::shared_ptr<Task> task, std::wstring text) {
    uint initialWork = task->getWorkTotal();

    auto menu = engine->getGUI()->getMenu();
    menu->reset();
    auto doc = menus::show(engine, "process", {
        util::wstr2str_utf8(langs::get(text))
    });
    std::dynamic_pointer_cast<gui::Container>(doc->getRoot())->listenInterval(0.01f, [=]() {
        task->update();

        uint tasksDone = task->getWorkDone();
        scripting::on_ui_progress(doc, tasksDone, initialWork);
    });
}
