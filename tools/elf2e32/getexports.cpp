/*
    getexports
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
#include <unistd.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "%s input\n", argv[0]);
		return 1;
	}
	const char* elfinput = argv[1];

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr, "Elf library out of date\n");
		return 1;
	}

	int fd = open(elfinput, O_RDONLY);
	if (fd < 0) {
		perror(elfinput);
		return 1;
	}

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);

	Elf_Scn* section = NULL;
	Elf32_Shdr* shdr;
	while ((section = elf_nextscn(elf, section)) != NULL) {
		if ((shdr = elf32_getshdr(section)) != NULL) {
			if (shdr->sh_type == SHT_SYMTAB)
				break;
		}
	}
	if (section == NULL) {
		fprintf(stderr, "Symbol table not found\n");
		return 1;
	}

	Elf_Data* data = NULL;

	if ((data = elf_getdata(section, data)) == 0 || data->d_size == 0) {
		fprintf(stderr, "Symbol table has no data");
		return 1;
	}

	Elf32_Sym* sym = (Elf32_Sym*) data->d_buf;
	Elf32_Sym* end = (Elf32_Sym*) ((uint8_t*)data->d_buf + data->d_size);
	for (; sym < end; sym++) {
		const char* name = elf_strptr(elf, shdr->sh_link, sym->st_name);
		if (!name) {
			fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
			continue;
		}
		uint8_t bind = ELF32_ST_BIND(sym->st_info);
		uint8_t type = ELF32_ST_TYPE(sym->st_info);
		uint8_t visibility = ELF32_ST_VISIBILITY(sym->st_other);
		if (bind == STB_GLOBAL && (type == STT_FUNC || type == STT_OBJECT) && visibility == STV_DEFAULT && (sym->st_value || type == STT_OBJECT)) {
			if (type == STT_OBJECT) {
				Elf_Scn* ref = elf_getscn(elf, sym->st_shndx);
				Elf32_Shdr* refhdr;
				if (ref != NULL && (refhdr = elf32_getshdr(ref))) {
					if (refhdr->sh_type == SHT_PROGBITS) {
						printf(" %s DATA %d\n", name, refhdr->sh_size);
					}
				}
			} else {
				printf(" %s\n", name);
			}
		}
	}

	elf_end(elf);
	close(fd);

	return 0;
}

