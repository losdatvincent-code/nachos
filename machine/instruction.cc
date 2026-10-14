/*! \file  instruction.cc
    \brief Instruction class, with method for decoding and printing

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

#include "instruction.h"

// Instruction names (for debug)
const char *riscvNamesOP[8] = {"add", "sll", "cmplt", "cmpltu",
                               "xor", "",    "or",    "and"};
const char *riscvNamesOPI[8] = {"addi", "slli", "slti", "cmpltuii",
                                "xori", "slri", "ori",  "andi"};
const char *riscvNamesOPW[8] = {"addw", "sllw", "", "", "", "srw", "", ""};
const char *riscvNamesOPIW[8] = {"addwi", "sllwi", "", "", "", "sri", "", ""};
const char *riscvNamesLD[8] = {"lb", "lh", "lw", "ld", "lbu", "lhu", "lwu"};
const char *riscvNamesST[8] = {"sb", "sh", "sw", "sd"};
const char *riscvNamesBR[8] = {"beq", "bne", "",     "",
                               "blt", "bge", "bltu", "gbeu"};
const char *riscvNamesMUL[8] = {"mpylo", "mpyhi", "mpyhi", "mpyhi",
                                "divhi", "divhi", "divlo", "divlo"};

Instruction::Instruction() {}

Instruction::Instruction(uint64_t val) { this->value = val; }
//----------------------------------------------------------------------

// Instruction::Decode

//! 	Decode a RISCV instruction

//----------------------------------------------------------------------

void

Instruction::Decode()

{
  opcode = value & 0x7f;
  rs1 = ((value >> 15) & 0x1f);
  rs2 = ((value >> 20) & 0x1f);
  rs3 = ((value >> 27) & 0x1f);
  rd = ((value >> 7) & 0x1f);
  funct7 = ((value >> 25) & 0x7f);
  funct7_smaller = funct7 & 0x3e;

  funct3 = ((value >> 12) & 0x7);
  imm12_I = ((value >> 20) & 0xfff);
  imm12_S = ((value >> 20) & 0xfe0) + ((value >> 7) & 0x1f);

  imm12_I_signed = (imm12_I >= 2048) ? imm12_I - 4096 : imm12_I;
  imm12_S_signed = (imm12_S >= 2048) ? imm12_S - 4096 : imm12_S;

  imm13 = ((value >> 19) & 0x1000) + ((value >> 20) & 0x7e0) +
          ((value >> 7) & 0x1e) + ((value << 4) & 0x800);
  imm13_signed = (imm13 >= 4096) ? imm13 - 8192 : imm13;

  imm31_12 = value & 0xfffff000;
  imm31_12_signed = imm31_12;

  imm21_1 = (value & 0xff000) + ((value >> 9) & 0x800) +
            ((value >> 20) & 0x7fe) + ((value >> 11) & 0x100000);
  imm21_1_signed = (imm21_1 >= 1048576) ? imm21_1 - 2097152 : imm21_1;

  shamt = ((value >> 20) & 0x3f);
}

std::string
Instruction::printDecodedInstrRISCV(uint64_t pc) {
  if (this->opcode == RISCV_OPIW)   // If we are on opiw, shamt only have 5bits
    this->shamt = this->rs2;

  // In case of immediate shift instr, as shamt needs one more bit the lower bit
  // of funct7 has to be set to 0
  if (this->opcode == RISCV_OPI &&
      (this->funct3 == RISCV_OPI_SLLI || this->funct3 == RISCV_OPI_SRI))
    this->funct7 = this->funct7 & 0x3f;

  std::stringstream stream;
  std::ostringstream ss1;
  std::ostringstream ss2;

  switch (this->opcode) {
  case RISCV_LUI:
    stream << "lui \tx" + std::to_string(this->rd) + ",0x";
    stream << std::hex << (this->imm31_12 >> 12)
           << std::dec;   // shift of 12 bits to have same output as objdump
    break;
  case RISCV_AUIPC:
    stream << "auipc";
    stream << "\tx" + std::to_string(this->rd) + ",0x";
    stream << std::hex << (this->imm31_12 >> 12)
           << std::dec;   // shift of 12 bits to have same output as objdump
    break;
  case RISCV_JAL:
    if (this->rd == 0)
      stream << "j \t" + std::to_string(this->imm31_12);
    else {
      stream << "jal \tx" + std::to_string(this->rd) + ",0x";
      stream << std::hex << (pc - 4 + this->imm21_1_signed) << std::dec;
    }
    break;
  case RISCV_JALR:
    if (this->rd == 0) {
      if (this->rs1 == 1) {   // branch to return address register
        stream << "ret";
      } else {
        stream << std::hex << "jr \t0x" + std::to_string(this->imm31_12)
               << std::dec;
      }
    } else {
      stream << "jalr \t" + std::to_string(this->imm12_I_signed) + "(r" +
                    std::to_string(this->rs1) + ")";
    }
    break;
  case RISCV_BR:
    stream << riscvNamesBR[this->funct3];
    stream << " \tx" + std::to_string(this->rs1) + " r" +
                  std::to_string(this->rs2) + " " +
                  std::to_string(this->imm13_signed);
    break;
  case RISCV_LD:
    stream << riscvNamesLD[this->funct3];
    stream << " \tx" + std::to_string(this->rd) + "," +
                  std::to_string(this->imm12_I_signed) + "(x" +
                  std::to_string(this->rs1) + ")";
    break;
  case RISCV_ST:
    stream << riscvNamesST[this->funct3] << "\t";
    stream << "x" + std::to_string(this->rs2) + "," +
                  std::to_string(this->imm12_S_signed) + "(x" +
                  std::to_string(this->rs1) + ")";
    break;
  case RISCV_OPI:
    if (this->funct3 == RISCV_OPI_SRI)
      if (this->funct7 == RISCV_OPI_SRI_SRLI)
        stream << "slrii \tx" + std::to_string(this->rd) + " = x" +
                      std::to_string(this->rs1) + ", " +
                      std::to_string(this->shamt);
      else   // SRAI
        stream << "srai \tx" + std::to_string(this->rd) + " = x" +
                      std::to_string(this->rs1) + ", " +
                      std::to_string(this->shamt);
    else if (this->funct3 == RISCV_OPI_SLLI) {
      stream << riscvNamesOPI[this->funct3];
      stream << " \tx" + std::to_string(this->rd) + " = x" +
                    std::to_string(this->rs1) + ", " +
                    std::to_string(this->shamt);
    } else {
      stream << riscvNamesOPI[this->funct3];
      stream << " \tx" + std::to_string(this->rd) + ",x" +
                    std::to_string(this->rs1) + "," +
                    std::to_string(this->imm12_I_signed);
    }
    break;
  case RISCV_OP:
    if (this->funct7 == 1) {
      stream << riscvNamesMUL[this->funct3];
      stream << " \tx" + std::to_string(this->rd) + " = x" +
                    std::to_string(this->rs1) + ", r" +
                    std::to_string(this->rs2);
    } else {
      if (this->funct3 == RISCV_OP_ADD)
        if (this->funct7 == RISCV_OP_ADD_ADD)
          stream << "add \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", x" +
                        std::to_string(this->rs2);
        else
          stream << "sub \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", r" +
                        std::to_string(this->rs2);
      else if (this->funct3 == RISCV_OP_SR)
        if (this->funct7 == RISCV_OP_SR_SRL)
          stream << "srl \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", r" +
                        std::to_string(this->rs2);
        else   // SRA
          stream << "sra \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", x" +
                        std::to_string(this->rs2);
      else {
        stream << riscvNamesOP[this->funct3];
        stream << " x" + std::to_string(this->rd) + " = x" +
                      std::to_string(this->rs1) + ",x" +
                      std::to_string(this->rs2);
      }
    }
    break;
  case RISCV_OPIW:
    if (this->funct3 == RISCV_OPIW_SRW)
      if (this->funct7 == RISCV_OPIW_SRW_SRLIW)
        stream << "srlwi \tx" + std::to_string(this->rd) + " = x" +
                      std::to_string(this->rs1) + ", " +
                      std::to_string(this->rs2);
      else   // SRAI
        stream << "srawi \tx" + std::to_string(this->rd) + " = x" +
                      std::to_string(this->rs1) + ", " +
                      std::to_string(this->rs2);
    else if (this->funct3 == RISCV_OPIW_SLLIW) {
      stream << riscvNamesOPI[this->funct3];
      stream << " x" + std::to_string(this->rd) + " = x" +
                    std::to_string(this->rs1) + ", " +
                    std::to_string(this->rs2);
    } else {
      stream << riscvNamesOPIW[this->funct3];
      stream << " x" + std::to_string(this->rd) + " = x" +
                    std::to_string(this->rs1) + ", " +
                    std::to_string(this->imm12_I_signed);
    }
    break;
  case RISCV_OPW:
    if (this->funct7 == 1) {
      stream << riscvNamesMUL[this->funct3];
      stream << "w \tx" + std::to_string(this->rd) + " = x" +
                    std::to_string(this->rs1) + ", x" +
                    std::to_string(this->rs2);
    } else {
      if (this->funct3 == RISCV_OP_ADD)
        if (this->funct7 == RISCV_OPW_ADDSUBW_ADDW)
          stream << "addw \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", x" +
                        std::to_string(this->rs2);
        else
          stream << "subw \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", x" +
                        std::to_string(this->rs2);
      else if (this->funct3 == RISCV_OPW_SRW)
        if (this->funct7 == RISCV_OPW_SRW_SRLW)
          stream << "srlw \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", r" +
                        std::to_string(this->rs2);
        else   // SRAW
          stream << "sraw \tx" + std::to_string(this->rd) + " = x" +
                        std::to_string(this->rs1) + ", x" +
                        std::to_string(this->rs2);
      else {
        stream << riscvNamesOPW[this->funct3];
        stream << " x" + std::to_string(this->rd) + " = x" +
                      std::to_string(this->rs1) + ", x" +
                      std::to_string(this->rs2);
      }
    }
    break;
  case RISCV_SYSTEM:
    stream << "ecall";
    break;
  default:
    stream << "??? ";
    break;
  }

  std::string result(stream.str());
  for (int addedSpace = result.size(); addedSpace < 20; addedSpace++)
    result.append(" ");

  return result;
}
