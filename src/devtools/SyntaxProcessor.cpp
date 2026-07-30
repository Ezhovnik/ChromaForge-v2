#include <devtools/SyntaxProcessor.h>

#include <coders/commons.h>
#include <coders/syntax_parser.h>
#include <graphics/core/Font.h>

static std::unique_ptr<FontStylesScheme> build_styles(
    const FontStylesScheme& colorScheme,
    const std::vector<devtools::Token>& tokens
) {
    FontStylesScheme styles {colorScheme.palette, {}};
    if (styles.palette.empty()) {
        styles.palette.push_back(FontStyle {
            false, false, false, false, glm::vec4(0.8f, 0.8f, 0.8f, 1)
        });
    }
    size_t offset = 0;
    for (int i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens.at(i);
        if (token.tag != devtools::TokenTag::Keyword &&
            token.tag != devtools::TokenTag::String &&
            token.tag != devtools::TokenTag::Integer &&
            token.tag != devtools::TokenTag::Number &&
            token.tag != devtools::TokenTag::Comment &&
            token.tag != devtools::TokenTag::Unexpected) {
            continue;
        }
        if (token.start.pos > offset) {
            styles.map.insert(styles.map.end(), token.start.pos - offset, 0);
        }
        offset = token.end.pos;
        int styleIndex;
        switch (token.tag) {
            case devtools::TokenTag::Keyword: styleIndex = devtools::SyntaxStyles::Keyword; break;
            case devtools::TokenTag::String:
            case devtools::TokenTag::Integer:
            case devtools::TokenTag::Number: styleIndex = devtools::SyntaxStyles::Literal; break;
            case devtools::TokenTag::Comment: styleIndex = devtools::SyntaxStyles::Comment; break;
            case devtools::TokenTag::Unexpected: styleIndex = devtools::SyntaxStyles::Error; break;
            default:
                styleIndex = 0;
                break;
        }
        if (styleIndex >= styles.palette.size()) {
            styleIndex = 0;
        }
        styles.map.insert(
            styles.map.end(), token.end.pos - token.start.pos, styleIndex
        );
    }
    styles.map.push_back(0);
    return std::make_unique<FontStylesScheme>(std::move(styles));
}

void devtools::SyntaxProcessor::addSyntax(
    std::unique_ptr<Syntax> syntax
) {
    const auto ptr = syntax.get();
    langs.emplace_back(std::move(syntax));

    for (auto& ext : ptr->extensions) {
        langsExtensions[ext] = ptr;
    }
}

std::unique_ptr<FontStylesScheme> devtools::SyntaxProcessor::highlight(
    const FontStylesScheme& colorScheme,
    const std::string& ext,
    std::wstring_view source
) const {
    const auto& found = langsExtensions.find(ext);
    if (found == langsExtensions.end()) {
        return nullptr;
    } 
    const auto& syntax = *found->second;
    try {
        auto tokens = tokenize(syntax, "<string>", source);
        return build_styles(colorScheme, tokens);
    } catch (const parsing_error& err) {
        return nullptr;
    }
}
