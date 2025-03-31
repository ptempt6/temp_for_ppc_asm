#include "token.h"
#include <stdexcept>

Token::Token(TokenType type, const std::string& value, size_t line, size_t column)
        : type(type), value(value), line(line), column(column) {}

TokenType Token::getType() const { return type; }
const std::string& Token::getValue() const { return value; }
size_t Token::getLine() const { return line; }
size_t Token::getColumn() const { return column; }

std::ostream& operator<<(std::ostream& os, const Token& token) {
    const char* typeNames[] = {
            "INSTRUCTION", "REGISTER", "DIRECTIVE", "LABEL",
            "NUMBER", "COMMA", "LPAREN", "RPAREN",
            "PLUS", "MINUS", "COLON", "EOL", "UNKNOWN"
    };

    os << "Line " << token.line << ", Col " << token.column << ": "
       << typeNames[static_cast<int>(token.type)] << " '" << token.value << "'";
    return os;
}
