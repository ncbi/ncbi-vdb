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
#define yyparse         AST_parse
#define yylex           AST_lex
#define yyerror         AST_error
#define yydebug         AST_debug
#define yynerrs         AST_nerrs


/* Copy the first part of user declarations.  */
#line 27 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:339  */

    #define YYDEBUG 1

    #include <stdio.h>

    #include "ASTBuilder.hpp"
    using namespace ncbi::SchemaParser;

    #include "schema-ast-tokens.h"

    #define AST_lex NextToken
    static int NextToken ( YYSTYPE* p_token, ParseTreeScanner& p_sb )
    {
        return p_sb . NextToken ( p_token -> tok );
    }

    void AST_error ( void* parser, ASTBuilder& p_builder, ParseTreeScanner&, const char* msg )
    {
        /*TODO: retrieve name/location data from the scanner, add to the report */
        p_builder . ReportError ( "%s", msg );
    }


#line 96 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:339  */

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
   by #include "schema-ast-tokens.h".  */
#ifndef YY_AST_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED
# define YY_AST_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int AST_debug;
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
    PT_ASTLIST = 310,
    PT_PARSE = 311,
    PT_SOURCE = 312,
    PT_VERSION_1_0 = 313,
    PT_VERSION_2 = 314,
    PT_SCHEMA_1_0 = 315,
    PT_INCLUDE = 316,
    PT_TYPEDEF = 317,
    PT_FQN = 318,
    PT_IDENT = 319,
    PT_PHYSIDENT = 320,
    PT_UINT = 321,
    PT_TYPESET = 322,
    PT_TYPESETDEF = 323,
    PT_FORMAT = 324,
    PT_CONST = 325,
    PT_ALIAS = 326,
    PT_EXTERN = 327,
    PT_FUNCTION = 328,
    PT_UNTYPED = 329,
    PT_ROWLENGTH = 330,
    PT_FUNCDECL = 331,
    PT_EMPTY = 332,
    PT_SCHEMASIG = 333,
    PT_SCHEMAFORMAL = 334,
    PT_RETURNTYPE = 335,
    PT_FACTSIG = 336,
    PT_FUNCSIG = 337,
    PT_FUNCPARAMS = 338,
    PT_FORMALPARAM = 339,
    PT_ELLIPSIS = 340,
    PT_FUNCPROLOGUE = 341,
    PT_RETURN = 342,
    PT_PRODSTMT = 343,
    PT_PRODTRIGGER = 344,
    PT_SCHEMA = 345,
    PT_VALIDATE = 346,
    PT_PHYSICAL = 347,
    PT_PHYSPROLOGUE = 348,
    PT_PHYSSTMT = 349,
    PT_PHYSBODYSTMT = 350,
    PT_TABLE = 351,
    PT_TABLEPARENTS = 352,
    PT_TABLEBODY = 353,
    PT_FUNCEXPR = 354,
    PT_FACTPARMS = 355,
    PT_COLUMN = 356,
    PT_COLUMNEXPR = 357,
    PT_COLDECL = 358,
    PT_TYPEDCOL = 359,
    PT_COLSTMT = 360,
    PT_DFLTVIEW = 361,
    PT_PHYSMBR = 362,
    PT_PHYSCOL = 363,
    PT_PHYSCOLDEF = 364,
    PT_COLSCHEMAPARMS = 365,
    PT_COLSCHEMAPARAM = 366,
    PT_COLUNTYPED = 367,
    PT_DATABASE = 368,
    PT_TYPEEXPR = 369,
    PT_DBBODY = 370,
    PT_DBDAD = 371,
    PT_DBMEMBER = 372,
    PT_TBLMEMBER = 373,
    PT_NOHEADER = 374,
    PT_CASTEXPR = 375,
    PT_CONSTVECT = 376,
    PT_NEGATE = 377,
    PT_UNARYPLUS = 378,
    PT_VERSNAME = 379,
    PT_ARRAY = 380,
    PT_AT = 381,
    PT_PHYSENCEXPR = 382,
    PT_PHYSENCREF = 383,
    PT_TYPEDCOLEXPR = 384
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 58 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:355  */

  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;
  AST_ParamSig* paramSig;
  bool          boolean;

#line 276 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int AST_parse (AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb);

#endif /* !YY_AST_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:358  */

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
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   649

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  146
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  85
/* YYNRULES -- Number of rules.  */
#define YYNRULES  176
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  632

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   384

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     130,   131,   136,     2,   133,     2,     2,   144,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   145,   132,
     140,   139,   141,     2,   142,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   134,     2,   135,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   137,   143,   138,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   251,   251,   252,   256,   257,   258,   262,   266,   270,
     274,   279,   280,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   303,   308,   309,
     313,   314,   318,   322,   323,   327,   332,   333,   337,   338,
     342,   346,   350,   354,   355,   356,   361,   362,   366,   367,
     371,   372,   376,   377,   381,   382,   386,   390,   391,   392,
     393,   394,   398,   402,   403,   407,   408,   412,   413,   417,
     418,   419,   423,   427,   428,   432,   433,   437,   438,   442,
     445,   446,   450,   455,   464,   469,   470,   473,   475,   480,
     481,   485,   487,   489,   491,   498,   503,   504,   508,   509,
     513,   514,   518,   519,   523,   524,   525,   529,   533,   535,
     540,   541,   545,   546,   550,   551,   552,   556,   557,   562,
     563,   564,   568,   570,   572,   577,   578,   582,   583,   587,
     588,   592,   593,   594,   595,   601,   605,   606,   609,   610,
     614,   615,   616,   617,   621,   623,   628,   629,   635,   641,
     642,   643,   644,   649,   653,   654,   658,   659,   663,   664,
     665,   676,   677,   681,   682,   683,   687,   688,   692,   693,
     699,   703,   704,   708,   712,   713,   717
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
  "KW_version", "KW_view", "KW_virtual", "KW_void", "KW_write",
  "PT_ASTLIST", "PT_PARSE", "PT_SOURCE", "PT_VERSION_1_0", "PT_VERSION_2",
  "PT_SCHEMA_1_0", "PT_INCLUDE", "PT_TYPEDEF", "PT_FQN", "PT_IDENT",
  "PT_PHYSIDENT", "PT_UINT", "PT_TYPESET", "PT_TYPESETDEF", "PT_FORMAT",
  "PT_CONST", "PT_ALIAS", "PT_EXTERN", "PT_FUNCTION", "PT_UNTYPED",
  "PT_ROWLENGTH", "PT_FUNCDECL", "PT_EMPTY", "PT_SCHEMASIG",
  "PT_SCHEMAFORMAL", "PT_RETURNTYPE", "PT_FACTSIG", "PT_FUNCSIG",
  "PT_FUNCPARAMS", "PT_FORMALPARAM", "PT_ELLIPSIS", "PT_FUNCPROLOGUE",
  "PT_RETURN", "PT_PRODSTMT", "PT_PRODTRIGGER", "PT_SCHEMA", "PT_VALIDATE",
  "PT_PHYSICAL", "PT_PHYSPROLOGUE", "PT_PHYSSTMT", "PT_PHYSBODYSTMT",
  "PT_TABLE", "PT_TABLEPARENTS", "PT_TABLEBODY", "PT_FUNCEXPR",
  "PT_FACTPARMS", "PT_COLUMN", "PT_COLUMNEXPR", "PT_COLDECL",
  "PT_TYPEDCOL", "PT_COLSTMT", "PT_DFLTVIEW", "PT_PHYSMBR", "PT_PHYSCOL",
  "PT_PHYSCOLDEF", "PT_COLSCHEMAPARMS", "PT_COLSCHEMAPARAM",
  "PT_COLUNTYPED", "PT_DATABASE", "PT_TYPEEXPR", "PT_DBBODY", "PT_DBDAD",
  "PT_DBMEMBER", "PT_TBLMEMBER", "PT_NOHEADER", "PT_CASTEXPR",
  "PT_CONSTVECT", "PT_NEGATE", "PT_UNARYPLUS", "PT_VERSNAME", "PT_ARRAY",
  "PT_AT", "PT_PHYSENCEXPR", "PT_PHYSENCREF", "PT_TYPEDCOLEXPR", "'('",
  "')'", "';'", "','", "'['", "']'", "'*'", "'{'", "'}'", "'='", "'<'",
  "'>'", "'@'", "'|'", "'/'", "':'", "$accept", "parse", "source",
  "version_1", "version_2", "schema_1", "schema_2", "schema_decls",
  "schema_decl", "typedef", "new_type_names", "typespec", "arrayspec",
  "dim", "typeset", "typeset_spec", "fmtdef", "const", "alias", "function",
  "func_decl", "schema_sig_opt", "schema_formals", "schema_formal",
  "return_type", "fact_sig", "param_sig", "param_signature",
  "formals_list", "formals", "formal", "vararg", "prologue",
  "script_prologue", "script_stmts", "script_stmt", "production",
  "extern_function", "script", "script_decl", "validate", "physical",
  "phys_return_type", "phys_prologue", "phys_body", "phys_body_stmt",
  "table", "parents_opt", "tbl_parents", "tbl_body", "tbl_stmts",
  "tbl_stmt", "column_decl", "column_expr", "col_modifiers_opt",
  "col_modifiers", "col_modifier", "col_decl", "phys_enc_ref", "typed_col",
  "col_ident", "col_body_opt", "col_body", "col_stmt", "database",
  "opt_dbdad", "dbbody", "db_members", "db_member", "template_opt",
  "include", "expr", "cond_expr", "cond_chain", "type_expr", "uint_expr",
  "schema_parms", "schema_parm", "factory_parms_opt", "factory_parms",
  "fqn", "qualnames", "ident", "fqn_opt_vers", "fqn_vers", YY_NULLPTR
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
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
      40,    41,    59,    44,    91,    93,    42,   123,   125,    61,
      60,    62,    64,   124,    47,    58
};
# endif

#define YYPACT_NINF -520

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-520)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -5,   -73,    74,    83,  -520,   -56,    56,   138,  -520,   207,
      16,   129,   132,   151,   144,  -520,    46,  -520,   164,   238,
     150,   168,   169,  -520,   274,   286,   171,   172,   173,   175,
     176,   177,   178,   179,   180,   182,   183,   184,   185,  -520,
      84,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,
    -520,  -520,  -520,  -520,  -520,  -520,  -520,   186,   188,   277,
     269,   271,   285,   297,   300,   293,   291,   287,   276,   290,
     288,   302,  -520,  -520,   200,   201,   318,   272,   272,   272,
      -2,   272,   261,   194,     8,   263,   220,   213,   213,  -520,
    -520,   208,   210,   283,   273,   -15,   212,   272,  -520,  -520,
     272,   214,   216,   217,   218,   219,   275,   223,   221,   224,
    -520,   225,   -34,   226,    95,   -23,  -520,   294,   227,   229,
     230,   228,   272,   231,   232,  -520,   342,   345,   220,  -520,
     235,   220,  -520,   334,   233,   239,   241,  -520,   213,   272,
    -520,   242,   270,  -520,   244,   262,   246,     4,    -2,   243,
    -520,   247,   245,     2,   250,   272,   272,   303,  -520,   303,
     308,   330,   -11,   367,   174,   372,   249,   259,   252,   266,
     264,   379,  -520,   333,   100,  -520,   343,  -520,     1,  -520,
     279,  -520,   267,  -520,  -520,  -520,   280,   281,     6,     6,
     282,   284,   292,   289,  -520,   296,   303,  -520,   295,   307,
     298,   346,   268,     6,   278,  -520,   299,   301,   304,    -2,
     305,  -520,   306,  -520,   265,   309,   311,   312,  -520,   174,
    -520,   174,   220,   323,  -520,   272,  -520,   313,   310,   315,
     316,  -520,   319,   348,   317,   -30,  -520,   393,   320,  -520,
      -2,   321,   322,   324,   325,  -520,   326,   327,   331,   331,
     303,   329,   146,  -520,   328,  -520,   181,   141,  -520,     6,
     332,   335,  -520,   337,   338,   339,  -520,  -520,   152,  -520,
    -520,  -520,  -520,  -520,  -520,   341,   336,   347,     6,   -19,
     344,   323,   272,  -520,   349,   350,   352,   340,   153,  -520,
     159,   351,   107,  -520,  -520,   354,    -2,   353,   356,   357,
    -520,   359,   362,   174,   364,   364,   363,  -520,   365,   -26,
     368,   360,   370,   371,     6,   373,   374,   375,   376,  -520,
     101,  -520,  -520,  -520,   378,   377,   380,  -520,    76,  -520,
     381,  -520,   181,   117,  -520,   358,  -520,   331,   383,   384,
     385,  -520,  -520,   388,   361,     3,  -520,   366,   386,  -520,
    -520,   -20,   394,    17,   395,  -520,  -520,  -520,    10,    10,
     382,  -520,  -520,   314,   390,   391,   399,   272,   336,   404,
    -520,  -520,   355,   114,  -520,   396,   389,   361,   397,   400,
     -45,  -520,   405,   364,   364,  -520,   401,   398,   222,  -520,
    -520,   406,   421,   402,   403,   407,  -520,   409,   392,   410,
     411,   413,   156,  -520,   414,   415,   361,   114,  -520,    -1,
     408,  -520,   417,   412,   416,   198,   419,   422,   430,   418,
       6,     6,  -520,  -520,  -520,   187,   423,   425,  -520,   -16,
    -520,   355,  -520,   433,   114,   427,   420,   435,   438,   429,
     431,   187,   432,   432,  -520,  -520,  -520,   215,  -520,    -7,
    -520,   424,     2,   364,   364,   434,    75,  -520,  -520,  -520,
     436,    -2,   455,  -520,   437,   439,  -520,   272,   441,   442,
    -520,  -520,    78,   443,   444,   445,   447,  -520,   449,    21,
      21,     2,   448,   450,   451,   459,   428,  -520,  -520,   461,
     453,  -520,  -520,   456,   452,   454,   457,     2,   462,   463,
    -520,   -27,   458,   460,   465,   466,   467,   469,   470,   471,
     432,   472,   473,  -520,   474,   480,   481,   475,  -520,    87,
    -520,  -520,   482,   387,    27,    35,    35,  -520,  -520,   476,
    -520,  -520,  -520,   477,  -520,  -520,   479,   468,   478,   483,
    -520,     2,   485,     2,  -520,   486,   487,  -520,    63,  -520,
     426,  -520,   488,  -520,   187,   187,  -520,  -520,    -3,  -520,
     111,   484,  -520,   489,   104,   432,  -520,    82,    96,  -520,
    -520,   160,  -520,  -520,     2,   490,   495,  -520,   492,  -520,
     491,   493,   494,   496,   440,    -3,  -520,  -520,   497,   -50,
     499,   500,   502,   504,     6,  -520,     2,   498,  -520,    47,
    -520,  -520,  -520,   505,   506,    27,   163,   161,  -520,  -520,
    -520,  -520,   507,   446,   501,   503,   508,  -520,   510,   509,
     432,   432,  -520,   511,   512,   513,   515,   517,   518,  -520,
    -520,  -520
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,     0,     0,     0,     2,     0,
       0,     0,     0,     0,     0,    10,     0,     3,     0,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    26,
       0,    11,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     5,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     9,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     7,
       8,     0,     0,     0,     0,     0,     0,     0,    31,    30,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,     0,     0,     0,     0,   148,     0,     0,     0,
       0,     0,     0,     0,     0,    79,     0,     0,     0,    42,
       0,     0,    80,     0,     0,     0,     0,    85,     0,     0,
      96,     0,     0,   136,     0,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   170,     0,     0,    28,     0,    39,     0,   152,
       0,   151,     0,   149,   150,    41,     0,     0,     0,     0,
       0,     0,     0,     0,   156,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,   135,     0,     0,     0,     0,
       0,    34,     0,    33,     0,     0,     0,     0,   174,     0,
     175,     0,     0,     0,    52,     0,    53,     0,     0,     0,
       0,   176,     0,   100,     0,     0,   171,     0,     0,    29,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,    48,     0,    86,     0,     0,    84,     0,
       0,     0,   137,     0,     0,     0,    27,    36,     0,    32,
     158,   160,   159,    43,    44,     0,     0,     0,     0,     0,
       0,     0,     0,    57,     0,     0,     0,     0,     0,    98,
       0,     0,     0,   139,   172,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   104,
       0,   102,   105,   106,     0,     0,     0,   141,     0,   140,
       0,    37,     0,     0,    45,     0,    82,     0,     0,     0,
       0,    47,   157,     0,     0,     0,    55,     0,     0,    97,
      99,     0,     0,     0,     0,   101,   103,    95,     0,     0,
       0,   143,   142,     0,     0,     0,     0,     0,     0,     0,
      50,    51,     0,     0,    67,     0,     0,     0,     0,     0,
       0,    89,     0,     0,     0,   110,     0,     0,     0,   147,
     146,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,    63,     0,     0,     0,     0,    58,     0,
       0,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   138,    35,    56,     0,     0,     0,   173,     0,
      62,     0,    59,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   114,   115,   116,     0,   112,     0,
     107,     0,     0,     0,     0,     0,     0,    73,    76,    70,
       0,     0,     0,    64,     0,     0,    60,     0,     0,     0,
      91,    88,     0,     0,     0,     0,     0,   113,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    74,    83,     0,
       0,    68,    61,     0,     0,     0,     0,     0,     0,     0,
     111,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,     0,     0,     0,     0,   154,     0,
      77,    78,     0,     0,     0,     0,     0,   117,   118,     0,
     108,   144,   145,     0,    72,    66,     0,     0,     0,     0,
     153,     0,     0,     0,   166,     0,     0,   126,     0,   125,
       0,   109,     0,    94,     0,     0,    87,   155,     0,   168,
       0,     0,   120,     0,     0,     0,    75,     0,     0,   164,
     165,     0,   161,   163,     0,     0,     0,   124,     0,   127,
       0,     0,     0,     0,     0,     0,   169,   121,     0,     0,
       0,     0,     0,     0,     0,   162,     0,     0,   131,     0,
     129,   122,   123,     0,     0,     0,     0,     0,   128,   130,
      92,    93,     0,     0,     0,     0,     0,   119,     0,     0,
       0,     0,   167,     0,     0,     0,     0,     0,     0,   134,
     132,   133
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -520,  -520,  -520,  -520,  -520,   464,  -520,  -520,   521,  -520,
    -520,   -67,  -417,  -520,  -520,  -520,  -520,  -520,  -520,   529,
    -520,  -107,  -520,   195,  -126,  -187,  -235,   143,  -248,  -520,
      49,  -336,   109,   234,  -435,  -434,  -231,  -520,  -520,   520,
    -520,  -520,  -520,  -520,  -520,   134,  -520,  -520,  -520,  -520,
    -520,   254,  -520,  -520,  -520,  -520,    37,  -520,  -520,    60,
      18,  -520,  -520,   -47,  -520,  -520,  -520,  -520,   257,   253,
    -520,  -148,  -433,  -520,  -244,  -519,  -520,     7,   -36,    43,
     -77,  -520,  -293,  -186,   -80
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    14,    15,    16,    22,    40,    41,    42,
     174,   194,    98,   212,    43,   268,    44,    45,    46,    47,
     105,   112,   252,   253,   137,   199,   276,   285,   345,   402,
     403,   378,   299,   300,   456,   457,   458,    48,    49,   108,
      50,    51,   138,   230,   380,   381,    52,   142,   288,   261,
     320,   321,   322,   323,   387,   447,   448,   417,   480,   504,
     548,   580,   599,   600,    53,   145,   170,   328,   329,   391,
      54,   559,   474,   519,   195,   183,   571,   572,   546,   560,
      99,   147,   549,   219,   220
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      93,    94,    95,   221,   100,   182,   472,   114,   115,   461,
     475,   339,   340,    97,   277,   179,   179,   234,   121,   436,
     123,   157,   487,   124,   159,   263,   437,   304,   438,   343,
     213,   188,   248,   189,   249,   305,    92,   404,   487,   570,
     385,   106,   192,    92,    92,   152,   135,    92,    92,   547,
     379,     1,    92,   389,   143,   597,    92,     3,   164,   319,
      92,    92,   165,   180,    92,    92,   570,   180,   180,    92,
     227,   435,   386,   289,     4,     8,   184,   533,   186,   187,
     374,   175,   598,     5,   107,   136,   410,   390,   375,   319,
     413,   414,   303,   144,   193,   193,   373,   113,   465,   338,
     623,   184,   368,   193,   544,    96,    96,   383,   264,    96,
     344,   218,   218,   522,    96,   250,   337,   120,    96,   567,
     568,   478,    96,    96,   278,   502,   218,   545,   350,   407,
     113,   439,   581,   487,   487,   172,   376,   211,    10,   377,
       6,   569,   239,   181,   181,    26,    27,    17,   254,   173,
     503,    28,   597,    29,    30,    31,    32,    33,   434,   578,
     483,   484,   455,   315,   316,   455,   315,   316,   569,   455,
     315,   316,   140,   267,    34,    35,    36,    23,   608,   598,
      37,   579,   218,   455,   315,   316,     9,   624,   625,   315,
     316,   374,   141,   325,   326,   563,   614,    38,   615,   375,
     564,   218,   317,   318,    13,   308,   486,   360,   361,   496,
     616,    26,    27,   582,    24,    72,    39,    28,   540,    29,
      30,    31,    32,    33,   325,   326,   444,   583,   445,   331,
     541,   208,   355,   209,   453,   454,   446,   218,   476,   327,
      34,    35,    36,   444,   574,   445,    37,   315,   316,   365,
     418,   197,   575,   446,   366,   198,   367,   419,   283,    18,
     317,   318,    19,    38,   284,    11,    12,    13,   102,   103,
     104,   242,   243,   244,   455,   315,   316,   280,   286,   281,
     287,    20,    39,   295,   313,   296,   314,   430,    25,   431,
     398,   584,    57,   585,   613,    58,   574,   110,   111,    55,
      56,    59,    60,    61,   482,    62,    63,    64,    65,    66,
      67,    76,    68,    69,    70,    71,    77,    79,    74,    78,
      75,    80,    81,    82,    83,    85,    86,    84,    88,    91,
      87,    89,    90,   506,    33,    92,   109,   113,   118,   116,
     117,   119,   122,   218,   218,   125,   126,   127,   128,   518,
     129,   107,   132,   131,   133,   134,   139,   148,   146,   149,
     151,   150,   462,   155,   154,   156,   158,   160,   167,   162,
     153,   163,   166,   161,   168,   184,   171,   169,   177,   178,
     176,   185,   479,   135,   190,   191,   196,   200,   201,   202,
     493,   203,   206,   557,   489,   205,   204,   207,   210,   215,
     229,   232,   251,   260,   184,   233,   265,   311,   605,   214,
     216,   217,   222,   275,   223,   235,   343,   400,   388,   225,
     184,   524,   298,   224,   523,   228,   586,   226,   338,   231,
     236,   237,   420,   301,   312,   240,   238,   464,   384,   401,
     245,   241,   246,   247,   255,   257,   394,   258,   262,   259,
     256,   266,   269,   270,   397,   271,   272,   273,   274,   279,
     412,   379,   290,   421,   184,   451,   184,   292,   490,   293,
     294,   297,   282,   291,   512,   364,   307,   399,    21,   309,
     463,   573,   324,   332,   477,   306,   333,   473,   334,   335,
     347,   310,   330,   336,   341,   366,   342,   184,   510,   346,
     348,   416,   349,   351,   352,   353,   354,   358,   573,   357,
     359,   302,   363,   369,   411,   370,   371,   218,   372,   184,
     393,   395,   396,   382,   426,   406,   405,   543,   408,   468,
     409,   415,   469,   422,   423,   537,   538,   542,   424,   425,
     505,   427,   428,   429,   550,   432,   440,   441,   433,   449,
     588,   442,   609,   450,   459,   443,   460,   452,   466,   467,
     470,    73,   471,   481,   485,   565,   511,   488,   491,   612,
     492,   494,   495,   497,   356,   180,   498,   499,   500,   501,
     507,   594,   508,   509,   513,   362,   514,   618,   525,   515,
     526,   516,   595,   520,   521,   517,   527,   528,   554,   529,
     530,   531,   532,   534,   535,   536,   539,   551,   555,   552,
     553,   101,   392,     0,   556,   558,   561,     0,   562,   566,
     577,   587,   589,     0,   576,   591,   130,   596,   607,   590,
     601,   602,   592,   603,   593,   604,   610,   611,   617,   606,
     619,   622,   620,   626,   627,   628,   629,   621,   630,   631
};

static const yytype_int16 yycheck[] =
{
      77,    78,    79,   189,    81,   153,   441,    87,    88,    25,
     443,   304,   305,    80,   249,    14,    14,   203,    95,    20,
      97,   128,   456,   100,   131,    55,    27,    46,    29,    55,
     178,   157,   219,   159,   221,   279,    63,   373,   472,   558,
      23,    33,    53,    63,    63,   122,    80,    63,    63,    14,
      95,    56,    63,    43,    77,   105,    63,   130,   138,   290,
      63,    63,   139,    66,    63,    63,   585,    66,    66,    63,
     196,   407,    55,   259,     0,   131,   153,   510,   155,   156,
      77,   148,   132,     0,    76,   119,   131,    77,    85,   320,
     383,   384,   278,   116,   114,   114,   344,   124,   434,    64,
     619,   178,   337,   114,    77,   125,   125,   351,   138,   125,
     136,   188,   189,   140,   125,   222,   303,   132,   125,   554,
     555,   128,   125,   125,   250,   104,   203,   100,   314,   377,
     124,   132,   565,   567,   568,   131,   133,   136,     0,   136,
      57,   558,   209,   142,   142,    61,    62,   131,   225,   145,
     129,    67,   105,    69,    70,    71,    72,    73,   406,    55,
     453,   454,    87,    88,    89,    87,    88,    89,   585,    87,
      88,    89,    77,   240,    90,    91,    92,   131,   131,   132,
      96,    77,   259,    87,    88,    89,   130,   620,   621,    88,
      89,    77,    97,   117,   118,   132,    35,   113,    37,    85,
     137,   278,   101,   102,    60,   282,   131,   131,   132,   131,
      49,    61,    62,   131,    50,   131,   132,    67,   131,    69,
      70,    71,    72,    73,   117,   118,    28,   131,    30,   296,
     143,   131,   131,   133,   420,   421,    38,   314,    23,   132,
      90,    91,    92,    28,   133,    30,    96,    88,    89,   132,
      28,    77,   141,    38,   137,    81,   139,    35,    77,   130,
     101,   102,   130,   113,    83,    58,    59,    60,    74,    75,
      76,     6,     7,     8,    87,    88,    89,   131,   137,   133,
     139,   130,   132,   131,   131,   133,   133,   131,    50,   133,
     367,   131,    18,   133,   131,     9,   133,    77,    78,   131,
     131,   130,   130,   130,   452,   130,   130,   130,   130,   130,
     130,    34,   130,   130,   130,   130,    47,    32,   132,    48,
     132,    24,    22,    30,    33,    49,    36,    40,    26,    11,
      42,   131,   131,   481,    73,    63,    73,   124,    55,   131,
     130,    68,   130,   420,   421,   131,   130,   130,   130,   497,
     131,    76,   131,   130,   130,   130,   130,   130,    64,   130,
     132,   131,   429,    21,   132,    20,   131,    33,    98,   130,
     139,   130,   130,   140,   130,   452,   130,   115,   131,   134,
     137,   131,   449,    80,    76,    55,    19,    15,   139,   130,
     467,   139,    13,   541,   461,   131,   130,    64,    55,   132,
      93,    55,    79,    55,   481,   137,    13,    55,   594,   130,
     130,   130,   130,    82,   130,   137,    55,    13,    23,   130,
     497,   501,    86,   131,   501,   130,   574,   131,    64,   131,
     131,   130,    26,    86,    94,   130,   132,     4,    44,    84,
     131,   135,   131,   131,   131,   130,   132,   131,   131,   130,
     140,   131,   131,   131,    55,   131,   131,   131,   131,   130,
      55,    95,   130,    42,   541,    35,   543,   130,    13,   131,
     131,   130,   144,   138,    13,   332,   281,   368,    14,   130,
     431,   558,   131,   130,   447,   141,   130,    55,   131,   130,
     130,   141,   138,   131,   131,   137,   131,   574,    39,   131,
     130,   103,   131,   130,   130,   130,   130,   130,   585,   131,
     130,   277,   131,   130,   380,   131,   131,   594,   130,   596,
     138,   131,   131,   137,   132,   136,   130,   140,   131,    94,
     130,   130,    94,   131,   131,    55,    55,    55,   131,   130,
     480,   131,   131,   130,   526,   131,   138,   130,   133,   130,
      55,   139,   599,   131,   131,   139,   131,   139,   131,   139,
     131,    40,   131,   139,   130,   139,   138,   131,   131,   605,
     131,   130,   130,   130,   320,    66,   132,   132,   131,   130,
     132,   141,   132,   132,   131,   328,   130,   141,   130,   137,
     130,   137,   585,   131,   131,   138,   131,   131,   130,   132,
     131,   131,   131,   131,   131,   131,   131,   131,   130,   132,
     131,    82,   359,    -1,   131,   130,   130,    -1,   131,   131,
     131,   131,   130,    -1,   140,   132,   106,   130,   130,   138,
     131,   131,   138,   131,   138,   131,   131,   131,   131,   596,
     139,   131,   139,   132,   132,   132,   131,   139,   131,   131
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    56,   147,   130,     0,     0,    57,   148,   131,   130,
       0,    58,    59,    60,   149,   150,   151,   131,   130,   130,
     130,   151,   152,   131,    50,    50,    61,    62,    67,    69,
      70,    71,    72,    73,    90,    91,    92,    96,   113,   132,
     153,   154,   155,   160,   162,   163,   164,   165,   183,   184,
     186,   187,   192,   210,   216,   131,   131,    18,     9,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   131,   154,   132,   132,    34,    47,    48,    32,
      24,    22,    30,    33,    40,    49,    36,    42,    26,   131,
     131,    11,    63,   226,   226,   226,   125,   157,   158,   226,
     226,   165,    74,    75,    76,   166,    33,    76,   185,    73,
      77,    78,   167,   124,   230,   230,   131,   130,    55,    68,
     132,   226,   130,   226,   226,   131,   130,   130,   130,   131,
     185,   130,   131,   130,   130,    80,   119,   170,   188,   130,
      77,    97,   193,    77,   116,   211,    64,   227,   130,   130,
     131,   132,   226,   139,   132,    21,    20,   167,   131,   167,
      33,   140,   130,   130,   230,   226,   130,    98,   130,   115,
     212,   130,   131,   145,   156,   157,   137,   131,   134,    14,
      66,   142,   217,   221,   226,   131,   226,   226,   170,   170,
      76,    55,    53,   114,   157,   220,    19,    77,    81,   171,
      15,   139,   130,   139,   130,   131,    13,    64,   131,   133,
      55,   136,   159,   217,   130,   132,   130,   130,   226,   229,
     230,   229,   130,   130,   131,   130,   131,   170,   130,    93,
     189,   131,    55,   137,   229,   137,   131,   130,   132,   157,
     130,   135,     6,     7,     8,   131,   131,   131,   171,   171,
     167,    79,   168,   169,   226,   131,   140,   130,   131,   130,
      55,   195,   131,    55,   138,    13,   131,   157,   161,   131,
     131,   131,   131,   131,   131,    82,   172,   172,   170,   130,
     131,   133,   144,    77,    83,   173,   137,   139,   194,   229,
     130,   138,   130,   131,   131,   131,   133,   130,    86,   178,
     179,    86,   179,   229,    46,   220,   141,   169,   226,   130,
     141,    55,    94,   131,   133,    88,    89,   101,   102,   182,
     196,   197,   198,   199,   131,   117,   118,   132,   213,   214,
     138,   157,   130,   130,   131,   130,   131,   171,    64,   228,
     228,   131,   131,    55,   136,   174,   131,   130,   130,   131,
     229,   130,   130,   130,   130,   131,   197,   131,   130,   130,
     131,   132,   214,   131,   173,   132,   137,   139,   172,   130,
     131,   131,   130,   174,    77,    85,   133,   136,   177,    95,
     190,   191,   137,   220,    44,    23,    55,   200,    23,    43,
      77,   215,   215,   138,   132,   131,   131,    55,   226,   178,
      13,    84,   175,   176,   177,   130,   136,   174,   131,   130,
     131,   191,    55,   228,   228,   130,   103,   203,    28,    35,
      26,    42,   131,   131,   131,   130,   132,   131,   131,   130,
     131,   133,   131,   133,   174,   177,    20,    27,    29,   132,
     138,   130,   139,   139,    28,    30,    38,   201,   202,   130,
     131,    35,   139,   229,   229,    87,   180,   181,   182,   131,
     131,    25,   157,   176,     4,   177,   131,   139,    94,    94,
     131,   131,   180,    55,   218,   218,    23,   202,   128,   157,
     204,   139,   217,   228,   228,   130,   131,   181,   131,   157,
      13,   131,   131,   226,   130,   130,   131,   130,   132,   132,
     131,   130,   104,   129,   205,   205,   217,   132,   132,   132,
      39,   138,    13,   131,   130,   137,   137,   138,   217,   219,
     131,   131,   140,   226,   230,   130,   130,   131,   131,   132,
     131,   131,   131,   218,   131,   131,   131,    55,    55,   131,
     131,   143,    55,   140,    77,   100,   224,    14,   206,   228,
     206,   131,   132,   131,   130,   130,   131,   217,   130,   217,
     225,   130,   131,   132,   137,   139,   131,   180,   180,   158,
     221,   222,   223,   226,   133,   141,   140,   131,    55,    77,
     207,   218,   131,   131,   131,   133,   217,   131,    55,   130,
     138,   132,   138,   138,   141,   223,   130,   105,   132,   208,
     209,   131,   131,   131,   131,   229,   225,   130,   131,   209,
     131,   131,   224,   131,    35,    37,    49,   131,   141,   139,
     139,   139,   131,   221,   218,   218,   132,   132,   132,   131,
     131,   131
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   146,   147,   147,   148,   148,   148,   149,   150,   151,
     152,   153,   153,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   155,   156,   156,
     157,   157,   158,   159,   159,   160,   161,   161,   162,   162,
     163,   164,   165,   166,   166,   166,   167,   167,   168,   168,
     169,   169,   170,   170,   171,   171,   172,   173,   173,   173,
     173,   173,   174,   175,   175,   176,   176,   177,   177,   178,
     178,   178,   179,   180,   180,   181,   181,   182,   182,   183,
     184,   184,   185,   186,   187,   188,   188,   189,   189,   190,
     190,   191,   191,   191,   191,   192,   193,   193,   194,   194,
     195,   195,   196,   196,   197,   197,   197,   198,   199,   199,
     200,   200,   201,   201,   202,   202,   202,   203,   203,   204,
     204,   204,   205,   205,   205,   206,   206,   207,   207,   208,
     208,   209,   209,   209,   209,   210,   211,   211,   212,   212,
     213,   213,   213,   213,   214,   214,   215,   215,   216,   217,
     217,   217,   217,   218,   219,   219,   220,   220,   221,   221,
     221,   222,   222,   223,   223,   223,   224,   224,   225,   225,
     226,   227,   227,   228,   229,   229,   230
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     4,     5,     4,     5,     5,     6,     6,     4,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,    10,     1,     3,
       1,     1,     7,     1,     1,    15,     1,     3,     6,     7,
       9,     7,     5,     7,     7,     9,     1,     9,     1,     3,
       5,     5,     4,     4,     1,     6,     6,     1,     5,     6,
       7,     8,     4,     1,     3,     5,     6,     1,     5,     4,
       6,     1,     9,     1,     2,     6,     1,     8,     8,     5,
       5,     6,     9,    17,     9,     1,     5,    13,     9,     1,
       2,     4,    13,    13,     8,    12,     1,     8,     1,     3,
       0,     4,     1,     2,     1,     1,     1,     5,     8,     9,
       1,     5,     1,     2,     1,     1,     1,     5,     5,    11,
       5,     7,     7,     7,     5,     1,     1,     1,     4,     1,
       2,     1,     7,     7,     7,     7,     1,     5,     9,     5,
       1,     1,     2,     2,     8,     8,     1,     1,     5,     1,
       1,     1,     1,     4,     1,     3,     1,     6,     4,     4,
       4,     1,     3,     1,     1,     1,     1,     9,     1,     3,
       4,     4,     6,     4,     1,     1,     5
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
      yyerror (p_ast, p_builder, p_sb, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, p_ast, p_builder, p_sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (p_ast);
  YYUSE (p_builder);
  YYUSE (p_sb);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, p_ast, p_builder, p_sb);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
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
                                              , p_ast, p_builder, p_sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, p_ast, p_builder, p_sb); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  YYUSE (yyvaluep);
  YYUSE (p_ast);
  YYUSE (p_builder);
  YYUSE (p_sb);
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
yyparse (AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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
      yychar = yylex (&yylval, p_sb);
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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 251 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = new AST ( (yyvsp[-1].tok) ); }
#line 1823 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 252 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = (yyvsp[-2].node); }
#line 1829 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 256 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 1835 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 257 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 1841 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 258 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); dynamic_cast < AST_Schema* > ( (yyval.node) ) -> SetVersion ( (yyvsp[-2].tok) -> GetValue () ); }
#line 1847 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 262 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-5].tok); }
#line 1853 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 266 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-2].tok); }
#line 1859 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 270 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Schema ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 1865 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 274 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Schema (); }
#line 1871 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 279 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 1877 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 280 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 1883 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 286 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1889 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 287 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1895 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 288 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1901 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 289 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1907 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 290 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1913 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 291 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1919 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1925 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 293 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1931 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 294 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1937 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 295 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1943 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 296 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1949 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1955 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 298 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1961 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 299 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 1967 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 304 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeDef ( (yyvsp[-9].tok), (yyvsp[-6].fqn), (yyvsp[-3].node) ); }
#line 1973 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 308 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 1979 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 309 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 1985 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 1991 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 314 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 1997 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 318 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
#line 2003 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 322 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2009 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2015 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 328 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeSet ( (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-5].node) ); }
#line 2021 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 332 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2027 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 333 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2033 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 337 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-5].tok), (yyvsp[-2].fqn), 0 ); }
#line 2039 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 338 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-6].tok), (yyvsp[-2].fqn), (yyvsp[-3].fqn) ); }
#line 2045 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 342 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . ConstDef ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2051 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 346 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . AliasDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2057 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 350 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2063 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 354 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . UntypedFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2069 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 355 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . RowlenFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2075 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 357 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), false, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].paramSig), (yyvsp[-2].paramSig), (yyvsp[-1].node) ); }
#line 2081 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2087 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 362 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2093 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 366 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2099 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2105 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 371 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].fqn) ); }
#line 2111 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].fqn) ); }
#line 2117 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 376 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2123 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 377 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2129 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 381 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[0].tok), 0, 0, false); }
#line 2135 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 382 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = (yyvsp[-2].paramSig); }
#line 2141 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 386 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = (yyvsp[-2].paramSig); }
#line 2147 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 390 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[0].tok), 0, 0, false); }
#line 2153 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 391 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-4].tok), (yyvsp[-2].node), 0, (yyvsp[-1].boolean) ); }
#line 2159 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 392 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-5].tok), 0, (yyvsp[-2].node), (yyvsp[-1].boolean) ); }
#line 2165 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 393 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].boolean) ); }
#line 2171 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 394 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-1].boolean) ); }
#line 2177 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 398 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2183 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 402 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2189 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 403 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2195 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 407 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Formal ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].tok), false ); }
#line 2201 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 408 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Formal ( (yyvsp[-5].tok), (yyvsp[-2].node), (yyvsp[-1].tok), true ); }
#line 2207 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 412 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.boolean) = false; }
#line 2213 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 413 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.boolean) = true; }
#line 2219 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 417 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2225 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 418 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].fqn); }
#line 2231 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 423 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2237 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 427 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2243 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 428 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2249 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 432 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-2].expr) ); }
#line 2255 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 433 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2261 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 437 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2267 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 438 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2273 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 442 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2279 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 445 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2285 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 446 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2291 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 451 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), true, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].paramSig), (yyvsp[-2].paramSig), (yyvsp[-1].node) ); }
#line 2297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-16].tok), false, (yyvsp[-8].node), (yyvsp[-7].node), (yyvsp[-6].fqn), (yyvsp[-5].paramSig), (yyvsp[-4].paramSig), (yyvsp[-3].node) ); }
#line 2303 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . PhysicalDecl ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].fqn), (yyvsp[-2].paramSig), (yyvsp[-1].node) ); }
#line 2309 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 469 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2315 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 470 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].node) ); }
#line 2321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 474 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-9].tok), (yyvsp[-4].node) ); }
#line 2327 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 476 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2333 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 480 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2339 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 481 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2345 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 486 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2351 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 488 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ) ; }
#line 2357 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 490 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ); }
#line 2363 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 492 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn) ); }
#line 2369 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 499 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TableDef ( (yyvsp[-11].tok), (yyvsp[-8].fqn), (yyvsp[-7].node), (yyvsp[-3].node) ); }
#line 2375 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 503 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2381 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 504 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2387 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 508 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2393 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 509 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2399 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 513 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2405 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 514 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2411 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 518 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2417 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 519 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2423 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 523 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2429 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 524 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2435 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 525 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2441 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 529 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2447 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 534 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-2].expr) ); }
#line 2453 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 536 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-2].expr) ); }
#line 2459 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 540 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 541 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2471 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 545 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2477 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 546 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2483 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 550 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2489 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 551 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2495 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 552 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2501 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 556 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2507 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 557 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2513 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 562 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-5].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2519 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 563 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2525 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 564 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
#line 2531 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 569 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node) ); }
#line 2537 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 571 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
#line 2543 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 573 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node) ); }
#line 2549 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 577 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2555 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 578 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2561 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 582 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2567 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 583 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2573 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 587 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2579 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 588 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2585 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 592 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2591 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 593 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2597 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 594 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2603 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 595 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2609 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 601 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . DatabaseDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2615 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 605 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2621 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 606 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].fqn); }
#line 2627 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 609 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2633 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 610 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }
#line 2639 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 614 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2645 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 615 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2651 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 616 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2657 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 617 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2663 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 622 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2669 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 624 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2675 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 628 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 629 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 635 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . Include ( (yyvsp[-4].tok), (yyvsp[-1].tok) ); }
#line 2693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 641 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2699 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 642 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].fqn) ); }
#line 2705 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 643 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( PT_AT ); }
#line 2711 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 644 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 2717 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 649 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2723 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 653 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].expr) ); }
#line 2729 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 654 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( (yyvsp[0].expr) ); }
#line 2735 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 658 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 659 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }
#line 2747 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 663 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 2753 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 664 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 2759 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 665 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 2765 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 676 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2771 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 677 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2777 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2783 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 682 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2789 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 683 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2795 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2801 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 688 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2807 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 2813 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 2819 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 699 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-1].fqn); }
#line 2825 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 703 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 2831 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 704 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 2837 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 708 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 2843 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 712 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 2849 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 713 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 2855 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 717 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }
#line 2861 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;


#line 2865 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
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
      yyerror (p_ast, p_builder, p_sb, YY_("syntax error"));
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
        yyerror (p_ast, p_builder, p_sb, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



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
                      yytoken, &yylval, p_ast, p_builder, p_sb);
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, p_ast, p_builder, p_sb);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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
  yyerror (p_ast, p_builder, p_sb, YY_("memory exhausted"));
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
                  yytoken, &yylval, p_ast, p_builder, p_sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, p_ast, p_builder, p_sb);
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
