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
#line 27 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:339  */

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
        p_builder . ReportInternalError ( p_msg, p_sb . GetSourceFileName () );
    }


#line 95 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:339  */

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
#ifndef YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED
# define YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED
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
    FLOAT_ = 264,
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
    PT_VIEWPARENTS = 387,
    PT_VIEWPARENT = 388,
    PT_MEMBEREXPR = 389,
    PT_JOINEXPR = 390
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 57 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:355  */

  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;

#line 279 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int AST_parse (AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb);

#endif /* !YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 295 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:358  */

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
#define YYLAST   1138

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  155
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  113
/* YYNRULES -- Number of rules.  */
#define YYNRULES  243
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  858

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   390

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
     137,   138,   143,   150,   140,   151,   153,   152,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   154,   139,
     147,   146,   148,     2,   136,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   141,     2,   142,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   144,   149,   145,     2,     2,     2,     2,
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
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   261,   261,   262,   266,   267,   268,   272,   276,   280,
     284,   289,   290,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   314,   319,
     320,   324,   325,   329,   333,   334,   338,   343,   344,   348,
     349,   353,   357,   361,   365,   366,   367,   372,   373,   377,
     378,   382,   383,   387,   388,   392,   393,   397,   401,   402,
     403,   404,   405,   409,   413,   414,   418,   419,   423,   424,
     428,   429,   430,   434,   438,   439,   443,   444,   448,   449,
     453,   456,   457,   461,   466,   475,   480,   481,   484,   486,
     491,   492,   496,   498,   500,   502,   509,   514,   515,   519,
     520,   524,   525,   529,   530,   534,   535,   536,   537,   538,
     539,   540,   541,   542,   546,   550,   551,   555,   556,   560,
     561,   562,   566,   567,   571,   573,   574,   578,   579,   580,
     584,   585,   589,   590,   594,   595,   599,   600,   601,   602,
     606,   608,   610,   612,   617,   621,   622,   626,   627,   631,
     632,   636,   637,   642,   646,   647,   650,   651,   655,   656,
     657,   658,   662,   664,   669,   670,   676,   682,   686,   687,
     691,   692,   693,   694,   695,   696,   697,   698,   699,   700,
     701,   702,   703,   704,   708,   713,   714,   717,   718,   722,
     723,   724,   728,   729,   733,   737,   738,   742,   743,   747,
     748,   752,   753,   754,   758,   759,   763,   764,   768,   772,
     773,   777,   780,   784,   785,   789,   790,   802,   804,   819,
     823,   824,   828,   832,   833,   837,   842,   848,   849,   853,
     857,   858,   862,   863,   867,   868,   869,   873,   874,   878,
     879,   883,   888,   889
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of source\"", "error", "$undefined", "UNRECOGNIZED", "ELLIPSIS",
  "INCREMENT", "DECIMAL", "OCTAL", "HEX", "FLOAT_", "EXP_FLOAT", "STRING",
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
  "PT_VIEWPARENTS", "PT_VIEWPARENT", "PT_MEMBEREXPR", "PT_JOINEXPR", "'@'",
  "'('", "')'", "';'", "','", "'['", "']'", "'*'", "'{'", "'}'", "'='",
  "'<'", "'>'", "'|'", "'+'", "'-'", "'/'", "'.'", "':'", "$accept",
  "parse", "source", "version_1", "version_2", "schema_1", "schema_2",
  "schema_decls", "schema_decl", "typedef", "new_type_names", "typespec",
  "arrayspec", "dim", "typeset", "typeset_spec", "fmtdef", "const",
  "alias", "function", "func_decl", "schema_sig_opt", "schema_formals",
  "schema_formal", "return_type", "fact_sig", "param_sig",
  "param_signature", "formals_list", "formals", "formal", "vararg",
  "prologue", "script_prologue", "script_stmts", "script_stmt",
  "production", "extern_function", "script", "script_decl", "validate",
  "physical", "phys_return_type", "phys_prologue", "phys_body",
  "phys_body_stmt", "table", "parents_opt", "tbl_parents", "tbl_body",
  "tbl_stmts", "tbl_stmt", "column_decl", "col_modifiers_opt",
  "col_modifiers", "col_modifier", "col_decl", "phys_enc_ref", "typed_col",
  "col_ident", "col_body_opt", "col_body", "col_stmt", "physmbr_decl",
  "phys_coldef", "col_schema_parms_opt", "col_schema_parms",
  "col_schema_parm", "col_schema_value", "database", "dbdad_opt", "dbbody",
  "db_members", "db_member", "template_opt", "include", "cond_expr",
  "cond_chain", "expr", "func_expr", "schema_parts_opt", "schema_parms",
  "schema_parm", "factory_parms_opt", "factory_parms_list",
  "factory_parms", "func_parms_opt", "expr_list", "uint_expr",
  "float_expr", "string_expr", "const_vect_expr", "bool_expr",
  "negate_expr", "cast_expr", "type_expr", "member_expr", "join_expr",
  "fqn", "qualnames", "ident", "fqn_opt_vers", "fqn_vers", "view",
  "view_parms", "view_parm", "view_body_opt", "view_body", "view_member",
  "view_parents_opt", "view_parents", "view_parent", "view_parent_parms", YY_NULLPTR
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
     385,   386,   387,   388,   389,   390,    64,    40,    41,    59,
      44,    91,    93,    42,   123,   125,    61,    60,    62,   124,
      43,    45,    47,    46,    58
};
# endif

#define YYPACT_NINF -674

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-674)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     112,   -41,   182,    53,  -674,    34,    54,   208,  -674,   288,
      92,   107,   142,   169,   251,   256,   191,  -674,   279,   282,
     401,   202,   216,   203,  -674,   340,   347,   222,   242,   246,
     257,   258,   272,   298,   299,   300,   301,   302,   303,   305,
     307,  -674,   230,  -674,  -674,  -674,  -674,  -674,  -674,  -674,
    -674,  -674,  -674,  -674,  -674,  -674,  -674,  -674,  -674,   401,
    -674,   228,   249,   338,   378,   396,   414,   424,   427,   420,
     418,   413,   405,   419,   416,   430,   408,  -674,  -674,   263,
     323,   324,   452,   402,   402,   402,     8,   402,   391,   309,
      16,   393,   211,   343,   343,   343,  -674,  -674,  -674,   342,
     344,   426,   421,   -18,   346,   402,  -674,  -674,   402,   348,
     351,   352,   355,   360,   423,   365,   366,   368,  -674,   369,
     -23,   370,   101,    88,   356,  -674,   442,   372,   373,   374,
     375,   402,   376,   380,  -674,   489,   494,   211,  -674,   382,
     211,  -674,   487,   377,   386,   388,  -674,   343,   402,  -674,
     389,   428,  -674,   390,   412,   474,   395,   121,     8,   392,
    -674,   397,   398,   185,   399,   402,   402,   451,  -674,   451,
     456,   482,    20,   521,   234,   527,   400,   410,   403,   415,
     407,   417,   537,  -674,   485,   236,  -674,   497,  -674,    91,
    -674,  -674,  -674,  -674,  -674,  -674,  -674,   422,   425,   429,
     431,   432,   433,   434,   435,  -674,   437,  -674,  -674,  -674,
    -674,  -674,  -674,  -674,  -674,  -674,  -674,  -674,  -674,   436,
     440,   -22,   -22,   443,   444,   441,   445,  -674,   446,   451,
    -674,   448,   460,   449,   500,   439,   -22,   447,  -674,   457,
     454,   458,   450,     8,   459,  -674,   455,  -674,   384,   411,
     461,   462,   406,   464,   498,   498,   463,   466,   467,  -674,
     234,  -674,   234,   211,   479,  -674,   402,  -674,   468,   453,
     465,   470,  -674,   472,   507,   473,   -19,   475,   265,  -674,
    -674,   552,   477,  -674,     8,   478,   480,   481,   483,   511,
     -22,    -7,   543,   185,   185,   486,     7,   469,  -674,   484,
     488,   509,   509,   451,   490,   270,  -674,   476,  -674,   213,
     267,  -674,   -22,   492,   493,  -674,   495,   496,   -22,   491,
     457,   499,  -674,  -674,   274,  -674,  -674,  -674,  -674,   503,
     139,   504,   506,   508,   510,   604,   512,   498,   564,  -674,
    -674,   514,   536,   542,   -22,    -8,   501,   479,   402,  -674,
     515,   505,   569,   535,   277,  -674,   261,   516,   161,  -674,
     498,     2,  -674,  -674,   502,     8,    22,  -674,   518,   519,
    -674,   185,   185,  -674,  -674,   520,  -674,   522,   524,   517,
     525,   526,   528,  -674,   530,   531,   234,   498,   498,   532,
    -674,   533,   -12,   534,   538,   539,   541,   -22,   540,   544,
     545,   546,   547,   548,  -674,  -674,   129,  -674,  -674,   549,
     551,   553,  -674,   177,  -674,   554,  -674,   556,   513,   557,
    -674,  -674,   278,  -674,  -674,  -674,   550,   172,   558,  -674,
     281,  -674,  -674,   185,    10,   213,   238,  -674,   529,  -674,
     509,   560,   561,  -674,  -674,   563,   577,   -33,  -674,   567,
     559,  -674,  -674,    -7,   590,    32,   617,   232,   622,  -674,
    -674,  -674,   133,   133,   523,  -674,  -674,  -674,   555,   181,
     565,   562,    22,   589,   568,  -674,   570,  -674,   571,   185,
      41,  -674,   573,   498,   574,   576,   609,   402,   536,  -674,
    -674,   588,   -17,  -674,   572,   575,   577,   578,   580,    67,
    -674,   624,   498,   498,  -674,   582,   581,    39,   585,    18,
     579,  -674,  -674,   662,   648,   583,   646,   586,  -674,   584,
     593,  -674,  -674,   587,   185,   594,   595,  -674,  -674,   185,
    -674,   596,   597,  -674,   591,   598,   600,   599,   284,  -674,
     601,   566,   577,   -17,  -674,    12,   602,  -674,   603,   605,
     606,   341,   608,   610,   671,   607,   612,   616,   585,   618,
     402,   -22,   -22,  -674,   613,    65,   619,  -674,   185,   285,
    -674,  -674,  -674,  -674,  -674,   308,   620,   621,     1,  -674,
     588,  -674,   716,   -17,   623,   614,   630,   631,   625,   626,
     308,   564,   564,  -674,  -674,  -674,   315,  -674,    13,  -674,
     627,   185,    21,  -674,   628,  -674,   632,   498,   498,   634,
     633,   635,  -674,    83,  -674,  -674,  -674,   289,  -674,   637,
      98,  -674,  -674,  -674,   638,     8,   728,  -674,   639,   640,
    -674,   402,   642,   643,  -674,  -674,   157,   629,   636,   644,
    -674,   647,   -43,   -43,   185,   649,   654,   650,   729,  -674,
     645,   651,   652,   655,   290,  -674,     8,   718,  -674,  -674,
     641,   185,   706,   653,  -674,  -674,   742,   656,  -674,  -674,
     658,   657,   659,   660,   661,   664,  -674,   -14,   663,   667,
     668,   669,   670,   672,   757,    97,   106,   673,   675,   676,
     -22,   677,   634,   498,     8,   678,  -674,   564,   679,   680,
    -674,   681,   725,   730,   682,  -674,  -674,   737,   683,   139,
      47,    47,  -674,  -674,   684,  -674,   127,  -674,   674,   -22,
     685,   564,   686,  -674,  -674,   687,  -674,  -674,   689,   498,
    -674,   688,  -674,  -674,   690,   692,   693,   694,   696,   698,
     699,  -674,   143,  -674,   695,  -674,   700,   564,   697,   139,
    -674,   701,  -674,   740,   564,   702,   704,  -674,   308,   308,
    -674,    22,  -674,  -674,   705,   193,   564,  -674,   707,   741,
     709,   711,   708,   712,   564,  -674,   173,   187,   293,  -674,
     713,  -674,   710,   714,   719,   715,  -674,  -674,   498,   720,
     717,   721,   722,   691,    59,   723,   724,  -674,    89,  -674,
     294,  -674,   726,   727,   731,   -22,   733,  -674,   117,  -674,
    -674,  -674,   734,   -16,  -674,  -674,  -674,  -674,   732,   498,
    -674,   735,   736,   139,   204,  -674,  -674,   402,   738,  -674,
     739,  -674,  -674,  -674,   743,   744,   745,   746,   747,   749,
    -674,  -674,   753,   564,   564,     4,  -674,   750,   755,   756,
     758,   759,   760,   761,  -674,  -674,  -674,  -674
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
       0,     0,     0,   219,     0,     0,    29,     0,    40,     0,
     204,   205,   206,   207,   171,   210,   209,     0,     0,     0,
       0,     0,     0,     0,     0,   172,     0,   173,   174,   175,
     176,   177,   178,   179,   181,   182,   183,   170,    42,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   153,     0,
       0,     0,     0,     0,     0,    35,     0,    34,     0,   185,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   223,
       0,   224,     0,     0,     0,    53,     0,    54,     0,     0,
       0,     0,   225,     0,   101,     0,     0,     0,     0,   227,
     220,     0,     0,    30,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,    49,     0,    87,     0,
       0,    85,     0,     0,     0,   155,     0,     0,     0,     0,
       0,     0,    28,    37,     0,    33,   201,   203,   202,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    44,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    58,
       0,     0,     0,     0,     0,    99,     0,     0,     0,   157,
       0,     0,   228,   221,     0,     0,     0,   192,     0,     0,
     193,     0,     0,   211,   180,     0,   216,     0,     0,     0,
       0,     0,     0,    72,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   113,   105,     0,   103,   106,     0,
       0,     0,   159,     0,   158,     0,   237,     0,     0,     0,
      38,   190,     0,   187,   191,   189,     0,     0,     0,   199,
       0,   222,   215,     0,     0,     0,     0,    46,     0,    83,
       0,     0,     0,    48,   214,     0,     0,     0,    56,     0,
       0,    98,   100,     0,     0,     0,     0,     0,     0,   102,
     104,    96,     0,     0,     0,   161,   160,   229,     0,     0,
       0,     0,     0,     0,     0,   197,     0,   212,     0,     0,
       0,   168,     0,     0,     0,     0,     0,     0,     0,    51,
      52,     0,     0,    68,     0,     0,     0,     0,     0,     0,
      90,     0,     0,     0,   115,     0,     0,     0,     0,     0,
       0,   165,   164,     0,     0,     0,     0,     0,   230,     0,
       0,   186,   188,     0,     0,     0,     0,   200,   167,     0,
     218,     0,     0,    70,     0,     0,     0,     0,     0,    64,
       0,     0,     0,     0,    59,     0,     0,    91,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   156,     0,     0,     0,    36,     0,     0,
     184,   208,   169,   217,    57,     0,     0,     0,     0,    63,
       0,    60,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   119,   120,   121,     0,   117,     0,   114,
       0,     0,     0,   110,     0,   109,     0,     0,     0,     0,
       0,     0,   236,     0,   232,   226,   195,     0,   198,     0,
       0,    74,    77,    71,     0,     0,     0,    65,     0,     0,
      61,     0,     0,     0,    92,    89,     0,     0,     0,     0,
     118,     0,     0,     0,     0,     0,     0,     0,     0,   111,
       0,     0,     0,     0,     0,   239,     0,     0,   231,   233,
       0,     0,     0,     0,    75,    84,     0,     0,    69,    62,
       0,     0,     0,     0,     0,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   196,     0,     0,     0,
      66,     0,     0,     0,     0,    78,    79,     0,     0,     0,
       0,     0,   122,   123,     0,   107,     0,   145,     0,     0,
       0,     0,     0,   162,   163,     0,   238,   240,     0,     0,
     194,     0,    73,    67,     0,     0,     0,     0,     0,     0,
       0,   131,     0,   130,     0,   108,     0,     0,     0,     0,
     140,     0,   112,     0,     0,     0,     0,    95,     0,     0,
      88,     0,   126,   125,     0,     0,     0,   141,     0,     0,
       0,     0,     0,     0,     0,    76,     0,     0,     0,   129,
       0,   132,     0,     0,     0,     0,   144,   142,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   143,     0,   242,
       0,   234,     0,     0,     0,     0,     0,   136,     0,   134,
     127,   128,     0,     0,   147,   150,   152,   151,     0,     0,
     235,     0,     0,     0,     0,   133,   135,     0,     0,   148,
       0,   243,    93,    94,     0,     0,     0,     0,     0,     0,
     241,   124,     0,     0,     0,     0,   146,     0,     0,     0,
       0,     0,     0,     0,   149,   139,   137,   138
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -674,  -674,  -674,  -674,  -674,   794,  -674,   766,    37,  -674,
    -674,   -81,  -354,  -674,  -674,  -674,  -674,  -674,  -674,   771,
    -674,  -105,  -674,   592,  -138,  -234,  -279,   471,  -412,  -674,
     280,  -454,   387,   611,  -574,  -606,  -292,  -674,  -674,   754,
    -674,  -674,  -674,  -674,  -674,   327,  -674,  -674,  -674,  -674,
    -674,   615,  -674,  -674,  -674,   235,  -674,  -674,   220,   165,
    -674,  -674,    46,  -414,   226,  -674,  -674,    66,    33,  -674,
    -674,  -674,  -674,   665,   438,  -674,  -573,  -674,  -148,  -674,
    -674,   122,   666,  -673,   174,  -674,  -674,   361,  -355,  -674,
    -674,  -674,  -674,  -674,  -674,  -278,  -674,  -674,   -83,  -674,
    -245,  -219,   -87,  -674,  -674,   703,  -674,  -674,   271,  -674,
    -674,   196,  -674
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    14,    15,    16,    23,    42,    43,    44,
     185,   227,   106,   246,    45,   324,    46,    47,    48,    49,
     113,   120,   305,   306,   146,   232,   342,   351,   447,   538,
     539,   497,   382,   383,   620,   621,   622,    50,    51,   116,
      52,    53,   147,   271,   499,   500,    54,   151,   354,   314,
     406,   407,   408,   506,   596,   597,   553,   643,   680,   742,
     782,   808,   809,   557,   648,   719,   813,   814,   815,    55,
     154,   180,   413,   414,   513,    56,   379,   480,   429,   207,
     290,   422,   423,   369,   370,   617,   476,   430,   208,   209,
     210,   211,   212,   213,   214,   228,   215,   216,   107,   157,
     743,   260,   261,    57,   278,   279,   519,   613,   614,   418,
     654,   655,   800
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
     101,   102,   103,   262,   108,   105,   122,   123,   124,   296,
     297,   424,   421,   331,   664,   206,   636,   275,   637,   638,
     130,   336,   132,   343,   482,   133,   301,   625,   302,   221,
     664,   222,   167,   585,   492,   169,   740,   316,   540,   387,
     586,   247,   587,   100,   445,   646,   493,   100,   162,   100,
     114,   100,   197,     5,   494,   558,   504,   100,   100,   144,
     174,   741,   493,   678,   405,   175,   100,   388,   554,   100,
     494,   330,   197,   100,   225,   555,   770,   186,   100,    78,
     217,   416,   219,   220,   543,   100,   679,   100,   505,   584,
     197,   268,   377,   355,   115,   559,     3,   812,   145,   360,
     190,   191,   192,   193,   121,   194,   217,   495,   226,   226,
     496,     6,   121,   295,   405,   415,    78,   424,   421,   104,
     104,   129,   828,   195,   731,   386,   317,   556,   104,   629,
     583,   446,   647,   707,   417,   104,   226,   196,   259,   259,
     104,   641,   441,   442,   604,   333,   334,   104,   751,   104,
     834,   588,   440,   259,   100,   610,   100,   197,   303,   197,
     337,   488,   283,   483,   498,   344,   806,   152,   611,     1,
     664,   664,     8,   610,   768,   502,   717,   511,   452,   528,
     149,   773,     4,   307,   776,   777,   611,   619,   398,   399,
     529,     9,   198,   783,   190,   191,   192,   193,   807,   194,
     150,   790,   812,   323,   612,   546,   153,   259,    10,   718,
     217,   217,   512,   199,   200,   201,   202,   195,   367,   398,
     399,   658,   612,   428,   806,   203,   204,   205,   474,   259,
      17,   196,   400,   401,   245,   259,   663,   517,   531,   402,
     835,   368,   836,   403,    18,   720,   619,   398,   399,   780,
     100,   475,   721,   197,   837,   825,   807,   549,   550,   183,
     518,   259,   619,   398,   399,   391,   746,   459,   404,   508,
     848,   849,   781,   747,   509,   184,   619,   398,   399,    19,
     410,   411,   764,   425,   420,   481,   198,   765,   217,   217,
     118,   119,   349,    27,    28,   673,   410,   411,   350,    29,
     412,    30,    31,    32,    33,    34,    20,   199,   200,   201,
     202,   791,    13,   230,   259,   464,   465,   231,    22,   203,
     204,   205,    35,    36,    37,   792,    27,    28,    38,    24,
      25,   527,    29,    26,    30,    31,    32,    33,    34,   639,
      58,    60,   607,   608,   593,    39,   594,    11,    12,    13,
     217,   398,   399,    59,   595,    35,    36,    37,    61,    63,
      40,    38,   651,   652,   400,   401,    62,    80,    77,    41,
     593,   402,   594,    82,   242,   403,   243,   485,    39,    64,
     595,   572,   486,    65,   487,   110,   111,   112,    81,   425,
     286,   287,   288,    40,    66,    67,   217,   619,   398,   399,
     404,    96,    41,   319,   535,   320,   424,   421,   346,    68,
     347,   352,   364,   353,   365,   396,   471,   397,   472,   478,
     616,   479,   579,   618,   580,   479,    83,   660,   691,   661,
     692,   793,   818,   472,   819,    69,    70,    71,    72,    73,
      74,   217,    75,   816,    76,    84,   217,    85,   728,    86,
      87,    88,    89,   645,    90,    91,    92,    94,   816,    93,
      95,    97,    98,    99,    27,    28,    34,   100,   117,   121,
      29,   725,    30,    31,    32,    33,    34,   606,   259,   259,
     125,   126,   127,   131,   755,   217,   134,   847,   135,   136,
     816,   128,   137,    35,    36,    37,   682,   626,   138,    38,
     749,   115,   140,   155,   141,   142,   143,   148,   156,   158,
     159,   165,   160,   696,   161,   166,    39,   642,   217,   164,
     168,   170,   163,   172,   171,   173,   176,   178,   177,   179,
     181,    40,   182,   144,   223,   188,   187,   218,   224,   189,
      41,   229,   233,   799,   666,   238,   234,   235,   670,   236,
     240,   241,   237,   244,   239,   270,   273,   293,   289,   248,
     304,   217,   249,   313,   295,   321,   250,   329,   251,   252,
     253,   254,   255,   257,   831,   693,   256,   258,   217,   265,
     263,   264,   266,   274,   267,   269,   823,   272,   277,   282,
     709,   276,   280,   341,   708,   281,   284,   285,   291,   332,
     309,   298,   310,   292,   299,   300,   308,   259,   311,   312,
     338,   315,   318,   729,   294,   322,   325,   375,   326,   327,
     378,   328,   339,   335,   381,   394,   340,   345,   348,   356,
     384,   395,   358,   445,   359,   503,   259,   363,   357,   361,
     366,   507,   371,   372,   510,   523,   373,   419,   374,   389,
     376,   380,   392,   393,   409,   426,   427,   469,   431,   434,
     432,   433,   435,   436,   498,   534,   437,   438,   515,   439,
     443,   444,   448,   486,   537,   449,   450,   453,   425,   451,
     548,   454,   455,   456,   457,   458,   552,   461,   462,   561,
     463,   562,   467,   468,   556,   470,   477,   473,   489,   490,
     491,   516,   564,   501,   520,   524,   582,   600,   525,   541,
     521,   530,   532,   526,   533,   817,   544,   545,   542,   551,
     628,   563,   259,   565,   568,   560,   632,   633,   575,   566,
     817,   567,   570,   571,   573,   574,   578,   576,   577,   581,
     590,   667,   694,   686,   838,   598,   697,   589,   599,   602,
     609,   591,   592,   601,   603,   699,   605,   615,   623,   624,
     631,   630,   817,   634,   635,   647,   649,   653,   674,   650,
     656,   716,   657,   644,   662,   675,   665,   668,   669,   671,
     672,   735,   676,   687,   677,   368,   736,   685,   683,   695,
     688,   689,   690,   738,   700,   701,   772,   785,   698,   705,
     710,   702,   706,   703,   711,   704,   712,   713,    21,   714,
     715,   748,   722,   723,   724,   726,   730,   732,   733,   734,
     737,   197,   745,   750,   752,    79,   547,   756,   757,   758,
     759,   640,   760,   761,   753,   754,   762,   763,   767,   805,
     771,   766,   775,   779,   769,   788,   784,   786,   774,   787,
     794,   789,   798,   796,   826,   795,   802,   797,   801,   109,
     627,   810,   811,   681,   820,   821,   803,   804,   139,   822,
     824,   827,   684,   832,   833,   536,   744,   840,   850,   829,
     830,   841,   739,   778,   659,   569,   839,   846,   727,   851,
     842,   843,   844,   845,   852,   853,   854,   855,   856,   857,
       0,   514,     0,     0,     0,     0,   484,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   390,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   385,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   460,     0,   362,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   466,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   522
};

static const yytype_int16 yycheck[] =
{
      83,    84,    85,   222,    87,    86,    93,    94,    95,   254,
     255,   366,   366,   291,   620,   163,   590,   236,   591,   592,
     103,    14,   105,   302,    14,   108,   260,    26,   262,   167,
     636,   169,   137,    21,   446,   140,   709,    56,   492,    47,
      28,   189,    30,    65,    56,    24,    79,    65,   131,    65,
      34,    65,    68,     0,    87,    37,    24,    65,    65,    82,
     147,    14,    79,   106,   356,   148,    65,   345,    29,    65,
      87,   290,    68,    65,    54,    36,   749,   158,    65,    42,
     163,    79,   165,   166,   496,    65,   129,    65,    56,   543,
      68,   229,   337,   312,    78,   509,   137,   113,   121,   318,
       9,    10,    11,    12,   126,    14,   189,   140,   116,   116,
     143,    58,   126,    66,   406,   360,    79,   472,   472,   127,
     127,   139,   138,    32,   697,   344,   145,   109,   127,   583,
     542,   143,   111,   147,   132,   127,   116,    46,   221,   222,
     127,   128,   387,   388,   558,   293,   294,   127,   721,   127,
     823,   139,   386,   236,    65,    90,    65,    68,   263,    68,
     153,   440,   243,   153,    97,   303,   107,    79,   103,    57,
     776,   777,   138,    90,   747,   453,    79,    44,   397,   138,
      79,   754,     0,   266,   758,   759,   103,    89,    90,    91,
     149,   137,   101,   766,     9,    10,    11,    12,   139,    14,
      99,   774,   113,   284,   139,   138,   118,   290,     0,   112,
     293,   294,    79,   122,   123,   124,   125,    32,    79,    90,
      91,   138,   139,   371,   107,   134,   135,   136,    56,   312,
     138,    46,   103,   104,   143,   318,   138,    56,   483,   110,
      36,   102,    38,   114,   137,   139,    89,    90,    91,    56,
      65,    79,   146,    68,    50,   138,   139,   502,   503,   138,
      79,   344,    89,    90,    91,   348,   139,   138,   139,    37,
     843,   844,    79,   146,    42,   154,    89,    90,    91,   137,
     119,   120,   139,   366,   365,   433,   101,   144,   371,   372,
      79,    80,    79,    63,    64,   138,   119,   120,    85,    69,
     139,    71,    72,    73,    74,    75,   137,   122,   123,   124,
     125,   138,    61,    79,   397,   138,   139,    83,    62,   134,
     135,   136,    92,    93,    94,   138,    63,    64,    98,   138,
      51,   479,    69,    51,    71,    72,    73,    74,    75,    24,
     138,   138,   561,   562,    29,   115,    31,    59,    60,    61,
     433,    90,    91,   137,    39,    92,    93,    94,    18,   137,
     130,    98,   607,   608,   103,   104,    19,   139,   138,   139,
      29,   110,    31,    35,   138,   114,   140,   139,   115,   137,
      39,   529,   144,   137,   146,    76,    77,    78,   139,   472,
       6,     7,     8,   130,   137,   137,   479,    89,    90,    91,
     139,   138,   139,   138,   487,   140,   761,   761,   138,   137,
     140,   144,   138,   146,   140,   138,   138,   140,   140,   138,
     568,   140,   138,   138,   140,   140,    48,   138,   138,   140,
     140,   138,   138,   140,   140,   137,   137,   137,   137,   137,
     137,   524,   137,   798,   137,    49,   529,    33,   693,    25,
      23,    31,    34,   601,    41,    50,    37,    27,   813,    43,
      52,   138,   138,    11,    63,    64,    75,    65,    75,   126,
      69,   690,    71,    72,    73,    74,    75,   560,   561,   562,
     138,   137,    56,   137,   729,   568,   138,   842,   137,   137,
     845,    70,   137,    92,    93,    94,   644,   578,   138,    98,
     719,    78,   137,   147,   138,   137,   137,   137,    66,   137,
     137,    22,   138,   661,   139,    21,   115,   598,   601,   139,
     138,    34,   146,   137,   147,   137,   137,   137,   100,   117,
      56,   130,   137,    82,    78,   138,   144,   138,    56,   141,
     139,    20,    15,   788,   625,   138,   146,   137,   631,   146,
      13,    66,   137,    56,   137,    95,    56,   151,   147,   137,
      81,   644,   137,    56,    66,    13,   137,    56,   137,   137,
     137,   137,   137,   137,   819,   656,   139,   137,   661,   138,
     137,   137,   137,   144,   138,   137,   805,   138,   131,   139,
     677,   144,   138,    84,   677,   137,   137,   142,   137,    56,
     147,   138,   137,   141,   138,   138,   138,   690,   138,   137,
     141,   138,   137,   694,   150,   138,   138,    13,   138,   138,
      56,   138,   138,   137,    88,    56,   138,   137,   152,   137,
      88,    96,   137,    56,   138,    45,   719,   138,   145,   148,
     137,    24,   138,   137,    22,    56,   138,   145,   138,   148,
     138,   137,   137,   148,   138,   137,   137,   144,   138,   142,
     138,   137,   137,   137,    97,    56,   138,   137,   145,   138,
     138,   138,   138,   144,    86,   137,   137,   137,   761,   138,
      56,   137,   137,   137,   137,   137,   105,   138,   137,    27,
     137,    43,   138,   137,   109,   138,   138,   147,   138,   138,
     137,   146,    56,   144,   139,   137,   140,    36,   138,   137,
     148,   138,   138,   142,   138,   798,   138,   137,   143,   137,
       4,   138,   805,   137,   137,   146,    96,    96,   137,   145,
     813,   138,   138,   138,   138,   138,   137,   139,   138,   138,
     137,    13,    24,    14,   827,   137,    40,   145,   138,   137,
     137,   146,   146,   146,   138,    13,   138,   138,   138,   138,
     146,   138,   845,   138,   138,   111,   138,   133,   139,   137,
     137,    14,   137,   146,   137,   139,   138,   138,   138,   137,
     137,    56,   138,   138,   137,   102,    56,   137,   139,   148,
     139,   139,   137,    56,   138,   137,    56,    56,   145,   138,
     137,   144,   138,   144,   137,   145,   138,   138,    14,   139,
     138,   137,   139,   138,   138,   138,   138,   138,   138,   138,
     138,    68,   138,   138,   138,    59,   499,   139,   138,   137,
     137,   596,   138,   137,   147,   146,   138,   138,   138,   148,
     139,   146,   138,   138,   147,   137,   139,   138,   146,   138,
     137,   139,   137,   139,   808,   145,   139,   138,   138,    88,
     580,   138,   138,   643,   138,   138,   145,   145,   114,   138,
     137,   137,   646,   138,   138,   488,   711,   138,   845,   813,
     148,   138,   708,   761,   613,   524,   148,   138,   692,   139,
     146,   146,   146,   146,   139,   139,   138,   138,   138,   138,
      -1,   463,    -1,    -1,    -1,    -1,   435,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   347,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   343,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   406,    -1,   320,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   413,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   472
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    57,   156,   137,     0,     0,    58,   157,   138,   137,
       0,    59,    60,    61,   158,   159,   160,   138,   137,   137,
     137,   160,    62,   161,   138,    51,    51,    63,    64,    69,
      71,    72,    73,    74,    75,    92,    93,    94,    98,   115,
     130,   139,   162,   163,   164,   169,   171,   172,   173,   174,
     192,   193,   195,   196,   201,   224,   230,   258,   138,   137,
     138,    18,    19,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   138,   163,   162,
     139,   139,    35,    48,    49,    33,    25,    23,    31,    34,
      41,    50,    37,    43,    27,    52,   138,   138,   138,    11,
      65,   253,   253,   253,   127,   166,   167,   253,   253,   174,
      76,    77,    78,   175,    34,    78,   194,    75,    79,    80,
     176,   126,   257,   257,   257,   138,   137,    56,    70,   139,
     253,   137,   253,   253,   138,   137,   137,   137,   138,   194,
     137,   138,   137,   137,    82,   121,   179,   197,   137,    79,
      99,   202,    79,   118,   225,   147,    66,   254,   137,   137,
     138,   139,   253,   146,   139,    22,    21,   176,   138,   176,
      34,   147,   137,   137,   257,   253,   137,   100,   137,   117,
     226,    56,   137,   138,   154,   165,   166,   144,   138,   141,
       9,    10,    11,    12,    14,    32,    46,    68,   101,   122,
     123,   124,   125,   134,   135,   136,   233,   234,   243,   244,
     245,   246,   247,   248,   249,   251,   252,   253,   138,   253,
     253,   179,   179,    78,    56,    54,   116,   166,   250,    20,
      79,    83,   180,    15,   146,   137,   146,   137,   138,   137,
      13,    66,   138,   140,    56,   143,   168,   233,   137,   137,
     137,   137,   137,   137,   137,   137,   139,   137,   137,   253,
     256,   257,   256,   137,   137,   138,   137,   138,   179,   137,
      95,   198,   138,    56,   144,   256,   144,   131,   259,   260,
     138,   137,   139,   166,   137,   142,     6,     7,     8,   147,
     235,   137,   141,   151,   150,    66,   255,   255,   138,   138,
     138,   180,   180,   176,    81,   177,   178,   253,   138,   147,
     137,   138,   137,    56,   204,   138,    56,   145,   137,   138,
     140,    13,   138,   166,   170,   138,   138,   138,   138,    56,
     256,   250,    56,   233,   233,   137,    14,   153,   141,   138,
     138,    84,   181,   181,   179,   137,   138,   140,   152,    79,
      85,   182,   144,   146,   203,   256,   137,   145,   137,   138,
     256,   148,   260,   138,   138,   140,   137,    79,   102,   238,
     239,   138,   137,   138,   138,    13,   138,   255,    56,   231,
     137,    88,   187,   188,    88,   188,   256,    47,   250,   148,
     178,   253,   137,   148,    56,    96,   138,   140,    90,    91,
     103,   104,   110,   114,   139,   191,   205,   206,   207,   138,
     119,   120,   139,   227,   228,   255,    79,   132,   264,   145,
     166,   167,   236,   237,   243,   253,   137,   137,   233,   233,
     242,   138,   138,   137,   142,   137,   137,   138,   137,   138,
     180,   255,   255,   138,   138,    56,   143,   183,   138,   137,
     137,   138,   256,   137,   137,   137,   137,   137,   137,   138,
     206,   138,   137,   137,   138,   139,   228,   138,   137,   144,
     138,   138,   140,   147,    56,    79,   241,   138,   138,   140,
     232,   233,    14,   153,   182,   139,   144,   146,   181,   138,
     138,   137,   183,    79,    87,   140,   143,   186,    97,   199,
     200,   144,   250,    45,    24,    56,   208,    24,    37,    42,
      22,    44,    79,   229,   229,   145,   146,    56,    79,   261,
     139,   148,   237,    56,   137,   138,   142,   233,   138,   149,
     138,   255,   138,   138,    56,   253,   187,    86,   184,   185,
     186,   137,   143,   183,   138,   137,   138,   200,    56,   255,
     255,   137,   105,   211,    29,    36,   109,   218,    37,   218,
     146,    27,    43,   138,    56,   137,   145,   138,   137,   242,
     138,   138,   233,   138,   138,   137,   139,   138,   137,   138,
     140,   138,   140,   183,   186,    21,    28,    30,   139,   145,
     137,   146,   146,    29,    31,    39,   209,   210,   137,   138,
      36,   146,   137,   138,   218,   138,   253,   256,   256,   137,
      90,   103,   139,   262,   263,   138,   233,   240,   138,    89,
     189,   190,   191,   138,   138,    26,   166,   185,     4,   186,
     138,   146,    96,    96,   138,   138,   189,   231,   231,    24,
     210,   128,   166,   212,   146,   233,    24,   111,   219,   138,
     137,   255,   255,   133,   265,   266,   137,   137,   138,   263,
     138,   140,   137,   138,   190,   138,   166,    13,   138,   138,
     253,   137,   137,   138,   139,   139,   138,   137,   106,   129,
     213,   213,   233,   139,   219,   137,    14,   138,   139,   139,
     137,   138,   140,   166,    24,   148,   233,    40,   145,    13,
     138,   137,   144,   144,   145,   138,   138,   147,   253,   257,
     137,   137,   138,   138,   139,   138,    14,    79,   112,   220,
     139,   146,   139,   138,   138,   256,   138,   266,   255,   166,
     138,   231,   138,   138,   138,    56,    56,   138,    56,   239,
     238,    14,   214,   255,   214,   138,   139,   146,   137,   256,
     138,   231,   138,   147,   146,   255,   139,   138,   137,   137,
     138,   137,   138,   138,   139,   144,   146,   138,   231,   147,
     238,   139,    56,   231,   146,   138,   189,   189,   236,   138,
      56,    79,   215,   231,   139,    56,   138,   138,   137,   139,
     231,   138,   138,   138,   137,   145,   139,   138,   137,   255,
     267,   138,   139,   145,   145,   148,   107,   139,   216,   217,
     138,   138,   113,   221,   222,   223,   243,   253,   138,   140,
     138,   138,   138,   256,   137,   138,   217,   137,   138,   222,
     148,   255,   138,   138,   238,    36,    38,    50,   253,   148,
     138,   138,   146,   146,   146,   146,   138,   243,   231,   231,
     223,   139,   139,   139,   138,   138,   138,   138
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   155,   156,   156,   157,   157,   157,   158,   159,   160,
     161,   162,   162,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   164,   165,
     165,   166,   166,   167,   168,   168,   169,   170,   170,   171,
     171,   172,   173,   174,   175,   175,   175,   176,   176,   177,
     177,   178,   178,   179,   179,   180,   180,   181,   182,   182,
     182,   182,   182,   183,   184,   184,   185,   185,   186,   186,
     187,   187,   187,   188,   189,   189,   190,   190,   191,   191,
     192,   193,   193,   194,   195,   196,   197,   197,   198,   198,
     199,   199,   200,   200,   200,   200,   201,   202,   202,   203,
     203,   204,   204,   205,   205,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   207,   208,   208,   209,   209,   210,
     210,   210,   211,   211,   212,   212,   212,   213,   213,   213,
     214,   214,   215,   215,   216,   216,   217,   217,   217,   217,
     218,   218,   218,   218,   219,   220,   220,   221,   221,   222,
     222,   223,   223,   224,   225,   225,   226,   226,   227,   227,
     227,   227,   228,   228,   229,   229,   230,   231,   232,   232,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   234,   235,   235,   236,   236,   237,
     237,   237,   238,   238,   239,   240,   240,   241,   241,   242,
     242,   243,   243,   243,   244,   244,   245,   245,   246,   247,
     247,   248,   249,   250,   250,   251,   251,   252,   252,   253,
     254,   254,   255,   256,   256,   257,   258,   259,   259,   260,
     261,   261,   262,   262,   263,   263,   263,   264,   264,   265,
     265,   266,   267,   267
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
       5,     1,     1,     1,     9,     0,     6,     1,     3,     1,
       1,     1,     1,     1,     9,     1,     3,     1,     4,     1,
       3,     4,     4,     4,     1,     1,     1,     1,     9,     1,
       1,     5,     7,     1,     6,     6,     5,     9,     8,     4,
       4,     6,     4,     1,     1,     5,    15,     1,     3,     5,
       1,     4,     1,     2,     8,     9,     1,     1,     8,     1,
       3,    10,     1,     3
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
#line 261 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = new AST ( (yyvsp[-1].tok) ); }
#line 2028 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 262 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = (yyvsp[-2].node); }
#line 2034 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 266 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2040 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 267 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[-2].tok) ); }
#line 2046 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 268 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[-2].tok) ); }
#line 2052 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 272 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-5].tok); }
#line 2058 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 276 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-5].tok); }
#line 2064 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 280 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2070 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 284 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2076 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 289 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2082 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 290 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2088 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 296 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2094 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 297 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2100 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 298 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2106 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 299 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2112 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 300 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2118 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 301 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2124 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 302 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2130 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 303 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2136 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 304 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2142 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 305 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2148 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 306 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2154 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 307 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2160 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 308 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2166 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 309 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2172 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 310 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2178 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 315 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeDef ( (yyvsp[-9].tok), (yyvsp[-6].fqn), (yyvsp[-3].node) ); }
#line 2184 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 319 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2190 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 320 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2196 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 324 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2202 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 325 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2208 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 329 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
#line 2214 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 333 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2220 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 334 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2226 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 339 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeSet ( (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-5].node) ); }
#line 2232 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 343 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2238 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 344 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2244 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 348 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-5].tok), (yyvsp[-2].fqn), 0 ); }
#line 2250 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 349 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-6].tok), (yyvsp[-2].fqn), (yyvsp[-3].fqn) ); }
#line 2256 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 353 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . ConstDef ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2262 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 357 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . AliasDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2268 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 361 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2274 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 365 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . UntypedFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2280 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 366 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . RowlenFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2286 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 368 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), false, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2292 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 372 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2298 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 373 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2304 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 377 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2310 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 378 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2316 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 382 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].fqn) ); }
#line 2322 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 383 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].fqn) ); }
#line 2328 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 387 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2334 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 388 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2340 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 392 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2346 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 393 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2352 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 397 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2358 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 401 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2364 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 402 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), new AST (), (yyvsp[-1].node) ); }
#line 2370 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 403 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), new AST (), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2376 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 404 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2382 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 405 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2388 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 409 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2394 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 413 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2400 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 414 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2406 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 418 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), new AST ( (yyvsp[-1].tok) ), new AST () ); }
#line 2412 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 419 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-2].node), new AST ( (yyvsp[-1].tok) ), new AST ( (yyvsp[-3].tok) ) ); }
#line 2418 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 423 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2424 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 424 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }
#line 2430 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 428 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2436 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 429 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].fqn); }
#line 2442 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 434 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2448 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 438 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2454 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 439 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2460 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 443 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-2].expr) ); }
#line 2466 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 444 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2472 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 448 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2478 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 449 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2484 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 453 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2490 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 456 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2496 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 457 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2502 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 462 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), true, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2508 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 471 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-16].tok), false, (yyvsp[-8].node), (yyvsp[-7].node), (yyvsp[-6].fqn), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].node) ); }
#line 2514 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 476 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . PhysicalDecl ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2520 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 480 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2526 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 481 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].node) ); }
#line 2532 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 485 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-9].tok), (yyvsp[-4].node) ); }
#line 2538 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 487 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2544 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 491 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2550 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 492 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2556 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 497 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2562 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 499 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ) ; }
#line 2568 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 501 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ); }
#line 2574 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 503 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn) ); }
#line 2580 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 510 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TableDef ( (yyvsp[-11].tok), (yyvsp[-8].fqn), (yyvsp[-7].node), (yyvsp[-3].node) ); }
#line 2586 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 514 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2592 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 515 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2598 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 519 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2604 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 520 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2610 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 524 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2616 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 525 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2622 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 529 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2628 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 530 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2634 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 534 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2640 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 535 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2646 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 536 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-2].expr) ); }
#line 2652 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 537 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-2].expr) ); }
#line 2658 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 538 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }
#line 2664 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 539 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }
#line 2670 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 540 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2676 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 541 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-4].fqn) ); }
#line 2682 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 542 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2688 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 546 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2694 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 550 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2700 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 551 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2706 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 555 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2712 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 556 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2718 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 560 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2724 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 561 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2730 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 562 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2736 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 566 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2742 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 567 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2748 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 572 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-5].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2754 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 573 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2760 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 574 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2766 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 578 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node) ); }
#line 2772 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 579 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
#line 2778 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 580 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node) ); }
#line 2784 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 584 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2790 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 585 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2796 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 589 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2802 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 590 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2808 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 594 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2814 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 595 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2820 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 599 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2826 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 600 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2832 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 601 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2838 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 602 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2844 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 607 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }
#line 2850 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 609 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }
#line 2856 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 611 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2862 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 613 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2868 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 617 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2874 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 621 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2880 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 622 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2886 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 626 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2892 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 627 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2898 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 631 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].node) ); }
#line 2904 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 632 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2910 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 636 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2916 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 637 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2922 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 642 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . DatabaseDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2928 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 646 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2934 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 647 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].fqn); }
#line 2940 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 650 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2946 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 651 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }
#line 2952 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 655 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2958 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 656 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2964 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 657 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2970 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 658 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2976 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 663 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2982 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 665 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2988 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 669 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2994 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 670 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3000 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 676 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . Include ( (yyvsp[-2].tok), (yyvsp[-1].tok) ); }
#line 3006 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 682 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 3012 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 686 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].expr) ); }
#line 3018 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 687 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( (yyvsp[0].expr) ); }
#line 3024 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 691 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].fqn) ); }
#line 3030 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 692 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3036 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 693 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3042 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 694 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3048 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 695 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3054 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 696 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3060 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 697 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3066 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 698 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3072 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 699 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3078 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 700 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3084 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 701 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 3090 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 702 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3096 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 703 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3102 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 704 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3108 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 709 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( (yyvsp[-5].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-4].node) ); (yyval.expr) -> AddNode ( (yyvsp[-2].node) ); }
#line 3114 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 713 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 3120 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 714 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 3126 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 717 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3132 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 718 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3138 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 722 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 3144 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 723 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3150 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 724 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 3156 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 728 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3162 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 729 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3168 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 733 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 3174 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 737 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3180 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 738 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3186 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 742 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3192 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 743 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 3198 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 747 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3204 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 748 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3210 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 752 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3216 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 753 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3222 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 754 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3228 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 758 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3234 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 759 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3240 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 763 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3246 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 764 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3252 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 768 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); }
#line 3258 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 772 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3264 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 773 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3270 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 777 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-4].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }
#line 3276 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 780 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-6].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }
#line 3282 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 784 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3288 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 785 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }
#line 3294 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 789 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-5].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-1].fqn) ); }
#line 3300 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 791 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {   /* remove leading '.'*/
            (yyval.expr) = new AST_Expr ( (yyvsp[-4].tok) );
            (yyval.expr) -> AddNode ( (yyvsp[-2].fqn) );
            AST * ident = new AST ( PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( & t );
            (yyval.expr) -> AddNode ( ident );
        }
#line 3313 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 803 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-6].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-4].expr) ); (yyval.expr) -> AddNode ( (yyvsp[-1].fqn) ); }
#line 3319 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 805 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {   /* remove leading '.'*/
            (yyval.expr) = new AST_Expr ( (yyvsp[-7].tok) );
            (yyval.expr) -> AddNode ( (yyvsp[-5].fqn) );
            (yyval.expr) -> AddNode ( (yyvsp[-3].expr) );
            AST * ident = new AST ( PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( & t );
            (yyval.expr) -> AddNode ( ident );
        }
#line 3333 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 819 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-1].fqn); }
#line 3339 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 823 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3345 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 824 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3351 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 828 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3357 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 832 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 3363 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 833 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 3369 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 837 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }
#line 3375 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 844 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . ViewDef ( (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-2].node) ); }
#line 3381 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 848 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3387 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 849 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3393 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 853 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].fqn) ); }
#line 3399 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 857 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3405 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 858 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 3411 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 862 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3417 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 863 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3423 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 867 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 3429 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 868 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 3435 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 869 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 3441 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 873 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3447 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 874 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-2].node) ); }
#line 3453 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 878 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3459 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 879 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3465 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 884 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-9].tok), (yyvsp[-7].fqn), (yyvsp[-3].node) ); }
#line 3471 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 888 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 3477 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 889 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 3483 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;


#line 3487 "/home/boshkins/ncbi/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
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
