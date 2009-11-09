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

#include "reloc.h"
#include <libelf.h>
#include "e32image.h"
#include "caseinsensitive.h"
#include <ctype.h>
#include "writeelf.h"
#include <fcntl.h>
#include <unistd.h>
#include "elfutils.h"
#include "epocversion.h"

#ifndef EF_ARM_INTERWORK
#define EF_ARM_INTERWORK 0x04
#endif

#ifdef ELF_HASH_UNSIGNED
typedef unsigned char* elf_string;
#else
typedef char* elf_string;
#endif



void fixImportRelocation(uint32_t offset, FILE* out, uint32_t ordinal) {
	fseek(out, offset, SEEK_SET);
	uint8_t origBuf[4];
	fread(origBuf, 1, sizeof(origBuf), out);
	uint32_t orig = origBuf[0] | (origBuf[1]<<8) | (origBuf[2]<<16) | (origBuf[3]<<24);
	ordinal += orig << 16;
	fseek(out, offset, SEEK_SET);
	writeUint32(ordinal, out);
}

void fixImportRelocation(uint32_t offset, FILE* out, const char* symbol, const char* lib, const char* libpath) {
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
		printf("fixImportRelocation SHT_DYNSYM not found\n");
		elf_end(elf);
		close(fd);
		return;
	}
	Elf32_Sym* sym;
	Elf32_Addr addr;
	if (!findSymbol(elf, section, shdr, symbol, &sym)) {
		printf("fixImportRelocation symbol %s not found\n", symbol);
		elf_end(elf);
		close(fd);
		return;
	}
	addr = sym->st_value;

	if (!findSection(elf, SHT_PROGBITS, &section, &shdr)) {
		printf("fixImportRelocation section SHT_PROGBITS not found\n");
		elf_end(elf);
		close(fd);
		return;
	}

	addr -= shdr->sh_addr;

	Elf_Data* data = NULL;
	data = elf_rawdata(section, data);
	if (addr >= data->d_size) {
		printf("fixImportRelocation addr past d_size\n");
		elf_end(elf);
		close(fd);
		return;
	}
//	uint32_t ordinal = *((uint32_t*) (((uint8_t*)data->d_buf) + addr));
	uint8_t* dataptr = (uint8_t*) data->d_buf;
	dataptr += addr;
	uint32_t ordinal = dataptr[0] | (dataptr[1]<<8) | (dataptr[2]<<16) | (dataptr[3]<<24);
	elf_end(elf);
	close(fd);

	fixImportRelocation(offset, out, ordinal);
}

uint32_t fixRelocation(uint32_t offset, FILE* out, uint32_t value) {
	fseek(out, offset, SEEK_SET);
	uint8_t origBuf[4];
	fread(origBuf, 1, sizeof(origBuf), out);
	uint32_t orig = origBuf[0] | (origBuf[1]<<8) | (origBuf[2]<<16) | (origBuf[3]<<24);
	value += orig;
	fseek(out, offset, SEEK_SET);
	writeUint32(value, out);
	return value;
}


void ImportList::write(FILE* out, bool doSort, bool padsize) {
	if (doSort)
		sort(libraries.begin(), libraries.end(), Library::compare);
	uint32_t size = 0;
	size += 4;
	for (unsigned int i = 0; i < libraries.size(); i++) {
		size += 8;
		size += libraries[i]->addresses.size()*4;
//		printf("%d imports for lib %s\n", libraries[i]->addresses.size(), libraries[i]->name);
	}
	for (unsigned int i = 0; i < libraries.size(); i++) {
		libraries[i]->offset = size;
		size += strlen(libraries[i]->name) + 1;
	}
	uint32_t padding = (size & 3) ? (4 - (size & 3)) : 0;
	if (padsize)
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


void RelocationList::writeSublist(uint32_t prefix, FILE* out, E32ImageHeader* header) {
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

void RelocationList::write(FILE* out, E32ImageHeader* header) {
	uint32_t size = 0;
	uint32_t num = count();
	for (map<uint32_t, vector<Relocation> >::iterator it = sublists.begin(); it != sublists.end(); it++)
		size += sublistSize(it->first);
	writeUint32(size, out);
	writeUint32(num, out);
	for (map<uint32_t, vector<Relocation> >::iterator it = sublists.begin(); it != sublists.end(); it++)
		writeSublist(it->first, out, header);
}



void ExportList::addExport(const char* name, uint32_t addr, bool code, int size) {
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
	if (epocVersion >= EPOC_VERSION_9_3)
		fprintf(stderr, "Warning: New Symbol %s not found, export(s) not yet Frozen\n", name);
}

void ExportList::write(FILE* out, E32ImageHeader* header, RelocationList* relocations) {
	writeUint32(exports.size(), out);
	for (unsigned int i = 0; i < exports.size(); i++) {
		uint32_t addr = ftell(out) - header->codeOffset;
		writeUint32(exports[i]->address, out);
		relocations->addRelocation(addr, exports[i]->address);
	}
}

void ExportList::writeDef(const char* filename) {
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


void ExportList::writeDso(const char* filename, const char* soname) {
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
//	fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
	close(fd);
}

