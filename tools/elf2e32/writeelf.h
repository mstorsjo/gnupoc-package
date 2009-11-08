/*
    Elf2e32
    Copyright 2007 - 2009 Martin Storsjö

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

    Martin Storsjö
    martin@martin.st
*/

#ifndef __WRITEELF_H
#define __WRITEELF_H

#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#define R_ARM_NONE	0
#define R_ARM_ABS32	2
#define R_ARM_GLOB_DAT	21
#define R_ARM_RELATIVE	23

template<class T, Elf_Type TRANS> class StructArray {
public:
	StructArray() {
		bufferSize = 10240;
		buffer = (uint8_t*) malloc(bufferSize);
		used = 0;
		section = NULL;
	}
	~StructArray() {
		free(buffer);
	}
	T* objectAtIndex(uint32_t index) {
		if (index < used)
			return &((T*)buffer)[index];
		return NULL;
	}
	T* appendObject() {
		used++;
		if (sizeof(T)*used > bufferSize) {
			while (sizeof(T)*used > bufferSize)
				bufferSize *= 2;
			buffer = (uint8_t*) realloc(buffer, bufferSize);
		}
		return objectAtIndex(used - 1);
	}
	void appendObject(const T& obj) {
		T* ptr = appendObject();
		*ptr = obj;
	}
	uint32_t usedSize() {
		return sizeof(T)*used;
	}
	uint8_t* getBuffer() {
		return buffer;
	}
	Elf_Scn* createSection(Elf* elf) {
		section = elf_newscn(elf);
		elf_newdata(section);
		return section;
	}
	Elf_Scn* getSection() {
		return section;
	}
	uint32_t sectionIndex() {
		return elf_ndxscn(section);
	}
	Elf32_Shdr* getShdr() {
		return elf32_getshdr(section);
	}
	void update(int align = 4) {
		Elf_Data* data = elf_getdata(section, NULL);
		data->d_align = align;
		data->d_off = 0;
		data->d_buf = buffer;
		data->d_type = TRANS;
		data->d_size = usedSize();
		data->d_version = EV_CURRENT;
	}
protected:
	uint8_t* buffer;
	uint32_t bufferSize;
	uint32_t used;
	Elf_Scn* section;
};
class StringSection : public StructArray<uint8_t, ELF_T_BYTE> {
public:
	void appendString(const char* str) {
		int len = strlen(str);
		if (used + len > bufferSize) {
			while (used + len > bufferSize)
				bufferSize *= 2;
			buffer = (uint8_t*) realloc(buffer, bufferSize);
		}
		memcpy(buffer + used, str, len);
		used += len;
	}
};

typedef StructArray<Elf32_Dyn, ELF_T_DYN> DynArray;
typedef StructArray<Elf32_Word, ELF_T_WORD> OrdinalArray;
typedef StructArray<Elf32_Sym, ELF_T_SYM> SymbolArray;
typedef StructArray<uint8_t, ELF_T_VDEF> VerdefArray;
typedef StructArray<Elf32_Half, ELF_T_HALF> VersymArray;
typedef StructArray<Elf32_Word, ELF_T_WORD> HashArray;
typedef StructArray<Elf32_Word, ELF_T_WORD> CodeSection;
typedef StructArray<Elf32_Rel, ELF_T_REL> RelocationSection;

class StringTable {
public:
	StringTable() {
		bufferSize = 10240;
		buffer = (uint8_t*) malloc(bufferSize);
		used = 0;
		section = NULL;
		appendString("");
	}
	~StringTable() {
		free(buffer);
	}
	uint32_t appendString(const char* str) {
		int len = strlen(str);
		len++;
		uint32_t pos = used;
		if (used + len < bufferSize) {
			while (used + len > bufferSize)
				bufferSize *= 2;
			buffer = (uint8_t*) realloc(buffer, bufferSize);
		}
		strcpy((char*)buffer + used, str);
		used += len;
		return pos;
	}
	uint32_t usedSize() {
		return used;
	}
	uint8_t* getBuffer() {
		return buffer;
	}
	Elf_Scn* createSection(Elf* elf) {
		section = elf_newscn(elf);
		elf_newdata(section);
		getShdr()->sh_entsize = 1;
		getShdr()->sh_addralign = 0;
		return section;
	}
	Elf_Scn* getSection() {
		return section;
	}
	uint32_t sectionIndex() {
		return elf_ndxscn(section);
	}
	Elf32_Shdr* getShdr() {
		return elf32_getshdr(section);
	}
	void update(int align = 0) {
		Elf_Data* data = elf_getdata(section, NULL);
		data->d_align = align;
		data->d_off = 0;
		data->d_buf = buffer;
		data->d_type = ELF_T_BYTE;
		data->d_size = usedSize();
		data->d_version = EV_CURRENT;
	}
private:
	uint8_t* buffer;
	uint32_t bufferSize;
	uint32_t used;
	Elf_Scn* section;
};

#endif
