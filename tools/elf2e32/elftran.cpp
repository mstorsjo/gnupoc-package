/*
    Elftran
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
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <libelf.h>
#include "caseinsensitive.h"
#include "epocversion.h"
#include "e32image.h"
#include "elfutils.h"
#include "reloc.h"

int getTypeSize(Elf_Type type) {
	switch (type) {
	case ELF_T_DYN:
		return sizeof(Elf32_Dyn);
	case ELF_T_SYM:
		return sizeof(Elf32_Sym);
	case ELF_T_REL:
		return sizeof(Elf32_Rel);
	case ELF_T_BYTE:
	default:
		return 1;
	}
}

Elf_Data* getTranslatedElfData(Elf* elf, off_t offset, size_t size, Elf_Type type) {
#ifdef NO_ELF_RAWCHUNK
	Elf_Data* data = (Elf_Data*) malloc(sizeof(Elf_Data));
	memset(data, 0, sizeof(Elf_Data));
	data->d_buf = malloc(size);
	data->d_type = type;
	data->d_version = EV_CURRENT;
	data->d_size = size;
	data->d_off = 0;
	data->d_align = 0;
	size_t fileSize = 0;
	const char* orig = elf_rawfile(elf, &fileSize);
	if (!orig) {
		// elf_rawfile returns NULL with elfutils libelf unless opened with ELF_C_READ_MMAP,
		// but that version has elf_getdata_rawchunk instead.
		// That bug is fixed in the latest upstream version.
		fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
		return NULL;
	}
	fileSize -= offset;
	orig += offset;
	if (data->d_size > fileSize)
		data->d_size = fileSize;

	// Make the size an integer number of elements of the given type
	data->d_size -= data->d_size % getTypeSize(type);

	Elf_Data indata;
	indata.d_buf = (void*) orig;
	indata.d_type = type;
	indata.d_version = EV_CURRENT;
	indata.d_size = data->d_size;
	indata.d_off = 0;
	indata.d_align = 0;
	Elf32_Ehdr* ehdr = elf32_getehdr(elf);
	Elf_Data* out = elf32_xlatetom(data, &indata, ehdr->e_ident[EI_DATA]);
	if (!out) {
		fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
		free(data->d_buf);
		free(data);
		return NULL;
	}
	return data;
#else
	return elf_getdata_rawchunk(elf, offset, size, type);
#endif
}

void freeElfData(Elf_Data* data) {
#ifdef NO_ELF_RAWCHUNK
	free(data->d_buf);
	free(data);
#endif
}

ImportList importList;
RelocationList relocationList;
RelocationList dataRelocationList;
ExportList exportList;

char* parseNameValue(const char* name, Elf32_Word* valuePtr) {
	if (strncmp(name, "#<DLL>", 6))
		return NULL;
	name += 6;
	const char* end = strstr(name, "#<\\DLL>");
	if (!end)
		return NULL;
	int len = end - name;
	char* out = (char*) malloc(len + 1);
	memcpy(out, name, len);
	out[len] = '\0';
	end += 7;
	unsigned int value;
	sscanf(end, "%x", &value);
	*valuePtr = value;
	return out;
}

void parseDynamic(Elf* elf, Elf32_Phdr* phdr, FILE* out, E32ImageHeader* header) {
	Elf_Data* data = getTranslatedElfData(elf, phdr->p_offset, phdr->p_filesz, ELF_T_DYN);

	Elf32_Word symtabStart = 0;
	Elf32_Word symtabSize  = 0;
	Elf32_Word strtabStart = 0;
	Elf32_Word strtabSize  = 0;
	Elf32_Word hashStart   = 0;
	Elf32_Word relStart    = 0;
	Elf32_Word relSize     = 0;
	Elf32_Dyn* dyn = (Elf32_Dyn*) data->d_buf;
	Elf32_Dyn* dynEnd = (Elf32_Dyn*) ((uint8_t*)data->d_buf + data->d_size);
	for (; dyn < dynEnd && dyn->d_tag; dyn++) {
		switch (dyn->d_tag) {
		case DT_SYMTAB:  symtabStart = dyn->d_un.d_val; break;
		case 0x70000001: symtabSize  = dyn->d_un.d_val; break;
		case DT_STRTAB:  strtabStart = dyn->d_un.d_val; break;
		case DT_STRSZ:   strtabSize  = dyn->d_un.d_val; break;
		case DT_HASH:    hashStart   = dyn->d_un.d_val; break;
		case DT_REL:     relStart    = dyn->d_un.d_val; break;
		case DT_RELSZ:   relSize     = dyn->d_un.d_val; break;
		}
	}
	if (!symtabStart || !symtabSize) {
		fprintf(stderr, "Symtab not found in dynamic segment\n");
		freeElfData(data);
		return;
	}
	if (!strtabStart || !strtabSize) {
		fprintf(stderr, "Strtab not found in dynamic segment\n");
		freeElfData(data);
		return;
	}
	if (!relStart || !relSize) {
		fprintf(stderr, "Rel not found in dynamic segment\n");
		freeElfData(data);
		return;
	}

	Elf_Data* symtabData = getTranslatedElfData(elf, phdr->p_offset + symtabStart, symtabSize, ELF_T_SYM);
	Elf_Data* strtabData = getTranslatedElfData(elf, phdr->p_offset + strtabStart, strtabSize, ELF_T_BYTE);
	Elf_Data* relData    = getTranslatedElfData(elf, phdr->p_offset + relStart,    relSize,    ELF_T_REL);
	const char* strtabPtr = (const char*) strtabData->d_buf;

	Elf32_Sym* symArray = (Elf32_Sym*) symtabData->d_buf;
	Elf32_Sym* symEnd = (Elf32_Sym*) ((uint8_t*)symtabData->d_buf + symtabData->d_size);

	Elf32_Sym* sym = (Elf32_Sym*) symtabData->d_buf;
	for (; sym < symEnd; sym++) {
		const char* name = &strtabPtr[sym->st_name];
		if (!strcmp(name, "DLL##ExportTable")) {
			header->exportDirOffset = sym->st_value - header->codeBase + header->codeOffset;
			header->flags |= KImageDll;
		} else if (!strcmp(name, "DLL##ExportTableSize")) {
			if (sym->st_value >= header->codeBase && sym->st_value < header->codeBase + header->codeSize) {
				uint32_t offset = sym->st_value - header->codeBase + header->codeOffset;
				fseek(out, offset, SEEK_SET);
				uint8_t buf[4];
				fread(buf, 1, sizeof(buf), out);
				uint32_t value = buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24);
				header->exportDirCount = value;
			}
		}
	}

	Elf32_Rel* rel = (Elf32_Rel*) relData->d_buf;
	Elf32_Rel* relEnd = (Elf32_Rel*) ((uint8_t*)relData->d_buf + relData->d_size);
	for (; rel < relEnd; rel++) {
		int type = ELF32_R_TYPE(rel->r_info);
		int index = ELF32_R_SYM(rel->r_info);
		Elf32_Sym* sym = &symArray[index];
		if (sym < symArray || sym >= symEnd)
			continue;
		const char* name = &strtabPtr[sym->st_name];
		if (sym->st_value) {
			// Does not occur?
		} else {
			Elf32_Word value;
			char* dllname = parseNameValue(name, &value);
			if (rel->r_offset >= header->codeBase && rel->r_offset < header->codeBase + header->codeSize) {
				if (type == R_ARM_ABS32) {
					fixImportRelocation(rel->r_offset - header->codeBase + header->codeOffset, out, value);
					importList.addImport(dllname, rel->r_offset - header->codeBase);
				} else if (type == R_ARM_RABS22) {
					value = fixRelocation(rel->r_offset - header->codeBase + header->codeOffset, out, 0);
					relocationList.addRelocation(rel->r_offset - header->codeBase, value);
				} else {
				}
			} else if (rel->r_offset >= header->dataBase && rel->r_offset < header->dataBase + header->dataSize + header->bssSize) {
				value = fixRelocation(rel->r_offset - header->dataBase + header->dataOffset, out, 0);
				dataRelocationList.addRelocation(rel->r_offset - header->dataBase, value);
			}
			free(dllname);
		}
	}
	freeElfData(data);
	freeElfData(symtabData);
	freeElfData(strtabData);
	freeElfData(relData);
}

int parseOptions(char** argv, int argc, E32ImageHeader* header, E32ImageHeaderComp* headerComp, E32ImageHeaderV* headerV, int* dlldata, const char** elfinput, const char** output, int* dumpFlags, bool* headersModified) {
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-sid")) {
			headerV->secureId = strtol(argv[i + 1], NULL, 0);
			i++;
		} else if (!strcmp(argv[i], "-uid1")) {
			header->uid1 = strtol(argv[i + 1], NULL, 0);
			i++;
		} else if (!strcmp(argv[i], "-uid2")) {
			header->uid2 = strtol(argv[i + 1], NULL, 0);
			i++;
		} else if (!strcmp(argv[i], "-uid3")) {
			header->uid3 = strtol(argv[i + 1], NULL, 0);
			i++;
		} else if (!strcmp(argv[i], "-vid")) {
			headerV->vendorId = strtol(argv[i + 1], NULL, 0);
			i++;
		} else if (!strcmp(argv[i], "-capability")) {
			getCapabilities(argv[i + 1], headerV->caps);
			i++;
		} else if (!strcmp(argv[i], "-fpu")) {
			if (!strcmp(argv[i + 1], "softvfp")) {
				header->flags = (header->flags & ~KImageHWFloatMask);
			} else if (!strcmp(argv[i + 1], "vfpv2")) {
				header->flags = (header->flags & ~KImageHWFloatMask) | KImageHWFloat_VFPv2;
			} else {
				fprintf(stderr, "Unknown argument for parameter fpu: %s\n", argv[i + 1]);
				return 1;
			}
			i++;
		} else if (!strcmp(argv[i], "-heap")) {
			if (i + 2 < argc) {
				header->heapSizeMin = strtol(argv[i + 1], NULL, 0);
				header->heapSizeMax = strtol(argv[i + 2], NULL, 0);
				i += 2;
			} else {
				fprintf(stderr, "Not enough arguments for -heap\n");
				return 1;
			}
		} else if (!strcmp(argv[i], "-stack")) {
			header->stackSize = strtol(argv[i + 1], NULL, 0);
			i++;
		} else if (!strcmp(argv[i], "-nocompress")) {
			header->compressionType = 0;
		} else if (!strcmp(argv[i], "-compress")) {
			header->compressionType = KUidCompressionDeflate;
		} else if (!strcmp(argv[i], "-nocall") || !strcmp(argv[i], "-nocallentrypoint")) {
			header->flags |= KImageNoCallEntryPoint;
		} else if (!strcmp(argv[i], "-call") || !strcmp(argv[i], "-callentrypoint")) {
			header->flags &= ~KImageNoCallEntryPoint;
		} else if (!strcmp(argv[i], "-fixed")) {
		} else if (!strcmp(argv[i], "-allowdlldata")) {
			*dlldata = 1;
		} else if (!strcmp(argv[i], "-version")) {
			int major = 10, minor = 0;
			sscanf(argv[i + 1], "%d.%d", &major, &minor);
			header->moduleVersion = (major << 16) | (minor);
			i++;
		} else if (!strcmp(argv[i], "-compressionmethod")) {
			if (!strcmp(argv[i + 1], "none"))
				header->compressionType = 0;
			else if (!strcmp(argv[i + 1], "deflate"))
				header->compressionType = KUidCompressionDeflate;
			else if (!strcmp(argv[i + 1], "bytepair"))
				printf("Bytepair compression not supported!\n");
			else
				printf("Unknown compression method \"%s\"\n", argv[i + 1]);
			i++;
		} else if (!strcmp(argv[i], "-defaultpaged")) {
		} else if (!strcmp(argv[i], "-unpaged")) {
			header->flags |= KImageUnpaged;
		} else if (!strcmp(argv[i], "-sym_name_lkup")) {
			// FIXME?
		} else if (!strcmp(argv[i], "-debuggable")) {
			header->flags |= KImageDebuggable;
		} else if (!strcmp(argv[i], "-codepaging")) {
			if (!strcmp(argv[i + 1], "paged")) {
				header->flags |= KImagePaged;
			} else if (!strcmp(argv[i + 1], "unpaged")) {
				header->flags |= KImageUnpaged;
			} else if (!strcmp(argv[i + 1], "default")) {
				header->flags &= ~(KImageUnpaged | KImagePaged);
			} else {
				fprintf(stderr, "Unsupported codepaging value %s\n", optarg);
			}
			i++;
		} else if (!strcmp(argv[i], "-datapaging")) {
			if (!strcmp(argv[i + 1], "paged")) {
				header->flags |= KImageDataPaged;
			} else if (!strcmp(argv[i + 1], "unpaged")) {
				header->flags |= KImageDataUnpaged;
			} else if (!strcmp(argv[i + 1], "default")) {
				header->flags &= ~(KImageDataUnpaged | KImageDataPaged);
			} else {
				fprintf(stderr, "Unsupported codepaging value %s\n", optarg);
			}
			i++;
		} else if (!strcmp(argv[i], "-smpsafe")) {
			header->flags |= KImageSMPSafe;
		} else if (!strcmp(argv[i], "-dump")) {
			for (const char* c = argv[i + 1]; *c; c++) {
				if (tolower(*c) == 'h')
					*dumpFlags |= KDumpFlagHeader;
				else if (tolower(*c) == 's')
					*dumpFlags |= KDumpFlagSecurity;
				else if (tolower(*c) == 'c')
					*dumpFlags |= KDumpFlagCode;
				else if (tolower(*c) == 'd')
					*dumpFlags |= KDumpFlagData;
				else if (tolower(*c) == 'e')
					*dumpFlags |= KDumpFlagExport;
				else if (tolower(*c) == 'i')
					*dumpFlags |= KDumpFlagImport;
			}
			i++;
			continue;
		} else {
			if (argv[i][0] == '-') {
				fprintf(stderr, "Unhandled parameter %s?\n", argv[i]);
			}
			if (!*elfinput)
				*elfinput = argv[i];
			else if (!*output)
				*output = argv[i];
			continue;
		}
		*headersModified = true;
	}
	return 0;
}


int main(int argc, char *argv[]) {
	detectVersion();

//	int fixedaddress = 0;
//	int unfrozen = 0;
//	int noexportlibrary = 0;
	int dlldata = 0;

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
	if (epocVersion <= EPOC_VERSION_9_1)
		header.toolsVersion = 2 | (1<<8) | (549<<16);
	else if (epocVersion <= EPOC_VERSION_9_2)
		header.toolsVersion = 2 | (1<<8) | (564<<16);
	else if (epocVersion <= EPOC_VERSION_9_3)
		header.toolsVersion = 2 | (1<<8) | (576<<16);
	else if (epocVersion <= EPOC_VERSION_9_4)
		header.toolsVersion = 2 | (1<<8) | (596<<16);
	else
		header.toolsVersion = 2 | (2<<8) | (  0<<16);
	uint64_t timestamp = uint64_t(time(NULL))*1000000 + 0xDCDDB3E5D20000LL;
	header.timeLo = (timestamp >> 0) & 0xffffffff;
	header.timeHi = (timestamp >> 32) & 0xffffffff;
	header.flags = KImageImpFmt_ELF | KImageHdrFmt_V | KImageEpt_Eka2 | KImageABI_EABI;
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
	header.cpuIdentifier = ECpuArmV4;
	headerComp.uncompressedSize = 0;
	headerV.exceptionDescriptor = 0;
	headerV.spare2 = 0;
	headerV.exportDescSize = 0; // FIXME: do we need to set this sometime?
	headerV.exportDescType = KImageHdr_ExpD_NoHoles;
	headerV.exportDesc[0] = 0; // FIXME

	const char* output = NULL;
	const char* elfinput = NULL;
	int dumpFlags = 0;
	bool headersModified = false;

	int ret;
	if ((ret = parseOptions(argv, argc, &header, &headerComp, &headerV, &dlldata, &elfinput, &output, &dumpFlags, &headersModified)) != 0)
		return ret;

	if (elfinput && !output) {
		const char* e32input = elfinput;
		FILE* in = fopen(e32input, "r+b");
		if (!in) {
			perror(e32input);
			return 1;
		}
		readHeaders(in, &header, &headerComp, &headerV);
		if (headersModified) {
			output = elfinput = NULL;
			parseOptions(argv, argc, &header, &headerComp, &headerV, &dlldata, &elfinput, &output, &dumpFlags, &headersModified);
		}
		if (!headersModified || dumpFlags)
			dumpE32Image(e32input, in, dumpFlags, &header, &headerComp, &headerV);
		if (headersModified) {
			finalizeE32Image(in, &header, &headerComp, &headerV, e32input, false);
		} else
			fclose(in);
		return 0;
	}

	header.uidChecksum = uidCrc(header.uid1, header.uid2, header.uid3);

	if (!elfinput) {
		printf("nothing to do\n");
		return 1;
	}

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr, "Elf library out of date\n");
		return 1;
	}

	int fd = open(elfinput, O_RDONLY);
	if (fd < 0) {
		perror(elfinput);
		return 1;
	}

	unlink(output);
	FILE* out = fopen(output, "w+b");

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	if (!elf) {
		fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
		close(fd);
		return 1;
	}
	Elf_Scn* section = NULL;
	Elf32_Ehdr* ehdr = elf32_getehdr(elf);
	if (!ehdr) {
		fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
		elf_end(elf);
		close(fd);
		return 1;
	}
//	findExports(elf, &exportList);
	if (findSymbol(elf, "Symbian$$CPP$$Exception$$Descriptor", &headerV.exceptionDescriptor))
		headerV.exceptionDescriptor |= 1;

	Elf32_Phdr* phdr = elf32_getphdr(elf);
	Elf32_Phdr* dynamicPhdr = NULL;
	for (unsigned int i = 0; i < ehdr->e_phnum; i++, phdr++) {
		if ((phdr->p_type == PT_LOAD) && ((phdr->p_flags & (PF_R | PF_X)) == (PF_R | PF_X))) {
			header.codeBase = phdr->p_vaddr;
		} else if ((phdr->p_type == PT_LOAD) && ((phdr->p_flags & (PF_R | PF_W)) == (PF_R | PF_W))) {
			header.dataBase = phdr->p_vaddr;
			header.bssSize = phdr->p_memsz - phdr->p_filesz;
		} else if (phdr->p_type == PT_DYNAMIC) {
			dynamicPhdr = phdr;
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

	header.importOffset = ftell(out);

	section = NULL;
//	printf("code: %x - %x\n", header.codeBase, header.codeBase + header.codeSize);
//	printf("data: %x - %x\n", header.dataBase, header.dataBase + header.dataSize);
	parseDynamic(elf, dynamicPhdr, out, &header);

	if ((header.flags & KImageDll) && !dlldata) {
		// On S60 5.0, this is an error
		if (header.dataSize > 0) {
			printf("Dll %s has initialized data.\n", elfinput);
		} else if (header.bssSize > 0) {
			printf("Dll %s has uninitialized data.\n", elfinput);
		}
	}

	if (headerV.exceptionDescriptor)
		headerV.exceptionDescriptor -= header.codeBase;
	header.entryPoint = ehdr->e_entry - header.codeBase;

	fseek(out, header.importOffset, SEEK_SET);
	header.dllRefTableCount = importList.numLibraries();
//	printf("writing imports at %x\n", ftell(out));
	importList.write(out, false, false);

//	printf("writing relocs at %x\n", ftell(out));
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

	return 0;
}

