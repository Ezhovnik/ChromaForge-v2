#ifndef GRAPHICS_UI_GUI_UTIL_H_
#define GRAPHICS_UI_GUI_UTIL_H_

#include <string>
#include <memory>

#include "GUI.h"
#include "../../delegates.h"
#include "../../typedefs.h"

namespace gui {
    class Button;
}

namespace guiutil {
    std::shared_ptr<gui::Button> backButton(std::shared_ptr<gui::Menu> menu);
    std::shared_ptr<gui::Button> gotoButton(std::wstring text, const std::string& page, std::shared_ptr<gui::Menu> menu);
    void alert(gui::GUI* gui, const std::wstring& text, runnable on_hidden = nullptr);
    void confirm(gui::GUI* gui, const std::wstring& text, runnable on_confirm = nullptr, std::wstring yestext=L"", std::wstring notext=L"");
    std::shared_ptr<gui::UINode> create(const std::string& source, scriptenv env=0);
}

#endif // GRAPHICS_UI_GUI_UTIL_H_
