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
#include "elfutils.h"

class SectionWriter {
protected:
	SectionWriter() {
		bufferSize = 10240;
		buffer = (uint8_t*) malloc(bufferSize);
		used = 0;
		section = NULL;
		elementSize = 1;
	}
	void reserveSpace(int elements) {
		if (elementSize*(used + elements) > bufferSize) {
			while (elementSize*(used + elements) > bufferSize)
				bufferSize *= 2;
			buffer = (uint8_t*) realloc(buffer, bufferSize);
		}
	}
public:
	~SectionWriter() {
		free(buffer);
	}
	uint32_t usedSize() {
		return elementSize*used;
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
	void flagDirty() {
		elf_flagscn(section, ELF_C_SET, ELF_F_DIRTY);
	}

protected:
	uint8_t* buffer;
	uint32_t bufferSize;
	uint32_t used;
	uint32_t elementSize;
	Elf_Scn* section;
};

template<class T, Elf_Type TRANS> class StructArray : public SectionWriter {
public:
	StructArray() {
		elementSize = sizeof(T);
	}
	T* objectAtIndex(uint32_t index) {
		if (index < used)
			return &((T*)buffer)[index];
		return NULL;
	}
	T* appendObject() {
		reserveSpace(1);
		used++;
		return objectAtIndex(used - 1);
	}
	T* appendClearedObject() {
		T* ptr = appendObject();
		memset(ptr, 0, sizeof(T));
		return ptr;
	}
	void appendObject(const T& obj) {
		T* ptr = appendObject();
		*ptr = obj;
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
};
class StringSection : public StructArray<uint8_t, ELF_T_BYTE> {
public:
	void appendString(const char* str) {
		int len = strlen(str);
		reserveSpace(len);
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

class StringTable : public SectionWriter {
public:
	StringTable() {
		elementSize = 1;
		appendString("");
	}
	uint32_t appendString(const char* str) {
		int len = strlen(str);
		len++;
		uint32_t pos = used;
		reserveSpace(len);
		strcpy((char*)buffer + used, str);
		used += len;
		return pos;
	}
	Elf_Scn* createSection(Elf* elf) {
		SectionWriter::createSection(elf);
		getShdr()->sh_entsize = 1;
		getShdr()->sh_addralign = 0;
		return section;
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
};

#endif
