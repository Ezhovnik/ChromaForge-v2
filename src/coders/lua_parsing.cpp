#include <coders/lua_parsing.h>

#include <set>

#include <coders/BasicParser.h>
#include <debug/Logger.h>

using namespace lua;

static std::set<std::wstring_view> keywords {
    L"and", L"break", L"do", L"else", L"elseif", L"end", L"false", L"for", L"function", 
    L"if", L"in", L"local", L"nil", L"not", L"or", L"repeat", L"return", L"then", L"true",
    L"until", L"while"
};

static bool is_lua_keyword(std::wstring_view view) {
    return keywords.find(view) != keywords.end();
}

inline bool is_lua_identifier_start(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

inline bool is_lua_identifier_part(int c) {
    return is_lua_identifier_start(c) || is_digit(c);
}

inline bool is_lua_operator_start(int c) {
    return c == '=' || c == '~' || c == '+' || c == '-' || c == '/' || c == '*'
        || c == '%' || c == '^' || c == '#' || c == '<' || c == '>' || c == ':'
        || c == '.';
}

class Tokenizer : BasicParser<wchar_t> {
    std::vector<devtools::Token> tokens;
public:
    Tokenizer(std::string_view file, std::wstring_view source)
        : BasicParser(file, source) {
    }

    std::wstring parseLuaName() {
        char c = peek();
        if (!is_identifier_start(c)) {
            LOG_ERROR("Identifier expected");
            throw error("Identifier expected");
        }
        int start = pos;
        while (hasNext() && is_identifier_part(source[pos])) {
            pos++;
        }
        return std::wstring(source.substr(start, pos - start));
    }

    inline devtools::Location currentLocation() const {
        return devtools::Location {
            static_cast<int>(pos),
            static_cast<int>(linestart),
            static_cast<int>(line)};
    }

    void emitToken(
        devtools::TokenTag tag, std::wstring name, devtools::Location start, bool standalone=false
    ) {
        tokens.emplace_back(
            tag,
            std::move(name),
            std::move(start),
            currentLocation()
        );
        if (standalone) skip(1);
    }

    std::wstring parseOperator() {
        int start = pos;
        wchar_t first = peek();
        switch (first) {
            case '#': case '+': case '/': case '*': case '^':
            case '%':
                skip(1);
                return std::wstring({first});
            case '-':
                skip(1);
                if (hasNext() && peekNoJump() == '-') {
                    skip(1);
                    return L"--";
                }
                return std::wstring({first});
        }
        skip(1);
        char second = peekNoJump();
        if ((first == '=' && second == '=') || (first == '~' && second == '=') ||
            (first == '<' && second == '=') || (first == '>' && second == '=')) {
            skip(1);
            return std::wstring(source.substr(start, pos - start));
        }
        if (first == '.' && second == '.') {
            skip(1);
            if (peekNoJump() == '.') {
                skip(1);
            }
        }
        return std::wstring(source.substr(start, pos - start));
    }

    std::vector<devtools::Token> tokenize() {
        skipWhitespace();
        while (hasNext()) {
            skipWhitespace();
            if (!hasNext()) {
                continue;
            }
            wchar_t c = peek();
            auto start = currentLocation();
            if (is_lua_identifier_start(c)) {
                auto name = parseLuaName();
                devtools::TokenTag tag = (is_lua_keyword(name) ? devtools::TokenTag::Keyword : devtools::TokenTag::Name);
                emitToken(
                    tag,
                    std::move(name),
                    start
                );
                continue;
            } else if (is_digit(c)) {
                dv::value value;
                auto tag = devtools::TokenTag::Unexpected;
                try {
                    value = parseNumber(1);
                    tag = value.isInteger() ? devtools::TokenTag::Integer : devtools::TokenTag::Number;
                } catch (const parsing_error& err) {}
                auto literal = source.substr(start.pos, pos - start.pos);
                emitToken(tag, std::wstring(literal), start);
                continue;
            }
            switch (c) {
                case '(': case '[': case '{': 
                    if (isNext(L"[==[")) {
                        auto string = readUntil(L"]==]", true);
                        skip(4);
                        emitToken(
                            devtools::TokenTag::Comment,
                            std::wstring(string) + L"]==]",
                            start
                        );
                        continue;
                    } else if (isNext(L"[[")) {
                        skip(2);
                        auto string = readUntil(L"]]", true);
                        skip(2);
                        emitToken(
                            devtools::TokenTag::String,
                            std::wstring(string),
                            start
                        );
                        continue;
                    }
                    emitToken(
                        devtools::TokenTag::OpenBracket,
                        std::wstring({c}),
                        start,
                        true
                    );
                    continue;
                case ')': case ']': case '}': 
                    emitToken(
                        devtools::TokenTag::CloseBracket,
                        std::wstring({c}),
                        start,
                        true
                    );
                    continue;
                case ',':
                    emitToken(
                        devtools::TokenTag::Comma,
                        std::wstring({c}),
                        start,
                        true
                    );
                    continue;
                case ';':
                    emitToken(
                        devtools::TokenTag::Semicolon,
                        std::wstring({c}),
                        start,
                        true
                    );
                    continue;
                case '\'': case '"': {
                    skip(1);
                    auto string = parseString(c, false);
                    emitToken(
                        devtools::TokenTag::String,
                        std::move(string),
                        start
                    );
                    continue;
                }
                default: break;
            }
            if (is_lua_operator_start(c)) {
                auto text = parseOperator();
                if (text == L"--") {
                    auto string = readUntilEOL();
                    emitToken(
                        devtools::TokenTag::Comment,
                        std::wstring(string),
                        start
                    );
                    skipLine();
                    continue;
                }
                emitToken(
                    devtools::TokenTag::Operator,
                    std::move(text),
                    start
                );
                continue;
            }
            auto text = readUntilWhitespace();
            emitToken(
                devtools::TokenTag::Unexpected,
                std::wstring(text),
                start
            );
        }
        return std::move(tokens);
    }
};

std::vector<devtools::Token> lua::tokenize(std::string_view file, std::wstring_view source) {
    return Tokenizer(file, source).tokenize();
}
