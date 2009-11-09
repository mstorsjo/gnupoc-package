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

#ifndef __E32IMAGE_H
#define __E32IMAGE_H

#include <stdint.h>
#include <stdio.h>

struct E32ImageHeader {
	uint32_t uid1;
	uint32_t uid2;
	uint32_t uid3;
	uint32_t uidChecksum;
	uint32_t signature;
	uint32_t headerCrc;
	uint32_t moduleVersion;
	uint32_t compressionType;
	uint32_t toolsVersion;
	uint32_t timeLo;
	uint32_t timeHi;
	uint32_t flags;
	int32_t codeSize;
	int32_t dataSize;
	int32_t heapSizeMin;
	int32_t heapSizeMax;
	int32_t stackSize;
	int32_t bssSize;
	uint32_t entryPoint;
	uint32_t codeBase;
	uint32_t dataBase;
	int32_t dllRefTableCount;
	uint32_t exportDirOffset;
	int32_t exportDirCount;
	int32_t textSize;
	uint32_t codeOffset;
	uint32_t dataOffset;
	uint32_t importOffset;
	uint32_t codeRelocOffset;
	uint32_t dataRelocOffset;
	uint16_t processPriority;
	uint16_t cpuIdentifier;
};

struct E32ImageHeaderComp {
	uint32_t uncompressedSize;
};

struct E32ImageHeaderV {
	uint32_t secureId;
	uint32_t vendorId;
	uint32_t caps[2];
	uint32_t exceptionDescriptor;
	uint32_t spare2;
	uint16_t exportDescSize;
	uint8_t exportDescType;
	uint8_t exportDesc[1];
};

void writeUint32(uint32_t value, FILE* out);
void writeUint16(uint16_t value, FILE* out);
void writeUint8(uint8_t value, FILE* out);

void writeHeaders(FILE* out, E32ImageHeader* header, E32ImageHeaderComp* headerComp, E32ImageHeaderV* headerV);

struct Capability {
	const char* name;
	int bit;
};
extern const Capability capabilityNames[];

int findCapabilityBit(const char* name);

/*
void setBit(int bit, uint32_t* caps);
void clearBit(int bit, uint32_t* caps);
*/
void setCapability(const char* name, uint32_t* caps);
void clearCapability(const char* name, uint32_t* caps);

void getCapabilities(char* str, uint32_t* caps);

#define KImageHWFloatMask	0x00f00000
#define KImageHWFloat_VFPv2	0x00100000
#define KImageImpFmt_ELF	0x10000000
#define KImageHdrFmt_V		0x02000000
#define KImageEpt_Eka2		0x00000020
#define KImageABI_EABI		0x00000008
#define KImageNoCallEntryPoint	0x00000002
#define KImageDll		0x00000001
#define KImageOldJFlag		0x00000008
#define EPriorityForeground	350
#define ECpuArmV5		0x2001
#define KImageHdr_ExpD_NoHoles		0x00
#define KImageHdr_ExpD_FullBitmap	0x01
#define KImageCrcInitialiser	0xc90fdaa2
#define KTextRelocType		0x1000
#define KDataRelocType		0x2000

#define KUidCompressionDeflate	0x101f7afc

uint32_t uidCrc(uint32_t uid1, uint32_t uid2, uint32_t uid3);


#endif
