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

#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>



enum SvgbError {
	ErrEof = 0,
	ErrExpected,
	ErrElementNotFound,
	ErrAttributeNotFound,
	ErrPathCommandNotFound,
	ErrBadPathCommands,
	ErrUnimplemented
};

void expectByte(uint8_t expected, FILE* in) {
	uint8_t byte;
	if (fread(&byte, 1, 1, in) != 1)
		throw ErrEof;
	if (byte != expected) {
		fprintf(stderr, "Expected %x\n", expected);
		throw ErrExpected;
	}
}

uint8_t readByte(FILE* in) {
	uint8_t byte;
	if (fread(&byte, 1, 1, in) != 1)
		throw ErrEof;
	return byte;
}

uint16_t readUint16(FILE* in) {
	uint8_t buf[2];
	if (fread(buf, 1, 2, in) != 2)
		throw ErrEof;
	return buf[0] | (buf[1] << 8);
}

uint32_t readUint32(FILE* in) {
	uint8_t buf[4];
	if (fread(buf, 1, 4, in) != 4)
		throw ErrEof;
	return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

float readFixed32(FILE* in) {
	uint32_t i = readUint32(in);
	float fact = 1.0f/(1<<16);
	return float(i)*fact;
}

static const uint8_t header[] = { 0xce, 0x56, 0xfa, 0x03 };

void expectHeader(FILE* in) {
	readByte(in);
	for (int i = 1; i < 4; i++)
		expectByte(header[i], in);
}

const struct ElementName {
	const char* name;
	uint8_t id;
} elementNames[] = {
	{ "svg", 0x00 },
	{ "defs", 0x03 },
	{ "path", 0x1e },
	{ "a", 0x12 },
	{ "animate", 0x22 },
	{ "animateColor", 0x23 },
	{ "animateMotion", 0x24 }, // an empty tag contains 7a 00 00 00 00 00 94
	{ "animateTransform", 0x25 },
	{ "set", 0x26 },
	{ "mpath", 0x27 },
	{ "circle", 0x1b },
	{ "desc", 0x04 },
	{ "ellipse", 0x1c },
	{ "g", 0x0b },
	{ "title", 0x07 },
	{ "use", 0x1a },
	{ "font-face", 0x14 },
	{ "font-face-name", 0x08 },
	{ "font-face-src", 0x09 },
	{ "glyph", 0x15 },
	{ "image", 0x16 },
	{ "line", 0x1d },
	{ "missing-glyph", 0x17 },
	{ "polygon", 0x1f },
	{ "polyline", 0x20 },
	{ "rect", 0x21 },
	{ "switch", 0x0f },
	{ "text", 0x19 }, // followed by e8 03 fd 00 fe
	{ "linearGradient", 0x28 },
	{ "stop", 0x2a },
	{ "radialGradient", 0x29 },
	{ NULL, 0 }
};

typedef void (*parseAttribFunc)(FILE* in, FILE* out);

void parseFixed(FILE* in, FILE* out) {
	float val = readFixed32(in);
	fprintf(out, "%f", val);
}

void parsePercentLength(FILE* in, FILE* out) {
	uint8_t flag = readByte(in);
	float val = readFixed32(in);
	fprintf(out, "%f", val);
	if (flag)
		fprintf(out, "%%");
}

void parseViewBox(FILE* in, FILE* out) {
	float x = readFixed32(in);
	float y = readFixed32(in);
	float w = readFixed32(in);
	float h = readFixed32(in);
	fprintf(out, "%f %f %f %f", x, y, w, h);
}

void parseString(FILE* in, FILE* out) {
	uint8_t len = readByte(in);
	len /= 2;
	wchar_t* str = new wchar_t[len + 1];
	for (uint8_t i = 0; i < len; i++)
		str[i] = readUint16(in);
	str[len] = '\0';
	int size = sizeof(wchar_t)*(len + 1);
	char* utf8Str = new char[size];
	wcstombs(utf8Str, str, size); // FIXME: convert to real utf8, regardless of locale
	fprintf(out, "%s", utf8Str);
	delete [] utf8Str;
	delete [] str;
}

void parseColor(FILE* in, FILE* out) {
	uint32_t color = readUint32(in);
	fprintf(out, "#");
	if ((color >> 24) & 0xff)
		fprintf(out, "%02x", (color >> 24) & 0xff);
	fprintf(out, "%02x%02x%02x", (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

void parseFill(FILE* in, FILE* out) {
	uint8_t byte = readByte(in);
	if (byte == 0)
		parseColor(in, out);
	if (byte == 1) {
		fprintf(out, "url(#");
		parseString(in, out);
		fprintf(out, ")");
	}
}

const struct PathCommand {
	char commandName;
	uint8_t command;
	int consume;
} pathCommands[] = {
	{ 'M', 0x00, 2 },
	{ 'L', 0x01, 2 },
	{ 'Q', 0x02, 4 },
	{ 'C', 0x03, 6 },
	{ 'z', 0x04, 0 },
	{ 0, 0, 0 }
};

const PathCommand* findCommand(uint8_t id) {
	const PathCommand* ptr = pathCommands;
	while (ptr->commandName) {
		if (ptr->command == id)
			return ptr;
		ptr++;
	}
	throw ErrPathCommandNotFound;
}

void parsePathCommands(FILE* in, FILE* out) {
	uint16_t numCommands = readUint16(in);
	uint8_t* commands = new uint8_t[numCommands];
	for (uint16_t i = 0; i < numCommands; i++)
		commands[i] = readByte(in);
	uint16_t numCoords = readUint16(in);
	float* coords = new float[numCoords];
	for (uint16_t i = 0; i < numCoords; i++)
		coords[i] = readFixed32(in);

	uint16_t index = 0;
	for (uint16_t i = 0; i < numCommands; i++) {
		const PathCommand* command = findCommand(commands[i]);
		fprintf(out, "%c ", command->commandName);
		if (index + command->consume > numCoords)
			throw ErrBadPathCommands;
		for (int j = 0; j < command->consume; j++)
			fprintf(out, "%f ", coords[index++]);
	}
	delete [] commands;
	delete [] coords;

}

void parsePreserveAspectRatio(FILE* in, FILE* out) {
	uint8_t val = readByte(in);
	if (val == 0)
		fprintf(out, "none");
}

void parseZoomAndPan(FILE* in, FILE* out) {
	uint8_t val = readByte(in);
	if (val == 0)
		fprintf(out, "disable");
	else if (val == 1)
		fprintf(out, "magnify");
}

void parseGradientUnits(FILE* in, FILE* out) {
	uint8_t val = readByte(in);
	if (val == 0)
		fprintf(out, "userSpaceOnUse");
	else if (val == 1)
		fprintf(out, "objectBoundingBox");
}

void parseSpreadMethod(FILE* in, FILE* out) {
	uint8_t val = readByte(in);
	if (val == 0)
		fprintf(out, "pad");
	if (val == 1)
		fprintf(out, "reflect");
	if (val == 2)
		fprintf(out, "repeat");
}

void parseNull(FILE* in, FILE* out) {
	fprintf(stderr, "Unimplemented!\n");
	throw ErrUnimplemented;
}

const struct Attribute {
	const char* name;
	uint16_t id;
	parseAttribFunc func;
	const char* element;
} attributes[] = {
	{ "version", 0x004c, parseFixed, NULL },
	{ "x", 0x0031, parseFixed, NULL },
	{ "y", 0x0030, parseFixed, NULL },
	{ "x1", 0x0034, parseFixed, NULL },
	{ "y1", 0x0032, parseFixed, NULL },
	{ "x2", 0x0035, parseFixed, NULL },
	{ "y2", 0x0033, parseFixed, NULL },
	{ "gradientUnits", 0x0056, parseGradientUnits, NULL },
	{ "gradientTransform", 0x007b, parseNull, NULL },
	{ "spreadMethod", 0x0055, parseSpreadMethod, NULL },
	{ "xlink:href", 0x006d, parseNull, NULL }, // 00
	{ "offset", 0x0054, parseFixed, NULL },
	{ "stop-color", 0x0051, parseColor, NULL },
	{ "stop-opacity", 0x0057, parseFixed, NULL },
	{ "width", 0x001a, parsePercentLength, "svg" },
	{ "height", 0x001b, parsePercentLength, "svg" },
	{ "width", 0x001a, parseFixed, NULL },
	{ "height", 0x001b, parseFixed, NULL },
	{ "rx", 0x001d, parseFixed, NULL },
	{ "ry", 0x001e, parseFixed, NULL },
	{ "viewBox", 0x0058, parseViewBox, NULL },
	{ "preserveAspectRatio", 0x005b, parsePreserveAspectRatio, NULL },
	{ "zoomAndPan", 0x005a, parseZoomAndPan, NULL },
	{ "baseProfile", 0x0059, parseString, NULL },
	{ "id", 0x005c, parseString, NULL },
	{ "fill", 0x0000, parseFill, NULL },
	{ "fill-opacity", 0x0016, parseFixed, NULL },
	{ "color", 0x000f, parseColor, NULL },
	{ "d", 0x004f, parsePathCommands, NULL },
	{ "transform", 0x0042, parseNull, NULL }, // 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00
	{ "xml:base", 0x005d, parseString, NULL },
	{ "xml:lang", 0x005e, parseString, NULL },
	{ "xml:space", 0x005f, parseString, NULL },
	{ "requiredFeatures", 0x0061, parseString, NULL },
	{ "requiredExtensions", 0x0060, parseString, NULL },
	{ "systemLanguage", 0x0062, parseString, NULL },
	{ "fill-rule", 0x000a, parseString, NULL },
	{ "stroke", 0x0001, parseColor, NULL },
	{ "stroke-dasharray", 0x0008, parseNull, NULL }, // 00
	{ "stroke-dashoffset", 0x000d, parseNull, NULL }, // 00 00 00 00
	{ "stroke-linecap", 0x000b, parseString, NULL },
	{ "stroke-miterlimit", 0x000e, parseNull, NULL }, // 00 00 00 00
	{ "stroke-width", 0x0002, parseFixed, NULL },
	{ NULL, 0, NULL, NULL }
};

#define END_ELEMENT 0xfe
#define END_FILE 0xff
#define END_ATTRIBUTES 0x03e8

const char* getElementName(uint8_t id) {
	const struct ElementName* ptr = elementNames;
	while (ptr->name) {
		if (ptr->id == id) {
			return ptr->name;
		}
		ptr++;
	}
	fprintf(stderr, "Element for id %x not found\n", id);
	throw ErrElementNotFound;
	return NULL;
}

void findParseAttribute(FILE* in, FILE* out, uint16_t id, const char* parent) {
	const struct Attribute* ptr = attributes;
	while (ptr->name) {
		if (ptr->id == id) {
			if (ptr->element) {
				if (strcmp(ptr->element, parent)) {
					ptr++;
					continue;
				}
			}
			fprintf(out, " %s=\"", ptr->name);
			ptr->func(in, out);
			fprintf(out, "\"");
			return;
		}
		ptr++;
	}
	fprintf(stderr, "Attribute for id %x not found\n", id);
	throw ErrAttributeNotFound;
}

void parseAttributes(FILE* in, FILE* out, const char* parent) {
	while (true) {
		uint16_t id = readUint16(in);
		if (id == END_ATTRIBUTES)
			break;
		findParseAttribute(in, out, id, parent);
	}
}

void indent(FILE* out, int size) {
	for (int i = 0; i < size; i++)
		fprintf(out, "\t");
}

void parseElement2(FILE* in, FILE* out, int depth, uint8_t tag) {
	const char* name = getElementName(tag);
	indent(out, depth);
	fprintf(out, "<%s", name);
	parseAttributes(in, out, name);
	fprintf(out, ">\n");

	while (true) {
		uint8_t child = readByte(in);
		if (child == END_ELEMENT)
			break;

		parseElement2(in, out, depth + 1, child);
	}

	indent(out, depth);
	fprintf(out, "</%s>\n", name);
}

void parseElement(FILE* in, FILE* out, int depth) {
	uint8_t tag;
	if (fread(&tag, 1, 1, in) != 1)
		throw ErrEof;

	parseElement2(in, out, depth, tag);
}

void parseSvgb(FILE* in, FILE* out) {
	expectHeader(in);
	fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");

	parseElement(in, out, 0);
	expectByte(END_FILE, in);
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("%s in.svgb out.svg\n", argv[0]);
		return 0;
	}

	FILE* in = fopen(argv[1], "rb");
	if (!in) {
		perror(argv[1]);
		return 1;
	}

	FILE* out = fopen(argv[2], "w");
	if (!out) {
		perror(argv[2]);
		return 1;
	}

	try {
		parseSvgb(in, out);
	} catch (SvgbError err) {
		printf("error: %d\n", err);
	}

	fclose(out);
	fclose(in);

	return 0;
}

