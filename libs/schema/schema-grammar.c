/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         Schema_parse
#define yylex           Schema_lex
#define yyerror         Schema_error
#define yydebug         Schema_debug
#define yynerrs         Schema_nerrs


/* Copy the first part of user declarations.  */
#line 27 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:339  */

    #define YYDEBUG 1

    #include <stdio.h>

    #include "schema-lex.h"
    #define Schema_lex SchemaScan_yylex

    #include "ParseTree.hpp"

    void Schema_error ( YYLTYPE *llocp, struct SchemaScanBlock* sb, const char* msg )
    {
        /*TODO: send message to the C++ parser for proper display and recovery */
        printf("Line %i pos %i: %s\n", llocp -> first_line, llocp -> first_column, msg);
    }


#line 90 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "schema-tokens.h".  */
#ifndef YY_SCHEMA_HOME_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED
# define YY_SCHEMA_HOME_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int Schema_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END_SOURCE = 0,
    UNRECOGNIZED = 258,
    ELLIPSIS = 259,
    INCREMENT = 260,
    DECIMAL = 261,
    OCTAL = 262,
    HEX = 263,
    FLOAT = 264,
    EXP_FLOAT = 265,
    STRING = 266,
    ESCAPED_STRING = 267,
    IDENTIFIER_1_0 = 268,
    PHYSICAL_IDENTIFIER_1_0 = 269,
    VERSION = 270,
    UNTERM_STRING = 271,
    UNTERM_ESCAPED_STRING = 272,
    VERS_1_0 = 273,
    KW___no_header = 274,
    KW___row_length = 275,
    KW___untyped = 276,
    KW_alias = 277,
    KW_column = 278,
    KW_const = 279,
    KW_control = 280,
    KW_database = 281,
    KW_decode = 282,
    KW_default = 283,
    KW_encode = 284,
    KW_extern = 285,
    KW_false = 286,
    KW_fmtdef = 287,
    KW_function = 288,
    KW_include = 289,
    KW_limit = 290,
    KW_physical = 291,
    KW_read = 292,
    KW_readonly = 293,
    KW_return = 294,
    KW_schema = 295,
    KW_static = 296,
    KW_table = 297,
    KW_template = 298,
    KW_trigger = 299,
    KW_true = 300,
    KW_type = 301,
    KW_typedef = 302,
    KW_typeset = 303,
    KW_validate = 304,
    KW_version = 305,
    KW_view = 306,
    KW_virtual = 307,
    KW_void = 308,
    KW_write = 309
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int Schema_parse (struct SchemaScanBlock* sb);

#endif /* !YY_SCHEMA_HOME_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 210 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  69
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   539

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  119
/* YYNRULES -- Number of rules.  */
#define YYNRULES  253
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  463

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   309

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    56,     2,     2,     2,
      61,    62,    65,    68,    57,    71,     2,    72,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    73,    55,
      63,    60,    64,     2,    67,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    69,     2,    70,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,    66,    59,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   120,   120,   121,   129,   130,   131,   135,   139,   143,
     149,   153,   154,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   175,   176,   180,
     184,   190,   197,   198,   202,   203,   209,   213,   217,   223,
     224,   228,   229,   233,   240,   241,   245,   249,   256,   257,
     261,   268,   272,   279,   283,   290,   294,   295,   296,   304,
     308,   312,   313,   317,   318,   322,   326,   327,   331,   332,
     336,   337,   341,   342,   346,   350,   354,   355,   356,   357,
     358,   362,   363,   367,   371,   372,   376,   377,   381,   382,
     383,   387,   391,   392,   396,   397,   401,   402,   408,   412,
     419,   420,   424,   428,   429,   433,   434,   438,   439,   440,
     441,   445,   451,   457,   461,   462,   466,   467,   471,   472,
     476,   477,   481,   482,   486,   487,   488,   489,   490,   491,
     492,   493,   494,   498,   499,   503,   504,   505,   509,   510,
     511,   512,   516,   517,   518,   519,   520,   524,   525,   526,
     530,   531,   535,   536,   537,   538,   542,   546,   547,   551,
     552,   556,   560,   561,   565,   566,   570,   571,   575,   576,
     580,   587,   588,   592,   593,   597,   598,   599,   600,   601,
     602,   603,   604,   605,   606,   610,   614,   618,   624,   625,
     629,   630,   634,   635,   639,   640,   644,   645,   649,   650,
     654,   655,   659,   660,   664,   665,   666,   670,   671,   675,
     676,   680,   683,   684,   688,   689,   693,   694,   698,   699,
     700,   704,   705,   706,   712,   716,   717,   721,   722,   726,
     727,   731,   732,   733,   737,   738,   742,   746,   750,   751,
     757,   758,   760,   761,   762,   763,   764,   765,   766,   767,
     771,   772,   776,   777
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of source\"", "error", "$undefined", "UNRECOGNIZED", "ELLIPSIS",
  "INCREMENT", "DECIMAL", "OCTAL", "HEX", "FLOAT", "EXP_FLOAT", "STRING",
  "ESCAPED_STRING", "IDENTIFIER_1_0", "PHYSICAL_IDENTIFIER_1_0", "VERSION",
  "UNTERM_STRING", "UNTERM_ESCAPED_STRING", "VERS_1_0", "KW___no_header",
  "KW___row_length", "KW___untyped", "KW_alias", "KW_column", "KW_const",
  "KW_control", "KW_database", "KW_decode", "KW_default", "KW_encode",
  "KW_extern", "KW_false", "KW_fmtdef", "KW_function", "KW_include",
  "KW_limit", "KW_physical", "KW_read", "KW_readonly", "KW_return",
  "KW_schema", "KW_static", "KW_table", "KW_template", "KW_trigger",
  "KW_true", "KW_type", "KW_typedef", "KW_typeset", "KW_validate",
  "KW_version", "KW_view", "KW_virtual", "KW_void", "KW_write", "';'",
  "'$'", "','", "'{'", "'}'", "'='", "'('", "')'", "'<'", "'>'", "'*'",
  "'|'", "'@'", "'+'", "'['", "']'", "'-'", "'/'", "':'", "$accept",
  "parse", "source", "version_1_0", "version_2_x", "schema_2_x",
  "schema_1_0", "schema_1_0_decl_seq", "schema_1_0_decl",
  "script_1_0_decl", "validate_1_0_decl", "include_directive",
  "typedef_1_0_decl", "typedef_1_0_new_name_list", "typedef_1_0_new_name",
  "typeset_1_0_decl", "typeset_1_0_new_name", "typeset_1_0_def",
  "typespec_1_0_list", "typespec_1_0", "typespec_1_0_name",
  "format_1_0_decl", "format_1_0_new_name", "format_1_0_name",
  "const_1_0_decl", "const_1_0_new_name", "alias_1_0_decl",
  "alias_1_0_new_name", "extern_1_0_decl", "ext_func_1_0_decl",
  "function_1_0_decl", "func_1_0_decl", "untyped_func_1_0_decl",
  "row_length_func_1_0_decl", "func_1_0_new_name",
  "opt_func_1_0_schema_sig", "func_1_0_schema_sig",
  "func_1_0_schema_formals", "func_1_0_schema_formal",
  "func_1_0_return_type", "opt_func_1_0_fact_sig", "func_1_0_fact_sig",
  "func_1_0_param_sig", "func_1_0_param_signature",
  "func_1_0_formal_params", "formal_param_1_0", "opt_control_1_0",
  "func_1_0_vararg_formals", "func_1_0_prologue", "func_1_0_body",
  "script_1_0_stmt_seq", "script_1_0_stmt", "production_1_0_stmt",
  "physical_1_0_decl", "phys_1_0_decl", "phys_1_0_return_type",
  "phys_1_0_new_name", "phys_1_0_prologue", "phys_1_0_body",
  "phys_1_0_body_stmt", "phys_1_0_stmt", "table_1_0_decl",
  "new_tbl_1_0_name", "opt_tbl_1_0_parents", "tbl_1_0_parents",
  "tbl_1_0_parent", "tbl_1_0_body", "tbl_1_0_stmt_seq", "tbl_1_0_stmt",
  "col_1_0_modifier_seq", "col_1_0_modifier", "column_1_0_decl",
  "col_1_0_decl", "typed_column_decl_1_0", "column_body_1_0",
  "column_stmt_1_0", "default_view_1_0_decl", "physmbr_1_0_decl",
  "opt_KW_column", "phys_coldef_1_0", "opt_col_schema_parms_1_0",
  "col_schema_parms_1_0", "col_schema_parm_1_0", "col_schema_value_1_0",
  "untyped_tbl_expr_1_0", "cond_expr_1_0", "expression_1_0",
  "primary_expr_1_0", "assign_expr_1_0", "func_expr_1_0",
  "schema_parms_1_0", "schema_parm_1_0", "func_1_0_name",
  "opt_factory_parms_1_0", "factory_parms_1_0", "factory_parm_1_0",
  "opt_func_1_0_parms", "func_1_0_parms", "uint_expr_1_0",
  "float_expr_1_0", "string_expr_1_0", "const_vect_expr_1_0",
  "opt_const_vect_exprlist_1_0", "const_vect_exprlist_1_0",
  "bool_expr_1_0", "negate_expr_1_0", "type_expr_1_0", "database_1_0_decl",
  "opt_database_dad_1_0", "database_body_1_0", "database_members_1_0",
  "database_member_1_0", "opt_template_1_0", "db_member_1_0",
  "table_member_1_0", "opt_version_1_0", "fqn_1_0", "ident_1_0", "dim_1_0", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,    59,    36,    44,   123,   125,
      61,    40,    41,    60,    62,    42,   124,    64,    43,    91,
      93,    45,    47,    58
};
# endif

#define YYPACT_NINF -369

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-369)))

#define YYTABLE_NINF -193

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     337,  -369,   245,   245,   245,    80,   245,   141,    49,    67,
     139,   245,   245,   245,    80,    32,  -369,   133,   152,   427,
     113,  -369,   427,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,    16,
    -369,    13,    18,  -369,  -369,   146,   245,    77,   245,   245,
      55,  -369,  -369,  -369,   126,  -369,  -369,   112,  -369,   141,
    -369,   156,    21,    16,   164,   157,  -369,   185,   223,  -369,
    -369,  -369,  -369,  -369,  -369,   234,   247,   157,   174,   285,
     157,   245,   288,  -369,   255,   157,   277,    23,   294,   245,
     142,  -369,   245,   248,  -369,   245,  -369,   126,  -369,   245,
    -369,   245,   302,  -369,   317,  -369,   274,   245,   310,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
     245,   387,   320,  -369,   183,   183,   471,   333,  -369,  -369,
     345,  -369,  -369,  -369,  -369,  -369,  -369,    10,   183,   351,
     245,   322,  -369,   356,  -369,   357,  -369,    55,  -369,  -369,
     245,  -369,   358,  -369,   358,    25,   371,  -369,    34,   368,
    -369,  -369,   245,  -369,   340,  -369,   360,   157,  -369,   363,
     266,  -369,  -369,  -369,  -369,  -369,  -369,   362,   373,  -369,
    -369,   157,  -369,   283,   374,  -369,   379,   183,    39,   177,
    -369,  -369,  -369,  -369,   157,    51,   375,  -369,   241,  -369,
     245,  -369,   377,   108,   389,  -369,   415,  -369,   101,   429,
    -369,  -369,   388,   403,  -369,   244,  -369,  -369,   390,   433,
    -369,   245,  -369,  -369,   183,   387,   245,  -369,   183,   392,
    -369,   268,  -369,   183,  -369,   393,  -369,  -369,  -369,  -369,
    -369,   263,  -369,   115,  -369,   425,   391,   166,  -369,   245,
     153,   298,   122,   396,  -369,  -369,   245,   401,   140,   404,
     402,   387,   245,  -369,    27,   455,  -369,   413,   408,   415,
     417,   410,  -369,  -369,  -369,  -369,  -369,  -369,  -369,   410,
    -369,  -369,  -369,   345,    41,  -369,   183,   283,  -369,  -369,
     411,   426,  -369,  -369,  -369,   245,   245,  -369,  -369,   430,
    -369,    20,   425,  -369,   473,   428,  -369,   242,   245,  -369,
     431,   396,   396,  -369,   250,  -369,   242,  -369,   192,  -369,
     432,  -369,   183,   387,   270,  -369,   347,   345,   283,  -369,
    -369,   387,   474,   245,   434,  -369,   183,  -369,  -369,   435,
    -369,  -369,  -369,   183,    39,    39,    47,  -369,  -369,   425,
    -369,   430,  -369,  -369,   183,   436,   242,  -369,   438,    90,
     245,  -369,  -369,  -369,  -369,   439,   437,   183,   442,   272,
     245,  -369,   175,   183,   245,   276,   278,  -369,  -369,  -369,
     165,   440,    39,  -369,   441,  -369,   183,  -369,   481,   488,
     430,  -369,   218,  -369,  -369,  -369,  -369,   214,  -369,  -369,
     447,  -369,   245,    39,   443,   444,   445,   307,  -369,   253,
    -369,   245,   387,  -369,   387,   183,   345,   183,   446,   451,
     454,  -369,  -369,   448,  -369,    39,   345,   407,   183,   183,
     175,  -369,  -369,  -369,  -369,  -369,   157,   441,  -369,  -369,
    -369,  -369,  -369,  -369,   345,   245,  -369,   441,   441,  -369,
     245,  -369,  -369
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,    63,     0,    63,
      63,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,     4,    10,    11,    20,    21,    25,    13,    14,    15,
      16,    17,    18,    19,    22,    23,    24,   250,   251,     0,
     240,     0,     0,    53,    54,     0,     0,    47,     0,     0,
       0,    55,    56,    57,     0,    64,    30,     0,    98,    63,
      27,   114,     0,     0,     0,    37,    29,     0,     0,     1,
       3,     5,     9,     6,    12,     0,     0,    52,     0,     0,
      50,     0,   225,    44,     0,    46,     0,    61,     0,     0,
       0,    66,     0,   221,    70,     0,    71,     0,   100,     0,
      28,     0,     0,   113,     0,    32,    34,     0,     0,     8,
       7,   242,   243,   244,   245,   246,   247,   248,   249,   241,
      51,   204,   206,   205,   207,   208,   209,   210,   217,   216,
       0,     0,     0,   176,     0,   212,     0,     0,   173,   177,
     194,   178,   179,   180,   181,   182,   183,   175,     0,     0,
       0,     0,    45,     0,    62,     0,    68,     0,    65,    69,
       0,   223,    72,   101,    72,     0,   115,   116,   118,     0,
     112,    31,     0,    35,     0,    39,    41,    43,    36,     0,
       0,   188,   191,   190,   253,   184,   214,     0,   213,   219,
     220,   218,   252,     0,     0,   193,     0,     0,   238,   234,
     224,    60,    59,    67,   222,    84,     0,    73,     0,   102,
       0,   119,     0,     0,   135,   136,   159,   137,   159,     0,
     132,   121,     0,     0,   122,     0,   133,   125,     0,     0,
      33,     0,    38,    42,     0,     0,     0,   211,     0,   250,
     198,     0,   196,   200,    48,     0,   239,   226,   235,   233,
     227,   234,   229,     0,    85,    84,     0,    86,    81,     0,
      84,     0,     0,     0,    99,   117,     0,     0,     0,     0,
       0,     0,     0,   138,    43,     0,   160,     0,   162,   159,
       0,     0,   124,   120,   123,   135,   134,   126,   127,     0,
      40,   174,   189,   194,   192,   215,     0,     0,   195,   202,
       0,   201,    49,   228,   230,     0,     0,   231,   232,    86,
      74,    84,    84,    77,     0,     0,    88,     0,     0,    58,
       0,     0,     0,   107,     0,   105,     0,   103,     0,   131,
       0,   139,     0,     0,     0,   146,     0,   194,     0,   156,
     129,     0,     0,     0,     0,   128,     0,    97,    96,     0,
     199,   197,   187,     0,   238,   238,    84,    78,    87,    84,
      82,    86,    83,    75,     0,     0,    91,    92,     0,     0,
       0,   108,   109,   104,   106,     0,     0,     0,     0,     0,
       0,   149,   155,     0,     0,     0,     0,   164,   167,   169,
     168,   158,   238,   130,   185,   171,   200,   203,     0,     0,
      86,    79,     0,    90,    93,    95,    89,     0,   111,   170,
       0,   140,     0,   238,     0,     0,     0,     0,   150,     0,
     144,     0,     0,   163,     0,     0,   194,     0,     0,     0,
       0,    80,    94,     0,   141,   238,   194,     0,     0,     0,
     155,   147,   148,   145,   165,   166,   168,   157,   161,   172,
     186,   236,   237,   110,   194,     0,   154,   152,   153,   151,
       0,   143,   142
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -369,  -369,  -369,  -369,  -369,  -369,   492,  -369,   490,  -369,
    -369,  -369,  -369,  -369,   341,  -369,  -369,  -369,  -369,   -93,
    -369,  -369,   468,  -369,  -369,   449,  -369,  -369,  -369,  -369,
     193,    60,  -369,  -369,   -18,   506,  -369,  -369,   359,   -23,
     353,  -369,  -369,   258,  -249,  -253,  -369,  -294,  -369,   194,
    -369,   155,  -103,  -369,  -369,  -369,  -369,  -369,  -369,   195,
      54,  -369,  -369,  -369,  -369,   312,  -369,  -369,   300,  -369,
     299,   301,   257,  -368,  -369,    87,  -369,  -195,  -369,  -369,
    -369,  -369,   106,   105,  -369,  -321,   -70,  -369,   243,  -369,
    -252,   296,   297,  -280,   196,   238,   143,  -369,  -124,   400,
    -369,  -369,  -369,  -369,  -369,  -369,   -22,  -369,  -369,  -369,
    -369,   286,  -369,  -369,  -369,  -333,    -2,   -72,   -88
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    73,    21,    22,    23,    24,
      25,    26,    27,   104,   105,    28,    64,   108,   174,   272,
     176,    29,    45,    46,    30,    79,    31,    76,    32,    43,
      33,    51,    52,    53,    86,    54,    55,    90,    91,    95,
     206,   207,   261,   256,   257,   258,   259,   313,   319,   365,
     366,   367,   368,    34,    58,    99,   164,   264,   324,   325,
     327,    35,    61,   102,   166,   167,   170,   223,   224,   225,
     226,   227,   273,   335,   417,   418,   228,   277,   278,   342,
     343,   386,   387,   388,   267,   394,   395,   138,   347,   139,
     180,   181,   140,   194,   241,   242,   300,   301,   141,   142,
     143,   144,   187,   188,   145,   146,   229,    36,   151,   200,
     251,   252,   253,   307,   308,   247,   147,    40,    81
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      39,    41,    42,   119,    47,   161,   309,   182,   137,    62,
      63,    65,   189,   349,   175,   357,   420,   156,   173,   334,
     159,   398,   399,   280,   358,   195,    37,    38,    92,    37,
      38,    88,    96,    82,    98,    96,   103,    77,   154,    80,
     209,    67,   337,   402,    85,   254,    87,    87,    93,   211,
      68,   358,    93,   443,   246,    93,   195,   384,   360,   426,
      56,   106,   419,   361,   185,   186,   222,   401,    37,    38,
      60,  -192,   254,  -192,   163,    96,   254,   162,   196,    80,
     436,   379,    78,    75,   344,   359,    75,   461,   233,    75,
     338,    75,   462,    87,    75,    93,    75,   165,    75,   168,
      75,    89,   454,   360,   447,   177,   431,    75,   179,   183,
     400,   182,    75,     7,    75,   -76,   255,   457,   458,   100,
     222,    37,    38,   240,   276,    37,    38,   245,    93,    93,
      50,    97,   -46,    69,   191,    92,   268,   279,   290,    37,
      38,   305,   320,   269,   270,   406,   448,   182,   198,   321,
      75,   322,    70,    37,    38,    93,   455,   306,   204,    48,
      49,    48,    49,    75,   291,    94,   314,    93,   295,    72,
     106,   271,    59,   299,   460,   330,   270,   323,   254,    94,
     121,   122,   123,   124,   125,   126,   127,    37,    38,   121,
     122,   123,   124,   125,   126,   127,    37,    38,    44,   157,
     336,    83,    50,   271,    50,   128,   158,    66,   168,   182,
     414,   274,   415,   183,   128,   -76,   101,   389,   255,   129,
     248,    93,   107,   311,   416,   424,   350,   240,   129,   177,
      75,   312,   249,    93,   294,   130,   250,   131,    75,   132,
     109,   133,   134,   135,   130,   136,   131,    37,    38,   183,
     133,   134,   135,   376,   136,    37,    38,   177,    37,    38,
     111,   112,   378,   113,   328,    75,   274,   213,   240,    93,
     320,   114,   285,   432,   215,   433,   115,   321,   110,   322,
     116,   364,   217,   397,   427,   117,   219,    75,   118,   121,
     122,   123,   124,   125,   126,   127,   239,    38,   389,   262,
     389,   263,   120,   354,   355,   323,   248,   410,   442,   373,
     152,   183,   336,   456,   128,    93,   369,    78,   249,   427,
     160,    75,   303,   235,    93,   297,   299,   235,   129,   235,
     236,    93,   298,   297,   380,   422,   412,     1,   153,   390,
     421,   392,   423,    78,   130,   148,   131,    75,   150,   336,
     133,   134,   135,   316,   136,   155,   317,   449,   318,     2,
     169,     3,   440,     4,    93,   178,   441,     5,   407,     6,
       7,     8,   171,     9,   172,   371,   372,    10,   413,    11,
     199,    37,    38,   336,    12,    13,    14,    15,   336,   212,
     184,   213,    16,   121,   122,   123,   214,   231,   215,   232,
      37,    38,   381,   192,   216,   382,   217,   383,   193,   218,
     435,   197,   219,   121,   122,   123,    37,    38,   201,   202,
     390,   205,   446,   220,   212,   234,   213,   221,   210,    78,
     238,   214,   237,   215,   244,   243,   260,   266,   276,   216,
     275,   217,   281,   282,   218,   288,   289,   219,   302,     2,
     254,     3,   296,     4,   326,   310,   329,     5,   220,     6,
       7,     8,   283,     9,   332,   333,   339,    10,   340,    11,
     346,   341,   345,   352,    12,    13,    14,   121,   122,   123,
     124,   125,    16,   353,    37,    38,   362,   356,   391,   393,
     363,   370,   377,   405,   429,   403,   396,   411,   408,   409,
     425,   430,   434,   437,   438,   439,   451,   427,   450,   452,
     453,    71,    74,   230,    84,    57,   203,   208,   315,   374,
     375,   404,   265,   284,   286,   331,   287,   459,   444,   445,
     149,   292,   348,   293,   385,   351,   190,   304,     0,   428
};

static const yytype_int16 yycheck[] =
{
       2,     3,     4,    75,     6,    93,   255,   131,    78,    11,
      12,    13,   136,   293,   107,   309,   384,    89,   106,   271,
      92,   354,   355,   218,     4,    15,    13,    14,    50,    13,
      14,    49,    54,    15,    57,    57,    15,    39,    15,    41,
      15,     9,    15,   364,    46,    25,    48,    49,    50,    15,
      18,     4,    54,   421,    15,    57,    15,   337,   311,   392,
      11,    63,   383,   312,   134,   135,   169,   361,    13,    14,
      10,    61,    25,    63,    97,    97,    25,    95,   148,    81,
     413,   333,    69,    73,   279,    65,    73,   455,   176,    73,
      63,    73,   460,    95,    73,    97,    73,    99,    73,   101,
      73,    46,   435,   356,   425,   107,   400,    73,   130,   131,
     359,   235,    73,    33,    73,    64,    65,   438,   439,    59,
     223,    13,    14,   193,    23,    13,    14,   197,   130,   131,
      63,    19,    55,     0,   136,   157,    28,    36,   231,    13,
      14,    26,    20,    35,    36,    55,   426,   271,   150,    27,
      73,    29,     0,    13,    14,   157,   436,    42,   160,    20,
      21,    20,    21,    73,   234,    53,   259,   169,   238,    56,
     172,    63,    33,   243,   454,    35,    36,    55,    25,    53,
       6,     7,     8,     9,    10,    11,    12,    13,    14,     6,
       7,     8,     9,    10,    11,    12,    13,    14,     5,    57,
     272,    55,    63,    63,    63,    31,    64,    14,   210,   333,
      35,   213,    37,   235,    31,    62,    60,   341,    65,    45,
      43,   223,    58,    57,    49,    60,   296,   297,    45,   231,
      73,    65,    55,   235,   236,    61,    59,    63,    73,    65,
      55,    67,    68,    69,    61,    71,    63,    13,    14,   271,
      67,    68,    69,    61,    71,    13,    14,   259,    13,    14,
      26,    27,   332,    29,   266,    73,   268,    23,   338,   271,
      20,    37,    28,    55,    30,    61,    42,    27,    55,    29,
      46,    39,    38,   353,    66,    51,    44,    73,    54,     6,
       7,     8,     9,    10,    11,    12,    13,    14,   422,    58,
     424,    60,    55,   305,   306,    55,    43,   377,    55,    59,
      55,   333,   384,   437,    31,   317,   318,    69,    55,    66,
      72,    73,    59,    57,   326,    57,   396,    57,    45,    57,
      64,   333,    64,    57,    64,    57,    64,     0,    61,   341,
      64,   343,    64,    69,    61,    60,    63,    73,    60,   421,
      67,    68,    69,    55,    71,    61,    58,   427,    60,    22,
      58,    24,    55,    26,   366,    55,    59,    30,   370,    32,
      33,    34,    55,    36,    57,   321,   322,    40,   380,    42,
      58,    13,    14,   455,    47,    48,    49,    50,   460,    21,
      70,    23,    55,     6,     7,     8,    28,    57,    30,    59,
      13,    14,    55,    70,    36,    58,    38,    60,    63,    41,
     412,    60,    44,     6,     7,     8,    13,    14,    62,    62,
     422,    63,   424,    55,    21,    62,    23,    59,    57,    69,
      57,    28,    70,    30,    55,    61,    61,    60,    23,    36,
      51,    38,    13,    55,    41,    55,    13,    44,    55,    22,
      25,    24,    60,    26,    58,    64,    55,    30,    55,    32,
      33,    34,    59,    36,    60,    63,    11,    40,    55,    42,
      60,    63,    55,    62,    47,    48,    49,     6,     7,     8,
       9,    10,    55,    57,    13,    14,    13,    57,    14,    55,
      62,    60,    60,    55,    13,    59,    61,    55,    59,    62,
      60,    13,    55,    60,    60,    60,    55,    66,    62,    55,
      62,    19,    22,   172,    46,     9,   157,   164,   260,   324,
     326,   366,   210,   223,   225,   268,   225,   440,   422,   424,
      81,   235,   289,   236,   338,   297,   136,   251,    -1,   396
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     0,    22,    24,    26,    30,    32,    33,    34,    36,
      40,    42,    47,    48,    49,    50,    55,    75,    76,    77,
      78,    80,    81,    82,    83,    84,    85,    86,    89,    95,
      98,   100,   102,   104,   127,   135,   181,    13,    14,   190,
     191,   190,   190,   103,   104,    96,    97,   190,    20,    21,
      63,   105,   106,   107,   109,   110,    11,   109,   128,    33,
     105,   136,   190,   190,    90,   190,   104,     9,    18,     0,
       0,    80,    56,    79,    82,    73,   101,   190,    69,    99,
     190,   192,    15,    55,    96,   190,   108,   190,   108,    46,
     111,   112,   180,   190,    53,   113,   180,    19,   113,   129,
     105,    60,   137,    15,    87,    88,   190,    58,    91,    55,
      55,    26,    27,    29,    37,    42,    46,    51,    54,   191,
      55,     6,     7,     8,     9,    10,    11,    12,    31,    45,
      61,    63,    65,    67,    68,    69,    71,   160,   161,   163,
     166,   172,   173,   174,   175,   178,   179,   190,    60,    99,
      60,   182,    55,    61,    15,    61,   191,    57,    64,   191,
      72,   192,   108,   113,   130,   190,   138,   139,   190,    58,
     140,    55,    57,   192,    92,    93,    94,   190,    55,   180,
     164,   165,   172,   180,    70,   160,   160,   176,   177,   172,
     173,   190,    70,    63,   167,    15,   160,    60,   190,    58,
     183,    62,    62,   112,   190,    63,   114,   115,   114,    15,
      57,    15,    21,    23,    28,    30,    36,    38,    41,    44,
      55,    59,   126,   141,   142,   143,   144,   145,   150,   180,
      88,    57,    59,   192,    62,    57,    64,    70,    57,    13,
     160,   168,   169,    61,    55,   160,    15,   189,    43,    55,
      59,   184,   185,   186,    25,    65,   117,   118,   119,   120,
      61,   116,    58,    60,   131,   139,    60,   158,    28,    35,
      36,    63,    93,   146,   190,    51,    23,   151,   152,    36,
     151,    13,    55,    59,   142,    28,   144,   145,    55,    13,
      93,   160,   165,   166,   190,   160,    60,    57,    64,   160,
     170,   171,    55,    59,   185,    26,    42,   187,   188,   118,
      64,    57,    65,   121,    93,   117,    55,    58,    60,   122,
      20,    27,    29,    55,   132,   133,    58,   134,   190,    55,
      35,   146,    60,    63,   164,   147,   191,    15,    63,    11,
      55,    63,   153,   154,   151,    55,    60,   162,   162,   167,
     160,   169,    62,    57,   190,   190,    57,   121,     4,    65,
     119,   118,    13,    62,    39,   123,   124,   125,   126,   190,
      60,   134,   134,    59,   133,   123,    61,    60,   160,   164,
      64,    55,    58,    60,   167,   168,   155,   156,   157,   172,
     190,    14,   190,    55,   159,   160,    61,   160,   189,   189,
     118,   121,   159,    59,   125,    55,    55,   190,    59,    62,
     160,    55,    64,   190,    35,    37,    49,   148,   149,   159,
     147,    64,    57,    64,    60,    60,   189,    66,   170,    13,
      13,   121,    55,    61,    55,   190,   189,    60,    60,    60,
      55,    59,    55,   147,   156,   157,   190,   159,   167,   160,
      62,    55,    55,    62,   189,   167,   172,   159,   159,   149,
     167,   147,   147
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    75,    76,    76,    76,    77,    78,    79,
      80,    81,    81,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    83,    83,    84,
      85,    86,    87,    87,    88,    88,    89,    90,    91,    92,
      92,    93,    93,    94,    95,    95,    96,    97,    98,    98,
      99,   100,   101,   102,   103,   104,   105,   105,   105,   106,
     107,   108,   108,   109,   109,   110,   111,   111,   112,   112,
     113,   113,   114,   114,   115,   116,   117,   117,   117,   117,
     117,   118,   118,   119,   120,   120,   121,   121,   122,   122,
     122,   123,   124,   124,   125,   125,   126,   126,   127,   128,
     129,   129,   130,   131,   131,   132,   132,   133,   133,   133,
     133,   134,   135,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   143,   143,   144,   144,   144,   145,   145,
     145,   145,   146,   146,   146,   146,   146,   147,   147,   147,
     148,   148,   149,   149,   149,   149,   150,   151,   151,   152,
     152,   153,   154,   154,   155,   155,   156,   156,   157,   157,
     158,   159,   159,   160,   160,   161,   161,   161,   161,   161,
     161,   161,   161,   161,   161,   162,   163,   163,   164,   164,
     165,   165,   166,   166,   167,   167,   168,   168,   169,   169,
     170,   170,   171,   171,   172,   172,   172,   173,   173,   174,
     174,   175,   176,   176,   177,   177,   178,   178,   179,   179,
     179,   180,   180,   180,   181,   182,   182,   183,   183,   184,
     184,   185,   185,   185,   186,   186,   187,   188,   189,   189,
     190,   190,   190,   190,   190,   190,   190,   190,   190,   190,
     191,   191,   192,   192
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     2,     3,     3,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     2,
       2,     4,     1,     3,     1,     2,     4,     1,     3,     1,
       3,     1,     2,     1,     3,     4,     1,     1,     6,     7,
       1,     4,     1,     2,     1,     2,     1,     1,     6,     4,
       4,     1,     2,     0,     1,     3,     1,     3,     2,     2,
       1,     1,     0,     1,     3,     3,     0,     2,     3,     4,
       5,     1,     3,     3,     0,     1,     0,     2,     1,     3,
       3,     1,     1,     2,     3,     2,     3,     3,     2,     5,
       1,     2,     2,     2,     3,     1,     2,     1,     2,     2,
       5,     3,     4,     2,     0,     2,     1,     3,     1,     2,
       3,     2,     1,     2,     2,     1,     2,     2,     3,     3,
       4,     3,     1,     1,     2,     1,     1,     1,     2,     3,
       5,     6,     8,     7,     4,     5,     2,     4,     4,     2,
       1,     3,     3,     3,     3,     0,     3,     5,     3,     0,
       1,     4,     0,     3,     1,     3,     3,     1,     1,     1,
       4,     1,     3,     1,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     8,     5,     1,     3,
       1,     1,     1,     2,     0,     3,     1,     3,     1,     3,
       0,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     0,     1,     1,     3,     1,     1,     2,     2,
       2,     1,     3,     2,     5,     0,     3,     2,     3,     1,
       2,     2,     2,     1,     0,     1,     5,     5,     0,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     3,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, sb, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct SchemaScanBlock* sb)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (sb);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, struct SchemaScanBlock* sb)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, sb);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, struct SchemaScanBlock* sb)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, sb); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, struct SchemaScanBlock* sb)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (sb);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (struct SchemaScanBlock* sb)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, sb);
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
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
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 120 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = new TokenNode ( Token ( (yyvsp[0]) ) ); }
#line 1781 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:1646  */
    break;

  case 3:
#line 121 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                (yyval) = new RuleNode("#parse");
                                (yyval) -> AddChild ( (yyvsp[-1]) );
                                (yyval) -> AddChild ( new TokenNode ( Token ( (yyvsp[0]) ) ) );
                            }
#line 1791 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:1646  */
    break;


#line 1795 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, sb, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, sb, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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
                      yytoken, &yylval, &yylloc, sb);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
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

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, sb);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, sb, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, sb);
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
  return yyresult;
}
