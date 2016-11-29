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

    void Schema_error ( YYLTYPE *llocp, struct SchemaScanBlock* sb, const char* msg )
    {
    /*TODO: call back into the C++ parser */
    /*    sb->report_error ( sb, msg );*/
        printf("Line %i pos %i: %s\n", llocp -> first_line, llocp -> first_column, msg);
    }


#line 89 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:339  */

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
    VERSION = 269,
    UNTERM_STRING = 270,
    UNTERM_ESCAPED_STRING = 271,
    VERS_1_0 = 272,
    KW___no_header = 273,
    KW___row_length = 274,
    KW___untyped = 275,
    KW_alias = 276,
    KW_column = 277,
    KW_const = 278,
    KW_control = 279,
    KW_database = 280,
    KW_decode = 281,
    KW_default = 282,
    KW_encode = 283,
    KW_extern = 284,
    KW_false = 285,
    KW_fmtdef = 286,
    KW_function = 287,
    KW_include = 288,
    KW_limit = 289,
    KW_physical = 290,
    KW_read = 291,
    KW_readonly = 292,
    KW_return = 293,
    KW_schema = 294,
    KW_static = 295,
    KW_table = 296,
    KW_template = 297,
    KW_trigger = 298,
    KW_true = 299,
    KW_type = 300,
    KW_typedef = 301,
    KW_typeset = 302,
    KW_validate = 303,
    KW_version = 304,
    KW_view = 305,
    KW_virtual = 306,
    KW_void = 307,
    KW_write = 308
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

#line 208 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:358  */

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
#define YYFINAL  68
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   478

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  121
/* YYNRULES -- Number of rules.  */
#define YYNRULES  245
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  454

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   308

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    55,     2,     2,     2,
      60,    61,    64,    68,    56,    71,    66,    65,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    72,    54,
      62,    59,    63,     2,    67,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    69,     2,    70,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    57,    73,    58,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   118,   118,   119,   123,   124,   125,   129,   133,   137,
     143,   147,   148,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   169,   170,   174,
     178,   184,   191,   192,   196,   197,   203,   207,   211,   217,
     218,   222,   223,   227,   234,   235,   239,   243,   250,   251,
     255,   262,   266,   273,   277,   284,   288,   289,   290,   298,
     302,   306,   307,   311,   312,   316,   320,   321,   325,   326,
     330,   331,   335,   336,   340,   344,   348,   349,   350,   351,
     352,   356,   357,   361,   365,   366,   370,   371,   375,   376,
     377,   381,   385,   386,   390,   391,   395,   396,   397,   403,
     407,   415,   416,   420,   424,   425,   429,   430,   434,   435,
     436,   437,   441,   447,   453,   457,   458,   462,   463,   467,
     468,   472,   476,   477,   481,   482,   483,   484,   485,   486,
     487,   488,   492,   493,   497,   498,   499,   503,   504,   505,
     506,   510,   514,   516,   517,   521,   522,   523,   527,   528,
     532,   533,   534,   535,   539,   543,   547,   548,   552,   556,
     557,   561,   562,   566,   567,   571,   572,   589,   596,   597,
     601,   602,   603,   604,   605,   606,   607,   608,   609,   610,
     611,   615,   619,   623,   629,   630,   634,   635,   636,   640,
     641,   645,   646,   650,   651,   655,   656,   660,   661,   665,
     666,   670,   671,   672,   676,   677,   681,   682,   686,   689,
     690,   694,   695,   699,   700,   704,   705,   706,   710,   711,
     717,   721,   722,   726,   727,   731,   732,   736,   737,   738,
     742,   743,   747,   751,   755,   756,   760,   761,   768,   769,
     771,   772,   776,   780,   784,   785
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of source\"", "error", "$undefined", "UNRECOGNIZED", "ELLIPSIS",
  "INCREMENT", "DECIMAL", "OCTAL", "HEX", "FLOAT", "EXP_FLOAT", "STRING",
  "ESCAPED_STRING", "IDENTIFIER_1_0", "VERSION", "UNTERM_STRING",
  "UNTERM_ESCAPED_STRING", "VERS_1_0", "KW___no_header", "KW___row_length",
  "KW___untyped", "KW_alias", "KW_column", "KW_const", "KW_control",
  "KW_database", "KW_decode", "KW_default", "KW_encode", "KW_extern",
  "KW_false", "KW_fmtdef", "KW_function", "KW_include", "KW_limit",
  "KW_physical", "KW_read", "KW_readonly", "KW_return", "KW_schema",
  "KW_static", "KW_table", "KW_template", "KW_trigger", "KW_true",
  "KW_type", "KW_typedef", "KW_typeset", "KW_validate", "KW_version",
  "KW_view", "KW_virtual", "KW_void", "KW_write", "';'", "'$'", "','",
  "'{'", "'}'", "'='", "'('", "')'", "'<'", "'>'", "'*'", "'/'", "'.'",
  "'@'", "'+'", "'['", "']'", "'-'", "':'", "'|'", "$accept", "parse",
  "source", "version_1_0", "version_2_x", "schema_2_x", "schema_1_0",
  "schema_1_0_decl_seq", "schema_1_0_decl", "script_1_0_decl",
  "validate_1_0_decl", "include_directive", "typedef_1_0_decl",
  "typedef_1_0_new_name_list", "typedef_1_0_new_name", "typeset_1_0_decl",
  "typeset_1_0_new_name", "typeset_1_0_def", "typespec_1_0_list",
  "typespec_1_0", "typespec_1_0_name", "format_1_0_decl",
  "format_1_0_new_name", "format_1_0_name", "const_1_0_decl",
  "const_1_0_new_name", "alias_1_0_decl", "alias_1_0_new_name",
  "extern_1_0_decl", "ext_func_1_0_decl", "function_1_0_decl",
  "func_1_0_decl", "untyped_func_1_0_decl", "row_length_func_1_0_decl",
  "func_1_0_new_name", "opt_func_1_0_schema_sig", "func_1_0_schema_sig",
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
  "col_1_0_decl", "phys_encoding_expr_1_0", "typed_column_decl_1_0",
  "column_body_1_0", "column_stmt_1_0", "default_view_1_0_decl",
  "physmbr_1_0_decl", "opt_KW_column", "phys_coldef_1_0",
  "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
  "col_schema_parm_1_0", "col_schema_value_1_0", "untyped_tbl_expr_1_0",
  "expression_1_0", "uncast_expr_1_0", "assign_expr_1_0", "func_expr_1_0",
  "schema_parms_1_0", "schema_parm_1_0", "func_1_0_name",
  "opt_factory_parms_1_0", "factory_parms_1_0", "factory_parm_1_0",
  "opt_func_1_0_parms", "func_1_0_parms", "uint_expr_1_0",
  "float_expr_1_0", "string_expr_1_0", "const_vect_expr_1_0",
  "opt_const_vect_exprlist_1_0", "const_vect_exprlist_1_0",
  "bool_expr_1_0", "negate_expr_1_0", "type_expr_1_0", "database_1_0_decl",
  "opt_database_dad_1_0", "database_body_1_0", "database_members_1_0",
  "database_member_1_0", "opt_template_1_0", "db_member_1_0",
  "table_member_1_0", "opt_version_1_0", "vardim_type_expr_1_0", "fqn_1_0",
  "ident_1_0", "dim_1_0", "cond_expr_1_0", YY_NULLPTR
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
     305,   306,   307,   308,    59,    36,    44,   123,   125,    61,
      40,    41,    60,    62,    42,    47,    46,    64,    43,    91,
      93,    45,    58,   124
};
# endif

#define YYPACT_NINF -337

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-337)))

#define YYTABLE_NINF -190

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     335,  -337,    55,    55,    55,    37,    55,   128,   126,    99,
     161,    55,    55,    55,    37,    24,  -337,   177,   225,   365,
     164,  -337,   365,  -337,  -337,  -337,  -337,  -337,  -337,  -337,
    -337,  -337,  -337,  -337,  -337,  -337,  -337,  -337,    13,  -337,
       7,    20,  -337,  -337,   186,    55,    14,    55,    55,    19,
     192,  -337,  -337,    83,  -337,  -337,    31,  -337,   128,  -337,
     204,    21,    13,   215,   203,  -337,   226,   245,  -337,  -337,
    -337,  -337,  -337,  -337,   106,   254,   203,   176,   268,   203,
      55,   269,  -337,   279,   203,   286,    23,   289,    55,    71,
    -337,    55,    93,  -337,  -337,   280,    55,  -337,  -337,    83,
    -337,    55,  -337,    55,   294,  -337,   232,  -337,   103,    55,
     301,  -337,  -337,  -337,  -337,  -337,  -337,  -337,  -337,  -337,
    -337,  -337,  -337,  -337,  -337,  -337,    55,   298,   339,  -337,
     176,   176,   285,   287,  -337,  -337,   297,  -337,  -337,  -337,
    -337,  -337,  -337,    42,   176,   303,    55,   306,  -337,   308,
    -337,   311,  -337,    19,  -337,  -337,    55,   305,   309,  -337,
     309,    38,   317,  -337,    41,   310,  -337,  -337,    55,  -337,
     259,  -337,   316,   203,  -337,   319,   195,  -337,  -337,   145,
    -337,  -337,  -337,   307,   322,  -337,  -337,   203,  -337,   188,
     327,  -337,   325,   176,    43,   210,  -337,  -337,  -337,  -337,
     203,  -337,   107,   331,  -337,   263,  -337,    55,  -337,   333,
     105,   343,  -337,   373,  -337,   114,   386,   347,   133,  -337,
     252,  -337,   348,   349,    16,  -337,    55,  -337,  -337,   176,
     298,    55,    55,  -337,   176,   346,  -337,   220,  -337,   176,
    -337,   354,  -337,  -337,  -337,  -337,  -337,   219,  -337,   116,
    -337,   390,   352,   175,  -337,    55,   107,   283,   243,   353,
     362,  -337,    55,   363,   187,   359,   358,   298,  -337,    55,
     410,  -337,   368,   361,   373,   370,   366,  -337,  -337,  -337,
    -337,  -337,   372,  -337,  -337,   366,    55,  -337,  -337,  -337,
     297,    44,   203,  -337,   176,   188,  -337,  -337,   367,   371,
    -337,  -337,  -337,    55,    55,  -337,  -337,   374,  -337,    18,
     390,  -337,   416,   375,   165,    55,  -337,   376,   353,   353,
    -337,   183,  -337,   165,  -337,  -337,   132,  -337,   378,  -337,
     176,   298,   240,    55,  -337,  -337,   298,   377,    55,   379,
    -337,   176,  -337,  -337,  -337,    15,   380,  -337,  -337,  -337,
     176,    43,    43,    26,  -337,  -337,   390,  -337,   374,  -337,
    -337,   176,   381,   165,  -337,   384,   203,    55,  -337,  -337,
    -337,  -337,   383,   385,   176,  -337,   244,    55,  -337,   284,
     253,  -337,  -337,  -337,   -11,    55,    43,  -337,  -337,   369,
     366,   176,  -337,   418,   419,   374,  -337,    72,  -337,  -337,
    -337,   154,  -337,  -337,  -337,    55,   420,   179,   176,   298,
    -337,   298,   388,   297,   176,  -337,   387,   391,   395,  -337,
    -337,   389,   420,   297,   392,   393,   394,  -337,   224,  -337,
    -337,  -337,  -337,   203,   176,  -337,  -337,  -337,  -337,  -337,
    -337,   297,  -337,   318,   176,   176,   179,  -337,   369,  -337,
    -337,   369,   369,  -337
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,    63,     0,    63,
      63,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,     4,    10,    11,    20,    21,    25,    13,    14,    15,
      16,    17,    18,    19,    22,    23,    24,   242,     0,   238,
       0,     0,    53,    54,     0,     0,    46,     0,     0,     0,
       0,    56,    57,     0,    64,    30,     0,    99,    63,    27,
     115,     0,     0,     0,    37,    29,     0,     0,     1,     3,
       5,     9,     6,    12,     0,     0,    52,     0,     0,    50,
       0,   221,    44,     0,    46,     0,    61,     0,     0,     0,
      66,     0,   218,    55,    70,     0,     0,   237,    71,     0,
     101,     0,    28,     0,     0,   114,     0,    32,    34,     0,
       0,     8,     7,   240,   241,   239,    51,   201,   203,   202,
     204,   205,   206,   207,   214,   213,     0,     0,     0,   172,
       0,   209,     0,     0,   168,   173,   191,   174,   175,   176,
     177,   178,   179,   170,     0,     0,     0,     0,    45,     0,
      62,     0,    68,     0,    65,    69,     0,     0,    72,   102,
      72,     0,   116,   117,   119,     0,   113,    31,     0,    35,
       0,    39,    41,    43,    36,     0,     0,   184,   188,   186,
     171,   180,   211,     0,   210,   216,   217,   215,   243,     0,
       0,   190,     0,     0,   234,   230,   220,    60,    59,    67,
     219,   236,    76,     0,    73,     0,   103,     0,   120,     0,
     142,   134,   135,   156,   136,   156,     0,     0,     0,   122,
       0,   132,     0,     0,     0,    33,     0,    38,    42,     0,
       0,     0,     0,   208,     0,   242,   195,     0,   193,   197,
      48,     0,   235,   222,   231,   229,   223,   230,   225,     0,
      85,    84,     0,    86,    81,     0,    76,    88,     0,     0,
       0,   118,     0,     0,   142,     0,     0,     0,   137,     0,
       0,   157,     0,   159,   156,     0,     0,   124,   121,   123,
     134,   133,     0,   125,   127,     0,     0,    40,   169,   185,
     191,   189,   187,   212,     0,     0,   192,   199,     0,   198,
      49,   224,   226,     0,     0,   227,   228,    86,    74,    84,
      84,    77,     0,     0,     0,     0,    58,     0,     0,     0,
     108,     0,   106,     0,   104,   100,     0,   131,     0,   138,
       0,     0,     0,     0,   154,   129,     0,     0,     0,     0,
     128,     0,    98,   126,    97,     0,     0,   196,   194,   183,
       0,   234,   234,    84,    78,    87,    84,    82,    86,    83,
      75,     0,     0,    91,    92,     0,    89,     0,   109,   110,
     105,   107,     0,     0,     0,   139,     0,     0,   141,   147,
       0,   161,   164,   166,   165,     0,   234,   130,   244,   181,
       0,   197,   200,     0,     0,    86,    79,     0,    90,    93,
      95,     0,   112,   167,   140,     0,   234,     0,     0,     0,
     160,     0,     0,   191,     0,    96,     0,     0,     0,    80,
      94,     0,   234,   191,     0,     0,     0,   153,     0,   148,
     146,   162,   163,   165,     0,   158,   245,   182,   232,   233,
     111,   191,   144,     0,     0,     0,     0,   145,   155,   143,
     152,   150,   151,   149
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -337,  -337,  -337,  -337,  -337,  -337,   425,  -337,   432,  -337,
    -337,  -337,  -337,  -337,   288,  -337,  -337,  -337,  -337,  -105,
    -337,  -337,   412,  -337,  -337,   382,  -337,  -337,  -337,  -337,
      58,    64,  -337,  -337,    27,   446,  -337,  -337,   312,     6,
     299,  -337,  -337,   202,  -245,  -286,  -337,  -292,  -337,   137,
    -337,    98,  -106,  -337,  -337,  -337,  -337,  -337,  -337,   142,
    -279,  -337,  -337,  -337,  -337,   257,  -337,  -337,   248,  -337,
     247,   249,   206,  -337,  -337,  -337,    22,  -337,  -184,  -337,
    -337,  -337,  -337,    62,    61,  -337,   -60,  -337,  -273,  -337,
    -254,   246,   242,  -285,  -337,   180,    86,  -337,  -124,   342,
    -337,  -337,  -337,  -337,  -337,  -337,   -35,  -337,  -337,  -337,
    -337,   231,  -337,  -337,  -337,  -333,  -337,    -2,   -67,   -92,
    -336
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    72,    21,    22,    23,    24,
      25,    26,    27,   106,   107,    28,    63,   110,   170,   171,
     172,    29,    44,    45,    30,    78,    31,    75,    32,    42,
      33,    50,    51,    52,    85,    53,    54,    89,    90,    96,
     203,   204,   257,   252,   253,   254,   255,   311,   316,   362,
     363,   364,   365,    34,    57,   101,   160,   260,   321,   322,
     324,    35,    60,   104,   162,   163,   166,   218,   219,   220,
     221,   222,   268,   269,   378,   428,   429,   223,   272,   273,
     337,   338,   380,   381,   382,   263,   388,   134,   342,   135,
     176,   177,   136,   190,   237,   238,   298,   299,   137,   138,
     139,   140,   183,   184,   141,   142,    97,    36,   147,   196,
     247,   248,   249,   305,   306,   243,    98,   143,    39,    80,
     389
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      38,    40,    41,   178,    46,   346,   307,   115,   185,    61,
      62,    64,   344,   332,    91,   354,   169,   133,   393,   394,
      37,   152,   355,   357,   155,   397,    37,   -47,   390,   285,
     355,   275,    37,    66,    81,   105,    76,   150,    79,   368,
     369,    67,   250,    84,    37,    86,    86,    92,   411,    99,
     250,    92,   206,   413,    92,   208,   191,   242,   191,   217,
     108,    74,   100,    43,    88,   358,   396,   357,    37,     7,
     181,   182,    65,   423,    59,    87,    77,   376,    79,    74,
     228,   286,   356,    94,   192,    74,    74,    74,    74,   441,
     339,   175,    74,    74,    86,    74,    37,    92,   448,   161,
      95,   164,  -189,   419,  -189,   159,   178,   173,   451,   452,
      74,   395,   217,    74,    74,    74,    74,   415,    91,    37,
     -84,   287,   102,   158,    92,   179,   420,   153,   435,   236,
     187,   250,   264,   241,   154,    94,   271,    55,   442,   265,
     266,   303,   113,   178,   194,   414,    37,    47,    48,   274,
     312,    92,    95,   209,   200,   210,   449,   304,   156,   114,
     211,    49,   212,   224,   333,    74,   108,   267,   213,   288,
     214,   251,    77,   215,   293,    74,   216,    68,    37,   297,
      47,    48,   117,   118,   119,   120,   121,   122,   123,    37,
      49,   278,   373,    58,   117,   118,   119,   120,   121,   122,
     123,   235,   317,   361,    74,   164,   124,   178,   216,   318,
     232,   319,   383,   424,   421,   425,   224,    74,   124,    71,
     125,   328,   266,    49,   173,    69,    74,   426,   179,   291,
     292,   309,   125,   427,   347,   236,   126,   320,   127,   310,
      82,   370,   128,   129,   130,   131,    93,   132,   126,   267,
     127,   230,   244,   173,   128,   129,   130,   131,   231,   132,
     326,   244,   317,   103,   245,   179,   379,   173,   246,   318,
     375,   319,   109,   245,   210,    74,   295,   301,   446,   280,
     111,   212,   447,   296,   345,   383,   167,   383,   168,   214,
     392,   117,   118,   119,   120,   121,   230,   320,    37,   112,
     230,   351,   352,   377,   117,   118,   119,   405,   116,   409,
     406,    37,   224,   366,   404,   226,   410,   227,   412,   450,
     258,   224,   259,    37,   117,   118,   119,   144,   146,   179,
     209,   297,   210,   148,   384,     1,   386,   211,   422,   212,
     314,   407,   315,   408,   157,   213,   149,   214,   430,   151,
     215,   165,   180,   216,   436,   174,     2,   188,     3,   189,
       4,   224,   193,   195,     5,   401,     6,     7,     8,   197,
       9,   202,   198,   207,    10,   201,    11,   233,   234,   240,
     229,    12,    13,    14,    15,    77,     2,   239,     3,    16,
       4,   256,   262,   270,     5,   271,     6,     7,     8,   276,
       9,   277,   283,   284,    10,   294,    11,   384,   300,   433,
     323,    12,    13,    14,   250,   308,   325,   327,   330,    16,
     331,   334,   335,   336,   340,   341,   343,   350,   349,   359,
     353,   417,   418,   387,   242,   367,   360,   374,   400,   398,
     391,   402,   414,   385,    70,   438,   403,   434,   437,   439,
     440,   443,   444,   445,    73,    56,   225,    83,   313,   205,
     372,   399,   145,   371,   261,   199,   279,   281,   453,   282,
     329,   431,   432,   290,   186,   348,   289,   416,   302
};

static const yytype_uint16 yycheck[] =
{
       2,     3,     4,   127,     6,   290,   251,    74,   132,    11,
      12,    13,   285,   267,    49,   307,   108,    77,   351,   352,
      13,    88,     4,   309,    91,   361,    13,    13,    13,    13,
       4,   215,    13,     9,    14,    14,    38,    14,    40,   318,
     319,    17,    24,    45,    13,    47,    48,    49,    59,    18,
      24,    53,    14,   386,    56,    14,    14,    14,    14,   165,
      62,    72,    56,     5,    45,   310,   358,   353,    13,    32,
     130,   131,    14,   406,    10,    48,    69,   331,    80,    72,
     172,    65,    64,    52,   144,    72,    72,    72,    72,   422,
     274,   126,    72,    72,    96,    72,    13,    99,   434,   101,
      69,   103,    60,   395,    62,    99,   230,   109,   444,   445,
      72,   356,   218,    72,    72,    72,    72,   390,   153,    13,
      13,   226,    58,    96,   126,   127,    54,    56,   413,   189,
     132,    24,    27,   193,    63,    52,    22,    11,   423,    34,
      35,    25,    36,   267,   146,    73,    13,    19,    20,    35,
     255,   153,    69,    20,   156,    22,   441,    41,    65,    53,
      27,    62,    29,   165,   269,    72,   168,    62,    35,   229,
      37,    64,    69,    40,   234,    72,    43,     0,    13,   239,
      19,    20,     6,     7,     8,     9,    10,    11,    12,    13,
      62,    58,    60,    32,     6,     7,     8,     9,    10,    11,
      12,    13,    19,    38,    72,   207,    30,   331,    43,    26,
      65,    28,   336,    34,    60,    36,   218,    72,    30,    55,
      44,    34,    35,    62,   226,     0,    72,    48,   230,   231,
     232,    56,    44,    54,   294,   295,    60,    54,    62,    64,
      54,    58,    66,    67,    68,    69,    54,    71,    60,    62,
      62,    56,    42,   255,    66,    67,    68,    69,    63,    71,
     262,    42,    19,    59,    54,   267,   333,   269,    58,    26,
     330,    28,    57,    54,    22,    72,    56,    58,    54,    27,
      54,    29,    58,    63,   286,   409,    54,   411,    56,    37,
     350,     6,     7,     8,     9,    10,    56,    54,    13,    54,
      56,   303,   304,    63,     6,     7,     8,    63,    54,    56,
     377,    13,   314,   315,   374,    56,    63,    58,   385,   443,
      57,   323,    59,    13,     6,     7,     8,    59,    59,   331,
      20,   391,    22,    54,   336,     0,   338,    27,   405,    29,
      57,    57,    59,    59,    64,    35,    60,    37,   408,    60,
      40,    57,    13,    43,   414,    54,    21,    70,    23,    62,
      25,   363,    59,    57,    29,   367,    31,    32,    33,    61,
      35,    62,    61,    56,    39,    70,    41,    70,    56,    54,
      61,    46,    47,    48,    49,    69,    21,    60,    23,    54,
      25,    60,    59,    50,    29,    22,    31,    32,    33,    13,
      35,    54,    54,    54,    39,    59,    41,   409,    54,   411,
      57,    46,    47,    48,    24,    63,    54,    54,    59,    54,
      62,    11,    54,    62,    54,    59,    54,    56,    61,    13,
      56,    13,    13,    54,    14,    59,    61,    59,    54,    58,
      60,    58,    73,    66,    19,    54,    61,    59,    61,    54,
      61,    59,    59,    59,    22,     9,   168,    45,   256,   160,
     323,   363,    80,   321,   207,   153,   218,   220,   446,   220,
     264,   409,   411,   231,   132,   295,   230,   391,   247
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     0,    21,    23,    25,    29,    31,    32,    33,    35,
      39,    41,    46,    47,    48,    49,    54,    75,    76,    77,
      78,    80,    81,    82,    83,    84,    85,    86,    89,    95,
      98,   100,   102,   104,   127,   135,   181,    13,   191,   192,
     191,   191,   103,   104,    96,    97,   191,    19,    20,    62,
     105,   106,   107,   109,   110,    11,   109,   128,    32,   105,
     136,   191,   191,    90,   191,   104,     9,    17,     0,     0,
      80,    55,    79,    82,    72,   101,   191,    69,    99,   191,
     193,    14,    54,    96,   191,   108,   191,   108,    45,   111,
     112,   180,   191,    54,    52,    69,   113,   180,   190,    18,
     113,   129,   105,    59,   137,    14,    87,    88,   191,    57,
      91,    54,    54,    36,    53,   192,    54,     6,     7,     8,
       9,    10,    11,    12,    30,    44,    60,    62,    66,    67,
      68,    69,    71,   160,   161,   163,   166,   172,   173,   174,
     175,   178,   179,   191,    59,    99,    59,   182,    54,    60,
      14,    60,   192,    56,    63,   192,    65,    64,   108,   113,
     130,   191,   138,   139,   191,    57,   140,    54,    56,   193,
      92,    93,    94,   191,    54,   180,   164,   165,   172,   191,
      13,   160,   160,   176,   177,   172,   173,   191,    70,    62,
     167,    14,   160,    59,   191,    57,   183,    61,    61,   112,
     191,    70,    62,   114,   115,   114,    14,    56,    14,    20,
      22,    27,    29,    35,    37,    40,    43,   126,   141,   142,
     143,   144,   145,   151,   191,    88,    56,    58,   193,    61,
      56,    63,    65,    70,    56,    13,   160,   168,   169,    60,
      54,   160,    14,   189,    42,    54,    58,   184,   185,   186,
      24,    64,   117,   118,   119,   120,    60,   116,    57,    59,
     131,   139,    59,   159,    27,    34,    35,    62,   146,   147,
      50,    22,   152,   153,    35,   152,    13,    54,    58,   142,
      27,   144,   145,    54,    54,    13,    65,    93,   160,   165,
     166,   191,   191,   160,    59,    56,    63,   160,   170,   171,
      54,    58,   185,    25,    41,   187,   188,   118,    63,    56,
      64,   121,    93,   117,    57,    59,   122,    19,    26,    28,
      54,   132,   133,    57,   134,    54,   191,    54,    34,   146,
      59,    62,   164,    93,    11,    54,    62,   154,   155,   152,
      54,    59,   162,    54,   162,   191,   167,   160,   169,    61,
      56,   191,   191,    56,   121,     4,    64,   119,   118,    13,
      61,    38,   123,   124,   125,   126,   191,    59,   134,   134,
      58,   133,   123,    60,    59,   160,   164,    63,   148,   192,
     156,   157,   158,   172,   191,    66,   191,    54,   160,   194,
      13,    60,   160,   189,   189,   118,   121,   194,    58,   125,
      54,   191,    58,    61,   160,    63,   192,    57,    59,    56,
      63,    59,   192,   189,    73,   162,   170,    13,    13,   121,
      54,    60,   192,   189,    34,    36,    48,    54,   149,   150,
     160,   157,   158,   191,    59,   167,   160,    61,    54,    54,
      61,   189,   167,    59,    59,    59,    54,    58,   194,   167,
     172,   194,   194,   150
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
     122,   123,   124,   124,   125,   125,   126,   126,   126,   127,
     128,   129,   129,   130,   131,   131,   132,   132,   133,   133,
     133,   133,   134,   135,   136,   137,   137,   138,   138,   139,
     139,   140,   141,   141,   142,   142,   142,   142,   142,   142,
     142,   142,   143,   143,   144,   144,   144,   145,   145,   145,
     145,   146,   147,   147,   147,   148,   148,   148,   149,   149,
     150,   150,   150,   150,   151,   152,   153,   153,   154,   155,
     155,   156,   156,   157,   157,   158,   158,   159,   160,   160,
     161,   161,   161,   161,   161,   161,   161,   161,   161,   161,
     161,   162,   163,   163,   164,   164,   165,   165,   165,   166,
     166,   167,   167,   168,   168,   169,   169,   170,   170,   171,
     171,   172,   172,   172,   173,   173,   174,   174,   175,   176,
     176,   177,   177,   178,   178,   179,   179,   179,   180,   180,
     181,   182,   182,   183,   183,   184,   184,   185,   185,   185,
     186,   186,   187,   188,   189,   189,   190,   190,   191,   191,
     191,   191,   192,   193,   194,   194
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     2,     3,     3,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     2,
       2,     4,     1,     3,     1,     2,     4,     1,     3,     1,
       3,     1,     2,     1,     3,     4,     1,     1,     6,     7,
       1,     4,     1,     2,     1,     3,     1,     1,     6,     4,
       4,     1,     2,     0,     1,     3,     1,     3,     2,     2,
       1,     1,     0,     1,     3,     3,     0,     2,     3,     4,
       5,     1,     3,     3,     0,     1,     0,     2,     0,     2,
       3,     1,     1,     2,     3,     2,     5,     3,     3,     2,
       6,     1,     2,     2,     2,     3,     1,     2,     1,     2,
       2,     5,     3,     4,     2,     0,     2,     1,     3,     1,
       2,     3,     1,     2,     2,     2,     3,     2,     3,     3,
       4,     3,     1,     2,     1,     1,     1,     2,     3,     4,
       5,     3,     0,     7,     6,     4,     3,     1,     1,     3,
       3,     3,     3,     1,     3,     6,     0,     1,     4,     0,
       3,     1,     3,     3,     1,     1,     1,     4,     1,     4,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     8,     5,     1,     3,     1,     3,     1,     1,
       2,     0,     3,     1,     3,     1,     3,     0,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     3,     0,
       1,     1,     3,     1,     1,     2,     2,     2,     1,     3,
       5,     0,     3,     2,     3,     1,     2,     2,     2,     1,
       0,     1,     5,     5,     0,     1,     3,     1,     1,     3,
       3,     3,     1,     3,     1,     3
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
      
#line 1761 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:1646  */
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
