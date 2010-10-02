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

#include "elfutils.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

bool getLibraryName(Elf_Data* verneed, Elf32_Word verneedNum, Elf32_Half index, Elf32_Verneed** needPtr, Elf32_Vernaux** auxPtr) {
	Elf32_Verneed* need = (Elf32_Verneed*) verneed->d_buf;
	Elf32_Verneed* end = (Elf32_Verneed*) ((uint8_t*)verneed->d_buf + verneed->d_size);
	for (uint32_t i = 0; i < verneedNum && need < end; i++) {
		Elf32_Vernaux* aux = (Elf32_Vernaux*) (((uint8_t*)need) + need->vn_aux);
		if (aux->vna_other == index) {
			*needPtr = need;
			*auxPtr = aux;
			return true;
		}
		need = (Elf32_Verneed*) (((uint8_t*)need) + need->vn_next);
	}
	return false;
}

const char* getDsoName(Elf* elf, size_t stringSection, Elf_Data* verneed, Elf32_Word verneedNum, Elf32_Half index) {
	Elf32_Verneed* need;
	Elf32_Vernaux* aux;
	if (!getLibraryName(verneed, verneedNum, index, &need, &aux))
		return NULL;
	return elf_strptr(elf, stringSection, need->vn_file);
}

const char* getDllName(Elf* elf, size_t stringSection, Elf_Data* verneed, Elf32_Word verneedNum, Elf32_Half index) {
	Elf32_Verneed* need;
	Elf32_Vernaux* aux;
	if (!getLibraryName(verneed, verneedNum, index, &need, &aux))
		return NULL;
	return elf_strptr(elf, stringSection, aux->vna_name);
}

bool findSymbol(Elf* elf, Elf_Scn* section, Elf32_Shdr* shdr, const char* symbolName, Elf32_Sym** symPtr) {
	Elf_Data* data = NULL;

	if ((data = elf_getdata(section, data)) == 0 || data->d_size == 0) {
		fprintf(stderr, "Symbol table has no data");
		return false;
	}

	Elf32_Sym* sym = (Elf32_Sym*) data->d_buf;
	Elf32_Sym* end = (Elf32_Sym*) ((uint8_t*)data->d_buf + data->d_size);
	for (; sym < end; sym++) {
		const char* name = elf_strptr(elf, shdr->sh_link, sym->st_name);
		if (!name) {
			fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
			continue;
		}
		if (!strcmp(name, symbolName)) {
			*symPtr = sym;
			return true;
		}
	}
	return false;
}

bool findSection(Elf* elf, Elf32_Word type, Elf_Scn** sectionPtr, Elf32_Shdr** shdrPtr) {
	Elf_Scn* section = NULL;
	while ((section = elf_nextscn(elf, section)) != NULL) {
		Elf32_Shdr* shdr;
		if ((shdr = elf32_getshdr(section)) != NULL) {
			if (shdr->sh_type == type) {
				*sectionPtr = section;
				*shdrPtr = shdr;
				return true;
			}
		}
	}
	return false;
}

bool getDynamicValue(Elf* elf, Elf_Scn* section, Elf32_Shdr* shdr, Elf32_Sword tag, Elf32_Word* value) {
	Elf_Data* data = NULL;

	if ((data = elf_getdata(section, data)) == 0 || data->d_size == 0) {
		fprintf(stderr, "Dynamic table has no data");
		return false;
	}

	Elf32_Dyn* dyn = (Elf32_Dyn*) data->d_buf;
	Elf32_Dyn* end = (Elf32_Dyn*) ((uint8_t*)data->d_buf + data->d_size);
	for (; dyn < end; dyn++) {
		if (dyn->d_tag == tag) {
			*value = dyn->d_un.d_val;
			return true;
		}
	}
	return false;
}

bool getDynamicValue(Elf* elf, Elf32_Sword tag, Elf32_Word* value) {	
	Elf_Scn* section;
	Elf32_Shdr* shdr;
	if (!findSection(elf, SHT_DYNAMIC, &section, &shdr))
		return false;
	return getDynamicValue(elf, section, shdr, tag, value);
}

bool findSymbol(Elf* elf, const char* symbol, Elf32_Addr* addr) {
	Elf_Scn* section;
	Elf32_Shdr* shdr;
	if (!findSection(elf, SHT_SYMTAB, &section, &shdr))
		return false;
	Elf32_Sym* sym;
	if (!findSymbol(elf, section, shdr, symbol, &sym))
		return false;
	*addr = sym->st_value;
	return true;
}

