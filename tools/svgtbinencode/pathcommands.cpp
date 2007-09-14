/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse pathcommandsparse
#define yylex   pathcommandslex
#define yyerror pathcommandserror
#define yylval  pathcommandslval
#define yychar  pathcommandschar
#define yydebug pathcommandsdebug
#define yynerrs pathcommandsnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DIGIT_SEQUENCE = 258,
     FLOATING_POINT_CONSTANT = 259
   };
#endif
/* Tokens.  */
#define DIGIT_SEQUENCE 258
#define FLOATING_POINT_CONSTANT 259




/* Copy the first part of user declarations.  */
#line 19 "pathcommands.y"


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




/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 326 "pathcommands.y"
{
	int ival;
	float fval;
	struct coord_pair {
		float x, y;
	} pair;
}
/* Line 187 of yacc.c.  */
#line 420 "pathcommands.cpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 433 "pathcommands.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  17
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   140

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  41
/* YYNRULES -- Number of rules.  */
#define YYNRULES  78
/* YYNRULES -- Number of states.  */
#define YYNSTATES  125

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   259

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    28,    27,    29,     2,     2,    25,    26,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    23,     2,    15,     2,     2,
       2,     2,    11,     2,     2,     2,     9,     5,     2,     2,
       2,    19,     2,    17,    21,     2,    13,     2,     2,     2,
       7,     2,     2,     2,     2,     2,     2,    24,     2,    16,
       2,     2,     2,     2,    12,     2,     2,     2,    10,     6,
       2,     2,     2,    20,     2,    18,    22,     2,    14,     2,
       2,     2,     8,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,     8,    10,    13,    16,    18,
      19,    21,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    45,    48,    50,    54,    56,    58,    61,    64,
      66,    70,    73,    76,    78,    82,    85,    88,    90,    94,
      97,   100,   102,   106,   112,   115,   118,   120,   124,   128,
     131,   134,   136,   140,   144,   147,   150,   152,   156,   159,
     162,   164,   168,   180,   184,   186,   188,   190,   193,   196,
     198,   200,   202,   203,   205,   207,   209,   210,   212
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      31,     0,    -1,    32,    -1,    33,    -1,    -1,    34,    -1,
      34,    33,    -1,    38,    35,    -1,    36,    -1,    -1,    37,
      -1,    37,    36,    -1,    40,    -1,    41,    -1,    43,    -1,
      45,    -1,    47,    -1,    50,    -1,    53,    -1,    56,    -1,
      58,    -1,     5,    39,    -1,     6,    39,    -1,    61,    -1,
      61,    66,    42,    -1,     7,    -1,     8,    -1,     9,    42,
      -1,    10,    42,    -1,    61,    -1,    61,    66,    42,    -1,
      11,    44,    -1,    12,    44,    -1,    62,    -1,    62,    66,
      44,    -1,    13,    46,    -1,    14,    46,    -1,    62,    -1,
      62,    66,    46,    -1,    15,    48,    -1,    16,    48,    -1,
      49,    -1,    49,    66,    48,    -1,    61,    66,    61,    66,
      61,    -1,    17,    51,    -1,    18,    51,    -1,    52,    -1,
      52,    66,    51,    -1,    61,    66,    61,    -1,    19,    54,
      -1,    20,    54,    -1,    55,    -1,    55,    66,    54,    -1,
      61,    66,    61,    -1,    21,    57,    -1,    22,    57,    -1,
      61,    -1,    61,    66,    57,    -1,    23,    59,    -1,    24,
      59,    -1,    60,    -1,    60,    66,    59,    -1,    63,    66,
      63,    66,    64,    66,    65,    66,    65,    66,    61,    -1,
      62,    66,    62,    -1,    64,    -1,    68,    -1,     4,    -1,
      69,    68,    -1,    69,     4,    -1,    25,    -1,    26,    -1,
      67,    -1,    -1,    27,    -1,     3,    -1,    70,    -1,    -1,
      28,    -1,    29,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   337,   337,   341,   342,   346,   347,   351,   355,   356,
     360,   361,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   377,   378,   382,   383,   387,   388,   392,   393,   397,
     398,   402,   403,   407,   408,   412,   413,   417,   418,   422,
     423,   427,   428,   432,   436,   437,   441,   442,   446,   450,
     451,   455,   456,   460,   464,   465,   469,   470,   474,   475,
     479,   480,   484,   489,   493,   497,   498,   502,   503,   507,
     507,   511,   512,   516,   520,   524,   525,   529,   530
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DIGIT_SEQUENCE",
  "FLOATING_POINT_CONSTANT", "'M'", "'m'", "'Z'", "'z'", "'L'", "'l'",
  "'H'", "'h'", "'V'", "'v'", "'C'", "'c'", "'S'", "'s'", "'Q'", "'q'",
  "'T'", "'t'", "'A'", "'a'", "'0'", "'1'", "','", "'+'", "'-'", "$accept",
  "svg_path", "moveto_drawto_command_groups_",
  "moveto_drawto_command_groups", "moveto_drawto_command_group",
  "drawto_commands_", "drawto_commands", "drawto_command", "moveto",
  "moveto_argument_sequence", "closepath", "lineto",
  "lineto_argument_sequence", "horizontal_lineto",
  "horizontal_lineto_argument_sequence", "vertical_lineto",
  "vertical_lineto_argument_sequence", "curveto",
  "curveto_argument_sequence", "curveto_argument", "smooth_curveto",
  "smooth_curveto_argument_sequence", "smooth_curveto_argument",
  "quadratic_bezier_curveto", "quadratic_bezier_curveto_argument_sequence",
  "quadratic_bezier_curveto_argument", "smooth_quadratic_bezier_curveto",
  "smooth_quadratic_bezier_curveto_argument_sequence", "elliptical_arc",
  "elliptical_arc_argument_sequence", "elliptical_arc_argument",
  "coordinate_pair", "coordinate", "nonnegative_number", "number", "flag",
  "comma_", "comma", "integer_constant", "sign_", "sign", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,    77,   109,    90,   122,    76,
     108,    72,   104,    86,   118,    67,    99,    83,   115,    81,
     113,    84,   116,    65,    97,    48,    49,    44,    43,    45
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    30,    31,    32,    32,    33,    33,    34,    35,    35,
      36,    36,    37,    37,    37,    37,    37,    37,    37,    37,
      37,    38,    38,    39,    39,    40,    40,    41,    41,    42,
      42,    43,    43,    44,    44,    45,    45,    46,    46,    47,
      47,    48,    48,    49,    50,    50,    51,    51,    52,    53,
      53,    54,    54,    55,    56,    56,    57,    57,    58,    58,
      59,    59,    60,    61,    62,    63,    63,    64,    64,    65,
      65,    66,    66,    67,    68,    69,    69,    70,    70
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     1,     2,     2,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     1,     3,     1,     1,     2,     2,     1,
       3,     2,     2,     1,     3,     2,     2,     1,     3,     2,
       2,     1,     3,     5,     2,     2,     1,     3,     3,     2,
       2,     1,     3,     3,     2,     2,     1,     3,     2,     2,
       1,     3,    11,     3,     1,     1,     1,     2,     2,     1,
       1,     1,     0,     1,     1,     1,     0,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,    76,    76,     0,     2,     3,     5,     9,    77,    78,
      21,    23,    72,    64,     0,    75,    22,     1,     6,    25,
      26,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,     0,     0,     7,     8,    10,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    73,
      76,    71,    76,    74,    68,    67,    27,    29,    28,    31,
      33,    32,    35,    37,    36,    39,    41,    72,    40,    44,
      46,    72,    45,    49,    51,    72,    50,    54,    56,    55,
      66,    58,    60,    72,    65,    59,    11,    24,    63,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,     0,
       0,    30,    34,    38,    42,    72,    47,    48,    52,    53,
      57,    61,    72,    76,    76,    43,    72,     0,    69,    70,
      72,     0,    72,    76,    62
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    37,    38,    39,     7,    10,
      40,    41,    56,    42,    59,    43,    62,    44,    65,    66,
      45,    69,    70,    46,    73,    74,    47,    77,    48,    81,
      82,    57,    12,    83,    13,   120,    50,    51,    84,    14,
      15
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -78
static const yytype_int8 yypact[] =
{
       6,     8,     8,     4,   -78,   -78,     6,   116,   -78,   -78,
     -78,    18,   -10,   -78,    36,   -78,   -78,   -78,   -78,   -78,
     -78,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,    38,    38,   -78,   -78,   116,
     -78,   -78,   -78,   -78,   -78,   -78,   -78,   -78,   -78,   -78,
       8,   -78,     8,   -78,   -78,   -78,   -78,    18,   -78,   -78,
      18,   -78,   -78,    18,   -78,   -78,    18,   -10,   -78,   -78,
      18,   -10,   -78,   -78,    18,   -10,   -78,   -78,    18,   -78,
     -78,   -78,    21,   -10,   -78,   -78,   -78,   -78,   -78,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,    38,
      38,   -78,   -78,   -78,   -78,   -10,   -78,   -78,   -78,   -78,
     -78,   -78,   -10,     8,     8,   -78,   -10,    26,   -78,   -78,
     -10,    26,   -10,     8,   -78
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -78,   -78,   -78,    13,   -78,   -78,   -25,   -78,   -78,    32,
     -78,   -78,   -12,   -78,   -21,   -78,   -13,   -78,   -26,   -78,
     -78,   -15,   -78,   -78,   -16,   -78,   -78,   -14,   -78,   -18,
     -78,    -1,   -17,   -77,   -71,   -72,    -7,   -78,    30,   -78,
     -78
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -73
static const yytype_int8 yytable[] =
{
      11,    11,    68,    61,    17,    52,    60,    60,    63,    63,
      58,     1,     2,    64,    86,    72,    76,    49,    85,    18,
      79,   -72,   -72,   112,   -72,   -72,    67,    67,    71,    71,
      75,    75,    78,    78,    16,    88,     8,     9,    87,    53,
      54,    53,    80,   116,    55,    49,   -72,   -72,    49,   122,
      89,   118,   119,    90,     0,     0,    91,     0,     0,    92,
      93,     0,     0,    94,    95,     0,   104,    96,    97,   102,
       0,    98,     0,    60,    63,    99,   100,   101,   103,   106,
     108,   111,     0,     0,   110,     0,     0,     0,     0,     0,
       0,    67,   105,    71,   107,    75,   109,    78,   113,     0,
       0,     0,     0,     0,     0,   114,     0,     0,     0,   117,
       0,     0,   115,   121,     0,   123,     0,     0,     0,     0,
       0,     0,   124,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36
};

static const yytype_int8 yycheck[] =
{
       1,     2,    28,    24,     0,    12,    23,    24,    25,    26,
      22,     5,     6,    26,    39,    30,    32,    27,    36,     6,
      34,     3,     4,   100,     3,     4,    27,    28,    29,    30,
      31,    32,    33,    34,     2,    52,    28,    29,    50,     3,
       4,     3,     4,   114,    14,    27,    28,    29,    27,   121,
      57,    25,    26,    60,    -1,    -1,    63,    -1,    -1,    66,
      67,    -1,    -1,    70,    71,    -1,    92,    74,    75,    90,
      -1,    78,    -1,    90,    91,    82,    83,    89,    91,    94,
      96,    99,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    93,    94,    95,    96,    97,    98,   105,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,   116,
      -1,    -1,   113,   120,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,   123,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     5,     6,    31,    32,    33,    34,    38,    28,    29,
      39,    61,    62,    64,    69,    70,    39,     0,    33,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    35,    36,    37,
      40,    41,    43,    45,    47,    50,    53,    56,    58,    27,
      66,    67,    66,     3,     4,    68,    42,    61,    42,    44,
      62,    44,    46,    62,    46,    48,    49,    61,    48,    51,
      52,    61,    51,    54,    55,    61,    54,    57,    61,    57,
       4,    59,    60,    63,    68,    59,    36,    42,    62,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    42,    44,    46,    48,    61,    51,    61,    54,    61,
      57,    59,    63,    66,    66,    61,    64,    66,    25,    26,
      65,    66,    65,    66,    61
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 337 "pathcommands.y"
    {;}
    break;

  case 3:
#line 341 "pathcommands.y"
    {;}
    break;

  case 5:
#line 346 "pathcommands.y"
    {;}
    break;

  case 6:
#line 347 "pathcommands.y"
    {;}
    break;

  case 7:
#line 351 "pathcommands.y"
    {;}
    break;

  case 8:
#line 355 "pathcommands.y"
    {;}
    break;

  case 10:
#line 360 "pathcommands.y"
    {;}
    break;

  case 11:
#line 361 "pathcommands.y"
    {;}
    break;

  case 12:
#line 365 "pathcommands.y"
    {;}
    break;

  case 13:
#line 366 "pathcommands.y"
    {;}
    break;

  case 14:
#line 367 "pathcommands.y"
    {;}
    break;

  case 15:
#line 368 "pathcommands.y"
    {;}
    break;

  case 16:
#line 369 "pathcommands.y"
    {;}
    break;

  case 17:
#line 370 "pathcommands.y"
    {;}
    break;

  case 18:
#line 371 "pathcommands.y"
    {;}
    break;

  case 19:
#line 372 "pathcommands.y"
    {;}
    break;

  case 20:
#line 373 "pathcommands.y"
    {;}
    break;

  case 21:
#line 377 "pathcommands.y"
    { add_moveto(0); ;}
    break;

  case 22:
#line 378 "pathcommands.y"
    { add_moveto(1); ;}
    break;

  case 23:
#line 382 "pathcommands.y"
    { push_coord_pair((yyvsp[(1) - (1)].pair).x, (yyvsp[(1) - (1)].pair).y); ;}
    break;

  case 24:
#line 383 "pathcommands.y"
    { push_coord_pair((yyvsp[(1) - (3)].pair).x, (yyvsp[(1) - (3)].pair).y); ;}
    break;

  case 25:
#line 387 "pathcommands.y"
    { append_command(CloseCurve); ;}
    break;

  case 26:
#line 388 "pathcommands.y"
    { append_command(CloseCurve); ;}
    break;

  case 27:
#line 392 "pathcommands.y"
    { add_lineto(0); ;}
    break;

  case 28:
#line 393 "pathcommands.y"
    { add_lineto(1); ;}
    break;

  case 29:
#line 397 "pathcommands.y"
    { push_coord_pair((yyvsp[(1) - (1)].pair).x, (yyvsp[(1) - (1)].pair).y); ;}
    break;

  case 30:
#line 398 "pathcommands.y"
    { push_coord_pair((yyvsp[(1) - (3)].pair).x, (yyvsp[(1) - (3)].pair).y); ;}
    break;

  case 31:
#line 402 "pathcommands.y"
    { add_horz_lineto(0); ;}
    break;

  case 32:
#line 403 "pathcommands.y"
    { add_horz_lineto(1); ;}
    break;

  case 33:
#line 407 "pathcommands.y"
    { push_coord((yyvsp[(1) - (1)].fval)); ;}
    break;

  case 34:
#line 408 "pathcommands.y"
    { push_coord((yyvsp[(1) - (3)].fval)); ;}
    break;

  case 35:
#line 412 "pathcommands.y"
    { add_vert_lineto(0); ;}
    break;

  case 36:
#line 413 "pathcommands.y"
    { add_vert_lineto(1); ;}
    break;

  case 37:
#line 417 "pathcommands.y"
    { push_coord((yyvsp[(1) - (1)].fval)); ;}
    break;

  case 38:
#line 418 "pathcommands.y"
    { push_coord((yyvsp[(1) - (3)].fval)); ;}
    break;

  case 39:
#line 422 "pathcommands.y"
    { add_curveto(0); ;}
    break;

  case 40:
#line 423 "pathcommands.y"
    { add_curveto(1); ;}
    break;

  case 41:
#line 427 "pathcommands.y"
    {;}
    break;

  case 42:
#line 428 "pathcommands.y"
    {;}
    break;

  case 43:
#line 432 "pathcommands.y"
    { push_coord_pair((yyvsp[(5) - (5)].pair).x, (yyvsp[(5) - (5)].pair).y); push_coord_pair((yyvsp[(3) - (5)].pair).x, (yyvsp[(3) - (5)].pair).y); push_coord_pair((yyvsp[(1) - (5)].pair).x, (yyvsp[(1) - (5)].pair).y); ;}
    break;

  case 44:
#line 436 "pathcommands.y"
    { add_smooth_curveto(0); ;}
    break;

  case 45:
#line 437 "pathcommands.y"
    { add_smooth_curveto(1); ;}
    break;

  case 46:
#line 441 "pathcommands.y"
    {;}
    break;

  case 47:
#line 442 "pathcommands.y"
    {;}
    break;

  case 48:
#line 446 "pathcommands.y"
    { push_coord_pair((yyvsp[(3) - (3)].pair).x, (yyvsp[(3) - (3)].pair).y); push_coord_pair((yyvsp[(1) - (3)].pair).x, (yyvsp[(1) - (3)].pair).y); ;}
    break;

  case 49:
#line 450 "pathcommands.y"
    { add_quad_curveto(0); ;}
    break;

  case 50:
#line 451 "pathcommands.y"
    { add_quad_curveto(1); ;}
    break;

  case 51:
#line 455 "pathcommands.y"
    {;}
    break;

  case 52:
#line 456 "pathcommands.y"
    {;}
    break;

  case 53:
#line 460 "pathcommands.y"
    { push_coord_pair((yyvsp[(3) - (3)].pair).x, (yyvsp[(3) - (3)].pair).y); push_coord_pair((yyvsp[(1) - (3)].pair).x, (yyvsp[(1) - (3)].pair).y); ;}
    break;

  case 54:
#line 464 "pathcommands.y"
    { add_smooth_quad_curveto(0); ;}
    break;

  case 55:
#line 465 "pathcommands.y"
    { add_smooth_quad_curveto(1); ;}
    break;

  case 56:
#line 469 "pathcommands.y"
    { push_coord_pair((yyvsp[(1) - (1)].pair).x, (yyvsp[(1) - (1)].pair).y); ;}
    break;

  case 57:
#line 470 "pathcommands.y"
    { push_coord_pair((yyvsp[(1) - (3)].pair).x, (yyvsp[(1) - (3)].pair).y); ;}
    break;

  case 58:
#line 474 "pathcommands.y"
    { yyerror("Elliptical arcs not supported"); ;}
    break;

  case 59:
#line 475 "pathcommands.y"
    { yyerror("Elliptical arcs not supported"); ;}
    break;

  case 60:
#line 479 "pathcommands.y"
    {;}
    break;

  case 61:
#line 480 "pathcommands.y"
    {;}
    break;

  case 62:
#line 485 "pathcommands.y"
    {;}
    break;

  case 63:
#line 489 "pathcommands.y"
    { (yyval.pair).x = (yyvsp[(1) - (3)].fval); (yyval.pair).y = (yyvsp[(3) - (3)].fval); ;}
    break;

  case 64:
#line 493 "pathcommands.y"
    { (yyval.fval) = (yyvsp[(1) - (1)].fval); ;}
    break;

  case 65:
#line 497 "pathcommands.y"
    { (yyval.fval) = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 66:
#line 498 "pathcommands.y"
    { (yyval.fval) = (yyvsp[(1) - (1)].fval); ;}
    break;

  case 67:
#line 502 "pathcommands.y"
    { (yyval.fval) = (yyvsp[(1) - (2)].ival) * (yyvsp[(2) - (2)].ival); ;}
    break;

  case 68:
#line 503 "pathcommands.y"
    { (yyval.fval) = (yyvsp[(1) - (2)].ival) * (yyvsp[(2) - (2)].fval); ;}
    break;

  case 70:
#line 507 "pathcommands.y"
    {;}
    break;

  case 71:
#line 511 "pathcommands.y"
    {;}
    break;

  case 73:
#line 516 "pathcommands.y"
    {;}
    break;

  case 74:
#line 520 "pathcommands.y"
    { (yyval.ival) = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 75:
#line 524 "pathcommands.y"
    { (yyval.ival) = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 76:
#line 525 "pathcommands.y"
    { (yyval.ival) = 1; ;}
    break;

  case 77:
#line 529 "pathcommands.y"
    { (yyval.ival) = 1; ;}
    break;

  case 78:
#line 530 "pathcommands.y"
    { (yyval.ival) = -1; ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2117 "pathcommands.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 533 "pathcommands.y"


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



