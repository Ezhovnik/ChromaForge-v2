#pragma once

#include <string>

namespace devtools {
    struct Location {
        int pos;
        int lineStart;
        int line;
    };

    enum class TokenTag {
        Keyword,
        Name,
        Integer,
        Number,
        OpenBracket,
        CloseBracket,
        String,
        Operator,
        Comma,
        Semicolon,
        Unexpected,
        Comment
    };

    struct Token {
        TokenTag tag;
        std::wstring text;
        Location start;
        Location end;

        Token(TokenTag tag, std::wstring text, Location start, Location end)
            : tag(tag),
                text(std::move(text)),
                start(std::move(start)),
                end(std::move(end)) {}
    };
}
