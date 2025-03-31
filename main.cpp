#include <string>
#include "lexer.h"
#include "PowerPCInstruction.h"
#include "PowerPCParser"

int main() {

    std::string sourceCode = R"(
        .text
        .global _start
    _start:
        add r3, r1, r2
        addi r4, r0, 0x100
    )";


    Lexer lexer(sourceCode);
    auto tokens = lexer.tokenize();


    PowerPCParser parser(tokens);
    auto instructions = parser.parse();


    for (const auto& instr : instructions) {
        std::cout << "Parsed instruction: " << instr.primary_mnemonic << std::endl;
        std::cout << "  " << instr.pseudocode << std::endl;
    }

    return 0;
}