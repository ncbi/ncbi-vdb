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
        p_builder . ReportError ( msg );
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
    PT_PHYSENCREF = 382,
    PT_TYPEDCOLEXPR = 383
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

#line 275 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int AST_parse (AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb);

#endif /* !YY_AST_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 291 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:358  */

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
#define YYLAST   1075

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  147
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  100
/* YYNRULES -- Number of rules.  */
#define YYNRULES  216
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  759

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   383

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
     129,   130,   135,   143,   132,   144,     2,   145,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   146,   131,
     139,   138,   140,     2,   142,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   133,     2,   134,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   136,   141,   137,     2,     2,     2,     2,
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
     125,   126,   127,   128
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   256,   256,   257,   261,   262,   263,   267,   271,   275,
     279,   284,   285,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   308,   313,   314,
     318,   319,   323,   327,   328,   332,   337,   338,   342,   343,
     347,   351,   355,   359,   360,   361,   366,   367,   371,   372,
     376,   377,   381,   382,   386,   387,   391,   395,   396,   397,
     398,   399,   403,   407,   408,   412,   413,   417,   418,   422,
     423,   424,   428,   432,   433,   437,   438,   442,   443,   447,
     450,   451,   455,   460,   469,   474,   475,   478,   480,   485,
     486,   490,   492,   494,   496,   503,   508,   509,   513,   514,
     518,   519,   523,   524,   528,   529,   530,   531,   532,   533,
     534,   535,   539,   543,   544,   548,   549,   553,   554,   555,
     559,   560,   564,   566,   567,   571,   572,   573,   577,   578,
     582,   583,   587,   588,   592,   593,   594,   595,   599,   601,
     603,   605,   610,   614,   615,   619,   620,   624,   625,   629,
     630,   635,   639,   640,   643,   644,   648,   649,   650,   651,
     655,   657,   662,   663,   669,   675,   679,   680,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   693,   694,   695,
     699,   704,   705,   708,   709,   713,   714,   715,   719,   720,
     724,   725,   729,   730,   734,   735,   739,   740,   741,   745,
     746,   750,   751,   755,   759,   760,   764,   767,   771,   772,
     778,   782,   783,   787,   791,   792,   796
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
  "PT_AT", "PT_PHYSENCREF", "PT_TYPEDCOLEXPR", "'('", "')'", "';'", "','",
  "'['", "']'", "'*'", "'{'", "'}'", "'='", "'<'", "'>'", "'|'", "'@'",
  "'+'", "'-'", "'/'", "':'", "$accept", "parse", "source", "version_1",
  "version_2", "schema_1", "schema_2", "schema_decls", "schema_decl",
  "typedef", "new_type_names", "typespec", "arrayspec", "dim", "typeset",
  "typeset_spec", "fmtdef", "const", "alias", "function", "func_decl",
  "schema_sig_opt", "schema_formals", "schema_formal", "return_type",
  "fact_sig", "param_sig", "param_signature", "formals_list", "formals",
  "formal", "vararg", "prologue", "script_prologue", "script_stmts",
  "script_stmt", "production", "extern_function", "script", "script_decl",
  "validate", "physical", "phys_return_type", "phys_prologue", "phys_body",
  "phys_body_stmt", "table", "parents_opt", "tbl_parents", "tbl_body",
  "tbl_stmts", "tbl_stmt", "column_decl", "col_modifiers_opt",
  "col_modifiers", "col_modifier", "col_decl", "phys_enc_ref", "typed_col",
  "col_ident", "col_body_opt", "col_body", "col_stmt", "physmbr_decl",
  "phys_coldef", "col_schema_parms_opt", "col_schema_parms",
  "col_schema_parm", "col_schema_value", "database", "opt_dbdad", "dbbody",
  "db_members", "db_member", "template_opt", "include", "cond_expr",
  "cond_chain", "expr", "func_expr", "schema_parts_opt", "schema_parms",
  "schema_parm", "factory_parms_opt", "factory_parms", "func_parms_opt",
  "expr_list", "uint_expr", "float_expr", "string_expr", "const_vect_expr",
  "bool_expr", "negate_expr", "cast_expr", "type_expr", "fqn", "qualnames",
  "ident", "fqn_opt_vers", "fqn_vers", YY_NULLPTR
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
     375,   376,   377,   378,   379,   380,   381,   382,   383,    40,
      41,    59,    44,    91,    93,    42,   123,   125,    61,    60,
      62,   124,    64,    43,    45,    47,    58
};
# endif

#define YYPACT_NINF -577

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-577)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     119,   -34,   284,    76,  -577,   101,   200,   309,  -577,   273,
     221,   241,   243,   244,   261,  -577,   245,  -577,   324,   326,
     180,   247,   248,  -577,   361,   371,   252,   254,   255,   256,
     257,   263,   265,   266,   267,   268,   269,   270,   272,  -577,
     143,  -577,  -577,  -577,  -577,  -577,  -577,  -577,  -577,  -577,
    -577,  -577,  -577,  -577,  -577,  -577,  -577,   251,   274,   353,
     341,   342,   357,   376,   380,   373,   374,   366,   359,   375,
     367,   384,  -577,  -577,   282,   283,   403,   352,   352,   352,
      95,   352,   343,   260,    13,   344,   280,   294,   294,  -577,
    -577,   289,   291,   370,   358,   -19,   292,   352,  -577,  -577,
     352,   299,   302,   303,   304,   305,   364,   308,   311,   313,
    -577,   314,    88,   315,   129,   122,  -577,   382,   318,   319,
     320,   321,   352,   296,   322,  -577,   428,   434,   280,  -577,
     325,   280,  -577,   424,   323,   329,   330,  -577,   294,   352,
    -577,   332,   362,  -577,   334,   349,   337,   131,    95,   333,
    -577,   338,   339,    56,   340,   352,   352,   387,  -577,   387,
     395,   418,    55,   455,    93,   460,   345,   348,   346,   350,
     351,   465,  -577,   416,   193,  -577,   431,  -577,    18,  -577,
    -577,  -577,  -577,  -577,  -577,  -577,   360,   363,   365,   368,
     369,   372,  -577,   356,  -577,  -577,  -577,  -577,  -577,  -577,
    -577,  -577,  -577,  -577,   377,   378,    -8,    -8,   379,   381,
     383,   385,  -577,   386,   387,  -577,   388,   397,   389,   433,
     355,    -8,   390,  -577,   391,   393,   394,    95,   398,  -577,
     396,  -577,   331,   354,   399,   400,   392,   401,   402,   404,
     405,  -577,    93,  -577,    93,   280,   417,  -577,   352,  -577,
     407,   406,   409,   410,  -577,   412,   440,   413,   -30,  -577,
     486,   419,  -577,    95,   420,   421,   422,   423,   445,    -8,
      71,   447,    56,    56,  -577,   425,   426,   438,   438,   387,
     429,   213,  -577,   414,  -577,   192,   210,  -577,    -8,   432,
     411,  -577,   435,   427,   430,  -577,  -577,   217,  -577,  -577,
    -577,  -577,   436,   148,   437,   439,   441,   442,  -577,  -577,
     444,   443,   453,    -8,    47,   446,   417,   352,  -577,   448,
     449,   450,   452,   220,  -577,   206,   451,   -57,  -577,  -577,
     454,    95,    58,  -577,   456,   458,    56,    56,  -577,  -577,
     459,   461,   462,  -577,   464,   466,    93,   467,   467,   468,
    -577,   470,   -12,   471,   473,   474,   475,    -8,   477,   478,
     479,   480,   481,   482,  -577,   190,  -577,  -577,   483,   485,
     487,  -577,   186,  -577,   488,  -577,  -577,   224,  -577,  -577,
    -577,   408,   191,   489,  -577,   231,   192,   126,  -577,   476,
    -577,   438,   491,   492,   493,  -577,  -577,   495,   469,    17,
    -577,   484,   490,  -577,  -577,    71,   498,    49,   531,    23,
     494,  -577,  -577,  -577,   104,   104,   457,  -577,  -577,   496,
     497,    58,   508,   499,  -577,   500,  -577,   463,    56,   501,
     503,   511,   352,   443,   556,  -577,  -577,   520,   181,  -577,
     505,   504,   469,   506,   509,   -44,  -577,   515,   467,   467,
    -577,   512,   472,   271,   510,    39,   502,  -577,  -577,   548,
     534,   514,   516,  -577,  -577,   513,    56,   517,   518,  -577,
     519,  -577,   521,   522,   525,   526,   523,   232,  -577,   527,
     528,   469,   181,  -577,    11,   524,  -577,   529,   507,   530,
     258,   533,   535,   543,   532,   537,   541,   510,   542,   352,
      -8,    -8,  -577,  -577,    56,   235,  -577,  -577,  -577,   253,
     544,   545,  -577,    10,  -577,   520,  -577,   576,   181,   546,
     539,   557,   565,   549,   550,   253,   540,   540,  -577,  -577,
    -577,   259,  -577,    57,  -577,   547,    56,    -6,  -577,   551,
    -577,   538,   467,   467,  -577,   236,  -577,   553,   100,  -577,
    -577,  -577,   554,    95,   569,  -577,   558,   559,  -577,   352,
     561,   562,  -577,  -577,   114,   563,   552,   555,   564,  -577,
     566,   139,   139,    56,   567,   578,   568,   570,  -577,   571,
     572,   573,   560,    56,   582,   536,  -577,  -577,   602,   575,
    -577,  -577,   577,   574,   579,   580,    56,   581,   583,  -577,
     -13,   585,   587,   588,   589,   590,   592,   611,   117,   184,
     593,   595,   596,   597,  -577,   540,   598,   599,  -577,   600,
     608,   609,   601,    91,  -577,  -577,  -577,   623,   584,   148,
      33,    33,  -577,  -577,   603,  -577,   188,  -577,   591,    -8,
     604,   540,   605,  -577,  -577,  -577,   606,  -577,  -577,   610,
     607,   612,   613,  -577,    56,   615,    56,   616,  -577,   154,
    -577,   594,  -577,   617,   540,   614,   148,  -577,   618,  -577,
     620,  -577,   253,   253,  -577,  -577,    58,   173,  -577,   621,
     204,   540,  -577,   624,   638,   622,   626,  -577,   130,   135,
     239,   627,  -577,   619,  -577,   625,   628,   630,   629,  -577,
    -577,   631,   632,   633,  -577,    59,   634,   635,  -577,   134,
     636,   637,    -8,   641,  -577,   -48,  -577,  -577,  -577,   642,
      43,  -577,  -577,  -577,  -577,   644,   645,   148,   275,  -577,
    -577,   352,   639,  -577,  -577,  -577,   646,   640,   643,   647,
     648,   650,  -577,   672,   540,   540,   264,  -577,   651,   652,
     653,   657,   658,   659,   660,  -577,  -577,  -577,  -577
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
      46,     0,     0,     0,     0,     0,   164,     0,     0,     0,
       0,     0,     0,     0,     0,    79,     0,     0,     0,    42,
       0,     0,    80,     0,     0,     0,     0,    85,     0,     0,
      96,     0,     0,   152,     0,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   210,     0,     0,    28,     0,    39,     0,   199,
     200,   201,   202,   169,   205,   204,     0,     0,     0,     0,
       0,     0,   170,     0,   171,   172,   173,   174,   175,   176,
     177,   179,   168,    41,     0,     0,     0,     0,     0,     0,
       0,     0,   208,     0,     0,    54,     0,     0,     0,     0,
       0,     0,     0,   151,     0,     0,     0,     0,     0,    34,
       0,    33,     0,   181,     0,     0,     0,     0,     0,     0,
       0,   214,     0,   215,     0,     0,     0,    52,     0,    53,
       0,     0,     0,     0,   216,     0,   100,     0,     0,   211,
       0,     0,    29,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,    48,     0,    86,     0,     0,    84,     0,     0,
       0,   153,     0,     0,     0,    27,    36,     0,    32,   196,
     198,   197,     0,     0,     0,     0,     0,     0,    43,    44,
       0,     0,     0,     0,     0,     0,     0,     0,    57,     0,
       0,     0,     0,     0,    98,     0,     0,     0,   155,   212,
       0,     0,     0,   188,     0,     0,     0,     0,   206,   178,
       0,     0,     0,    71,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   104,     0,   102,   105,     0,     0,
       0,   157,     0,   156,     0,    37,   186,     0,   183,   187,
     185,     0,     0,     0,   194,     0,     0,     0,    45,     0,
      82,     0,     0,     0,     0,    47,   209,     0,     0,     0,
      55,     0,     0,    97,    99,     0,     0,     0,     0,     0,
       0,   101,   103,    95,     0,     0,     0,   159,   158,     0,
       0,     0,     0,     0,   192,     0,   207,     0,     0,     0,
       0,     0,     0,     0,     0,    50,    51,     0,     0,    67,
       0,     0,     0,     0,     0,     0,    89,     0,     0,     0,
     113,     0,     0,     0,     0,     0,     0,   163,   162,     0,
       0,     0,     0,   182,   184,     0,     0,     0,     0,   195,
       0,    69,     0,     0,     0,     0,     0,     0,    63,     0,
       0,     0,     0,    58,     0,     0,    90,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   154,    35,     0,     0,   180,   203,    56,     0,
       0,     0,   213,     0,    62,     0,    59,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   117,   118,
     119,     0,   115,     0,   112,     0,     0,     0,   109,     0,
     108,     0,     0,     0,   190,     0,   193,     0,     0,    73,
      76,    70,     0,     0,     0,    64,     0,     0,    60,     0,
       0,     0,    91,    88,     0,     0,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,     0,     0,   110,     0,
       0,     0,     0,     0,     0,     0,    74,    83,     0,     0,
      68,    61,     0,     0,     0,     0,     0,     0,     0,   114,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   191,     0,     0,     0,    65,     0,
       0,     0,     0,     0,   166,    77,    78,     0,     0,     0,
       0,     0,   120,   121,     0,   106,     0,   143,     0,     0,
       0,     0,     0,   160,   161,   189,     0,    72,    66,     0,
       0,     0,     0,   165,     0,     0,     0,     0,   129,     0,
     128,     0,   107,     0,     0,     0,     0,   138,     0,   111,
       0,    94,     0,     0,    87,   167,     0,     0,   123,     0,
       0,     0,   139,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   127,     0,   130,     0,     0,     0,     0,   142,
     140,     0,     0,     0,   124,     0,     0,     0,   141,     0,
       0,     0,     0,     0,   134,     0,   132,   125,   126,     0,
       0,   145,   148,   150,   149,     0,     0,     0,     0,   131,
     133,     0,     0,   146,    92,    93,     0,     0,     0,     0,
       0,     0,   122,     0,     0,     0,     0,   144,     0,     0,
       0,     0,     0,     0,     0,   147,   137,   135,   136
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -577,  -577,  -577,  -577,  -577,   682,  -577,  -577,   662,  -577,
    -577,   -68,  -323,  -577,  -577,  -577,  -577,  -577,  -577,   663,
    -577,  -109,  -577,   187,  -123,  -203,  -254,   118,  -350,  -577,
      68,  -386,   196,   586,  -510,  -522,  -288,  -577,  -577,   655,
    -577,  -577,  -577,  -577,  -577,   262,  -577,  -577,  -577,  -577,
    -577,   347,  -577,  -577,  -577,   168,  -577,  -577,   136,    78,
    -577,  -577,    24,  -397,   167,  -577,  -577,    34,    26,  -577,
    -577,  -577,  -577,   649,   415,  -577,  -516,  -577,  -145,  -577,
    -577,    87,   654,  -576,   121,  -577,   327,  -316,  -577,  -577,
    -577,  -577,  -577,  -577,  -260,   -77,  -577,  -334,  -200,   -82
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    14,    15,    16,    22,    40,    41,    42,
     174,   212,    98,   230,    43,   297,    44,    45,    46,    47,
     105,   112,   281,   282,   137,   217,   311,   320,   399,   477,
     478,   443,   342,   343,   548,   549,   550,    48,    49,   108,
      50,    51,   138,   253,   445,   446,    52,   142,   323,   290,
     365,   366,   367,   452,   531,   532,   492,   572,   603,   659,
     695,   715,   716,   496,   577,   639,   720,   721,   722,    53,
     145,   170,   372,   373,   459,    54,   566,   623,   384,   194,
     269,   377,   378,   335,   545,   425,   385,   195,   196,   197,
     198,   199,   200,   201,   213,   202,   147,   660,   242,   243
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      93,    94,    95,    99,   100,   114,   115,   244,   193,   376,
     304,   567,    97,   393,   394,   564,   379,   575,   121,   157,
     123,   257,   159,   124,   312,   292,   586,   179,   180,   181,
     182,   520,   183,   231,   206,   553,   207,   364,   521,   277,
     522,   278,   586,   397,    92,   152,   106,   658,   438,   184,
      92,   444,   479,   657,   348,    92,   164,   713,   498,   454,
     369,   370,   165,   185,   455,   179,   180,   181,   182,   303,
     183,    99,   450,    92,   371,   497,     5,   364,   204,   205,
     175,    92,   729,   714,   186,    99,   485,   184,   324,   107,
     685,   250,   482,   347,   439,     3,   519,   392,   376,   646,
     539,   185,   440,   576,   451,   379,    92,   293,   210,   186,
      92,   113,   120,   346,   488,   489,   113,   187,    92,    92,
      92,    92,   186,   398,   186,   668,   627,   306,   307,   241,
     241,   518,   557,     6,    92,    96,   279,   433,   188,   189,
     190,   191,   523,   391,   241,   448,   495,   457,   683,   441,
      99,   736,   442,   229,   719,   187,   313,   404,    92,   262,
     192,   211,   688,   689,   713,   696,   586,   586,   135,   211,
     215,   283,    96,   732,   216,     1,   188,   189,   190,   191,
      96,   458,    96,    96,   570,   211,    99,   547,   358,   359,
     714,   383,   241,    99,   637,   296,    96,    92,   192,   143,
     186,   547,   358,   359,    26,    27,   140,   136,   580,   581,
      28,   241,    29,    30,    31,    32,    33,   547,   358,   359,
      96,   653,   547,   358,   359,   333,   141,   638,   749,   750,
     585,     8,   654,    34,    35,    36,   241,    99,   144,    37,
     351,    26,    27,   601,   595,   719,   423,    28,   334,    29,
      30,    31,    32,    33,    99,   380,    38,   430,   439,   693,
     701,   172,   431,   375,   432,   702,   440,   602,   424,   318,
      34,    35,    36,    72,    39,   319,    37,   173,   358,   359,
     241,   694,   568,   469,     4,   679,   528,   528,   529,   529,
     680,   360,   361,    38,   358,   359,   530,   530,   362,   493,
     542,   543,   363,   369,   370,   583,   494,   360,   361,    10,
     737,    39,   738,   691,   362,   640,   416,   417,   363,   663,
     411,    13,   641,   226,   739,   227,   664,    92,    99,     9,
     186,    11,    12,    13,   102,   103,   104,   265,   266,   267,
     547,   358,   359,   315,   380,   316,   321,   330,   322,   331,
     356,    17,   357,   376,   420,   473,   421,   110,   111,   544,
     379,   427,   514,   428,   515,   546,   582,   428,   583,   703,
      18,   421,    19,    20,    24,    23,    25,    55,    56,    57,
      58,    59,    74,    60,    61,    62,    63,    76,    77,    79,
      78,   574,    64,   723,    65,    66,    67,    68,    69,    70,
      80,    71,    81,    82,   723,    75,    84,    83,    85,    87,
      88,    86,    89,    90,    91,    92,    33,   109,   113,   116,
     117,   122,   541,   241,   241,   118,   119,   748,   605,   125,
     723,   126,   127,   128,   153,   129,    99,   131,   614,   666,
     107,   132,   133,   134,   139,   554,   146,   148,   149,   155,
     150,   624,   151,   154,   156,   158,    99,   160,   162,   163,
     167,   166,   161,   168,   169,   571,   171,   135,   177,   176,
     203,   208,   178,   209,   214,   218,    99,   220,   224,   222,
     225,   223,   592,   219,   221,   588,   228,   238,   255,   232,
     252,   256,   233,   268,   234,   289,   280,   235,   236,   294,
     302,   237,   305,   350,   429,   354,   239,   240,   245,   675,
     246,   544,   727,   247,   248,   456,   249,   251,   629,   254,
     310,   259,   260,   628,   397,   261,   258,   263,   270,   341,
     264,   392,   274,   271,   275,   276,   272,   284,   286,   344,
     287,   288,   449,   291,   273,   285,   355,   422,   326,   295,
     298,   299,   300,   301,   453,   308,   309,   328,   314,   317,
     329,   325,   241,   465,   327,   332,   472,   336,   337,   475,
     487,   338,   339,   340,   500,   491,   501,   352,   535,   444,
     556,   368,   589,   555,   609,   381,   349,   382,   386,   353,
     387,   374,   388,   389,   461,   565,   390,   468,   395,   380,
     396,   400,   401,   402,   476,   403,   405,   406,   407,   408,
     409,   410,   431,   413,   414,   617,   415,   495,   419,   426,
     434,   615,   435,   436,   437,   636,   447,   462,   466,   474,
     467,   470,   724,   471,   480,   241,   483,   463,   484,   481,
     499,   490,   504,   724,   502,   526,   503,   506,   507,   508,
     509,   560,   513,   510,   740,   511,   512,   516,   525,   561,
     517,   524,   533,   650,   651,   534,   537,   579,   527,   724,
     536,   538,   540,   616,   551,   552,   558,   559,   655,   562,
     563,   578,   584,   597,   587,   573,   598,   576,   590,   591,
     593,   594,   596,   698,   599,   600,    21,   608,   606,   569,
     613,   610,    73,   611,   612,   618,   619,   486,   604,   661,
     620,   625,   412,   626,   630,   621,   631,   622,   632,   633,
     665,   634,   635,   656,   642,   643,   644,   645,   647,   648,
     649,   652,   681,   662,   667,   669,   672,   670,   186,   730,
     671,   673,   607,   674,   676,   101,   678,   682,   705,   686,
     687,   692,   699,   684,   733,   697,   700,   704,   709,   707,
     708,   130,   706,   690,   717,   718,   725,   726,   710,   711,
     728,   731,   751,   712,   734,   735,   742,   677,   743,   741,
     747,   744,   752,   753,   754,   745,   746,   755,   756,   757,
     758,     0,     0,   505,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     460,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   345,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   418,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   464
};

static const yytype_int16 yycheck[] =
{
      77,    78,    79,    80,    81,    87,    88,   207,   153,   332,
     270,   527,    80,   347,   348,   525,   332,    23,    95,   128,
      97,   221,   131,   100,   278,    55,   548,     9,    10,    11,
      12,    20,    14,   178,   157,    25,   159,   325,    27,   242,
      29,   244,   564,    55,    63,   122,    33,    14,   398,    31,
      63,    95,   438,   629,   314,    63,   138,   105,   455,    36,
     117,   118,   139,    45,    41,     9,    10,    11,    12,   269,
      14,   148,    23,    63,   131,    36,     0,   365,   155,   156,
     148,    63,   130,   131,    66,   162,   130,    31,   288,    76,
     666,   214,   442,    46,    77,   129,   482,    64,   421,   615,
     497,    45,    85,   109,    55,   421,    63,   137,    53,    66,
      63,   124,   131,   313,   448,   449,   124,    99,    63,    63,
      63,    63,    66,   135,    66,   641,   139,   272,   273,   206,
     207,   481,   518,    57,    63,   125,   245,   391,   120,   121,
     122,   123,   131,   346,   221,   405,   107,    43,   664,   132,
     227,   727,   135,   135,   111,    99,   279,   357,    63,   227,
     142,   114,   672,   673,   105,   681,   688,   689,    80,   114,
      77,   248,   125,   130,    81,    56,   120,   121,   122,   123,
     125,    77,   125,   125,   127,   114,   263,    87,    88,    89,
     131,   336,   269,   270,    77,   263,   125,    63,   142,    77,
      66,    87,    88,    89,    61,    62,    77,   119,   542,   543,
      67,   288,    69,    70,    71,    72,    73,    87,    88,    89,
     125,   130,    87,    88,    89,    77,    97,   110,   744,   745,
     130,   130,   141,    90,    91,    92,   313,   314,   116,    96,
     317,    61,    62,   104,   130,   111,    55,    67,   100,    69,
      70,    71,    72,    73,   331,   332,   113,   131,    77,    55,
     130,   130,   136,   331,   138,   130,    85,   128,    77,    77,
      90,    91,    92,   130,   131,    83,    96,   146,    88,    89,
     357,    77,    23,   428,     0,   131,    28,    28,    30,    30,
     136,   101,   102,   113,    88,    89,    38,    38,   108,    28,
     500,   501,   112,   117,   118,   132,    35,   101,   102,     0,
      35,   131,    37,   140,   108,   131,   130,   131,   112,   131,
     130,    60,   138,   130,    49,   132,   138,    63,   405,   129,
      66,    58,    59,    60,    74,    75,    76,     6,     7,     8,
      87,    88,    89,   130,   421,   132,   136,   130,   138,   132,
     130,   130,   132,   676,   130,   432,   132,    77,    78,   504,
     676,   130,   130,   132,   132,   130,   130,   132,   132,   130,
     129,   132,   129,   129,    50,   130,    50,   130,   130,    18,
       9,   129,   131,   129,   129,   129,   129,    34,    47,    32,
      48,   536,   129,   709,   129,   129,   129,   129,   129,   129,
      24,   129,    22,    30,   720,   131,    40,    33,    49,    42,
      26,    36,   130,   130,    11,    63,    73,    73,   124,   130,
     129,   129,   499,   500,   501,    55,    68,   743,   573,   130,
     746,   129,   129,   129,   138,   130,   513,   129,   583,   639,
      76,   130,   129,   129,   129,   513,    64,   129,   129,    21,
     130,   596,   131,   131,    20,   130,   533,    33,   129,   129,
      98,   129,   139,   129,   115,   533,   129,    80,   130,   136,
     130,    76,   133,    55,    19,    15,   553,   129,    13,   129,
      64,   130,   559,   138,   138,   553,    55,   131,    55,   129,
      93,   136,   129,   139,   129,    55,    79,   129,   129,    13,
      55,   129,    55,   316,   386,    55,   129,   129,   129,   654,
     129,   656,   712,   130,   129,    21,   130,   129,   600,   130,
      82,   130,   129,   600,    55,   131,   136,   129,   129,    86,
     134,    64,   130,   133,   130,   130,   144,   130,   129,    86,
     130,   129,    44,   130,   143,   139,    94,   139,   137,   130,
     130,   130,   130,   130,    23,   130,   130,   130,   129,   145,
     130,   129,   639,    55,   129,   129,    55,   130,   129,    13,
      55,   130,   130,   129,    26,   103,    42,   129,    35,    95,
       4,   130,    13,   515,    14,   129,   140,   129,   129,   140,
     129,   137,   130,   129,   137,    55,   130,   134,   130,   676,
     130,   130,   129,   129,    84,   130,   129,   129,   129,   129,
     129,   129,   136,   130,   129,    13,   129,   107,   130,   130,
     129,    39,   130,   130,   129,    14,   136,   131,   129,   433,
     130,   130,   709,   130,   129,   712,   130,   140,   129,   135,
     138,   129,   129,   720,   130,   138,   130,   130,   130,   130,
     129,    94,   129,   131,   731,   130,   130,   130,   129,    94,
     132,   137,   129,    55,    55,   130,   129,   129,   138,   746,
     138,   130,   130,   137,   130,   130,   130,   138,    55,   130,
     130,   130,   129,   131,   130,   138,   131,   109,   130,   130,
     129,   129,   129,    55,   130,   129,    14,   129,   131,   531,
     140,   130,    40,   131,   131,   130,   129,   445,   572,   631,
     136,   130,   365,   130,   129,   136,   129,   137,   130,   130,
     129,   131,   130,   139,   131,   130,   130,   130,   130,   130,
     130,   130,   138,   130,   130,   130,   129,   131,    66,   715,
     130,   129,   575,   130,   129,    82,   130,   130,   129,   131,
     130,   130,   130,   139,   720,   131,   130,   130,   129,   131,
     130,   106,   137,   676,   130,   130,   130,   130,   137,   137,
     129,   129,   746,   140,   130,   130,   130,   656,   138,   140,
     130,   138,   131,   131,   131,   138,   138,   130,   130,   130,
     130,    -1,    -1,   466,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     415,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   312,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   372,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   421
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    56,   148,   129,     0,     0,    57,   149,   130,   129,
       0,    58,    59,    60,   150,   151,   152,   130,   129,   129,
     129,   152,   153,   130,    50,    50,    61,    62,    67,    69,
      70,    71,    72,    73,    90,    91,    92,    96,   113,   131,
     154,   155,   156,   161,   163,   164,   165,   166,   184,   185,
     187,   188,   193,   216,   222,   130,   130,    18,     9,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   130,   155,   131,   131,    34,    47,    48,    32,
      24,    22,    30,    33,    40,    49,    36,    42,    26,   130,
     130,    11,    63,   242,   242,   242,   125,   158,   159,   242,
     242,   166,    74,    75,    76,   167,    33,    76,   186,    73,
      77,    78,   168,   124,   246,   246,   130,   129,    55,    68,
     131,   242,   129,   242,   242,   130,   129,   129,   129,   130,
     186,   129,   130,   129,   129,    80,   119,   171,   189,   129,
      77,    97,   194,    77,   116,   217,    64,   243,   129,   129,
     130,   131,   242,   138,   131,    21,    20,   168,   130,   168,
      33,   139,   129,   129,   246,   242,   129,    98,   129,   115,
     218,   129,   130,   146,   157,   158,   136,   130,   133,     9,
      10,    11,    12,    14,    31,    45,    66,    99,   120,   121,
     122,   123,   142,   225,   226,   234,   235,   236,   237,   238,
     239,   240,   242,   130,   242,   242,   171,   171,    76,    55,
      53,   114,   158,   241,    19,    77,    81,   172,    15,   138,
     129,   138,   129,   130,    13,    64,   130,   132,    55,   135,
     160,   225,   129,   129,   129,   129,   129,   129,   131,   129,
     129,   242,   245,   246,   245,   129,   129,   130,   129,   130,
     171,   129,    93,   190,   130,    55,   136,   245,   136,   130,
     129,   131,   158,   129,   134,     6,     7,     8,   139,   227,
     129,   133,   144,   143,   130,   130,   130,   172,   172,   168,
      79,   169,   170,   242,   130,   139,   129,   130,   129,    55,
     196,   130,    55,   137,    13,   130,   158,   162,   130,   130,
     130,   130,    55,   245,   241,    55,   225,   225,   130,   130,
      82,   173,   173,   171,   129,   130,   132,   145,    77,    83,
     174,   136,   138,   195,   245,   129,   137,   129,   130,   130,
     130,   132,   129,    77,   100,   230,   130,   129,   130,   130,
     129,    86,   179,   180,    86,   180,   245,    46,   241,   140,
     170,   242,   129,   140,    55,    94,   130,   132,    88,    89,
     101,   102,   108,   112,   183,   197,   198,   199,   130,   117,
     118,   131,   219,   220,   137,   158,   159,   228,   229,   234,
     242,   129,   129,   225,   225,   233,   129,   129,   130,   129,
     130,   172,    64,   244,   244,   130,   130,    55,   135,   175,
     130,   129,   129,   130,   245,   129,   129,   129,   129,   129,
     129,   130,   198,   130,   129,   129,   130,   131,   220,   130,
     130,   132,   139,    55,    77,   232,   130,   130,   132,   174,
     131,   136,   138,   173,   129,   130,   130,   129,   175,    77,
      85,   132,   135,   178,    95,   191,   192,   136,   241,    44,
      23,    55,   200,    23,    36,    41,    21,    43,    77,   221,
     221,   137,   131,   140,   229,    55,   129,   130,   134,   225,
     130,   130,    55,   242,   179,    13,    84,   176,   177,   178,
     129,   135,   175,   130,   129,   130,   192,    55,   244,   244,
     129,   103,   203,    28,    35,   107,   210,    36,   210,   138,
      26,    42,   130,   130,   129,   233,   130,   130,   130,   129,
     131,   130,   130,   129,   130,   132,   130,   132,   175,   178,
      20,    27,    29,   131,   137,   129,   138,   138,    28,    30,
      38,   201,   202,   129,   130,    35,   138,   129,   130,   210,
     130,   242,   245,   245,   225,   231,   130,    87,   181,   182,
     183,   130,   130,    25,   158,   177,     4,   178,   130,   138,
      94,    94,   130,   130,   181,    55,   223,   223,    23,   202,
     127,   158,   204,   138,   225,    23,   109,   211,   130,   129,
     244,   244,   130,   132,   129,   130,   182,   130,   158,    13,
     130,   130,   242,   129,   129,   130,   129,   131,   131,   130,
     129,   104,   128,   205,   205,   225,   131,   211,   129,    14,
     130,   131,   131,   140,   225,    39,   137,    13,   130,   129,
     136,   136,   137,   224,   225,   130,   130,   139,   242,   246,
     129,   129,   130,   130,   131,   130,    14,    77,   110,   212,
     131,   138,   131,   130,   130,   130,   223,   130,   130,   130,
      55,    55,   130,   130,   141,    55,   139,   230,    14,   206,
     244,   206,   130,   131,   138,   129,   245,   130,   223,   130,
     131,   130,   129,   129,   130,   225,   129,   231,   130,   131,
     136,   138,   130,   223,   139,   230,   131,   130,   181,   181,
     228,   140,   130,    55,    77,   207,   223,   131,    55,   130,
     130,   130,   130,   130,   130,   129,   137,   131,   130,   129,
     137,   137,   140,   105,   131,   208,   209,   130,   130,   111,
     213,   214,   215,   234,   242,   130,   130,   245,   129,   130,
     209,   129,   130,   214,   130,   130,   230,    35,    37,    49,
     242,   140,   130,   138,   138,   138,   138,   130,   234,   223,
     223,   215,   131,   131,   131,   130,   130,   130,   130
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   147,   148,   148,   149,   149,   149,   150,   151,   152,
     153,   154,   154,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   156,   157,   157,
     158,   158,   159,   160,   160,   161,   162,   162,   163,   163,
     164,   165,   166,   167,   167,   167,   168,   168,   169,   169,
     170,   170,   171,   171,   172,   172,   173,   174,   174,   174,
     174,   174,   175,   176,   176,   177,   177,   178,   178,   179,
     179,   179,   180,   181,   181,   182,   182,   183,   183,   184,
     185,   185,   186,   187,   188,   189,   189,   190,   190,   191,
     191,   192,   192,   192,   192,   193,   194,   194,   195,   195,
     196,   196,   197,   197,   198,   198,   198,   198,   198,   198,
     198,   198,   199,   200,   200,   201,   201,   202,   202,   202,
     203,   203,   204,   204,   204,   205,   205,   205,   206,   206,
     207,   207,   208,   208,   209,   209,   209,   209,   210,   210,
     210,   210,   211,   212,   212,   213,   213,   214,   214,   215,
     215,   216,   217,   217,   218,   218,   219,   219,   219,   219,
     220,   220,   221,   221,   222,   223,   224,   224,   225,   225,
     225,   225,   225,   225,   225,   225,   225,   225,   225,   225,
     226,   227,   227,   228,   228,   229,   229,   229,   230,   230,
     231,   231,   232,   232,   233,   233,   234,   234,   234,   235,
     235,   236,   236,   237,   238,   238,   239,   240,   241,   241,
     242,   243,   243,   244,   245,   245,   246
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
       0,     4,     1,     2,     1,     1,     8,     9,     5,     5,
       6,     9,     5,     1,     5,     1,     2,     1,     1,     1,
       5,     5,    11,     5,     7,     7,     7,     5,     1,     1,
       1,     4,     1,     2,     1,     7,     7,     7,     6,     7,
       8,     9,     6,     1,     9,     1,     2,     6,     1,     1,
       1,     7,     1,     5,     9,     5,     1,     1,     2,     2,
       8,     8,     1,     1,     5,     4,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     5,     1,
       9,     0,     6,     1,     3,     1,     1,     1,     1,     9,
       1,     3,     1,     4,     1,     3,     4,     4,     4,     1,
       1,     1,     1,     9,     1,     1,     5,     7,     1,     6,
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
#line 256 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = new AST ( (yyvsp[-1].tok) ); }
#line 1961 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 257 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { p_ast = (yyvsp[-2].node); }
#line 1967 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 261 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 1973 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 262 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 1979 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 263 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); dynamic_cast < AST_Schema* > ( (yyval.node) ) -> SetVersion ( (yyvsp[-2].tok) -> GetValue () ); }
#line 1985 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 267 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-5].tok); }
#line 1991 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 271 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.tok) = (yyvsp[-2].tok); }
#line 1997 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 275 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Schema ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2003 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 279 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Schema (); }
#line 2009 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 284 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2015 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 285 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2021 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 291 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2027 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2033 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 293 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2039 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 294 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2045 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 295 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2051 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 296 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2057 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2063 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 298 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2069 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 299 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2075 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 300 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2081 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 301 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2087 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 302 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2093 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 303 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2099 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 304 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2105 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 309 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeDef ( (yyvsp[-9].tok), (yyvsp[-6].fqn), (yyvsp[-3].node) ); }
#line 2111 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2117 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 314 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2123 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 318 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2129 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 319 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2135 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
#line 2141 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 327 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2147 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 328 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2153 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 333 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TypeSet ( (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-5].node) ); }
#line 2159 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 337 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2165 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 338 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2171 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 342 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-5].tok), (yyvsp[-2].fqn), 0 ); }
#line 2177 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 343 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FmtDef ( (yyvsp[-6].tok), (yyvsp[-2].fqn), (yyvsp[-3].fqn) ); }
#line 2183 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 347 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . ConstDef ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2189 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 351 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . AliasDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2195 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 355 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2201 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 359 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . UntypedFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2207 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 360 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . RowlenFunctionDecl ( (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
#line 2213 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 362 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), false, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].paramSig), (yyvsp[-2].paramSig), (yyvsp[-1].node) ); }
#line 2219 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 366 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2225 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2231 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 371 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2237 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2243 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 376 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].fqn) ); }
#line 2249 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 377 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].fqn) ); }
#line 2255 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 381 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2261 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 382 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2267 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 386 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[0].tok), 0, 0, false); }
#line 2273 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 387 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = (yyvsp[-2].paramSig); }
#line 2279 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 391 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = (yyvsp[-2].paramSig); }
#line 2285 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 395 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[0].tok), 0, 0, false); }
#line 2291 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 396 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-4].tok), (yyvsp[-2].node), 0, (yyvsp[-1].boolean) ); }
#line 2297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 397 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-5].tok), 0, (yyvsp[-2].node), (yyvsp[-1].boolean) ); }
#line 2303 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 398 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].boolean) ); }
#line 2309 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 399 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.paramSig) = new AST_ParamSig ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-1].boolean) ); }
#line 2315 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 403 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 407 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2327 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 408 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2333 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 412 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Formal ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].tok), false ); }
#line 2339 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 413 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST_Formal ( (yyvsp[-5].tok), (yyvsp[-2].node), (yyvsp[-1].tok), true ); }
#line 2345 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 417 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.boolean) = false; }
#line 2351 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 418 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.boolean) = true; }
#line 2357 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 422 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2363 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 423 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].fqn); }
#line 2369 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 428 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2375 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 432 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2381 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 433 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2387 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 437 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-2].expr) ); }
#line 2393 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 438 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2399 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 442 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2405 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 443 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
#line 2411 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 447 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2417 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 450 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2423 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 451 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2429 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 456 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-8].tok), true, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].paramSig), (yyvsp[-2].paramSig), (yyvsp[-1].node) ); }
#line 2435 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . FunctionDecl ( (yyvsp[-16].tok), false, (yyvsp[-8].node), (yyvsp[-7].node), (yyvsp[-6].fqn), (yyvsp[-5].paramSig), (yyvsp[-4].paramSig), (yyvsp[-3].node) ); }
#line 2441 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 470 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . PhysicalDecl ( (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].fqn), (yyvsp[-2].paramSig), (yyvsp[-1].node) ); }
#line 2447 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 474 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2453 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 475 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-1].node) ); }
#line 2459 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 479 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-9].tok), (yyvsp[-4].node) ); }
#line 2465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 481 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2471 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 485 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2477 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 486 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2483 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 491 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2489 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 493 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ) ; }
#line 2495 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 495 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-4].node) ); }
#line 2501 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 497 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn) ); }
#line 2507 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 504 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . TableDef ( (yyvsp[-11].tok), (yyvsp[-8].fqn), (yyvsp[-7].node), (yyvsp[-3].node) ); }
#line 2513 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 508 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2519 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 509 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2525 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 513 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2531 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 514 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].fqn) ); }
#line 2537 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 518 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 2543 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 519 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2549 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 523 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2555 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 524 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2561 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 528 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2567 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 529 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2573 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 530 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-2].expr) ); }
#line 2579 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 531 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-2].expr) ); }
#line 2585 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 532 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }
#line 2591 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 533 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-2].tok), (yyvsp[-1].node) ); }
#line 2597 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 534 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-3].tok), (yyvsp[-1].node) ); }
#line 2603 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 535 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-4].fqn) ); }
#line 2609 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 539 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2615 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 543 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2621 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 544 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 2627 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 548 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2633 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 549 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2639 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 553 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2645 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 554 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2651 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 555 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2657 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 559 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2663 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 560 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2669 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 565 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-10].tok), (yyvsp[-5].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2675 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 566 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 567 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
#line 2687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 571 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node) ); }
#line 2693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 572 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
#line 2699 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 573 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    {  (yyval.node) = new AST ( (yyvsp[-4].tok), (yyvsp[-2].node) ); }
#line 2705 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 577 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2711 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 578 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2717 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 582 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2723 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 583 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2729 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 587 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2735 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 588 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 592 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2747 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 593 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2753 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 594 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2759 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 595 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2765 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 600 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }
#line 2771 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 602 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-6].tok), (yyvsp[-3].node), new AST ( (yyvsp[-2].tok) ) ); }
#line 2777 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 604 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2783 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 606 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-8].tok), (yyvsp[-5].node), new AST ( (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
#line 2789 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 610 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
#line 2795 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 614 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2801 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 615 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2807 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 619 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2813 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 620 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2819 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 624 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].node) ); }
#line 2825 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 625 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 2831 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 629 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 2837 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 630 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 2843 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 635 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . DatabaseDef ( (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
#line 2849 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 639 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2855 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 640 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].fqn); }
#line 2861 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 643 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 2867 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 644 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-4].tok) ); }
#line 2873 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 648 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2879 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 649 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); }
#line 2885 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 650 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 2891 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 651 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 2897 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 656 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2903 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 658 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
#line 2909 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 662 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2915 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 663 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 2921 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 669 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = p_builder . Include ( (yyvsp[-4].tok), (yyvsp[-1].tok) ); }
#line 2927 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 675 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2933 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 679 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].expr) ); }
#line 2939 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 680 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( (yyvsp[0].expr) ); }
#line 2945 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 684 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].fqn) ); }
#line 2951 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 685 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 2957 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 686 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( PT_AT ); }
#line 2963 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2969 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 688 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2975 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 689 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2981 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 690 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2987 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 691 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2993 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2999 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3005 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 694 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 3011 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 695 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 3017 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 700 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( (yyvsp[-5].fqn) ); (yyval.expr) -> AddNode ( (yyvsp[-4].node) ); (yyval.expr) -> AddNode ( (yyvsp[-2].node) ); }
#line 3023 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 704 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( PT_EMPTY ); }
#line 3029 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 705 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); }
#line 3035 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 708 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3041 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 709 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].node) ); }
#line 3047 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 713 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].fqn); }
#line 3053 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 714 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3059 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 715 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].expr); }
#line 3065 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 719 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3071 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 720 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-3].node); }
#line 3077 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 724 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3083 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 725 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3089 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 729 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[0].tok) ); }
#line 3095 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 730 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-1].node); }
#line 3101 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 734 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST (); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3107 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 735 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( (yyvsp[0].expr) ); }
#line 3113 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 739 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3119 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 740 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3125 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3131 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 745 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3137 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 746 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3143 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 750 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3149 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 751 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3155 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 755 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); }
#line 3161 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 759 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3167 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 760 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[0].tok) ); }
#line 3173 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 764 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-4].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }
#line 3179 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 767 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.expr) = new AST_Expr ( (yyvsp[-6].tok) ); (yyval.expr) -> AddNode ( (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( (yyvsp[-1].expr) ); }
#line 3185 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 771 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = (yyvsp[0].node); }
#line 3191 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 772 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.node) = new AST ( (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }
#line 3197 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 778 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-1].fqn); }
#line 3203 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 782 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3209 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 783 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3215 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 787 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = new AST_FQN ( (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( (yyvsp[-1].tok) ); }
#line 3221 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 791 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 3227 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 792 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[0].fqn); }
#line 3233 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 796 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1646  */
    { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }
#line 3239 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
    break;


#line 3243 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.cpp" /* yacc.c:1646  */
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
