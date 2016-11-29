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
#define YYLAST   524

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  119
/* YYNRULES -- Number of rules.  */
#define YYNRULES  252
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  465

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
      60,    61,    64,    67,    56,    70,    65,    71,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    72,    54,
      62,    59,    63,     2,    66,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,    69,     2,     2,     2,     2,     2,     2,
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
     377,   381,   385,   386,   390,   391,   395,   396,   402,   406,
     413,   414,   418,   422,   423,   427,   428,   432,   433,   434,
     435,   439,   445,   451,   455,   456,   460,   461,   465,   466,
     470,   471,   475,   476,   480,   481,   482,   483,   484,   485,
     486,   487,   488,   492,   493,   497,   498,   499,   503,   504,
     505,   506,   510,   511,   512,   513,   514,   518,   519,   520,
     524,   525,   529,   530,   531,   532,   536,   540,   544,   545,
     549,   553,   554,   558,   559,   563,   564,   568,   569,   573,
     580,   581,   585,   586,   587,   588,   589,   590,   591,   592,
     593,   594,   595,   599,   603,   607,   613,   614,   618,   619,
     623,   624,   628,   629,   633,   634,   638,   639,   643,   644,
     648,   649,   653,   654,   655,   659,   660,   664,   665,   669,
     672,   673,   677,   678,   682,   683,   687,   688,   689,   693,
     694,   695,   701,   705,   706,   710,   711,   715,   716,   720,
     721,   722,   726,   727,   731,   735,   739,   740,   746,   747,
     749,   750,   751,   752,   753,   754,   755,   756,   760,   764,
     765,   769,   770
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
  "'{'", "'}'", "'='", "'('", "')'", "'<'", "'>'", "'*'", "'.'", "'@'",
  "'+'", "'['", "']'", "'-'", "'/'", "':'", "'|'", "$accept", "parse",
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
  "col_1_0_decl", "typed_column_decl_1_0", "column_body_1_0",
  "column_stmt_1_0", "default_view_1_0_decl", "physmbr_1_0_decl",
  "opt_KW_column", "phys_coldef_1_0", "opt_col_schema_parms_1_0",
  "col_schema_parms_1_0", "col_schema_parm_1_0", "col_schema_value_1_0",
  "untyped_tbl_expr_1_0", "expression_1_0", "primary_expr_1_0",
  "assign_expr_1_0", "func_expr_1_0", "schema_parms_1_0",
  "schema_parm_1_0", "func_1_0_name", "opt_factory_parms_1_0",
  "factory_parms_1_0", "factory_parm_1_0", "opt_func_1_0_parms",
  "func_1_0_parms", "uint_expr_1_0", "float_expr_1_0", "string_expr_1_0",
  "const_vect_expr_1_0", "opt_const_vect_exprlist_1_0",
  "const_vect_exprlist_1_0", "bool_expr_1_0", "negate_expr_1_0",
  "type_expr_1_0", "database_1_0_decl", "opt_database_dad_1_0",
  "database_body_1_0", "database_members_1_0", "database_member_1_0",
  "opt_template_1_0", "db_member_1_0", "table_member_1_0",
  "opt_version_1_0", "fqn_1_0", "ident_1_0", "dim_1_0", "cond_expr_1_0", YY_NULLPTR
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
      40,    41,    60,    62,    42,    46,    64,    43,    91,    93,
      45,    47,    58,   124
};
# endif

#define YYPACT_NINF -362

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-362)))

#define YYTABLE_NINF -191

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     406,  -362,    24,    24,    24,    41,    24,   122,   139,    59,
     106,    24,    24,    24,    41,   209,  -362,   194,   204,   453,
      80,  -362,   453,  -362,  -362,  -362,  -362,  -362,  -362,  -362,
    -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,    14,  -362,
      13,    21,  -362,  -362,    99,    24,    19,    24,    24,    27,
    -362,  -362,  -362,    28,  -362,  -362,   147,  -362,   122,  -362,
     115,    25,    14,   131,   138,  -362,   220,   244,  -362,  -362,
    -362,  -362,  -362,  -362,   188,   259,   138,   337,   169,   138,
      24,   251,  -362,   277,   138,   203,    34,   274,    24,    76,
    -362,    24,   174,  -362,    24,  -362,    28,  -362,    24,  -362,
      24,   238,  -362,   151,  -362,   214,    24,   287,  -362,  -362,
    -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,
    -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,  -362,    24,
     281,   283,   311,  -362,   241,   241,   270,   284,  -362,  -362,
     301,  -362,  -362,  -362,  -362,  -362,  -362,    43,   241,   307,
      24,   316,  -362,   313,  -362,   314,  -362,    27,  -362,  -362,
      24,  -362,   306,  -362,   306,    39,   320,  -362,    42,   163,
    -362,  -362,    24,  -362,   271,  -362,   309,   138,  -362,   319,
     205,  -362,  -362,  -362,  -362,  -362,  -362,  -362,   315,   326,
    -362,  -362,   138,  -362,   348,   323,  -362,   332,   241,    45,
     178,  -362,  -362,  -362,  -362,   138,   105,   327,  -362,   305,
    -362,    24,  -362,   329,   110,   339,  -362,   368,  -362,    30,
     380,  -362,  -362,   340,   429,  -362,   235,  -362,  -362,   341,
     383,  -362,    24,  -362,  -362,   241,   281,    24,  -362,   241,
     350,  -362,   210,  -362,   241,  -362,   344,  -362,  -362,  -362,
    -362,  -362,   201,  -362,   146,  -362,   376,   349,   100,  -362,
      24,   105,   269,   154,   360,  -362,  -362,    24,   365,   117,
     361,   362,   281,    24,  -362,    50,   411,  -362,   371,   364,
     368,   374,   373,  -362,  -362,  -362,  -362,  -362,  -362,  -362,
     373,  -362,  -362,  -362,   301,    48,  -362,   241,   348,  -362,
    -362,   369,   377,  -362,  -362,  -362,    24,    24,  -362,  -362,
     378,  -362,    18,   376,  -362,   423,   379,  -362,   153,    24,
    -362,   384,   360,   360,  -362,   135,  -362,   153,  -362,   155,
    -362,   385,  -362,   241,   281,   228,  -362,   276,   301,   348,
    -362,  -362,   281,   381,    24,   394,  -362,   241,  -362,  -362,
     390,  -362,  -362,  -362,   241,    45,    45,    46,  -362,  -362,
     376,  -362,   378,  -362,  -362,   241,   399,   153,  -362,   405,
      -5,    24,  -362,  -362,  -362,  -362,   403,   401,   241,   409,
     234,    24,  -362,   189,   241,    24,   236,   256,  -362,  -362,
    -362,    74,    24,    45,  -362,  -362,   392,   241,  -362,   454,
     455,   378,  -362,    89,  -362,  -362,  -362,  -362,   184,  -362,
    -362,   416,  -362,    24,    45,   412,   414,   418,   264,  -362,
     124,  -362,    24,   281,  -362,   281,   420,   301,   241,   419,
     421,   427,  -362,  -362,   428,  -362,    45,   301,   330,   241,
     241,   189,  -362,  -362,  -362,  -362,  -362,   138,   241,  -362,
    -362,  -362,  -362,  -362,  -362,   301,    24,  -362,   392,   392,
    -362,   392,    24,  -362,  -362
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,    63,     0,    63,
      63,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,     4,    10,    11,    20,    21,    25,    13,    14,    15,
      16,    17,    18,    19,    22,    23,    24,   248,     0,   238,
       0,     0,    53,    54,     0,     0,    46,     0,     0,     0,
      55,    56,    57,     0,    64,    30,     0,    98,    63,    27,
     114,     0,     0,     0,    37,    29,     0,     0,     1,     3,
       5,     9,     6,    12,     0,     0,    52,     0,     0,    50,
       0,   223,    44,     0,    46,     0,    61,     0,     0,     0,
      66,     0,   219,    70,     0,    71,     0,   100,     0,    28,
       0,     0,   113,     0,    32,    34,     0,     0,     8,     7,
     240,   241,   242,   243,   244,   245,   246,   247,   239,    51,
     202,   204,   203,   205,   206,   207,   208,   215,   214,     0,
       0,     0,     0,   174,     0,   210,     0,     0,   170,   175,
     192,   176,   177,   178,   179,   180,   181,   172,     0,     0,
       0,     0,    45,     0,    62,     0,    68,     0,    65,    69,
       0,   221,    72,   101,    72,     0,   115,   116,   118,     0,
     112,    31,     0,    35,     0,    39,    41,    43,    36,     0,
       0,   186,   189,   188,   250,   173,   182,   212,     0,   211,
     217,   218,   216,   249,     0,     0,   191,     0,     0,   236,
     232,   222,    60,    59,    67,   220,    76,     0,    73,     0,
     102,     0,   119,     0,     0,   135,   136,   158,   137,   158,
       0,   132,   121,     0,     0,   122,     0,   133,   125,     0,
       0,    33,     0,    38,    42,     0,     0,     0,   209,     0,
     248,   196,     0,   194,   198,    48,     0,   237,   224,   233,
     231,   225,   232,   227,     0,    85,    84,     0,    86,    81,
       0,    76,     0,     0,     0,    99,   117,     0,     0,     0,
       0,     0,     0,     0,   138,    43,     0,   159,     0,   161,
     158,     0,     0,   124,   120,   123,   135,   134,   126,   127,
       0,    40,   171,   187,   192,   190,   213,     0,     0,   193,
     200,     0,   199,    49,   226,   228,     0,     0,   229,   230,
      86,    74,    84,    84,    77,     0,     0,    88,     0,     0,
      58,     0,     0,     0,   107,     0,   105,     0,   103,     0,
     131,     0,   139,     0,     0,     0,   146,     0,   192,     0,
     156,   129,     0,     0,     0,     0,   128,     0,    97,    96,
       0,   197,   195,   185,     0,   236,   236,    84,    78,    87,
      84,    82,    86,    83,    75,     0,     0,    91,    92,     0,
       0,     0,   108,   109,   104,   106,     0,     0,     0,     0,
       0,     0,   149,   155,     0,     0,     0,     0,   163,   166,
     168,   167,     0,   236,   130,   251,   183,   198,   201,     0,
       0,    86,    79,     0,    90,    93,    95,    89,     0,   111,
     169,     0,   140,     0,   236,     0,     0,     0,     0,   150,
       0,   144,     0,     0,   162,     0,     0,   192,     0,     0,
       0,     0,    80,    94,     0,   141,   236,   192,     0,     0,
       0,   155,   147,   148,   145,   164,   165,   167,     0,   160,
     252,   184,   234,   235,   110,   192,     0,   154,   152,   153,
     151,   157,     0,   143,   142
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -362,  -362,  -362,  -362,  -362,  -362,   471,  -362,   469,  -362,
    -362,  -362,  -362,  -362,   321,  -362,  -362,  -362,  -362,  -101,
    -362,  -362,   450,  -362,  -362,   417,  -362,  -362,  -362,  -362,
     255,    61,  -362,  -362,   -15,   487,  -362,  -362,   345,   -22,
     334,  -362,  -362,   242,  -250,  -282,  -362,  -294,  -362,   177,
    -362,   141,  -100,  -362,  -362,  -362,  -362,  -362,  -362,   180,
      49,  -362,  -362,  -362,  -362,   295,  -362,  -362,   285,  -362,
     286,   288,   246,  -361,  -362,    69,  -362,  -196,  -362,  -362,
    -362,  -362,    88,    91,  -362,   -58,  -362,   223,  -362,  -251,
     282,   280,  -280,   181,   221,   125,  -362,  -123,   387,  -362,
    -362,  -362,  -362,  -362,  -362,   -41,  -362,  -362,  -362,  -362,
     272,  -362,  -362,  -362,  -327,    -2,   -71,   -74,  -340
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    72,    21,    22,    23,    24,
      25,    26,    27,   103,   104,    28,    63,   107,   174,   273,
     176,    29,    44,    45,    30,    78,    31,    75,    32,    42,
      33,    50,    51,    52,    85,    53,    54,    89,    90,    94,
     207,   208,   262,   257,   258,   259,   260,   314,   320,   366,
     367,   368,   369,    34,    57,    98,   164,   265,   325,   326,
     328,    35,    60,   101,   166,   167,   170,   224,   225,   226,
     227,   228,   274,   336,   418,   419,   229,   278,   279,   343,
     344,   387,   388,   389,   268,   395,   138,   348,   139,   180,
     181,   140,   195,   242,   243,   301,   302,   141,   142,   143,
     144,   188,   189,   145,   146,   230,    36,   151,   201,   252,
     253,   254,   308,   309,   248,   147,    39,    80,   396
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      38,    40,    41,   118,    46,   175,   310,   182,    91,    61,
      62,    64,    95,   190,   350,    95,   358,   156,   161,   137,
     159,   335,   359,   281,   421,   403,    37,    37,   399,   400,
     361,   173,   -47,    87,    97,    81,    76,    37,    79,   102,
      37,    37,   255,    84,   420,    86,    86,    92,   154,   407,
     359,    92,   277,   210,    92,    95,   212,   196,   385,   247,
     105,   444,   196,   362,   338,   280,   427,    74,   402,   223,
     255,    59,    88,     7,   163,   361,   186,   187,    79,   162,
      93,    77,   360,   380,   345,    74,    74,   437,   179,   183,
     197,    74,    86,    74,    92,   463,   165,    74,   168,   458,
     459,   464,   234,  -190,   177,  -190,    74,   432,   461,   455,
     401,    74,   339,   182,    74,    74,    91,    74,   -84,    99,
      74,    49,    74,    37,   223,    47,    48,    92,    92,   255,
      37,   291,   157,   425,   192,    71,   241,   269,    58,   158,
     246,    47,    48,   433,   270,   271,    74,   449,   199,   182,
      55,   331,   271,    82,   321,    92,   312,   456,   205,   315,
      37,   322,   428,   323,   313,    96,    37,    92,    49,   256,
     105,   306,   272,   321,   100,   462,    37,   292,   443,   272,
     322,   296,   323,   213,    49,   214,   300,   307,   106,   324,
     215,   365,   216,   374,    68,   183,   220,   428,   217,    93,
     218,    37,   337,   219,    69,   171,   220,   172,   324,   168,
      74,   182,   275,   110,   111,   377,   112,   221,    66,   390,
     249,   222,    92,   415,   113,   416,    67,    74,   148,   114,
     177,   183,   250,   115,    92,   295,   251,   417,   116,   351,
     241,   117,    77,   249,   434,   160,    74,   120,   121,   122,
     123,   124,   125,   126,    37,   250,    74,   214,   177,   304,
      43,   236,   286,   153,   216,   329,   298,   275,   237,    65,
      92,   127,   218,   299,   108,   379,   120,   121,   122,   123,
     124,   241,    77,    37,   236,   128,    74,   120,   121,   122,
     236,   381,   298,   183,    37,   169,   398,   413,   109,   422,
     390,   129,   390,   130,   355,   356,   132,   133,   134,   135,
     150,   136,   423,   119,   337,   457,    92,   370,   441,   424,
     411,   426,   442,   317,   185,    92,   318,   232,   319,   233,
     382,   152,    92,   383,   155,   384,   120,   121,   122,   300,
     391,   178,   393,   120,   121,   122,   123,   124,   125,   126,
      37,   337,   184,   193,   120,   121,   122,   123,   124,   125,
     126,   240,   263,   194,   264,    92,   198,   127,   206,   408,
     450,   372,   373,   200,   202,   203,   211,    77,   127,   414,
     235,   128,   239,   244,   238,   337,   245,   261,   267,   276,
     277,   337,   128,   282,   283,   289,   290,   129,   303,   130,
     255,   131,   132,   133,   134,   135,     1,   136,   129,   297,
     130,   436,   311,   132,   133,   134,   135,   327,   136,   330,
     333,   391,   340,   447,   334,   341,   342,     2,   346,     3,
     353,     4,   347,   354,   357,     5,   363,     6,     7,     8,
     364,     9,    37,   371,   378,    10,   392,    11,   394,   213,
     397,   214,    12,    13,    14,    15,   215,   404,   216,   406,
      16,   409,   410,   412,   217,   428,   218,   430,   431,   219,
     435,   438,   220,   439,     2,   452,     3,   440,     4,   448,
     451,   453,     5,   221,     6,     7,     8,   284,     9,   454,
      70,    73,    10,   231,    11,    83,    56,   149,   209,    12,
      13,    14,   204,   316,   376,   375,   266,    16,   405,   285,
     460,   445,   287,   349,   288,   332,   446,   294,   293,   352,
     386,     0,   429,   191,   305
};

static const yytype_int16 yycheck[] =
{
       2,     3,     4,    74,     6,   106,   256,   130,    49,    11,
      12,    13,    53,   136,   294,    56,   310,    88,    92,    77,
      91,   272,     4,   219,   385,   365,    13,    13,   355,   356,
     312,   105,    13,    48,    56,    14,    38,    13,    40,    14,
      13,    13,    24,    45,   384,    47,    48,    49,    14,    54,
       4,    53,    22,    14,    56,    96,    14,    14,   338,    14,
      62,   422,    14,   313,    14,    35,   393,    72,   362,   169,
      24,    10,    45,    32,    96,   357,   134,   135,    80,    94,
      52,    68,    64,   334,   280,    72,    72,   414,   129,   130,
     148,    72,    94,    72,    96,   456,    98,    72,   100,   439,
     440,   462,   176,    60,   106,    62,    72,   401,   448,   436,
     360,    72,    62,   236,    72,    72,   157,    72,    13,    58,
      72,    62,    72,    13,   224,    19,    20,   129,   130,    24,
      13,   232,    56,    59,   136,    55,   194,    27,    32,    63,
     198,    19,    20,    54,    34,    35,    72,   427,   150,   272,
      11,    34,    35,    54,    19,   157,    56,   437,   160,   260,
      13,    26,    73,    28,    64,    18,    13,   169,    62,    64,
     172,    25,    62,    19,    59,   455,    13,   235,    54,    62,
      26,   239,    28,    20,    62,    22,   244,    41,    57,    54,
      27,    38,    29,    58,     0,   236,    43,    73,    35,    52,
      37,    13,   273,    40,     0,    54,    43,    56,    54,   211,
      72,   334,   214,    25,    26,    60,    28,    54,     9,   342,
      42,    58,   224,    34,    36,    36,    17,    72,    59,    41,
     232,   272,    54,    45,   236,   237,    58,    48,    50,   297,
     298,    53,    68,    42,    60,    71,    72,     6,     7,     8,
       9,    10,    11,    12,    13,    54,    72,    22,   260,    58,
       5,    56,    27,    60,    29,   267,    56,   269,    63,    14,
     272,    30,    37,    63,    54,   333,     6,     7,     8,     9,
      10,   339,    68,    13,    56,    44,    72,     6,     7,     8,
      56,    63,    56,   334,    13,    57,   354,    63,    54,    63,
     423,    60,   425,    62,   306,   307,    65,    66,    67,    68,
      59,    70,    56,    54,   385,   438,   318,   319,    54,    63,
     378,   392,    58,    54,    13,   327,    57,    56,    59,    58,
      54,    54,   334,    57,    60,    59,     6,     7,     8,   397,
     342,    54,   344,     6,     7,     8,     9,    10,    11,    12,
      13,   422,    69,    69,     6,     7,     8,     9,    10,    11,
      12,    13,    57,    62,    59,   367,    59,    30,    62,   371,
     428,   322,   323,    57,    61,    61,    56,    68,    30,   381,
      61,    44,    56,    60,    69,   456,    54,    60,    59,    50,
      22,   462,    44,    13,    54,    54,    13,    60,    54,    62,
      24,    64,    65,    66,    67,    68,     0,    70,    60,    59,
      62,   413,    63,    65,    66,    67,    68,    57,    70,    54,
      59,   423,    11,   425,    62,    54,    62,    21,    54,    23,
      61,    25,    59,    56,    56,    29,    13,    31,    32,    33,
      61,    35,    13,    59,    59,    39,    65,    41,    54,    20,
      60,    22,    46,    47,    48,    49,    27,    58,    29,    54,
      54,    58,    61,    54,    35,    73,    37,    13,    13,    40,
      54,    59,    43,    59,    21,    54,    23,    59,    25,    59,
      61,    54,    29,    54,    31,    32,    33,    58,    35,    61,
      19,    22,    39,   172,    41,    45,     9,    80,   164,    46,
      47,    48,   157,   261,   327,   325,   211,    54,   367,   224,
     441,   423,   226,   290,   226,   269,   425,   237,   236,   298,
     339,    -1,   397,   136,   252
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     0,    21,    23,    25,    29,    31,    32,    33,    35,
      39,    41,    46,    47,    48,    49,    54,    75,    76,    77,
      78,    80,    81,    82,    83,    84,    85,    86,    89,    95,
      98,   100,   102,   104,   127,   135,   180,    13,   189,   190,
     189,   189,   103,   104,    96,    97,   189,    19,    20,    62,
     105,   106,   107,   109,   110,    11,   109,   128,    32,   105,
     136,   189,   189,    90,   189,   104,     9,    17,     0,     0,
      80,    55,    79,    82,    72,   101,   189,    68,    99,   189,
     191,    14,    54,    96,   189,   108,   189,   108,    45,   111,
     112,   179,   189,    52,   113,   179,    18,   113,   129,   105,
      59,   137,    14,    87,    88,   189,    57,    91,    54,    54,
      25,    26,    28,    36,    41,    45,    50,    53,   190,    54,
       6,     7,     8,     9,    10,    11,    12,    30,    44,    60,
      62,    64,    65,    66,    67,    68,    70,   159,   160,   162,
     165,   171,   172,   173,   174,   177,   178,   189,    59,    99,
      59,   181,    54,    60,    14,    60,   190,    56,    63,   190,
      71,   191,   108,   113,   130,   189,   138,   139,   189,    57,
     140,    54,    56,   191,    92,    93,    94,   189,    54,   179,
     163,   164,   171,   179,    69,    13,   159,   159,   175,   176,
     171,   172,   189,    69,    62,   166,    14,   159,    59,   189,
      57,   182,    61,    61,   112,   189,    62,   114,   115,   114,
      14,    56,    14,    20,    22,    27,    29,    35,    37,    40,
      43,    54,    58,   126,   141,   142,   143,   144,   145,   150,
     179,    88,    56,    58,   191,    61,    56,    63,    69,    56,
      13,   159,   167,   168,    60,    54,   159,    14,   188,    42,
      54,    58,   183,   184,   185,    24,    64,   117,   118,   119,
     120,    60,   116,    57,    59,   131,   139,    59,   158,    27,
      34,    35,    62,    93,   146,   189,    50,    22,   151,   152,
      35,   151,    13,    54,    58,   142,    27,   144,   145,    54,
      13,    93,   159,   164,   165,   189,   159,    59,    56,    63,
     159,   169,   170,    54,    58,   184,    25,    41,   186,   187,
     118,    63,    56,    64,   121,    93,   117,    54,    57,    59,
     122,    19,    26,    28,    54,   132,   133,    57,   134,   189,
      54,    34,   146,    59,    62,   163,   147,   190,    14,    62,
      11,    54,    62,   153,   154,   151,    54,    59,   161,   161,
     166,   159,   168,    61,    56,   189,   189,    56,   121,     4,
      64,   119,   118,    13,    61,    38,   123,   124,   125,   126,
     189,    59,   134,   134,    58,   133,   123,    60,    59,   159,
     163,    63,    54,    57,    59,   166,   167,   155,   156,   157,
     171,   189,    65,   189,    54,   159,   192,    60,   159,   188,
     188,   118,   121,   192,    58,   125,    54,    54,   189,    58,
      61,   159,    54,    63,   189,    34,    36,    48,   148,   149,
     192,   147,    63,    56,    63,    59,   190,   188,    73,   169,
      13,    13,   121,    54,    60,    54,   189,   188,    59,    59,
      59,    54,    58,    54,   147,   156,   157,   189,    59,   166,
     159,    61,    54,    54,    61,   188,   166,   171,   192,   192,
     149,   192,   166,   147,   147
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
     148,   148,   149,   149,   149,   149,   150,   151,   152,   152,
     153,   154,   154,   155,   155,   156,   156,   157,   157,   158,
     159,   159,   160,   160,   160,   160,   160,   160,   160,   160,
     160,   160,   160,   161,   162,   162,   163,   163,   164,   164,
     165,   165,   166,   166,   167,   167,   168,   168,   169,   169,
     170,   170,   171,   171,   171,   172,   172,   173,   173,   174,
     175,   175,   176,   176,   177,   177,   178,   178,   178,   179,
     179,   179,   180,   181,   181,   182,   182,   183,   183,   184,
     184,   184,   185,   185,   186,   187,   188,   188,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   189,   190,   191,
     191,   192,   192
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
       1,     3,     3,     3,     3,     0,     3,     6,     0,     1,
       4,     0,     3,     1,     3,     3,     1,     1,     1,     4,
       1,     4,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     8,     5,     1,     3,     1,     1,
       1,     2,     0,     3,     1,     3,     1,     3,     0,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     3,
       0,     1,     1,     3,     1,     1,     2,     2,     2,     1,
       3,     2,     5,     0,     3,     2,     3,     1,     2,     2,
       2,     1,     0,     1,     5,     5,     0,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     3,
       3,     1,     3
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
      
#line 1774 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.c" /* yacc.c:1646  */
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
