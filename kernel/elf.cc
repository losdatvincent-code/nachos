
/*! \file  elf.h
    \brief Data structures pertaining to the ELF32 and ELF64 1.1 file format
    specification
*/

#include "elf.h"

/** 	Management of ELF files, called when loading a new program in memory
 //
 //	\param elfFile is the file containing the object code
 //   \param is32bits (1 our 0) says if 32-bit or 64-bit elf header
 //   \param err: error code 0 if OK, -1 otherwise
 */
ElfFile::ElfFile(OpenFile *elfFile, char is32bits, uint64_t *err) {

  // Type of Elf header
  this->is32Hdr = is32bits;
  this->incorrect_header = 0;

  // Read header and check validity
  if (this->is32Hdr) {
    elfFile->ReadAt((char *) &elf32Hdr, sizeof(elf32Hdr), 0);
    CheckELF32Header(&this->elf32Hdr, err);
  } else {
    elfFile->ReadAt((char *) &elf64Hdr, sizeof(elf64Hdr), 0);
    CheckELF64Header(&this->elf64Hdr, err);
  }
  if (*err != NO_ERROR) {
    this->incorrect_header = 1;
    return;
  }

  // Read section table
  if (this->is32Hdr) {
    section_table32 =
        (Elf32_Shdr *) new char[elf32Hdr.e_shnum * sizeof(elf32Hdr)];
    elfFile->ReadAt((char *) section_table32,
                      elf32Hdr.e_shnum * sizeof(elf32Hdr), elf32Hdr.e_shoff);
    // Read section names
    shname_section32 = &section_table32[elf32Hdr.e_shstrndx];
    shnames = new char[shname_section32->sh_size];
    elfFile->ReadAt(shnames, shname_section32->sh_size,
                      shname_section32->sh_offset);
  } else {
    section_table64 =
        (Elf64_Shdr *) new char[elf64Hdr.e_shnum * sizeof(elf64Hdr)];
    elfFile->ReadAt((char *) section_table64,
                      elf64Hdr.e_shnum * sizeof(elf64Hdr), elf64Hdr.e_shoff);
    // Read section names
    shname_section64 = &section_table64[elf64Hdr.e_shstrndx];
    shnames = new char[shname_section64->sh_size];
    elfFile->ReadAt(shnames, shname_section64->sh_size,
                      shname_section64->sh_offset);
  }
}

void
ElfFile::CheckELF32Header(Elf32_Ehdr *elfHdr, uint64_t *err) {
  /* Make sure it is an ELF file by looking at its header (see elf32.h) */
  if (elfHdr->e_ident[EI_MAG0] != 0x7f || elfHdr->e_ident[EI_MAG1] != 'E' ||
      elfHdr->e_ident[EI_MAG2] != 'L' || elfHdr->e_ident[EI_MAG3] != 'F') {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure the ELF file type is correct */
  if (elfHdr->e_ident[EI_CLASS] != ELFCLASS32 ||
      elfHdr->e_ident[EI_VERSION] != EV_CURRENT) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Check the endianess of the generated code */
  if (elfHdr->e_ident[EI_DATA] == ELFDATA2MSB) {
    risc_endianess = IS_BIG_ENDIAN;
  } else {
    risc_endianess = IS_LITTLE_ENDIAN;
  }

  /* if the endianess is not the same as machine, fails.*/
  if (risc_endianess != host_endianess) {
    *err = WRONG_FILE_ENDIANESS;
    return;
  }

  /* Make sure ELF binary code a RISCV executable */
  if (elfHdr->e_machine != EM_RISC || elfHdr->e_type != ET_EXEC) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure ELF file internal structures are consistent with what
     we expect */
  if (elfHdr->e_ehsize != sizeof(Elf32_Ehdr) ||
      elfHdr->e_shentsize != sizeof(Elf32_Shdr)) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure ELF section table is available */
  if (elfHdr->e_shoff < sizeof(Elf32_Ehdr)) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure there is a string section name section */
  if (elfHdr->e_shstrndx >= elfHdr->e_shnum) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }
  *err = NO_ERROR;
}

void
ElfFile::CheckELF64Header(Elf64_Ehdr *elfHdr, uint64_t *err) {
  /* Make sure it is an ELF file by looking at its header (see elf.h) */
  if (elfHdr->e_ident[EI_MAG0] != 0x7f || elfHdr->e_ident[EI_MAG1] != 'E' ||
      elfHdr->e_ident[EI_MAG2] != 'L' || elfHdr->e_ident[EI_MAG3] != 'F') {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure the ELF file type is correct */
  if (elfHdr->e_ident[EI_CLASS] != ELFCLASS64 ||
      elfHdr->e_ident[EI_VERSION] != EV_CURRENT) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Check the endianess of the generated code */
  if (elfHdr->e_ident[EI_DATA] == ELFDATA2MSB) {
    risc_endianess = IS_BIG_ENDIAN;
  } else {
    risc_endianess = IS_LITTLE_ENDIAN;
  }

  /* if the endianess is not the same as machine, fails.*/
  if (risc_endianess != host_endianess) {
    *err = WRONG_FILE_ENDIANESS;
    return;
  }

  /* Make sure ELF binary code a RISCV executable */
  if (elfHdr->e_machine != EM_RISC || elfHdr->e_type != ET_EXEC) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure ELF file internal structures are consistent with what
     we expect */
  if (elfHdr->e_ehsize != sizeof(Elf64_Ehdr) ||
      elfHdr->e_shentsize != sizeof(Elf64_Shdr)) {
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure ELF section table is available */
  if (elfHdr->e_shoff < sizeof(Elf64_Ehdr)) {
    printf("Error 2\n");
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  /* Make sure there is a string section name section */
  if (elfHdr->e_shstrndx >= elfHdr->e_shnum) {
    printf("Error 3\n");
    *err = EXEC_FILE_FORMAT_ERROR;
    return;
  }

  *err = NO_ERROR;
}
