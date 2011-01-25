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

#include "e32image.h"
#include "caseinsensitive.h"
#include <string.h>
#include "deflate.h"
#include <zlib.h>
#include <fstream>
#include <stdlib.h>

using namespace std;
void DeflateCompress(char* bytes, TInt size, ostream& os);

extern "C" {
#include "crc.h"
}

void writeUint32(uint32_t value, FILE* out) {
	uint8_t buf[] = { value >> 0, value >> 8, value >> 16, value >> 24 };
	fwrite(buf, 1, 4, out);
}

void writeUint16(uint16_t value, FILE* out) {
	uint8_t buf[] = { value >> 0, value >> 8 };
	fwrite(buf, 1, 2, out);
}

void writeUint8(uint8_t value, FILE* out) {
	fwrite(&value, 1, 1, out);
}

uint32_t readUint32(FILE* in) {
	uint8_t buf[4];
	fread(buf, 1, 4, in);
	return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

uint16_t readUint16(FILE* in) {
	uint8_t buf[2];
	fread(buf, 1, 2, in);
	return buf[0] | (buf[1] << 8);
}

uint8_t readUint8(FILE* in) {
	uint8_t buf[1];
	fread(buf, 1, 1, in);
	return buf[0];
}

void writeHeaders(FILE* out, const E32ImageHeader* header, const E32ImageHeaderComp* headerComp, const E32ImageHeaderV* headerV) {
	writeUint32(header->uid1, out);		// 0
	writeUint32(header->uid2, out);		// 4
	writeUint32(header->uid3, out);		// 8
	writeUint32(header->uidChecksum, out);	// c
	writeUint32(header->signature, out);	// 10
	writeUint32(header->headerCrc, out);	// 14
	writeUint32(header->moduleVersion, out);	// 18
	writeUint32(header->compressionType, out);	// 1c
	writeUint32(header->toolsVersion, out);	// 20
	writeUint32(header->timeLo, out);	// 24
	writeUint32(header->timeHi, out);	// 28
	writeUint32(header->flags, out);	// 2c
	writeUint32(header->codeSize, out);	// 30
	writeUint32(header->dataSize, out);	// 34
	writeUint32(header->heapSizeMin, out);	// 38
	writeUint32(header->heapSizeMax, out);	// 3c
	writeUint32(header->stackSize, out);	// 40
	writeUint32(header->bssSize, out);	// 44
	writeUint32(header->entryPoint, out);	// 48
	writeUint32(header->codeBase, out);	// 4c
	writeUint32(header->dataBase, out);	// 50
	writeUint32(header->dllRefTableCount, out);	// 54
	writeUint32(header->exportDirOffset, out);	// 58
	writeUint32(header->exportDirCount, out);	// 5c
	writeUint32(header->textSize, out);	// 60
	writeUint32(header->codeOffset, out);	// 64
	writeUint32(header->dataOffset, out);	// 68
	writeUint32(header->importOffset, out);	// 6c
	writeUint32(header->codeRelocOffset, out);	// 70
	writeUint32(header->dataRelocOffset, out);	// 74
	writeUint16(header->processPriority, out);	// 78
	writeUint16(header->cpuIdentifier, out);	// 7a
	writeUint32(headerComp->uncompressedSize, out);	// 7c
	writeUint32(headerV->secureId, out);	// 80
	writeUint32(headerV->vendorId, out);	// 84
	writeUint32(headerV->caps[0], out);	// 88
	writeUint32(headerV->caps[1], out);	// 8c
	writeUint32(headerV->exceptionDescriptor, out);	// 90
	writeUint32(headerV->spare2, out);	// 94
	writeUint16(headerV->exportDescSize, out);	// 98
	writeUint8(headerV->exportDescType, out);	// 9a
	writeUint8(headerV->exportDesc[0], out);	// 9b
}

void readHeaders(FILE* in, E32ImageHeader* header, E32ImageHeaderComp* headerComp, E32ImageHeaderV* headerV) {
	header->uid1                 = readUint32(in);
	header->uid2                 = readUint32(in);
	header->uid3                 = readUint32(in);
	header->uidChecksum          = readUint32(in);
	header->signature            = readUint32(in);
	header->headerCrc            = readUint32(in);
	header->moduleVersion        = readUint32(in);
	header->compressionType      = readUint32(in);
	header->toolsVersion         = readUint32(in);
	header->timeLo               = readUint32(in);
	header->timeHi               = readUint32(in);
	header->flags                = readUint32(in);
	header->codeSize             = readUint32(in);
	header->dataSize             = readUint32(in);
	header->heapSizeMin          = readUint32(in);
	header->heapSizeMax          = readUint32(in);
	header->stackSize            = readUint32(in);
	header->bssSize              = readUint32(in);
	header->entryPoint           = readUint32(in);
	header->codeBase             = readUint32(in);
	header->dataBase             = readUint32(in);
	header->dllRefTableCount     = readUint32(in);
	header->exportDirOffset      = readUint32(in);
	header->exportDirCount       = readUint32(in);
	header->textSize             = readUint32(in);
	header->codeOffset           = readUint32(in);
	header->dataOffset           = readUint32(in);
	header->importOffset         = readUint32(in);
	header->codeRelocOffset      = readUint32(in);
	header->dataRelocOffset      = readUint32(in);
	header->processPriority      = readUint16(in);
	header->cpuIdentifier        = readUint16(in);
	headerComp->uncompressedSize = readUint32(in);
	headerV->secureId            = readUint32(in);
	headerV->vendorId            = readUint32(in);
	headerV->caps[0]             = readUint32(in);
	headerV->caps[1]             = readUint32(in);
	headerV->exceptionDescriptor = readUint32(in);
	headerV->spare2              = readUint32(in);
	headerV->exportDescSize      = readUint16(in);
	headerV->exportDescType      = readUint8(in);
	headerV->exportDesc[0]       = readUint8(in);
}

const Capability capabilityNames[] = {
	{ "TCB",		0 },
	{ "CommDD",		1 },
	{ "PowerMgmt",		2 },
	{ "MultimediaDD",	3 },
	{ "ReadDeviceData",	4 },
	{ "WriteDeviceData",	5 },
	{ "DRM",		6 },
	{ "TrustedUI",		7 },
	{ "ProtServ",		8 },
	{ "DiskAdmin",		9 },
	{ "NetworkControl",	10 },
	{ "AllFiles",		11 },
	{ "SwEvent",		12 },
	{ "NetworkServices",	13 },
	{ "LocalServices",	14 },
	{ "ReadUserData",	15 },
	{ "WriteUserData",	16 },
	{ "Location",		17 },
	{ "SurroundingsDD",	18 },
	{ "UserEnvironment",	19 },
	{ NULL,			0 }
};

int findCapabilityBit(const char* name) {
	const Capability* cap = capabilityNames;
	while (cap->name) {
		if (!stricmp(name, cap->name))
			return cap->bit;
		cap++;
	}
	return -1;
}

const char* findCapabilityName(int bit) {
	const Capability* cap = capabilityNames;
	while (cap->name) {
		if (cap->bit == bit)
			return cap->name;
		cap++;
	}
	return NULL;
}

void setBit(int bit, uint32_t* caps) {
	if (bit >= 0 && bit < 32)
		caps[0] |= 1<<bit;
	else if (bit >= 32 && bit < 64)
		caps[1] |= 1<<(bit-32);
}

void clearBit(int bit, uint32_t* caps) {
	if (bit >= 0 && bit < 32)
		caps[0] &= ~(1<<bit);
	else if (bit >= 32 && bit < 64)
		caps[1] &= ~(1<<(bit-32));
}

void setCapability(const char* name, uint32_t* caps) {
	if (!stricmp(name, "all")) {
		const Capability* cap = capabilityNames;
		while (cap->name) {
			setBit(cap->bit, caps);
			cap++;
		}
	} else if (!stricmp(name, "none")) {
	} else {
		int bit = findCapabilityBit(name);
		if (bit >= 0)
			setBit(bit, caps);
	}
}

void clearCapability(const char* name, uint32_t* caps) {
	int bit = findCapabilityBit(name);
	if (bit >= 0)
		clearBit(bit, caps);
}

void getCapabilities(const char* origStr, uint32_t* caps) {
	char* str = strdup(origStr);
	memset(caps, 0, 2);
	char* ptr = str;
	bool set = true;
	char* start = str;
	while (*ptr) {
		if (*ptr == '+' || *ptr == ' ') {
			*ptr = '\0';
			if (set)
				setCapability(start, caps);
			else	
				clearCapability(start, caps);
			ptr++;
			start = ptr;
			set = true;
		} else if (*ptr == '-') {
			*ptr = '\0';
			if (set)
				setCapability(start, caps);
			else	
				clearCapability(start, caps);
			ptr++;
			start = ptr;
			set = false;
		} else {
			ptr++;
		}
	}
	if (set)
		setCapability(start, caps);
	else	
		clearCapability(start, caps);
	free(str);
}

uint32_t uidCrc(uint32_t uid1, uint32_t uid2, uint32_t uid3) {
	uint8_t buf1[] = { (uid1 >> 8), (uid1 >> 24), (uid2 >> 8), (uid2 >> 24), (uid3 >> 8), (uid3 >> 24) };
	uint8_t buf2[] = { (uid1 >> 0), (uid1 >> 16), (uid2 >> 0), (uid2 >> 16), (uid3 >> 0), (uid3 >> 16) };
//	uint32_t crc1 = crc32(0, buf1, 6);
//	uint32_t crc2 = crc32(0, buf2, 6);
	uint16_t crc1 = crcSlow(buf1, 6);
	uint16_t crc2 = crcSlow(buf2, 6);
	return (crc1<<16) | crc2;
}


void finalizeE32Image(FILE* out, E32ImageHeader* header, const E32ImageHeaderComp* headerComp, const E32ImageHeaderV* headerV, const char* filename, bool compress) {
	fseek(out, 0, SEEK_SET);
	header->headerCrc = KImageCrcInitialiser;
#define CRCSIZE 0x9c
	writeHeaders(out, header, headerComp, headerV);
	uint8_t buf[CRCSIZE];
	fseek(out, 0, SEEK_SET);
	fread(buf, 1, CRCSIZE, out);
	header->headerCrc = ~crc32(0xffffffff, buf, CRCSIZE);
	fseek(out, 0, SEEK_SET);
	writeHeaders(out, header, headerComp, headerV);

	if (header->compressionType == KUidCompressionDeflate && compress) {
		fseek(out, 0, SEEK_END);
		uint32_t len = ftell(out);
		len -= CRCSIZE;
		uint8_t* headerData = (uint8_t*) malloc(CRCSIZE);
		uint8_t* data = (uint8_t*) malloc(len);
		fseek(out, 0, SEEK_SET);
		fread(headerData, 1, CRCSIZE, out);
		fread(data, 1, len, out);
		fclose(out);

		ofstream stream(filename, ios_base::binary | ios_base::out);
		stream.write((const char*) headerData, CRCSIZE);
		DeflateCompress((char*) data, len, stream);
		stream.close();

		free(data);
		free(headerData);
	} else {
		fclose(out);
	}
}

void dumpE32Image(const char* filename, FILE* in, int flags, const E32ImageHeader* header, const E32ImageHeaderComp* headerComp, const E32ImageHeaderV* headerV) {
	if (!flags)
		flags = KDumpFlagHeader | KDumpFlagSecurity | KDumpFlagCode | KDumpFlagData | KDumpFlagExport | KDumpFlagImport;

	FILE* out = stdout;
	fprintf(out, "E32ImageFile '%s'\n", filename);
	if (flags & KDumpFlagHeader) {
		fprintf(out, "V%d.%02d(%d)\tTime Stamp: %08x,%08x\n", header->toolsVersion & 0xff, (header->toolsVersion >> 8) & 0xff, (header->toolsVersion >> 16) & 0xffff, header->timeHi, header->timeLo);
		fprintf(out, "EPOC %s for %s CPU\n", header->flags & KImageDll ? "Dll" : "Exe", header->cpuIdentifier == ECpuArmV4 ? "ARMV4" : (header->cpuIdentifier == ECpuArmV5 ? "ARMV5" : "Unknown"));
		fprintf(out, "Flags:\t%08x\n", header->flags);
		fprintf(out, "Priority %s\n", header->processPriority == EPriorityForeground ? "Foreground" : "Unknown");
		// Entry points are not called
		// Image header is format 2
		if (!header->compressionType)
			fprintf(out, "Image is not compressed\n");
		else if (header->compressionType == KUidCompressionDeflate)
			fprintf(out, "Image is compressed using the DEFLATE algorithm\n");
		else
			fprintf(out, "Image is compressed using an unknown algorithm\n");
		if (header->compressionType)
			fprintf(out, "Uncompressed size %08x\n", headerComp->uncompressedSize + CRCSIZE);
		if (!(header->flags & KImageHWFloatMask))
			fprintf(out, "Image FPU support : Soft VFP\n");
		else
			fprintf(out, "Image FPU support : Other\n");
		fprintf(out, "Pageability : %s\n", (header->flags & KImageUnpaged) ? "Unpaged" : "Default");
	}
	if (flags & (KDumpFlagHeader | KDumpFlagSecurity)) {
		fprintf(out, "Secure ID: %08x\n", headerV->secureId);
		fprintf(out, "Vendor ID: %08x\n", headerV->vendorId);
		fprintf(out, "Capabilities: %08x %08x\n", headerV->caps[1], headerV->caps[0]);
		if (flags & KDumpFlagSecurity) {
			for (int i = 0; i < 64; i++) {
				uint32_t val = i < 32 ? headerV->caps[0] : headerV->caps[1];
				if (val & (1 << (i & 31))) {
					const char* name = findCapabilityName(i);
					if (name)
						fprintf(out, "              %s\n", name);
				}
			}
		}
	}
	if (flags & KDumpFlagHeader) {
		fprintf(out, "Exception Descriptor Offset:  %08x\n", headerV->exceptionDescriptor);
		//fprintf(out, "Exception Index Table Base: %08x\n", 0);
		//fprintf(out, "Exception Index Table Limit: %08x\n", 0);
		//fprintf(out, "RO Segment Base: %08x\n", 0);
		//fprintf(out, "RO Segment Limit: %08x\n", 0);
		//fprintf(out, "Export Description: Size=%03x, Type=%02x\n", 0, 0);
		//fprintf(out, "\n");
		//fprintf(out, "Export description consistent\n");
		fprintf(out, "Module Version: %d.%d\n", (header->moduleVersion >> 16) & 0xffff, header->moduleVersion & 0xffff);
		//fprintf(out, "Imports are ELF-style\n");
		//fprintf(out, "ARM EABI\n");
		//fprintf(out, "Built against EKA2\n");
		fprintf(out, "Uids:\t\t%08x %08x %08x (%08x)\n", header->uid1, header->uid2, header->uid3, header->uidChecksum);
		fprintf(out, "Header CRC:\t%08x\n", header->headerCrc);
		fprintf(out, "File Size:\t%08x\n", headerComp->uncompressedSize + CRCSIZE);
		fprintf(out, "Code Size:\t%08x\n", header->codeSize);
		fprintf(out, "Data Size:\t%08x\n", header->dataSize);
		fprintf(out, "Compression:\t%08x\n", header->compressionType);
		fprintf(out, "Min Heap Size:\t%08x\n", header->heapSizeMin);
		fprintf(out, "Max Heap Size:\t%08x\n", header->heapSizeMax);
		fprintf(out, "Stack Size:\t%08x\n", header->stackSize);
		fprintf(out, "Code link addr:\t%08x\n", header->codeBase);
		fprintf(out, "Data link addr:\t%08x\n", header->dataBase);
		fprintf(out, "Code reloc offset:\t%08x\n", header->codeRelocOffset);
		fprintf(out, "Data reloc offset:\t%08x\n", header->dataRelocOffset);
	}
	fprintf(out, "\n");
}

