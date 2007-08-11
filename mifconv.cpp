#include <stdio.h>
#include <stdint.h>
#include <vector>

using std::vector;

void writeUint32(uint32_t value, FILE* out) {
	uint8_t buf[] = { value >> 0, value >> 8, value >> 16, value >> 24 };
	fwrite(buf, 1, 4, out);
}

const char* outname = NULL;
const char* headername = NULL;
const char* parameterfile = NULL;

uint32_t colorType = 0;
uint32_t maskType = 0;
uint32_t animType = 0;

class ImageFile {
public:
	ImageFile(const char* n) {
		name = strdup(n);
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
		if (param[0] == 'h' || param[0] == 'H')
			headername = param + 1;
		else if (param[0] == 'a' || param[0] == 'A')
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
			outname = arg;
		} else {
			// push new image using current color type
			ImageFile image(arg);
			image.colorType = colorType;
			image.maskType = maskType;
			image.animType = animType;
			FILE* in = fopen(arg, "rb");
			if (!in) {
				perror(arg);
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
	char line[1000];
	FILE* in = fopen(filename, "r");
	if (!in) {
		perror(filename);
		return;
	}
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

	fclose(out);

	return 0;
}

