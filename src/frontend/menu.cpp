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

void menus::create_menus(Engine* engine) {
    auto menu = engine->getGUI()->getMenu();
    menu->setPageLoader([=](auto name) {
        auto file = engine->getResPaths()->find("layouts/pages/" + name + ".xml");
        auto fullname = BUILTIN_CONTENT_NAMESPACE + ":pages/" + name;
        auto document = UIDocument::read(scripting::get_root_environment(), fullname, file).release();
        engine->getAssets()->store(document, fullname);
        scripting::on_ui_open(document, {});
        return document->getRoot();
    });
}

void menus::create_version_label(Engine* engine) {
    auto gui = engine->getGUI();
    auto text = "v" + ENGINE_VERSION_STRING + " development build ";
    gui->add(guiutil::create(
        "<label z-index='1000' color='#FFFFFF80' gravity='top-right' margin='4'>"
        + text +
        "</label>"
    ));
}

void menus::show(Engine* engine, const std::string& name, std::vector<std::unique_ptr<dynamic::Value>> args) {
    auto menu = engine->getGUI()->getMenu();
    auto file = engine->getResPaths()->find("layouts/" + name + ".xml");
    auto fullname = BUILTIN_CONTENT_NAMESPACE + ":layouts/" + name;

    auto document = UIDocument::read(scripting::get_root_environment(), fullname, file).release();
    engine->getAssets()->store(document, fullname);
    scripting::on_ui_open(document, std::move(args));
    menu->addPage(name, document->getRoot());
    menu->setPage(name);
}

void menus::show_process_panel(Engine* engine, std::shared_ptr<Task> task, std::wstring text) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = std::dynamic_pointer_cast<gui::Panel>(guiutil::create(
        "<panel size='400' padding='8' interval='1' color='#00000080/>"
    ));
    if (!text.empty()) {
        panel->add(std::make_shared<gui::Label>(langs::get(text)));
    }

    auto label = std::make_shared<gui::Label>(L"0%");
    panel->add(label);

    uint initialWork = task->getWorkTotal();

    panel->listenInterval(0.01f, [=]() {
        task->update();

        uint tasksDone = task->getWorkDone();
        float progress = tasksDone/static_cast<float>(initialWork);
        label->setText(
            std::to_wstring(tasksDone)+
            L"/" + std::to_wstring(initialWork) + L" (" +
            std::to_wstring(static_cast<int>(progress * 100)) + L"%)"
        );
    });

    menu->reset();
    menu->addPage("process", panel);
    menu->setPage("process", false);
}
