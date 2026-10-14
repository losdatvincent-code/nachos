/*! \file  elf.h
    \brief Data structures pertaining to the ELF32 and ELF64 1.1 file format
    specification
*/

#ifndef NACHOS_ELF_H
#define NACHOS_ELF_H

#include "filesys/filehdr.h"
#include "filesys/filesys.h"
#include "filesys/openfile.h"
#include "kernel/msgerror.h"
#include <stdint.h>

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef int64_t Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;

#define EI_NIDENT 16

//! ELF file header (only used fields are commented)
// ELF 32 bits
typedef struct {
  unsigned char e_ident[EI_NIDENT];   //!< Identification of the file.
                                      //!< Starts with '0x7f 'E' 'L' 'F'
  Elf32_Half e_type;                  //!< File type
  Elf32_Half e_machine;               //!< Architecture (here EM_RISC)
  Elf32_Word e_version;               //!< Object file version
  Elf32_Addr e_entry;                 //!< Address of the entry point
  Elf32_Off e_phoff;                  //!< Offset of the program header table
  Elf32_Off e_shoff;                  //!< Offset of the section header table
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;   //!< Header size (bytes)
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
} Elf32_Ehdr;

// ELF 64 bits
typedef struct {
  unsigned char e_ident[EI_NIDENT];   //!< Identification of the file.
                                      //!< Starts with '0x7f 'E' 'L' 'F'
  Elf64_Half e_type;                  //!< File type
  Elf64_Half e_machine;               //!< Architecture (here EM_RISC)
  Elf64_Word e_version;               //!< Object file version
  Elf64_Addr e_entry;                 //!< Address of the entry point
  Elf64_Off e_phoff;                  //!< Offset of the program header table
  Elf64_Off e_shoff;                  //!< Offset of the section header table
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;   //!< Header size (bytes)
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

/* e_ident offsets */
#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7

/* e_ident[EI_CLASS] */
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

/* e_ident[EI_DATA] */
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/* e_type */
#define ET_NONE   0      /* No file type       */
#define ET_REL    1      /* Relocatable file   */
#define ET_EXEC   2      /* Executable file    */
#define ET_DYN    3      /* Shared object file */
#define ET_CORE   4      /* Core file          */
#define ET_LOPROC 0xff00 /* Processor-specific */
#define ET_HIPROC 0xffff /* Processor-specific */

/* e_machine */
#define EM_NONE  0   /* No machine     */
#define EM_M32   1   /* AT&T WE 32100  */
#define EM_SPARC 2   /* SPARC          */
#define EM_386   3   /* Intel 80386    */
#define EM_68K   4   /* Motorola 68000 */
#define EM_88K   5   /* Motorola 88000 */
#define EM_860   7   /* Intel 80860    */
#define EM_MIPS  8   /* MIPS R3000     */
#define EM_RISC  243 /* RISCV */

/* e_version */
#define EV_NONE    0 /* invalid version */
#define EV_CURRENT 1 /* current version */

//! Program header
typedef struct {
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;

/* Reserved section table indexes */
#define SHN_UNDEF     0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff01
#define SHN_HIPROC    0xff1f
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

//! Section header (only used fields are commented)
typedef struct {
  Elf32_Word sh_name;   //!< Section name (index in string table)
  Elf32_Word sh_type;   //!< Section type (see flags below)
  Elf32_Word sh_flags;
  Elf32_Addr sh_addr;    //!< Section virtual address
  Elf32_Off sh_offset;   //!< Section offset in file
  Elf32_Word sh_size;    //!< Section size (bytes)
  Elf32_Word sh_link;
  Elf32_Word sh_info;
  Elf32_Word sh_addralign;
  Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct {
  Elf64_Word sh_name;   //!< Section name (index in string table)
  Elf64_Word sh_type;   //!< Section type (see flags below)
  Elf64_Xword sh_flags;
  Elf64_Addr sh_addr;    //!< Section virtual address
  Elf64_Off sh_offset;   //!< Section offset in file
  Elf64_Xword sh_size;   //!< Section size (bytes)
  Elf64_Word sh_link;
  Elf64_Word sh_info;
  Elf64_Xword sh_addralign;
  Elf64_Xword sh_entsize;
} Elf64_Shdr;

/* section type */

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8   //!< The section occupies no space in the file
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11
#define SHT_LOPROC   0x70000000
#define SHT_HIPROC   0x7fffffff
#define SHT_LOUSER   0x80000000
#define SHT_HIUSER   0xffffffff

/* section flags */
#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC  0xf0000000

class ElfFile {
  char is32Hdr;
  char incorrect_header;
  Elf64_Ehdr elf64Hdr;   // Header du fichier executable
  Elf32_Ehdr elf32Hdr;   // Header du fichier executable
  void CheckELF32Header(Elf32_Ehdr *elfHdr, uint64_t *err);
  void CheckELF64Header(Elf64_Ehdr *elfHdr, uint64_t *err);
  Elf32_Shdr *section_table32;
  Elf64_Shdr *section_table64;
  Elf32_Shdr *shname_section32;
  Elf64_Shdr *shname_section64;
  char *shnames;

public:
  /** 	Management of ELF files, called when loading a new program in memory
   //
   //	\param elfFile is the file containing the object code
   //   \param is32bits (1 our 0) says if 32-bit or 64-bit elf header
   //   \param err: error code 0 if OK, -1 otherwise
   */
  ElfFile(OpenFile *elfFile, char is32bits, uint64_t *err);

  /**   Deallocates an address space and in particular frees
   *   all memory it uses (RAM and swap area).
   */
  ~ElfFile() {
    if (incorrect_header == 0) {
      if (is32Hdr)
        delete section_table32;
      else
        delete section_table64;
      delete shnames;
    }
  }

  /**	Get number of sections in Elf
   *      \return number of sections
   */
  uint16_t getShNum() {
    if (is32Hdr)
      return elf32Hdr.e_shnum;
    else
      return elf64Hdr.e_shnum;
  }

  /**	Get address of entry point
   *      \return virtual address of entry point
   */
  uint64_t getEntry() {
    if (is32Hdr)
      return elf32Hdr.e_entry;
    else
      return elf64Hdr.e_entry;
  }

  /**	Get virtual address of section number i
   *      \param i = section number
   *      \return virtual address of section
   */
  uint64_t getShAddr(int i) {
    if (is32Hdr)
      return section_table32[i].sh_addr;
    else
      return section_table64[i].sh_addr;
  }

  /**	Get size of section number i
   *      \param i = section number
   *      \return size of section in bytes
   */
  uint64_t getShSize(int i) {
    if (is32Hdr)
      return section_table32[i].sh_size;
    else
      return section_table64[i].sh_size;
  }

  /**	Get flags of section number i
   *      \param i = section number
   *      \return size of section in bytes
   */
  uint64_t getShFlags(int i) {
    if (is32Hdr)
      return section_table32[i].sh_flags;
    else
      return section_table64[i].sh_flags;
  }

  /**	Get type of section number i
   *      \param i = section number
   *      \return type of section
   */
  uint64_t getShType(int i) {
    if (is32Hdr)
      return section_table32[i].sh_type;
    else
      return section_table64[i].sh_type;
  }

  /**	Get offset in file for section number i
   *      \param i = section number
   *      \return offset in file (bytes)
   */
  uint64_t getShOffset(int i) {
    if (is32Hdr)
      return section_table32[i].sh_offset;
    else
      return section_table64[i].sh_offset;
  }

  /**	Get name of section number i
   *      \param i = section number
   *      \return name of section
   */
  const char *getShName(int i) {
    if (is32Hdr)
      return shnames + section_table32[i].sh_name;
    else
      return shnames + section_table64[i].sh_name;
  }
};

#endif /* NACHOS_ELF_H */
