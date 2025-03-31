

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <memory>
#include <stdexcept>


enum class TokenType {
    IDENTIFIER,
    REGISTER,
    NUMBER,
    LPAREN,
    RPAREN,
    COMMA,
    COLON,
    DOT,
    END_OF_LINE,
    END_OF_FILE,
    UNKNOWN
};


struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t, const std::string& v, int l, int c)
            : type(t), value(v), line(l), column(c) {}
};


struct NFAState {
    int id;
    bool is_accepting;
    TokenType accepting_type;

    NFAState(int i, bool accept = false, TokenType t = TokenType::UNKNOWN)
            : id(i), is_accepting(accept), accepting_type(t) {}
};


struct NFATransition {
    std::shared_ptr<NFAState> from;
    std::shared_ptr<NFAState> to;
    char input;

    NFATransition(std::shared_ptr<NFAState> f, std::shared_ptr<NFAState> t, char c = 0)
            : from(f), to(t), input(c) {}
};


class NFA {
    public:
    std::shared_ptr<NFAState> start_state;
    std::vector<std::shared_ptr<NFAState>> states;
    std::vector<NFATransition> transitions;


    std::set<int> epsilon_closure(std::shared_ptr<NFAState> state) {
        std::set<int> closure;
        std::vector<std::shared_ptr<NFAState>> stack;
        stack.push_back(state);

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();

            if (closure.insert(current->id).second) {
                for (const auto& trans : transitions) {
                    if (trans.from->id == current->id && trans.input == 0) {
                        stack.push_back(trans.to);
                    }
                }
            }
        }

        return closure;
    }


    std::set<int> epsilon_closure(const std::set<int>& state_ids) {
        std::set<int> closure;
        for (int id : state_ids) {
            auto s_closure = epsilon_closure(states[id]);
            closure.insert(s_closure.begin(), s_closure.end());
        }
        return closure;
    }


    std::set<int> move(const std::set<int>& state_ids, char c) {
        std::set<int> new_states;
        for (int id : state_ids) {
            for (const auto& trans : transitions) {
                if (trans.from->id == id && trans.input == c) {
                    new_states.insert(trans.to->id);
                }
            }
        }
        return new_states;
    }

public:
    NFA() {

        start_state = std::make_shared<NFAState>(0);
        states.push_back(start_state);
    }

    void add_state(std::shared_ptr<NFAState> state) {
        states.push_back(state);
    }

    void add_transition(std::shared_ptr<NFAState> from,
                        std::shared_ptr<NFAState> to, char c = 0) {
        transitions.emplace_back(from, to, c);
    }


    bool accepts(const std::string& input, TokenType& token_type) {
        std::set<int> current_states = epsilon_closure(start_state);

        for (char c : input) {
            current_states = epsilon_closure(move(current_states, c));
            if (current_states.empty()) break;
        }

        for (int id : current_states) {
            if (states[id]->is_accepting) {
                token_type = states[id]->accepting_type;
                return true;
            }
        }

        return false;
    }


    std::pair<std::string, TokenType> longest_match(const std::string& input, size_t start_pos) {
        std::set<int> current_states = epsilon_closure(start_state);
        size_t last_accept_pos = start_pos;
        TokenType last_accept_type = TokenType::UNKNOWN;

        for (size_t pos = start_pos; pos < input.size(); pos++) {
            char c = input[pos];
            current_states = epsilon_closure(move(current_states, c));

            if (current_states.empty()) break;


            for (int id : current_states) {
                if (states[id]->is_accepting) {
                    last_accept_pos = pos + 1;
                    last_accept_type = states[id]->accepting_type;
                    break;
                }
            }
        }

        if (last_accept_pos > start_pos) {
            return {input.substr(start_pos, last_accept_pos - start_pos), last_accept_type};
        }

        return {"", TokenType::UNKNOWN};
    }
};


class Lexer {
    std::string input;
    int position;
    int line;
    int column;

    NFA nfa;

    void build_nfa() {



        auto id_start = std::make_shared<NFAState>(1);
        auto id_accept = std::make_shared<NFAState>(2, true, TokenType::IDENTIFIER);
        nfa.add_state(id_start);
        nfa.add_state(id_accept);


        for (char c = 'a'; c <= 'z'; c++) nfa.add_transition(id_start, id_accept, c);
        for (char c = 'A'; c <= 'Z'; c++) nfa.add_transition(id_start, id_accept, c);
        nfa.add_transition(id_start, id_accept, '_');


        for (char c = 'a'; c <= 'z'; c++) nfa.add_transition(id_accept, id_accept, c);
        for (char c = 'A'; c <= 'Z'; c++) nfa.add_transition(id_accept, id_accept, c);
        for (char c = '0'; c <= '9'; c++) nfa.add_transition(id_accept, id_accept, c);
        nfa.add_transition(id_accept, id_accept, '_');
        nfa.add_transition(id_accept, id_accept, '.');


        auto reg_start = std::make_shared<NFAState>(3);
        auto reg_r = std::make_shared<NFAState>(4);
        auto reg_digit1 = std::make_shared<NFAState>(5, true, TokenType::REGISTER);
        auto reg_digit2 = std::make_shared<NFAState>(6, true, TokenType::REGISTER);
        nfa.add_state(reg_start);
        nfa.add_state(reg_r);
        nfa.add_state(reg_digit1);
        nfa.add_state(reg_digit2);

        nfa.add_transition(reg_start, reg_r, 'r');
        for (char c = '0'; c <= '9'; c++) nfa.add_transition(reg_r, reg_digit1, c);
        for (char c = '0'; c <= '9'; c++) nfa.add_transition(reg_digit1, reg_digit2, c);


        auto num_start = std::make_shared<NFAState>(7);
        auto num_digit = std::make_shared<NFAState>(8, true, TokenType::NUMBER);
        auto num_hex = std::make_shared<NFAState>(9);
        auto num_hex_digit = std::make_shared<NFAState>(10, true, TokenType::NUMBER);
        nfa.add_state(num_start);
        nfa.add_state(num_digit);
        nfa.add_state(num_hex);
        nfa.add_state(num_hex_digit);

        for (char c = '0'; c <= '9'; c++) {
            nfa.add_transition(num_start, num_digit, c);
            nfa.add_transition(num_digit, num_digit, c);
        }

        nfa.add_transition(num_start, num_hex, '0');
        nfa.add_transition(num_hex, num_hex_digit, 'x');

        for (char c = '0'; c <= '9'; c++) nfa.add_transition(num_hex_digit, num_hex_digit, c);
        for (char c = 'a'; c <= 'f'; c++) nfa.add_transition(num_hex_digit, num_hex_digit, c);
        for (char c = 'A'; c <= 'F'; c++) nfa.add_transition(num_hex_digit, num_hex_digit, c);


        auto lparen = std::make_shared<NFAState>(11, true, TokenType::LPAREN);
        auto rparen = std::make_shared<NFAState>(12, true, TokenType::RPAREN);
        auto comma = std::make_shared<NFAState>(13, true, TokenType::COMMA);
        auto colon = std::make_shared<NFAState>(14, true, TokenType::COLON);
        auto dot = std::make_shared<NFAState>(15, true, TokenType::DOT);
        nfa.add_state(lparen);
        nfa.add_state(rparen);
        nfa.add_state(comma);
        nfa.add_state(colon);
        nfa.add_state(dot);

        nfa.add_transition(nfa.start_state, lparen, '(');
        nfa.add_transition(nfa.start_state, rparen, ')');
        nfa.add_transition(nfa.start_state, comma, ',');
        nfa.add_transition(nfa.start_state, colon, ':');
        nfa.add_transition(nfa.start_state, dot, '.');


        auto eol = std::make_shared<NFAState>(16, true, TokenType::END_OF_LINE);
        nfa.add_state(eol);
        nfa.add_transition(nfa.start_state, eol, '\n');
    }

    void skip_whitespace() {
        while (position < input.size()) {
            char c = input[position];
            if (c == ' ' || c == '\t') {
                position++;
                column++;
            } else if (c == '#') {

                while (position < input.size() && input[position] != '\n') {
                    position++;
                }
                column = 1;
            } else {
                break;
            }
        }
    }

public:
    Lexer(const std::string& src)
            : input(src), position(0), line(1), column(1) {
        build_nfa();
    }

    Token next_token() {
        skip_whitespace();

        if (position >= input.size()) {
            return Token(TokenType::END_OF_FILE, "", line, column);
        }

        auto [value, type] = nfa.longest_match(input, position);

        if (type != TokenType::UNKNOWN) {
            int start_column = column;
            position += value.size();
            column += value.size();

            if (type == TokenType::END_OF_LINE) {
                line++;
                column = 1;
            }

            return Token(type, value, line, start_column);
        }


        char c = input[position];
        std::string unknown(1, c);
        position++;
        column++;
        return Token(TokenType::UNKNOWN, unknown, line, column - 1);
    }
};


int main() {
    std::string test_code = R"(
        add r3, r1, r2
        addi r4, r0, 0x10
        loop:
        stw r5, 8(r1)
        b loop
    )";

    Lexer lexer(test_code);

    while (true) {
        Token token = lexer.next_token();

        std::cout << "Token: " << static_cast<int>(token.type)
                  << " (" << token.value << ") at line " << token.line
                  << ", column " << token.column << "\n";

        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }

    return 0;
}