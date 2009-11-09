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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include <zlib.h>
#include <fstream>
#include "deflate.h"
#include "writeelf.h"
#include "caseinsensitive.h"
#include "epocversion.h"
#include "e32image.h"
#include "elfutils.h"

#include <sys/types.h>

using namespace std;
void DeflateCompress(char* bytes, TInt size, ostream& os);



#ifndef EF_ARM_INTERWORK
#define EF_ARM_INTERWORK 0x04
#endif

#ifdef ELF_HASH_UNSIGNED
typedef unsigned char* elf_string;
#else
typedef char* elf_string;
#endif





struct RelocSections {
	Elf32_Shdr* dynsymHeader;
	Elf_Scn* dynsymSection;
	Elf32_Shdr* symverHeader;
	Elf_Scn* symverSection;
	Elf32_Shdr* verneedHeader;
	Elf_Scn* verneedSection;
	Elf32_Word verneedNum;
};

void fixRelocation(uint32_t offset, FILE* out, const char* symbol, const char* lib, const char* libpath) {
	char buffer[1000];
	sprintf(buffer, "%s/%s", libpath, lib);
	int fd = open(buffer, O_RDONLY);
	if (fd < 0) {
		// file not found, try to lowercase the lib filename
		sprintf(buffer, "%s/", libpath);
		const char* inptr = lib;
		char* outptr = buffer + strlen(buffer);
		while (*inptr)
			*outptr++ = tolower(*inptr++);
		*outptr++ = '\0';
		fd = open(buffer, O_RDONLY);
	}
	if (fd < 0) {
		// file still not found, do a case insensitive search
		if (findCaseInsensitive(buffer))
			fd = open(buffer, O_RDONLY);
	}
	if (fd < 0) {
		printf("file not found %s\n", lib);
		return;
	}
	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);

	Elf_Scn* section = NULL;
	Elf32_Shdr* shdr = NULL;
	if (!findSection(elf, SHT_DYNSYM, &section, &shdr)) {
		printf("fixRelocation SHT_DYNSYM not found\n");
		elf_end(elf);
		close(fd);
		return;
	}
	Elf32_Sym* sym;
	Elf32_Addr addr;
	if (!findSymbol(elf, section, shdr, symbol, &sym)) {
		printf("fixRelocation symbol %s not found\n", symbol);
		elf_end(elf);
		close(fd);
		return;
	}
	addr = sym->st_value;

	if (!findSection(elf, SHT_PROGBITS, &section, &shdr)) {
		printf("fixRelocation section SHT_PROGBITS not found\n");
		elf_end(elf);
		close(fd);
		return;
	}

	addr -= shdr->sh_addr;

	Elf_Data* data = NULL;
	data = elf_rawdata(section, data);
	if (addr >= data->d_size) {
		printf("fixRelocation addr past d_size\n");
		elf_end(elf);
		close(fd);
		return;
	}
//	uint32_t ordinal = *((uint32_t*) (((uint8_t*)data->d_buf) + addr));
	uint8_t* dataptr = (uint8_t*) data->d_buf;
	dataptr += addr;
	uint32_t ordinal = dataptr[0] | (dataptr[1]<<8) | (dataptr[2]<<16) | (dataptr[3]<<24);
	fseek(out, offset, SEEK_SET);
	uint8_t origBuf[4];
	fread(origBuf, 1, sizeof(origBuf), out);
	uint32_t orig = origBuf[0] | (origBuf[1]<<8) | (origBuf[2]<<16) | (origBuf[3]<<24);
	ordinal += orig << 16;
	fseek(out, offset, SEEK_SET);
//	fwrite(&ordinal, 1, sizeof(ordinal), out);
	writeUint32(ordinal, out);
	elf_end(elf);
	close(fd);
}

void fixRelocation(uint32_t offset, FILE* out, Elf32_Addr value) {
	fseek(out, offset, SEEK_SET);
	uint8_t origBuf[4];
	fread(origBuf, 1, sizeof(origBuf), out);
	uint32_t orig = origBuf[0] | (origBuf[1]<<8) | (origBuf[2]<<16) | (origBuf[3]<<24);
	value += orig;
	fseek(out, offset, SEEK_SET);
	writeUint32(value, out);
}

#include <vector>
#include <algorithm>
#include <map>

using std::vector;
using std::map;

class ImportList {
public:
	class Library {
	public:
		Library(const char* name) {
			this->name = strdup(name);
		}
		~Library() {
			free(name);
		}
		void list() {
			printf("%s:\n", name);
			for (unsigned int i = 0; i < addresses.size(); i++)
				printf("\t%d\n", addresses[i]);
		}
		char* name;
		vector<uint32_t> addresses;
		uint32_t offset;
		static bool compare(const Library* lib1, const Library* lib2) {
			return strcmp(lib1->name, lib2->name) < 0;
		}
	};

	~ImportList() {
		for (unsigned int i = 0; i < libraries.size(); i++)
			delete libraries[i];
		libraries.clear();
	}
	Library* findLibrary(const char* name) {
		for (unsigned int i = 0; i < libraries.size(); i++) {
			if (!strcmp(libraries[i]->name, name))
				return libraries[i];
		}
		Library* lib = new Library(name);
		libraries.push_back(lib);
		return lib;
	}
	void addImport(const char* libname, uint32_t addr) {
		Library* lib = findLibrary(libname);
		lib->addresses.push_back(addr);
	}
	void listImports() {
		for (unsigned int i = 0; i < libraries.size(); i++)
			libraries[i]->list();
	}
	void write(FILE* out) {
		sort(libraries.begin(), libraries.end(), Library::compare);
		uint32_t size = 0;
		size += 4;
		for (unsigned int i = 0; i < libraries.size(); i++) {
			size += 8;
			size += libraries[i]->addresses.size()*4;
//			printf("%d imports for lib %s\n", libraries[i]->addresses.size(), libraries[i]->name);
		}
		for (unsigned int i = 0; i < libraries.size(); i++) {
			libraries[i]->offset = size;
			size += strlen(libraries[i]->name) + 1;
		}
		uint32_t padding = (size & 3) ? (4 - (size & 3)) : 0;
		size += padding;
		writeUint32(size, out);
		for (unsigned int i = 0; i < libraries.size(); i++) {
			writeUint32(libraries[i]->offset, out);
			writeUint32(libraries[i]->addresses.size(), out);
			for (unsigned int j = 0; j < libraries[i]->addresses.size(); j++)
				writeUint32(libraries[i]->addresses[j], out);
		}
		for (unsigned int i = 0; i < libraries.size(); i++)
			fwrite(libraries[i]->name, 1, strlen(libraries[i]->name)+1, out);
		for (unsigned int i = 0; i < padding; i++)
			writeUint8(0, out);
	}
	int numLibraries() {
		return libraries.size();
	}

private:
	vector<Library*> libraries;
};

class RelocationList {
public:
	class Relocation {
	public:
		Relocation(uint16_t off, uint32_t val) {
			offset = off;
			value = val;
		}
		uint16_t offset;
		uint32_t value;
		bool operator<(const Relocation& reloc) const {
			return offset < reloc.offset;
		}
	};
	void addRelocation(uint32_t offset, uint32_t value) {
//		relocations.push_back(Relocation(offset, value));
		uint32_t prefix = offset & 0xfffff000;
		sublists[prefix].push_back(Relocation(offset & 0xfff, value));
	}
	uint32_t sublistSize(uint32_t prefix, uint32_t* padding = NULL) {
		const vector<Relocation>& relocations = sublists[prefix];
		uint32_t relocSize = 8 + 2*relocations.size();
		uint32_t relocPadding = (relocSize & 3) ? (4 - (relocSize & 3)) : 0;
		relocSize += relocPadding;
		if (padding)
			*padding = relocPadding;
		return relocSize;
	}
	void writeSublist(uint32_t prefix, FILE* out, E32ImageHeader* header) {
		vector<Relocation>& relocations = sublists[prefix];
		sort(relocations.begin(), relocations.end());
		uint32_t relocPadding;
		uint32_t relocSize = sublistSize(prefix, &relocPadding);
		writeUint32(prefix, out);
		writeUint32(relocSize, out);
		for (unsigned int i = 0; i < relocations.size(); i++) {
			uint16_t reloc = relocations[i].offset;
			uint32_t addr = relocations[i].value;
			if (addr >= header->codeBase && addr <= header->codeBase + header->codeSize) {
				reloc |= KTextRelocType;
			} else if (addr >= header->dataBase && addr <= header->dataBase + header->dataSize + header->bssSize) {
				reloc |= KDataRelocType;
			}
			writeUint16(reloc, out);
		}
		for (unsigned int i = 0; i < relocPadding; i++)
			writeUint8(0, out);
	}
	void write(FILE* out, E32ImageHeader* header) {
		uint32_t size = 0;
		uint32_t num = count();
		for (map<uint32_t, vector<Relocation> >::iterator it = sublists.begin(); it != sublists.end(); it++)
			size += sublistSize(it->first);
		writeUint32(size, out);
		writeUint32(num, out);
		for (map<uint32_t, vector<Relocation> >::iterator it = sublists.begin(); it != sublists.end(); it++)
			writeSublist(it->first, out, header);
	}
	int count() {
		uint32_t num = 0;
		for (map<uint32_t, vector<Relocation> >::iterator it = sublists.begin(); it != sublists.end(); it++)
			num += it->second.size();
		return num;
	}
private:
//	vector<Relocation> relocations;
	map<uint32_t, vector<Relocation> > sublists;
};

class ExportList {
public:
	class Export {
	public:
		Export(const char* name, uint32_t value, bool code = true, int size = 0) {
			this->name = NULL;
			if (name)
				this->name = strdup(name);
			address = value;
			this->code = code;
			this->size = size;
		}
		void setName(const char* name) {
			free(this->name);
			this->name = NULL;
			if (name)
				this->name = strdup(name);
		}
		~Export() {
			free(name);
		}
		char* name;
		uint32_t address;
		bool code;
		int size;
		static bool compare(const Export* exp1, const Export* exp2) {
			if (!exp1->name && !exp2->name)
				return 0;
			if (!exp1->name)
				return -1;
			if (!exp2->name)
				return 1;
			return strcmp(exp1->name, exp2->name) < 0;
		}
	};

	ExportList() {
		presetOrdinals = 0;
	}
	~ExportList() {
		clear();
	}
	void clear() {
		for (unsigned int i = 0; i < exports.size(); i++)
			delete exports[i];
		exports.clear();
	}
	void addExportOrdinal(const char* name, uint32_t ordinal) {
		while (exports.size() < ordinal)
			exports.push_back(new Export(NULL, 0));
		if (presetOrdinals < ordinal)
			presetOrdinals = ordinal;
		exports[ordinal - 1]->setName(name);
	}
	void addExport(const char* name, uint32_t addr, bool code = true, int size = 0) {
		for (unsigned int i = 0; i < exports.size(); i++) {
			if (exports[i]->name && !strcmp(exports[i]->name, name)) {
				exports[i]->address = addr;
				exports[i]->code = code;
				exports[i]->size = size;
				return;
			}
		}
		Export* exp = new Export(name, addr, code, size);
		exports.push_back(exp);
	}
	void doSort() {
		sort(exports.begin() + presetOrdinals, exports.end(), Export::compare);
	}
	void write(FILE* out, E32ImageHeader* header, RelocationList* relocations) {
		writeUint32(exports.size(), out);
		for (unsigned int i = 0; i < exports.size(); i++) {
			uint32_t addr = ftell(out) - header->codeOffset;
			writeUint32(exports[i]->address, out);
			relocations->addRelocation(addr, exports[i]->address);
		}
	}
	int numExports() {
		return exports.size();
	}
	void writeDef(const char* filename) {
		FILE* out = fopen(filename, "w");
		fprintf(out, "EXPORTS\r\n");
		for (unsigned int i = 0; i < exports.size(); i++) {
			if (i == presetOrdinals)
				fprintf(out, "; NEW:\r\n");
			if (epocVersion <= EPOC_VERSION_9_1) { // S60 3.0 style
				fprintf(out, "\t%s @ %d NONAME ; %s\r\n", exports[i]->name, i+1, exports[i]->code ? "CODE" : "DATA");
			} else { // S60 3.1 and onwards
				if (exports[i]->code)
					fprintf(out, "\t%s @ %d NONAME\r\n", exports[i]->name, i+1);
				else
					fprintf(out, "\t%s @ %d NONAME DATA %d\r\n", exports[i]->name, i+1, exports[i]->size);
			}
		}
		fprintf(out, "\r\n");
		fclose(out);
	}


	void writeDso(const char* filename, const char* soname) {
		const char* sep = strrchr(filename, '/');
		const char* basename = filename;
		if (sep)
			basename = sep + 1;
		int fd = open(filename, O_RDWR | O_TRUNC | O_CREAT, 0666);
		Elf* elf = elf_begin(fd, ELF_C_WRITE, NULL);
		Elf32_Ehdr* ehdr = elf32_newehdr(elf);
		uint8_t ident[] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, ELFCLASS32, ELFDATA2LSB, EV_CURRENT, ELFOSABI_NONE, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		memcpy(ehdr->e_ident, ident, sizeof(ident));
		ehdr->e_type = ET_DYN;
		ehdr->e_machine = EM_ARM;
		ehdr->e_version = EV_CURRENT;
		ehdr->e_entry = 0;
		ehdr->e_flags = EF_ARM_INTERWORK | 0x4000000;
		Elf32_Phdr* phdr = elf32_newphdr(elf, 2);
		Elf32_Phdr* phdrOrig = phdr;
/*
		phdr->p_type = PT_LOAD;
		phdr->p_offset = 0x19c;
		phdr->p_vaddr = 0;
		phdr->p_paddr = 0;
		phdr->p_filesz = 0x6c;
		phdr->p_memsz = 0x6c;
		phdr->p_flags = PF_X;
		phdr->p_align = 0x4;
		phdr++;
		phdr->p_type = PT_DYNAMIC;
		phdr->p_offset = 0x208;
		phdr->p_vaddr = 0;
		phdr->p_paddr = 0;
		phdr->p_filesz = 0x50;
		phdr->p_memsz = 0;
		phdr->p_flags = PF_R;
		phdr->p_align = 0x4;
*/
		OrdinalArray ordinalArray;
		DynArray dynArray;
		HashArray hashArray;
		VerdefArray verdefArray;
		VersymArray versymArray;
		StringTable strtab;
		SymbolArray symtab;
		StringTable shstrtab;

		int prime = exports.size()/2;
		if (prime == 0)
			prime = 1;
		hashArray.appendObject(prime);
		hashArray.appendObject(exports.size()+1);
		for (int i = 0; i < prime; i++)
			hashArray.appendObject(0);
		for (unsigned int i = 0; i < exports.size()+1; i++)
			hashArray.appendObject(0);
		uint32_t* hashTable = hashArray.objectAtIndex(2);
		uint32_t* hashChain = hashArray.objectAtIndex(2 + prime);

		Elf32_Sym* firstSym = symtab.appendObject();
		firstSym->st_name = 0;
		firstSym->st_value = 0;
		firstSym->st_size = 0;
		firstSym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE);
		firstSym->st_shndx = 0;

		versymArray.appendObject(0);
		for (unsigned int i = 0; i < exports.size(); i++) {
			ordinalArray.appendObject(i+1);
			Elf32_Sym* sym = symtab.appendObject();
			sym->st_name = strtab.appendString(exports[i]->name);
			sym->st_value = 4*i;
			sym->st_size = 4;
			sym->st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
			sym->st_shndx = 1;
			uint32_t hash = elf_hash((const elf_string) exports[i]->name) % prime;
			hashChain[i+1] = hashTable[hash];
			hashTable[hash] = i+1;
			versymArray.appendObject(2);
		}
		ordinalArray.appendObject(0);

		uint32_t filenameIndex = strtab.appendString(basename);
		uint32_t sonameIndex = strtab.appendString(soname);

		for (unsigned int i = 0; i < 2*(sizeof(Elf32_Verdef)+sizeof(Elf32_Verdaux)); i++)
			verdefArray.appendObject(0);
		Elf32_Verdef* def = (Elf32_Verdef*) verdefArray.objectAtIndex(0);
		def->vd_version = VER_DEF_CURRENT;
		def->vd_flags = VER_FLG_BASE;
		def->vd_ndx = VER_NDX_GLOBAL;
		def->vd_cnt = 1;
		def->vd_hash = elf_hash((const elf_string) basename);
		def->vd_aux = sizeof(Elf32_Verdef);
		def->vd_next = sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux);
		Elf32_Verdaux* daux = (Elf32_Verdaux*) verdefArray.objectAtIndex(def->vd_aux);
		daux->vda_name = filenameIndex;
		daux->vda_next = 0;
		def = (Elf32_Verdef*) verdefArray.objectAtIndex(def->vd_next);
		def->vd_version = VER_DEF_CURRENT;
		def->vd_flags = 0;
		def->vd_ndx = 2;
		def->vd_cnt = 1;
		def->vd_hash = elf_hash((const elf_string) soname);
		def->vd_aux = sizeof(Elf32_Verdef);
		def->vd_next = 0;
		daux = (Elf32_Verdaux*) verdefArray.objectAtIndex(2*sizeof(Elf32_Verdef) + sizeof(Elf32_Verdaux));
		daux->vda_name = sonameIndex;
		daux->vda_next = 0;

		Elf32_Dyn* dyn;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_SONAME;
		dyn->d_un.d_val = filenameIndex;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_SYMTAB;
		dyn->d_un.d_val = 0;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_SYMENT;
		dyn->d_un.d_val = 0;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_STRTAB;
		dyn->d_un.d_val = 0;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_STRSZ;
		dyn->d_un.d_val = 0;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_VERSYM;
		dyn->d_un.d_val = 0;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_VERDEF;
		dyn->d_un.d_val = 0;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_VERDEFNUM;
		dyn->d_un.d_val = 2;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_HASH;
		dyn->d_un.d_val = 0;
		dyn = dynArray.appendObject();
		dyn->d_tag = DT_NULL;
		dyn->d_un.d_val = 0;

		ordinalArray.createSection(elf);
		ordinalArray.getShdr()->sh_name = shstrtab.appendString("ER_RO");
		ordinalArray.getShdr()->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
		ordinalArray.getShdr()->sh_type = SHT_PROGBITS;
		ordinalArray.getShdr()->sh_addralign = 4;

		dynArray.createSection(elf);
		dynArray.getShdr()->sh_name = shstrtab.appendString(".dynamic");
		dynArray.getShdr()->sh_type = SHT_DYNAMIC;
		dynArray.getShdr()->sh_addralign = 4;

		hashArray.createSection(elf);
		hashArray.getShdr()->sh_name = shstrtab.appendString(".hash");
		hashArray.getShdr()->sh_type = SHT_HASH;
		hashArray.getShdr()->sh_addralign = 4;
		hashArray.getShdr()->sh_entsize = 0;

		verdefArray.createSection(elf);
		verdefArray.getShdr()->sh_name = shstrtab.appendString(".version_d");
		verdefArray.getShdr()->sh_type = SHT_GNU_verdef;
		verdefArray.getShdr()->sh_addralign = 4;
		verdefArray.getShdr()->sh_entsize = 8;
		verdefArray.getShdr()->sh_info = 2;

		versymArray.createSection(elf);
		versymArray.getShdr()->sh_name = shstrtab.appendString(".version");
		versymArray.getShdr()->sh_type = SHT_GNU_versym;
		versymArray.getShdr()->sh_addralign = 2;
		versymArray.getShdr()->sh_entsize = 2;

		strtab.createSection(elf);
		strtab.getShdr()->sh_name = shstrtab.appendString(".strtab");
		strtab.getShdr()->sh_type = SHT_STRTAB;


		symtab.createSection(elf);
		symtab.getShdr()->sh_name = shstrtab.appendString(".dynsym");
		symtab.getShdr()->sh_type = SHT_DYNSYM;
		symtab.getShdr()->sh_addralign = 4;
		symtab.getShdr()->sh_info = 1;

		shstrtab.createSection(elf);
		shstrtab.getShdr()->sh_name = shstrtab.appendString(".shstrtab");
		shstrtab.getShdr()->sh_type = SHT_STRTAB;

		ehdr->e_shstrndx = shstrtab.sectionIndex();
		dynArray.getShdr()->sh_link = strtab.sectionIndex();
		symtab.getShdr()->sh_link = strtab.sectionIndex();
		hashArray.getShdr()->sh_link = symtab.sectionIndex();
		verdefArray.getShdr()->sh_link = strtab.sectionIndex();
		versymArray.getShdr()->sh_link = symtab.sectionIndex();


		strtab.appendString("");
		strtab.getBuffer()[strtab.usedSize()-1] = ' ';
		ordinalArray.update();
		dynArray.update();
		hashArray.update();
		verdefArray.update();
		versymArray.update();
		strtab.update();
		symtab.update();
		shstrtab.update();

		elf_update(elf, ELF_C_NULL);

		phdr = phdrOrig;
		phdr->p_type = PT_LOAD;
		phdr->p_offset = ordinalArray.getShdr()->sh_offset;
		phdr->p_vaddr = 0;
		phdr->p_paddr = 0;
		phdr->p_filesz = ordinalArray.getShdr()->sh_size;
		phdr->p_memsz = ordinalArray.getShdr()->sh_size;
		phdr->p_flags = PF_X;
		phdr->p_align = 0x4;
		phdr++;
		phdr->p_type = PT_DYNAMIC;
		phdr->p_offset = dynArray.getShdr()->sh_offset;
		phdr->p_vaddr = 0;
		phdr->p_paddr = 0;
		phdr->p_filesz = dynArray.getShdr()->sh_size;
		phdr->p_memsz = 0;
		phdr->p_flags = PF_R;
		phdr->p_align = 0x4;

		dynArray.objectAtIndex(1)->d_un.d_val = symtab.getShdr()->sh_offset;
		dynArray.objectAtIndex(2)->d_un.d_val = 16;
		dynArray.objectAtIndex(3)->d_un.d_val = strtab.getShdr()->sh_offset;
		dynArray.objectAtIndex(4)->d_un.d_val = strtab.getShdr()->sh_size;
		dynArray.objectAtIndex(5)->d_un.d_val = versymArray.getShdr()->sh_offset;
		dynArray.objectAtIndex(6)->d_un.d_val = verdefArray.getShdr()->sh_offset;
		dynArray.objectAtIndex(8)->d_un.d_val = hashArray.getShdr()->sh_offset;

		elf_update(elf, ELF_C_WRITE);

		elf_end(elf);
//		fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
		close(fd);
	}

private:
	vector<Export*> exports;
	uint32_t presetOrdinals;
};


ImportList importList;
RelocationList relocationList;
RelocationList dataRelocationList;
ExportList exportList;

void checkRelocations(Elf* elf, Elf_Scn* relocationSection, Elf32_Shdr* relocationHeader, RelocSections* sections, E32ImageHeader* header, FILE* out, const char* libpath) {
	Elf_Data* relocations = NULL;
	Elf_Data* dynsyms = NULL;
	Elf_Data* symver = NULL;
	Elf_Data* verneed = NULL;

	relocations = elf_getdata(relocationSection, relocations);
	dynsyms = elf_getdata(sections->dynsymSection, dynsyms);
	symver = elf_getdata(sections->symverSection, symver);
	verneed = elf_getdata(sections->verneedSection, verneed);

	Elf32_Rel* rel = (Elf32_Rel*) relocations->d_buf;
	Elf32_Rel* end = (Elf32_Rel*) ((uint8_t*)relocations->d_buf + relocations->d_size);
	Elf32_Sym* symArray = (Elf32_Sym*) dynsyms->d_buf;
	Elf32_Sym* endSym = (Elf32_Sym*) ((uint8_t*)dynsyms->d_buf + dynsyms->d_size);
	Elf32_Half* ver = (Elf32_Half*) symver->d_buf;
	for (; rel < end; rel++) {
//		const char* name = elf_strptr(elf, sections->dynsymHeader->sh_link, ELF32_R_SYM(rel->r_info));
		int type = ELF32_R_TYPE(rel->r_info);
		int index = ELF32_R_SYM(rel->r_info);
		Elf32_Sym* sym = &symArray[index];
		Elf32_Half verIndex = ver[index];
		if (sym >= symArray && sym < endSym) {
			const char* name = elf_strptr(elf, sections->dynsymHeader->sh_link, symArray[index].st_name);
			if (sym->st_value) {
				Elf32_Addr value = sym->st_value;
//				if (!strcmp(name, "Image$$ER_RO$$Base"))
//					value |= 1;
				if (rel->r_offset >= header->codeBase && rel->r_offset < header->codeBase + header->codeSize) {
//					if (type == R_ARM_ABS32 || type == R_ARM_GLOB_DAT)
					if (type != R_ARM_RELATIVE)
						fixRelocation(rel->r_offset - header->codeBase + header->codeOffset, out, value);
					relocationList.addRelocation(rel->r_offset - header->codeBase, sym->st_value);
				} else if (rel->r_offset >= header->dataBase && rel->r_offset < header->dataBase + header->dataSize + header->bssSize) {
//					if (type == R_ARM_ABS32)
					if (type != R_ARM_RELATIVE)
						fixRelocation(rel->r_offset - header->dataBase + header->dataOffset, out, value);
					dataRelocationList.addRelocation(rel->r_offset - header->dataBase, sym->st_value);
				} else {
					printf("unhandled relocation of symbol %s\n", name);
				}
			} else {
				const char* dsoname = getDsoName(elf, sections->verneedHeader->sh_link, verneed, sections->verneedNum, verIndex);
				const char* dllname = getDllName(elf, sections->verneedHeader->sh_link, verneed, sections->verneedNum, verIndex);
				if (type != R_ARM_NONE) {
					fixRelocation(rel->r_offset - header->codeBase + header->codeOffset, out, name, dsoname, libpath);
					importList.addImport(dllname, rel->r_offset - header->codeBase);
				}
			}
		}
	}
}

void findExports(Elf* elf, ExportList* exportList) {
	Elf_Scn* section;
	Elf32_Shdr* shdr;
	if (!findSection(elf, SHT_DYNSYM, &section, &shdr))
		return;

	Elf_Data* data = NULL;

	if ((data = elf_getdata(section, data)) == 0 || data->d_size == 0) {
		fprintf(stderr, "Symbol table has no data");
		return;
	}

	Elf32_Sym* sym = (Elf32_Sym*) data->d_buf;
	Elf32_Sym* end = (Elf32_Sym*) ((uint8_t*)data->d_buf + data->d_size);
	int sum = 0;
	for (; sym < end; sym++) {
		const char* name = elf_strptr(elf, shdr->sh_link, sym->st_name);
		if (!name) {
			fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
			continue;
		}
		uint8_t bind = ELF32_ST_BIND(sym->st_info);
		uint8_t type = ELF32_ST_TYPE(sym->st_info);
		if (bind == STB_GLOBAL && (type == STT_FUNC || type == STT_OBJECT) && sym->st_value) {
			if (!strncmp(name, "_ZTS", 4))
				continue;
			exportList->addExport(name, sym->st_value, type == STT_FUNC, sym->st_size);
//			printf("%s: %x %d %x %x %x\n", name, sym->st_value, sym->st_size, sym->st_info, sym->st_other, sym->st_shndx);
			sum++;
		}
	}
//	printf("%d symbols\n", sum);
}

void parseSysDef(char* str, ExportList* exportList) {
	char* ptr = str;
	char* token = NULL;
	while ((token = strtok(ptr, ";")) != NULL) {
		ptr = NULL;
		char* name = token;
		char* delim = strchr(name, ',');
		if (!delim)
			continue;
		*delim = '\0';
		uint32_t ordinal = atoi(delim + 1);
		exportList->addExportOrdinal(name, ordinal);
	}
}

void parseDefFile(const char* filename, ExportList* exportList) {
	FILE* in = fopen(filename, "r");
	char line[500];
	char *ptr;
	bool header = false;
	while ((ptr = fgets(line, sizeof(line), in)) != NULL) {
		int len = strlen(ptr);
		if (ptr[len-1] == '\r' || ptr[len-1] == '\n') ptr[--len] = '\0';
		if (ptr[len-1] == '\r' || ptr[len-1] == '\n') ptr[--len] = '\0';
		if (!header) {
			if (strcmp(ptr, "EXPORTS")) {
				fprintf(stderr, "Def file %s has bad header\n", filename);
				fclose(in);
				return;
			} else {
				header = true;
			}
		} else {
			char* comments = strchr(ptr, ';');
			if (comments)
				*comments = '\0';
			char name[500];
			int ordinal;
			if (sscanf(ptr, "\t%s @ %d", name, &ordinal) == 2) {
				exportList->addExportOrdinal(name, ordinal);
			}
/*
			while (isspace(*ptr))
				ptr++;
			if (*ptr == '\0')
				continue;
			const char* name = ptr;
			while (!isspace(*ptr) && *ptr != '\0')
				ptr++;
			if (ptr == '\0') {
				fprintf(stderr, "Bad line in def file %s\n", filename);
				fclose(in);
				return;
			}
			*ptr = '\0';
*/
		}
	}
}


int main(int argc, char *argv[]) {
	detectVersion();

	int fixedaddress = 0;
	int unfrozen = 0;
	int noexportlibrary = 0;
	int dlldata = 0;
	struct option long_options[] = {
		{ "definput", 1, NULL, 0 },
		{ "defoutput", 1, NULL, 0 },
		{ "elfinput", 1, NULL, 0 },
		{ "output", 1, NULL, 0 },
		{ "dso", 1, NULL, 0 },
		{ "targettype", 1, NULL, 0 },
		{ "linkas", 1, NULL, 0 },
		{ "uid1", 1, NULL, 0 },
		{ "uid2", 1, NULL, 0 },
		{ "uid3", 1, NULL, 0 },
		{ "sid", 1, NULL, 0 },
		{ "vid", 1, NULL, 0 },
		{ "fixedaddress", 0, &fixedaddress, 1 },
		{ "uncompressed", 0, NULL, 0 },
		{ "compressionmethod", 1, NULL, 0 },
		{ "heap", 1, NULL, 0 },
		{ "stack", 1, NULL, 0 },
		{ "unfrozen", 0, &unfrozen, 1 },
		{ "ignorenoncallable", 0, NULL, 0 },
		{ "noexportlibrary", 0, &noexportlibrary, 1 },
		{ "capability", 1, NULL, 0 },
		{ "libpath", 1, NULL, 0 },
		{ "sysdef", 1, NULL, 0 },
		{ "log", 1, NULL, 0 },
		{ "messagefile", 1, NULL, 0 },
		{ "dumpmessagefile", 1, NULL, 0 },
		{ "dlldata", 0, &dlldata, 1 },
		{ "dump", 1, NULL, 0 },
		{ "e32input", 1, NULL, 0 },
		{ "priority", 1, NULL, 0 },
		{ "version", 1, NULL, 0 },
		{ "callentry", 1, NULL, 0 },
		{ "fpu", 1, NULL, 0 },
		{ "paged", 0, NULL, 0 },
		{ "unpaged", 0, NULL, 0 },
		{ "defaultpaged", 0, NULL, 0 },
		{ "excludeunwantedexports", 0, NULL, 0 },
		{ "customdlltarget", 0, NULL, 0 },
		{ "namedlookup", 0, NULL, 0 },
		{ "debuggable", 0, NULL, 0 },
		{ NULL, 0, NULL, 0 }
	};

	E32ImageHeader header;
	E32ImageHeaderComp headerComp;
	E32ImageHeaderV headerV;
	memset(&header, 0, sizeof(header));
	memset(&headerComp, 0, sizeof(headerComp));
	memset(&headerV, 0, sizeof(headerV));

	header.signature = 'E' | ('P'<<8) | ('O'<<16) | ('C'<<24);
	header.headerCrc = 0;
	header.moduleVersion = 10<<16 | 0;
	header.compressionType = KUidCompressionDeflate;
	header.toolsVersion = 2 | (0<<8) | (505<<16); // S60 3.0 - 3.2
//	header.toolsVersion = 2 | (0<<8) | (512<<16); // S60 5.0 version
	uint64_t timestamp = uint64_t(time(NULL))*1000000 + 0xDCDDB3E5D20000LL;
	header.timeLo = (timestamp >> 0) & 0xffffffff;
	header.timeHi = (timestamp >> 32) & 0xffffffff;
	header.flags = KImageImpFmt_ELF | KImageHdrFmt_V | KImageEpt_Eka2 | KImageABI_EABI | KImageNoCallEntryPoint;
	header.codeSize = 0;
	header.dataSize = 0;
	header.heapSizeMin = 0x00001000;
	header.heapSizeMax = 0x00100000;
	header.stackSize = 0x00002000;
	header.bssSize = 0;
	header.entryPoint = 0;
	header.codeBase = 0;
	header.dataBase = 0;
	header.dllRefTableCount = 0;
	header.exportDirOffset = 0;
	header.exportDirCount = 0;
	header.textSize = 0;
	header.codeOffset = sizeof(header) + sizeof(headerComp) + sizeof(headerV);
	header.dataOffset = 0;
	header.importOffset = 0;
	header.codeRelocOffset = 0;
	header.dataRelocOffset = 0;
	header.processPriority = EPriorityForeground;
	header.cpuIdentifier = ECpuArmV5;
	headerComp.uncompressedSize = 0;
	headerV.exceptionDescriptor = 0;
	headerV.spare2 = 0;
	headerV.exportDescSize = 0; // FIXME: do we need to set this sometime?
	headerV.exportDescType = KImageHdr_ExpD_FullBitmap;
	headerV.exportDesc[0] = 0; // FIXME

	const char* output = NULL;
	const char* elfinput = NULL;
	const char* libpath = NULL;
	const char* defoutput = NULL;
	char* dso = NULL;
	const char* linkas = NULL;

	while (true) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "", long_options, &option_index);
		const char* name = NULL;
		if (c == -1)
			break;

		switch (c) {
		case 0:
			name = long_options[option_index].name;
			if (!strcmp(name, "sid")) {
				sscanf(optarg, "%x", &headerV.secureId);
			} else if (!strcmp(name, "uid1")) {
				sscanf(optarg, "%x", &header.uid1);
			} else if (!strcmp(name, "uid2")) {
				sscanf(optarg, "%x", &header.uid2);
			} else if (!strcmp(name, "uid3")) {
				sscanf(optarg, "%x", &header.uid3);
			} else if (!strcmp(name, "vid")) {
				sscanf(optarg, "%x", &headerV.vendorId);
			} else if (!strcmp(name, "capability")) {
				getCapabilities(optarg, headerV.caps);
			} else if (!strcmp(name, "fpu")) {
				if (!strcmp(optarg, "softvfp")) {
					header.flags = (header.flags & ~KImageHWFloatMask);
				} else if (!strcmp(optarg, "vfpv2")) {
					header.flags = (header.flags & ~KImageHWFloatMask) | KImageHWFloat_VFPv2;
				} else {
					fprintf(stderr, "Unknown argument for parameter fpu: %s\n", optarg);
					return 1;
				}
			} else if (!strcmp(name, "targettype")) {
				if (!stricmp(optarg, "EXE")) {
				} else if (!stricmp(optarg, "PLUGIN")) {
					header.flags |= KImageDll | KImageOldJFlag;
					headerV.exportDescType = KImageHdr_ExpD_NoHoles;
				} else if (!stricmp(optarg, "DLL")) {
					header.flags |= KImageDll | KImageOldJFlag;
					headerV.exportDescType = KImageHdr_ExpD_NoHoles;
				}
			} else if (!strcmp(name, "output")) {
				output = optarg;
			} else if (!strcmp(name, "elfinput")) {
				elfinput = optarg;
			} else if (!strcmp(name, "linkas")) {
				linkas = optarg;
			} else if (!strcmp(name, "libpath")) {
				libpath = optarg;
			} else if (!strcmp(name, "heap")) {
				sscanf(optarg, "%x,%x", (uint32_t*) &header.heapSizeMin, (uint32_t*) &header.heapSizeMax);
			} else if (!strcmp(name, "stack")) {
				sscanf(optarg, "%x", (uint32_t*) &header.stackSize);
			} else if (!strcmp(name, "defoutput")) {
				defoutput = optarg;
			} else if (!strcmp(name, "sysdef")) {
				parseSysDef(optarg, &exportList);
			} else if (!strcmp(name, "definput")) {
				parseDefFile(optarg, &exportList);
			} else if (!strcmp(name, "dso")) {
				dso = strdup(optarg);
				char* ptr = dso;
				while (*ptr) {
					if (*ptr == '\\') *ptr = '/';
					ptr++;
				}
			} else if (!strcmp(name, "uncompressed")) {
				header.compressionType = 0;
			} else if (!strcmp(name, "compressionmethod")) {
				if (!strcmp(optarg, "none")) {
					header.compressionType = 0;
				} else if (!strcmp(optarg, "inflate")) {
					header.compressionType = KUidCompressionDeflate;
				} else if (!strcmp(optarg, "bytepair")) {
					printf("Bytepair compression not supported!\n");
				} else {
					printf("Unknown compression method \"%s\"\n", optarg);
				}
			} else if (!strcmp(name, "fixedaddress")) {
				// FIXME
			} else if (!strcmp(name, "noexportlibrary")) {
			} else if (!strcmp(name, "dlldata")) {
				// FIXME
			} else if (!strcmp(name, "unfrozen")) {
				// FIXME
			} else if (!strcmp(name, "version")) {
				int major = 10, minor = 0;
				sscanf(optarg, "%d.%d", &major, &minor);
				header.moduleVersion = (major << 16) | (minor);
			} else {
				fprintf(stderr, "*** Unhandled parameter %s\n", name);
			}
			break;
		case '?':
		case ':':
		default:
			return 1;
		}
	}
	header.uidChecksum = uidCrc(header.uid1, header.uid2, header.uid3);

	if (!elfinput && (exportList.numExports() == 0 || !dso)) {
		printf("nothing to do\n");
		free(dso);
		return 1;
	}

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr, "Elf library out of date\n");
		free(dso);
		return 1;
	}

	if (!elfinput) {
		exportList.doSort();
		if (dso && !noexportlibrary)
			exportList.writeDso(dso, linkas);
		free(dso);
		return 0;
	}

	int fd = open(elfinput, O_RDONLY);
	unlink(output);
	FILE* out = fopen(output, "w+b");

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	Elf_Scn* section = NULL;
	Elf32_Ehdr* ehdr = elf32_getehdr(elf);
	RelocSections relocSections;
	getDynamicValue(elf, DT_VERNEEDNUM, &relocSections.verneedNum);
	findSection(elf, SHT_DYNSYM, &relocSections.dynsymSection, &relocSections.dynsymHeader);
	findSection(elf, SHT_GNU_versym, &relocSections.symverSection, &relocSections.symverHeader);
	findSection(elf, SHT_GNU_verneed, &relocSections.verneedSection, &relocSections.verneedHeader);
	findExports(elf, &exportList);
	if (findSymbol(elf, "Symbian$$CPP$$Exception$$Descriptor", &headerV.exceptionDescriptor))
		headerV.exceptionDescriptor |= 1;

	Elf32_Phdr* phdr = elf32_getphdr(elf);
	for (unsigned int i = 0; i < ehdr->e_phnum; i++, phdr++) {
		if ((phdr->p_type == PT_LOAD) && ((phdr->p_flags & (PF_R | PF_W)) == (PF_R | PF_W))) {
			header.bssSize = phdr->p_memsz - phdr->p_filesz;
		}
	}

	fseek(out, header.codeOffset, SEEK_SET);
	while ((section = elf_nextscn(elf, section)) != NULL) {
		Elf32_Shdr* shdr;
		if ((shdr = elf32_getshdr(section)) != NULL) {
			const char* name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
			if (!strcmp(name, ".text") || !strcmp(name, "ER_RO"))
				header.codeBase = shdr->sh_addr;
			else if (!strcmp(name, ".data") || !strcmp(name, "ER_RW") || !strcmp(name, "ER_ZI"))
				header.dataBase = shdr->sh_addr;
/*
			else if (!strcmp(name, ".bss")) {
				header.bssSize = shdr->sh_size;
				uint32_t align = shdr->sh_addralign;
				if (align > 8)
					align = 8;
				header.bssSize = (header.bssSize + (align-1)) & (~(align-1));
			}
*/
		}
	}
	section = NULL;

	while ((section = elf_nextscn(elf, section)) != NULL) {
		Elf32_Shdr* shdr;
		if ((shdr = elf32_getshdr(section)) != NULL) {
			const char* name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
			if ((shdr->sh_flags & SHF_ALLOC) == 0)
				continue;

			if (!strcmp(name, ".data") || !strcmp(name, "ER_RW") || !strcmp(name, "ER_ZI"))
				continue;

			Elf_Data* data = NULL;
			if ((data = elf_rawdata(section, data)) == NULL || data->d_size == 0 || data->d_buf == 0)
				continue;

			if (header.codeBase + header.codeSize < shdr->sh_addr) {
				uint32_t pad = shdr->sh_addr - (header.codeBase + header.codeSize);
//				printf("at start of section, missing %d bytes\n", pad);
//				printf("at %x\n", ftell(out));
				for (unsigned int i = 0; i < pad; i++) {
					writeUint8(0, out);
					header.codeSize++;
					header.textSize++;
				}
			} else if (header.codeBase + header.codeSize > shdr->sh_addr) {
				printf("at start of section, %d bytes too far\n", (header.codeBase + header.codeSize) - shdr->sh_addr);
			}

//			printf("writing section %s at %x\n", name, ftell(out));
			fwrite(data->d_buf, 1, data->d_size, out);
			header.codeSize += shdr->sh_size;
			header.textSize += shdr->sh_size;
//			printf("now at %x\n", ftell(out));
		}
	}
	if (header.codeSize & 3) {
		uint32_t pad = 4 - (header.codeSize & 3);
//		printf("padding %d bytes\n", pad);
//		printf("at %x\n", ftell(out));
		for (unsigned int i = 0; i < pad; i++) {
			writeUint8(0, out);
			header.codeSize++;
			header.textSize++;
		}
	}

	if (header.flags & KImageDll) {
		exportList.doSort();
		if (defoutput)
			exportList.writeDef(defoutput);
		if (dso && !noexportlibrary)
			exportList.writeDso(dso, linkas);
		header.exportDirOffset = ftell(out) + 4;
		header.exportDirCount = exportList.numExports();
		uint32_t start = ftell(out);
		exportList.write(out, &header, &relocationList);
		uint32_t end = ftell(out);
		header.codeSize += end - start;
		header.textSize += end - start;
	}


	while ((section = elf_nextscn(elf, section)) != NULL) {
		Elf32_Shdr* shdr;
		if ((shdr = elf32_getshdr(section)) != NULL) {
			const char* name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
			if (strcmp(name, ".data") && strcmp(name, "ER_RW"))
				continue;

			Elf_Data* data = NULL;
			if ((data = elf_rawdata(section, data)) == NULL || data->d_size == 0 || data->d_buf == 0)
				continue;

//			printf("writing data section %s at %x\n", name, ftell(out));
			header.dataOffset = ftell(out);
			fwrite(data->d_buf, 1, data->d_size, out);
			header.dataSize += shdr->sh_size;
//			printf("now at %x\n", ftell(out));
/*
			if (data->d_size & 3) {
				uint32_t pad = 4 - (data->d_size & 3);
				for (int i = 0; i < pad; i++) {
					writeUint8(0, out);
					header.dataSize++;
				}
			}
*/
		}
	}
	if (header.dataSize & 3) {
		uint32_t pad = 4 - (header.dataSize & 3);
		for (unsigned int i = 0; i < pad; i++)
			writeUint8(0, out);
	}

	if (header.flags & KImageDll) {
		// On S60 5.0, this is an error
		if (header.dataSize > 0) {
			printf("ELF File %s contains initialized writable data\n.", elfinput);
		} else if (header.bssSize > 0) {
			printf("ELF File %s contains uninitialized writable data\n.", elfinput);
		}
	}

	header.importOffset = ftell(out);

	section = NULL;
//	printf("code: %x - %x\n", header.codeBase, header.codeBase + header.codeSize);
//	printf("data: %x - %x\n", header.dataBase, header.dataBase + header.dataSize);
	while ((section = elf_nextscn(elf, section)) != NULL) {
		Elf32_Shdr* shdr;
		if ((shdr = elf32_getshdr(section)) != NULL) {
			if (shdr->sh_type == SHT_REL) {
				checkRelocations(elf, section, shdr, &relocSections, &header, out, libpath);
			}
		}
	}

	if (headerV.exceptionDescriptor)
		headerV.exceptionDescriptor -= header.codeBase;
	header.entryPoint = ehdr->e_entry - header.codeBase;

	fseek(out, header.importOffset, SEEK_SET);
	header.dllRefTableCount = importList.numLibraries();
	importList.write(out);

	header.codeRelocOffset = ftell(out);
	relocationList.write(out, &header);
	if (dataRelocationList.count() > 0) {
		header.dataRelocOffset = ftell(out);
		dataRelocationList.write(out, &header);
	}

	headerComp.uncompressedSize = ftell(out) - header.codeOffset;

	elf_end(elf);
	close(fd);

	fseek(out, 0, SEEK_SET);
	header.headerCrc = KImageCrcInitialiser;
#define CRCSIZE 0x9c
	writeHeaders(out, &header, &headerComp, &headerV);
	uint8_t buf[CRCSIZE];
	fseek(out, 0, SEEK_SET);
	fread(buf, 1, CRCSIZE, out);
	header.headerCrc = ~crc32(0xffffffff, buf, CRCSIZE);
	fseek(out, 0, SEEK_SET);
	writeHeaders(out, &header, &headerComp, &headerV);


	if (header.compressionType == KUidCompressionDeflate) {
		fseek(out, 0, SEEK_END);
		uint32_t len = ftell(out);
		len -= CRCSIZE;
		uint8_t* headerData = (uint8_t*) malloc(CRCSIZE);
		uint8_t* data = (uint8_t*) malloc(len);
		fseek(out, 0, SEEK_SET);
		fread(headerData, 1, CRCSIZE, out);
		fread(data, 1, len, out);
		fclose(out);

		ofstream stream(output, ios_base::binary | ios_base::out);
		stream.write((const char*) headerData, CRCSIZE);
//		writeHeaders(out, &header, &headerComp, &headerV);
		DeflateCompress((char*) data, len, stream);
		stream.close();

		free(data);
		free(headerData);
	} else {
		fclose(out);
	}

	free(dso);

	return 0;
}

