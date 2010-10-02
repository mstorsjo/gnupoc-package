/*
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
*/

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const char* styleStrPtr;
char curStringToken[200];

#define YY_INPUT(buf, result, max_size) 		\
	do {						\
		size_t n = 0;				\
		while (*styleStrPtr && n < max_size)	\
			buf[n++] = *styleStrPtr++;	\
		result = n;				\
	} while (0)

%}

Digit [0-9]
Whitespace [ \t\r\n]

%option noyywrap
%option yylineno

%%

:			return ':';
;			return ';';
{Whitespace}		; /* skipped */
([^ \t\r\n:;])+		{ strcpy(curStringToken, yytext); return 1; }

%%

void initStyleTokenizer(const char* str) {
	styleStrPtr = str;
}

const char* styleStringToken() {
	return curStringToken;
}

