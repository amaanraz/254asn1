#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/* Unpacks the 32-bit machine code instruction given into the correct
 * type within the instruction struct */
Instruction parse_instruction(uint32_t instruction_bits) {
  /* YOUR CODE HERE */
  Instruction instruction;
  // add x9, x20, x21   hex: 01 5A 04 B3, binary = 0000 0001 0101 1010 0000 0100 1011 0011
  // Opcode: 0110011 (0x33) Get the Opcode by &ing 0x1111111, bottom 7 bits
  instruction.opcode = instruction_bits & ((1U << 7) - 1);

  // Shift right to move to pointer to interpret next fields in instruction.
  instruction_bits >>= 7;

  switch (instruction.opcode) {
  // R-Type
  case 0x33:
    // instruction: 0000 0001 0101 1010 0000 0100 1, destination : 01001
    instruction.rtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0001 0101 1010 0000, func3 : 000
    instruction.rtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // instruction: 0000 0001 0101 1010 0, src1: 10100
    instruction.rtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0001 0101, src2: 10101
    instruction.rtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // funct7: 0000 000
    instruction.rtype.funct7 = instruction_bits & ((1U << 7) - 1);
    break;
      
  // I-Type 
  case 0x13:
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.imm = instruction_bits & ((1U << 12) -1);

  break;
  
  // LOAD
  case 0x3:

    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.imm = instruction_bits & ((1U << 12) - 1);
    break;
  //SBtype
  case 0x63:
    // opcode, imm5, funct3, rs1, rs2, imm7

    instruction.sbtype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.sbtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.sbtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.sbtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.sbtype.imm7 = instruction_bits & ((1U << 7) - 1);
    break;

  // UJ type
  case 0x6f:

    // rd
    instruction.ujtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.ujtype.imm = instruction_bits & ((1U << 20) - 1);
    break;
  
  // I-Type-ecall
  case 0x73:
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.funct3 = instruction_bits &  ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.imm = instruction_bits & ((1U << 12) -1);

  break;
      
  // S-Type
  case 0x23:
    instruction.stype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.stype.funct3 = instruction_bits & ((1U << 3) -1);
    instruction_bits >>= 3;

    instruction.stype.rs1 = instruction_bits & ((1U << 5) -1);
    instruction_bits >>= 5;

    instruction.stype.rs2 = instruction_bits & ((1U << 5) -1);
    instruction_bits >>= 5;

    instruction.stype.imm7 = instruction_bits & ((1U << 7) - 1);
  break;
  
  // cases for other types of instructions
  /* YOUR CODE HERE */

  #ifndef TESTING
  default:
    exit(EXIT_FAILURE);
  #endif
  }
  return instruction;
}

/************************Helper functions************************/
/* Here, you will need to implement a few common helper functions, 
 * which you will call in other functions when parsing, printing, 
 * or executing the instructions. */

/* Sign extends the given field to a 32-bit integer where field is
 * interpreted an n-bit integer. */
int sign_extend_number(unsigned int field, unsigned int n) {
  /* YOUR CODE HERE */
  return 0;
}

/* Return the number of bytes (from the current PC) to the branch label using
 * the given branch instruction */
int get_branch_offset(Instruction instruction) {
  // imm[12|10:5] imm[4:1|11] 0 bit: 0
  // 1111 1110 1100 1101 1000 0000 1110 0011

  // opcode : 110 0011 -> 0x63
  // imm5 : 00001
  // imm7: 1111111

  // ex: offset = 1 1 11111 0000 0 -> signed : -32

  unsigned int imm5 = instruction.sbtype.imm5;
  unsigned int imm7 = instruction.sbtype.imm7;

  // 1:4 00000
  int var1 = imm5 & 0x1E;

  // 5:10 00011111
  int var2 = imm7 & 0x1F;

  // 11 00001
  int var3 = imm5 & 0x1;

  // 12 01000000
  int var4 = imm7 & 0x40;
  int offset = (var4 << 5) | (var3 << 10) | (var2 << 5) | var1;
  // check msb for sign
  int sign_bit = offset >> 11;
  if(sign_bit){
    int all = 0xFFF;
    // return signed representation
    offset = offset - all - 1;
  }
  return offset;
}

/* Returns the number of bytes (from the current PC) to the jump label using the
 * given jump instruction */
int get_jump_offset(Instruction instruction) {
  // restucture the imm value; imm[20|10:1|11|19:12] 20 bits long

  int imm = instruction.ujtype.imm;
  // 19:12
  int var1 = imm & 0xFF;
  // 11
  int var2 = (imm >> 8) & 0x1;
  //10:1 
  int var3 = ((imm >> 9) & 0x3FF) << 1;
  // 20
  int var4 = (imm >> 19) & 0x1;

  int offset = (var4 << 19) | (var1 << 11) | (var2 << 10) | var3;

  // check msb for sign
  int sign_bit = offset >> 19;
  if(sign_bit){
    int all = 0xFFFFF;
    // return signed representation
    offset = offset - all - 1;
  }

  return offset;
}

/* Returns the number of bytes (from the current PC) to the base address using the
 * given store instruction */
int get_store_offset(Instruction instruction) {
  /* YOUR CODE HERE */
  return 0;
}
/************************Helper functions************************/

void handle_invalid_instruction(Instruction instruction) {
  printf("Invalid Instruction: 0x%08x\n", instruction.bits);
}

void handle_invalid_read(Address address) {
  printf("Bad Read. Address: 0x%08x\n", address);
  exit(-1);
}

void handle_invalid_write(Address address) {
  printf("Bad Write. Address: 0x%08x\n", address);
  exit(-1);
}
