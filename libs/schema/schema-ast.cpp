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
    static int NextToken ( YYSTYPE * p_token, ParseTreeScanner & p_sb )
    {
        return p_sb . NextToken ( p_token -> tok );
    }

    void AST_error ( void * p_parser, ASTBuilder & p_builder, ParseTreeScanner & p_sb, const char * p_msg )
    {
        p_builder . ReportInternalError ( p_msg, p_sb . GetSource () );
    }


#line 95 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:339  */

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
    VERS_2_0 = 274,
    KW___no_header = 275,
    KW___row_length = 276,
    KW___untyped = 277,
    KW_alias = 278,
    KW_column = 279,
    KW_const = 280,
    KW_control = 281,
    KW_database = 282,
    KW_decode = 283,
    KW_default = 284,
    KW_encode = 285,
    KW_extern = 286,
    KW_false = 287,
    KW_fmtdef = 288,
    KW_function = 289,
    KW_include = 290,
    KW_limit = 291,
    KW_physical = 292,
    KW_read = 293,
    KW_readonly = 294,
    KW_return = 295,
    KW_schema = 296,
    KW_static = 297,
    KW_table = 298,
    KW_template = 299,
    KW_trigger = 300,
    KW_true = 301,
    KW_type = 302,
    KW_typedef = 303,
    KW_typeset = 304,
    KW_validate = 305,
    KW_version = 306,
    KW_view = 307,
    KW_virtual = 308,
    KW_void = 309,
    KW_write = 310,
    PT_ASTLIST = 311,
    PT_PARSE = 312,
    PT_SOURCE = 313,
    PT_VERSION_1_0 = 314,
    PT_VERSION_2 = 315,
    PT_SCHEMA_1_0 = 316,
    PT_SCHEMA_2_0 = 317,
    PT_INCLUDE = 318,
    PT_TYPEDEF = 319,
    PT_FQN = 320,
    PT_IDENT = 321,
    PT_PHYSIDENT = 322,
    PT_UINT = 323,
    PT_TYPESET = 324,
    PT_TYPESETDEF = 325,
    PT_FORMAT = 326,
    PT_CONST = 327,
    PT_ALIAS = 328,
    PT_EXTERN = 329,
    PT_FUNCTION = 330,
    PT_UNTYPED = 331,
    PT_ROWLENGTH = 332,
    PT_FUNCDECL = 333,
    PT_EMPTY = 334,
    PT_SCHEMASIG = 335,
    PT_SCHEMAFORMAL = 336,
    PT_RETURNTYPE = 337,
    PT_FACTSIG = 338,
    PT_FUNCSIG = 339,
    PT_FUNCPARAMS = 340,
    PT_FORMALPARAM = 341,
    PT_ELLIPSIS = 342,
    PT_FUNCPROLOGUE = 343,
    PT_RETURN = 344,
    PT_PRODSTMT = 345,
    PT_PRODTRIGGER = 346,
    PT_SCHEMA = 347,
    PT_VALIDATE = 348,
    PT_PHYSICAL = 349,
    PT_PHYSPROLOGUE = 350,
    PT_PHYSSTMT = 351,
    PT_PHYSBODYSTMT = 352,
    PT_TABLE = 353,
    PT_TABLEPARENTS = 354,
    PT_TABLEBODY = 355,
    PT_FUNCEXPR = 356,
    PT_FACTPARMS = 357,
    PT_COLUMN = 358,
    PT_COLUMNEXPR = 359,
    PT_COLDECL = 360,
    PT_TYPEDCOL = 361,
    PT_COLSTMT = 362,
    PT_DFLTVIEW = 363,
    PT_PHYSMBR = 364,
    PT_PHYSCOL = 365,
    PT_PHYSCOLDEF = 366,
    PT_COLSCHEMAPARMS = 367,
    PT_COLSCHEMAPARAM = 368,
    PT_COLUNTYPED = 369,
    PT_DATABASE = 370,
    PT_TYPEEXPR = 371,
    PT_DBBODY = 372,
    PT_DBDAD = 373,
    PT_DBMEMBER = 374,
    PT_TBLMEMBER = 375,
    PT_NOHEADER = 376,
    PT_CASTEXPR = 377,
    PT_CONSTVECT = 378,
    PT_NEGATE = 379,
    PT_UNARYPLUS = 380,
    PT_VERSNAME = 381,
    PT_ARRAY = 382,
    PT_PHYSENCREF = 383,
    PT_TYPEDCOLEXPR = 384,
    PT_VIEW = 385,
    PT_VIEWPARAM = 386,
    PT_VIEWPARENTS = 387
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 57 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:355  */

  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;

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
#define YYLAST   1095

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  151
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  108
/* YYNRULES -- Number of rules.  */
#define YYNRULES  232
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  819

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   387

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
     134,   135,   140,   147,   137,   148,     2,   149,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   150,   136,
     144,   143,   145,     2,   133,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   138,     2,   139,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   141,   146,   142,     2,     2,     2,     2,
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
     125,   126,   127,   128,   129,   130,   131,   132
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   257,   257,   258,   262,   263,   264,   268,   272,   276,
     280,   285,   286,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   310,   315,
     316,   320,   321,   325,   329,   330,   334,   339,   340,   344,
     345,   349,   353,   357,   361,   362,   363,   368,   369,   373,
     374,   378,   379,   383,   384,   388,   389,   393,   397,   398,
     399,   400,   401,   405,   409,   410,   414,   415,   419,   420,
     424,   425,   426,   430,   434,   435,   439,   440,   444,   445,
     449,   452,   453,   457,   462,   471,   476,   477,   480,   482,
     487,   488,   492,   494,   496,   498,   505,   510,   511,   515,
     516,   520,   521,   525,   526,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   542,   546,   547,   551,   552,   556,
     557,   558,   562,   563,   567,   569,   570,   574,   575,   576,
     580,   581,   585,   586,   590,   591,   595,   596,   597,   598,
     602,   604,   606,   608,   613,   617,   618,   622,   623,   627,
     628,   632,   633,   638,   642,   643,   646,   647,   651,   652,
     653,   654,   658,   660,   665,   666,   672,   678,   682,   683,
     687,   688,   689,   690,   691,   692,   693,   694,   695,   696,
     697,   698,   702,   707,   708,   711,   712,   716,   717,   718,
     722,   723,   727,   731,   732,   736,   737,   741,   742,   746,
     747,   748,   752,   753,   757,   758,   762,   766,   767,   771,
     774,   778,   779,   785,   789,   790,   794,   798,   799,   803,
     808,   814,   815,   819,   823,   824,   828,   829,   830,   834,
     835,   839,   840
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
  "UNTERM_STRING", "UNTERM_ESCAPED_STRING", "VERS_1_0", "VERS_2_0",
  "KW___no_header", "KW___row_length", "KW___untyped", "KW_alias",
  "KW_column", "KW_const", "KW_control", "KW_database", "KW_decode",
  "KW_default", "KW_encode", "KW_extern", "KW_false", "KW_fmtdef",
  "KW_function", "KW_include", "KW_limit", "KW_physical", "KW_read",
  "KW_readonly", "KW_return", "KW_schema", "KW_static", "KW_table",
  "KW_template", "KW_trigger", "KW_true", "KW_type", "KW_typedef",
  "KW_typeset", "KW_validate", "KW_version", "KW_view", "KW_virtual",
  "KW_void", "KW_write", "PT_ASTLIST", "PT_PARSE", "PT_SOURCE",
  "PT_VERSION_1_0", "PT_VERSION_2", "PT_SCHEMA_1_0", "PT_SCHEMA_2_0",
  "PT_INCLUDE", "PT_TYPEDEF", "PT_FQN", "PT_IDENT", "PT_PHYSIDENT",
  "PT_UINT", "PT_TYPESET", "PT_TYPESETDEF", "PT_FORMAT", "PT_CONST",
  "PT_ALIAS", "PT_EXTERN", "PT_FUNCTION", "PT_UNTYPED", "PT_ROWLENGTH",
  "PT_FUNCDECL", "PT_EMPTY", "PT_SCHEMASIG", "PT_SCHEMAFORMAL",
  "PT_RETURNTYPE", "PT_FACTSIG", "PT_FUNCSIG", "PT_FUNCPARAMS",
  "PT_FORMALPARAM", "PT_ELLIPSIS", "PT_FUNCPROLOGUE", "PT_RETURN",
  "PT_PRODSTMT", "PT_PRODTRIGGER", "PT_SCHEMA", "PT_VALIDATE",
  "PT_PHYSICAL", "PT_PHYSPROLOGUE", "PT_PHYSSTMT", "PT_PHYSBODYSTMT",
  "PT_TABLE", "PT_TABLEPARENTS", "PT_TABLEBODY", "PT_FUNCEXPR",
  "PT_FACTPARMS", "PT_COLUMN", "PT_COLUMNEXPR", "PT_COLDECL",
  "PT_TYPEDCOL", "PT_COLSTMT", "PT_DFLTVIEW", "PT_PHYSMBR", "PT_PHYSCOL",
  "PT_PHYSCOLDEF", "PT_COLSCHEMAPARMS", "PT_COLSCHEMAPARAM",
  "PT_COLUNTYPED", "PT_DATABASE", "PT_TYPEEXPR", "PT_DBBODY", "PT_DBDAD",
  "PT_DBMEMBER", "PT_TBLMEMBER", "PT_NOHEADER", "PT_CASTEXPR",
  "PT_CONSTVECT", "PT_NEGATE", "PT_UNARYPLUS", "PT_VERSNAME", "PT_ARRAY",
  "PT_PHYSENCREF", "PT_TYPEDCOLEXPR", "PT_VIEW", "PT_VIEWPARAM",
  "PT_VIEWPARENTS", "'@'", "'('", "')'", "';'", "','", "'['", "']'", "'*'",
  "'{'", "'}'", "'='", "'<'", "'>'", "'|'", "'+'", "'-'", "'/'", "':'",
  "$accept", "parse", "source", "version_1", "version_2", "schema_1",
  "schema_2", "schema_decls", "schema_decl", "typedef", "new_type_names",
  "typespec", "arrayspec", "dim", "typeset", "typeset_spec", "fmtdef",
  "const", "alias", "function", "func_decl", "schema_sig_opt",
  "schema_formals", "schema_formal", "return_type", "fact_sig",
  "param_sig", "param_signature", "formals_list", "formals", "formal",
  "vararg", "prologue", "script_prologue", "script_stmts", "script_stmt",
  "production", "extern_function", "script", "script_decl", "validate",
  "physical", "phys_return_type", "phys_prologue", "phys_body",
  "phys_body_stmt", "table", "parents_opt", "tbl_parents", "tbl_body",
  "tbl_stmts", "tbl_stmt", "column_decl", "col_modifiers_opt",
  "col_modifiers", "col_modifier", "col_decl", "phys_enc_ref", "typed_col",
  "col_ident", "col_body_opt", "col_body", "col_stmt", "physmbr_decl",
  "phys_coldef", "col_schema_parms_opt", "col_schema_parms",
  "col_schema_parm", "col_schema_value", "database", "opt_dbdad", "dbbody",
  "db_members", "db_member", "template_opt", "include", "cond_expr",
  "cond_chain", "expr", "func_expr", "schema_parts_opt", "schema_parms",
  "schema_parm", "factory_parms_opt", "factory_parms_list",
  "factory_parms", "func_parms_opt", "expr_list", "uint_expr",
  "float_expr", "string_expr", "const_vect_expr", "bool_expr",
  "negate_expr", "cast_expr", "type_expr", "fqn", "qualnames", "ident",
  "fqn_opt_vers", "fqn_vers", "view", "view_parms", "view_parm",
  "view_body", "view_member", "opt_view_parents", "view_parents", YY_NULLPTR
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
     385,   386,   387,    64,    40,    41,    59,    44,    91,    93,
      42,   123,   125,    61,    60,    62,   124,    43,    45,    47,
      58
};
# endif

#define YYPACT_NINF -661

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-661)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -7,   -49,   160,    89,  -661,    -9,    42,   180,  -661,   280,
      98,   159,   165,   173,   255,   260,   196,  -661,   307,   308,
     279,   235,   252,   257,  -661,   379,   381,   267,   269,   274,
     278,   293,   296,   304,   306,   309,   310,   311,   312,   313,
     317,  -661,   193,  -661,  -661,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,  -661,   279,
    -661,   281,   290,   369,   366,   392,   409,   427,   430,   423,
     421,   417,   410,   424,   419,   436,   412,  -661,  -661,   263,
     331,   333,   458,   405,   405,   405,     9,   405,   396,   268,
      97,   397,   295,   347,   347,   347,  -661,  -661,  -661,   340,
     342,   422,   407,   -12,   345,   405,  -661,  -661,   405,   346,
     348,   349,   350,   351,   411,   357,   355,   358,  -661,   360,
      70,   361,    -3,   132,   341,  -661,   432,   362,   365,   367,
     364,   405,   363,   368,  -661,   479,   482,   295,  -661,   370,
     295,  -661,   473,   371,   374,   375,  -661,   347,   405,  -661,
     376,   413,  -661,   377,   395,   460,   383,   148,     9,   378,
    -661,   385,   384,   178,   386,   405,   405,   441,  -661,   441,
     446,   469,   -14,   506,   242,   512,   387,   394,   388,   399,
     400,   402,   516,  -661,   468,   212,  -661,   481,  -661,    32,
    -661,  -661,  -661,  -661,  -661,  -661,  -661,   404,   406,   408,
     414,   415,   416,  -661,   403,  -661,  -661,  -661,  -661,  -661,
    -661,  -661,  -661,  -661,  -661,   418,   420,    19,    19,   425,
     428,   431,   429,  -661,   433,   441,  -661,   435,   448,   437,
     485,   426,    19,   434,  -661,   439,   438,   440,   442,     9,
     443,  -661,   444,  -661,   356,   401,   445,   447,   398,   449,
     451,   452,   453,  -661,   242,  -661,   242,   295,   470,  -661,
     405,  -661,   454,   450,   456,   457,  -661,   459,   497,   462,
     -28,   461,   250,  -661,  -661,   542,   463,  -661,     9,   464,
     465,   466,   467,   502,    19,     3,   509,   178,   178,  -661,
     472,   474,   487,   487,   441,   471,   254,  -661,   455,  -661,
      83,   238,  -661,    19,   476,   475,  -661,   477,   478,    19,
     480,   439,   483,  -661,  -661,   270,  -661,  -661,  -661,  -661,
     486,   143,   484,   488,   491,   492,  -661,  -661,   489,   493,
     494,    19,     2,   490,   470,   405,  -661,   495,   496,   520,
     507,   276,  -661,   292,   498,   111,  -661,   518,   -22,  -661,
    -661,   500,     9,     5,  -661,   503,   504,  -661,   178,   178,
    -661,  -661,   505,   510,   499,  -661,   511,   501,   242,   518,
     518,   508,  -661,   514,   -19,   515,   513,   517,   519,    19,
     521,   522,   523,   524,   525,   526,  -661,  -661,   124,  -661,
    -661,   527,   529,   530,  -661,   153,  -661,   531,   532,  -661,
     534,   528,   535,  -661,  -661,   283,  -661,  -661,  -661,   533,
     195,   536,  -661,   287,    83,    96,  -661,   537,  -661,   487,
     538,   539,  -661,  -661,   541,   552,    72,  -661,   543,   540,
    -661,  -661,     3,   546,   197,   588,    49,   592,  -661,  -661,
    -661,    28,    28,   544,  -661,  -661,   602,  -661,   547,   560,
     548,   549,     5,   565,   545,  -661,   550,  -661,   553,   178,
     554,   556,   572,   405,   493,  -661,  -661,   566,    11,  -661,
     559,   555,   552,   562,   564,   126,  -661,   574,   518,   518,
    -661,   567,   561,   248,   563,    -1,   557,  -661,  -661,   604,
     589,   568,   569,    19,   571,   575,  -661,  -661,   573,   178,
     576,   577,  -661,   578,  -661,   580,   551,   581,   583,   288,
    -661,   584,   585,   552,    11,  -661,    17,   579,  -661,   586,
     582,   587,   275,   590,   591,   610,   593,   594,   596,   563,
     597,   405,    19,    19,  -661,  -661,  -661,   294,   113,  -661,
     178,   298,  -661,  -661,  -661,   277,   598,   599,     1,  -661,
     566,  -661,   649,    11,   600,   595,   603,   606,   605,   607,
     277,   620,   620,  -661,  -661,  -661,   266,  -661,    -5,  -661,
     601,   178,     6,  -661,   608,  -661,   611,   518,   518,  -661,
      19,   612,   613,  -661,    71,  -661,  -661,   299,  -661,   614,
      94,  -661,  -661,  -661,   615,     9,   648,  -661,   616,   617,
    -661,   405,   619,   621,  -661,  -661,   105,   622,   570,   618,
     623,  -661,   625,   152,   152,   178,   624,   626,   627,   666,
    -661,   628,   629,   630,  -661,     9,   658,   631,  -661,   632,
     178,   669,   633,  -661,  -661,   702,   634,  -661,  -661,   636,
     635,   637,   638,   178,   639,   644,  -661,   -33,   647,   650,
     651,   652,   646,   653,   704,   123,   176,   654,   656,   657,
     518,     9,   659,   660,  -661,   620,   661,   662,  -661,   663,
     671,   673,   664,    90,  -661,  -661,  -661,   683,   655,   143,
      73,    73,  -661,  -661,   665,  -661,   177,  -661,   667,    19,
     668,   620,   670,  -661,  -661,   640,   518,  -661,  -661,   672,
    -661,  -661,   674,   676,   677,   678,  -661,   178,   680,   681,
     682,  -661,   174,  -661,   642,  -661,   684,   620,   645,   143,
    -661,   679,  -661,   620,   675,   685,  -661,   277,   277,  -661,
    -661,     5,  -661,  -661,   686,   213,   620,  -661,   687,   693,
     689,   690,   691,   620,  -661,   109,   128,   302,  -661,   688,
    -661,   692,   694,   696,   695,  -661,  -661,   697,   699,   698,
     700,   701,   -32,   703,   706,  -661,    27,  -661,   708,   709,
     710,    19,   705,  -661,    43,  -661,  -661,  -661,   713,   -10,
    -661,  -661,  -661,  -661,  -661,   714,   715,   143,   244,  -661,
    -661,   405,   707,  -661,  -661,  -661,   716,   711,   712,   717,
     718,   721,  -661,   725,   620,   620,   265,  -661,   722,   723,
     726,   728,   729,   730,   731,  -661,  -661,  -661,  -661
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,     0,     0,     0,     2,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     0,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    27,     0,    11,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,     5,     0,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    10,     7,     8,     0,
       0,     0,     0,     0,     0,     0,    32,    31,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    47,     0,
       0,     0,     0,     0,     0,   166,     0,     0,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,    43,     0,
       0,    81,     0,     0,     0,     0,    86,     0,     0,    97,
       0,     0,   154,     0,     0,     0,     0,     0,     0,     0,
      39,     0,     0,     0,     0,     0,     0,     0,    82,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   213,     0,     0,    29,     0,    40,     0,
     202,   203,   204,   205,   171,   208,   207,     0,     0,     0,
       0,     0,     0,   172,     0,   173,   174,   175,   176,   177,
     178,   179,   181,   170,    42,     0,     0,     0,     0,     0,
       0,     0,     0,   211,     0,     0,    55,     0,     0,     0,
       0,     0,     0,     0,   153,     0,     0,     0,     0,     0,
       0,    35,     0,    34,     0,   183,     0,     0,     0,     0,
       0,     0,     0,   217,     0,   218,     0,     0,     0,    53,
       0,    54,     0,     0,     0,     0,   219,     0,   101,     0,
       0,     0,     0,   221,   214,     0,     0,    30,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,    49,     0,    87,
       0,     0,    85,     0,     0,     0,   155,     0,     0,     0,
       0,     0,     0,    28,    37,     0,    33,   199,   201,   200,
       0,     0,     0,     0,     0,     0,    44,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    58,     0,     0,     0,
       0,     0,    99,     0,     0,     0,   157,     0,     0,   222,
     215,     0,     0,     0,   190,     0,     0,   191,     0,     0,
     209,   180,     0,     0,     0,    72,     0,     0,     0,     0,
       0,     0,    50,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   113,   105,     0,   103,
     106,     0,     0,     0,   159,     0,   158,     0,     0,   229,
       0,     0,     0,    38,   188,     0,   185,   189,   187,     0,
       0,     0,   197,     0,     0,     0,    46,     0,    83,     0,
       0,     0,    48,   212,     0,     0,     0,    56,     0,     0,
      98,   100,     0,     0,     0,     0,     0,     0,   102,   104,
      96,     0,     0,     0,   161,   160,     0,   223,     0,     0,
       0,     0,     0,     0,     0,   195,     0,   210,     0,     0,
       0,     0,     0,     0,     0,    51,    52,     0,     0,    68,
       0,     0,     0,     0,     0,     0,    90,     0,     0,     0,
     115,     0,     0,     0,     0,     0,     0,   165,   164,     0,
       0,     0,     0,     0,     0,     0,   184,   186,     0,     0,
       0,     0,   198,     0,    70,     0,     0,     0,     0,     0,
      64,     0,     0,     0,     0,    59,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   156,   216,   231,     0,     0,    36,
       0,     0,   182,   206,    57,     0,     0,     0,     0,    63,
       0,    60,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   119,   120,   121,     0,   117,     0,   114,
       0,     0,     0,   110,     0,   109,     0,     0,     0,   230,
       0,     0,     0,   228,     0,   224,   193,     0,   196,     0,
       0,    74,    77,    71,     0,     0,     0,    65,     0,     0,
      61,     0,     0,     0,    92,    89,     0,     0,     0,     0,
       0,   118,     0,     0,     0,     0,     0,     0,     0,     0,
     111,     0,     0,     0,   232,     0,     0,     0,   225,     0,
       0,     0,     0,    75,    84,     0,     0,    69,    62,     0,
       0,     0,     0,     0,     0,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   194,     0,     0,     0,    66,     0,
       0,     0,     0,     0,   168,    78,    79,     0,     0,     0,
       0,     0,   122,   123,     0,   107,     0,   145,     0,     0,
       0,     0,     0,   162,   163,     0,     0,   220,   192,     0,
      73,    67,     0,     0,     0,     0,   167,     0,     0,     0,
       0,   131,     0,   130,     0,   108,     0,     0,     0,     0,
     140,     0,   112,     0,     0,     0,    95,     0,     0,    88,
     169,     0,   126,   125,     0,     0,     0,   141,     0,     0,
       0,     0,     0,     0,    76,     0,     0,     0,   129,     0,
     132,     0,     0,     0,     0,   144,   142,     0,     0,     0,
       0,     0,     0,     0,     0,   143,     0,   226,     0,     0,
       0,     0,     0,   136,     0,   134,   127,   128,     0,     0,
     147,   150,   152,   151,   227,     0,     0,     0,     0,   133,
     135,     0,     0,   148,    93,    94,     0,     0,     0,     0,
       0,     0,   124,     0,     0,     0,     0,   146,     0,     0,
       0,     0,     0,     0,     0,   149,   139,   137,   138
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -661,  -661,  -661,  -661,  -661,   727,  -661,   743,    20,  -661,
    -661,   -81,  -343,  -661,  -661,  -661,  -661,  -661,  -661,   719,
    -661,  -111,  -661,   558,  -146,  -202,  -275,   353,  -401,  -661,
     214,  -433,   372,   609,  -546,  -575,  -287,  -661,  -661,   734,
    -661,  -661,  -661,  -661,  -661,   297,  -661,  -661,  -661,  -661,
    -661,   380,  -661,  -661,  -661,   205,  -661,  -661,   190,    81,
    -661,  -661,    38,  -316,   189,  -661,  -661,    47,    22,  -661,
    -661,  -661,  -661,   641,   391,  -661,  -550,  -661,  -150,  -661,
    -661,   106,   643,  -660,   175,  -661,  -661,   373,  -347,  -661,
    -661,  -661,  -661,  -661,  -661,  -269,   -83,  -661,  -336,  -215,
     -86,  -661,  -661,   720,  -661,   273,  -661,  -661
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    14,    15,    16,    23,    42,    43,    44,
     185,   223,   106,   242,    45,   315,    46,    47,    48,    49,
     113,   120,   296,   297,   146,   228,   329,   338,   426,   509,
     510,   473,   364,   365,   590,   591,   592,    50,    51,   116,
      52,    53,   147,   265,   475,   476,    54,   151,   341,   305,
     388,   389,   390,   482,   566,   567,   524,   614,   650,   712,
     751,   774,   775,   528,   619,   689,   779,   780,   781,    55,
     154,   180,   395,   396,   489,    56,   608,   673,   412,   205,
     284,   405,   406,   356,   357,   587,   456,   413,   206,   207,
     208,   209,   210,   211,   212,   224,   107,   157,   713,   254,
     255,    57,   272,   273,   584,   585,   401,   537
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
     101,   102,   103,   256,   108,   105,   407,   122,   123,   124,
     404,   398,   609,   204,   606,   633,   322,   269,   330,   710,
     130,   217,   132,   218,   468,   133,   167,   595,   307,   169,
     617,   633,   100,   420,   421,   511,   529,   424,   555,   243,
     221,   190,   191,   192,   193,   556,   194,   557,   162,   369,
       1,   100,   292,   100,   293,   100,   387,   399,   197,   740,
     100,   174,    78,   370,   195,   175,   100,   100,   100,   321,
     100,   514,   487,   197,   100,   772,   149,   186,   196,   262,
     213,   554,   215,   216,   100,     3,   484,   711,   342,     5,
     469,   485,   100,   121,   347,   197,   150,   100,   470,    78,
     197,   387,   222,   778,   773,   407,   213,   488,   527,   404,
     400,   677,   553,   104,   308,   699,   368,   618,   222,   222,
     599,   425,   104,   612,   129,   792,     8,   796,   104,   104,
     104,   114,   104,   198,   253,   253,   104,   324,   325,   397,
     778,   721,   520,   521,   464,   121,   294,     6,   331,   253,
     772,   469,   144,   558,   199,   200,   201,   202,   277,   470,
       4,   581,   336,   478,   431,   203,   419,   738,   337,   530,
     633,   633,   241,   742,   582,   115,     9,   298,   789,   773,
      10,   745,   746,   589,   380,   381,   752,   190,   191,   192,
     193,   145,   194,   758,   589,   380,   381,   314,   589,   380,
     381,   253,   687,   581,   213,   213,   627,   583,   411,   471,
     195,   152,   472,   574,   380,   381,   582,   589,   380,   381,
     253,   480,   354,   474,   196,   706,   253,   382,   383,   632,
     392,   393,   461,    17,   384,   688,   707,   462,   385,   463,
     642,   622,   623,   100,   759,   355,   197,   394,   253,   583,
     153,   454,   373,   481,   809,   810,    27,    28,   648,   438,
     386,   517,    29,   760,    30,    31,    32,    33,    34,   749,
     408,   403,   392,   393,   455,   213,   213,   525,   536,   198,
     797,   649,   798,   183,   526,    35,    36,    37,   443,   444,
     610,    38,   750,    18,   799,   563,   253,   564,   184,    19,
     199,   200,   201,   202,   563,   565,   564,    20,    39,   502,
     734,   203,   690,   716,   565,   735,    13,   577,   578,   691,
     717,   226,    22,    40,   695,   227,    27,    28,    77,    41,
     100,    24,    29,   197,    30,    31,    32,    33,    34,    11,
      12,    13,    27,    28,   110,   111,   112,   238,    29,   239,
      30,    31,    32,    33,    34,    35,    36,    37,    25,    26,
     724,    38,   280,   281,   282,   624,   589,   380,   381,   408,
      58,    35,    36,    37,   118,   119,   213,    38,    39,   339,
     506,   340,   380,   381,   407,   310,    59,   311,   404,   333,
     586,   334,    60,    40,    39,   382,   383,    61,    96,    41,
      62,    63,   384,    64,    82,   351,   385,   352,    65,    40,
     253,   378,    66,   379,    83,    41,   213,    80,   451,   782,
     452,   616,   458,   549,   459,   550,    81,    67,   386,   579,
      68,   580,   782,   588,   629,   459,   630,   761,    69,   452,
      70,    84,    85,    71,    72,    73,    74,    75,   576,   253,
     253,    76,    86,    87,    88,    89,   808,   213,    90,   782,
      91,    92,    93,    94,    95,   652,    97,   596,    98,    99,
     100,    34,   117,   121,   719,   125,   126,   128,   127,   131,
     664,   134,   135,   136,   137,   155,   138,   613,   213,   115,
     141,   140,   142,   674,   143,   148,   158,   253,   156,   159,
     161,   165,   160,   166,   164,   168,   163,   170,   172,   173,
     176,   178,   179,   177,   635,   171,   181,   182,   639,   187,
     188,   214,   189,   144,   219,   220,   225,   229,   231,   236,
     230,   232,   213,   233,   237,   234,   235,   240,   244,   250,
     245,   267,   246,   264,   660,   283,   287,   213,   247,   248,
     249,   295,   251,   304,   252,   312,   787,   730,   320,   257,
     213,   679,   258,   260,   678,   323,   259,   268,   261,   263,
     271,   328,   266,   274,   275,   270,   376,   278,   276,   285,
     696,   363,   366,   279,   397,   286,   289,   290,   291,   299,
     301,   479,   302,   303,   300,   309,   288,   306,   313,   316,
     317,   318,   319,   377,   335,   332,   253,   326,   424,   327,
     343,   345,   483,   346,   486,   492,   494,   344,   350,   358,
     353,   498,   359,   362,   213,   348,   360,   361,   505,   374,
     519,   532,   533,   391,   416,   371,   418,   409,   410,   414,
     474,   375,   402,   422,   415,   417,   570,   428,   408,   423,
     427,   429,   508,   598,   430,   432,   433,   434,   435,   436,
     437,   636,   440,   441,   442,   446,   523,   447,   448,   449,
     450,   457,   527,   465,   466,   467,   607,   453,   462,   499,
     656,   477,   661,   783,   495,   500,   491,   546,   253,   503,
     493,   504,   501,   512,   496,   513,   783,   515,   516,   602,
     531,   522,   603,   534,   535,   538,   644,   540,   800,   665,
     539,   542,   543,   544,   545,   667,   547,   548,   686,   551,
     560,   559,   552,   783,   568,   561,   569,   703,   572,   704,
     562,   573,   575,   593,   594,   600,   571,   618,   601,   708,
     604,    21,   605,   620,   615,   621,   625,   626,   631,   754,
     634,   637,   638,   640,   645,   641,   643,   355,   646,   647,
     653,   655,   714,   657,   597,   658,   659,   460,   439,   668,
     669,   611,   518,   662,   675,   666,   670,   663,   671,   676,
     672,   680,   684,   723,   681,   736,   682,   683,   685,   739,
     692,   693,   694,   197,   697,   698,   700,   701,   702,   705,
     715,   718,    79,   720,   651,   722,   654,   109,   725,   726,
     727,   728,   790,   729,   731,   741,   732,   733,   743,   737,
     744,   748,   762,   753,   755,   756,   793,   757,   811,   766,
     764,   765,   767,   490,   763,   768,   507,   747,   776,   788,
     769,   777,   770,   784,   785,   786,   771,   791,   139,   794,
     795,   802,   801,   709,   803,   804,   807,   628,   812,   813,
     805,   806,   814,   815,   816,   817,   818,     0,     0,     0,
       0,     0,   541,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   367,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   349,     0,     0,     0,     0,   445,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   497
};

static const yytype_int16 yycheck[] =
{
      83,    84,    85,   218,    87,    86,   353,    93,    94,    95,
     353,   347,   562,   163,   560,   590,   285,   232,   293,   679,
     103,   167,   105,   169,   425,   108,   137,    26,    56,   140,
      24,   606,    65,   369,   370,   468,    37,    56,    21,   189,
      54,     9,    10,    11,    12,    28,    14,    30,   131,    47,
      57,    65,   254,    65,   256,    65,   343,    79,    68,   719,
      65,   147,    42,   332,    32,   148,    65,    65,    65,   284,
      65,   472,    44,    68,    65,   107,    79,   158,    46,   225,
     163,   514,   165,   166,    65,   134,    37,    14,   303,     0,
      79,    42,    65,   126,   309,    68,    99,    65,    87,    79,
      68,   388,   116,   113,   136,   452,   189,    79,   109,   452,
     132,   144,   513,   127,   142,   665,   331,   111,   116,   116,
     553,   140,   127,   128,   136,   135,   135,   787,   127,   127,
     127,    34,   127,   101,   217,   218,   127,   287,   288,    66,
     113,   691,   478,   479,   419,   126,   257,    58,   294,   232,
     107,    79,    82,   136,   122,   123,   124,   125,   239,    87,
       0,    90,    79,   432,   379,   133,   368,   717,    85,   485,
     745,   746,   140,   723,   103,    78,   134,   260,   135,   136,
       0,   727,   728,    89,    90,    91,   736,     9,    10,    11,
      12,   121,    14,   743,    89,    90,    91,   278,    89,    90,
      91,   284,    79,    90,   287,   288,   135,   136,   358,   137,
      32,    79,   140,   529,    90,    91,   103,    89,    90,    91,
     303,    24,    79,    97,    46,   135,   309,   103,   104,   135,
     119,   120,   136,   135,   110,   112,   146,   141,   114,   143,
     135,   577,   578,    65,   135,   102,    68,   136,   331,   136,
     118,    56,   335,    56,   804,   805,    63,    64,   106,   135,
     136,   135,    69,   135,    71,    72,    73,    74,    75,    56,
     353,   352,   119,   120,    79,   358,   359,    29,   493,   101,
      36,   129,    38,   135,    36,    92,    93,    94,   135,   136,
      24,    98,    79,   134,    50,    29,   379,    31,   150,   134,
     122,   123,   124,   125,    29,    39,    31,   134,   115,   459,
     136,   133,   136,   136,    39,   141,    61,   532,   533,   143,
     143,    79,    62,   130,   660,    83,    63,    64,   135,   136,
      65,   135,    69,    68,    71,    72,    73,    74,    75,    59,
      60,    61,    63,    64,    76,    77,    78,   135,    69,   137,
      71,    72,    73,    74,    75,    92,    93,    94,    51,    51,
     696,    98,     6,     7,     8,   580,    89,    90,    91,   452,
     135,    92,    93,    94,    79,    80,   459,    98,   115,   141,
     463,   143,    90,    91,   731,   135,   134,   137,   731,   135,
     540,   137,   135,   130,   115,   103,   104,    18,   135,   136,
      19,   134,   110,   134,    35,   135,   114,   137,   134,   130,
     493,   135,   134,   137,    48,   136,   499,   136,   135,   766,
     137,   571,   135,   135,   137,   137,   136,   134,   136,   135,
     134,   137,   779,   135,   135,   137,   137,   135,   134,   137,
     134,    49,    33,   134,   134,   134,   134,   134,   531,   532,
     533,   134,    25,    23,    31,    34,   803,   540,    41,   806,
      50,    37,    43,    27,    52,   615,   135,   548,   135,    11,
      65,    75,    75,   126,   689,   135,   134,    70,    56,   134,
     630,   135,   134,   134,   134,   144,   135,   568,   571,    78,
     135,   134,   134,   643,   134,   134,   134,   580,    66,   134,
     136,    22,   135,    21,   136,   135,   143,    34,   134,   134,
     134,   134,   117,   100,   595,   144,    56,   134,   601,   141,
     135,   135,   138,    82,    78,    56,    20,    15,   134,    13,
     143,   143,   615,   134,    66,   135,   134,    56,   134,   136,
     134,    56,   134,    95,   625,   144,   148,   630,   134,   134,
     134,    81,   134,    56,   134,    13,   771,   707,    56,   134,
     643,   647,   134,   134,   647,    56,   135,   141,   135,   134,
     131,    84,   135,   135,   134,   141,    56,   134,   136,   134,
     661,    88,    88,   139,    66,   138,   135,   135,   135,   135,
     134,    45,   135,   134,   144,   134,   147,   135,   135,   135,
     135,   135,   135,    96,   149,   134,   689,   135,    56,   135,
     134,   134,    24,   135,    22,    13,    56,   142,   135,   135,
     134,    56,   134,   134,   707,   145,   135,   135,    56,   134,
      56,    27,    43,   135,   135,   145,   135,   134,   134,   134,
      97,   145,   142,   135,   134,   134,    36,   134,   731,   135,
     135,   134,    86,     4,   135,   134,   134,   134,   134,   134,
     134,    13,   135,   134,   134,   134,   105,   135,   134,   141,
     135,   135,   109,   135,   135,   134,    56,   144,   141,   134,
      14,   141,    24,   766,   136,   135,   142,   136,   771,   135,
     143,   135,   139,   134,   145,   140,   779,   135,   134,    96,
     143,   134,    96,   135,   135,   134,   136,   134,   791,    40,
     135,   135,   135,   135,   134,    13,   135,   134,    14,   135,
     134,   142,   137,   806,   134,   143,   135,    56,   134,    56,
     143,   135,   135,   135,   135,   135,   143,   111,   143,    56,
     135,    14,   135,   135,   143,   134,   134,   134,   134,    56,
     135,   135,   135,   134,   136,   134,   134,   102,   135,   134,
     136,   134,   681,   135,   550,   136,   136,   414,   388,   135,
     134,   566,   475,   142,   135,   142,   141,   145,   141,   135,
     142,   134,   136,   143,   134,   143,   135,   135,   135,   144,
     136,   135,   135,    68,   135,   135,   135,   135,   135,   135,
     135,   134,    59,   135,   614,   135,   617,    88,   136,   135,
     134,   134,   774,   135,   134,   136,   135,   135,   143,   135,
     135,   135,   134,   136,   135,   135,   779,   136,   806,   134,
     136,   135,   135,   442,   142,   136,   464,   731,   135,   134,
     142,   135,   142,   135,   135,   135,   145,   134,   114,   135,
     135,   135,   145,   678,   143,   143,   135,   584,   136,   136,
     143,   143,   136,   135,   135,   135,   135,    -1,    -1,    -1,
      -1,    -1,   499,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   334,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   330,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   311,    -1,    -1,    -1,    -1,   395,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   452
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    57,   152,   134,     0,     0,    58,   153,   135,   134,
       0,    59,    60,    61,   154,   155,   156,   135,   134,   134,
     134,   156,    62,   157,   135,    51,    51,    63,    64,    69,
      71,    72,    73,    74,    75,    92,    93,    94,    98,   115,
     130,   136,   158,   159,   160,   165,   167,   168,   169,   170,
     188,   189,   191,   192,   197,   220,   226,   252,   135,   134,
     135,    18,    19,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   135,   159,   158,
     136,   136,    35,    48,    49,    33,    25,    23,    31,    34,
      41,    50,    37,    43,    27,    52,   135,   135,   135,    11,
      65,   247,   247,   247,   127,   162,   163,   247,   247,   170,
      76,    77,    78,   171,    34,    78,   190,    75,    79,    80,
     172,   126,   251,   251,   251,   135,   134,    56,    70,   136,
     247,   134,   247,   247,   135,   134,   134,   134,   135,   190,
     134,   135,   134,   134,    82,   121,   175,   193,   134,    79,
      99,   198,    79,   118,   221,   144,    66,   248,   134,   134,
     135,   136,   247,   143,   136,    22,    21,   172,   135,   172,
      34,   144,   134,   134,   251,   247,   134,   100,   134,   117,
     222,    56,   134,   135,   150,   161,   162,   141,   135,   138,
       9,    10,    11,    12,    14,    32,    46,    68,   101,   122,
     123,   124,   125,   133,   229,   230,   239,   240,   241,   242,
     243,   244,   245,   247,   135,   247,   247,   175,   175,    78,
      56,    54,   116,   162,   246,    20,    79,    83,   176,    15,
     143,   134,   143,   134,   135,   134,    13,    66,   135,   137,
      56,   140,   164,   229,   134,   134,   134,   134,   134,   134,
     136,   134,   134,   247,   250,   251,   250,   134,   134,   135,
     134,   135,   175,   134,    95,   194,   135,    56,   141,   250,
     141,   131,   253,   254,   135,   134,   136,   162,   134,   139,
       6,     7,     8,   144,   231,   134,   138,   148,   147,   135,
     135,   135,   176,   176,   172,    81,   173,   174,   247,   135,
     144,   134,   135,   134,    56,   200,   135,    56,   142,   134,
     135,   137,    13,   135,   162,   166,   135,   135,   135,   135,
      56,   250,   246,    56,   229,   229,   135,   135,    84,   177,
     177,   175,   134,   135,   137,   149,    79,    85,   178,   141,
     143,   199,   250,   134,   142,   134,   135,   250,   145,   254,
     135,   135,   137,   134,    79,   102,   234,   235,   135,   134,
     135,   135,   134,    88,   183,   184,    88,   184,   250,    47,
     246,   145,   174,   247,   134,   145,    56,    96,   135,   137,
      90,    91,   103,   104,   110,   114,   136,   187,   201,   202,
     203,   135,   119,   120,   136,   223,   224,    66,   249,    79,
     132,   257,   142,   162,   163,   232,   233,   239,   247,   134,
     134,   229,   229,   238,   134,   134,   135,   134,   135,   176,
     249,   249,   135,   135,    56,   140,   179,   135,   134,   134,
     135,   250,   134,   134,   134,   134,   134,   134,   135,   202,
     135,   134,   134,   135,   136,   224,   134,   135,   134,   141,
     135,   135,   137,   144,    56,    79,   237,   135,   135,   137,
     178,   136,   141,   143,   177,   135,   135,   134,   179,    79,
      87,   137,   140,   182,    97,   195,   196,   141,   246,    45,
      24,    56,   204,    24,    37,    42,    22,    44,    79,   225,
     225,   142,    13,   143,    56,   136,   145,   233,    56,   134,
     135,   139,   229,   135,   135,    56,   247,   183,    86,   180,
     181,   182,   134,   140,   179,   135,   134,   135,   196,    56,
     249,   249,   134,   105,   207,    29,    36,   109,   214,    37,
     214,   143,    27,    43,   135,   135,   250,   258,   134,   135,
     134,   238,   135,   135,   135,   134,   136,   135,   134,   135,
     137,   135,   137,   179,   182,    21,    28,    30,   136,   142,
     134,   143,   143,    29,    31,    39,   205,   206,   134,   135,
      36,   143,   134,   135,   214,   135,   247,   250,   250,   135,
     137,    90,   103,   136,   255,   256,   229,   236,   135,    89,
     185,   186,   187,   135,   135,    26,   162,   181,     4,   182,
     135,   143,    96,    96,   135,   135,   185,    56,   227,   227,
      24,   206,   128,   162,   208,   143,   229,    24,   111,   215,
     135,   134,   249,   249,   250,   134,   134,   135,   256,   135,
     137,   134,   135,   186,   135,   162,    13,   135,   135,   247,
     134,   134,   135,   134,   136,   136,   135,   134,   106,   129,
     209,   209,   229,   136,   215,   134,    14,   135,   136,   136,
     162,    24,   142,   145,   229,    40,   142,    13,   135,   134,
     141,   141,   142,   228,   229,   135,   135,   144,   247,   251,
     134,   134,   135,   135,   136,   135,    14,    79,   112,   216,
     136,   143,   136,   135,   135,   249,   162,   135,   135,   227,
     135,   135,   135,    56,    56,   135,   135,   146,    56,   235,
     234,    14,   210,   249,   210,   135,   136,   143,   134,   250,
     135,   227,   135,   143,   249,   136,   135,   134,   134,   135,
     229,   134,   135,   135,   136,   141,   143,   135,   227,   144,
     234,   136,   227,   143,   135,   185,   185,   232,   135,    56,
      79,   211,   227,   136,    56,   135,   135,   136,   227,   135,
     135,   135,   134,   142,   136,   135,   134,   135,   136,   142,
     142,   145,   107,   136,   212,   213,   135,   135,   113,   217,
     218,   219,   239,   247,   135,   135,   135,   250,   134,   135,
     213,   134,   135,   218,   135,   135,   234,    36,    38,    50,
     247,   145,   135,   143,   143,   143,   143,   135,   239,   227,
     227,   219,   136,   136,   136,   135,   135,   135,   135
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   151,   152,   152,   153,   153,   153,   154,   155,   156,
     157,   158,   158,   159,   159,   159,   159,   159,   159,   159,
     159,   159,   159,   159,   159,   159,   159,   159,   160,   161,
     161,   162,   162,   163,   164,   164,   165,   166,   166,   167,
     167,   168,   169,   170,   171,   171,   171,   172,   172,   173,
     173,   174,   174,   175,   175,   176,   176,   177,   178,   178,
     178,   178,   178,   179,   180,   180,   181,   181,   182,   182,
     183,   183,   183,   184,   185,   185,   186,   186,   187,   187,
     188,   189,   189,   190,   191,   192,   193,   193,   194,   194,
     195,   195,   196,   196,   196,   196,   197,   198,   198,   199,
     199,   200,   200,   201,   201,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   203,   204,   204,   205,   205,   206,
     206,   206,   207,   207,   208,   208,   208,   209,   209,   209,
     210,   210,   211,   211,   212,   212,   213,   213,   213,   213,
     214,   214,   214,   214,   215,   216,   216,   217,   217,   218,
     218,   219,   219,   220,   221,   221,   222,   222,   223,   223,
     223,   223,   224,   224,   225,   225,   226,   227,   228,   228,
     229,   229,   229,   229,   229,   229,   229,   229,   229,   229,
     229,   229,   230,   231,   231,   232,   232,   233,   233,   233,
     234,   234,   235,   236,   236,   237,   237,   238,   238,   239,
     239,   239,   240,   240,   241,   241,   242,   243,   243,   244,
     245,   246,   246,   247,   248,   248,   249,   250,   250,   251,
     252,   253,   253,   254,   255,   255,   256,   256,   256,   257,
     257,   258,   258
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     4,     5,     4,     5,     5,     6,     6,     4,
       4,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,    10,     1,
       3,     1,     1,     7,     1,     1,    15,     1,     3,     6,
       7,     9,     7,     5,     7,     7,     9,     1,     9,     1,
       3,     5,     5,     4,     4,     1,     6,     6,     1,     5,
       6,     7,     8,     4,     1,     3,     5,     6,     1,     5,
       4,     6,     1,     9,     1,     2,     6,     1,     8,     8,
       5,     5,     6,     9,    17,     9,     1,     5,    13,     9,
       1,     2,     4,    13,    13,     8,    12,     1,     8,     1,
       3,     0,     4,     1,     2,     1,     1,     8,     9,     5,
       5,     6,     9,     1,     5,     1,     5,     1,     2,     1,
       1,     1,     5,     5,    11,     5,     5,     7,     7,     5,
       1,     1,     1,     4,     1,     2,     1,     7,     7,     7,
       6,     7,     8,     9,     6,     1,     9,     1,     2,     6,
       1,     1,     1,     7,     1,     5,     9,     5,     1,     1,
       2,     2,     8,     8,     1,     1,     5,     4,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       5,     1,     9,     0,     6,     1,     3,     1,     1,     1,
       1,     1,     9,     1,     3,     1,     4,     1,     3,     4,
       4,     4,     1,     1,     1,     1,     9,     1,     1,     5,
       7,     1,     6,     4,     4,     6,     4,     1,     1,     5,
      18,     1,     3,     5,     1,     2,     8,     9,     1,     1,
       5,     1,     3
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
#line 257 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = new AST ( (yyvsp[-1].tok) ); }
#line 1997 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 258 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = (yyvsp[-2].node); }
#line 2003 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 262 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2009 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 263 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[-2].tok) ); }
#line 2015 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 264 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[-2].tok) ); }
#line 2021 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 268 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-5].tok); }
#line 2027 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 272 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-5].tok); }
#line 2033 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 276 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2039 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 280 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2045 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 285 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2051 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 286 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2057 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2063 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 293 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2069 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 294 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2075 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 295 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2081 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 296 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2087 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2093 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 298 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2099 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 299 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2105 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 300 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2111 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 301 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2117 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 302 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2123 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 303 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2129 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 304 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2135 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 305 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2141 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 306 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2147 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 311 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeDef ( (yyvsp[-9].tok), (yyvsp[-6].fqn), (yyvsp[-3].node) ); }
#line 2153 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 315 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2159 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 316 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2165 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 320 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2171 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2177 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 325 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
#line 2183 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 329 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2189 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 330 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2195 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 335 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeSet ( (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-5].node) ); }
#line 2201 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 339 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2207 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 340 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2213 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 344 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-5].tok), (yyvsp[-2].fqn), 0 ); }
#line 2219 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 345 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-6].tok), (yyvsp[-2].fqn), (yyvsp[-3].fqn) ); }
#line 2225 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 349 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . ConstDef ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2231 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 353 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . AliasDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2237 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 357 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2243 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . UntypedFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2249 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 362 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . RowlenFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2255 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 364 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), false, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2261 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 368 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2267 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 369 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2273 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 373 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2279 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 374 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2285 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 378 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].fqn) ); }
#line 2291 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 379 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].fqn) ); }
#line 2297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 383 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2303 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 384 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2309 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 388 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2315 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 389 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 393 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2327 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 397 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2333 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 398 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), new AST (), (yyvsp[-1].node) ); }
#line 2339 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 399 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), new AST (), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2345 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 400 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2351 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 401 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2357 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 405 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2363 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 409 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2369 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 410 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2375 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 414 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), new AST ( (yyvsp[-1].tok) ), new AST () ); }
#line 2381 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 415 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-2].node), new AST ( (yyvsp[-1].tok) ), new AST ( (yyvsp[-3].tok) ) ); }
#line 2387 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 419 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2393 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 420 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }
#line 2399 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 424 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2405 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 425 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].fqn); }
#line 2411 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 430 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2417 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 434 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2423 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 435 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2429 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 439 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-2].expr) ); }
#line 2435 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 440 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2441 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 444 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2447 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 445 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2453 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 449 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2459 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 452 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 453 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2471 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 458 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), true, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2477 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 467 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-16].tok), false, (yyvsp[-8].node), (yyvsp[-7].node), (yyvsp[-6].fqn), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].node) ); }
#line 2483 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 472 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . PhysicalDecl ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2489 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 476 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2495 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 477 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].node) ); }
#line 2501 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 481 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-9].tok), (yyvsp[-4].node) ); }
#line 2507 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 483 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2513 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 487 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2519 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 488 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2525 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 493 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2531 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 495 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ) ; }
#line 2537 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 497 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ); }
#line 2543 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 499 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn) ); }
#line 2549 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 506 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TableDef ( (yyvsp[-11].tok), (yyvsp[-8].fqn), (yyvsp[-7].node), (yyvsp[-3].node) ); }
#line 2555 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 510 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2561 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 511 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2567 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 515 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2573 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 516 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2579 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 520 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2585 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 521 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2591 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 525 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2597 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 526 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2603 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 530 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2609 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 531 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2615 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 532 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-2].expr) ); }
#line 2621 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 533 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-2].expr) ); }
#line 2627 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 534 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }
#line 2633 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 535 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }
#line 2639 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 536 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2645 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 537 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-4].fqn) ); }
#line 2651 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 538 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2657 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 542 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2663 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 546 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2669 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 547 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2675 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 551 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 552 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 556 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 557 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2699 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 558 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2705 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 562 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2711 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 563 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2717 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 568 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-5].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2723 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 569 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2729 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 570 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2735 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 574 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node) ); }
#line 2741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 575 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
#line 2747 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 576 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node) ); }
#line 2753 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 580 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2759 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 581 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2765 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 585 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2771 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 586 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2777 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 590 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2783 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 591 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2789 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 595 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2795 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 596 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2801 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 597 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2807 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 598 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2813 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 603 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }
#line 2819 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 605 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }
#line 2825 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 607 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2831 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 609 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2837 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 613 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2843 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 617 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2849 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 618 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2855 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 622 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2861 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 623 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2867 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 627 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].node) ); }
#line 2873 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 628 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2879 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 632 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2885 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 633 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2891 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 638 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . DatabaseDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2897 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 642 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2903 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 643 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].fqn); }
#line 2909 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 646 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2915 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 647 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }
#line 2921 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 651 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2927 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 652 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2933 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 653 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2939 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 654 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2945 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 659 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2951 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 661 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2957 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 665 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2963 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 666 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2969 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 672 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . Include ( (yyvsp[-2].tok), (yyvsp[-1].tok) ); }
#line 2975 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 678 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2981 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 682 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].expr) ); }
#line 2987 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 683 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( (yyvsp[0].expr) ); }
#line 2993 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].fqn) ); }
#line 2999 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 688 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3005 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 689 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3011 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 690 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3017 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 691 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3023 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3029 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3035 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 694 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3041 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 695 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3047 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 696 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3053 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 697 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 3059 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 698 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3065 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 703 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( (yyvsp[-5].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-4].node) ); (yyval.expr) -> AddNode ( (yyvsp[-2].node) ); }
#line 3071 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 707 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 3077 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 708 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 3083 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 711 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3089 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 712 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3095 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 716 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 3101 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 717 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3107 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 718 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 3113 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 722 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3119 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 723 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3125 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 727 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 3131 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 731 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3137 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 732 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3143 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 736 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3149 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 737 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 3155 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3161 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 742 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3167 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 746 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3173 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 747 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3179 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 748 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3185 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 752 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3191 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 753 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3197 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 757 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3203 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 758 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3209 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 762 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); }
#line 3215 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 766 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3221 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 767 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3227 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 771 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-4].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }
#line 3233 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 774 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-6].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }
#line 3239 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 778 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3245 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 779 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }
#line 3251 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 785 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-1].fqn); }
#line 3257 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 789 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3263 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 790 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3269 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 794 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3275 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 798 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 3281 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 799 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 3287 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 803 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }
#line 3293 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 810 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . ViewDef ( (yyvsp[-17].tok), (yyvsp[-14].fqn), (yyvsp[-10].node), (yyvsp[-7].node), (yyvsp[-3].node) ); }
#line 3299 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 814 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3305 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 815 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3311 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 819 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].fqn) ); }
#line 3317 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 823 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 824 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3329 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 828 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 3335 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 829 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 3341 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 830 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 3347 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 834 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3353 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 835 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].node) ); }
#line 3359 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 839 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 3365 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 840 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 3371 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;


#line 3375 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
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
