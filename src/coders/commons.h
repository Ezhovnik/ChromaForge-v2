#ifndef CODERS_COMMONS_H_
#define CODERS_COMMONS_H_

#include <string>
#include <stdexcept>

#include "../typedefs.h"

// Определение системы счисления по префиксу (0x, 0b, 0o)
inline int detect_base(char c) noexcept {
    switch (c) {
        case 'B':
        case 'b':
            return 2;
        case 'O':
        case 'o':
            return 8;
        case 'X':
        case 'x':
            return 16; 
    }
    return 10;
}

// Проверка, является ли символ цифрой
inline bool is_digit(char c) noexcept {
    return (c >= '0' && c <= '9');
}

// Проверка, является ли символ пробельным (пробел, перенос строки и т.д.)
inline bool is_whitespace(char c) noexcept {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f';
}

// Проверка, может ли символ быть началом идентификатора
inline bool is_identifier_start(char c) noexcept {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '-' || c == '.';
}

// Проверка, может ли символ быть частью идентификатора
inline bool is_identifier_part(char c) noexcept {
    return is_identifier_start(c) || is_digit(c);
}

extern std::string escape_string(std::string s);

class parsing_error : public std::runtime_error {
public:
    std::string filename;
    std::string source;
    uint pos;
    uint line;
    uint linestart;

    parsing_error(std::string message, 
                    std::string filename, 
                    std::string source, 
                    uint pos, 
                    uint line, 
                    uint linestart);

    std::string errorLog() const;
};

class BasicParser {
protected:
    std::string filename;
    std::string source;
    uint pos = 0;
    uint line = 1;
    uint linestart = 0;

    virtual void skipWhitespace();
    void expect(char expected);
    char peek();
    char nextChar();
    bool hasNext();
    void expectNewLine();

    std::string parseName();
    int64_t parseSimpleInt(int base);
    double parseNumber(int sign);
    std::string parseString(char chr);

    parsing_error error(std::string message);

    BasicParser(std::string filename, std::string source);
};

#endif // CODERS_COMMONS_H_
