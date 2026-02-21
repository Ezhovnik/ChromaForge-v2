#ifndef CODERS_COMMONS_H_
#define CODERS_COMMONS_H_

#include <string>
#include <stdexcept>
#include "../typedefs.h"

union number_u {
    double fval;
    int64_t ival;
};

inline int detect_base(char c) {
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

inline bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}

inline bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f';
}

inline bool is_identifier_start(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '-' || c == '.';
}

inline bool is_identifier_part(char c) {
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
    void skipLine();

    std::string parseName();
    int64_t parseSimpleInt(int base);
    bool parseNumber(int sign, number_u& out);
    std::string parseString(char chr, bool closeRequired = true);

    parsing_error error(std::string message);

    BasicParser(std::string filename, std::string source);
};

#endif // CODERS_COMMONS_H_
