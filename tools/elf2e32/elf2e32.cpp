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
#include "caseinsensitive.h"
#include "epocversion.h"
#include "e32image.h"
#include "elfutils.h"
#include "reloc.h"






struct RelocSections {
	Elf32_Shdr* dynsymHeader;
	Elf_Scn* dynsymSection;
	Elf32_Shdr* symverHeader;
	Elf_Scn* symverSection;
	Elf32_Shdr* verneedHeader;
	Elf_Scn* verneedSection;
	Elf32_Word verneedNum;
};

ImportList importList;
RelocationList relocationList;
RelocationList dataRelocationList;
ExportList exportList;

void checkRelocations(Elf* elf, Elf_Scn* relocationSection, Elf32_Shdr* relocationHeader, RelocSections* sections, E32ImageHeader* header, FILE* out, std::vector<const char*> libpath) {
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
					if (rel->r_offset >= header->codeBase && rel->r_offset < header->codeBase + header->codeSize) {
						fixImportRelocation(rel->r_offset - header->codeBase + header->codeOffset, out, name, dsoname, libpath);
						importList.addImport(dllname, rel->r_offset - header->codeBase);
					} else if (rel->r_offset >= header->dataBase && rel->r_offset < header->dataBase + header->dataSize + header->bssSize) {
						if (epocVersion >= EPOC_VERSION_9_4)
							printf("Error: '%s' Import relocation does not refer to code segment.\n", name);
						fixImportRelocation(rel->r_offset - header->dataBase + header->dataOffset, out, name, dsoname, libpath);
						importList.addImport(dllname, rel->r_offset - header->dataBase);
					} else {
						printf("unhandled import relocation of symbol %s\n", name);
					}
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
	int linenum = 0;
	int prevOrdinal = 0;
	while ((ptr = fgets(line, sizeof(line), in)) != NULL) {
		linenum++;
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
			bool code = true;
			if (strstr(ptr, "; DATA"))
				code = false;
			char* comments = strchr(ptr, ';');
			if (comments)
				*comments = '\0';
			char name[500];
			int ordinal;
			if (sscanf(ptr, "\t%s @ %d", name, &ordinal) == 2) {
				if (ordinal != prevOrdinal + 1)
					fprintf(stderr, "Ordinal number is not in sequence: %s[Line No=%d][%s]\n", filename, linenum, name);

				const char* ptr2 = ptr + strlen(name); // Skip past name
				ptr2 = strchr(ptr2, '@'); // Skip past @
				int size = 0;
				const char* ptr3 = strstr(ptr2, "DATA");
				if (ptr3) {
					code = false;
					sscanf(ptr3, "DATA %d", &size);
				}

				exportList->addExportOrdinal(name, ordinal, code, size);
				prevOrdinal = ordinal;
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
	fclose(in);
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
		{ "callentry", 0, NULL, 0 },
		{ "fpu", 1, NULL, 0 },
		{ "codepaging", 1, NULL, 0 },
		{ "datapaging", 1, NULL, 0 },
		{ "paged", 0, NULL, 0 },
		{ "unpaged", 0, NULL, 0 },
		{ "defaultpaged", 0, NULL, 0 },
		{ "excludeunwantedexports", 0, NULL, 0 },
		{ "customdlltarget", 0, NULL, 0 },
		{ "namedlookup", 0, NULL, 0 },
		{ "debuggable", 0, NULL, 0 },
		{ "smpsafe", 0, NULL, 0 },
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
	if (epocVersion <= EPOC_VERSION_9_3)
		header.toolsVersion = 2 | (0<<8) | (505<<16); // S60 3.0 - 3.2
	else if (epocVersion <= EPOC_VERSION_9_4)
		header.toolsVersion = 2 | (0<<8) | (512<<16); // S60 5.0 version
	else
		header.toolsVersion = 2 | (1<<8) | ( 15<<16); // Symbian^3 version
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
	std::vector<const char*> libpath;
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
				headerV.secureId = strtoul(optarg, NULL, 0);
			} else if (!strcmp(name, "uid1")) {
				header.uid1 = strtoul(optarg, NULL, 0);
			} else if (!strcmp(name, "uid2")) {
				header.uid2 = strtoul(optarg, NULL, 0);
			} else if (!strcmp(name, "uid3")) {
				header.uid3 = strtoul(optarg, NULL, 0);
			} else if (!strcmp(name, "vid")) {
				headerV.vendorId = strtoul(optarg, NULL, 0);
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
				} else if (!stricmp(optarg, "DLL")) {
					header.flags |= KImageDll | KImageOldJFlag;
				}
			} else if (!strcmp(name, "output")) {
				output = optarg;
			} else if (!strcmp(name, "elfinput")) {
				elfinput = optarg;
			} else if (!strcmp(name, "linkas")) {
				linkas = optarg;
			} else if (!strcmp(name, "libpath")) {
				libpath.push_back(optarg);
			} else if (!strcmp(name, "heap")) {
				char buf1[50];
				char buf2[50];
				strncpy(buf1, optarg, sizeof(buf1));
				buf1[sizeof(buf1) - 1] = '\0';
				char* ptr = strchr(buf1, ',');
				if (ptr) {
					*ptr = '\0';
					strncpy(buf2, ptr + 1, sizeof(buf2));
					buf2[sizeof(buf2) - 1] = '\0';
					header.heapSizeMax = strtoul(buf2, NULL, 0);
				}
				header.heapSizeMin = strtoul(buf1, NULL, 0);
			} else if (!strcmp(name, "stack")) {
				header.stackSize = strtoul(optarg, NULL, 0);
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
			} else if (!strcmp(name, "callentry")) {
//				header.flags &= ~KImageNoCallEntryPoint;
			} else if (!strcmp(name, "paged")) {
				header.flags |= KImagePaged;
			} else if (!strcmp(name, "unpaged")) {
				header.flags |= KImageUnpaged;
			} else if (!strcmp(name, "defaultpaged")) {
				header.flags &= ~(KImageUnpaged | KImagePaged);
			} else if (!strcmp(name, "codepaging")) {
				if (!strcmp(optarg, "paged")) {
					header.flags |= KImagePaged;
				} else if (!strcmp(optarg, "unpaged")) {
					header.flags |= KImageUnpaged;
				} else if (!strcmp(optarg, "default")) {
					header.flags &= ~(KImageUnpaged | KImagePaged);
				} else {
					fprintf(stderr, "Unsupported codepaging value %s\n", optarg);
				}
			} else if (!strcmp(name, "datapaging")) {
				if (!strcmp(optarg, "paged")) {
					header.flags |= KImageDataPaged;
				} else if (!strcmp(optarg, "unpaged")) {
					header.flags |= KImageDataUnpaged;
				} else if (!strcmp(optarg, "default")) {
					header.flags &= ~(KImageDataUnpaged | KImageDataPaged);
				} else {
					fprintf(stderr, "Unsupported datapaging value %s\n", optarg);
				}
			} else if (!strcmp(name, "smpsafe")) {
				header.flags |= KImageSMPSafe;
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
		printf("GnuPoc native elf2e32 replacement\n");
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
		if (dso && !noexportlibrary) {
			if (linkas)
				exportList.writeDso(dso, linkas);
			else
				fprintf(stderr, "No linkas parameter supplied\n");
		}
		free(dso);
		return 0;
	}

	if (!output) {
		printf("no output file specified\n");
		free(dso);
		return 1;
	}

	int fd = open(elfinput, O_RDONLY);
	if (fd < 0) {
		perror(elfinput);
		free(dso);
		return 1;
	}
	unlink(output);
	FILE* out = fopen(output, "w+b");

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	if (!elf) {
		fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
		free(dso);
		close(fd);
		return 1;
	}
	Elf_Scn* section = NULL;
	Elf32_Ehdr* ehdr = elf32_getehdr(elf);
	if (!ehdr) {
		fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
		free(dso);
		elf_end(elf);
		close(fd);
		return 1;
	}
	RelocSections relocSections = { NULL };
	getDynamicValue(elf, DT_VERNEEDNUM, &relocSections.verneedNum);
	findSection(elf, SHT_DYNSYM, &relocSections.dynsymSection, &relocSections.dynsymHeader);
	findSection(elf, SHT_GNU_versym, &relocSections.symverSection, &relocSections.symverHeader);
	findSection(elf, SHT_GNU_verneed, &relocSections.verneedSection, &relocSections.verneedHeader);
	if (header.flags & KImageDll) {
		findExports(elf, &exportList);
		if (exportList.numExports() > 0)
			headerV.exportDescType = KImageHdr_ExpD_NoHoles;
	}
	if (exportList.warnMissing(elfinput)) {
		elf_end(elf);
		close(fd);
		free(dso);
		return 1;
	}
	if (findSymbol(elf, "Symbian$$CPP$$Exception$$Descriptor", &headerV.exceptionDescriptor))
		headerV.exceptionDescriptor |= 1;

	Elf32_Phdr* phdr = elf32_getphdr(elf);
	for (unsigned int i = 0; i < ehdr->e_phnum; i++, phdr++) {
		if ((phdr->p_type == PT_LOAD) && ((phdr->p_flags & (PF_R | PF_X)) == (PF_R | PF_X))) {
			header.codeBase = phdr->p_vaddr;
		} else if ((phdr->p_type == PT_LOAD) && ((phdr->p_flags & (PF_R | PF_W)) == (PF_R | PF_W))) {
			header.dataBase = phdr->p_vaddr;
			header.bssSize = phdr->p_memsz - phdr->p_filesz;
		}
	}

	fseek(out, header.codeOffset, SEEK_SET);
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
		if (exportList.numExports() > 0) {
			header.exportDirOffset = ftell(out) + 4;
			header.exportDirCount = exportList.numExports();
			uint32_t start = ftell(out);
			exportList.write(out, &header, &relocationList);
			uint32_t end = ftell(out);
			header.codeSize += end - start;
			header.textSize += end - start;
		}
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
			printf("ELF File %s contains initialized writable data.\n", elfinput);
		} else if (header.bssSize > 0) {
			printf("ELF File %s contains uninitialized writable data.\n", elfinput);
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

	finalizeE32Image(out, &header, &headerComp, &headerV, output);

	free(dso);

	return 0;
}

