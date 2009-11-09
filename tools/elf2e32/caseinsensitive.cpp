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

#include "caseinsensitive.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

int stricmp(const char* s1, const char* s2) {
	while (true) {
		int c1 = toupper(*s1++);
		int c2 = toupper(*s2++);
		if (c1 < c2)
			return -1;
		if (c1 > c2)
			return 1;
		if (c1 == '\0')
			return 0;
	}
}

bool findCaseInsensitive(char* path, char* fullpath) {
	if (*path == '\0')
		return true;
	char* ptr = strchr(path, '/');
	if (ptr == path) {
		if (!fullpath) {
			return findCaseInsensitive(path+1, path);
		} else {
			return findCaseInsensitive(path+1, fullpath);
		}
	}
	int dirnamelen = path - fullpath;
	char* dirname;
	if (!fullpath) {
		dirnamelen = 0;
		dirname = strdup("./");
		fullpath = path;
	} else {
		dirname = (char*) malloc(dirnamelen+1);
		strncpy(dirname, fullpath, dirnamelen);
		dirname[dirnamelen] = '\0';
	}

	if (!ptr)
		ptr = path + strlen(path);
	int filenamelen = ptr - path;
	char* filename = (char*) malloc(filenamelen+1);
	strncpy(filename, path, filenamelen);
	filename[filenamelen] = '\0';

	DIR* dir = opendir(dirname);
	if (!dir) {
		free(filename);
		free(dirname);
		return false;
	}
	struct dirent* entry;
	bool found = false;
	while ((entry = readdir(dir)) != NULL) {
		if (!stricmp(filename, entry->d_name)) {
			char* testPath = strdup(fullpath);
			memcpy(testPath + dirnamelen, entry->d_name, filenamelen);
			char* nextPath = testPath + dirnamelen + filenamelen;
			if (*nextPath == '/')
				nextPath++;
			if (findCaseInsensitive(nextPath, testPath)) {
				strcpy(path, testPath + dirnamelen);
				found = true;
			}
			free(testPath);
			if (found)
				break;
		}
	}
	closedir(dir);
	free(filename);
	free(dirname);
	return found;
}

