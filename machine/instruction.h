/*! \file  instruction.h
    \brief composition of instructioninterface of instruction class

 DO NOT CHANGE -- part of the machine emulation

 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------

*/

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <sstream>
#include <string>
#include <strings.h>

#define RISCV_LUI   0x37
#define RISCV_AUIPC 0x17
#define RISCV_JAL   0x6f
#define RISCV_JALR  0x67
#define RISCV_BR    0x63
#define RISCV_LD    0x3
#define RISCV_ST    0x23
#define RISCV_OPI   0x13
#define RISCV_OP    0x33
#define RISCV_OPIW  0x1b
#define RISCV_OPW   0x3b

#define RISCV_BR_BEQ  0x0
#define RISCV_BR_BNE  0x1
#define RISCV_BR_BLT  0x4
#define RISCV_BR_BGE  0x5
#define RISCV_BR_BLTU 0x6
#define RISCV_BR_BGEU 0x7

#define RISCV_LD_LB  0x0
#define RISCV_LD_LH  0x1
#define RISCV_LD_LW  0x2
#define RISCV_LD_LD  0x3
#define RISCV_LD_LBU 0x4
#define RISCV_LD_LHU 0x5
#define RISCV_LD_LWU 0x6

#define RISCV_ST_STB 0x0
#define RISCV_ST_STH 0x1
#define RISCV_ST_STW 0x2
#define RISCV_ST_STD 0x3

#define RISCV_OPI_ADDI  0x0
#define RISCV_OPI_SLTI  0x2
#define RISCV_OPI_SLTIU 0x3
#define RISCV_OPI_XORI  0x4
#define RISCV_OPI_ORI   0x6
#define RISCV_OPI_ANDI  0x7
#define RISCV_OPI_SLLI  0x1
#define RISCV_OPI_SRI   0x5

#define RISCV_OPI_SRI_SRAI 0x20
#define RISCV_OPI_SRI_SRLI 0x0

#define RISCV_OP_ADD  0x0
#define RISCV_OP_SLL  0x1
#define RISCV_OP_SLT  0x2
#define RISCV_OP_SLTU 0x3
#define RISCV_OP_XOR  0x4
#define RISCV_OP_SR   0x5
#define RISCV_OP_OR   0x6
#define RISCV_OP_AND  0x7

#define RISCV_OP_ADD_ADD 0x0
#define RISCV_OP_ADD_SUB 0x20

#define RISCV_OP_SR_SRL 0x0
#define RISCV_OP_SR_SRA 0x20

#define RISCV_SYSTEM 0x73

#define RISCV_OPIW_ADDIW 0x0
#define RISCV_OPIW_SLLIW 0x1
#define RISCV_OPIW_SRW   0x5

#define RISCV_OPIW_SRW_SRLIW 0x0
#define RISCV_OPIW_SRW_SRAIW 0x20

#define RISCV_OPW_ADDSUBW 0x0
#define RISCV_OPW_SLLW    0x1
#define RISCV_OPW_SRW     0x5

#define RISCV_OPW_ADDSUBW_ADDW 0x0
#define RISCV_OPW_ADDSUBW_SUBW 0x20

#define RISCV_OPW_SRW_SRLW 0x0
#define RISCV_OPW_SRW_SRAW 0x20

#define RISCV_SYSTEM_ENV        0x0
#define RISCV_SYSTEM_ENV_ECALL  0x0
#define RISCV_SYSTEM_ENV_EBREAK 0x1
#define RISCV_SYSTEM_CSRRS      0x2

#define RISCV_SYSTEM_CSRRW  0x1
#define RISCV_SYSTEM_CSRRS  0x2
#define RISCV_SYSTEM_CSRRC  0x3
#define RISCV_SYSTEM_CSRRWI 0x5
#define RISCV_SYSTEM_CSRRSI 0x6
#define RISCV_SYSTEM_CSRRCI 0x7

#define RISCV_FLW    0x07
#define RISCV_FSW    0x27
#define RISCV_FMADD  0x43
#define RISCV_FMSUB  0x47
#define RISCV_FNMSUB 0x4b
#define RISCV_FNMADD 0x4f
#define RISCV_FP     0x53

#define RISCV_FP_ADD        0x0
#define RISCV_FP_SUB        0x4
#define RISCV_FP_MUL        0x8
#define RISCV_FP_DIV        0xc
#define RISCV_FP_SQRT       0x2c
#define RISCV_FP_FSGN       0x10
#define RISCV_FP_MINMAX     0x14
#define RISCV_FP_FCVTW      0x60
#define RISCV_FP_FMVXFCLASS 0x70
#define RISCV_FP_FCMP       0x50
#define RISCV_FP_FEQS       0x53
#define RISCV_FP_FCVTS      0x68
#define RISCV_FP_FCVTDS     0x21

#define RISCV_FP_FSGN_J  0x0
#define RISCV_FP_FSGN_JN 0x1
#define RISCV_FP_FSGN_JX 0x2

#define RISCV_FP_MINMAX_MIN 0x0
#define RISCV_FP_MINMAX_MAX 0x1

#define RISCV_FP_FCVTW_W  0x0
#define RISCV_FP_FCVTW_WU 0x1

#define RISCV_FP_FCVTS_W  0x0
#define RISCV_FP_FCVTS_WU 0x1

#define RISCV_FP_FMVXFCLASS_FMVX   0x0
#define RISCV_FP_FMVXFCLASS_FCLASS 0x1

#define RISCV_FP_FCMP_FEQ 2
#define RISCV_FP_FCMP_FLT 1
#define RISCV_FP_FCMP_FLE 0

#define RISCV_FP_FMVW 0x78

/* New FP instructions introduced by Isabelle */
// #define RISCV_FP_FMVXD 0x71
// #define RISCV_FP_FMVDX 0x79

// FIXME some special operations of the base instruction set are not yet
// supported. (FENCE)

/******************************************************************************
 * Specification of the standard M extension
 ********************************************
 * This extension brings the support for multiplication operation.
 * It is composed of the RISCV_OP opcode then a dedicated value for funct7 which
 *identify it. Then funct3 is used to determine which of the eight operation to
 *use. Added operations are MUL, MULH, MULHSU, MLHU, DIV, DIVU, REM, REMU
 ******************************************************************************/

#define RISCV_OP_M 0x1

#define RISCV_OP_M_MUL    0x0
#define RISCV_OP_M_MULH   0x1
#define RISCV_OP_M_MULHSU 0x2
#define RISCV_OP_M_MULHU  0x3
#define RISCV_OP_M_DIV    0x4
#define RISCV_OP_M_DIVU   0x5
#define RISCV_OP_M_REM    0x6
#define RISCV_OP_M_REMU   0x7

#define RISCV_OPW_M_MULW  0x0
#define RISCV_OPW_M_DIVW  0x4
#define RISCV_OPW_M_DIVUW 0x5
#define RISCV_OPW_M_REMW  0x6
#define RISCV_OPW_M_REMUW 0x7

#define RISCV_FENCE 0x0f

#define RISCV_ATOM      0x2f
#define RISCV_ATOM_LR   0x2
#define RISCV_ATOM_SC   0x3
#define RISCV_ATOM_SWAP 0x1
#define RISCV_ATOM_ADD  0
#define RISCV_ATOM_XOR  0x4
#define RISCV_ATOM_AND  0xc
#define RISCV_ATOM_OR   0x8
#define RISCV_ATOM_MIN  0x10
#define RISCV_ATOM_MAX  0x14
#define RISCV_ATOM_MINU 0x18
#define RISCV_ATOM_MAXU 0x1c

/*! \brief  Defines an instruction
//
//  Represented in both
// 	undecoded binary form
//      decoded to identify
//	    - operation to do
//	    - registers to act on
//	    - any immediate operand value
*/
class Instruction {
public:
  // FIXME: why 64 bits here?
  uint64_t value;   //!< Binary representation of the instruction
  uint8_t opcode, rs1, rs2, rs3, rd, funct7, funct7_smaller, funct3, shamt;
  unsigned short imm12_I, imm12_S;
  short imm12_I_signed, imm12_S_signed, imm13, imm13_signed;
  uint32_t imm31_12, imm21_1;
  int32_t imm31_12_signed, imm21_1_signed;
  Instruction();
  Instruction(uint64_t val);

  void Decode();   //!< Decode the binary representation of the instruction

  std::string printDecodedInstrRISCV(uint64_t pc);
};

#endif   // INSTRUCTION_H
