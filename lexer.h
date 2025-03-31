#ifndef PPCASM_LEXER_H
#define PPCASM_LEXER_H


#include <vector>
#include <string>
#include <regex>
#include "token.h"

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    struct TokenPattern {
        std::regex pattern;
        TokenType type;
    };

    void initializePatterns();
    void skipWhitespaceAndComments();
    bool tryMatchPattern(Token& token);

    const std::string source;
    size_t pos;
    size_t line;
    size_t column;
    std::vector<TokenPattern> tokenPatterns;
};



#endif //PPCASM_LEXER_H
