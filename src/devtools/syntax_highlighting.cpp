#include <devtools/syntax_highlighting.h>

#include <coders/commons.h>
#include <coders/lua_parsing.h>
#include <graphics/core/Font.h>

using namespace devtools;

static std::unique_ptr<FontStylesScheme> build_styles(
    const std::vector<lua::Token>& tokens
) {
    FontStylesScheme styles {
        {
            {false, false, glm::vec4(0.8f, 0.8f, 0.8f, 1)}, // Default
            {true, false, glm::vec4(0.9, 0.6f, 0.4f, 1)},   // Keyword
            {false, false, glm::vec4(0.4, 0.8f, 0.5f, 1)},  // String
            {false, false, glm::vec4(0.3, 0.3f, 0.3f, 1)},  // Comment
            {false, false, glm::vec4(0.4, 0.45f, 0.5f, 1)}, // Self
            {true, false, glm::vec4(1.0f, 0.2f, 0.1f, 1)}, // Unexpected
        }, 
        {}
    };
    size_t offset = 0;
    for (int i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens.at(i);
        if (token.tag != lua::TokenTag::Keyword &&
            token.tag != lua::TokenTag::String &&
            token.tag != lua::TokenTag::Integer &&
            token.tag != lua::TokenTag::Number &&
            token.tag != lua::TokenTag::Comment &&
            token.tag != lua::TokenTag::Unexpected) {
            continue;
        }
        if (token.start.pos > offset) {
            int n = token.start.pos - offset;
            styles.map.insert(styles.map.end(), token.start.pos - offset, 0);
        }
        offset = token.end.pos;
        int styleIndex;
        switch (token.tag) {
            case lua::TokenTag::Keyword: styleIndex = 1; break;
            case lua::TokenTag::String:
            case lua::TokenTag::Integer:
            case lua::TokenTag::Number: styleIndex = 2; break;
            case lua::TokenTag::Comment: styleIndex = 3; break;
            case lua::TokenTag::Unexpected: styleIndex = 5; break;
            default:
                styleIndex = 0;
                break;
        }
        styles.map.insert(
            styles.map.end(), token.end.pos - token.start.pos, styleIndex
        );
    }
    styles.map.push_back(0);
    return std::make_unique<FontStylesScheme>(std::move(styles));
}

std::unique_ptr<FontStylesScheme> devtools::syntax_highlight(
    const std::string& lang, std::string_view source
) {
    try {
        if (lang == "lua") {
            auto tokens = lua::tokenize("<string>", source);
            return build_styles(tokens);
        } else {
            return nullptr;
        }
    } catch (const parsing_error& err) {
        return nullptr;
    }
}
