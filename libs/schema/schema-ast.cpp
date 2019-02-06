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


  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int AST_parse (AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb);

#endif /* !YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */



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
#define YYLAST   1085

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  155
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  113
/* YYNRULES -- Number of rules.  */
#define YYNRULES  242
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  856

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
     773,   777,   780,   784,   785,   789,   790,   802,   809,   813,
     814,   818,   822,   823,   827,   832,   838,   839,   843,   847,
     848,   852,   853,   857,   858,   859,   863,   864,   868,   869,
     873,   878,   879
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

#define YYPACT_NINF -664

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-664)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      22,   -94,    69,    86,  -664,   -50,    31,   221,  -664,   338,
      92,   159,   186,   208,   232,   246,   224,  -664,   321,   322,
     261,   240,   251,   263,  -664,   366,   383,   279,   302,   305,
     306,   307,   310,   311,   312,   316,   317,   318,   320,   324,
     325,  -664,   180,  -664,  -664,  -664,  -664,  -664,  -664,  -664,
    -664,  -664,  -664,  -664,  -664,  -664,  -664,  -664,  -664,   261,
    -664,   319,   326,   415,   404,   410,   427,   438,   441,   435,
     433,   429,   418,   434,   430,   445,   422,  -664,  -664,   228,
     340,   341,   469,   416,   416,   416,   -20,   416,   409,   330,
     101,   411,   315,   361,   361,   361,  -664,  -664,  -664,   351,
     353,   436,   421,    -8,   356,   416,  -664,  -664,   416,   358,
     360,   362,   363,   364,   423,   367,   365,   368,  -664,   369,
     -26,   370,   125,    62,   371,  -664,   442,   372,   373,   374,
     375,   416,   376,   378,  -664,   491,   498,   315,  -664,   382,
     315,  -664,   487,   377,   386,   388,  -664,   361,   416,  -664,
     389,   428,  -664,   390,   412,   474,   394,   -84,   -20,   391,
    -664,   395,   393,    64,   398,   416,   416,   450,  -664,   450,
     459,   482,   -18,   519,   268,   525,   397,   407,   399,   413,
     414,   417,   534,  -664,   483,   110,  -664,   492,  -664,    91,
    -664,  -664,  -664,  -664,  -664,  -664,  -664,   419,   420,   424,
     425,   426,   431,   432,   437,  -664,   439,  -664,  -664,  -664,
    -664,  -664,  -664,  -664,  -664,  -664,  -664,  -664,  -664,   440,
     443,    -6,    -6,   444,   446,   447,   449,  -664,   451,   450,
    -664,   453,   456,   455,   497,   452,    -6,   454,  -664,   448,
     457,   460,   461,   -20,   462,  -664,   464,  -664,   403,   408,
     465,   463,   458,   466,   494,   494,   470,   472,   475,  -664,
     268,  -664,   268,   315,   477,  -664,   416,  -664,   476,   468,
     480,   481,  -664,   484,   508,   485,   -17,   488,   225,  -664,
    -664,   552,   486,  -664,   -20,   489,   490,   493,   495,   510,
      -6,    75,   511,    64,    64,   499,     7,   471,  -664,   500,
     501,   503,   503,   450,   504,   252,  -664,   478,  -664,   127,
     271,  -664,    -6,   505,   473,  -664,   506,   502,    -6,   496,
     448,   507,  -664,  -664,   281,  -664,  -664,  -664,  -664,   509,
     185,   512,   514,   515,   516,   557,   517,   494,   526,  -664,
    -664,   520,   513,   532,    -6,     3,   518,   477,   416,  -664,
     521,   522,   535,   479,   282,  -664,   172,   523,   229,  -664,
     494,   -41,  -664,  -664,   524,   -20,   -13,  -664,   527,   528,
    -664,    64,    64,  -664,  -664,   529,  -664,   530,   536,   533,
     537,   540,   541,  -664,   543,   544,   268,   494,   494,   545,
    -664,   546,   -22,   547,   549,   550,   551,    -6,   553,   554,
     555,   556,   558,   559,  -664,  -664,   128,  -664,  -664,   560,
     562,   563,  -664,   218,  -664,   564,  -664,   566,   561,   568,
    -664,  -664,   285,  -664,  -664,  -664,   531,   161,   569,  -664,
     286,  -664,  -664,    64,   548,   127,   235,  -664,   565,  -664,
     503,   570,   572,  -664,  -664,   567,   538,    29,  -664,   574,
     571,  -664,  -664,    75,   577,    34,   579,   140,   585,  -664,
    -664,  -664,   129,   129,   573,  -664,  -664,  -664,   542,   234,
     575,   576,   -13,   581,   580,  -664,   578,  -664,   539,    64,
     160,  -664,   494,   583,   584,   591,   416,   513,  -664,  -664,
     586,   -15,  -664,   582,   587,   538,   588,   590,   -44,  -664,
     592,   494,   494,  -664,   594,   589,   276,   602,    25,   593,
    -664,  -664,   599,   606,   595,   596,   597,  -664,   598,   600,
    -664,  -664,   603,    64,   607,   608,  -664,  -664,    64,   609,
     610,  -664,   604,   605,   611,   613,   289,  -664,   614,   615,
     538,   -15,  -664,    12,   612,  -664,   616,   617,   618,   313,
     619,   620,   623,   621,   622,   624,   602,   627,   416,    -6,
      -6,  -664,   629,   138,   630,  -664,    64,   290,  -664,  -664,
    -664,  -664,  -664,   323,   631,   632,    20,  -664,   586,  -664,
     625,   -15,   633,   626,   601,   636,   635,   637,   323,   526,
     526,  -664,  -664,  -664,   346,  -664,    39,  -664,   628,    64,
      11,  -664,   638,  -664,   640,   494,   494,   645,   642,   643,
    -664,   130,  -664,  -664,  -664,   293,  -664,   644,   156,  -664,
    -664,  -664,   646,   -20,   647,  -664,   648,   649,  -664,   416,
     651,   652,  -664,  -664,   169,   653,   654,   656,  -664,   658,
     198,   198,    64,   657,   650,   660,   698,  -664,   661,   659,
     662,   663,   294,  -664,   -20,   639,  -664,  -664,   634,    64,
     683,   664,  -664,  -664,   712,   665,  -664,  -664,   667,   641,
     666,   668,   669,   670,  -664,   -14,   674,   675,   676,   677,
     678,   680,   715,    97,   200,   681,   684,   685,    -6,   686,
     645,   494,   -20,   687,  -664,   526,   688,   689,  -664,   690,
     679,   695,   691,  -664,  -664,   727,   700,   185,    47,    47,
    -664,  -664,   692,  -664,   247,  -664,   682,    -6,   693,   526,
     694,  -664,  -664,   696,  -664,  -664,   699,   494,  -664,   697,
    -664,  -664,   701,   703,   704,   706,   705,   708,   709,  -664,
     243,  -664,   702,  -664,   711,   526,   707,   185,  -664,   713,
    -664,   734,   526,   710,   717,  -664,   323,   323,  -664,   -13,
    -664,  -664,   719,   250,   526,  -664,   714,   735,   720,   721,
     723,   722,   526,  -664,   190,   226,   297,  -664,   725,  -664,
     671,   724,   726,   728,  -664,  -664,   494,   729,   730,   731,
     732,   673,    83,   733,   736,  -664,    84,  -664,   298,  -664,
     737,   740,   741,    -6,   743,  -664,    57,  -664,  -664,  -664,
     744,    -2,  -664,  -664,  -664,  -664,   718,   494,  -664,   745,
     746,   185,   333,  -664,  -664,   416,   738,  -664,   747,  -664,
    -664,  -664,   749,   742,   748,   750,   751,   752,  -664,  -664,
     765,   526,   526,   171,  -664,   753,   754,   756,   760,   761,
     762,   763,  -664,  -664,  -664,  -664
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
       0,     0,     0,   218,     0,     0,    29,     0,    40,     0,
     204,   205,   206,   207,   171,   210,   209,     0,     0,     0,
       0,     0,     0,     0,     0,   172,     0,   173,   174,   175,
     176,   177,   178,   179,   181,   182,   183,   170,    42,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,     0,
      55,     0,     0,     0,     0,     0,     0,     0,   153,     0,
       0,     0,     0,     0,     0,    35,     0,    34,     0,   185,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,   223,     0,     0,     0,    53,     0,    54,     0,     0,
       0,     0,   224,     0,   101,     0,     0,     0,     0,   226,
     219,     0,     0,    30,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,    49,     0,    87,     0,
       0,    85,     0,     0,     0,   155,     0,     0,     0,     0,
       0,     0,    28,    37,     0,    33,   201,   203,   202,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    44,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    58,
       0,     0,     0,     0,     0,    99,     0,     0,     0,   157,
       0,     0,   227,   220,     0,     0,     0,   192,     0,     0,
     193,     0,     0,   211,   180,     0,   216,     0,     0,     0,
       0,     0,     0,    72,     0,     0,     0,     0,     0,     0,
      50,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   113,   105,     0,   103,   106,     0,
       0,     0,   159,     0,   158,     0,   236,     0,     0,     0,
      38,   190,     0,   187,   191,   189,     0,     0,     0,   199,
       0,   221,   215,     0,     0,     0,     0,    46,     0,    83,
       0,     0,     0,    48,   214,     0,     0,     0,    56,     0,
       0,    98,   100,     0,     0,     0,     0,     0,     0,   102,
     104,    96,     0,     0,     0,   161,   160,   228,     0,     0,
       0,     0,     0,     0,     0,   197,     0,   212,     0,     0,
       0,   168,     0,     0,     0,     0,     0,     0,    51,    52,
       0,     0,    68,     0,     0,     0,     0,     0,     0,    90,
       0,     0,     0,   115,     0,     0,     0,     0,     0,     0,
     165,   164,     0,     0,     0,     0,     0,   229,     0,     0,
     186,   188,     0,     0,     0,     0,   200,   167,     0,     0,
       0,    70,     0,     0,     0,     0,     0,    64,     0,     0,
       0,     0,    59,     0,     0,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   156,     0,     0,     0,    36,     0,     0,   184,   208,
     169,   217,    57,     0,     0,     0,     0,    63,     0,    60,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   119,   120,   121,     0,   117,     0,   114,     0,     0,
       0,   110,     0,   109,     0,     0,     0,     0,     0,     0,
     235,     0,   231,   225,   195,     0,   198,     0,     0,    74,
      77,    71,     0,     0,     0,    65,     0,     0,    61,     0,
       0,     0,    92,    89,     0,     0,     0,     0,   118,     0,
       0,     0,     0,     0,     0,     0,     0,   111,     0,     0,
       0,     0,     0,   238,     0,     0,   230,   232,     0,     0,
       0,     0,    75,    84,     0,     0,    69,    62,     0,     0,
       0,     0,     0,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   196,     0,     0,     0,    66,     0,
       0,     0,     0,    78,    79,     0,     0,     0,     0,     0,
     122,   123,     0,   107,     0,   145,     0,     0,     0,     0,
       0,   162,   163,     0,   237,   239,     0,     0,   194,     0,
      73,    67,     0,     0,     0,     0,     0,     0,     0,   131,
       0,   130,     0,   108,     0,     0,     0,     0,   140,     0,
     112,     0,     0,     0,     0,    95,     0,     0,    88,     0,
     126,   125,     0,     0,     0,   141,     0,     0,     0,     0,
       0,     0,     0,    76,     0,     0,     0,   129,     0,   132,
       0,     0,     0,     0,   144,   142,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   143,     0,   241,     0,   233,
       0,     0,     0,     0,     0,   136,     0,   134,   127,   128,
       0,     0,   147,   150,   152,   151,     0,     0,   234,     0,
       0,     0,     0,   133,   135,     0,     0,   148,     0,   242,
      93,    94,     0,     0,     0,     0,     0,     0,   240,   124,
       0,     0,     0,     0,   146,     0,     0,     0,     0,     0,
       0,     0,   149,   139,   137,   138
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -664,  -664,  -664,  -664,  -664,   791,  -664,   775,   115,  -664,
    -664,   -81,  -354,  -664,  -664,  -664,  -664,  -664,  -664,   780,
    -664,  -109,  -664,   288,  -140,  -236,  -279,   136,  -414,  -664,
      54,  -454,   175,   655,  -572,  -604,  -201,  -664,  -664,   758,
    -664,  -664,  -664,  -664,  -664,   158,  -664,  -664,  -664,  -664,
    -664,   331,  -664,  -664,  -664,   142,  -664,  -664,   113,   126,
    -664,  -664,     0,  -459,   193,  -664,  -664,    27,     8,  -664,
    -664,  -664,  -664,   672,   387,  -664,  -571,  -664,  -148,  -664,
    -664,   111,   401,  -663,   176,  -664,  -664,   379,  -355,  -664,
    -664,  -664,  -664,  -664,  -664,  -278,  -664,  -664,   -83,  -664,
    -245,  -219,   -87,  -664,  -664,   716,  -664,  -664,   278,  -664,
    -664,   201,  -664
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    14,    15,    16,    23,    42,    43,    44,
     185,   227,   106,   246,    45,   324,    46,    47,    48,    49,
     113,   120,   305,   306,   146,   232,   342,   351,   447,   536,
     537,   496,   382,   383,   618,   619,   620,    50,    51,   116,
      52,    53,   147,   271,   498,   499,    54,   151,   354,   314,
     406,   407,   408,   505,   594,   595,   551,   641,   678,   740,
     780,   806,   807,   555,   646,   717,   811,   812,   813,    55,
     154,   180,   413,   414,   512,    56,   379,   480,   429,   207,
     290,   422,   423,   369,   370,   615,   476,   430,   208,   209,
     210,   211,   212,   213,   214,   228,   215,   216,   107,   157,
     741,   260,   261,    57,   278,   279,   518,   611,   612,   418,
     652,   653,   798
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
     101,   102,   103,   262,   108,   105,   122,   123,   124,   296,
     297,   424,   421,   331,   662,   206,   634,   275,   635,   636,
     130,   336,   132,   343,   301,   133,   302,   221,   167,   222,
     662,   169,   491,   583,   445,   644,   225,   538,   416,   316,
     584,   247,   585,     3,   738,   100,   623,   100,   162,   557,
     387,   100,   100,   497,   183,   197,   144,   100,   503,   100,
     174,   739,   556,   100,   492,   175,   197,   388,   100,     4,
     184,   330,   493,   190,   191,   192,   193,   186,   194,     1,
     217,   541,   219,   220,   768,   100,     5,   582,     8,   268,
     504,   417,   377,   355,   544,   145,   195,   602,   226,   360,
     190,   191,   192,   193,   100,   194,   217,   104,   492,   104,
     196,   810,   121,   295,   104,   415,   493,   424,   421,   226,
     121,   446,   645,   195,   729,   386,   581,   627,   317,   100,
     104,   129,   197,   705,   554,   114,   826,   196,   259,   259,
     100,   152,   441,   442,     6,   333,   334,   104,   749,   100,
     440,   586,   197,   259,   303,   405,   100,    78,   832,   197,
     337,   487,   283,   344,   804,   198,   104,   639,     9,   494,
     662,   662,   495,   510,   766,   501,   715,   507,   452,   115,
     153,   771,   508,   307,   774,   775,   199,   200,   201,   202,
     804,   226,   198,   781,    78,   823,   805,   810,   203,   204,
     205,   788,   104,   323,   149,   405,   349,   259,   511,   716,
     217,   217,   350,   199,   200,   201,   202,   474,   398,   399,
     608,    10,   805,   428,   150,   203,   204,   205,   608,   259,
      17,   400,   401,   609,   245,   259,   100,   529,   402,   197,
     475,   609,   403,    27,    28,   617,   398,   399,   242,    29,
     243,    30,    31,    32,    33,    34,   547,   548,   617,   398,
     399,   259,   398,   399,   367,   391,   459,   404,   656,   610,
     846,   847,    35,    36,    37,   400,   401,   610,    38,   617,
     398,   399,   402,   425,   420,   481,   403,   368,   217,   217,
     516,    27,    28,    13,   661,    39,    18,    29,   527,    30,
      31,    32,    33,    34,   676,   552,   778,   671,    22,   528,
      40,   404,   553,   517,   259,   617,   398,   399,    77,    41,
      35,    36,    37,    19,    27,    28,    38,   677,   789,   779,
      29,   526,    30,    31,    32,    33,    34,   410,   411,   718,
     605,   606,   591,    39,   592,    20,   719,   230,   410,   411,
     217,   231,   593,    35,    36,    37,   464,   465,    40,    38,
     649,   650,    24,   319,   790,   320,    96,    41,   412,   833,
     637,   834,    25,    26,   484,   591,    39,   592,    58,   485,
     570,   486,   762,   835,    61,   593,   744,   763,    59,   425,
     346,    40,   347,   745,   118,   119,   217,    11,    12,    13,
      41,    60,    62,   533,   424,   421,   110,   111,   112,   286,
     287,   288,   617,   398,   399,   352,    63,   353,   614,   364,
     396,   365,   397,   471,   478,   472,   479,   577,   616,   578,
     479,   658,   689,   659,   690,   791,   816,   472,   817,    64,
     217,   814,    65,    66,    67,   217,   726,    68,    69,    70,
      82,   643,    83,    71,    72,    73,   814,    74,    80,    84,
      85,    75,    76,    86,    87,    81,    88,    89,    91,   723,
      90,    92,    94,    93,    95,   604,   259,   259,    97,    98,
      99,   100,   753,   217,    34,   845,   117,   121,   814,   125,
     126,   128,   127,   131,   680,   624,   134,   135,   747,   136,
     137,   115,   138,   141,   140,   142,   143,   148,   156,   158,
     159,   694,   160,   165,   161,   640,   217,   164,   155,   166,
     168,   170,   163,   172,   171,   173,   176,   178,   177,   179,
     181,   182,   144,   188,   189,   187,   218,   223,   224,   229,
     233,   797,   664,   234,   235,   236,   668,   240,   244,   241,
     237,   270,   238,   273,   239,   289,   248,   249,   304,   217,
     295,   250,   251,   252,   313,   321,   329,   332,   253,   254,
     375,   483,   829,   691,   255,   395,   217,   257,   256,   277,
     258,   263,   378,   264,   821,   265,   266,   341,   707,   267,
     269,   394,   706,   272,   445,   280,   274,   281,   276,   284,
     282,   381,   291,   506,   292,   259,   285,   509,   298,   293,
     299,   727,   338,   300,   308,   309,   294,   310,   357,   311,
     384,   312,   502,   315,   322,   318,   559,   325,   326,   626,
     348,   327,   625,   328,   259,   390,   335,   522,   339,   340,
     359,   345,   356,   358,   361,   363,   366,   532,   546,   560,
     371,   372,   562,   373,   374,   376,   545,   380,   392,   598,
     665,   409,   534,   692,   426,   427,   389,   431,   432,   419,
     393,   497,   535,   433,   435,   434,   425,   436,   473,   437,
     438,   525,   439,   443,   444,   448,   449,   450,   515,   451,
     453,   454,   455,   456,   550,   457,   458,   630,   461,   462,
     463,   482,   467,   468,   490,   469,   470,   477,   488,   485,
     489,   554,   684,   815,   519,   500,   524,   523,   514,   539,
     259,   530,   531,   695,   520,   697,   542,   543,   815,   714,
     540,   549,   631,   561,   563,   733,   638,   460,   565,   558,
     566,   573,   836,   564,   574,   568,   569,   571,   572,   575,
     576,   734,   579,   588,   679,   580,   596,   587,   597,   600,
     815,   645,   601,   589,   590,   603,   607,   599,   613,   621,
     622,   628,   629,   632,   642,   633,   647,   648,   651,   654,
     655,   660,   693,   736,   663,   700,   666,   667,   669,   670,
     770,   783,   672,   673,   674,   675,   681,   683,   686,   685,
     688,   687,   368,   698,   699,    21,   824,   703,   704,   696,
     701,   708,   709,   702,   710,   711,   793,   712,   713,   746,
     720,   803,   721,   722,   724,   728,   730,   731,   732,   735,
     743,   748,   750,   197,    79,   742,   754,   682,   827,   755,
     756,   757,   759,   751,   758,   752,   760,   761,   764,   765,
     513,   848,   769,   782,   767,   773,   772,   777,   784,   785,
     786,   787,   792,   794,   795,   796,   828,   799,   109,   800,
     776,   808,   139,   521,   809,   818,   801,   802,   819,   820,
     822,   825,   737,   830,   831,   838,   837,   839,   840,   657,
     844,   725,   849,   850,   841,   851,   842,   843,   852,   853,
     854,   855,   567,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   385,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   362,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   466
};

static const yytype_int16 yycheck[] =
{
      83,    84,    85,   222,    87,    86,    93,    94,    95,   254,
     255,   366,   366,   291,   618,   163,   588,   236,   589,   590,
     103,    14,   105,   302,   260,   108,   262,   167,   137,   169,
     634,   140,   446,    21,    56,    24,    54,   491,    79,    56,
      28,   189,    30,   137,   707,    65,    26,    65,   131,   508,
      47,    65,    65,    97,   138,    68,    82,    65,    24,    65,
     147,    14,    37,    65,    79,   148,    68,   345,    65,     0,
     154,   290,    87,     9,    10,    11,    12,   158,    14,    57,
     163,   495,   165,   166,   747,    65,     0,   541,   138,   229,
      56,   132,   337,   312,   138,   121,    32,   556,   116,   318,
       9,    10,    11,    12,    65,    14,   189,   127,    79,   127,
      46,   113,   126,    66,   127,   360,    87,   472,   472,   116,
     126,   143,   111,    32,   695,   344,   540,   581,   145,    65,
     127,   139,    68,   147,   109,    34,   138,    46,   221,   222,
      65,    79,   387,   388,    58,   293,   294,   127,   719,    65,
     386,   139,    68,   236,   263,   356,    65,    42,   821,    68,
     153,   440,   243,   303,   107,   101,   127,   128,   137,   140,
     774,   775,   143,    44,   745,   453,    79,    37,   397,    78,
     118,   752,    42,   266,   756,   757,   122,   123,   124,   125,
     107,   116,   101,   764,    79,   138,   139,   113,   134,   135,
     136,   772,   127,   284,    79,   406,    79,   290,    79,   112,
     293,   294,    85,   122,   123,   124,   125,    56,    90,    91,
      90,     0,   139,   371,    99,   134,   135,   136,    90,   312,
     138,   103,   104,   103,   143,   318,    65,   482,   110,    68,
      79,   103,   114,    63,    64,    89,    90,    91,   138,    69,
     140,    71,    72,    73,    74,    75,   501,   502,    89,    90,
      91,   344,    90,    91,    79,   348,   138,   139,   138,   139,
     841,   842,    92,    93,    94,   103,   104,   139,    98,    89,
      90,    91,   110,   366,   365,   433,   114,   102,   371,   372,
      56,    63,    64,    61,   138,   115,   137,    69,   138,    71,
      72,    73,    74,    75,   106,    29,    56,   138,    62,   149,
     130,   139,    36,    79,   397,    89,    90,    91,   138,   139,
      92,    93,    94,   137,    63,    64,    98,   129,   138,    79,
      69,   479,    71,    72,    73,    74,    75,   119,   120,   139,
     559,   560,    29,   115,    31,   137,   146,    79,   119,   120,
     433,    83,    39,    92,    93,    94,   138,   139,   130,    98,
     605,   606,   138,   138,   138,   140,   138,   139,   139,    36,
      24,    38,    51,    51,   139,    29,   115,    31,   138,   144,
     528,   146,   139,    50,    18,    39,   139,   144,   137,   472,
     138,   130,   140,   146,    79,    80,   479,    59,    60,    61,
     139,   138,    19,   486,   759,   759,    76,    77,    78,     6,
       7,     8,    89,    90,    91,   144,   137,   146,   566,   138,
     138,   140,   140,   138,   138,   140,   140,   138,   138,   140,
     140,   138,   138,   140,   140,   138,   138,   140,   140,   137,
     523,   796,   137,   137,   137,   528,   691,   137,   137,   137,
      35,   599,    48,   137,   137,   137,   811,   137,   139,    49,
      33,   137,   137,    25,    23,   139,    31,    34,    50,   688,
      41,    37,    27,    43,    52,   558,   559,   560,   138,   138,
      11,    65,   727,   566,    75,   840,    75,   126,   843,   138,
     137,    70,    56,   137,   642,   576,   138,   137,   717,   137,
     137,    78,   138,   138,   137,   137,   137,   137,    66,   137,
     137,   659,   138,    22,   139,   596,   599,   139,   147,    21,
     138,    34,   146,   137,   147,   137,   137,   137,   100,   117,
      56,   137,    82,   138,   141,   144,   138,    78,    56,    20,
      15,   786,   623,   146,   137,   146,   629,    13,    56,    66,
     137,    95,   138,    56,   137,   147,   137,   137,    81,   642,
      66,   137,   137,   137,    56,    13,    56,    56,   137,   137,
      13,   435,   817,   654,   137,    96,   659,   137,   139,   131,
     137,   137,    56,   137,   803,   138,   137,    84,   675,   138,
     137,    56,   675,   138,    56,   138,   144,   137,   144,   137,
     139,    88,   137,    24,   141,   688,   142,    22,   138,   151,
     138,   692,   141,   138,   138,   147,   150,   137,   145,   138,
      88,   137,    45,   138,   138,   137,    27,   138,   138,     4,
     152,   138,   578,   138,   717,   347,   137,    56,   138,   138,
     138,   137,   137,   137,   148,   138,   137,    56,    56,    43,
     138,   137,    56,   138,   138,   138,   498,   137,   137,    36,
      13,   138,   487,    24,   137,   137,   148,   138,   138,   145,
     148,    97,    86,   137,   137,   142,   759,   137,   147,   138,
     137,   142,   138,   138,   138,   138,   137,   137,   146,   138,
     137,   137,   137,   137,   105,   137,   137,    96,   138,   137,
     137,   153,   138,   137,   137,   144,   138,   138,   138,   144,
     138,   109,    14,   796,   139,   144,   138,   137,   145,   137,
     803,   138,   138,    40,   148,    13,   138,   137,   811,    14,
     143,   137,    96,   138,   137,    56,   594,   406,   138,   146,
     137,   137,   825,   145,   139,   138,   138,   138,   138,   138,
     137,    56,   138,   137,   641,   140,   137,   145,   138,   137,
     843,   111,   138,   146,   146,   138,   137,   146,   138,   138,
     138,   138,   146,   138,   146,   138,   138,   137,   133,   137,
     137,   137,   148,    56,   138,   144,   138,   138,   137,   137,
      56,    56,   139,   139,   138,   137,   139,   137,   139,   138,
     137,   139,   102,   138,   137,    14,   806,   138,   138,   145,
     144,   137,   137,   145,   138,   138,   145,   139,   138,   137,
     139,   148,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,    68,    59,   709,   139,   644,   811,   138,
     137,   137,   137,   147,   138,   146,   138,   138,   146,   138,
     463,   843,   139,   139,   147,   138,   146,   138,   138,   138,
     137,   139,   137,   139,   138,   137,   148,   138,    88,   139,
     759,   138,   114,   472,   138,   138,   145,   145,   138,   138,
     137,   137,   706,   138,   138,   138,   148,   138,   146,   611,
     138,   690,   139,   139,   146,   139,   146,   146,   138,   138,
     138,   138,   523,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   320,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   413
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
     232,   233,   153,   182,   139,   144,   146,   181,   138,   138,
     137,   183,    79,    87,   140,   143,   186,    97,   199,   200,
     144,   250,    45,    24,    56,   208,    24,    37,    42,    22,
      44,    79,   229,   229,   145,   146,    56,    79,   261,   139,
     148,   237,    56,   137,   138,   142,   233,   138,   149,   255,
     138,   138,    56,   253,   187,    86,   184,   185,   186,   137,
     143,   183,   138,   137,   138,   200,    56,   255,   255,   137,
     105,   211,    29,    36,   109,   218,    37,   218,   146,    27,
      43,   138,    56,   137,   145,   138,   137,   242,   138,   138,
     233,   138,   138,   137,   139,   138,   137,   138,   140,   138,
     140,   183,   186,    21,    28,    30,   139,   145,   137,   146,
     146,    29,    31,    39,   209,   210,   137,   138,    36,   146,
     137,   138,   218,   138,   253,   256,   256,   137,    90,   103,
     139,   262,   263,   138,   233,   240,   138,    89,   189,   190,
     191,   138,   138,    26,   166,   185,     4,   186,   138,   146,
      96,    96,   138,   138,   189,   231,   231,    24,   210,   128,
     166,   212,   146,   233,    24,   111,   219,   138,   137,   255,
     255,   133,   265,   266,   137,   137,   138,   263,   138,   140,
     137,   138,   190,   138,   166,    13,   138,   138,   253,   137,
     137,   138,   139,   139,   138,   137,   106,   129,   213,   213,
     233,   139,   219,   137,    14,   138,   139,   139,   137,   138,
     140,   166,    24,   148,   233,    40,   145,    13,   138,   137,
     144,   144,   145,   138,   138,   147,   253,   257,   137,   137,
     138,   138,   139,   138,    14,    79,   112,   220,   139,   146,
     139,   138,   138,   256,   138,   266,   255,   166,   138,   231,
     138,   138,   138,    56,    56,   138,    56,   239,   238,    14,
     214,   255,   214,   138,   139,   146,   137,   256,   138,   231,
     138,   147,   146,   255,   139,   138,   137,   137,   138,   137,
     138,   138,   139,   144,   146,   138,   231,   147,   238,   139,
      56,   231,   146,   138,   189,   189,   236,   138,    56,    79,
     215,   231,   139,    56,   138,   138,   137,   139,   231,   138,
     138,   138,   137,   145,   139,   138,   137,   255,   267,   138,
     139,   145,   145,   148,   107,   139,   216,   217,   138,   138,
     113,   221,   222,   223,   243,   253,   138,   140,   138,   138,
     138,   256,   137,   138,   217,   137,   138,   222,   148,   255,
     138,   138,   238,    36,    38,    50,   253,   148,   138,   138,
     146,   146,   146,   146,   138,   243,   231,   231,   223,   139,
     139,   139,   138,   138,   138,   138
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
     247,   248,   249,   250,   250,   251,   251,   252,   253,   254,
     254,   255,   256,   256,   257,   258,   259,   259,   260,   261,
     261,   262,   262,   263,   263,   263,   264,   264,   265,   265,
     266,   267,   267
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
       1,     5,     7,     1,     6,     6,     5,     9,     4,     4,
       6,     4,     1,     1,     5,    15,     1,     3,     5,     1,
       4,     1,     2,     8,     9,     1,     1,     8,     1,     3,
      10,     1,     3
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

    { p_ast = new AST ( (yyvsp[-1].tok) ); }

    break;

  case 3:

    { p_ast = (yyvsp[-2].node); }

    break;

  case 4:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 5:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[-2].tok) ); }

    break;

  case 6:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[-2].tok) ); }

    break;

  case 7:

    { (yyval.tok) = (yyvsp[-5].tok); }

    break;

  case 8:

    { (yyval.tok) = (yyvsp[-5].tok); }

    break;

  case 9:

    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }

    break;

  case 10:

    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }

    break;

  case 11:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 12:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 13:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 14:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 15:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 16:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 17:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 18:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 19:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 20:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 21:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 22:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 23:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 24:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 25:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 26:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 27:

    { (yyval.node) = new AST (); }

    break;

  case 28:

    { (yyval.node) = p_builder . TypeDef ( (yyvsp[-9].tok), (yyvsp[-6].fqn), (yyvsp[-3].node) ); }

    break;

  case 29:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 30:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 31:

    { (yyval.node) = (yyvsp[0].fqn); }

    break;

  case 32:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 33:

    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }

    break;

  case 34:

    { (yyval.node) = (yyvsp[0].expr); }

    break;

  case 35:

    { (yyval.node) = new AST ( PT_EMPTY ); }

    break;

  case 36:

    { (yyval.node) = p_builder . TypeSet ( (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-5].node) ); }

    break;

  case 37:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 38:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 39:

    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-5].tok), (yyvsp[-2].fqn), 0 ); }

    break;

  case 40:

    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-6].tok), (yyvsp[-2].fqn), (yyvsp[-3].fqn) ); }

    break;

  case 41:

    { (yyval.node) = p_builder . ConstDef ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }

    break;

  case 42:

    { (yyval.node) = p_builder . AliasDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }

    break;

  case 43:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 44:

    { (yyval.node) = p_builder . UntypedFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }

    break;

  case 45:

    { (yyval.node) = p_builder . RowlenFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }

    break;

  case 46:

    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), false, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 47:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 48:

    { (yyval.node) = (yyvsp[-3].node); }

    break;

  case 49:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 50:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 51:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].fqn) ); }

    break;

  case 52:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].fqn) ); }

    break;

  case 53:

    { (yyval.node) = new AST (); }

    break;

  case 54:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 55:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 56:

    { (yyval.node) = (yyvsp[-2].node); }

    break;

  case 57:

    { (yyval.node) = (yyvsp[-2].node); }

    break;

  case 58:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 59:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), new AST (), (yyvsp[-1].node) ); }

    break;

  case 60:

    { (yyval.node) = new AST ( (yyvsp[-5].tok), new AST (), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 61:

    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 62:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 63:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 64:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 65:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 66:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), new AST ( (yyvsp[-1].tok) ), new AST () ); }

    break;

  case 67:

    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-2].node), new AST ( (yyvsp[-1].tok) ), new AST ( (yyvsp[-3].tok) ) ); }

    break;

  case 68:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 69:

    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }

    break;

  case 70:

    { (yyval.node) = new AST ( PT_EMPTY ); }

    break;

  case 71:

    { (yyval.node) = (yyvsp[-2].fqn); }

    break;

  case 73:

    { (yyval.node) = (yyvsp[-3].node); }

    break;

  case 74:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 75:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 76:

    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-2].expr) ); }

    break;

  case 77:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 78:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }

    break;

  case 79:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }

    break;

  case 80:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 81:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 82:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 83:

    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), true, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 84:

    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-16].tok), false, (yyvsp[-8].node), (yyvsp[-7].node), (yyvsp[-6].fqn), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].node) ); }

    break;

  case 85:

    { (yyval.node) = p_builder . PhysicalDecl ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 86:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 87:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].node) ); }

    break;

  case 88:

    { (yyval.node) = new AST ( (yyvsp[-9].tok), (yyvsp[-4].node) ); }

    break;

  case 89:

    { (yyval.node) = (yyvsp[-3].node); }

    break;

  case 90:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 91:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 92:

    { (yyval.node) = new AST ( PT_EMPTY ); }

    break;

  case 93:

    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ) ; }

    break;

  case 94:

    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ); }

    break;

  case 95:

    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn) ); }

    break;

  case 96:

    { (yyval.node) = p_builder . TableDef ( (yyvsp[-11].tok), (yyvsp[-8].fqn), (yyvsp[-7].node), (yyvsp[-3].node) ); }

    break;

  case 97:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 98:

    { (yyval.node) = (yyvsp[-2].node); }

    break;

  case 99:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }

    break;

  case 100:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }

    break;

  case 101:

    { (yyval.node) = new AST ( PT_EMPTY ); }

    break;

  case 102:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 103:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 104:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 105:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 106:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 107:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-2].expr) ); }

    break;

  case 108:

    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-2].expr) ); }

    break;

  case 109:

    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }

    break;

  case 110:

    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }

    break;

  case 111:

    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }

    break;

  case 112:

    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-4].fqn) ); }

    break;

  case 113:

    { (yyval.node) = new AST (); }

    break;

  case 114:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 115:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 116:

    { (yyval.node) = (yyvsp[-2].node); }

    break;

  case 117:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 118:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 119:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 120:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 121:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 122:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 123:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 124:

    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-5].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }

    break;

  case 125:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }

    break;

  case 126:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }

    break;

  case 127:

    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node) ); }

    break;

  case 128:

    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }

    break;

  case 129:

    {  (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node) ); }

    break;

  case 130:

    { (yyval.node) = (yyvsp[0].fqn); }

    break;

  case 131:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 132:

    { (yyval.node) = new AST (); }

    break;

  case 133:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 134:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 135:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 136:

    { (yyval.node) = new AST (); }

    break;

  case 137:

    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }

    break;

  case 138:

    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }

    break;

  case 139:

    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }

    break;

  case 140:

    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }

    break;

  case 141:

    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }

    break;

  case 142:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }

    break;

  case 143:

    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }

    break;

  case 144:

    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }

    break;

  case 145:

    { (yyval.node) = new AST (); }

    break;

  case 146:

    { (yyval.node) = (yyvsp[-3].node); }

    break;

  case 147:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 148:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 149:

    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].node) ); }

    break;

  case 150:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 151:

    { (yyval.node) = (yyvsp[0].fqn); }

    break;

  case 152:

    { (yyval.node) = (yyvsp[0].expr); }

    break;

  case 153:

    { (yyval.node) = p_builder . DatabaseDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }

    break;

  case 154:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 155:

    { (yyval.node) = (yyvsp[-1].fqn); }

    break;

  case 156:

    { (yyval.node) = (yyvsp[-3].node); }

    break;

  case 157:

    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }

    break;

  case 158:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 159:

    { (yyval.node) = new AST (); }

    break;

  case 160:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 161:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 162:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }

    break;

  case 163:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }

    break;

  case 164:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 165:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 166:

    { (yyval.node) = p_builder . Include ( (yyvsp[-2].tok), (yyvsp[-1].tok) ); }

    break;

  case 167:

    { (yyval.expr) = (yyvsp[-1].expr); }

    break;

  case 168:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].expr) ); }

    break;

  case 169:

    { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( (yyvsp[0].expr) ); }

    break;

  case 170:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].fqn) ); }

    break;

  case 171:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 172:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 173:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 174:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 175:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 176:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 177:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 178:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 179:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 180:

    { (yyval.expr) = (yyvsp[-1].expr); }

    break;

  case 181:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 182:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 183:

    { (yyval.expr) = (yyvsp[0].expr); }

    break;

  case 184:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( (yyvsp[-5].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-4].node) ); (yyval.expr) -> AddNode ( (yyvsp[-2].node) ); }

    break;

  case 185:

    { (yyval.node) = new AST ( PT_EMPTY ); }

    break;

  case 186:

    { (yyval.node) = (yyvsp[-2].node); }

    break;

  case 187:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 188:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 189:

    { (yyval.node) = (yyvsp[0].fqn); }

    break;

  case 190:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 191:

    { (yyval.node) = (yyvsp[0].expr); }

    break;

  case 192:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 193:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 194:

    { (yyval.node) = (yyvsp[-3].node); }

    break;

  case 195:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }

    break;

  case 196:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }

    break;

  case 197:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 198:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 199:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }

    break;

  case 200:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }

    break;

  case 201:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }

    break;

  case 202:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }

    break;

  case 203:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }

    break;

  case 204:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 205:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 206:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 207:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 208:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); }

    break;

  case 209:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 210:

    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }

    break;

  case 211:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-4].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }

    break;

  case 212:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-6].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }

    break;

  case 213:

    { (yyval.node) = (yyvsp[0].node); }

    break;

  case 214:

    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }

    break;

  case 215:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-5].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-1].fqn) ); }

    break;

  case 216:

    {   /* remove leading '.'*/
            (yyval.expr) = new AST_Expr ( (yyvsp[-4].tok) );
            (yyval.expr) -> AddNode ( (yyvsp[-2].fqn) );
            AST * ident = new AST ( PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( & t );
            (yyval.expr) -> AddNode ( ident );
        }

    break;

  case 217:

    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-6].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-4].expr) ); (yyval.expr) -> AddNode ( (yyvsp[-1].fqn) ); }

    break;

  case 218:

    { (yyval.fqn) = (yyvsp[-1].fqn); }

    break;

  case 219:

    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }

    break;

  case 220:

    { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }

    break;

  case 221:

    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }

    break;

  case 222:

    { (yyval.fqn) = (yyvsp[0].fqn); }

    break;

  case 223:

    { (yyval.fqn) = (yyvsp[0].fqn); }

    break;

  case 224:

    { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }

    break;

  case 225:

    { (yyval.node) = p_builder . ViewDef ( (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-2].node) ); }

    break;

  case 226:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 227:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 228:

    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].fqn) ); }

    break;

  case 229:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 230:

    { (yyval.node) = (yyvsp[-1].node); }

    break;

  case 231:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 232:

    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 233:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }

    break;

  case 234:

    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }

    break;

  case 235:

    { (yyval.node) = new AST ( PT_EMPTY ); }

    break;

  case 236:

    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }

    break;

  case 237:

    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-2].node) ); }

    break;

  case 238:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 239:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }

    break;

  case 240:

    { (yyval.node) = new AST ( (yyvsp[-9].tok), (yyvsp[-7].fqn), (yyvsp[-3].node) ); }

    break;

  case 241:

    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }

    break;

  case 242:

    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }

    break;



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
