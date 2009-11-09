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

#include "e32image.h"
#include "caseinsensitive.h"
#include <string.h>

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

void writeHeaders(FILE* out, E32ImageHeader* header, E32ImageHeaderComp* headerComp, E32ImageHeaderV* headerV) {
	writeUint32(header->uid1, out);
	writeUint32(header->uid2, out);
	writeUint32(header->uid3, out);
	writeUint32(header->uidChecksum, out);
	writeUint32(header->signature, out);
	writeUint32(header->headerCrc, out);
	writeUint32(header->moduleVersion, out);
	writeUint32(header->compressionType, out);
	writeUint32(header->toolsVersion, out);
	writeUint32(header->timeLo, out);
	writeUint32(header->timeHi, out);
	writeUint32(header->flags, out);
	writeUint32(header->codeSize, out);
	writeUint32(header->dataSize, out);
	writeUint32(header->heapSizeMin, out);
	writeUint32(header->heapSizeMax, out);
	writeUint32(header->stackSize, out);
	writeUint32(header->bssSize, out);
	writeUint32(header->entryPoint, out);
	writeUint32(header->codeBase, out);
	writeUint32(header->dataBase, out);
	writeUint32(header->dllRefTableCount, out);
	writeUint32(header->exportDirOffset, out);
	writeUint32(header->exportDirCount, out);
	writeUint32(header->textSize, out);
	writeUint32(header->codeOffset, out);
	writeUint32(header->dataOffset, out);
	writeUint32(header->importOffset, out);
	writeUint32(header->codeRelocOffset, out);
	writeUint32(header->dataRelocOffset, out);
	writeUint16(header->processPriority, out);
	writeUint16(header->cpuIdentifier, out);
	writeUint32(headerComp->uncompressedSize, out);
	writeUint32(headerV->secureId, out);
	writeUint32(headerV->vendorId, out);
	writeUint32(headerV->caps[0], out);
	writeUint32(headerV->caps[1], out);
	writeUint32(headerV->exceptionDescriptor, out);
	writeUint32(headerV->spare2, out);
	writeUint16(headerV->exportDescSize, out);
	writeUint8(headerV->exportDescType, out);
	writeUint8(headerV->exportDesc[0], out);
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

void getCapabilities(char* str, uint32_t* caps) {
	memset(caps, 0, 2);
	char* ptr = str;
	bool set = true;
	char* start = str;
	while (*ptr) {
		if (*ptr == '+') {
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

