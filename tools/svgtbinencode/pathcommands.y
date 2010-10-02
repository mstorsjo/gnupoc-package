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
#include <stdarg.h>
#include "write.h"
#include "exceptions.h"

const char* strPtr = NULL;
/*
int yylex() {
	if (*strPtr) {
		return *strPtr++;
	}
	return 0;
}
*/

void yyerror(char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	throw ErrBadPathCommands;
}

/*#define YYDEBUG 1*/

#ifdef __cplusplus
extern "C"
#endif
int yylex();

enum Commands {
	Moveto = 0,
	Lineto = 1,
	QuadraticCurveto = 2,
	Curveto = 3,
	CloseCurve = 4
};

typedef struct {
	float x, y;
} coord_pair;

int coord_stack_pos;
int coord_stack_size;
float *coord_stack;

int coord_array_pos;
int coord_array_size;
coord_pair* coord_array;

int command_array_pos;
int command_array_size;
int* command_array;

coord_pair cur_pos;
coord_pair prev_control_point;
int prev_control_point_type;

void init_path_commands() {
	coord_stack_size = 3;
	coord_stack_pos = 0;
	coord_stack = (float*) malloc(sizeof(float)*coord_stack_size);

	command_array_size = 3;
	command_array_pos = 0;
	command_array = (int*) malloc(sizeof(int)*command_array_size);

	coord_array_size = 3;
	coord_array_pos = 0;
	coord_array = (coord_pair*) malloc(sizeof(coord_pair)*coord_array_size);

	cur_pos.x = 0;
	cur_pos.y = 0;

	prev_control_point_type = Moveto;
}
/*
void push_coord_pair(const coord_pair* coord) {
	if (coord_stack_pos + 1 >= coord_stack_size) {
		coord_stack_size *= 2;
		coord_stack = (float*) realloc(coord_stack, sizeof(float)*coord_stack_size);
	}
	coord_stack[coord_stack_pos++] = coord->x;
	coord_stack[coord_stack_pos++] = coord->y;
}
*/
void push_coord_pair(float x, float y) {
	if (coord_stack_pos + 1 >= coord_stack_size) {
		coord_stack_size *= 2;
		coord_stack = (float*) realloc(coord_stack, sizeof(float)*coord_stack_size);
	}
	coord_stack[coord_stack_pos++] = x;
	coord_stack[coord_stack_pos++] = y;
}

void push_coord(float coord) {
	if (coord_stack_pos == coord_stack_size) {
		coord_stack_size *= 2;
		coord_stack = (float*) realloc(coord_stack, sizeof(float)*coord_stack_size);
	}
	coord_stack[coord_stack_pos++] = coord;
}

void pop_coord_pair(coord_pair* coord) {
	coord->y = coord_stack[--coord_stack_pos];
	coord->x = coord_stack[--coord_stack_pos];
}

float pop_coord() {
	return coord_stack[--coord_stack_pos];
}

void append_coord_pair(const coord_pair* coord) {
	if (coord_array_pos == coord_array_size) {
		coord_array_size *= 2;
		coord_array = (coord_pair*) realloc(coord_array, sizeof(coord_pair)*coord_array_size);
	}
	coord_array[coord_array_pos++] = *coord;
}

void append_command(int command) {
	if (command_array_pos == command_array_size) {
		command_array_size *= 2;
		command_array = (int*) realloc(command_array, sizeof(int)*command_array_size);
	}
	command_array[command_array_pos++] = command;
}

void cleanup_path_commands() {
	free(coord_stack);
	free(command_array);
	free(coord_array);
	coord_stack = NULL;
	command_array = NULL;
	coord_array = NULL;
}

void add_lineto(int relative) {
	while (coord_stack_pos > 0) {
		coord_pair coord;
		pop_coord_pair(&coord);
		if (!relative) {
			cur_pos.x = 0;
			cur_pos.y = 0;
		}
		cur_pos.x += coord.x;
		cur_pos.y += coord.y;
		append_coord_pair(&cur_pos);
		append_command(Lineto);
	}
	prev_control_point_type = Lineto;
}

void add_moveto(int relative) {
	if (coord_stack_pos > 0) {
		coord_pair coord;
		pop_coord_pair(&coord);
		if (!relative) {
			cur_pos.x = 0;
			cur_pos.y = 0;
		}
		cur_pos.x += coord.x;
		cur_pos.y += coord.y;
		append_coord_pair(&cur_pos);
		append_command(Moveto);
	}
	add_lineto(relative);
	prev_control_point_type = Moveto;
}

void add_horz_lineto(int relative) {
	while (coord_stack_pos > 0) {
		float coord = pop_coord();
		if (!relative)
			cur_pos.x = 0;
		cur_pos.x += coord;
		append_coord_pair(&cur_pos);
		append_command(Lineto);
	}
	prev_control_point_type = Lineto;
}

void add_vert_lineto(int relative) {
	while (coord_stack_pos > 0) {
		float coord = pop_coord();
		if (!relative)
			cur_pos.y = 0;
		cur_pos.y += coord;
		append_coord_pair(&cur_pos);
		append_command(Lineto);
	}
	prev_control_point_type = Lineto;
}

void add_curveto(int relative) {
	while (coord_stack_pos > 0) {
		coord_pair c1, c2, t;
		pop_coord_pair(&c1);
		pop_coord_pair(&c2);
		pop_coord_pair(&t);
		if (relative) {
			c1.x += cur_pos.x;
			c1.y += cur_pos.y;
			c2.x += cur_pos.x;
			c2.y += cur_pos.y;
			cur_pos.x += t.x;
			cur_pos.y += t.y;
		} else {
			cur_pos = t;
		}
		append_coord_pair(&c1);
		append_coord_pair(&c2);
		append_coord_pair(&cur_pos);
		append_command(Curveto);
		prev_control_point = c2;
		prev_control_point_type = Curveto;
	}
}

void add_smooth_curveto(int relative) {
	while (coord_stack_pos > 0) {
		coord_pair c1, c2, t;
		pop_coord_pair(&c2);
		pop_coord_pair(&t);
		if (prev_control_point_type == Curveto) {
			c1.x = cur_pos.x + (cur_pos.x - prev_control_point.x);
			c1.y = cur_pos.y + (cur_pos.y - prev_control_point.y);
		} else {
			c1 = cur_pos;
		}
		if (relative) {
			c2.x += cur_pos.x;
			c2.y += cur_pos.y;
			cur_pos.x += t.x;
			cur_pos.y += t.y;
		} else {
			cur_pos = t;
		}
		append_coord_pair(&c1);
		append_coord_pair(&c2);
		append_coord_pair(&cur_pos);
		append_command(Curveto);
		prev_control_point = c2;
		prev_control_point_type = Curveto;
	}
}

void add_quad_curveto(int relative) {
	while (coord_stack_pos > 0) {
		coord_pair c,t;
		pop_coord_pair(&c);
		pop_coord_pair(&t);
		if (relative) {
			c.x += cur_pos.x;
			c.y += cur_pos.y;
			cur_pos.x += t.x;
			cur_pos.y += t.y;
		} else {
			cur_pos = t;
		}
		append_coord_pair(&c);
		append_coord_pair(&cur_pos);
		append_command(QuadraticCurveto);
		prev_control_point = c;
		prev_control_point_type = QuadraticCurveto;
	}
}

void add_smooth_quad_curveto(int relative) {
	while (coord_stack_pos > 0) {
		coord_pair c, t;
		pop_coord_pair(&t);
		if (prev_control_point_type == QuadraticCurveto) {
			c.x = cur_pos.x + (cur_pos.x - prev_control_point.x);
			c.y = cur_pos.y + (cur_pos.y - prev_control_point.y);
		} else {
			c = cur_pos;
		}
		if (relative) {
			cur_pos.x += t.x;
			cur_pos.y += t.y;
		} else {
			cur_pos = t;
		}
		append_coord_pair(&c);
		append_coord_pair(&cur_pos);
		append_command(QuadraticCurveto);
		prev_control_point = c;
		prev_control_point_type = QuadraticCurveto;
	}
}


%}

%token <ival> DIGIT_SEQUENCE
%token <fval> FLOATING_POINT_CONSTANT

%type <fval> number nonnegative_number coordinate
%type <ival> sign sign_ integer_constant
%type <pair> coordinate_pair

%union {
	int ival;
	float fval;
	struct coord_pair {
		float x, y;
	} pair;
}

%%

svg_path:
	moveto_drawto_command_groups_					{}
	;

moveto_drawto_command_groups_:
	moveto_drawto_command_groups					{}
	|
	;

moveto_drawto_command_groups:
	moveto_drawto_command_group					{}
	| moveto_drawto_command_group moveto_drawto_command_groups	{}
	;

moveto_drawto_command_group:
	moveto drawto_commands_			{}
	;

drawto_commands_:
	drawto_commands				{}
	|
	;

drawto_commands:
	drawto_command				{}
	| drawto_command drawto_commands	{}
	;

drawto_command:
	closepath				{}
	| lineto				{}
	| horizontal_lineto			{}
	| vertical_lineto			{}
	| curveto				{}
	| smooth_curveto			{}
	| quadratic_bezier_curveto		{}
	| smooth_quadratic_bezier_curveto	{}
	| elliptical_arc			{}
	;

moveto:
	'M' moveto_argument_sequence				{ add_moveto(0); }
	| 'm' moveto_argument_sequence				{ add_moveto(1); }
	;

moveto_argument_sequence:
	coordinate_pair						{ push_coord_pair($1.x, $1.y); }
	| coordinate_pair comma_ lineto_argument_sequence	{ push_coord_pair($1.x, $1.y); }
	;

closepath:
	'Z'							{ append_command(CloseCurve); }
	| 'z'							{ append_command(CloseCurve); }
	;

lineto:
	'L' lineto_argument_sequence				{ add_lineto(0); }
	| 'l' lineto_argument_sequence				{ add_lineto(1); }
	;

lineto_argument_sequence:
	coordinate_pair						{ push_coord_pair($1.x, $1.y); }
	| coordinate_pair comma_ lineto_argument_sequence	{ push_coord_pair($1.x, $1.y); }
	;

horizontal_lineto:
	'H' horizontal_lineto_argument_sequence			{ add_horz_lineto(0); }
	| 'h' horizontal_lineto_argument_sequence		{ add_horz_lineto(1); }
	;

horizontal_lineto_argument_sequence:
	coordinate						{ push_coord($1); }
	| coordinate comma_ horizontal_lineto_argument_sequence	{ push_coord($1); }
	;

vertical_lineto:
	'V' vertical_lineto_argument_sequence			{ add_vert_lineto(0); }
	| 'v' vertical_lineto_argument_sequence			{ add_vert_lineto(1); }
	;

vertical_lineto_argument_sequence:
	coordinate						{ push_coord($1); }
	| coordinate comma_ vertical_lineto_argument_sequence	{ push_coord($1); }
	;

curveto:
	'C' curveto_argument_sequence					{ add_curveto(0); }
	| 'c' curveto_argument_sequence					{ add_curveto(1); }
	;

curveto_argument_sequence:
	curveto_argument					{}
	| curveto_argument comma_ curveto_argument_sequence	{}
	;

curveto_argument:
	coordinate_pair comma_ coordinate_pair comma_ coordinate_pair	{ push_coord_pair($5.x, $5.y); push_coord_pair($3.x, $3.y); push_coord_pair($1.x, $1.y); }
	;

smooth_curveto:
	'S' smooth_curveto_argument_sequence				{ add_smooth_curveto(0); }
	| 's' smooth_curveto_argument_sequence				{ add_smooth_curveto(1); }
	;

smooth_curveto_argument_sequence:
	smooth_curveto_argument							{}
	| smooth_curveto_argument comma_ smooth_curveto_argument_sequence	{}
	;

smooth_curveto_argument:
	coordinate_pair comma_ coordinate_pair				{ push_coord_pair($3.x, $3.y); push_coord_pair($1.x, $1.y); }
	;

quadratic_bezier_curveto:
	'Q' quadratic_bezier_curveto_argument_sequence			{ add_quad_curveto(0); }
	| 'q' quadratic_bezier_curveto_argument_sequence		{ add_quad_curveto(1); }
	;

quadratic_bezier_curveto_argument_sequence:
	quadratic_bezier_curveto_argument				{}
	| quadratic_bezier_curveto_argument comma_ quadratic_bezier_curveto_argument_sequence	{}
	;

quadratic_bezier_curveto_argument:
	coordinate_pair comma_ coordinate_pair				{ push_coord_pair($3.x, $3.y); push_coord_pair($1.x, $1.y); }
	;

smooth_quadratic_bezier_curveto:
	'T' smooth_quadratic_bezier_curveto_argument_sequence		{ add_smooth_quad_curveto(0); }
	| 't' smooth_quadratic_bezier_curveto_argument_sequence		{ add_smooth_quad_curveto(1); }
	;

smooth_quadratic_bezier_curveto_argument_sequence:
	coordinate_pair									{ push_coord_pair($1.x, $1.y); }
	| coordinate_pair comma_ smooth_quadratic_bezier_curveto_argument_sequence	{ push_coord_pair($1.x, $1.y); }
	;

elliptical_arc:
	'A' elliptical_arc_argument_sequence		{ yyerror("Elliptical arcs not supported"); }
	| 'a' elliptical_arc_argument_sequence		{ yyerror("Elliptical arcs not supported"); }
	;

elliptical_arc_argument_sequence:
	elliptical_arc_argument							{}
	| elliptical_arc_argument comma_ elliptical_arc_argument_sequence	{}
	;

elliptical_arc_argument:
	nonnegative_number comma_ nonnegative_number comma_ 
		number comma_ flag comma_ flag comma_ coordinate_pair	{}
	;

coordinate_pair:
	coordinate comma_ coordinate	{ $$.x = $1; $$.y = $3; }
	;

coordinate:
	number				{ $$ = $1; }
	;

nonnegative_number:
	integer_constant		{ $$ = $1; }
	| FLOATING_POINT_CONSTANT	{ $$ = $1; }
	;

number:
	sign_ integer_constant		{ $$ = $1 * $2; }
	| sign_ FLOATING_POINT_CONSTANT { $$ = $1 * $2; }
	;

flag:
	'0' | '1'	{}
	;

comma_:
	comma		{}
	|
	;

comma:
	','		{}
	;

integer_constant:
	DIGIT_SEQUENCE	{ $$ = $1; }
	;

sign_:
	sign		{ $$ = $1; }
	|		{ $$ = 1; }
	;

sign:
	'+'		{ $$ = 1; }
	| '-'		{ $$ = -1; }
	;

%%

void writePathCommands(const char* str, FILE* out) {
	int retval, i;
	init_path_commands();
	strPtr = str;
/*	yydebug = 1;*/
	retval = yyparse();
	writeUint16(command_array_pos, out);
	for (i = 0; i < command_array_pos; i++)
		writeUint8(command_array[i], out);
	writeUint16(coord_array_pos*2, out);
	for (i = 0; i < coord_array_pos; i++) {
		writeFloat(coord_array[i].x, out);
		writeFloat(coord_array[i].y, out);
	}
//	printf("yyparse returned %d\n", retval);
	cleanup_path_commands();
}


