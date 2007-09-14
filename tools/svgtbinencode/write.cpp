/*
    Copyright 2007 Martin Storsjö

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

#include "write.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ConvertUTF.h"
#include "exceptions.h"

void writeUint8(uint8_t value, FILE* out) {
	if (fwrite(&value, 1, 1, out) != 1) {
		fprintf(stderr, "Unable to write\n");
		throw ErrWriteError;
	}
}

void writeUint16(uint16_t value, FILE* out) {
	uint8_t buf[] = { value & 0xff, (value >> 8) & 0xff };
	if (fwrite(buf, 1, 2, out) != 2) {
		fprintf(stderr, "Unable to write\n");
		throw ErrWriteError;
	}
}

void writeUint32(uint32_t value, FILE* out) {
	uint8_t buf[] = { value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, (value >> 24) & 0xff };
	if (fwrite(buf, 1, 4, out) != 4) {
		fprintf(stderr, "Unable to write\n");
		throw ErrWriteError;
	}
}


void writeFloat(float f, FILE* out) {
	int32_t intval = (int32_t) ((1<<16)*f);
	writeUint32(intval, out);
}

void writeFixed(const char* value, FILE* out) {
	float f;
	if (sscanf(value, "%f", &f) != 1) {
		fprintf(stderr, "Unable to parse \"%s\" as a floating point value\n", value);
		throw ErrBadAttribute;
	}
	writeFloat(f, out);
}

void writePercentLength(const char* value, FILE* out) {
	bool percent = strstr(value, "%") != NULL;
	writeUint8(percent ? 1 : 0, out);
	writeFixed(value, out);
}

void writeViewBox(const char* value, FILE* out) {
	float x, y, w, h;
	if (sscanf(value, "%f %f %f %f", &x, &y, &w, &h) != 4) {
		fprintf(stderr, "Unable to parse \"%s\" as a viewBox value\n", value);
		throw ErrBadAttribute;
	}
	writeFloat(x, out);
	writeFloat(y, out);
	writeFloat(w, out);
	writeFloat(h, out);
}

void writeString(const char* value, int len, FILE* out) {
	UTF16* utf16String = new UTF16[len];
	const UTF8* sourceStart = (UTF8*) value;
	UTF8* sourceEnd = (UTF8*) value + len;
	UTF16* targetStart = utf16String;
	UTF16* targetEnd = utf16String + len;
	if (ConvertUTF8toUTF16(&sourceStart, sourceEnd, &targetStart, targetEnd, lenientConversion) != conversionOK) {
		fprintf(stderr, "Unable to interpret argument as UTF8 text\n");
		throw ErrBadUTF8;
	}
	uint32_t datalen = (intptr_t) ((uint8_t*)targetStart - (uint8_t*)utf16String);
	writeUint8(datalen, out);
	for (UTF16* ptr = utf16String; ptr < targetStart; ptr++)
		writeUint16(*ptr, out);
	delete [] utf16String;
}

void writeString(const char* value, FILE* out) {
	int len = strlen(value);
	writeString(value, len, out);
}

void writeColor(const char* value, FILE* out) {
	uint32_t c1, c2, c3, c4;
	int parsed;
	if ((parsed = sscanf(value, "#%02x%02x%02x%02x", &c1, &c2, &c3, &c4)) < 2) {
		fprintf(stderr, "Unable to parse \"%s\" as a color\n", value);
		throw ErrBadAttribute;
	}
	uint8_t r, g, b, a;
	if (parsed == 2) {
		r = (c1 >> 4) & 0xf;
		g = c1 & 0xf;
		b = c2 & 0xf;
		r |= r << 4;
		g |= g << 4;
		b |= b << 4;
	} else if (parsed == 3) {
		r = c1; g = c2; b = c3; a = 0;
	} else if (parsed == 4) {
		a = c1; r = c2; g = c3; b = c4;
	}
	uint32_t val = (a << 24) | (r << 16) | (g << 8) | b;
	writeUint32(val, out);
}

void writeFill(const char* value, FILE* out) {
	const char* ptr;
	if ((ptr = strstr(value, "url(#")) != NULL) {
		ptr += 5;
		const char* endPtr = strrchr(ptr, ')');
		if (endPtr) {
			int len = (intptr_t) (endPtr - ptr);
			writeUint8(1, out);
			writeString(ptr, len, out);
		} else {
			fprintf(stderr, "Unable to parse \"%s\" as url fill\n", value);
			throw ErrBadAttribute;
		}
	} else {
		writeUint8(0, out);
		writeColor(value, out);
	}
}

const EnumString preserveAspectRatio[] = {
	{ "none", 0 },
	{ NULL, 0 }
};

const EnumString zoomAndPan[] = {
	{ "disable", 0 },
	{ "magnify", 1 },
	{ NULL, 0 }
};

const EnumString gradientUnits[] = {
	{ "userSpaceOnUse", 0 },
	{ "objectBoundingBox", 1 },
	{ NULL, 0 }
};

const EnumString spreadMethod[] = {
	{ "pad", 0 },
	{ "reflect", 1 },
	{ "repeat", 2 },
	{ NULL, 0 }
};

void writeEnumStringAttribute(const char* value, const EnumString* ptr, const char* attribName, FILE* out) {
	while (ptr->str) {
		if (!strcmp(ptr->str, value)) {
			writeUint8(ptr->value, out);
			return;
		}
		ptr++;
	}
	fprintf(stderr, "Unable to parse \"%s\" as a %s value\n", value, attribName);
	throw ErrBadAttribute;
}

void writePreserveAspectRatio(const char* value, FILE* out) {
	writeEnumStringAttribute(value, preserveAspectRatio, "preserveAspectRatio", out);
}

void writeZoomAndPan(const char* value, FILE* out) {
	writeEnumStringAttribute(value, zoomAndPan, "zoomAndPan", out);
}

void writeGradientUnits(const char* value, FILE* out) {
	writeEnumStringAttribute(value, gradientUnits, "gradientUnits", out);
}

void writeSpreadMethod(const char* value, FILE* out) {
	writeEnumStringAttribute(value, spreadMethod, "spreadMethod", out);
}

void writeNull(const char* value, FILE* out) {
	fprintf(stderr, "Unimplemented attribute with content \"%s\"\n", value);
	throw ErrUnimplemented;
}

