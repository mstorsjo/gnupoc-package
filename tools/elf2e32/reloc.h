/*
    Elf2e32
    Copyright 2007 - 2009 Martin Storsjö

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

    In addition, as a special exception, the copyright holder of this
    program gives permission to link the code of its release of elf2e32
    with modified Symbian code under the Symbian Example Source license;
    and distribute the linked executables. You must obey the GNU General
    Public License in all respects for all of the code used other the
    Symbian copyrighted code. If you modify this file, you may extend this
    exception to your version of the file, but you are not obliged to do
    so. If you do not wish to do so, delete this exception statement from
    your version.

    Martin Storsjö
    martin@martin.st
*/

#ifndef __RELOC_H
#define __RELOC_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct E32ImageHeader;

void fixImportRelocation(uint32_t offset, FILE* out, uint32_t value);
void fixImportRelocation(uint32_t offset, FILE* out, const char* symbol, const char* lib, const char* libpath);

void fixRelocation(uint32_t offset, FILE* out, uint32_t value);

#include <vector>
#include <algorithm>
#include <map>

using std::vector;
using std::map;

class ImportList {
public:
	class Library {
	public:
		Library(const char* name) {
			this->name = strdup(name);
		}
		~Library() {
			free(name);
		}
		void list(FILE* out) {
			printf("%s:\n", name);
			for (unsigned int i = 0; i < addresses.size(); i++) {
				fseek(out, addresses[i] + 0x9c, SEEK_SET);
				uint8_t buf[4];
				fread(buf, 1, sizeof(buf), out);
				uint32_t value = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
				uint32_t offset = value >> 16;
				value &= 0xffff;
//				printf("\t%d\n", addresses[i]);
				printf("\t%d", value);
				if (offset)
					printf(" offset by %d\n", offset);
				else
					printf("\n");
			}
		}
		char* name;
		vector<uint32_t> addresses;
		uint32_t offset;
		static bool compare(const Library* lib1, const Library* lib2) {
			return strcmp(lib1->name, lib2->name) < 0;
		}
	};

	~ImportList() {
		for (unsigned int i = 0; i < libraries.size(); i++)
			delete libraries[i];
		libraries.clear();
	}
	Library* findLibrary(const char* name) {
		for (unsigned int i = 0; i < libraries.size(); i++) {
			if (!strcmp(libraries[i]->name, name))
				return libraries[i];
		}
		Library* lib = new Library(name);
		libraries.push_back(lib);
		return lib;
	}
	void addImport(const char* libname, uint32_t addr) {
		Library* lib = findLibrary(libname);
		lib->addresses.push_back(addr);
	}
	void listImports(FILE* out) {
		for (unsigned int i = 0; i < libraries.size(); i++)
			libraries[i]->list(out);
	}
	void write(FILE* out);

	int numLibraries() {
		return libraries.size();
	}

private:
	vector<Library*> libraries;
};

class RelocationList {
public:
	class Relocation {
	public:
		Relocation(uint16_t off, uint32_t val) {
			offset = off;
			value = val;
		}
		uint16_t offset;
		uint32_t value;
		bool operator<(const Relocation& reloc) const {
			return offset < reloc.offset;
		}
	};
	void addRelocation(uint32_t offset, uint32_t value) {
//		relocations.push_back(Relocation(offset, value));
		uint32_t prefix = offset & 0xfffff000;
		sublists[prefix].push_back(Relocation(offset & 0xfff, value));
	}
	uint32_t sublistSize(uint32_t prefix, uint32_t* padding = NULL) {
		const vector<Relocation>& relocations = sublists[prefix];
		uint32_t relocSize = 8 + 2*relocations.size();
		uint32_t relocPadding = (relocSize & 3) ? (4 - (relocSize & 3)) : 0;
		relocSize += relocPadding;
		if (padding)
			*padding = relocPadding;
		return relocSize;
	}
	void writeSublist(uint32_t prefix, FILE* out, E32ImageHeader* header);
	void write(FILE* out, E32ImageHeader* header);
	int count() {
		uint32_t num = 0;
		for (map<uint32_t, vector<Relocation> >::iterator it = sublists.begin(); it != sublists.end(); it++)
			num += it->second.size();
		return num;
	}
private:
//	vector<Relocation> relocations;
	map<uint32_t, vector<Relocation> > sublists;
};

class ExportList {
public:
	class Export {
	public:
		Export(const char* name, uint32_t value, bool code = true, int size = 0) {
			this->name = NULL;
			if (name)
				this->name = strdup(name);
			address = value;
			this->code = code;
			this->size = size;
		}
		void setName(const char* name) {
			free(this->name);
			this->name = NULL;
			if (name)
				this->name = strdup(name);
		}
		~Export() {
			free(name);
		}
		char* name;
		uint32_t address;
		bool code;
		int size;
		static bool compare(const Export* exp1, const Export* exp2) {
			if (!exp1->name && !exp2->name)
				return 0;
			if (!exp1->name)
				return -1;
			if (!exp2->name)
				return 1;
			return strcmp(exp1->name, exp2->name) < 0;
		}
	};

	ExportList() {
		presetOrdinals = 0;
	}
	~ExportList() {
		clear();
	}
	void clear() {
		for (unsigned int i = 0; i < exports.size(); i++)
			delete exports[i];
		exports.clear();
	}
	void addExportOrdinal(const char* name, uint32_t ordinal) {
		while (exports.size() < ordinal)
			exports.push_back(new Export(NULL, 0));
		if (presetOrdinals < ordinal)
			presetOrdinals = ordinal;
		exports[ordinal - 1]->setName(name);
	}
	void addExport(const char* name, uint32_t addr, bool code = true, int size = 0) {
		for (unsigned int i = 0; i < exports.size(); i++) {
			if (exports[i]->name && !strcmp(exports[i]->name, name)) {
				exports[i]->address = addr;
				exports[i]->code = code;
				exports[i]->size = size;
				return;
			}
		}
		Export* exp = new Export(name, addr, code, size);
		exports.push_back(exp);
	}
	void doSort() {
		sort(exports.begin() + presetOrdinals, exports.end(), Export::compare);
	}
	void write(FILE* out, E32ImageHeader* header, RelocationList* relocations);
	int numExports() {
		return exports.size();
	}
	void writeDef(const char* filename);


	void writeDso(const char* filename, const char* soname);

private:
	vector<Export*> exports;
	uint32_t presetOrdinals;
};


#endif
