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

#include "epocversion.h"
#include <stdlib.h>
#include <stdio.h>
#include "caseinsensitive.h"

int epocVersion = EPOC_VERSION_UNKNOWN;

void detectVersion() {
	const char* epocroot = getenv("EPOCROOT");
	if (!epocroot)
		return;

	char buf[2000];
	sprintf(buf, "%s/epoc32/include/variant/symbian_os_v9.1.hrh", epocroot);
	if (findCaseInsensitive(buf)) {
		epocVersion = EPOC_VERSION_9_1;
		return;
	}
	sprintf(buf, "%s/epoc32/include/variant/symbian_os_v9.2.hrh", epocroot);
	if (findCaseInsensitive(buf)) {
		epocVersion = EPOC_VERSION_9_2;
		return;
	}
	sprintf(buf, "%s/epoc32/include/variant/symbian_os_v9.3.hrh", epocroot);
	if (findCaseInsensitive(buf)) {
		epocVersion = EPOC_VERSION_9_3;
		return;
	}
	sprintf(buf, "%s/epoc32/include/variant/symbian_os.hrh", epocroot);
	if (findCaseInsensitive(buf)) {
		epocVersion = EPOC_VERSION_9_4;
		return;
	}
}

