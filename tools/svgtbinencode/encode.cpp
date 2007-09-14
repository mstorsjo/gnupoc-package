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
#include <expat.h>
#include <string.h>
#include "write.h"
#include "exceptions.h"
#include "style-tokenizer.h"

#define END_ELEMENT 0xfe
#define END_FILE 0xff
#define END_ATTRIBUTES 0x03e8

static const uint8_t header[] = { 0xce, 0x56, 0xfa, 0x03 };

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
	{ "animateMotion", 0x24 },
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
	{ "text", 0x19 },
	{ "linearGradient", 0x28 },
	{ "stop", 0x2a },
	{ "radialGradient", 0x29 },
	{ NULL, 0 }
};

uint8_t findElementId(const char* name) {
	const ElementName* ptr = elementNames;
	while (ptr->name) {
		if (!strcmp(ptr->name, name))
			return ptr->id;
		ptr++;
	}
	fprintf(stderr, "Id for element \"%s\" not known\n", name);
	throw ErrUnknownElement;
}

typedef void (*writeAttribFunc)(const char* value, FILE* out);

const struct Attribute {
	const char* name;
	uint16_t id;
	writeAttribFunc func;
	const char* element;
} attributes[] = {
	{ "version", 0x004c, writeFixed, NULL },
	{ "x", 0x0031, writeFixed, NULL },
	{ "y", 0x0030, writeFixed, NULL },
	{ "x1", 0x0034, writeFixed, NULL },
	{ "y1", 0x0032, writeFixed, NULL },
	{ "x2", 0x0035, writeFixed, NULL },
	{ "y2", 0x0033, writeFixed, NULL },
	{ "gradientUnits", 0x0056, writeGradientUnits, NULL },
	{ "gradientTransform", 0x007b, writeNull, NULL },
	{ "spreadMethod", 0x0055, writeSpreadMethod, NULL },
	{ "xlink:href", 0x006d, writeNull, NULL }, // 00
	{ "offset", 0x0054, writeFixed, NULL },
	{ "stop-color", 0x0051, writeColor, NULL },
	{ "stop-opacity", 0x0057, writeFixed, NULL },
	{ "width", 0x001a, writePercentLength, "svg" },
	{ "height", 0x001b, writePercentLength, "svg" },
	{ "width", 0x001a, writeFixed, NULL },
	{ "height", 0x001b, writeFixed, NULL },
	{ "rx", 0x001d, writeFixed, NULL },
	{ "ry", 0x001e, writeFixed, NULL },
	{ "viewBox", 0x0058, writeViewBox, NULL },
	{ "preserveAspectRatio", 0x005b, writePreserveAspectRatio, NULL },
	{ "zoomAndPan", 0x005a, writeZoomAndPan, NULL },
	{ "baseProfile", 0x0059, writeString, NULL },
	{ "id", 0x005c, writeString, NULL },
	{ "fill", 0x0000, writeFill, NULL },
	{ "fill-opacity", 0x0016, writeFixed, NULL },
	{ "color", 0x000f, writeColor, NULL }, 
	{ "d", 0x004f, writePathCommands, NULL }, 
	{ "transform", 0x0042, writeNull, NULL }, // 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00
	{ "xml:base", 0x005d, writeString, NULL },
	{ "xml:lang", 0x005e, writeString, NULL },
	{ "xml:space", 0x005f, writeString, NULL },
	{ "requiredFeatures", 0x0061, writeString, NULL },
	{ "requiredExtensions", 0x0060, writeString, NULL },
	{ "systemLanguage", 0x0062, writeString, NULL },
	{ "fill-rule", 0x000a, writeString, NULL },
	{ "stroke", 0x0001, writeColor, NULL },
	{ "stroke-dasharray", 0x0008, writeNull, NULL }, // 00
	{ "stroke-dashoffset", 0x000d, writeNull, NULL }, // 00 00 00 00
	{ "stroke-linecap", 0x000b, writeString, NULL },
	{ "stroke-miterlimit", 0x000e, writeNull, NULL }, // 00 00 00 00
	{ "stroke-width", 0x0002, writeFixed, NULL },
	{ NULL, 0, NULL, NULL }
};

void parseStyle(const char* value, const char* element, FILE* out);

void writeAttribute(const char* name, const char* value, const char* element, FILE* out) {
	if (!strcmp(name, "style")) {
		parseStyle(value, element, out);
		return;
	}
	const Attribute* ptr = attributes;
	while (ptr->name) {
		if (!strcmp(ptr->name, name)) {
			if (ptr->element && strcmp(ptr->element, element)) {
				ptr++;
				continue;
			}
			writeUint16(ptr->id, out);
			ptr->func(value, out);
			return;
		}
		ptr++;
	}
	fprintf(stderr, "Attribute \"%s\" in %s skipped\n", name, element);
}

void parseStyle(const char* value, const char* element, FILE* out) {
	initStyleTokenizer(value);
	while (true) {
		char styleAttribute[200];
		char styleValue[200];
		int token;

		token = stylelex();
		if (token == 0)
			break;
		if (token != 1)
			throw ErrBadStyleAttribute;
		strcpy(styleAttribute, styleStringToken());

		token = stylelex();
		if (token != ':')
			throw ErrBadStyleAttribute;

		token = stylelex();
		if (token == 0 || token == ';')
			styleValue[0] = '\0';
		else if (token == 1)
			strcpy(styleValue, styleStringToken());
		else
			throw ErrBadStyleAttribute;

		writeAttribute(styleAttribute, styleValue, element, out);

		if (token == ';')
			continue;

		token = stylelex();
		if (token == 0)
			break;
		if (token != ';')
			throw ErrBadStyleAttribute;
	}
}

class SvgDocument {
public:
	SvgDocument(const char* name) {
		outName = new char[strlen(name)+2];
		strcpy(outName, name);
		strcat(outName, "b");
		out = fopen(outName, "wb");
		if (!out) {
			perror(outName);
			throw ErrCantOpen;
		}
		for (int i = 0; i < 4; i++)
			writeUint8(header[i], out);
	}
	~SvgDocument() {
		writeUint8(END_FILE, out);
		fclose(out);
		delete [] outName;
	}

	void startTag(const XML_Char* name, const XML_Char** attrs) {
		uint8_t id = findElementId(name);
		writeUint8(id, out);

		for (int i = 0; attrs[i]; i += 2) {
			writeAttribute(attrs[i], attrs[i+1], name, out);
		}

		writeUint16(END_ATTRIBUTES, out);
	}
	void endTag(const XML_Char* name) {
		writeUint8(END_ELEMENT, out);
	}


	static void staticStartTag(void* userData, const XML_Char* name, const XML_Char** attrs) {
		SvgDocument* self = (SvgDocument*) userData;
		self->startTag(name, attrs);
	}
	static void staticEndTag(void* userData, const XML_Char* name) {
		SvgDocument* self = (SvgDocument*) userData;
		self->endTag(name);
	}
private:
	FILE* out;
	char* outName;
};

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("%s file.svg\n", argv[0]);
		return 0;
	}

	FILE* in = fopen(argv[1], "r");
	if (!in) {
		perror(argv[1]);
		return 1;
	}

	XML_Parser parser = XML_ParserCreate(NULL);
	if (!parser) {
		fprintf(stderr, "Error in XML_ParserCreate\n");
		return 1;
	}

	SvgDocument* svg = NULL;
	try {
 		svg = new SvgDocument(argv[1]);
	} catch (SvgbError err) {
		return 1;
	}
	XML_SetUserData(parser, svg);

	XML_SetElementHandler(parser, SvgDocument::staticStartTag, SvgDocument::staticEndTag);

	while (true) {
		char buffer[8192];
		int n = fread(buffer, 1, sizeof(buffer), in);
		if (ferror(in)) {
			perror(argv[1]);
			return 1;
		}
		int done = feof(in);

		try {
			if (!XML_Parse(parser, buffer, n, done)) {
				fprintf(stderr, "Parser error at line %d:\n%s\n", (int) XML_GetCurrentLineNumber(parser), XML_ErrorString(XML_GetErrorCode(parser)));
				return 1;
			}
		} catch (SvgbError err) {
			break;
		}

		if (done)
			break;
	}

	XML_ParserFree(parser);

	delete svg;

	fclose(in);

	return 0;
}

