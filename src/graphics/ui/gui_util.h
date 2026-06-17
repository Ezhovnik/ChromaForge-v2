#pragma once

#include <string>
#include <memory>

#include <graphics/ui/GUI.h>
#include <delegates.h>
#include <typedefs.h>

namespace guiutil {
    void alert(
        const std::shared_ptr<gui::Menu>& menu,
        const std::wstring& text,
        const runnable& on_hidden = nullptr
    );
    void confirm(
        const std::shared_ptr<gui::Menu>& menu,
        const std::wstring& text,
        const runnable& on_confirm = nullptr,
        const runnable& on_deny=nullptr,
        std::wstring yestext=L"",
        std::wstring notext=L""
    );
    std::shared_ptr<gui::UINode> create(
        const std::string& source, 
        scriptenv env=0
    );
    void confirm_with_memo(
        const std::shared_ptr<gui::Menu>& menu,
        const std::wstring& text,
        const std::wstring& memo,
        const runnable& on_confirm=nullptr,
        std::wstring yestext=L"",
        std::wstring notext=L""
    );
}
