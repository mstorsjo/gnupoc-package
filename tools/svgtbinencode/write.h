/*
    Copyright 2007 Martin Storsjo

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
*/

#ifndef __WRITE_H
#define __WRITE_H

#include <stdio.h>
#include <stdint.h>

void writeUint8(uint8_t value, FILE* out);
void writeUint16(uint16_t value, FILE* out);
void writeUint32(uint32_t value, FILE* out);
void writeFloat(float f, FILE* out);
void writeFixed(const char* value, FILE* out);
void writePercentLength(const char* value, FILE* out);
void writeString(const char* value, int len, FILE* out);
void writeString(const char* value, FILE* out);

void writeViewBox(const char* value, FILE* out);
void writeColor(const char* value, FILE* out);
void writeFill(const char* value, FILE* out);
void writePathCommands(const char* value, FILE* out);

struct EnumString {
	const char* str;
	uint8_t value;
};

void writeEnumStringAttribute(const char* value, const EnumString* ptr, const char* attribName, FILE* out);

void writePreserveAspectRatio(const char* value, FILE* out);
void writeZoomAndPan(const char* value, FILE* out);
void writeGradientUnits(const char* value, FILE* out);
void writeSpreadMethod(const char* value, FILE* out);

void writeNull(const char* value, FILE* out);

#endif
