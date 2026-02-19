#include "gui_util.h"
#include "controls.h"
#include "panels.h"

#include <glm/glm.hpp>

using namespace gui;

Button* guiutil::backButton(PagesControl* menu) {
    return (new Button(L"Back", glm::vec4(10.f)))->listenAction([=](GUI* gui) {
        menu->back();
    });
}

Button* guiutil::gotoButton(std::wstring text, std::string page, PagesControl* menu) {
    return (new Button(text, glm::vec4(10.f)))->listenAction([=](GUI* gui) {
        menu->set(page);
    });
}

void guiutil::alert(GUI* gui, std::wstring text, gui::runnable on_hidden) {
    PagesControl* menu = gui->getMenu();
    Panel* panel = new Panel(glm::vec2(500, 200), glm::vec4(8.0f), 8.0f);
    panel->color(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(new Label(text));
    panel->add((new Button(L"Ok", glm::vec4(10.f)))->listenAction([=](GUI* gui) {
        if (on_hidden) on_hidden();
        menu->back();
    }));
    panel->refresh();
    menu->add("<alert>", panel);
    menu->set("<alert>");
}

void guiutil::confirm(GUI* gui, std::wstring text, gui::runnable on_confirm) {
    PagesControl* menu = gui->getMenu();
    Panel* panel = new Panel(glm::vec2(500, 200), glm::vec4(8.0f), 8.0f);
    panel->color(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    panel->add(new Label(text));
    Panel* subpanel = new Panel(glm::vec2(500, 53));
    subpanel->color(glm::vec4(0));
    subpanel->add((new Button(L"Yes", glm::vec4(8.0f)))->listenAction([=](GUI*){
        if (on_confirm) on_confirm();
        menu->back();
    }));
    subpanel->add((new Button(L"No", glm::vec4(8.0f)))->listenAction([=](GUI*){
        menu->back();
    }));
    panel->add(subpanel);

    panel->refresh();
    menu->add("<confirm>", panel);
    menu->set("<confirm>");
}
