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

    #include "ParseTree.hpp"
    using namespace ncbi::SchemaParser;

    #include "schema-tokens.h"
    #include "schema-lex.h"
    #define Schema_lex SchemaScan_yylex

    void Schema_error ( YYLTYPE *llocp, void* parser, struct SchemaScanBlock* sb, const char* msg )
    {
        /*TODO: send message to the C++ parser for proper display and recovery */
        printf("Line %i pos %i: %s\n", llocp -> first_line, llocp -> first_column, msg);
    }

    extern "C"
    {
        extern enum yytokentype SchemaScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, SchemaScanBlock* sb );
    }

    #define RULE_TOKEN(v,t) SchemaToken v = { t, NULL, 0, NULL, NULL };

#line 98 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:339  */

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
    KW_write = 309,
    PT_PARSE = 310,
    PT_SOURCE = 311,
    PT_VERSION_1_0 = 312,
    PT_SCHEMA_1_0 = 313,
    PT_INCLUDE = 314
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef SchemaToken YYSTYPE;
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



int Schema_parse (ParseTree** root, struct SchemaScanBlock* sb);

#endif /* !YY_SCHEMA_HOME_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 223 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   538

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  119
/* YYNRULES -- Number of rules.  */
#define YYNRULES  253
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  463

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   314

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    61,     2,     2,     2,
      66,    67,    70,    73,    62,    76,     2,    77,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    78,    60,
      68,    65,    69,     2,    72,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    74,     2,    75,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    63,    71,    64,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   138,   138,   142,   149,   154,   159,   167,   175,   179,
     185,   193,   194,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   219,   220,   224,
     228,   238,   245,   246,   250,   251,   257,   261,   265,   271,
     272,   276,   277,   281,   288,   289,   293,   297,   304,   305,
     309,   316,   320,   327,   331,   338,   342,   343,   344,   352,
     356,   360,   361,   365,   366,   370,   374,   375,   379,   380,
     384,   385,   389,   390,   394,   398,   402,   403,   404,   405,
     406,   410,   411,   415,   419,   420,   424,   425,   429,   430,
     431,   435,   439,   440,   444,   445,   449,   450,   456,   460,
     467,   468,   472,   476,   477,   481,   482,   486,   487,   488,
     489,   493,   499,   505,   509,   510,   514,   515,   519,   520,
     524,   525,   529,   530,   534,   535,   536,   537,   538,   539,
     540,   541,   542,   546,   547,   551,   552,   553,   557,   558,
     559,   560,   564,   565,   566,   567,   568,   572,   573,   574,
     578,   579,   583,   584,   585,   586,   590,   594,   595,   599,
     600,   604,   608,   609,   613,   614,   618,   619,   623,   624,
     628,   635,   636,   640,   641,   645,   646,   647,   648,   649,
     650,   651,   652,   653,   654,   658,   662,   666,   672,   673,
     677,   678,   682,   683,   687,   688,   692,   693,   697,   698,
     702,   703,   707,   708,   712,   713,   714,   718,   719,   723,
     724,   728,   731,   732,   736,   737,   741,   742,   746,   747,
     748,   752,   753,   754,   760,   764,   765,   769,   770,   774,
     775,   779,   780,   781,   785,   786,   790,   794,   798,   799,
     805,   806,   808,   809,   810,   811,   812,   813,   814,   815,
     819,   820,   824,   825
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
  "KW_version", "KW_view", "KW_virtual", "KW_void", "KW_write", "PT_PARSE",
  "PT_SOURCE", "PT_VERSION_1_0", "PT_SCHEMA_1_0", "PT_INCLUDE", "';'",
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
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
      59,    36,    44,   123,   125,    61,    40,    41,    60,    62,
      42,   124,    64,    43,    91,    93,    45,    47,    58
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
     365,  -369,   309,   309,   309,    39,   309,    56,    20,    62,
     186,   309,   309,   309,    39,   187,  -369,   175,   202,   436,
     176,  -369,   436,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,    16,
    -369,    12,    18,  -369,  -369,   207,   309,   -18,   309,   309,
     123,  -369,  -369,  -369,    69,  -369,  -369,   146,  -369,    56,
    -369,   214,    34,    16,   233,   224,  -369,   273,   283,  -369,
    -369,  -369,  -369,  -369,  -369,   321,   314,   224,   170,   240,
     224,   309,   263,  -369,   320,   224,   315,    36,   324,   309,
     228,  -369,   309,   268,  -369,   309,  -369,    69,  -369,   309,
    -369,   309,   322,  -369,   201,  -369,   -33,   309,   326,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
     309,   363,   317,  -369,   179,   179,   346,   318,  -369,  -369,
     328,  -369,  -369,  -369,  -369,  -369,  -369,    23,   179,   329,
     309,   337,  -369,   335,  -369,   342,  -369,   123,  -369,  -369,
     309,  -369,   336,  -369,   336,    41,   349,  -369,    43,   403,
    -369,  -369,   309,  -369,   244,  -369,   344,   224,  -369,   352,
     232,  -369,  -369,  -369,  -369,  -369,  -369,   348,   359,  -369,
    -369,   224,  -369,   264,   361,  -369,   370,   179,    53,   154,
    -369,  -369,  -369,  -369,   224,    46,   366,  -369,   167,  -369,
     309,  -369,   369,   126,   384,  -369,   414,  -369,   121,   427,
    -369,  -369,   382,   415,  -369,   291,  -369,  -369,   386,   435,
    -369,   309,  -369,  -369,   179,   363,   309,  -369,   179,   385,
    -369,   248,  -369,   179,  -369,   389,  -369,  -369,  -369,  -369,
    -369,   222,  -369,   100,  -369,   429,   383,   218,  -369,   309,
     128,   319,   221,   392,  -369,  -369,   309,   397,   136,   396,
     405,   363,   309,  -369,    35,   453,  -369,   411,   406,   414,
     417,   400,  -369,  -369,  -369,  -369,  -369,  -369,  -369,   400,
    -369,  -369,  -369,   328,    54,  -369,   179,   264,  -369,  -369,
     413,   419,  -369,  -369,  -369,   309,   309,  -369,  -369,   420,
    -369,    15,   429,  -369,   473,   421,  -369,   245,   309,  -369,
     422,   392,   392,  -369,   196,  -369,   245,  -369,    67,  -369,
     424,  -369,   179,   363,   256,  -369,   343,   328,   264,  -369,
    -369,   363,   476,   309,   431,  -369,   179,  -369,  -369,   426,
    -369,  -369,  -369,   179,    53,    53,   116,  -369,  -369,   429,
    -369,   420,  -369,  -369,   179,   430,   245,  -369,   433,    65,
     309,  -369,  -369,  -369,  -369,   434,   428,   179,   437,   258,
     309,  -369,   250,   179,   309,   282,   304,  -369,  -369,  -369,
     157,   438,    53,  -369,   439,  -369,   179,  -369,   486,   487,
     420,  -369,   160,  -369,  -369,  -369,  -369,    85,  -369,  -369,
     441,  -369,   309,    53,   440,   442,   443,     6,  -369,   168,
    -369,   309,   363,  -369,   363,   179,   328,   179,   444,   446,
     449,  -369,  -369,   445,  -369,    53,   328,   285,   179,   179,
     250,  -369,  -369,  -369,  -369,  -369,   224,   439,  -369,  -369,
    -369,  -369,  -369,  -369,   328,   309,  -369,   439,   439,  -369,
     309,  -369,  -369
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
    -369,  -369,  -369,  -369,  -369,  -369,   483,  -369,   482,  -369,
    -369,  -369,  -369,  -369,   341,  -369,  -369,  -369,  -369,   -93,
    -369,  -369,   468,  -369,  -369,   447,  -369,  -369,  -369,  -369,
     198,    14,  -369,  -369,     5,   506,  -369,  -369,   360,   -23,
     354,  -369,  -369,   259,  -249,  -204,  -369,  -294,  -369,   190,
    -369,   155,  -142,  -369,  -369,  -369,  -369,  -369,  -369,   199,
      40,  -369,  -369,  -369,  -369,   310,  -369,  -369,   299,  -369,
     300,   301,   261,  -368,  -369,    84,  -369,  -195,  -369,  -369,
    -369,  -369,   105,   106,  -369,  -321,   -70,  -369,   242,  -369,
    -235,   297,   298,  -280,   195,   238,   140,  -369,  -124,   401,
    -369,  -369,  -369,  -369,  -369,  -369,   -22,  -369,  -369,  -369,
    -369,   287,  -369,  -369,  -369,  -333,    -2,   -72,   -88
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
      63,    65,   189,   349,   175,   357,   420,   156,   173,   358,
     159,   398,   399,   280,    60,    37,    38,   222,    92,    37,
      38,    56,    96,    82,    98,    96,   334,    77,   195,    80,
     254,    78,   -46,   402,    85,    75,    87,    87,    93,   103,
     337,   154,    93,   443,    88,    93,   209,   384,   211,   426,
      75,   106,   419,   361,   185,   186,   440,   401,   246,   195,
     441,   254,     7,   100,   163,    96,    48,    49,   196,    80,
     436,   222,    37,    38,   344,   359,    78,   461,   233,  -192,
      75,  -192,   462,    87,    75,    93,    75,   165,   379,   168,
     162,    75,   454,   338,   447,   177,   431,   360,   179,   183,
     400,   182,    75,    75,    75,   -76,   255,   457,   458,    75,
     358,    75,    94,   240,    50,   406,   305,   245,    93,    93,
      50,    75,    75,   376,   191,    92,    37,    38,   290,    37,
      38,   254,   306,    75,   276,    75,   448,   182,   198,    37,
      38,   433,   360,   254,   268,    93,   455,   279,   204,    37,
      38,   269,   270,    75,   291,    97,   314,    93,   295,    89,
     106,   330,   270,   299,   460,    69,   121,   122,   123,   124,
     125,   126,   127,    37,    38,   121,   122,   123,   124,   125,
     126,   127,    37,    38,   271,   -76,    67,   248,   255,    94,
     336,   128,    70,    44,   271,    68,    48,    49,   168,   182,
     128,   274,    66,   183,   249,   129,   320,   389,   250,    59,
     432,    93,   424,   321,   129,   322,   350,   240,   442,   177,
     262,   427,   263,    93,   294,    75,   130,    72,   131,   427,
     132,   320,   133,   134,   135,   130,   136,   131,   321,   183,
     322,   133,   134,   135,    50,   136,   323,   177,    37,    38,
     373,   171,   378,   172,   328,   248,   274,    83,   240,    93,
     121,   122,   123,   124,   125,   126,   127,   239,    38,   101,
     311,   323,   249,   397,   364,   414,   303,   415,   312,   219,
     157,   121,   122,   123,   235,   128,   107,   158,   389,   416,
     389,   236,    75,   354,   355,   148,   231,   410,   232,   129,
     297,   183,   336,   456,   213,    93,   369,   298,   235,   285,
     235,   215,    37,    38,    93,   380,   299,   412,   150,   217,
     130,    93,   131,   109,    37,    38,   133,   134,   135,   390,
     136,   392,    78,   110,   297,   160,    75,   111,   112,   336,
     113,   421,   121,   122,   123,   124,   125,   449,   114,    37,
      38,   371,   372,   115,    93,     1,   422,   116,   407,   121,
     122,   123,   117,   423,   120,   118,    37,    38,   413,   316,
     152,   153,   317,   336,   318,   169,   178,     2,   336,     3,
     155,     4,   184,   192,   197,     5,   193,     6,     7,     8,
     199,     9,   201,   381,   205,    10,   382,    11,   383,   202,
     435,   210,    12,    13,    14,    15,    37,    38,    78,   234,
     390,   238,   446,   237,   212,    16,   213,   243,    37,    38,
     244,   214,   260,   215,   266,   275,   212,   276,   213,   216,
     281,   217,   282,   214,   218,   215,   288,   219,   289,   302,
     296,   216,   310,   217,   254,   326,   218,   329,     2,   219,
       3,   332,     4,   220,   339,   346,     5,   221,     6,     7,
       8,   340,     9,   333,   341,   220,    10,   345,    11,   283,
     352,   353,   356,    12,    13,    14,   362,   370,   363,   377,
     391,   393,   396,   405,   403,   409,    16,   411,   408,   429,
     430,   434,    71,   425,    74,   437,   451,   438,   439,   452,
     427,   450,   453,   230,    84,    57,   375,   203,   208,   315,
     265,   404,   284,   374,   459,   286,   287,   444,   149,   331,
     445,   348,   292,   385,   293,   351,   428,   190,   304
};

static const yytype_uint16 yycheck[] =
{
       2,     3,     4,    75,     6,    93,   255,   131,    78,    11,
      12,    13,   136,   293,   107,   309,   384,    89,   106,     4,
      92,   354,   355,   218,    10,    13,    14,   169,    50,    13,
      14,    11,    54,    15,    57,    57,   271,    39,    15,    41,
      25,    74,    60,   364,    46,    78,    48,    49,    50,    15,
      15,    15,    54,   421,    49,    57,    15,   337,    15,   392,
      78,    63,   383,   312,   134,   135,    60,   361,    15,    15,
      64,    25,    33,    59,    97,    97,    20,    21,   148,    81,
     413,   223,    13,    14,   279,    70,    74,   455,   176,    66,
      78,    68,   460,    95,    78,    97,    78,    99,   333,   101,
      95,    78,   435,    68,   425,   107,   400,   311,   130,   131,
     359,   235,    78,    78,    78,    69,    70,   438,   439,    78,
       4,    78,    53,   193,    68,    60,    26,   197,   130,   131,
      68,    78,    78,    66,   136,   157,    13,    14,   231,    13,
      14,    25,    42,    78,    23,    78,   426,   271,   150,    13,
      14,    66,   356,    25,    28,   157,   436,    36,   160,    13,
      14,    35,    36,    78,   234,    19,   259,   169,   238,    46,
     172,    35,    36,   243,   454,     0,     6,     7,     8,     9,
      10,    11,    12,    13,    14,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    68,    67,     9,    43,    70,    53,
     272,    31,     0,     5,    68,    18,    20,    21,   210,   333,
      31,   213,    14,   235,    60,    45,    20,   341,    64,    33,
      60,   223,    65,    27,    45,    29,   296,   297,    60,   231,
      63,    71,    65,   235,   236,    78,    66,    61,    68,    71,
      70,    20,    72,    73,    74,    66,    76,    68,    27,   271,
      29,    72,    73,    74,    68,    76,    60,   259,    13,    14,
      64,    60,   332,    62,   266,    43,   268,    60,   338,   271,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    65,
      62,    60,    60,   353,    39,    35,    64,    37,    70,    44,
      62,     6,     7,     8,    62,    31,    63,    69,   422,    49,
     424,    69,    78,   305,   306,    65,    62,   377,    64,    45,
      62,   333,   384,   437,    23,   317,   318,    69,    62,    28,
      62,    30,    13,    14,   326,    69,   396,    69,    65,    38,
      66,   333,    68,    60,    13,    14,    72,    73,    74,   341,
      76,   343,    74,    60,    62,    77,    78,    26,    27,   421,
      29,    69,     6,     7,     8,     9,    10,   427,    37,    13,
      14,   321,   322,    42,   366,     0,    62,    46,   370,     6,
       7,     8,    51,    69,    60,    54,    13,    14,   380,    60,
      60,    66,    63,   455,    65,    63,    60,    22,   460,    24,
      66,    26,    75,    75,    65,    30,    68,    32,    33,    34,
      63,    36,    67,    60,    68,    40,    63,    42,    65,    67,
     412,    62,    47,    48,    49,    50,    13,    14,    74,    67,
     422,    62,   424,    75,    21,    60,    23,    66,    13,    14,
      60,    28,    66,    30,    65,    51,    21,    23,    23,    36,
      13,    38,    60,    28,    41,    30,    60,    44,    13,    60,
      65,    36,    69,    38,    25,    63,    41,    60,    22,    44,
      24,    65,    26,    60,    11,    65,    30,    64,    32,    33,
      34,    60,    36,    68,    68,    60,    40,    60,    42,    64,
      67,    62,    62,    47,    48,    49,    13,    65,    67,    65,
      14,    60,    66,    60,    64,    67,    60,    60,    64,    13,
      13,    60,    19,    65,    22,    65,    60,    65,    65,    60,
      71,    67,    67,   172,    46,     9,   326,   157,   164,   260,
     210,   366,   223,   324,   440,   225,   225,   422,    81,   268,
     424,   289,   235,   338,   236,   297,   396,   136,   251
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     0,    22,    24,    26,    30,    32,    33,    34,    36,
      40,    42,    47,    48,    49,    50,    60,    80,    81,    82,
      83,    85,    86,    87,    88,    89,    90,    91,    94,   100,
     103,   105,   107,   109,   132,   140,   186,    13,    14,   195,
     196,   195,   195,   108,   109,   101,   102,   195,    20,    21,
      68,   110,   111,   112,   114,   115,    11,   114,   133,    33,
     110,   141,   195,   195,    95,   195,   109,     9,    18,     0,
       0,    85,    61,    84,    87,    78,   106,   195,    74,   104,
     195,   197,    15,    60,   101,   195,   113,   195,   113,    46,
     116,   117,   185,   195,    53,   118,   185,    19,   118,   134,
     110,    65,   142,    15,    92,    93,   195,    63,    96,    60,
      60,    26,    27,    29,    37,    42,    46,    51,    54,   196,
      60,     6,     7,     8,     9,    10,    11,    12,    31,    45,
      66,    68,    70,    72,    73,    74,    76,   165,   166,   168,
     171,   177,   178,   179,   180,   183,   184,   195,    65,   104,
      65,   187,    60,    66,    15,    66,   196,    62,    69,   196,
      77,   197,   113,   118,   135,   195,   143,   144,   195,    63,
     145,    60,    62,   197,    97,    98,    99,   195,    60,   185,
     169,   170,   177,   185,    75,   165,   165,   181,   182,   177,
     178,   195,    75,    68,   172,    15,   165,    65,   195,    63,
     188,    67,    67,   117,   195,    68,   119,   120,   119,    15,
      62,    15,    21,    23,    28,    30,    36,    38,    41,    44,
      60,    64,   131,   146,   147,   148,   149,   150,   155,   185,
      93,    62,    64,   197,    67,    62,    69,    75,    62,    13,
     165,   173,   174,    66,    60,   165,    15,   194,    43,    60,
      64,   189,   190,   191,    25,    70,   122,   123,   124,   125,
      66,   121,    63,    65,   136,   144,    65,   163,    28,    35,
      36,    68,    98,   151,   195,    51,    23,   156,   157,    36,
     156,    13,    60,    64,   147,    28,   149,   150,    60,    13,
      98,   165,   170,   171,   195,   165,    65,    62,    69,   165,
     175,   176,    60,    64,   190,    26,    42,   192,   193,   123,
      69,    62,    70,   126,    98,   122,    60,    63,    65,   127,
      20,    27,    29,    60,   137,   138,    63,   139,   195,    60,
      35,   151,    65,    68,   169,   152,   196,    15,    68,    11,
      60,    68,   158,   159,   156,    60,    65,   167,   167,   172,
     165,   174,    67,    62,   195,   195,    62,   126,     4,    70,
     124,   123,    13,    67,    39,   128,   129,   130,   131,   195,
      65,   139,   139,    64,   138,   128,    66,    65,   165,   169,
      69,    60,    63,    65,   172,   173,   160,   161,   162,   177,
     195,    14,   195,    60,   164,   165,    66,   165,   194,   194,
     123,   126,   164,    64,   130,    60,    60,   195,    64,    67,
     165,    60,    69,   195,    35,    37,    49,   153,   154,   164,
     152,    69,    62,    69,    65,    65,   194,    71,   175,    13,
      13,   126,    60,    66,    60,   195,   194,    65,    65,    65,
      60,    64,    60,   152,   161,   162,   195,   164,   172,   165,
      67,    60,    60,    67,   194,   172,   177,   164,   164,   154,
     172,   152,   152
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    79,    80,    80,    81,    81,    81,    82,    83,    84,
      85,    86,    86,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    88,    88,    89,
      90,    91,    92,    92,    93,    93,    94,    95,    96,    97,
      97,    98,    98,    99,   100,   100,   101,   102,   103,   103,
     104,   105,   106,   107,   108,   109,   110,   110,   110,   111,
     112,   113,   113,   114,   114,   115,   116,   116,   117,   117,
     118,   118,   119,   119,   120,   121,   122,   122,   122,   122,
     122,   123,   123,   124,   125,   125,   126,   126,   127,   127,
     127,   128,   129,   129,   130,   130,   131,   131,   132,   133,
     134,   134,   135,   136,   136,   137,   137,   138,   138,   138,
     138,   139,   140,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   146,   146,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   148,   148,   149,   149,   149,   150,   150,
     150,   150,   151,   151,   151,   151,   151,   152,   152,   152,
     153,   153,   154,   154,   154,   154,   155,   156,   156,   157,
     157,   158,   159,   159,   160,   160,   161,   161,   162,   162,
     163,   164,   164,   165,   165,   166,   166,   166,   166,   166,
     166,   166,   166,   166,   166,   167,   168,   168,   169,   169,
     170,   170,   171,   171,   172,   172,   173,   173,   174,   174,
     175,   175,   176,   176,   177,   177,   177,   178,   178,   179,
     179,   180,   181,   181,   182,   182,   183,   183,   184,   184,
     184,   185,   185,   185,   186,   187,   187,   188,   188,   189,
     189,   190,   190,   190,   191,   191,   192,   193,   194,   194,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     196,   196,   197,   197
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
      yyerror (&yylloc, root, sb, YY_("syntax error: cannot back up")); \
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
                  Type, Value, Location, root, sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseTree** root, struct SchemaScanBlock* sb)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (root);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseTree** root, struct SchemaScanBlock* sb)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, root, sb);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, ParseTree** root, struct SchemaScanBlock* sb)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , root, sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, root, sb); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ParseTree** root, struct SchemaScanBlock* sb)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (root);
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
yyparse (ParseTree** root, struct SchemaScanBlock* sb)
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
#line 138 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_PARSE);
                                *root =  new ParseTree ( t, new ParseTree ( (yyvsp[0]) ) );
                            }
#line 1801 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 142 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_PARSE);
                                *root =  new ParseTree ( t, ( ParseTree* ) (yyvsp[-1]) . subtree, new ParseTree ( (yyvsp[0]) ) );
                            }
#line 1810 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 150 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_SOURCE);
                                (yyval) . subtree = new ParseTree ( t, ( ParseTree* ) (yyvsp[0]) . subtree );
                            }
#line 1819 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 155 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_SOURCE);
                                (yyval) . subtree = new ParseTree ( t, ( ParseTree* ) (yyvsp[-1]) . subtree, ( ParseTree* ) (yyvsp[0]) . subtree );
                            }
#line 1828 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 160 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_SOURCE);
                                (yyval) . subtree = new ParseTree ( t, ( ParseTree* ) (yyvsp[-1]) . subtree, ( ParseTree* ) (yyvsp[0]) . subtree );
                            }
#line 1837 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 168 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_VERSION_1_0);
                                (yyval) . subtree = new ParseTree ( t, new ParseTree ( (yyvsp[-2]) ), new ParseTree ( (yyvsp[-1]) ), new ParseTree ( (yyvsp[0]) ) );
                            }
#line 1846 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 186 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_SCHEMA_1_0);
                                (yyval) . subtree = new ParseTree ( t, ( ParseTree* ) (yyvsp[0]) . subtree );
                            }
#line 1855 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 193 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = (yyvsp[0]) . subtree; }
#line 1861 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 195 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                ( ( ParseTree* ) (yyvsp[-1]) . subtree ) -> AddChild ( ( ParseTree* ) (yyvsp[0]) . subtree );
                                (yyval) . subtree = (yyvsp[-1]) . subtree;
                            }
#line 1870 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 214 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = (yyvsp[0]) . subtree; }
#line 1876 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 215 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = new ParseTree ( (yyvsp[0]) ); }
#line 1882 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 229 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    {
                                RULE_TOKEN ( t, PT_INCLUDE);
                                (yyval) . subtree = new ParseTree ( t, new ParseTree ( (yyvsp[-1]) ), new ParseTree ( (yyvsp[0]) ) );
                            }
#line 1891 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 1895 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
      yyerror (&yylloc, root, sb, YY_("syntax error"));
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
        yyerror (&yylloc, root, sb, yymsgp);
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
                      yytoken, &yylval, &yylloc, root, sb);
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
                  yystos[yystate], yyvsp, yylsp, root, sb);
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
  yyerror (&yylloc, root, sb, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, root, sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, root, sb);
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
