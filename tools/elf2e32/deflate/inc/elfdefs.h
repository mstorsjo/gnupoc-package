// ELFDEFS.H
//
// Copyright (c) 2001-2004 Symbian Software Ltd.  All rights reserved.
//

// lifted from the ARMELF spec

#ifndef __ELFDEFS_H__
#define __ELFDEFS_H__


// ARMELF 3.1.2
// Data Representation
typedef unsigned int Elf32_Addr;     //Unsigned program address
typedef unsigned short Elf32_Half;   //Unsigned medium integer
typedef unsigned int Elf32_Off;      //Unsigned file offset
typedef signed int Elf32_Sword;      //Signed large integer
typedef unsigned int Elf32_Word;     //Unsigned large integer
typedef unsigned char UChar;         //Unsigned small integer

/*
3.2 ELF Header 
Some object file control structures can grow, because the ELF header
contains their actual sizes. If the object file format changes, a
program may encounter control structures that are larger or smaller
than expected. Programs might therefore ignore extra information. The
treatment of missing information depends on context and will be
specified when and if extensions are defined.
*/
#define EI_NIDENT 16
typedef struct {

  // marks the file as an object file and provide machine-independent 
  // data with which to decode and interpret the file's contents.
  unsigned char e_ident[EI_NIDENT];

  // identifies the object file type.
  Elf32_Half e_type;

  // specifies the required architecture for an individual file.
  Elf32_Half e_machine;

  // identifies the object file version.
  Elf32_Word e_version;

  // gives the virtual address to which the system first transfers 
  // control, thus starting the process. If the file has no associated
  // entry point, this member holds zero.
  Elf32_Addr e_entry;

  // holds the program header table's file offset in bytes. If the 
  // file has no program header table, this member holds zero.
  Elf32_Off e_phoff;

  // holds the section header table's file offset in bytes. If the 
  // file has no section header table, this member holds zero.
  Elf32_Off e_shoff;

  // holds processor-specific flags associated with the file. Flag 
  // names take the form EF_machine_flag.
  Elf32_Word e_flags;

  // holds the ELF header's size in bytes.
  Elf32_Half e_ehsize;

  // holds the size in bytes of one entry in the file's program 
  // header table; all entries are the same size.
  Elf32_Half e_phentsize;

  // holds the number of entries in the program header table. 
  // Thus the product of e_phentsize and e_phnum gives the table's 
  // size in bytes. If a file has no program header table, e_phnum
  // holds the value zero.
  Elf32_Half e_phnum;

  // holds a section header's size in bytes. A section header is 
  // one entry in the section header table; all entries are the same size.
  Elf32_Half e_shentsize;

  // holds the number of entries in the section header table. Thus 
  // the product of e_shentsize and e_shnum gives the section header 
  // table's size in bytes. If a file has no section header table, 
  // e_shnum holds the value zero.
  Elf32_Half e_shnum;

  // holds the section header table index of the entry associated 
  // with the section name string table. If the file has no section 
  // name string table, this member holds the value SHN_UNDEF. 
  Elf32_Half e_shstrndx;

} Elf32_Ehdr;

// values for e_type
#define ET_NONE 	0 // No file type
#define ET_REL 		1 // Re-locatable
#define ET_EXEC 	2 // Executable file
#define ET_DYN 		3 // Shared object
#define ET_CORE 	4 // Core file
#define ET_LOPROC  0xff00 // Processor-specific
#define ET_HIPROC  0xffff // Processor-specific

//values for e_machine
#define EM_NONE 	0 // No machine
#define EM_M32 		1 // AT&T WE 32100
#define EM_SPARC 	2 // SPARC
#define EM_386 		3 // Intel Architecture
#define EM_68K 		4 // Motorola 68000
#define EM_88K 		5 // Motorola 88000
#define EM_860 		7 // Intel 80860
#define EM_MIPS 	8 // MIPS RS3000 Big-Endian
#define EM_MIPS_RS4_BE 10 // MIPS RS4000 Big-Endian
//#define RESERVED 11-16 Reserved for future use
#define EM_ARM 	       40 //ARM/Thumb Architecture

// values for e_version
#define EV_NONE 	0 // Invalid version
#define EV_CURRENT 	1 // Current version

// ELF Identification
#define EI_MAG0 	0 // File identification
#define EI_MAG1 	1 // File identification
#define EI_MAG2 	2 // File identification
#define EI_MAG3 	3 // File identification
#define EI_CLASS 	4 // File class
#define EI_DATA 	5 // Data encoding
#define EI_VERSION 	6 // File version
#define EI_PAD 		7 // Start of padding bytes

// values for e_ident[0-3]
#define ELFMAG0        0x7f // e_ident[EI_MAG0]
#define ELFMAG1 	'E' // e_ident[EI_MAG1]
#define ELFMAG2 	'L' // e_ident[EI_MAG2]
#define ELFMAG3 	'F' // e_ident[EI_MAG3]

// values for e_ident[EI_CLASS]- identifies the file's class, or capacity.
#define ELFCLASSNONE 	0 // Invalid class
#define ELFCLASS32 	1 // 32-bit objects
#define ELFCLASS64 	2 // 64-bit objects

// values for e_ident[EI_DATA] - specifies the data encoding of the 
// processor-specific data in the object file. 
#define ELFDATANONE 	0 // Invalid data encoding
#define ELFDATA2LSB 	1 // 2's complement , with LSB at lowest address.
#define ELFDATA2MSB 	2 // 2's complement , with MSB at lowest address.

// ARM/THUMB specific values for e_flags

// e_entry contains a program-loader entry point
#define EF_ARM_HASENTRY 0x02
// Each subsection of the symbol table is sorted by symbol value
#define EF_ARM_SYMSARESORTED 0x04
// Symbols in dynamic symbol tables that are defined in sections
// included in program segment n have st_shndx = n+ 1. 
#define EF_ARM_DYNSYMSUSESEGIDX 0x8
// Mapping symbols precede other local symbols in the symbol table
#define EF_ARM_MAPSYMSFIRST 0x10
// This masks an 8-bit version number, the version of the ARM EABI to
// which this ELF file conforms. This EABI is version 2. A value of 0
// denotes unknown conformance. (current version is 0x02000000)
#define EF_ARM_EABIMASK 0xFF000000

#define EF_ARM_EABI_VERSION 0x02000000

/* 
3.3 Sections

An object file's section header table lets one locate all the file's
sections. The section header table is an array of Elf32_Shdr
structures as described below. A section header table index is a
subscript into this array. The ELF header's e_shoff member gives the
byte offset from the beginning of the file to the section header
table; e_shnum tells how many entries the section header table
contains; e_shentsize gives the size in bytes of each entry.
*/

// Some section header table indexes are reserved; an object file will
// not have sections for these special indexes.

// marks an undefined, missing, irrelevant, or otherwise meaningless 
// section reference.
#define SHN_UNDEF 	0
// specifies the lower bound of the range of reserved indexes.
#define SHN_LORESERVE 	0xff00
// SHN_LOPROC-SHN_HIPRO - this inclusive range reserved for 
// processor-specific semantics.
#define SHN_LOPROC 	0xff00
#define SHN_HIPROC 	0xff1f
// Specifies absolute values for the corresponding reference. 
// For example, symbols defined relative to section number SHN_ABS have 
// absolute values and are not affected by relocation.
#define SHN_ABS 	0xfff1
// Symbols defined relative to this section are common symbols, 
// such as FORTRAN COMMON or unallocated C external variables.
#define SHN_COMMON 	0xfff2
// specifies the upper bound of the range of reserved indexes.
#define SHN_HIRESERVE 	0xffff

typedef struct {

  // specifies the name of the section. Its value is an index into the
  // section header string table section [see String Tablebelow],
  // giving the location of a null-terminated string.
  Elf32_Word sh_name;

  // categorizes the section's contents and semantics. Section types
  // and their descriptions appear below.
  Elf32_Word sh_type;

  // Sections support 1-bit flags that describe miscellaneous
  // attributes. Flag definitions appear below.
  Elf32_Word sh_flags;

  // If the section will appear in the memory image of a process, this
  // member gives the address at which the section's first byte should
  // reside. Otherwise, the member contains 0.
  Elf32_Addr sh_addr;

  // gives the byte offset from the beginning of the file to the first
  // byte in the section.One section type, SHT_NOBITS described below,
  // occupies no space in the file, and its sh_offset member locates
  // the conceptual placement in the file.
  Elf32_Off sh_offset;

  // gives the section's size in bytes. Unless the section type is
  // SHT_NOBITS, the section occupies sh_size bytes in the file. A
  // section of type SHT_NOBITS may have a non-zero size, but it
  // occupies no space in the file.
  Elf32_Word sh_size;

  // holds a section header table index link, whose interpretation
  // depends on the section type. A table below describes the values.
  Elf32_Word sh_link;

  // holds extra information, whose interpretation depends on the
  // section type. A table below describes the values.
  Elf32_Word sh_info;

  // Some sections have address alignment constraints. For example, if
  // a section holds a doubleword, the system must ensure double-word
  // alignment for the entire section. That is, the value of sh_addr
  // must be congruent to 0, modulo the value of
  // sh_addralign. Currently, only 0 and positive integral powers of
  // two are allowed. Values 0 and 1 mean the section has no alignment
  // constraints.
  Elf32_Word sh_addralign;

  // Some sections hold a table of fixed-size entries, such as a
  // symbol table. For such a section, this member gives the size in
  // bytes of each entry. The member contains 0 if the section does
  // not hold a table of fixedsize entries. A section header's sh_type
  // member specifies the section's semantics.
  Elf32_Word sh_entsize;

} Elf32_Shdr;

// values for sh_type 

#define SHT_NULL 0 // marks the section header as inactive; it does
 // not have an associated section. Other members of the section
 // header have undefined values.
#define SHT_PROGBITS 1 // The section holds information defined by the
 // program, whose format and meaning are determined solely by the
 // program.
#define SHT_SYMTAB 2 //These sections hold a symbol table.
#define SHT_STRTAB 3 // The section holds a string table.
#define SHT_RELA 4 // The section holds relocation entries with
 // explicit addends, such as type Elf32_Rela for the 32-bit class of
 // object files. An object file may have multiple relocation
 // sections. See Relocationbelow for details.
#define SHT_HASH 5 // The section holds a symbol hash table.
#define SHT_DYNAMIC 6 // The section holds information for dynamic
 // linking.
#define SHT_NOTE 7 // This section holds information that marks the
 // file in some way.
#define SHT_NOBITS 8 // A section of this type occupies no space in
 // the file but otherwise resembles SHT_PROGBITS. Although this
 // section contains no bytes, the sh_offset member contains the
 // conceptual file offset.
#define SHT_REL 9 // The section holds relocation entries without
 // explicit addends, such as type Elf32_Rel for the 32-bit class of
 // object files. An object file may have multiple relocation
 // sections. See Relocationbelow for details.
#define SHT_SHLIB 10 // This section type is reserved but has
 // unspecified semantics.
#define SHT_DYNSYM 11 // This section hold dynamic symbol information
// SHT_LOPROC through SHT_HIPROC - Values in this inclusive range are
// reserved for processor-specific semantics.
#define SHT_LOPROC     0x70000000
#define SHT_HIPROC     0x7fffffff
// Section types between SHT_LOUSER and SHT_HIUSER may be used by the
// application, without conflicting with current or future
// system-defined section types.
#define SHT_LOUSER 0x80000000 // This value specifies the lower bound
 // of the range of indexes reserved for application programs.
#define SHT_HIUSER 0xffffffff // This value specifies the upper bound
 // of the range of indexes reserved for application programs.

// values for sh_flags

// The section contains data that should be writable during process execution
#define SHF_WRITE 0x1 
// The section occupies memory during process execution. Some control
// sections do not reside in the memory image of an object file; this
// attribute is off for those sections
#define SHF_ALLOC 0x2 
// The section contains executable machine instructions.
#define SHF_EXECINSTR 0x4 
// Bits in this mask are reserved for processor-specific semantics.
#define SHF_MASKPROC 0xf0000000 


typedef struct {

  // holds an index into the object file's symbol string table, which
  // holds the character representations of the symbol names.
  Elf32_Word st_name;

  // gives the value of the associated symbol. Depending on the
  // context this may be an absolute value, an address, and so on
  Elf32_Addr st_value;

  // Many symbols have associated sizes. For example, a data object's
  // size is the number of bytes contained in the object. This member
  // holds 0 if the symbol has no size or an unknown size.
  Elf32_Word st_size;

  // This member specifies the symbol's type and binding
  // attributes. The following code shows how to manipulate the
  // values.
#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))
  unsigned char st_info;

  // This member currently holds 0 and has no defined meaning.
  unsigned char st_other;

  // Every symbol table entry is defined in relation to some section;
  // this member holds the relevant section header table index.
  Elf32_Half st_shndx;

} Elf32_Sym;

// Local symbols are not visible outside the object file containing
// their definition. Local symbols of the same name may exist in
// multiple files without interfering with each other.
#define STB_LOCAL 0
// Global symbols are visible to all object files being combined. One
// file's definition of a global symbol will satisfy another file's
// undefined reference to the same global symbol.
#define STB_GLOBAL 1
// Weak symbols resemble global symbols, but their definitions have
// lower precedence. Undefined weak symbols (weak references) may have
// processor- or OS-specific semantics
#define STB_WEAK 2 
// STB_LOPROC through STB_HIPROC - values in this inclusive range are
// reserved for processor-specific semantics.
#define STB_LOPROC 13 
#define STB_HIPROC 15

// The symbol's type is not specified.
#define STT_NOTYPE 0 
// The symbol is associated with a data object, such as a variable, an
// array, and so on.
#define STT_OBJECT 1 
// The symbol is associated with a function or other executable code.
#define STT_FUNC 2 
// The symbol is associated with a section. Symbol table entries of
// this type exist primarily for relocation and normally have
// STB_LOCAL binding.
#define STT_SECTION 3 
// A file symbol has STB_LOCAL binding, its section index is SHN_A BS,
// and it precedes the other STB_LOCAL symbols for the file, if it is
// present.
#define STT_FILE 4 
// Values in this inclusive range are reserved for processor-specific
// semantics. If a symbol's value refers to a specific location within
// a section, its section index member, st_shndx, holds an index into
// the section header table. As the section moves during relocation,
// the symbol's value changes as well, and references to the symbol
// continue to point to the same location in the program. Some special
// section index values give other semantics.
#define STT_LOPROC 13
#define STT_HIPROC 15


// Relocation Entries

typedef struct { 

  // r_offset gives the location at which to apply the relocation
  // action. For a relocatable file, the value is the byte offset from
  // the beginning of the section to the storage unit affected by the
  // relocation. For an executable file or a shared object, the value
  // is the virtual address of the storage unit affected by the
  // relocation.
  Elf32_Addr r_offset;

  // r_info gives both the symbol table index with respect to which
  // the relocation must be made, and the type of relocation to
  // apply. For example, a call instruction's relocation entry would
  // hold the symbol table index of the function being called. If the
  // index is STN_UNDEF, the undefined symbol index, the relocation
  // uses 0 as the symbol value. Relocation types are
  // processor-specific; descriptions of their behavior appear in
  // section 4.5, Relocation types. When the text in section 4.5
  // refers to a relocation entry's relocation type or symbol table
  // index, it means the result of applying ELF32_R_TYPE or
  // ELF32_R_SYM, respectively, to the entry's r_info member.

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

  Elf32_Word r_info; 
} Elf32_Rel; 

typedef struct {
  Elf32_Addr r_offset;
  Elf32_Word r_info;
  Elf32_Sword r_addend;
} Elf32_Rela;

// Program Header

typedef struct {

  // p_type tells what kind of segment this array element describes or
  // how to interpret the array element's information. Type values and
  // their meanings are given below.
  Elf32_Word p_type;

  // p_offset gives the offset from the start of the file at which the
  // first byte of the segment resides.
  Elf32_Off p_offset;

  // p_vaddr gives the virtual address at which the first byte of the
  // segment resides in memory.
  Elf32_Addr p_vaddr;

  // p_paddr - On systems for which physical addressing is relevant,
  // this member is reserved for the segment's physical address. This
  // member requires operating system specific information.
  Elf32_Addr p_paddr;

  // p_filesz gives the number of bytes in the file image of the
  // segment; it may be zero.
  Elf32_Word p_filesz;

  // p_memsz gives the number of bytes in the memory image of the
  // segment; it may be zero.
  Elf32_Word p_memsz;

  // p_flags gives flags relevant to the segment. Defined flag values
  // are given below.
  Elf32_Word p_flags;

  // p_align - Loadable process segments must have congruent values
  // for p_vaddr and p_offset, modulo the page size. This member gives
  // the value to which the segments are aligned in memory and in the
  // file. Values 0 and 1 mean that no alignment is
  // required. Otherwise, p_align should be a positive, integral power
  // of 2, and p_vaddr should equal p_offset, modulo p_align.
  Elf32_Word p_align;

} Elf32_Phdr;

// Segment types - values for p_type

// The array element is unused; other members' values are
// undefined. This type lets the program header table have ignored
// entries.
#define PT_NULL 0 
// The array element specifies a loadable segment, described by
// p_filesz and p_memsz (for additional explanation, see
// PT_LOAD below).
#define PT_LOAD 1 
// The array element specifies dynamic linking information. See
// subsection 4.7.
#define PT_DYNAMIC 2 
// The array element specifies the location and size of a
// null-terminated pathname to invoke as an interpreter.
#define PT_INTERP 3 
// The array element specifies the location and size of auxiliary
// information.
#define PT_NOTE 4 
// This segment type is reserved but has unspecified semantics.
#define PT_SHLIB 5 
// The array element, if present, specifies the location and size of
// the program header table itself (for additional explanation, see
// PT_ PHDR below).
#define PT_PHDR 6 
// Values in the inclusive [PT_LOPROC, PT_HIPROC] range are reserved
// for processor-specific semantics.
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

// values for p_flags
// The segment may be executed.
#define PF_X 1 
// The segment may be written to.
#define PF_W 2 
// The segment may be read.
#define PF_R 4 
// Reserved for processor-specific purposes (see 4.6, Program
// headers).
#define PF_MASKPROC 0xf0000000 
#define PF_ARM_ENTRY 0x80000000


// Relocation types

// ELF defines two sorts of relocation directive, SHT_REL, and
// SHT_RELA. Both identify:
//
// o A section containing the storage unit - byte, half-word, word, or
//   instruction - being relocated.
// o An offset within the section - or the address within an
//   executable program - of the storage unit itself.
// o A symbol,the value of which helps to define a new value for the
//   storage unit.
// o A relocation typethat defines the computation to be
//   performed. Computations are performed using 2's complement, 32-bit,
//   unsigned arithmetic with silent overflow.
// o An addend, that also helps to define a new value for the storage
//   unit.
//
// The addend may be encoded wholly in a field of the storage unit
// being relocated - relocation sort SHT_REL - or partly there and
// partly in the addendfield of the relocation directive - relocation
// sort SHT_RELA. Tables below describe the computation associated
// with each relocation type, using the following notation:
//
// A - denotes the addend used to compute the new value of the storage
//     unit being relocated.
//   - It is the value extracted from the storage unit being relocated
//     (relocation directives of sort SHT_REL) or the sum of that
//     value and the r_addend field of the relocation directive (sort
//     SHT_RELA).
//   - If it has a unit, the unit is bytes. An encoded address or
//     offset value is converted to bytes on extraction from a storage
//     unit and re-encoded on insertion into a storage unit.
//
// P - denotes the place (section offset or address of the storage
//     unit) being re-located. It is the sum of the r_offset field of
//     the relocation directive and the base address of the section
//     being re-located.
//
// S - denotes the value of the symbol whose symbol table index is
//     given in the r_info field of the relocation directive.
//
// B - denotes the base address of the consolidated section in which
//     the symbol is defined. For relocations of type R_ARM_SBREL32,
//     this is the least static data address (the static base).
//
// relocation types 0-16 are generic
//      Name             Type    Field                  Computation
//====================================================================
#define R_ARM_NONE         0  // Any                    No relocation. 
#define R_ARM_PC24         1  // ARM B/BL               S - P + A
#define R_ARM_ABS32        2  // 32-bit word            S + A
#define R_ARM_REL32        3  // 32-bit word            S - P + A
#define R_ARM_PC13         4  // ARM LDR r, [pc,...]    S - P + A
#define R_ARM_ABS16        5  // 16-bit half-word       S + A
#define R_ARM_ABS12        6  // ARM LDR/STR            S + A
#define R_ARM_THM_ABS5     7  // Thumb LDR/STR          S + A
#define R_ARM_ABS8         8  // 8-bit byte             S + A
#define R_ARM_SBREL32      9  // 32-bit word            S - B + A
#define R_ARM_THM_PC22    10  // Thumb BL pair          S - P + A
#define R_ARM_THM_PC8     11  // Thumb LDR r, [pc,...]  S - P + A
#define R_ARM_AMP_VCALL9  12  // AMP VCALL              Obsolete - SA-1500
#define R_ARM_SWI24       13  // ARM SWI                S + A
#define R_ARM_THM_SWI8    14  // Thumb SWI              S + A
#define R_ARM_XPC25       15  // ARM BLX                S - P + A
#define R_ARM_THM_XPC22   16  // Thumb BLX pair         S - P + A

// relocation types 17-31 are reserved for ARM Linux

#define R_ARM_ALU_PCREL_7_0   32 // ARM ADD/SUB         (S - P + A) & 0x000000FF
#define R_ARM_ALU_PCREL_15_8  33 // ARM ADD/SUB         (S - P + A) & 0x0000FF00
#define R_ARM_ALU_PCREL_23_15 34 // ARM ADD/SUB         (S - P + A) & 0x00FF0000
#define R_ARM_LDR_SBREL_11_0  35 // ARM ADD/SUB         (S - B + A) & 0x00000FFF
#define R_ARM_ALU_SBREL_19_12 36 // ARM ADD/SUB         (S - B + A) & 0x000FF000
#define R_ARM_ALU_SBREL_27_20 37 // ARM ADD/SUB         (S - B + A) & 0x0FF00000

// Dynamic relocation types 

// A small set of relocation types supports relocating executable ELF
// files. They are used only in a relocation section embedded in a
// dynamic segment (see section 4.7, Dynamic linking and
// relocation). They cannot be used in a relocation section in a
// re-locatable ELF file. In Figure 4-13 below:
//
// .S is the displacement from its statically linked virtual address
//    of the segment containing the symbol definition.
//
// .P is the displacement from its statically linked virtual address
//    of the segment containing the place to be relocated.
//
// .SB is the displacement of the segment pointed to by the static
//     base (PF_ARM_SB is set in the p_flags field of this segment's
//     program header - see 4.6, Program headers).


// types 249 - 255 are dynamic relocation types and are only used in dynamic sections
#define R_ARM_RXPC25    249    // ARM BLX             (.S - .P) + A
                               //                     For calls between program segments.
#define R_ARM_RSBREL32  250    // Word                (.S - .SB) + A
                               //                     For an offset from SB, the static base.
#define R_ARM_THM_RPC22 251    // Thumb BL/BLX pair   (.S - .P) + A
                               //                     For calls between program segments.
#define R_ARM_RREL32    252    // Word                (.S - .P) + A
                               //                     For on offset between two segments.
#define R_ARM_RABS32    253    // Word                .S + A
                               //                     For the address of a location in the target segment.
#define R_ARM_RPC24     254    // ARM B/BL            (.S - .P) + A
                               //                     For calls between program segments.
#define R_ARM_RBASE     255    // None                Identifies the segment being relocated by
                               //                     the following relocation directives.
// DYNAMIC SEGMENT
// The dynamic segment begins with a dynamic section containing an array of structures of type:
typedef struct Elf32_Dyn {
  Elf32_Sword d_tag;
  Elf32_Word d_val;
} Elf32_Dyn;

// This entry marks the end of the dynamic array. mandatory
#define DT_NULL 0
// Index in the string table of the name of a needed library. multiple
#define DT_NEEDED 1
// These entries are unused by versions 1-2 of the ARM EABI. unused
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
// The offset of the hash table section in the dynamic segment. mandatory
#define DT_HASH 4
// The offset of the string table section in the dynamic segment. mandatory
#define DT_STRTAB 5
//  The offset of the symbol table section in the dynamic segment. mandatory
#define DT_SYMTAB 6
// The offset in the dynamic segment of an SHT_RELA relocation
// section, Its bytesize,and the byte size of an ARMRELA-type
// relocation entry. optional
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9
// The byte size of the string table section. mandatory
#define DT_STRSZ 10
// The byte size of an ARM symbol table entry. mandatory
#define DT_SYMENT 11
// These entries are unused by versions 1-2 of the ARM EABI. unused
#define DT_INIT 12
#define DT_FINI 13
// The Index in the string table of the name of this shared object. mandatory
#define DT_SONAME 14
// Unused by the ARM EABI. unused
#define DT_RPATH 15
#define DT_SYMBOLIC 16
//The offset in the dynamic segment of an SHT_REL relocation section,
//Its bytesize, and the byte size of an ARMREL-type relocation
//entry. optional
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
// These entries are unused by versions 1-2 of the ARM EABI. unused
#define DT_PLTREL 20
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_BIND_NOW 24
// Values in this range are reserved to the ARM EABI. unused
#define DT_LOPROC  0x70000000
#define DT_HIPROC  0x7fffffff
#define DT_ARM_SYMTABSZ    0x70000000 // For RVCT 2.1
#define DT_ARM_SYMTABSZ_22 0x70000001 // The DT_ARM_SYMTABSZ tag value has been changed from RVCT2.2

// What the hash table looks like in the dynamic segment
typedef struct Elf32_HashTable {
  Elf32_Word nBuckets;
  Elf32_Word nChains;
  // Elf32_Word bucket[nBuckets];
  // Elf32_Word chain[nChains];
} Elf32_HashTable;

#endif





