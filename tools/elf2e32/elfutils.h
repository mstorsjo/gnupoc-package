/*
    Elf2e32
    Copyright 2007 - 2009 Martin Storsjo

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holder of this
    program gives permission to link the code of its release of elf2e32
    with modified Symbian code under the Symbian Example Source license;
    and distribute the linked executables. You must obey the GNU General
    Public License in all respects for all of the code used other the
    Symbian copyrighted code. If you modify this file, you may extend this
    exception to your version of the file, but you are not obliged to do
    so. If you do not wish to do so, delete this exception statement from
    your version.

    Martin Storsjo
    martin@martin.st
*/

#ifndef __ELFUTILS_H
#define __ELFUTILS_H

#ifndef R_ARM_NONE
#define R_ARM_NONE      0
#define R_ARM_ABS32     2
#define R_ARM_GLOB_DAT  21
#define R_ARM_RELATIVE  23
#endif
#ifndef R_ARM_RABS22
#define R_ARM_RABS22	253
#endif

#include <libelf.h>

bool getLibraryName(Elf_Data* verneed, Elf32_Word verneedNum, Elf32_Half index, Elf32_Verneed** needPtr, Elf32_Vernaux** auxPtr);
const char* getDsoName(Elf* elf, size_t stringSection, Elf_Data* verneed, Elf32_Word verneedNum, Elf32_Half index);
const char* getDllName(Elf* elf, size_t stringSection, Elf_Data* verneed, Elf32_Word verneedNum, Elf32_Half index);

bool findSymbol(Elf* elf, Elf_Scn* section, Elf32_Shdr* shdr, const char* symbolName, Elf32_Sym** symPtr);

bool findSection(Elf* elf, Elf32_Word type, Elf_Scn** sectionPtr, Elf32_Shdr** shdrPtr);

bool getDynamicValue(Elf* elf, Elf_Scn* section, Elf32_Shdr* shdr, Elf32_Sword tag, Elf32_Word* value);
bool getDynamicValue(Elf* elf, Elf32_Sword tag, Elf32_Word* value);

bool findSymbol(Elf* elf, const char* symbol, Elf32_Addr* addr);

#endif
