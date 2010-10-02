/*
    gendirective
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
	Elf32_Ehdr* ehdr = elf32_getehdr(elf);

	Elf_Scn* section = NULL;
	while ((section = elf_nextscn(elf, section)) != NULL) {
		Elf32_Shdr* shdr;
		if ((shdr = elf32_getshdr(section)) != NULL) {
			const char* name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
			if (strcmp(name, ".directive"))
				continue;
			Elf_Data* data = NULL;
			if ((data = elf_rawdata(section, data)) == NULL || data->d_size == 0 || data->d_buf == 0)
				continue;
			fwrite(data->d_buf, 1, data->d_size, stdout);
			fprintf(stdout, "\n");

			elf_end(elf);
			close(fd);
			return 0;
		}
	}

	fprintf(stderr, "No .directive section found!\n");
	elf_end(elf);
	close(fd);
	return 1;
}

