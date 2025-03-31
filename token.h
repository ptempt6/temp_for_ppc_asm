
#ifndef PPCASM_TOKEN_H
#define PPCASM_TOKEN_H


#include <string>
#include <ostream>

enum class TokenType {
    INSTRUCTION,
    REGISTER,
    DIRECTIVE,
    LABEL,
    NUMBER,
    COMMA,
    LPAREN,
    RPAREN,
    PLUS,
    MINUS,
    COLON,
    EOL,
    UNKNOWN
};

class Token {
public:
    Token(TokenType type, const std::string& value, size_t line, size_t column);

    TokenType getType() const;
    const std::string& getValue() const;
    size_t getLine() const;
    size_t getColumn() const;

    friend std::ostream& operator<<(std::ostream& os, const Token& token);

private:
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};


#endif //PPCASM_TOKEN_H
