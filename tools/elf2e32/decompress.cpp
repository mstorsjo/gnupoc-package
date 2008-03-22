#include "deflate.h"
#include <stdio.h>

#define HEADER_SIZE 156
#define KUidCompressionDeflate  0x101f7afc

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("%s in out\n", argv[0]);
		return 1;
	}

	FILE* in = fopen(argv[1], "rb");
	if (!in) {
		perror(argv[1]);
		return 1;
	}
	fseek(in, 0, SEEK_END);
	uint32_t len = ftell(in);
	fseek(in, 0, SEEK_SET);

	uint8_t* header = new uint8_t[HEADER_SIZE];
	fread(header, 1, HEADER_SIZE, in);
	len -= HEADER_SIZE;
	uint8_t* data = new uint8_t[len];
	fread(data, 1, len, in);
	fclose(in);

	uint32_t compression = header[0x1C + 0] | (header[0x1C + 1] << 8) | (header[0x1C + 2] << 16) | (header[0x1C + 3] << 24);
	if (compression != KUidCompressionDeflate) {
		printf("Input image not compressed!\n");
                return 1;
        }
        header[0x1C + 0] = 0;
        header[0x1C + 1] = 0;
        header[0x1C + 2] = 0;
        header[0x1C + 3] = 0;

	TBitInput bitInput(data, 8*len);
	CInflater* inflater = CInflater::NewLC(bitInput);

	FILE* out = fopen(argv[2], "wb");

	fwrite(header, 1, HEADER_SIZE, out);

	while (true) {
		uint8_t outbuf[1024];
		int n = inflater->ReadL(outbuf, sizeof(outbuf));
		if (n <= 0)
			break;
		fwrite(outbuf, 1, n, out);
	}
	fclose(out);

	delete inflater;
	delete [] data;
	delete [] header;

	return 0;
}

