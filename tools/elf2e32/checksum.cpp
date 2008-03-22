#include <stdio.h>
#include <stdint.h>
#include <zlib.h>

#define HEADER_SIZE 156
#define KImageCrcInitialiser	0xc90fdaa2

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("%s file\n", argv[0]);
		return 1;
	}

	FILE* in = fopen(argv[1], "r+b");
	if (!in) {
		perror(argv[1]);
		return 1;
	}

	uint8_t* header = new uint8_t[HEADER_SIZE];
	fread(header, 1, HEADER_SIZE, in);

	header[0x14 + 0] = (KImageCrcInitialiser >> 0);
	header[0x14 + 1] = (KImageCrcInitialiser >> 8);
	header[0x14 + 2] = (KImageCrcInitialiser >> 16);
	header[0x14 + 3] = (KImageCrcInitialiser >> 24);

	uint32_t crc = ~crc32(0xffffffff, header, HEADER_SIZE);

	header[0x14 + 0] = (crc >> 0);
	header[0x14 + 1] = (crc >> 8);
	header[0x14 + 2] = (crc >> 16);
	header[0x14 + 3] = (crc >> 24);

	fseek(in, 0, SEEK_SET);
	fwrite(header, 1, HEADER_SIZE, in);
	fclose(in);

	delete [] header;

	return 0;
}

