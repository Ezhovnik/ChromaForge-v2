#ifndef GRAPHICS_UI_GUI_UTIL_H_
#define GRAPHICS_UI_GUI_UTIL_H_

#include <string>
#include <memory>

#include "GUI.h"
#include "delegates.h"
#include "typedefs.h"

namespace guiutil {
    void alert(
        gui::GUI* gui, 
        const std::wstring& text, 
        const runnable& on_hidden = nullptr
    );
    void confirm(
        gui::GUI* gui, 
        const std::wstring& text, 
        const runnable& on_confirm = nullptr, 
        std::wstring yestext=L"", 
        std::wstring notext=L""
    );
    std::shared_ptr<gui::UINode> create(
        const std::string& source, 
        scriptenv env=0
    );
}

#endif // GRAPHICS_UI_GUI_UTIL_H_
