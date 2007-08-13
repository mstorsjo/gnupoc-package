/*
    Mifconv
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

    Martin Storsjö
    martin@martin.st
*/

#include <stdio.h>
#include <stdint.h>
#include <vector>

using std::vector;

void writeUint32(uint32_t value, FILE* out) {
	uint8_t buf[] = { value >> 0, value >> 8, value >> 16, value >> 24 };
	fwrite(buf, 1, 4, out);
}

char* outname = NULL;
char* headername = NULL;

uint32_t colorType = 0;
uint32_t maskType = 0;
uint32_t animType = 0;

void fixDirSep(char* str) {
	char* ptr = str;
	while (*ptr) {
		if (*ptr == '\\')
			*ptr = '/';
		ptr++;
	}
}

class ImageFile {
public:
	ImageFile(const char* n) {
		name = strdup(n);
		fixDirSep(name);
	}
	~ImageFile() {
		free(name);
	}
	ImageFile(const ImageFile& obj) {
		name = strdup(obj.name);
		size = obj.size;
		colorType = obj.colorType;
		maskType = obj.maskType;
		animType = obj.animType;
	}
	ImageFile& operator=(const ImageFile& obj) {
		free(name);
		name = strdup(obj.name);
		colorType = obj.colorType;
		maskType = obj.maskType;
		animType = obj.animType;
		return *this;
	}
	char* name;
	uint32_t size;
	uint32_t colorType;
	uint32_t maskType;
	uint32_t animType;
};
vector<ImageFile> images;

void readParamFile(const char* filename);

struct ColorType {
	const char* name;
	uint32_t code;
} colors[] = {
	{ "c32", 0x0b },
	{ "c24", 0x08 },
	{ "c16", 0x07 },
	{ "c12", 0x0a },
	{ "c8", 0x06 },
	{ "c4", 0x05 },
	{ "8", 0x04 },
	{ "4", 0x03 },
	{ "2", 0x02 },
	{ "1", 0x01 },
	{ NULL, 0 },
};

void processArgument(const char* arg) {
	if (arg[0] == '/') {
		const char* param = arg + 1;
		if (param[0] == 'h' || param[0] == 'H') {
			free(headername);
			headername = strdup(param + 1);
			fixDirSep(headername);
		} else if (param[0] == 'a' || param[0] == 'A')
			animType = 1;
		else if (param[0] == 'f' || param[0] == 'F')
			readParamFile(param + 1);
		ColorType* type = colors;
		while (type->name) {
			int len = strlen(type->name);
			if (!strncmp(type->name, param, len)) {
				colorType = type->code;
				const char* mask = strchr(param, ',');
				if (mask) {
					mask++;
					if (!strcmp(mask, "1"))
						maskType = 1;
					else if (!strcmp(mask, "8"))
						maskType = 4;
				}
				break;
			}
			type++;
		}
	} else {
		if (!outname) {
			outname = strdup(arg);
			fixDirSep(outname);
		} else {
			ImageFile image(arg);
			image.colorType = colorType;
			image.maskType = maskType;
			image.animType = animType;
			printf("Checking: %s\n", image.name);
			FILE* in = fopen(image.name, "rb");
			if (!in) {
				perror(image.name);
				exit(1);
			}
			fseek(in, 0, SEEK_END);
			image.size = ftell(in);
			fclose(in);
			images.push_back(image);
			colorType = 0;
			maskType = 0;
			animType = 0;
		}
	}
}

void readParamFile(const char* filename) {
	char* localName = strdup(filename);
	fixDirSep(localName);
	FILE* in = fopen(localName, "r");
	if (!in) {
		perror(localName);
		free(localName);
		return;
	}
	free(localName);
	char line[1000];
	while (fgets(line, sizeof(line), in)) {
		int len = strlen(line);
		if (line[len-1] == '\n' || line[len-1] == '\r')  line[--len] = '\0';
		if (line[len-1] == '\n' || line[len-1] == '\r')  line[--len] = '\0';
		char* ptr = line;
		char* token;
		while ((token = strtok(ptr, " \t")) != NULL) {
			ptr = NULL;
			processArgument(token);
		}
	}
	fclose(in);
}

char* toName(const char* str) {
	const char* slash = strrchr(str, '/');
	if (slash)
		slash++;
	else
		slash = str;
	char* base = strdup(slash);
	char* dot = strchr(base, '.');
	if (dot)
		*dot = '\0';
	base[0] = toupper(base[0]);
	char* ptr = &base[1];
	while (*ptr) {
		*ptr = tolower(*ptr);
		ptr++;
	}
	return base;
}

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		char* arg = argv[i];
		processArgument(arg);
	}

	if (!outname) {
		fprintf(stderr, "No MIF file name specified\n");
		return 1;
	}

	FILE* out = fopen(outname, "wb");
	if (!out) {
		perror(outname);
		return 1;
	}
	writeUint32(0x34232342, out);
	writeUint32(2, out);
	writeUint32(0x10, out);
	writeUint32(2*images.size(), out);

	uint32_t offset = 0x10 + 0x10*images.size();
	for (unsigned int i = 0; i < images.size(); i++) {
		writeUint32(offset, out);
		writeUint32(images[i].size + 0x20, out);
		writeUint32(offset, out);
		writeUint32(images[i].size + 0x20, out);
		offset += images[i].size + 0x20;
	}

	for (unsigned int i = 0; i < images.size(); i++) {
		writeUint32(0x34232343, out);
		writeUint32(1, out);
		writeUint32(0x20, out);
		writeUint32(images[i].size, out);
		writeUint32(1, out);
		writeUint32(images[i].colorType, out);
		writeUint32(images[i].animType, out);
		if (i == 0 && images[i].colorType == 0)
			writeUint32(0x7b8a7fe0, out);
		else
			writeUint32(images[i].maskType, out);
		printf("Loading file: %s\n", images[i].name);
		FILE* in = fopen(images[i].name, "rb");
		if (!in) {
			perror(images[i].name);
			return 1;
		}
		uint32_t written = 0;
		uint8_t buffer[10240];
		while (written < images[i].size) {
			uint32_t len = images[i].size - written;
			if (len > sizeof(buffer))
				len = sizeof(buffer);
			int n = fread(buffer, 1, len, in);
			fwrite(buffer, 1, n, out);
			if (n <= 0) {
				perror(images[i].name);
				return 1;
			}
			written += n;
		}
		fclose(in);
	}

	printf("Writing mif: %s\n", outname);
	fclose(out);

	if (headername) {
		printf("Writing mbg: %s\n", headername);
		out = fopen(headername, "w");
		if (!out) {
			perror(headername);
			return 1;
		}
		char* base = toName(headername);
		fprintf(out, " \r\n");
		fprintf(out, "/* This file has been generated, DO NOT MODIFY. */\r\n");
		fprintf(out, "enum TMif%s\r\n", base);
		fprintf(out, "\t{\r\n");
		uint32_t val = 16384;
		for (unsigned int i = 0; i < images.size(); i++) {
			char* curname = toName(images[i].name);
			fprintf(out, "\tEMbm%s%s = %d,\r\n", base, curname, val);
			val++;
			if (images[i].colorType == 0 || images[i].maskType != 0) {
				fprintf(out, "\tEMbm%s%s_mask = %d,\r\n", base, curname, val);
			}
			val++;
			free(curname);
		}
		fprintf(out, "\tEMbm%sLastElement\r\n", base);
		fprintf(out, "\t};\r\n");
		free(base);
		fclose(out);
	}

	return 0;
}

