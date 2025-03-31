
#ifndef PPCASM_POWERPCINSTRUCTION_H
#define PPCASM_POWERPCINSTRUCTION_H
#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include <iostream>


enum class ArchLevel {
    USIA,
    VEA,
    OEA
};


enum class PrivilegeLevel {
    User,
    Supervisor,
    Hypervisor
};


enum class InstructionForm {
    XO,
    D,
    DS,
    A,
    I,
    B,
    SC,
    M,
    MD,
    MDS,
    X
};


struct PowerPCInstruction {

    std::string name;
    std::string primary_mnemonic;


    struct SyntaxVariant {
        std::string mnemonic;
        std::string syntax;
        bool oe;
        bool rc;
    };
    std::vector<SyntaxVariant> syntax_variants;


    std::vector<std::string> power_mnemonics;


    struct Encoding {
        uint32_t base_opcode;

        struct Field {
            std::string name;
            uint8_t start_bit;
            uint8_t end_bit;
            uint32_t mask;

            Field(const std::string& n, uint8_t s, uint8_t e)
                    : name(n), start_bit(s), end_bit(e),
                      mask(((1 << (e - s + 1)) - 1) << (31 - e)) {}
        };

        std::vector<Field> fields;

        void addField(const std::string& name, uint8_t start, uint8_t end) {
            fields.emplace_back(name, start, end);
        }

        uint32_t getFullMask() const {
            uint32_t mask = 0;
            for (const auto& field : fields) {
                mask |= field.mask;
            }
            return mask;
        }
    };
    Encoding encoding;


    std::string pseudocode;
    std::string description;


    struct RegisterEffects {
        bool cr_lt;
        bool cr_gt;
        bool cr_eq;
        bool cr_so;
        bool xer_so;
        bool xer_ov;
        bool xer_ca;
    };
    RegisterEffects effects;


    ArchLevel arch_level;
    PrivilegeLevel privilege_level;
    bool is_optional;
    InstructionForm form;
};


PowerPCInstruction createADDInstruction() {
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


    add.effects = {
            true,
            true,
            true,
            true,
            true,
            true,
            false
    };


    add.arch_level = ArchLevel::USIA;
    add.privilege_level = PrivilegeLevel::User;
    add.is_optional = false;
    add.form = InstructionForm::XO;

    return add;
}


void printInstructionInfo(const PowerPCInstruction& instr) {
    std::cout << "Instruction Name: " << instr.name << "\n";
    std::cout << "Primary Mnemonic: " << instr.primary_mnemonic << "\n\n";

    std::cout << "Syntax Variants:\n";
    for (const auto& variant : instr.syntax_variants) {
        std::cout << "  " << variant.mnemonic << " " << variant.syntax
                  << " (OE=" << variant.oe << ", Rc=" << variant.rc << ")\n";
    }

    std::cout << "\nEquivalent POWER Mnemonics:\n";
    for (const auto& mnemonic : instr.power_mnemonics) {
        std::cout << "  " << mnemonic << "\n";
    }

    std::cout << "\nEncoding (Base Opcode: 0x" << std::hex << instr.encoding.base_opcode << "):\n";
    for (const auto& field : instr.encoding.fields) {
        std::cout << "  " << field.name << ": bits " << (int)field.start_bit
                  << "-" << (int)field.end_bit << " (mask: 0x"
                  << field.mask << ")\n";
    }

    std::cout << "\nPseudocode: " << instr.pseudocode << "\n";
    std::cout << "Description: " << instr.description << "\n";

    std::cout << "\nRegister Effects:\n";
    std::cout << "  CR: LT=" << instr.effects.cr_lt << " GT=" << instr.effects.cr_gt
              << " EQ=" << instr.effects.cr_eq << " SO=" << instr.effects.cr_so << "\n";
    std::cout << "  XER: SO=" << instr.effects.xer_so << " OV=" << instr.effects.xer_ov
              << " CA=" << instr.effects.xer_ca << "\n";

    std::cout << "\nClassification:\n";
    std::cout << "  Architecture Level: " << (instr.arch_level == ArchLevel::USIA ? "USIA" :
                                              instr.arch_level == ArchLevel::VEA ? "VEA" : "OEA") << "\n";
    std::cout << "  Privilege Level: " << (instr.privilege_level == PrivilegeLevel::User ? "User" :
                                           instr.privilege_level == PrivilegeLevel::Supervisor ? "Supervisor" : "Hypervisor") << "\n";
    std::cout << "  Optional: " << (instr.is_optional ? "Yes" : "No") << "\n";
    std::cout << "  Form: " << (instr.form == InstructionForm::XO ? "XO" :
                                instr.form == InstructionForm::D ? "D" :
                                instr.form == InstructionForm::DS ? "DS" : "...") << "\n";
}

int main() {
    auto add_instr = createADDInstruction();
    printInstructionInfo(add_instr);
    return 0;
}



#endif //PPCASM_POWERPCINSTRUCTION_H
