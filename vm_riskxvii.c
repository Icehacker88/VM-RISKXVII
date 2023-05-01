#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>

#define INST_MEM_SIZE 256
#define DATA_MEM_SIZE 1024
#define DATA_MEM_SIZE_INDEX 2048
#define REG_SIZE 32
#define HEAP_BANK_SIZE 128
#define HEAP_BANKS_ADDRESS 0xd700

int32_t pc = 0; // program counter
//struct blob {
//
//};
int32_t inst_mem[INST_MEM_SIZE]; // instruction memory
uint8_t data_mem[DATA_MEM_SIZE]; // data memory
uint32_t registers[REG_SIZE]; // register
int64_t heap_bank[HEAP_BANK_SIZE]; // heap bank

int32_t opcode_mask = 0x7f;
int32_t rd_mask = 0xf80;
int32_t func3_mask = 0x7000;
int32_t rs1_mask = 0xf8000;
int32_t rs2_mask = 0x1f00000;
int32_t func7_mask = 0xfe000000;
int32_t imm_TypeI_mask = 0xfff00000;
int32_t imm_TypeS_mask1 = 0xf80;
int32_t imm_TypeS_mask2 = 0xfe000000;
int32_t imm_TypeSB_mask1 = 0xf80;
int32_t imm_TypeSB_mask2 = 0xfe000000;
int32_t imm_TypeSB_mask1_mask = 0xf00; // 4:1
int32_t imm_TypeSB_mask2_mask1 = 0x7e000000; // 10:5
int32_t imm_TypeSB_mask2_mask2 = 0x80; // 11
int32_t imm_TypeSB_mask2_mask3 = 0x80000000; // 12
int32_t imm_TypeU_mask = 0xfffff000;
int32_t imm_TypeUJ_mask = 0xfffff000;
int32_t imm_TypeUJ_mask_mask1 = 0xff000; // 19:12
int32_t imm_TypeUJ_mask_mask2 = 0x100000; // 11
int32_t imm_TypeUJ_mask_mask3 = 0x7fe00000; // 10:1
int32_t imm_TypeUJ_mask_mask4 = 0x80000000; // 20

// Execute instructions
//void execute_instruction(int32_t instruction);

// declared check virtual routines function
int check_virtual_routines(int32_t rs1, int32_t imm);

// declared choose virtual routines
void choose_virtual_routines(int32_t rd, int32_t address);

// declared register dump
void register_dump();

// declared Not Implemented
void not_implemented(int32_t command);

// declared Illegal Operation
void illegal_operation(int32_t address);

// Register Dump
void register_dump() {
    printf("PC = 0x%x;\n", pc);
    for (int i = 0; i < 32; i++) {
        printf("R[%d] = 0x%x;\n", i, registers[i]);
    }
    exit(0);
}

// Not Implemented
void not_implemented(int32_t command) {
    printf("Instruction Not Implemented: 0x%x\n", command);
    register_dump();
}

// Illegal Operation
void illegal_operation(int32_t address) {
    printf("Illegal Operation: 0x%x\n", address);
    register_dump();
}

// Arithmetic and Logic Operations
// 1 add
void add(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = registers[rs1] + registers[rs2];
}

// 2 addi
void addi(int32_t rd, int32_t rs1, int32_t imm){
    registers[rd] = registers[rs1] + imm;
}

// 3 sub
void sub(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = registers[rs1] - registers[rs2];
}

// 4 lui
void lui(int32_t rd, int32_t imm){
    registers[rd] = imm << 12;
}

// 5 xor
void xor(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = registers[rs1] ^ registers[rs2];
}

// 6 xori
void xori(int32_t rd, int32_t rs1, int32_t imm) {
    registers[rd] = registers[rs1] ^ imm;
}

// 7 or
void or(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = registers[rs1] | registers[rs2];
}

// 8 ori
void ori(int32_t rd, int32_t rs1, int32_t imm) {
    registers[rd] = registers[rs1] | imm;
}

// 9 and
void and(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = registers[rs1] & registers[rs2];
}

// 10 andi
void andi(int32_t rd, int32_t rs1, int32_t imm) {
    registers[rd] = registers[rs1] & imm;
}

// 11 sll
void sll(int32_t rd, int32_t rs1, int32_t rs2) {
//    if (inst_mem[rs2] < 32){
//        registers[rd] = registers[rs1] << registers[rs2];
//    } else {
//        illegal_operation(inst_mem[pc / 4]);
//    }
    registers[rd] = registers[rs1] << registers[rs2];

}

// 12 srl
void srl(int32_t rd, int32_t rs1, int32_t rs2) {
//    if (inst_mem[rs2] < 32){
//        registers[rd] = registers[rs1] >> registers[rs2];
//    } else {
//        illegal_operation(inst_mem[pc / 4]);
//    }
    registers[rd] = registers[rs1] >> registers[rs2];

}

// 13 sra ???
void sra(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = registers[rs1] >> registers[rs2] | registers[rs1] << (REG_SIZE - registers[rs2]);
}

// Memory Access Operations
// 14 lb ???
void lb(int32_t rd, int32_t rs1, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, imm);
    int32_t address = registers[rs1] + imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) { // 1024
            int check = address % 4;
            if (check == 3) {
                registers[rd] = (int8_t) (inst_mem[address / 4] >> 24);
            } else if (check == 2) {
                registers[rd] = (int8_t) (inst_mem[address / 4] >> 16);
            } else if (check == 1) {
                registers[rd] = (int8_t) (inst_mem[address / 4] >> 8);
            } else if (check == 0) {
                registers[rd] = (int8_t) inst_mem[address / 4];
            }
        } else if (cvr == 1) { // 2048
            registers[rd] = (int32_t) data_mem[address - DATA_MEM_SIZE];
        } else if (cvr == 2) { // virtual routines
            choose_virtual_routines(rd, address);
        }
    }
}

// 15 lh
void lh(int32_t rd, int32_t rs1, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, imm);
    int32_t address = registers[rs1] + imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) { // 1024
//            registers[rd] = (int32_t) (inst_mem[address] >> 16);

            int8_t first_part;
            int8_t second_part;
            int16_t final_result;
//            registers[rd] = (uint32_t) (inst_mem[address] >> 16);
            int check = address % 4;
            if (check == 3) {
                first_part = (int8_t) (inst_mem[address / 4] >> 24);
            } else if (check == 2) {
                first_part = (int8_t) (inst_mem[address / 4] >> 16);
            } else if (check == 1) {
                first_part = (int8_t) (inst_mem[address / 4] >> 8);
            } else if (check == 0) {
                first_part = (int8_t) inst_mem[address / 4];
            }
            int check1 = (address + 1) % 4;
            if (check1 == 3) {
                second_part = (int16_t) (inst_mem[(address + 1) / 4] >> 24);
            } else if (check1 == 2) {
                second_part = (int16_t) (inst_mem[(address + 1) / 4] >> 16);
            } else if (check1 == 1) {
                second_part = (int16_t) (inst_mem[(address + 1) / 4] >> 8);
            } else if (check1 == 0) {
                second_part = (int16_t) inst_mem[(address + 1) / 4];
            }
            final_result = (((int16_t) (first_part)) << 8 | ((int16_t) second_part));
            registers[rd] = final_result;
        } else if (cvr == 1) { // 2048
            registers[rd] = (((int32_t) data_mem[address - DATA_MEM_SIZE]) << 8 | ((int32_t) data_mem[address + 1]));
        } else if (cvr == 2) { // virtual routines
            choose_virtual_routines(rd, address);
        }
    }
}

// 16 lw
void lw(int32_t rd, int32_t rs1, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, imm);
    int32_t address = registers[rs1] + imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) {
            registers[rd] = inst_mem[address / 4];
        } else if (cvr == 1) {
            registers[rd] = (((int32_t) data_mem[address - DATA_MEM_SIZE]) << 24 | ((int32_t) data_mem[address - DATA_MEM_SIZE + 1]) << 16 | ((int32_t) data_mem[address - DATA_MEM_SIZE + 2]) << 8 | ((int32_t) data_mem[address - DATA_MEM_SIZE + 3]));
        } else if (cvr == 2) {
            choose_virtual_routines(rd, address);
        }
    }
}

// 17 lbu
void lbu(int32_t rd, int32_t rs1, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, (uint32_t)imm);
    uint32_t address = registers[rs1] + (uint32_t)imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) { // 1024
            int check = address % 4;
            if (check == 3) {
                registers[rd] = (int8_t) (inst_mem[address / 4] >> 24);
            } else if (check == 2) {
                registers[rd] = (int8_t) (inst_mem[address / 4] >> 16);
            } else if (check == 1) {
                registers[rd] = (int8_t) (inst_mem[address / 4] >> 8);
            } else if (check == 0) {
                registers[rd] = (int8_t) inst_mem[address / 4];
            }
        } else if (cvr == 1) { // 2048
            registers[rd] = data_mem[address - DATA_MEM_SIZE];
        } else if (cvr == 2) { // virtual routines
            choose_virtual_routines(rd, address);
        }
    }
}

// 18 lhu
void lhu(int32_t rd, int32_t rs1, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, (uint32_t)imm);
    int32_t address = registers[rs1] + (uint32_t)imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) { // 1024
            int8_t first_part;
            int8_t second_part;
            int16_t final_result;
//            registers[rd] = (uint32_t) (inst_mem[address] >> 16);
            int check = address % 4;
            if (check == 3) {
                first_part = (int8_t) (inst_mem[address / 4] >> 24);
            } else if (check == 2) {
                first_part = (int8_t) (inst_mem[address / 4] >> 16);
            } else if (check == 1) {
                first_part = (int8_t) (inst_mem[address / 4] >> 8);
            } else if (check == 0) {
                first_part = (int8_t) inst_mem[address / 4];
            }
            int check1 = (address + 1) % 4;
            if (check1 == 3) {
                second_part = (int16_t) (inst_mem[(address + 1) / 4] >> 24);
            } else if (check1 == 2) {
                second_part = (int16_t) (inst_mem[(address + 1) / 4] >> 16);
            } else if (check1 == 1) {
                second_part = (int16_t) (inst_mem[(address + 1) / 4] >> 8);
            } else if (check1 == 0) {
                second_part = (int16_t) inst_mem[(address + 1) / 4];
            }
            final_result = (((int16_t) (first_part)) << 8 | ((int16_t) second_part));
            registers[rd] = final_result;
        } else if (cvr == 1) { // 2048
            registers[rd] = (((uint32_t) data_mem[address - DATA_MEM_SIZE]) << 8 | ((uint32_t) data_mem[address - DATA_MEM_SIZE + 1]));
        } else if (cvr == 2) { // virtual routines
            choose_virtual_routines(rd, address);
        }
    }
}

// 19 sb
void sb(int32_t rs1, int32_t rs2, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, imm);
    int32_t address = registers[rs1] + imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) { // 1024
            illegal_operation(inst_mem[pc / 4]);
        } else if (cvr == 1) { // 2048
            data_mem[address - DATA_MEM_SIZE] = (int8_t) (registers[rs2] >> 24);
        } else if (cvr == 2) { // virtual routines
            choose_virtual_routines(rs2, address);
        }
    }
}

// 20 sh
void sh(int32_t rs1, int32_t rs2, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, imm);
    int32_t address = registers[rs1] + imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) { // 1024
            illegal_operation(inst_mem[pc / 4]);
        } else if (cvr == 1) { // 2048
            data_mem[address - DATA_MEM_SIZE] = (int8_t) (registers[rs2] >> 24);
            data_mem[address - DATA_MEM_SIZE + 1] = (int8_t) (registers[rs2] >> 16);
        } else if (cvr == 2) { // virtual routines
            choose_virtual_routines(rs2, address);
        }
    }
}

// 21 sw
void sw(int32_t rs1, int32_t rs2, int32_t imm) {
    int32_t cvr = check_virtual_routines(rs1, imm);
    int32_t address = registers[rs1] + imm;
    // Error Handling
    if (address > HEAP_BANKS_ADDRESS || address < 0) {
        illegal_operation(inst_mem[pc / 4]);
    } else {
        if (cvr == 0) { // 1024
            illegal_operation(inst_mem[pc / 4]);
        } else if (cvr == 1) { // 2048
            data_mem[address - DATA_MEM_SIZE] = (int8_t) (registers[rs2] >> 24);
            data_mem[address - DATA_MEM_SIZE + 1] = (int8_t) (registers[rs2] >> 16);
            data_mem[address - DATA_MEM_SIZE + 2] = (int8_t) (registers[rs2] >> 8);
            data_mem[address - DATA_MEM_SIZE + 3] = (int8_t) (registers[rs2]);
        } else if (cvr == 2) { // virtual routines
            choose_virtual_routines(rs2, address);
        }
    }
}

// Program Flow Operations
// 22 slt
void slt(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = (registers[rs1] < registers[rs2]) ? 1 : 0;
}

// 23 slti
void slti(int32_t rd, int32_t rs1, int32_t imm) {
    registers[rd] = (registers[rs1] < imm) ? 1 : 0;
}

// 24 sltu
void sltu(int32_t rd, int32_t rs1, int32_t rs2) {
    registers[rd] = (registers[rs1] < registers[rs2]) ? 1 : 0;
}

// 25 sltiu
void sltiu(int32_t rd, int32_t rs1, int32_t imm) {
    registers[rd] = (registers[rs1] < imm) ? 1 : 0;
}

// 26 beq
void beq(int32_t rs1, int32_t rs2, int32_t imm) {
    if (registers[rs1] == registers[rs2]) {
        pc = pc + (imm << 1);
        pc -= 4;
    }
}

// 27 bne
void bne(int32_t rs1, int32_t rs2, int32_t imm) {
    if (registers[rs1] != registers[rs2]) {
        pc = pc + (imm << 1);
        pc -= 4;
    }
}

// 28 blt
void blt(int32_t rs1, int32_t rs2, int32_t imm) {
    if (registers[rs1] < registers[rs2]) {
        pc = pc + (imm << 1);
        pc -= 4;
    }
}

// 29 bltu
void bltu(int32_t rs1, int32_t rs2, int32_t imm) {
    if (registers[rs1] < registers[rs2]) {
        pc = pc + ((int32_t)imm << 1);
        pc -= 4;
    }
}

// 30 bge
void bge(int32_t rs1, int32_t rs2, int32_t imm) {

    if (registers[rs1] >= registers[rs2]) {
        pc = pc + (imm << 1);
        pc -= 4;
    }
}

// 31 bgeu
void bgeu(int32_t rs1, int32_t rs2, int32_t imm) {
    if (registers[rs1] < registers[rs2]) {
        pc = pc + ((uint32_t)imm << 1);
        pc -= 4;
    }
}

// 32 jal
void jal(int32_t rd, int32_t imm) {
    registers[rd] = pc + 4;
    pc = pc + (imm << 1);
    pc -= 4;
}

// 33 jalr
void jalr(int32_t rd, int32_t rs1, int32_t imm) {
    registers[rd] = pc + 4;
    pc = registers[rs1] + imm;
    pc -= 4;
}

// Virtual Routines
// 0x0800 - Console Write Character
void cwc(int32_t value) {
    printf("%c", registers[value]);
}

// 0x0804 - Console Write Signed Integer
void cwsi(int32_t value) {
    printf("%d", registers[value]);
}

// 0x0808 - Console Write Unsigned Integer
void cwui(int32_t value) {
    printf("%x", (int32_t) registers[value]); // %x: output hexadecimal
}

// 0x080C - Halt
void halt() {
    printf("CPU Halt Requested\n");
    exit(1);
}

// 0x0812 - Console Read Character
void crc(int32_t value) {
    char c;
    scanf("%c", &c);
    registers[value] = (int32_t) c;
}

// 0x0816 - Console Read Signed Integer
void crsi(int32_t value) {
    int c;
    scanf("%d", &c);
    registers[value] = (int32_t) c;
}

// 0x0820 - Dump PC
void DPc() {
    printf("%x", pc);
}

// 0x0824 - Dump Register Banks
void drb() {
    register_dump();
}

// 0x0828 - Dump Memory Word
void dmw(uint32_t value) {
    printf("%x", (((int32_t) data_mem[registers[value]]) << 24 | ((int32_t) data_mem[registers[value] + 1]) << 16 | ((int32_t) data_mem[registers[value] + 2]) << 8 | ((int32_t) data_mem[registers[value] + 3])));
}

// 0x0830, 0x0834 - Heap Banks
void heap_banks() {

}

// check if it needs virtual routines
int check_virtual_routines(int32_t rs1, int32_t imm) {
    if ((registers[rs1] + imm) < 0x400) {
        return 0;
    } else if ((registers[rs1] + imm) >= 0x400 && (registers[rs1] + imm) < 0x800) {
        return 1;
    } else if ((registers[rs1] + imm) >= 0x800 && (registers[rs1] + imm) < 0x8ff) {
        return 2;
    }
    return 3;
}

// choose virtual routines
void choose_virtual_routines(int32_t rd, int32_t address) {
    if (address == 0x0800) {
        cwc(rd);
    } else if (address == 0x0804) {
        cwsi(rd);
    } else if (address == 0x0808) {
        cwui(rd);
    } else if (address == 0x080C) {
        halt();
    } else if (address == 0x0812) {
        crc(rd);
    } else if (address == 0x0816) {
        crsi(rd);
    } else if (address == 0x0820) {
        DPc();
    } else if (address == 0x0824) {
        drb();
    } else if (address == 0x0828) {
        dmw(rd);
    } else if (address == 0x0830 || address == 0x0834) {
        heap_banks();
    }
}

// change 8 bits to 32 bits
//uint32_t change8to32(int8_t value) {
//    uint32_t extend_mask_8 = 0xffffff00;
//    if (value < 0) {
//        return (extend_mask_8 | (int32_t) value);
//    } else {
//        return (uint32_t) value;
//    }
//}

// change 16 bits to 32 bits
//uint32_t change16to32(int16_t value) {
//    uint16_t change_mask = 0x8000;
//    uint32_t extend_mask_16 = 0xffff0000;
//    if ((change_mask & value) == 0x80) {
//        return (extend_mask_16 | value);
//    } else {
//        return (uint32_t) value;
//    }
//}

// split and execute instructions
void execute_instruction(int32_t instruction) {
    // split instructions
    int32_t opcode = (instruction & opcode_mask);
    int32_t rd = ((instruction & rd_mask) >> 7);
    int32_t func3 = ((instruction & func3_mask) >> 12);
    int32_t rs1 = ((instruction & rs1_mask) >> 15);
    int32_t rs2 = ((instruction & rs2_mask) >> 20);
    int32_t func7 = ((instruction & func7_mask) >> 25);
    int32_t imm_TypeI = ((instruction & imm_TypeI_mask) >> 20);
    int32_t imm_TypeS = (((instruction & imm_TypeS_mask1) >> 7) | ((instruction & imm_TypeS_mask2) >> 20));
    int32_t imm_TypeSB = ((((instruction & imm_TypeSB_mask1) & imm_TypeSB_mask1_mask) >> 8) | (((instruction & imm_TypeSB_mask2) & imm_TypeSB_mask2_mask1) >> 21) |
            (((instruction & imm_TypeSB_mask2) & imm_TypeSB_mask2_mask2) << 4) | (((instruction & imm_TypeSB_mask2) & imm_TypeSB_mask2_mask3) >> 21));
    int32_t imm_TypeU = ((instruction & imm_TypeU_mask) >> 12);
    int32_t imm_TypeUJ = ((((instruction & imm_TypeUJ_mask) & imm_TypeUJ_mask_mask1) >> 1) | (((instruction & imm_TypeUJ_mask) & imm_TypeUJ_mask_mask2) >> 10) |
            (((instruction & imm_TypeUJ_mask) & imm_TypeUJ_mask_mask3) >> 21) | (((instruction & imm_TypeUJ_mask) & imm_TypeUJ_mask_mask4) >> 12));

    // execute instructions
    if (opcode == 0x33) { // Type R
        if (func3 == 0x0 && func7 == 0x0) {
            add(rd, rs1, rs2);
        } else if (func3 == 0x0 && func7 == 0x20) {
            sub(rd, rs1, rs2);
        } else if (func3 == 0x1 && func7 == 0x0) {
            sll(rd, rs1, rs2);
        } else if (func3 == 0x2 && func7 == 0x0) {
            slt(rd, (int32_t) rs1, (int32_t) rs2);
        } else if (func3 == 0x3 && func7 == 0x0) {
            sltu(rd, rs1, rs2);
        } else if (func3 == 0x4 && func7 == 0x0) {
            xor(rd, rs1, rs2);
        } else if (func3 == 0x5 && func7 == 0x0) {
            srl(rd, rs1, rs2);
        } else if (func3 == 0x5 && func7 == 0x20) {
            sra(rd, rs1, rs2);
        } else if (func3 == 0x6 && func7 == 0x0) {
            or(rd, rs1, rs2);
        } else if (func3 == 0x7 && func7 == 0x0) {
            and(rd, rs1, rs2);
        } else {
            not_implemented(instruction);
        }
    } else if (opcode == 0x3 || opcode == 0x13 || opcode == 0x67) { // Type I
        if (opcode == 0x3) {
            if (func3 == 0x0) {
                lb((int32_t) rd, (int32_t) rs1, (int32_t) imm_TypeI);
            } else if (func3 == 0x1) {
                lh((int32_t) rd, (int32_t) rs1, (int32_t) imm_TypeI);
            } else if (func3 == 0x2) {
                lw((int32_t) rd, (int32_t) rs1, (int32_t) imm_TypeI);
            } else if (func3 == 0x4){
                lbu(rd, rs1, imm_TypeI);
            } else if (func3 == 0x5){
                lhu(rd, rs1, imm_TypeI);
            } else {
                not_implemented(instruction);
            }
        } else if (opcode == 0x13) {
            if (func3 == 0x0) {
                addi(rd, rs1, imm_TypeI);
            } else if (func3 == 0x2){
                slti(rd, (int32_t) rs1, (int32_t) imm_TypeI);
            } else if (func3 == 0x3){
                sltiu(rd, rs1, imm_TypeI);
            } else if (func3 == 0x4){
                xori(rd, rs1, imm_TypeI);
            } else if (func3 == 0x6){
                ori(rd, rs1, imm_TypeI);
            } else if (func3 == 0x7){
                andi(rd, rs1, imm_TypeI);
            } else {
                not_implemented(instruction);
            }
        } else {
            jalr((int32_t) rd, (int32_t) rs1, (int32_t) imm_TypeI);
        }
    } else if (opcode == 0x23) { // Type S
        if (func3 == 0x0) {
            sb(rs1, rs2, imm_TypeS);
        } else if (func3 == 0x1) {
            sh(rs1, rs2, imm_TypeS);
        } else if (func3 == 0x2) {
            sw(rs1, rs2, imm_TypeS);
        } else {
            not_implemented(instruction);
        }
    } else if (opcode == 0x63) { // Type SB
        if (func3 == 0x0) {
            beq((int32_t) rs1, (int32_t) rs2, (int32_t) imm_TypeSB);
        } else if (func3 == 0x1) {
            bne((int32_t) rs1, (int32_t) rs2, (int32_t) imm_TypeSB);
        } else if (func3 == 0x4) {
            blt((int32_t) rs1, (int32_t) rs2, (int32_t) imm_TypeSB);
        } else if (func3 == 0x5) {
            bge((int32_t) rs1, (int32_t) rs2, (int32_t) imm_TypeSB);
        } else if (func3 == 0x6) {
            bltu(rs1, rs2, imm_TypeSB);
        } else if (func3 == 0x7) {
            bgeu(rs1, rs2, imm_TypeSB);
        } else {
            not_implemented(instruction);
        }
    } else if (opcode == 0x37) { // Type U
        lui(rd, imm_TypeU);
    } else if (opcode == 0x6F) { // Type UJ
        jal((int32_t) rd, (int32_t) imm_TypeUJ);
    } else {
        not_implemented(instruction);
    }
    pc += 4;
}

int main(int argc, char *argv[]) {

    // read binary files
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        return 1;
    }

    int n = 0;
    int m = 1024;

    while(1) {
        if (n >= INST_MEM_SIZE) {
            break;
        }
        fread(&inst_mem[n], sizeof(int32_t), 1, file);
        n += 1;

    }

    while(1) {
        if (m >= DATA_MEM_SIZE_INDEX) {
            break;
        }
        fread(&data_mem[m - DATA_MEM_SIZE], sizeof(int8_t), 1, file);
        m += 1;

    }

    while((pc >= 0) && (pc < DATA_MEM_SIZE)) {
//        printf("-0x%x-",inst_mem[pc/4]);
//        printf("%d, r2  %d\n", pc, registers[2]);
//        printf("pp%x\n", inst_mem[pc / 4]);
//        if(registers[1] == 96){
//
//            exit(1);
//        }
        execute_instruction(inst_mem[pc / 4]);
        registers[0] = 0;
    }
//    free(buffer);
    fclose(file);
    return 0;
}