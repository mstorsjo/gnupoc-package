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

#ifndef __EPOCVERSION_H
#define __EPOCVERSION_H

extern int epocVersion;
#define MAKE_VERSION(a,b) ((a << 8) | (b))
#define EPOC_VERSION_UNKNOWN 0
#define EPOC_VERSION_9_1 MAKE_VERSION(9, 1)
#define EPOC_VERSION_9_2 MAKE_VERSION(9, 2)
#define EPOC_VERSION_9_3 MAKE_VERSION(9, 3)
#define EPOC_VERSION_9_4 MAKE_VERSION(9, 4)

void detectVersion();

#endif
