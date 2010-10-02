/*
    Bindiff
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

    Martin Storsjo
    martin@martin.st
*/

#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <vector>

using std::vector;

class Ignore {
public:
	class Segment {
	public:
		uint32_t start, end;
		Segment(uint32_t s, uint32_t e) {
			start = s;
			end = e;
		}
		bool isIn(uint32_t addr) {
			return addr >= start && addr < end;
		}
	};
	void addIgnore(uint32_t start, uint32_t end) {
		segments.push_back(Segment(start, end));
	}
	bool isIgnored(uint32_t addr) {
		for (unsigned int i = 0; i < segments.size(); i++)
			if (segments[i].isIn(addr))
				return true;
		return false;
	}
	vector<Segment> segments;
};

void printHelp(const char* argv0) {
	printf("%s file1 file2 [-i range ...]\n", argv0);
	printf("\t-i range\tignore changes in the given range (e.g. 0x42,0x43)\n");
	printf("\n");
}

int main(int argc, char *argv[]) {
	const char* filename1 = NULL, *filename2 = NULL;
	Ignore ignores;
	
	char opt;
	while ((opt = getopt(argc, argv, "f:i:h")) != -1) {
		switch (opt) {
		case 'i': {
			uint32_t start, end;
			sscanf(optarg, "%x,%x", &start, &end);
			ignores.addIgnore(start, end);
		}
			break;
		case 'h':
		case ':':
		case '?':
			printHelp(argv[0]);
			return 0;
		}
	}

	for (int i = optind; i < argc; i++) {
		if (!filename1)
			filename1 = argv[i];
		else if (!filename2)
			filename2 = argv[i];
		else {
			printf("Two files are already specified\n");
			printHelp(argv[0]);
			return 1;
		}
	}
	if (!filename2) {
		printHelp(argv[0]);
		return 0;
	}

	bool identical = true;
	FILE* file1 = fopen(filename1, "rb");
	if (!file1) {
		perror(filename1);
		return 1;
	}
	FILE* file2 = fopen(filename2, "rb");
	if (!file2) {
		perror(filename2);
		return 1;
	}
	fseek(file1, 0, SEEK_END);
	fseek(file2, 0, SEEK_END);
	int size1 = ftell(file1);
	int size2 = ftell(file2);
	fseek(file1, 0, SEEK_SET);
	fseek(file2, 0, SEEK_SET);
	uint8_t* buf1 = new uint8_t[size1];
	uint8_t* buf2 = new uint8_t[size2];
	fread(buf1, 1, size1, file1);
	fread(buf2, 1, size2, file2);
	fclose(file1);
	fclose(file2);
	bool matching = true;
	int diffstart = 0;
	uint32_t tot = 0;
	int pos;
	for (pos = 0; pos < size1 && pos < size2; pos++) {
		bool match = buf1[pos] == buf2[pos];
		if (ignores.isIgnored(pos))
			match = true;
		if (!match) {
			identical = false;
			if (matching) {
				matching = false;
				diffstart = pos;
			}
		} else {
			if (!matching) {
				printf("diff from %#x to %#x (%d bytes)\n", diffstart, pos-1, pos - diffstart);
				tot += pos - diffstart;
				matching = true;
			}
		}
	}
	if (!matching) {
		printf("diff from %#x to %#x (%d bytes)\n", diffstart, pos-1, pos - diffstart);
		tot += pos - diffstart;
	}
	if (size1 > size2)
		printf("%s is %d bytes longer than %s\n", filename1, size1-size2, filename2);
	else if (size1 < size2)
		printf("%s is %d bytes shorter than %s\n", filename1, size2-size1, filename2);
	if (size1 != size2)
		identical = false;
	if (tot)
		printf("a total of %d bytes differ\n", tot);
	delete [] buf1;
	delete [] buf2;
	return identical ? 0 : 1;
}

