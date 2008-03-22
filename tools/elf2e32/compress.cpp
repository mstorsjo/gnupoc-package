#include "deflate.h"
#include <stdio.h>
#include <fstream>

#define HEADER_SIZE 156
#define KUidCompressionDeflate	0x101f7afc

using namespace std;

void DeflateCompress(char *bytes,TInt size,ostream &os);

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
	if (compression != 0) {
		printf("Input image not uncompressed!\n");
		return 1;
	}
	header[0x1C + 0] = (KUidCompressionDeflate >> 0);
	header[0x1C + 1] = (KUidCompressionDeflate >> 8);
	header[0x1C + 2] = (KUidCompressionDeflate >> 16);
	header[0x1C + 3] = (KUidCompressionDeflate >> 24);

	ofstream stream(argv[2], ios_base::binary | ios_base::out);
	stream.write((const char*) header, HEADER_SIZE);
	DeflateCompress((char*)data, len, stream);
	stream.close();

	delete [] data;
	delete [] header;

	return 0;
}

