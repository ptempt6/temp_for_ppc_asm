
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "lexer.h"

class PowerPCParser {
public:
    PowerPCParser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {
        initializeInstructions();
    }


    std::vector<PowerPCInstruction> parse() {
        std::vector<PowerPCInstruction> instructions;

        while (!isAtEnd()) {
            try {

                while (match({TokenType::DIRECTIVE, TokenType::LABEL})) {
                    advance();
                }

                if (check(TokenType::INSTRUCTION)) {
                    instructions.push_back(parseInstruction());
                } else {
                    advance();
                    std::cerr << "Warning: Unknown token at line " << previous().getLine() << std::endl;
                }
            } catch (const std::runtime_error& e) {
                std::cerr << "Error at line " << currentToken().getLine() << ": " << e.what() << std::endl;
                synchronize();
            }
        }

        return instructions;
    }

private:
    const std::vector<Token>& tokens;
    size_t current;


    std::unordered_map<std::string, PowerPCInstruction> instructionSet;

    void initializeInstructions() {

        PowerPCInstruction add;
        add.name = "Add";
        add.primary_mnemonic = "add";
        add.syntax_variants = {
                {"add",  "rD,rA,rB", false, false},
                {"add.", "rD,rA,rB", false, true},
                {"addo", "rD,rA,rB", true,  false},
                {"addo.", "rD,rA,rB", true,  true}
        };
        add.power_mnemonics = {"cax", "cax.", "caxo", "caxo."};


        add.encoding.base_opcode = 0x7C000214;
        add.encoding.addField("D", 6, 10);
        add.encoding.addField("A", 11, 15);
        add.encoding.addField("B", 16, 20);
        add.encoding.addField("OE", 21, 21);
        add.encoding.addField("XO", 22, 30);
        add.encoding.addField("Rc", 31, 31);


        add.pseudocode = "rD ← (rA) + (rB)";
        add.description = "The sum (rA) + (rB) is placed into rD.";


        add.effects = {true, true, true, true, true, true, false};


        add.arch_level = ArchLevel::USIA;
        add.privilege_level = PrivilegeLevel::User;
        add.is_optional = false;
        add.form = InstructionForm::XO;

        instructionSet["add"] = add;
        instructionSet["add."] = add;
        instructionSet["addo"] = add;
        instructionSet["addo."] = add;


    }


    bool isAtEnd() const { return current >= tokens.size(); }
    const Token& currentToken() const { return tokens.at(current); }
    const Token& previous() const { return tokens.at(current - 1); }
    const Token& advance() { return tokens.at(current++); }

    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return currentToken().getType() == type;
    }

    bool match(const std::vector<TokenType>& types) {
        for (const auto& type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }


    void synchronize() {
        advance();
        while (!isAtEnd()) {
            if (previous().getType() == TokenType::EOL) return;
            if (check(TokenType::INSTRUCTION)) return;
            advance();
        }
    }


    PowerPCInstruction parseInstruction() {
        const Token& instrToken = advance();
        auto it = instructionSet.find(instrToken.getValue());
        if (it == instructionSet.end()) {
            throw std::runtime_error("Unknown instruction: " + instrToken.getValue());
        }

        PowerPCInstruction instruction = it->second;


        if (instruction.primary_mnemonic == "add") {
            parseAddOperands(instruction);
        }



        if (!match({TokenType::EOL})) {
            throw std::runtime_error("Expected end of line after instruction");
        }

        return instruction;
    }


    void parseAddOperands(PowerPCInstruction& instruction) {



        if (!check(TokenType::REGISTER)) {
            throw std::runtime_error("Expected register as first operand");
        }
        std::string rd = advance().getValue();


        if (!match({TokenType::COMMA})) {
            throw std::runtime_error("Expected comma after first operand");
        }


        if (!check(TokenType::REGISTER)) {
            throw std::runtime_error("Expected register as second operand");
        }
        std::string ra = advance().getValue();


        if (!match({TokenType::COMMA})) {
            throw std::runtime_error("Expected comma after second operand");
        }


        if (!check(TokenType::REGISTER)) {
            throw std::runtime_error("Expected register as third operand");
        }
        std::string rb = advance().getValue();


    }
};

