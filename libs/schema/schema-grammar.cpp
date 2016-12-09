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

    static
    ParseTree*
    P ( YYSTYPE & p_prod )
    {
        assert ( p_prod . subtree );
        return ( ParseTree * ) p_prod . subtree;
    }

    static
    ParseTree*
    T ( YYSTYPE & p_term )
    {
        assert ( p_term . subtree == 0 );
        return new ParseTree ( p_term );
    }

    /* Create production node */
    static
    ParseTree *
    MakeTree ( int p_token,
               ParseTree * p_ch1 = 0,
               ParseTree * p_ch2 = 0,
               ParseTree * p_ch3 = 0,
               ParseTree * p_ch4 = 0,
               ParseTree * p_ch5 = 0,
               ParseTree * p_ch6 = 0,
               ParseTree * p_ch7 = 0,
               ParseTree * p_ch8 = 0
             )
    {
        SchemaToken v = { p_token, NULL, 0, NULL, NULL };
        ParseTree * ret = new ParseTree ( v );
        if ( p_ch1 != 0 ) ret -> AddChild ( p_ch1 );
        if ( p_ch2 != 0 ) ret -> AddChild ( p_ch2 );
        if ( p_ch3 != 0 ) ret -> AddChild ( p_ch3 );
        if ( p_ch4 != 0 ) ret -> AddChild ( p_ch4 );
        if ( p_ch5 != 0 ) ret -> AddChild ( p_ch5 );
        if ( p_ch6 != 0 ) ret -> AddChild ( p_ch6 );
        if ( p_ch7 != 0 ) ret -> AddChild ( p_ch7 );
        if ( p_ch8 != 0 ) ret -> AddChild ( p_ch8 );
        return ret;
    }

    /* Add to a flat list node */
    static
    ParseTree *
    AddToList ( ParseTree * p_root, ParseTree * p_br1 = 0, ParseTree * p_br2 = 0 )
    {
        if ( p_br1 != 0 )
        {
            p_root -> AddChild ( p_br1 );
        }
        if ( p_br2 != 0 )
        {
            p_root -> AddChild ( p_br2 );
        }
        return p_root;
    }



#line 157 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:339  */

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
    PT_VERSION_2 = 313,
    PT_SCHEMA_1_0 = 314,
    PT_INCLUDE = 315,
    PT_TYPEDEF = 316,
    PT_FQN = 317,
    PT_IDENT = 318,
    PT_DIM = 319,
    PT_UINT = 320,
    PT_TYPESET = 321,
    PT_TYPESETDEF = 322,
    PT_FORMAT = 323,
    PT_CONST = 324,
    PT_ALIAS = 325,
    PT_EXTERN = 326,
    PT_FUNCTION = 327,
    PT_UNTYPED = 328,
    PT_ROWLENGTH = 329,
    PT_FUNCDECL = 330,
    PT_EMPTY = 331,
    PT_SCHEMASIG = 332,
    PT_SCHEMAFORMAL = 333,
    PT_RETURNTYPE = 334,
    PT_FACTSIG = 335,
    PT_FUNCSIG = 336,
    PT_FUNCPARAMS = 337,
    PT_FORMALPARAM = 338,
    PT_ELLIPSIS = 339,
    PT_FUNCPROLOGUE = 340,
    PT_RETURN = 341,
    PT_PRODSTMT = 342,
    PT_ASSIGN = 343,
    PT_SCHEMA = 344,
    PT_VALIDATE = 345,
    PT_PHYSICAL = 346,
    PT_PHYSPROLOGUE = 347,
    PT_PHYSSTMT = 348,
    PT_PHYSBODYSTMT = 349,
    PT_TABLE = 350,
    PT_TABLEPARENTS = 351,
    PT_TABLEBODY = 352,
    PT_FUNCEXPR = 353,
    PT_FACTPARMS = 354,
    PT_COLUMN = 355,
    PT_COLDECL = 356,
    PT_TYPEDCOL = 357,
    PT_COLMODIFIER = 358,
    PT_COLSTMT = 359,
    PT_CONDEXPR = 360,
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
    PT_DATABASEMEMBER = 372,
    PT_DBMEMBER = 373,
    PT_TABLEMEMBER = 374,
    PT_TBLMEMBER = 375,
    PT_NOHEADER = 376,
    PT_CASTEXPR = 377,
    PT_CONSTVECT = 378,
    PT_NEGATE = 379,
    PT_UNARYPLUS = 380,
    PT_FACTPARMNAMED = 381,
    PT_VERSNAME = 382
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

#line 350 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   657

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  147
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  113
/* YYNRULES -- Number of rules.  */
#define YYNRULES  245
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  464

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   382

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,   129,     2,     2,     2,
     134,   135,   138,   141,   130,   144,     2,   145,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   146,   128,
     136,   133,   137,     2,   140,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   142,     2,   143,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   131,   139,   132,     2,     2,     2,     2,
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
     125,   126,   127
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   265,   265,   266,   270,   271,   272,   276,   280,   284,
     290,   294,   295,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   316,   318,   323,
     328,   334,   342,   343,   347,   348,   355,   360,   364,   371,
     372,   376,   377,   381,   388,   390,   395,   399,   406,   408,
     413,   420,   425,   432,   437,   444,   448,   449,   450,   460,
     465,   470,   471,   475,   479,   480,   484,   485,   489,   490,
     494,   495,   499,   503,   507,   508,   510,   512,   514,   519,
     520,   524,   529,   530,   534,   535,   539,   541,   543,   548,
     549,   553,   554,   558,   560,   567,   577,   578,   582,   584,
     589,   590,   594,   596,   598,   600,   605,   612,   620,   621,
     625,   626,   630,   632,   637,   638,   642,   643,   644,   645,
     646,   647,   648,   649,   653,   655,   657,   659,   664,   665,
     669,   670,   674,   675,   676,   680,   682,   684,   686,   688,
     693,   695,   697,   702,   703,   707,   708,   709,   710,   714,
     719,   721,   723,   725,   730,   735,   736,   740,   741,   745,
     746,   750,   751,   758,   759,   763,   764,   768,   769,   770,
     771,   772,   773,   774,   775,   776,   777,   781,   785,   794,
     803,   804,   808,   809,   813,   814,   818,   819,   823,   824,
     828,   829,   833,   834,   838,   839,   840,   844,   845,   849,
     850,   854,   858,   859,   863,   864,   868,   869,   873,   874,
     875,   879,   880,   881,   887,   895,   896,   900,   901,   905,
     906,   910,   911,   912,   916,   917,   921,   926,   933,   934,
     936,   937,   938,   939,   940,   941,   942,   943,   947,   948,
     952,   953,   957,   961,   965,   966
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
  "PT_SOURCE", "PT_VERSION_1_0", "PT_VERSION_2", "PT_SCHEMA_1_0",
  "PT_INCLUDE", "PT_TYPEDEF", "PT_FQN", "PT_IDENT", "PT_DIM", "PT_UINT",
  "PT_TYPESET", "PT_TYPESETDEF", "PT_FORMAT", "PT_CONST", "PT_ALIAS",
  "PT_EXTERN", "PT_FUNCTION", "PT_UNTYPED", "PT_ROWLENGTH", "PT_FUNCDECL",
  "PT_EMPTY", "PT_SCHEMASIG", "PT_SCHEMAFORMAL", "PT_RETURNTYPE",
  "PT_FACTSIG", "PT_FUNCSIG", "PT_FUNCPARAMS", "PT_FORMALPARAM",
  "PT_ELLIPSIS", "PT_FUNCPROLOGUE", "PT_RETURN", "PT_PRODSTMT",
  "PT_ASSIGN", "PT_SCHEMA", "PT_VALIDATE", "PT_PHYSICAL",
  "PT_PHYSPROLOGUE", "PT_PHYSSTMT", "PT_PHYSBODYSTMT", "PT_TABLE",
  "PT_TABLEPARENTS", "PT_TABLEBODY", "PT_FUNCEXPR", "PT_FACTPARMS",
  "PT_COLUMN", "PT_COLDECL", "PT_TYPEDCOL", "PT_COLMODIFIER", "PT_COLSTMT",
  "PT_CONDEXPR", "PT_DFLTVIEW", "PT_PHYSMBR", "PT_PHYSCOL",
  "PT_PHYSCOLDEF", "PT_COLSCHEMAPARMS", "PT_COLSCHEMAPARAM",
  "PT_COLUNTYPED", "PT_DATABASE", "PT_TYPEEXPR", "PT_DBBODY", "PT_DBDAD",
  "PT_DATABASEMEMBER", "PT_DBMEMBER", "PT_TABLEMEMBER", "PT_TBLMEMBER",
  "PT_NOHEADER", "PT_CASTEXPR", "PT_CONSTVECT", "PT_NEGATE",
  "PT_UNARYPLUS", "PT_FACTPARMNAMED", "PT_VERSNAME", "';'", "'$'", "','",
  "'{'", "'}'", "'='", "'('", "')'", "'<'", "'>'", "'*'", "'|'", "'@'",
  "'+'", "'['", "']'", "'-'", "'/'", "':'", "$accept", "parse", "source",
  "version_1_0", "version_2_x", "schema_2_x", "schema_1_0",
  "schema_1_0_decl_seq", "schema_1_0_decl", "script_1_0_decl",
  "validate_1_0_decl", "include_directive", "typedef_1_0_decl",
  "typedef_1_0_new_name_list", "typedef_1_0_new_name", "typeset_1_0_decl",
  "typeset_1_0_new_name", "typeset_1_0_def", "typespec_1_0_list",
  "typespec_1_0", "typespec_1_0_name", "format_1_0_decl",
  "format_1_0_new_name", "format_1_0_name", "const_1_0_decl",
  "const_1_0_new_name", "alias_1_0_decl", "alias_1_0_new_name",
  "extern_1_0_decl", "ext_func_1_0_decl", "function_1_0_decl",
  "func_1_0_decl", "untyped_func_1_0_decl", "row_length_func_1_0_decl",
  "opt_func_1_0_schema_sig", "func_1_0_schema_sig",
  "func_1_0_schema_formals", "func_1_0_schema_formal",
  "func_1_0_return_type", "opt_func_1_0_fact_sig", "func_1_0_fact_sig",
  "func_1_0_param_sig", "func_1_0_param_signature",
  "func_1_0_formal_params", "formal_param_1_0", "opt_control_1_0",
  "func_1_0_vararg_formals", "func_1_0_prologue", "script_1_0_stmt_seq",
  "script_1_0_stmt", "production_1_0_stmt", "physical_1_0_decl",
  "phys_1_0_return_type", "phys_1_0_prologue", "phys_1_0_body",
  "phys_1_0_body_stmt", "phys_1_0_stmt", "table_1_0_decl",
  "opt_tbl_1_0_parents", "tbl_1_0_parents", "tbl_1_0_body",
  "tbl_1_0_stmt_seq", "tbl_1_0_stmt", "column_1_0_decl",
  "opt_col_1_0_modifier_seq", "col_1_0_modifier_seq", "col_1_0_modifier",
  "col_1_0_decl", "typed_column_decl_1_0", "column_body_1_0",
  "column_stmt_1_0", "default_view_1_0_decl", "physmbr_1_0_decl",
  "phys_coldef_1_0", "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
  "col_schema_parm_1_0", "col_schema_value_1_0", "cond_expr_1_0",
  "expression_1_0", "primary_expr_1_0", "assign_expr_1_0", "func_expr_1_0",
  "schema_parms_1_0", "schema_parm_1_0", "opt_factory_parms_1_0",
  "factory_parms_1_0", "factory_parm_1_0", "opt_func_1_0_parms",
  "func_1_0_parms", "uint_expr_1_0", "float_expr_1_0", "string_expr_1_0",
  "const_vect_expr_1_0", "opt_const_vect_exprlist_1_0",
  "const_vect_exprlist_1_0", "bool_expr_1_0", "negate_expr_1_0",
  "type_expr_1_0", "database_1_0_decl", "opt_database_dad_1_0",
  "database_body_1_0", "database_members_1_0", "database_member_1_0",
  "opt_template_1_0", "db_member_1_0", "table_member_1_0", "fqn_1_0",
  "ident_1_0", "dim_1_0", "empty", "fqn_vers", "fqn_opt_vers", YY_NULLPTR
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
     375,   376,   377,   378,   379,   380,   381,   382,    59,    36,
      44,   123,   125,    61,    40,    41,    60,    62,    42,   124,
      64,    43,    91,    93,    45,    47,    58
};
# endif

#define YYPACT_NINF -275

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-275)))

#define YYTABLE_NINF -245

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     426,  -275,   383,   383,   383,   103,   383,    21,     3,    22,
      64,   383,   383,   383,   103,   251,  -275,   192,   199,   529,
      89,  -275,   529,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,    16,
    -275,    -9,    -8,   125,  -275,  -275,   144,   383,   -17,   383,
     383,   182,  -275,  -275,  -275,   198,  -275,  -275,  -275,   194,
      21,  -275,   148,    16,   159,   153,  -275,   168,   215,  -275,
    -275,  -275,  -275,  -275,  -275,   553,   232,   153,   296,   193,
     153,   383,  -275,   383,   189,  -275,  -275,   243,   153,    -8,
    -275,   239,   246,   383,    45,  -275,   383,   236,  -275,   383,
    -275,   198,  -275,   383,  -275,   383,   258,  -275,   150,  -275,
     207,   383,   259,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,   383,   280,   203,  -275,   355,   355,
     305,   248,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
       9,   257,   355,   266,  -275,    52,  -275,  -275,   271,   272,
    -275,   182,  -275,  -275,   383,  -275,   273,  -275,   273,   278,
    -275,   464,  -275,  -275,   383,  -275,   205,  -275,   269,   153,
    -275,   277,   138,  -275,  -275,  -275,  -275,  -275,  -275,   281,
     293,  -275,  -275,  -275,   153,  -275,   408,   294,  -275,   299,
     355,  -275,  -275,  -275,    59,  -275,   162,  -275,  -275,  -275,
    -275,   153,   -19,   295,  -275,  -275,   224,   383,   298,   382,
    -275,    -2,  -275,    40,   422,  -275,  -275,  -275,   490,  -275,
    -275,   419,   218,  -275,  -275,   430,  -275,  -275,   383,  -275,
    -275,   355,   280,   383,  -275,   355,   311,  -275,   147,  -275,
     355,  -275,   317,  -275,  -275,   383,   383,  -275,  -275,  -275,
     424,   310,    85,  -275,   383,   391,   -19,   223,    11,   320,
    -275,  -275,   383,   443,   319,   280,  -275,   447,   383,  -275,
      -2,  -275,   324,  -275,  -275,    36,  -275,  -275,   324,  -275,
    -275,  -275,   257,  -275,   355,   408,  -275,  -275,   328,   334,
    -275,  -275,   452,   454,   339,  -275,  -275,     8,   424,  -275,
    -275,   457,   336,  -275,   213,   383,  -275,   346,   320,   320,
    -275,    74,  -275,   213,  -275,   -25,   344,   466,   161,  -275,
    -275,  -275,   -29,   109,   257,  -275,   355,   353,    38,   349,
     347,   280,   383,  -275,    20,   257,   356,   352,  -275,  -275,
    -275,   355,   359,   360,    95,  -275,  -275,   424,  -275,   339,
    -275,  -275,   355,    47,  -275,  -275,   -15,   383,  -275,  -275,
    -275,  -275,    54,   358,  -275,   200,   280,  -275,   280,  -275,
     355,  -275,   351,  -275,  -275,   365,  -275,   355,   280,   186,
    -275,   244,   408,   383,  -275,   355,  -275,  -275,  -275,   339,
    -275,   -13,  -275,  -275,  -275,    31,  -275,   373,  -275,   355,
    -275,  -275,   153,    62,   355,   355,   378,   187,   383,  -275,
     185,   355,   195,  -275,   372,  -275,  -275,   374,  -275,    97,
    -275,  -275,   384,  -275,   383,   257,   377,   380,   381,   212,
    -275,  -275,   105,   383,  -275,  -275,  -275,  -275,   257,   383,
     395,   355,   355,   185,  -275,  -275,  -275,   383,  -275,  -275,
     351,   351,  -275,  -275
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   242,     0,   242,
     242,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,     4,    10,    11,    20,    21,    25,    13,    14,    15,
      16,    17,    18,    19,    22,    23,    24,   238,   239,     0,
     228,     0,     0,   242,    53,    54,     0,     0,    47,     0,
       0,     0,    55,    56,    57,     0,    62,    61,    30,     0,
     242,    27,   242,     0,     0,    37,    29,     0,     0,     1,
       3,     5,     9,     6,    12,     0,     0,    52,     0,     0,
      50,     0,   243,     0,     0,   215,    44,     0,    46,   244,
     245,     0,     0,     0,     0,    64,     0,   211,    68,     0,
      69,     0,    96,     0,    28,     0,     0,   108,     0,    32,
      34,     0,     0,     8,     7,   230,   231,   232,   233,   234,
     235,   236,   237,   229,    51,   194,   196,   195,   197,   198,
     199,   200,   207,   206,     0,     0,     0,   168,     0,   242,
       0,     0,   165,   169,   170,   171,   172,   173,   174,   175,
     167,   242,     0,     0,   216,   242,   214,    45,     0,     0,
      66,     0,    63,    67,     0,   213,   242,    97,   242,   109,
     110,   242,   107,    31,     0,    35,     0,    39,    41,    43,
      36,     0,     0,   180,   183,   182,   241,   176,   204,     0,
     203,   202,   209,   210,   208,   240,     0,     0,   184,     0,
       0,   225,   223,   217,   242,   219,     0,   224,    60,    59,
      65,   212,   242,     0,    71,    70,     0,     0,     0,   132,
     133,   242,   134,   242,     0,   123,   113,   116,   242,   114,
     117,     0,   129,   130,   118,     0,   128,    33,     0,    38,
      42,     0,     0,     0,   201,     0,   238,   188,     0,   186,
     242,    48,     0,   218,   220,     0,     0,   221,   222,    83,
     242,     0,   242,    79,     0,    74,   242,     0,     0,     0,
      95,   111,     0,     0,   242,     0,   120,     0,     0,   155,
     242,   119,     0,   112,   115,     0,   132,   131,     0,    40,
     166,   181,   242,   205,     0,     0,   185,   192,     0,   191,
     190,    49,     0,     0,   242,    82,    72,   242,   242,    75,
      84,     0,     0,    86,     0,     0,    58,     0,     0,     0,
     102,     0,   100,     0,    98,     0,     0,     0,     0,   157,
     160,   162,   161,     0,   242,   121,     0,     0,     0,     0,
       0,     0,     0,   124,    43,   242,     0,     0,   189,   187,
     179,     0,     0,     0,   242,    76,    85,   242,    80,   242,
      81,    73,     0,     0,    89,    92,     0,     0,   103,   104,
      99,   101,     0,     0,   149,     0,     0,   156,     0,   150,
       0,   154,   177,   163,    94,     0,   125,     0,     0,     0,
     139,     0,     0,     0,    93,   242,   193,   226,   227,   242,
      77,     0,    88,    90,    87,     0,   106,     0,   152,     0,
     158,   159,   161,     0,     0,     0,     0,     0,     0,   142,
     242,     0,     0,   137,     0,    78,    91,     0,   122,     0,
     151,   164,     0,   126,     0,   242,     0,     0,     0,     0,
     143,   148,     0,     0,   178,   105,   153,   127,   242,     0,
       0,     0,     0,   242,   140,   141,   138,     0,   136,   147,
     145,   146,   144,   135
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -275,  -275,  -275,  -275,  -275,  -275,   496,  -275,   494,  -275,
    -275,  -275,  -275,  -275,   343,  -275,  -275,  -275,  -275,  -110,
    -275,  -275,   472,  -275,  -275,   440,  -275,  -275,  -275,  -275,
     256,    90,  -275,  -275,   513,  -275,  -275,   362,    66,   357,
    -275,  -275,   261,  -243,  -230,  -275,  -272,  -275,   201,   -98,
     -93,  -275,  -275,  -275,  -275,   208,  -274,  -275,  -275,  -275,
    -275,  -275,   302,  -275,  -275,  -275,   300,   197,  -240,  -275,
      80,  -275,  -107,   262,  -275,  -275,   163,   160,  -211,   -56,
    -275,   249,  -275,  -236,   301,  -265,   149,   245,   151,  -275,
    -112,   405,  -275,  -275,  -275,  -275,  -275,  -275,   -12,  -275,
    -275,  -275,  -275,   354,  -275,  -275,  -275,     7,   -59,   -18,
      -7,     4,   -24
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    73,    21,    22,    23,    24,
      25,    26,    27,   108,   109,    28,    64,   112,   176,   342,
     178,    29,    46,    47,    30,    79,    31,    76,    32,    44,
      33,    52,    53,    54,    55,    56,    94,    95,    99,   213,
     214,   267,   261,   262,   263,   264,   309,   316,   363,   364,
     365,    34,   103,   270,   321,   322,   324,    35,   106,   169,
     172,   228,   229,   230,   231,   232,   233,   343,   390,   439,
     440,   234,   276,   277,   278,   328,   329,   330,   382,   383,
     142,   337,   143,   182,   183,   197,   248,   249,   298,   299,
     144,   145,   146,   147,   189,   190,   148,   149,   235,    36,
      84,   156,   204,   205,   206,   257,   258,   150,    40,    81,
     198,    90,   151
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      57,   177,    57,    57,    37,    38,   259,    82,    43,    39,
      41,    42,   356,    48,    58,    62,   123,   304,    42,    63,
      65,   274,   141,   184,    82,    91,    92,   347,   192,    37,
      38,   317,   355,   259,   160,    82,    85,   163,   318,    96,
     319,    49,    50,   100,   368,   369,    77,   100,    80,    37,
      38,    37,    38,    57,    88,   107,    89,    89,    97,   154,
      37,    38,    97,   274,   338,   359,    97,    37,    38,   381,
     110,   339,   340,   385,   340,   166,   280,   358,   227,   165,
     393,   170,   187,   188,    49,    50,   362,   400,    80,   100,
      89,   224,   175,   362,   317,   201,   199,    60,   224,   356,
      61,   318,   201,   319,   378,   389,    89,   168,    97,   373,
      42,   -46,    89,   404,   399,   426,   281,    75,   179,   260,
     259,    75,   181,   185,   358,   102,   414,   425,   289,    75,
     184,    75,   191,    78,   275,   227,     7,    75,    75,   320,
     247,    97,    97,  -244,   252,  -244,   357,   194,   207,    96,
     104,   401,   417,   423,   311,    75,   392,    51,    51,   215,
     240,   215,    75,   331,   236,   427,    75,   167,    97,   413,
     449,   211,   341,   335,   341,   161,   275,    75,    97,   402,
     202,   110,   162,   457,   203,   290,   406,   202,   255,   293,
     430,   253,    69,   271,   297,    37,    38,   207,   429,    70,
      51,   414,   320,   456,   256,   265,   370,    37,    38,   458,
     442,    37,    38,   101,   279,   307,   279,   463,    72,   292,
     436,   236,   437,   308,    89,   446,    37,    38,    93,   184,
     185,   302,   303,   455,   438,    97,   414,   379,   348,   247,
     460,   461,   380,   300,   414,   179,   286,    98,   220,    97,
      89,    98,   362,   305,   334,   310,   222,   224,    83,   265,
      67,    45,    89,    89,   331,   403,   331,   279,   242,    68,
      66,   179,    86,   279,   403,   243,   184,   295,   173,   325,
     174,   105,   332,   391,   296,    89,   125,   126,   127,   345,
     111,   376,   344,    37,    38,   396,   113,   310,   377,    75,
     305,   305,   125,   126,   127,   128,   129,   130,   131,    37,
      38,   125,   126,   127,   128,   129,   242,   242,    37,    38,
     155,    97,   366,   418,   434,   295,   152,   132,   408,   185,
      97,   416,   443,   409,   391,   238,   247,   239,   459,   297,
     453,   133,   345,   114,   454,   344,   186,   305,    97,    78,
     305,   313,   310,    75,   314,   268,   315,   269,   431,   432,
     124,   125,   126,   127,   128,   129,   130,   131,    37,    38,
      97,   157,   419,   158,   405,   420,   185,   421,    78,    97,
     159,   164,    75,   332,   391,   412,   132,   180,   300,   171,
     391,   195,   310,   196,   435,    97,    37,    38,   391,   200,
     133,   125,   126,   127,   -82,   -82,   208,   209,   217,   212,
     448,    78,   241,   441,   125,   126,   127,   128,   129,   130,
     131,   246,    38,   245,   244,    89,     1,   251,   250,   266,
     134,   272,   135,   273,   136,   282,   137,   138,   139,   132,
     140,    89,   285,   288,   294,   301,   441,   306,     2,   259,
       3,   323,     4,   133,   326,   275,     5,   336,     6,     7,
       8,   333,     9,   350,   351,   352,    10,   353,    11,   354,
     360,   361,   374,    12,    13,    14,    15,    37,    38,   367,
     375,   384,   387,   388,   394,   218,   395,   397,   398,   134,
     414,   135,   219,   407,   220,   137,   138,   139,   415,   140,
     221,   428,   222,    37,    38,   223,   433,   444,   224,   445,
     450,   218,   447,   451,   452,    71,    74,   237,   219,    87,
     220,   153,    59,   210,   372,   216,   221,   312,   222,   371,
     284,   223,   287,   462,   224,   386,   327,   346,   411,   410,
     349,   422,   134,   291,   135,   193,   424,     0,   137,   138,
     139,     2,   140,     3,    16,     4,     0,     0,   254,     5,
       0,     6,     7,     8,     0,     9,    37,    38,     0,    10,
       0,    11,     0,     0,     0,     0,    12,    13,    14,   115,
     116,     0,   117,     0,     0,     0,     0,     0,     0,     0,
     118,     0,   225,     0,     0,   119,   226,     0,     0,   120,
       0,     0,     0,     0,   121,     0,     0,   122,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   225,     0,
       0,     0,   283,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       7,   111,     9,    10,    13,    14,    25,    15,     4,     2,
       3,     4,     4,     6,    11,    11,    75,   260,    11,    12,
      13,    23,    78,   135,    15,    49,    50,   292,   140,    13,
      14,    20,   304,    25,    93,    15,    43,    96,    27,    51,
      29,    20,    21,    55,   318,   319,    39,    59,    41,    13,
      14,    13,    14,    60,    47,    62,    49,    50,    51,    83,
      13,    14,    55,    23,    28,   308,    59,    13,    14,   334,
      63,    35,    36,    35,    36,    99,    36,   307,   171,    97,
     345,   105,   138,   139,    20,    21,    39,   359,    81,   101,
      83,    44,   110,    39,    20,    43,   152,    33,    44,     4,
      10,    27,    43,    29,   133,   341,    99,   103,   101,   134,
     103,   128,   105,   128,   357,   128,   223,   146,   111,   138,
      25,   146,   134,   135,   354,    59,   139,   399,   238,   146,
     242,   146,   139,   142,   136,   228,    33,   146,   146,   128,
     196,   134,   135,   134,   200,   136,   138,   140,   155,   161,
      60,   362,   388,   393,   264,   146,   136,   136,   136,   166,
     178,   168,   146,   275,   171,   134,   146,   101,   161,   380,
     435,   164,   136,   280,   136,   130,   136,   146,   171,   132,
     128,   174,   137,   448,   132,   241,   132,   128,    26,   245,
     128,   132,     0,   217,   250,    13,    14,   204,   409,     0,
     136,   139,   128,   443,    42,   212,   132,    13,    14,   449,
     421,    13,    14,    19,   221,   130,   223,   457,   129,   243,
      35,   228,    37,   138,   217,   128,    13,    14,    46,   341,
     242,   255,   256,   128,    49,   228,   139,   128,   294,   295,
     451,   452,   133,   250,   139,   238,    28,    53,    30,   242,
     243,    53,    39,   260,   278,   262,    38,    44,   133,   266,
       9,     5,   255,   256,   376,   363,   378,   274,   130,    18,
      14,   264,   128,   280,   372,   137,   388,   130,   128,   272,
     130,   133,   275,   342,   137,   278,     6,     7,     8,   285,
     131,   130,   285,    13,    14,   351,   128,   304,   137,   146,
     307,   308,     6,     7,     8,     9,    10,    11,    12,    13,
      14,     6,     7,     8,     9,    10,   130,   130,    13,    14,
     131,   314,   315,   137,   137,   130,   133,    31,   128,   341,
     323,   387,   137,   133,   393,   130,   392,   132,   450,   395,
     128,    45,   338,   128,   132,   338,   143,   354,   341,   142,
     357,   128,   359,   146,   131,   131,   133,   133,   414,   415,
     128,     6,     7,     8,     9,    10,    11,    12,    13,    14,
     363,   128,   128,   134,   367,   131,   388,   133,   142,   372,
     134,   145,   146,   376,   443,   378,    31,   128,   395,   131,
     449,   143,   399,   136,   418,   388,    13,    14,   457,   133,
      45,     6,     7,     8,    13,    14,   135,   135,   130,   136,
     434,   142,   135,   420,     6,     7,     8,     9,    10,    11,
      12,    13,    14,   130,   143,   418,     0,   128,   134,   134,
     134,   133,   136,    51,   138,    13,   140,   141,   142,    31,
     144,   434,    23,    13,   133,   128,   453,   137,    22,    25,
      24,   131,    26,    45,    11,   136,    30,   133,    32,    33,
      34,    14,    36,   135,   130,    13,    40,    13,    42,   130,
      13,   135,   128,    47,    48,    49,    50,    13,    14,   133,
      14,   128,   133,   136,   128,    21,   134,   128,   128,   134,
     139,   136,    28,   135,    30,   140,   141,   142,   133,   144,
      36,   128,    38,    13,    14,    41,   128,   135,    44,   135,
     133,    21,   128,   133,   133,    19,    22,   174,    28,    47,
      30,    81,     9,   161,   323,   168,    36,   266,    38,   321,
     228,    41,   232,   453,    44,   338,   274,   288,   378,   376,
     295,   392,   134,   242,   136,   140,   395,    -1,   140,   141,
     142,    22,   144,    24,   128,    26,    -1,    -1,   204,    30,
      -1,    32,    33,    34,    -1,    36,    13,    14,    -1,    40,
      -1,    42,    -1,    -1,    -1,    -1,    47,    48,    49,    26,
      27,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      37,    -1,   128,    -1,    -1,    42,   132,    -1,    -1,    46,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,    -1,
      -1,    -1,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     0,    22,    24,    26,    30,    32,    33,    34,    36,
      40,    42,    47,    48,    49,    50,   128,   148,   149,   150,
     151,   153,   154,   155,   156,   157,   158,   159,   162,   168,
     171,   173,   175,   177,   198,   204,   246,    13,    14,   254,
     255,   254,   254,   258,   176,   177,   169,   170,   254,    20,
      21,   136,   178,   179,   180,   181,   182,   257,    11,   181,
      33,   178,   258,   254,   163,   254,   177,     9,    18,     0,
       0,   153,   129,   152,   155,   146,   174,   254,   142,   172,
     254,   256,    15,   133,   247,   257,   128,   169,   254,   254,
     258,   259,   259,    46,   183,   184,   245,   254,    53,   185,
     245,    19,   185,   199,   178,   133,   205,   257,   160,   161,
     254,   131,   164,   128,   128,    26,    27,    29,    37,    42,
      46,    51,    54,   255,   128,     6,     7,     8,     9,    10,
      11,    12,    31,    45,   134,   136,   138,   140,   141,   142,
     144,   226,   227,   229,   237,   238,   239,   240,   243,   244,
     254,   259,   133,   172,   259,   131,   248,   128,   134,   134,
     255,   130,   137,   255,   145,   256,   259,   185,   258,   206,
     259,   131,   207,   128,   130,   256,   165,   166,   167,   254,
     128,   245,   230,   231,   237,   245,   143,   226,   226,   241,
     242,   257,   237,   238,   254,   143,   136,   232,   257,   226,
     133,    43,   128,   132,   249,   250,   251,   257,   135,   135,
     184,   254,   136,   186,   187,   257,   186,   130,    21,    28,
      30,    36,    38,    41,    44,   128,   132,   197,   208,   209,
     210,   211,   212,   213,   218,   245,   257,   161,   130,   132,
     256,   135,   130,   137,   143,   130,    13,   226,   233,   234,
     134,   128,   226,   132,   250,    26,    42,   252,   253,    25,
     138,   189,   190,   191,   192,   257,   134,   188,   131,   133,
     200,   259,   133,    51,    23,   136,   219,   220,   221,   257,
      36,   219,    13,   132,   209,    23,    28,   213,    13,   166,
     226,   231,   259,   226,   133,   130,   137,   226,   235,   236,
     257,   128,   259,   259,   190,   257,   137,   130,   138,   193,
     257,   166,   189,   128,   131,   133,   194,    20,    27,    29,
     128,   201,   202,   131,   203,   254,    11,   220,   222,   223,
     224,   237,   254,    14,   259,   219,   133,   228,    28,    35,
      36,   136,   166,   214,   254,   258,   228,   232,   226,   234,
     135,   130,    13,    13,   130,   193,     4,   138,   191,   190,
      13,   135,    39,   195,   196,   197,   254,   133,   203,   203,
     132,   202,   195,   134,   128,    14,   130,   137,   133,   128,
     133,   232,   225,   226,   128,    35,   214,   133,   136,   230,
     215,   255,   136,   232,   128,   134,   226,   128,   128,   190,
     193,   225,   132,   196,   128,   254,   132,   135,   128,   133,
     223,   224,   254,   225,   139,   133,   226,   230,   137,   128,
     131,   133,   233,   215,   235,   193,   128,   134,   128,   225,
     128,   226,   226,   128,   137,   259,    35,    37,    49,   216,
     217,   257,   225,   137,   135,   135,   128,   128,   259,   232,
     133,   133,   133,   128,   132,   128,   215,   232,   215,   237,
     225,   225,   217,   215
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   147,   148,   148,   149,   149,   149,   150,   151,   152,
     153,   154,   154,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   156,   156,   157,
     158,   159,   160,   160,   161,   161,   162,   163,   164,   165,
     165,   166,   166,   167,   168,   168,   169,   170,   171,   171,
     172,   173,   174,   175,   176,   177,   178,   178,   178,   179,
     180,   181,   181,   182,   183,   183,   184,   184,   185,   185,
     186,   186,   187,   188,   189,   189,   189,   189,   189,   190,
     190,   191,   192,   192,   193,   193,   194,   194,   194,   195,
     195,   196,   196,   197,   197,   198,   199,   199,   200,   200,
     201,   201,   202,   202,   202,   202,   203,   204,   205,   205,
     206,   206,   207,   207,   208,   208,   209,   209,   209,   209,
     209,   209,   209,   209,   210,   210,   210,   210,   211,   211,
     212,   212,   213,   213,   213,   214,   214,   214,   214,   214,
     215,   215,   215,   216,   216,   217,   217,   217,   217,   218,
     219,   219,   219,   219,   220,   221,   221,   222,   222,   223,
     223,   224,   224,   225,   225,   226,   226,   227,   227,   227,
     227,   227,   227,   227,   227,   227,   227,   228,   229,   229,
     230,   230,   231,   231,   232,   232,   233,   233,   234,   234,
     235,   235,   236,   236,   237,   237,   237,   238,   238,   239,
     239,   240,   241,   241,   242,   242,   243,   243,   244,   244,
     244,   245,   245,   245,   246,   247,   247,   248,   248,   249,
     249,   250,   250,   250,   251,   251,   252,   253,   254,   254,
     254,   254,   254,   254,   254,   254,   254,   254,   255,   255,
     256,   256,   257,   258,   259,   259
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
       4,     1,     1,     3,     1,     3,     2,     2,     1,     1,
       1,     1,     3,     3,     1,     2,     3,     4,     5,     1,
       3,     3,     1,     1,     1,     2,     1,     3,     3,     1,
       2,     3,     1,     4,     4,     6,     1,     2,     2,     3,
       1,     2,     1,     2,     2,     5,     3,     4,     1,     2,
       1,     3,     3,     2,     1,     2,     1,     1,     1,     2,
       2,     3,     6,     1,     3,     4,     6,     7,     1,     1,
       1,     2,     1,     1,     1,     7,     6,     3,     5,     2,
       4,     4,     2,     1,     3,     3,     3,     3,     1,     4,
       3,     5,     4,     6,     3,     1,     3,     1,     3,     3,
       1,     1,     1,     1,     3,     1,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     8,     5,
       1,     3,     1,     1,     1,     3,     1,     3,     1,     3,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     1,     1,     2,     2,
       2,     1,     3,     2,     4,     1,     2,     2,     3,     1,
       2,     2,     2,     1,     1,     1,     4,     4,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       3,     3,     0,     2,     1,     1
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
#line 265 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, T ( (yyvsp[0]) ) ); }
#line 1973 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 266 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 1979 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 270 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[0]) ) ); }
#line 1985 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 271 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 1991 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 272 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 1997 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 276 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_1_0, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2003 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 280 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_2, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2009 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 284 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2015 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 290 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_1_0, P ( (yyvsp[0]) ) ); }
#line 2021 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 294 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2027 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 295 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 2033 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 299 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2039 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 300 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2045 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 301 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2051 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 302 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2057 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 303 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2063 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 304 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2069 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 305 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2075 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 306 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2081 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 307 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2087 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 308 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2093 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 309 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2099 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 310 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2105 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 311 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2111 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 312 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2117 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 317 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2123 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 319 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2129 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 324 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VALIDATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2135 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 328 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2141 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 338 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2147 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 342 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2153 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 343 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P((yyvsp[-2])), T((yyvsp[-1])), P((yyvsp[0])) ); }
#line 2159 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 347 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2165 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 348 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2171 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 356 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESET, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2177 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 360 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2183 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 365 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESETDEF, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2189 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 371 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2195 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 372 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2201 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 376 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2207 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 377 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2213 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 381 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2219 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 389 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2225 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 391 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2231 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 395 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2237 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 399 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2243 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 407 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2249 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 409 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-6]) ), P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2255 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 413 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2261 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 421 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ALIAS, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2267 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 425 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2273 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 433 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EXTERN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2279 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 437 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2285 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 444 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCTION, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2291 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 448 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2297 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 449 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2303 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 456 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCDECL, P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2309 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 461 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNTYPED, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2315 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 466 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ROWLENGTH, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2321 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 470 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2327 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 471 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2333 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 475 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMASIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2339 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 479 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2345 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 480 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2351 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 484 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2357 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 485 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2363 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 489 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, T ( (yyvsp[0]) ) ); }
#line 2369 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 490 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, P ( (yyvsp[0]) ) ); }
#line 2375 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 494 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2381 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 495 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2387 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 499 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2393 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 503 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2399 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 507 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2405 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 509 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2411 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 511 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2417 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 513 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2423 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 515 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2429 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 519 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2435 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 520 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2441 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 525 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2447 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 529 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2453 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 530 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2459 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 534 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2465 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 535 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ELLIPSIS, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2471 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 540 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[0]) ) ); }
#line 2477 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 542 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2483 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 544 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2489 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 548 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2495 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 549 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2501 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 553 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURN, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2507 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 554 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2513 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 559 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2519 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 561 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2525 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 573 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSICAL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2531 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 577 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2537 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 578 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NOHEADER, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2543 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 583 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2549 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 585 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2555 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 589 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2561 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 590 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2567 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 595 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[0]) ) ); }
#line 2573 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 597 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2579 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 599 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2585 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 601 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2591 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 606 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSSTMT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2597 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 616 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2603 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 620 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2609 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 621 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2615 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 625 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2621 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 626 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2627 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 631 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2633 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 633 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2639 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 637 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2645 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 638 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2651 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 642 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2657 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 643 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2663 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 644 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2669 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 645 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2675 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 646 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2681 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 647 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2687 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 648 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUNTYPED, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2693 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 649 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2699 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 654 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2705 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 656 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2711 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 658 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2717 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 660 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2723 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 664 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2729 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 665 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2735 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 669 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2741 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 670 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2747 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 674 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLMODIFIER, T ( (yyvsp[0]) ) ); }
#line 2753 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 675 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLMODIFIER, T ( (yyvsp[0]) ) ); }
#line 2759 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 676 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLMODIFIER, T ( (yyvsp[0]) ) ); }
#line 2765 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 681 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, T ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2771 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 683 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2777 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 685 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2783 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 687 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2789 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 689 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2795 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 694 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2801 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 696 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2807 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 698 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2813 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 702 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2819 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 703 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2825 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 707 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2831 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 708 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2837 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 709 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2843 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 710 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2849 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 715 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DFLTVIEW, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2855 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 720 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2861 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 722 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2867 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 724 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2873 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 726 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2879 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 731 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOLDEF, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2885 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 735 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2891 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 736 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2897 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 740 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2903 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 741 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2909 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 745 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2915 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 746 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2921 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 750 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2927 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 751 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2933 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 758 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONDEXPR, P ( (yyvsp[0]) ) ); }
#line 2939 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 759 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2945 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 763 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2951 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 764 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CASTEXPR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2957 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 768 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2963 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 769 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2969 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 770 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2975 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 771 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2981 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 772 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2987 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 773 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2993 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 774 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2999 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 775 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3005 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 776 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3011 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 777 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNARYPLUS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3017 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 781 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ASSIGN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3023 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 793 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, T ( (yyvsp[-7]) ), P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3029 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 799 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3035 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 803 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3041 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 804 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3047 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 808 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3053 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 809 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3059 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 813 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3065 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 814 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3071 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 818 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3077 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 819 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3083 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 823 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3089 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 824 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMNAMED, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[-1]) ) ); }
#line 3095 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 828 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3101 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 829 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3107 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 833 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3113 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 834 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3119 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 838 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3125 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 839 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3131 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 840 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3137 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 844 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3143 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 845 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3149 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 849 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3155 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 850 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3161 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 854 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONSTVECT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3167 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 858 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3173 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 859 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3179 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 863 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3185 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 864 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3191 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 868 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3197 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 869 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3203 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 873 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3209 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 874 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3215 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 875 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3221 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 879 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3227 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 880 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1])), P ( (yyvsp[0]) ) ); }
#line 3233 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 881 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3239 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 891 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2])), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3245 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 895 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3251 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 896 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBDAD, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3257 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 900 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3263 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 901 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3269 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 905 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3275 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 906 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3281 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 910 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASEMEMBER, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3287 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 911 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEMEMBER, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3293 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 916 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3299 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 917 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3305 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 922 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBMEMBER, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3311 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 927 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TBLMEMBER, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3317 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 933 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3323 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 934 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3329 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 936 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3335 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 937 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3341 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 938 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3347 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 939 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3353 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 940 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3359 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 941 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3365 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 942 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3371 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 943 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3377 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 947 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 3383 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 948 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 3389 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 952 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DIM, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3395 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 953 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DIM, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3401 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 957 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EMPTY ); }
#line 3407 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 961 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSNAME, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3413 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 965 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3419 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 966 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3425 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 3429 "/home/ncbi/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
