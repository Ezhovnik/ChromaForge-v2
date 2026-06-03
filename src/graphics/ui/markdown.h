#pragma once

#include <string>
#include <memory>

struct FontStylesScheme;

namespace markdown {
    template <typename CharT>
    struct Result {
        std::basic_string<CharT> text;
        std::unique_ptr<FontStylesScheme> styles;
    };

    Result<char> process(std::string_view source, bool eraseMarkdown);
    Result<wchar_t> process(std::wstring_view source, bool eraseMarkdown);
}
