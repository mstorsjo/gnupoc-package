/*
    genstubs
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
#include <libelf.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "writeelf.h"


void writeStub(const char* file, const char* symbol, const char* name) {
	int fd = open(file, O_RDWR | O_TRUNC | O_CREAT, 0666);
	if (fd < 0) {
		perror(file);
		return;
	}
	Elf* elf = elf_begin(fd, ELF_C_WRITE, NULL);
	Elf32_Ehdr* ehdr = elf32_newehdr(elf);
	uint8_t ident[] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, ELFCLASS32, ELFDATA2LSB, EV_CURRENT, ELFOSABI_NONE, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	memcpy(ehdr->e_ident, ident, sizeof(ident));
	ehdr->e_type = ET_REL;
	ehdr->e_machine = EM_ARM;
	ehdr->e_version = EV_CURRENT;
	ehdr->e_entry = 0;
	ehdr->e_flags = 0x2000000;
	ehdr->e_phentsize = sizeof(Elf32_Phdr);

	StringTable shstrtab;
	StringTable strtab;
	SymbolArray symtab;
	StringSection comment;
	StringSection directive;
	CodeSection stubcode;
	RelocationSection rel;

	stubcode.createSection(elf);
	rel.createSection(elf);
	symtab.createSection(elf);
	comment.createSection(elf);
	shstrtab.createSection(elf);
	directive.createSection(elf);
	strtab.createSection(elf);

	stubcode.getShdr()->sh_name = shstrtab.appendString("StubCode");
	stubcode.getShdr()->sh_type = SHT_PROGBITS;
	stubcode.getShdr()->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
	stubcode.getShdr()->sh_addralign = 4;
	stubcode.appendObject(0xe51ff004); // ldr pc, [pc, #-4]
	stubcode.appendObject(0);

	rel.getShdr()->sh_name = shstrtab.appendString(".relStubCode");
	rel.getShdr()->sh_type = SHT_REL;
	rel.getShdr()->sh_link = comment.sectionIndex();
	rel.getShdr()->sh_info = 1;
	rel.getShdr()->sh_addralign = 0;
	Elf32_Rel* r = rel.appendObject();
	r->r_offset = 4;
	r->r_info = ELF32_R_INFO(7, R_ARM_ABS32); // Symbol nr 7, 'name'

	symtab.getShdr()->sh_name = shstrtab.appendString(".symtab");
	symtab.getShdr()->sh_type = SHT_SYMTAB;
	symtab.getShdr()->sh_info = directive.sectionIndex();

	Elf32_Sym* sym;
	sym = symtab.appendClearedObject();
	sym->st_name = 0;
	sym->st_value = 0;
	sym->st_size = 0;
	sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE);
	sym->st_shndx = 0;

	sym = symtab.appendClearedObject();
	sym->st_name = strtab.appendString("$a");
	sym->st_value = 0;
	sym->st_size = 0;
	sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_FUNC);
	sym->st_shndx = stubcode.sectionIndex();

	sym = symtab.appendClearedObject();
	sym->st_name = strtab.appendString("$d");
	sym->st_value = 4;
	sym->st_size = 0;
	sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_OBJECT);
	sym->st_shndx = stubcode.sectionIndex();

	sym = symtab.appendClearedObject();
	sym->st_name = strtab.appendString("StubCode");
	sym->st_value = 0;
	sym->st_size = stubcode.usedSize();
	sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
	sym->st_shndx = stubcode.sectionIndex();

	sym = symtab.appendClearedObject();
	sym->st_name = strtab.appendString(".directive");
	sym->st_value = 0;
	sym->st_size = 0;
	sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
	sym->st_shndx = symtab.sectionIndex();

	sym = symtab.appendClearedObject();
	sym->st_name = strtab.appendString("theImportedSymbol");
	sym->st_value = 4;
	sym->st_size = 0;
	sym->st_info = ELF32_ST_INFO(STB_LOCAL, STT_FUNC);
	sym->st_shndx = stubcode.sectionIndex();

	sym = symtab.appendClearedObject();
	sym->st_name = strtab.appendString(symbol);
	sym->st_value = 0;
	sym->st_size = 0;
	sym->st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
	sym->st_shndx = stubcode.sectionIndex();

	sym = symtab.appendClearedObject();
	sym->st_name = strtab.appendString(name);
	sym->st_value = 0;
	sym->st_size = 0;
	sym->st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
	sym->st_shndx = 0;

	strtab.getShdr()->sh_name = shstrtab.appendString(".strtab");
	strtab.getShdr()->sh_type = SHT_STRTAB;
	strtab.getShdr()->sh_entsize = 0;

	comment.appendString("\tAndy's magic stub generator vsn 0.2\n");
	comment.getShdr()->sh_name = shstrtab.appendString(".comment");
	comment.getShdr()->sh_type = SHT_PROGBITS;
	comment.getShdr()->sh_addralign = 0;

	directive.appendString("#<SYMEDIT>\n");
	directive.appendString("IMPORT ");
	directive.appendString(name);
	directive.appendString("\n");
	directive.getShdr()->sh_name = shstrtab.appendString(".directive");
	directive.getShdr()->sh_type = SHT_PROGBITS;

	shstrtab.getShdr()->sh_name = shstrtab.appendString(".shstrtab");
	shstrtab.getShdr()->sh_type = SHT_STRTAB;
	shstrtab.getShdr()->sh_entsize = 0;



	ehdr->e_shstrndx = shstrtab.sectionIndex();
	symtab.getShdr()->sh_link = strtab.sectionIndex();

	strtab.update(0);
	symtab.update(0);
	comment.update(0);
	shstrtab.update(0);
	directive.update(1);
	stubcode.update(4);
	rel.update(0);

	elf_update(elf, ELF_C_WRITE);

	elf_end(elf);
	close(fd);
}

int main(int argc, char *argv[]) {
	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr, "Elf library out of date\n");
		return 1;
	}

	char buf[10000];
	while (fgets(buf, sizeof(buf), stdin)) {
		char* file = buf;
		while (isspace(*file))
			file++;
		if (*file == '\0')
			continue;
		char* symbol = file + 1;
		while (!isspace(*symbol) && *symbol != '\0')
			symbol++;
		if (*symbol == '\0')
			continue;
		*symbol = '\0';
		symbol++;
		while (isspace(*symbol))
			symbol++;
		if (*symbol == '\0')
			continue;
		char* name = symbol + 1;
		while (!isspace(*name) && *name != '\0')
			name++;
		if (*name == '\0')
			continue;
		*name = '\0';
		name++;
		while (isspace(*name))
			name++;
		if (*name == '\0')
			continue;
		char* end = name + 1;
		while (!isspace(*end) && *end != '\0')
			end++;
		*end = '\0';

		writeStub(file, symbol, name);
	}

	return 0;
}

