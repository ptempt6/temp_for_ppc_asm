#include "lexer.h"
#include <stdexcept>
#include <iostream>

Lexer::Lexer(const std::string& source)
        : source(source), pos(0), line(1), column(1) {
    initializePatterns();
}

void Lexer::initializePatterns() {

    tokenPatterns.push_back({std::regex("^r[0-9]+"), TokenType::REGISTER});
    tokenPatterns.push_back({std::regex("^cr[0-7]"), TokenType::REGISTER});
    tokenPatterns.push_back({std::regex("^lr"), TokenType::REGISTER});
    tokenPatterns.push_back({std::regex("^ctr"), TokenType::REGISTER});
    tokenPatterns.push_back({std::regex("^xer"), TokenType::REGISTER});


    tokenPatterns.push_back({std::regex("^0[xX][0-9a-fA-F]+"), TokenType::NUMBER});
    tokenPatterns.push_back({std::regex("^[+-]?[0-9]+"), TokenType::NUMBER});


    tokenPatterns.push_back({std::regex("^addi?"), TokenType::INSTRUCTION});
    tokenPatterns.push_back({std::regex("^lwz"), TokenType::INSTRUCTION});
    tokenPatterns.push_back({std::regex("^stw"), TokenType::INSTRUCTION});
    tokenPatterns.push_back({std::regex("^b(l?r?)"), TokenType::INSTRUCTION});
    tokenPatterns.push_back({std::regex("^cmp"), TokenType::INSTRUCTION});
    tokenPatterns.push_back({std::regex("^m[tf]lr"), TokenType::INSTRUCTION});



    tokenPatterns.push_back({std::regex("^\\.[a-zA-Z]+"), TokenType::DIRECTIVE});


    tokenPatterns.push_back({std::regex("^[a-zA-Z_][a-zA-Z0-9_]*:"), TokenType::LABEL});


    tokenPatterns.push_back({std::regex("^,"), TokenType::COMMA});
    tokenPatterns.push_back({std::regex("^\\("), TokenType::LPAREN});
    tokenPatterns.push_back({std::regex("^\\)"), TokenType::RPAREN});
    tokenPatterns.push_back({std::regex("^\\+"), TokenType::PLUS});
    tokenPatterns.push_back({std::regex("^-"), TokenType::MINUS});
    tokenPatterns.push_back({std::regex("^:"), TokenType::COLON});
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < source.length()) {
        skipWhitespaceAndComments();
        if (pos >= source.length()) break;

        Token token(TokenType::UNKNOWN, "", line, column);
        if (tryMatchPattern(token)) {
            tokens.push_back(token);
        } else {

            std::string value(1, source[pos]);
            tokens.emplace_back(TokenType::UNKNOWN, value, line, column);
            pos++;
            column++;
        }
    }

    tokens.emplace_back(TokenType::EOL, "", line, column);
    return tokens;
}

void Lexer::skipWhitespaceAndComments() {
    while (pos < source.length()) {
        if (isspace(source[pos])) {
            if (source[pos] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            pos++;
        } else if (source[pos] == '#') {

            while (pos < source.length() && source[pos] != '\n') {
                pos++;
                column++;
            }
        } else {
            break;
        }
    }
}

bool Lexer::tryMatchPattern(Token& token) {
    std::string remaining = source.substr(pos);
    std::smatch match;

    for (const auto& pattern : tokenPatterns) {
        if (std::regex_search(remaining, match, pattern.pattern,
                              std::regex_constants::match_continuous)) {
            std::string value = match.str();
            token = Token(pattern.type, value, line, column);


            pos += value.length();
            column += value.length();

            return true;
        }
    }

    return false;
}
