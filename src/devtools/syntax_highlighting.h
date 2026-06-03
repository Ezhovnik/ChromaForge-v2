#pragma once

#include <string>
#include <memory>

struct FontStylesScheme;

namespace devtools {
    enum SyntaxStyles {
        Default,
        Keyword,
        Literal,
        Comment,
        Error
    };

    std::unique_ptr<FontStylesScheme> syntax_highlight(
        const std::string& lang, std::string_view source
    );
}
