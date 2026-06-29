#include <graphics/ui/gui_util.h>

#include <glm/glm.hpp>

#include <graphics/ui/elements/display/Label.h>
#include <graphics/ui/elements/layout/Menu.h>
#include <graphics/ui/elements/control/Button.h>
#include <frontend/locale/langs.h>
#include <delegates.h>
#include <graphics/ui/gui_xml.h>
#include <logic/scripting/scripting.h>
#include <util/stringutil.h>
#include <graphics/ui/elements/control/TextBox.h>
#include <window/Events.h>
#include <engine/Engine.h>

using namespace gui;

void guiutil::alert(
    Engine& engine,
    const std::wstring& text,
    const runnable& on_hidden
) {
    GUI& gui = engine.getGUI();
    auto panel = std::make_shared<Panel>(
        gui,
        glm::vec2(
            glm::min(
                static_cast<size_t>(650),
                glm::max(text.length() * 10, static_cast<size_t>(200))
            ),
            300
        ),
        glm::vec4(4.0f),
        4.0f
    );
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));

    auto menuPtr = gui.getMenu();
    auto& menu = *menuPtr;
    runnable on_hidden_final = [on_hidden, &menu]() {
        menu.removePage("<alert>");
        if (on_hidden) {
            on_hidden();
        } else if (!menu.back()) {
            menu.reset();
        }
    };

    auto label = std::make_shared<Label>(gui, text);
    label->setMultiline(true);
    label->setSize(glm::vec2(1, 24));
    label->setAutoResize(true);
    panel->add(label);
    panel->add(std::make_shared<Button>(
        gui,
        langs::get(L"Ok"),
        glm::vec4(10.f), 
        [on_hidden_final](GUI&) {
            on_hidden_final();
        }
    ));
    panel->refresh();

    auto& input = engine.getInput();
    panel->keepAlive(input.addKeyCallback(keycode::ENTER, [on_hidden_final]() {
        on_hidden_final();
        return true;
    }));
    panel->keepAlive(input.addKeyCallback(keycode::ESCAPE, [on_hidden_final]() {
        on_hidden_final();
        return true;
    }));
    menu.addPage("<alert>", panel, true);
    menu.setPage("<alert>");
}

void guiutil::confirm(
    Engine& engine,
    const std::wstring& text,
    const runnable& on_confirm,
    const runnable& on_deny,
    std::wstring yestext,
    std::wstring notext
) {
    if (yestext.empty()) yestext = langs::get(L"Yes");
    if (notext.empty()) notext = langs::get(L"No");    

    auto& gui = engine.getGUI();
    auto& input = engine.getInput();

    auto container = std::make_shared<Container>(
        gui, glm::vec2(5000, 5000)
    );
    container->setColor(glm::vec4(0.05f, 0.05f, 0.05f, 0.7f));
    auto panel = std::make_shared<Panel>(
        gui, glm::vec2(600, 200), glm::vec4(8.0f), 8.0f
    );
    panel->setGravity(Gravity::center_center);
    container->add(panel);
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(std::make_shared<Label>(gui, text));
    auto subpanel = std::make_shared<Panel>(gui, glm::vec2(600, 53));
    subpanel->setColor(glm::vec4(0));

    auto menu = gui.getMenu();

    runnable on_confirm_final = [on_confirm, menu]() {
        menu->removePage("<confirm>");
        if (on_confirm) {
            on_confirm();
        } else if (!menu->back()) {
            menu->reset();
        }
    };

    runnable on_deny_final = [on_deny, menu]() {
        menu->removePage("<confirm>");
        if (on_deny) {
            on_deny();
        } else if (!menu->back()) {
            menu->reset();
        }
    };

    subpanel->add(std::make_shared<Button>(gui, yestext, glm::vec4(8.f), [=](GUI&){
        on_confirm_final();
    }));

    subpanel->add(std::make_shared<Button>(gui, notext, glm::vec4(8.f), [=](GUI&){
        on_deny_final();
    }));

    panel->add(subpanel);
    panel->keepAlive(input.addKeyCallback(keycode::ENTER, [=]() {
        on_confirm_final();
        return true;
    }));
    panel->keepAlive(input.addKeyCallback(keycode::ESCAPE, [=]() {
        on_deny_final();
        return true;
    }));

    panel->refresh();
    menu->addPage("<confirm>", container, true);
    menu->setPage("<confirm>");
}

std::shared_ptr<gui::UINode> guiutil::create(
    GUI& gui, const std::string& source, scriptenv env
) {
    if (env == nullptr) env = scripting::get_root_environment();

    UIXmlReader reader(gui, env);
    return reader.readXML("[string]", source);
}

void guiutil::confirm_with_memo(
    Engine& engine,
    const std::wstring& text,
    const std::wstring& memo,
    const runnable& on_confirm,
    std::wstring yestext,
    std::wstring notext
) {
    auto& gui = engine.getGUI();
    auto menu = gui.getMenu();

    if (yestext.empty()) yestext = langs::get(L"Yes");
    if (notext.empty()) notext = langs::get(L"No");

    auto panel = std::make_shared<Panel>(
        gui, glm::vec2(600, 500), glm::vec4(8.0f), 8.0f
    );
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(std::make_shared<Label>(gui, text));

    auto textbox = std::make_shared<TextBox>(gui, L"");
    textbox->setMultiline(true);
    textbox->setTextWrapping(true);
    textbox->setSize(glm::vec2(600, 300));
    textbox->setText(memo);
    textbox->setEditable(false);
    panel->add(textbox);

    auto subpanel = std::make_shared<Panel>(gui, glm::vec2(600, 53));
    subpanel->setColor(glm::vec4(0));

    subpanel->add(std::make_shared<Button>(gui, yestext, glm::vec4(8.0f), [=](GUI&){
        if (on_confirm)
            on_confirm();
        menu->back();
    }));

    subpanel->add(std::make_shared<Button>(gui, notext, glm::vec4(8.0f), [=](GUI&){
        menu->back();
    }));

    panel->add(subpanel);

    panel->refresh();
    menu->addPage("<confirm>", panel, true);
    menu->setPage("<confirm>");
}
