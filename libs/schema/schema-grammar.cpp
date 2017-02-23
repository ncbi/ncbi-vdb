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
#line 27 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:339  */

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

    /* Create a flat list */
    static
    ParseTree *
    MakeList ( YYSTYPE & p_prod )
    {
        SchemaToken v = { PT_ASTLIST, NULL, 0, NULL, NULL };
        ParseTree * ret = new ParseTree ( v );
        ret -> AddChild ( P ( p_prod ) );
        return ret;
    }

    /* Add to a flat list node */
    static
    ParseTree *
    AddToList ( ParseTree * p_root, ParseTree * p_br1, ParseTree * p_br2 = 0 )
    {
        assert ( p_br1 != 0 );
        p_root -> AddChild ( p_br1 );
        if ( p_br2 != 0 )
        {
            p_root -> AddChild ( p_br2 );
        }
        return p_root;
    }



#line 166 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:339  */

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
#ifndef YY_SCHEMA_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED
# define YY_SCHEMA_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED
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
    PT_PRODTRIGGER = 343,
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
    PT_DFLTVIEW = 360,
    PT_PHYSMBR = 361,
    PT_PHYSCOL = 362,
    PT_PHYSCOLDEF = 363,
    PT_COLSCHEMAPARMS = 364,
    PT_COLSCHEMAPARAM = 365,
    PT_COLUNTYPED = 366,
    PT_DATABASE = 367,
    PT_TYPEEXPR = 368,
    PT_DBBODY = 369,
    PT_DBDAD = 370,
    PT_DATABASEMEMBER = 371,
    PT_DBMEMBER = 372,
    PT_TABLEMEMBER = 373,
    PT_TBLMEMBER = 374,
    PT_NOHEADER = 375,
    PT_CASTEXPR = 376,
    PT_CONSTVECT = 377,
    PT_NEGATE = 378,
    PT_UNARYPLUS = 379,
    PT_FACTPARMNAMED = 380,
    PT_VERSNAME = 381,
    PT_ARRAY = 382,
    PT_AT = 383
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

#endif /* !YY_SCHEMA_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 360 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   746

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  148
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  112
/* YYNRULES -- Number of rules.  */
#define YYNRULES  248
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  479

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
       2,     2,     2,     2,     2,     2,   130,     2,     2,     2,
     138,   139,   136,   144,   131,   145,     2,   146,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   147,   129,
     140,   137,   141,     2,   143,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   134,     2,   135,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   132,   142,   133,     2,     2,     2,     2,
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
       0,   281,   281,   282,   286,   287,   288,   292,   296,   300,
     306,   307,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   330,   338,   339,   345,
     350,   354,   361,   362,   366,   367,   371,   372,   378,   380,
     385,   389,   396,   403,   408,   414,   418,   419,   420,   430,
     435,   440,   441,   445,   449,   450,   454,   455,   459,   460,
     464,   465,   469,   473,   474,   476,   478,   480,   485,   486,
     490,   494,   498,   499,   501,   503,   505,   510,   511,   515,
     516,   521,   522,   526,   528,   530,   535,   536,   540,   541,
     548,   553,   560,   562,   570,   577,   587,   588,   592,   594,
     599,   600,   604,   606,   608,   610,   615,   622,   630,   631,
     635,   636,   640,   642,   647,   648,   652,   653,   654,   655,
     656,   657,   658,   659,   663,   665,   670,   672,   674,   676,
     681,   682,   686,   687,   691,   692,   693,   697,   699,   701,
     703,   705,   710,   712,   714,   719,   720,   724,   728,   729,
     733,   734,   735,   736,   740,   745,   747,   749,   751,   756,
     761,   762,   766,   767,   771,   772,   776,   777,   784,   785,
     789,   790,   794,   795,   796,   797,   798,   799,   800,   801,
     802,   803,   804,   808,   817,   826,   827,   831,   832,   836,
     837,   841,   842,   846,   847,   851,   852,   856,   857,   861,
     862,   863,   867,   868,   872,   873,   877,   881,   882,   886,
     887,   891,   892,   896,   897,   898,   899,   903,   904,   910,
     918,   919,   923,   924,   928,   929,   933,   934,   935,   939,
     940,   944,   949,   957,   965,   966,   968,   969,   970,   971,
     972,   973,   974,   975,   979,   983,   987,   991,   992
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
  "PT_UINT", "PT_TYPESET", "PT_TYPESETDEF", "PT_FORMAT", "PT_CONST",
  "PT_ALIAS", "PT_EXTERN", "PT_FUNCTION", "PT_UNTYPED", "PT_ROWLENGTH",
  "PT_FUNCDECL", "PT_EMPTY", "PT_SCHEMASIG", "PT_SCHEMAFORMAL",
  "PT_RETURNTYPE", "PT_FACTSIG", "PT_FUNCSIG", "PT_FUNCPARAMS",
  "PT_FORMALPARAM", "PT_ELLIPSIS", "PT_FUNCPROLOGUE", "PT_RETURN",
  "PT_PRODSTMT", "PT_PRODTRIGGER", "PT_SCHEMA", "PT_VALIDATE",
  "PT_PHYSICAL", "PT_PHYSPROLOGUE", "PT_PHYSSTMT", "PT_PHYSBODYSTMT",
  "PT_TABLE", "PT_TABLEPARENTS", "PT_TABLEBODY", "PT_FUNCEXPR",
  "PT_FACTPARMS", "PT_COLUMN", "PT_COLDECL", "PT_TYPEDCOL",
  "PT_COLMODIFIER", "PT_COLSTMT", "PT_DFLTVIEW", "PT_PHYSMBR",
  "PT_PHYSCOL", "PT_PHYSCOLDEF", "PT_COLSCHEMAPARMS", "PT_COLSCHEMAPARAM",
  "PT_COLUNTYPED", "PT_DATABASE", "PT_TYPEEXPR", "PT_DBBODY", "PT_DBDAD",
  "PT_DATABASEMEMBER", "PT_DBMEMBER", "PT_TABLEMEMBER", "PT_TBLMEMBER",
  "PT_NOHEADER", "PT_CASTEXPR", "PT_CONSTVECT", "PT_NEGATE",
  "PT_UNARYPLUS", "PT_FACTPARMNAMED", "PT_VERSNAME", "PT_ARRAY", "PT_AT",
  "';'", "'$'", "','", "'{'", "'}'", "'['", "']'", "'*'", "'='", "'('",
  "')'", "'<'", "'>'", "'|'", "'@'", "'+'", "'-'", "'/'", "':'", "$accept",
  "parse", "source", "version_1_0", "version_2_x", "schema_2_x",
  "schema_1_0", "schema_1_0_decl", "typedef_1_0_decl",
  "typedef_1_0_new_name_list", "typeset_1_0_decl", "typeset_1_0_new_name",
  "typeset_1_0_def", "typespec_1_0_list", "typespec_1_0", "dim_1_0",
  "format_1_0_decl", "format_1_0_new_name", "format_1_0_name",
  "const_1_0_decl", "alias_1_0_decl", "alias_1_0_new_name",
  "function_1_0_decl", "func_1_0_decl", "untyped_func_1_0_decl",
  "row_length_func_1_0_decl", "opt_func_1_0_schema_sig",
  "func_1_0_schema_sig", "func_1_0_schema_formals",
  "func_1_0_schema_formal", "func_1_0_return_type",
  "opt_func_1_0_fact_sig", "func_1_0_fact_sig", "func_1_0_fact_signature",
  "func_1_0_fact_params", "fact_param_1_0", "func_1_0_param_sig",
  "func_1_0_param_signature", "func_1_0_formal_params", "formal_param_1_0",
  "func_1_0_vararg_formals", "func_1_0_prologue", "script_1_0_stmt_seq",
  "script_1_0_stmt", "extern_1_0_decl", "ext_func_1_0_decl",
  "script_1_0_decl", "validate_1_0_decl", "physical_1_0_decl",
  "phys_1_0_return_type", "phys_1_0_prologue", "phys_1_0_body",
  "phys_1_0_body_stmt", "phys_1_0_stmt", "table_1_0_decl",
  "opt_tbl_1_0_parents", "tbl_1_0_parents", "tbl_1_0_body",
  "tbl_1_0_stmt_seq", "tbl_1_0_stmt", "production_1_0_stmt",
  "column_1_0_decl", "opt_col_1_0_modifier_seq", "col_1_0_modifier_seq",
  "col_1_0_modifier", "col_1_0_decl", "typed_column_decl_1_0", "col_ident",
  "phys_ident", "column_body_1_0", "column_stmt_1_0",
  "default_view_1_0_decl", "physmbr_1_0_decl", "phys_coldef_1_0",
  "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
  "col_schema_parm_1_0", "col_schema_value_1_0", "cond_expr_1_0",
  "expression_1_0", "primary_expr_1_0", "func_expr_1_0",
  "schema_parms_1_0", "schema_parm_1_0", "opt_factory_parms_1_0",
  "factory_parms_1_0", "factory_parm_1_0", "opt_func_1_0_parms",
  "func_1_0_parms", "uint_expr_1_0", "float_expr_1_0", "string_expr_1_0",
  "const_vect_expr_1_0", "opt_const_vect_exprlist_1_0",
  "const_vect_exprlist_1_0", "bool_expr_1_0", "negate_expr_1_0",
  "type_expr_1_0", "database_1_0_decl", "opt_database_dad_1_0",
  "database_body_1_0", "database_members_1_0", "database_member_1_0",
  "opt_template_1_0", "db_member_1_0", "table_member_1_0",
  "include_directive", "fqn_1_0", "ident_1_0", "empty", "fqn_vers",
  "fqn_opt_vers", YY_NULLPTR
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
     375,   376,   377,   378,   379,   380,   381,   382,   383,    59,
      36,    44,   123,   125,    91,    93,    42,    61,    40,    41,
      60,    62,   124,    64,    43,    45,    47,    58
};
# endif

#define YYPACT_NINF -314

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-314)))

#define YYTABLE_NINF -248

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     547,  -314,    59,    59,    59,    64,    59,   -10,    83,   -65,
      69,    59,    59,    59,    64,   256,  -314,   112,   145,   617,
      35,   617,  -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,
    -314,  -314,  -314,  -314,  -314,  -314,  -314,    -8,  -314,    59,
      92,    -1,    51,  -314,  -314,    96,    59,    -4,    59,    59,
      61,  -314,  -314,  -314,     3,  -314,  -314,  -314,   164,   -10,
    -314,    94,    -8,   119,   110,  -314,   191,   192,  -314,  -314,
     617,  -314,  -314,  -314,   241,   195,   110,   116,   417,  -314,
      59,   226,  -314,  -314,   218,   110,    42,   178,    59,  -314,
     165,  -314,    59,    58,  -314,    59,  -314,     3,  -314,    59,
    -314,    59,   229,  -314,   308,  -314,    59,   235,  -314,  -314,
    -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,
     481,  -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,
    -314,   481,  -314,    59,   395,  -314,   481,   464,   230,  -314,
    -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,  -314,    16,
    -314,   187,    -1,  -314,   -28,  -314,  -314,   238,   267,  -314,
      61,  -314,  -314,    59,   270,  -314,   270,   282,  -314,   208,
    -314,  -314,    59,   258,  -314,  -314,   287,  -314,   285,   286,
    -314,   293,   171,  -314,  -314,  -314,  -314,  -314,  -314,  -314,
     110,  -314,   493,   295,  -314,  -314,  -314,  -314,   -14,  -314,
      85,  -314,  -314,  -314,  -314,   110,    37,   303,  -314,  -314,
     249,    59,   306,   393,  -314,    44,  -314,    55,   432,  -314,
    -314,   433,   501,  -314,  -314,  -314,   424,   260,  -314,  -314,
    -314,  -314,    59,  -314,  -314,  -314,   481,   481,   395,    59,
     312,  -314,   173,  -314,   481,  -314,  -314,    59,    59,  -314,
    -314,    59,   438,   311,   177,  -314,  -314,    45,   246,   131,
     321,  -314,  -314,    59,   443,   316,   395,  -314,   444,    59,
    -314,    44,  -314,   320,   322,  -314,  -314,    29,  -314,  -314,
    -314,  -314,  -314,  -314,   187,   481,   493,  -314,  -314,   324,
     333,  -314,   453,   454,   338,  -314,  -314,    28,    59,  -314,
    -314,    59,   157,   466,   342,   276,  -314,  -314,  -314,   174,
      59,  -314,   345,   321,   321,  -314,    89,  -314,   174,  -314,
     204,   356,   472,   176,  -314,  -314,  -314,   168,    12,   187,
    -314,   481,   481,    50,   359,   357,   395,   266,  -314,    38,
     187,   360,  -314,  -314,  -314,   481,   379,   380,   346,  -314,
    -314,    59,  -314,   338,   497,   382,  -314,  -314,     0,   157,
    -314,   481,    -5,  -314,   498,   -48,    59,  -314,  -314,  -314,
    -314,     4,   376,  -314,   172,   395,  -314,   395,  -314,   481,
    -314,    90,  -314,    99,   381,  -314,   481,   395,   197,  -314,
     247,  -314,  -314,   493,   266,   481,  -314,  -314,  -314,   338,
    -314,  -314,   189,  -314,   157,  -314,   382,   101,  -314,  -314,
     383,  -314,   219,  -314,   387,  -314,   481,  -314,  -314,   110,
     105,  -314,   481,  -314,   481,   388,   199,    59,  -314,   163,
     481,   215,  -314,   384,  -314,   382,  -314,  -314,   481,   386,
    -314,   106,  -314,  -314,   398,  -314,    59,   187,   396,   397,
     399,    78,  -314,  -314,   113,   266,  -314,  -314,   133,  -314,
    -314,  -314,   187,   266,   391,   481,   481,   163,  -314,  -314,
    -314,  -314,   266,  -314,  -314,   390,   390,  -314,  -314
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   245,     0,   245,
     245,     0,     0,     0,     0,     0,    25,     0,     0,     0,
       0,     4,    10,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,   244,     0,   234,     0,
      34,     0,   245,    91,    90,     0,     0,    40,     0,     0,
       0,    45,    46,    47,     0,    52,    51,   233,     0,   245,
      92,   245,     0,     0,    30,    94,     0,     0,     1,     3,
       5,     9,     6,    11,     0,     0,    44,     0,     0,   246,
       0,     0,   220,    38,     0,    40,     0,     0,     0,   217,
       0,    54,     0,    34,    58,     0,    59,     0,    96,     0,
      93,     0,     0,   108,     0,    27,     0,     0,     8,     7,
     236,   237,   238,   239,   240,   241,   242,   243,   235,    43,
       0,   199,   201,   200,   202,   203,   204,   205,   147,   212,
     211,   245,    37,     0,     0,   174,     0,     0,     0,   173,
      36,   170,   175,   176,   177,   178,   179,   180,   181,   172,
     248,   245,   247,   221,   245,   219,    39,     0,     0,    56,
       0,    53,    57,     0,   245,    97,   245,   109,   110,   245,
     107,    26,     0,     0,    32,    29,     0,   209,     0,   208,
     207,     0,     0,   185,   188,   187,   182,   214,   215,   216,
     213,    35,     0,     0,   189,   230,   228,   222,   245,   224,
       0,   229,    50,    49,    55,   218,   245,     0,    61,    60,
       0,     0,     0,   134,   135,   245,   136,   245,     0,   123,
     113,     0,   245,   114,   116,   117,     0,   131,   132,   118,
     130,    28,     0,    31,    42,   206,     0,     0,     0,     0,
     244,   193,     0,   191,   245,   223,   225,     0,     0,   226,
     227,     0,     0,     0,   245,    68,    63,   245,     0,     0,
       0,    95,   111,     0,     0,   245,     0,   120,     0,     0,
     160,   245,   119,     0,     0,   112,   115,     0,   134,   133,
      33,   210,   171,   186,   245,     0,     0,   190,   197,     0,
     196,   195,     0,     0,   245,    70,    62,     0,     0,    64,
      81,     0,     0,     0,     0,   245,    77,    72,    83,     0,
       0,    48,     0,     0,     0,   102,     0,   100,     0,    98,
       0,     0,     0,     0,   162,   165,   167,   166,     0,   245,
     121,     0,     0,     0,     0,     0,     0,     0,   126,    34,
     245,     0,   194,   192,   184,     0,     0,     0,     0,    65,
      82,     0,    69,   245,     0,   245,    79,    71,     0,     0,
      73,     0,     0,    86,     0,     0,     0,   103,   104,    99,
     101,     0,     0,   154,     0,     0,   161,     0,   155,     0,
     159,     0,   168,     0,     0,   127,     0,     0,     0,   141,
       0,   146,   145,     0,     0,   245,   198,   231,   232,   245,
      66,    80,     0,    74,     0,    78,   245,     0,    85,    87,
       0,    84,     0,   106,     0,   157,     0,   163,   164,   166,
       0,   125,     0,   124,     0,     0,     0,     0,   144,   245,
       0,     0,   139,     0,    67,   245,    75,    88,     0,     0,
     122,     0,   156,   169,     0,   128,     0,   245,     0,     0,
       0,     0,   148,   153,     0,     0,   183,    76,     0,   105,
     158,   129,   245,     0,     0,     0,     0,   245,   142,   143,
     140,    89,     0,   138,   152,   150,   151,   149,   137
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -314,  -314,  -314,  -314,  -314,  -314,   509,    40,  -314,  -314,
    -314,  -314,  -314,  -314,   117,  -314,  -314,   484,  -314,  -314,
    -314,  -314,   355,    67,  -314,  -314,   526,  -314,  -314,   389,
      11,   374,  -314,  -314,  -175,  -253,  -314,  -314,  -272,  -254,
    -293,  -314,   223,     9,  -314,  -314,  -314,  -314,  -314,  -314,
    -314,  -314,   228,   -20,  -314,  -314,  -314,  -314,  -314,   326,
    -314,  -314,  -314,  -314,   319,   217,  -239,  -314,  -110,  -314,
      87,  -314,  -178,   291,  -314,  -314,   183,   175,  -313,   198,
    -314,  -314,  -287,   325,  -261,   166,   278,   170,  -314,  -116,
     429,  -314,  -314,  -314,  -314,  -314,  -314,     1,  -314,  -314,
    -314,  -314,   369,  -314,  -314,  -314,  -314,    34,   -68,    -7,
      22,   -73
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    72,    21,    22,    23,   104,
      24,    63,   107,   173,    89,   138,    25,    45,    46,    26,
      27,    75,    28,    51,    52,    53,    54,    55,    90,    91,
      95,   207,   208,   253,   254,   255,   258,   304,   305,   306,
     299,   311,   362,   363,    29,    44,    30,    31,    32,    99,
     261,   316,   317,   319,    33,   102,   167,   170,   222,   223,
     224,   225,   226,   227,   228,   338,   389,   390,   139,   451,
     452,   229,   267,   268,   269,   323,   324,   325,   381,   382,
     141,   142,   182,   183,   193,   242,   243,   289,   290,   143,
     144,   145,   146,   178,   179,   147,   148,   185,    34,    81,
     155,   198,   199,   200,   249,   250,    35,   149,    38,   300,
     150,   151
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,   349,    56,    56,   350,    36,   118,   153,    36,   -41,
      48,    49,   360,    36,    79,   195,    36,    36,   184,   383,
     159,   188,   164,   341,   162,   301,    42,   187,   168,   195,
     355,    79,   350,    61,   361,    82,    37,    40,    41,   272,
      47,    36,    36,   361,   352,    41,    62,    64,   407,   388,
      36,    92,    56,    79,   103,    96,    94,   333,    36,    96,
     400,    73,   403,    36,   334,   335,   420,   265,   380,    98,
     301,    76,    36,    77,    36,    50,   294,    60,   265,   394,
      85,   411,    86,    87,    93,   384,   335,   406,    93,    48,
      49,   271,    93,   330,    57,   352,    40,     7,    96,    74,
     426,   196,    59,   441,   405,   197,   434,    88,   165,   312,
      73,   247,    68,   436,   152,   196,   313,   454,   314,   245,
      39,   166,   184,   353,   180,   458,   100,   248,   408,   152,
      50,    93,   435,    41,   181,   152,   404,   413,   262,    74,
      40,   378,   457,    74,   194,    69,    74,   201,   405,   379,
     326,   312,   475,   476,  -247,   432,  -247,   209,   313,   209,
     314,    92,   230,    74,   351,    71,   284,    93,    93,   336,
      36,   190,    78,   251,   292,   293,   399,    36,   393,   105,
     157,   302,   301,    97,   266,    74,   463,    36,    80,    74,
     336,   201,    78,   350,    93,   266,   329,   205,   448,   256,
     449,   472,    36,    40,   163,    74,    40,   467,   270,    50,
     270,   468,   450,   361,   301,   230,   470,    94,   315,   421,
     184,    36,   369,   174,   473,    83,    78,   391,   423,   212,
     437,   101,   422,   478,   442,   460,   213,   291,   214,    74,
      40,   422,   469,   422,   215,   152,   216,   422,   422,   217,
     307,   106,   218,   120,    36,   422,    40,    74,   270,   326,
     315,   326,   471,    74,   270,    66,    40,   110,   111,   392,
     112,   184,    93,   152,    67,   422,   140,   194,   113,    36,
     128,   152,   152,   114,   391,    40,   221,   115,   278,   231,
     214,    40,   116,   367,   368,   117,   160,   320,   216,   340,
     327,   415,   238,   152,   286,   377,   161,   375,   297,   416,
     364,   339,   239,   298,   287,    74,   158,   376,   176,   364,
     108,   109,   194,   252,   119,    74,   392,   192,   238,   177,
     238,    40,    40,   194,   186,    40,    40,   219,   427,   221,
     446,   220,   372,    93,   365,   391,   286,   156,   474,   280,
     350,    74,    93,   391,   447,   340,   455,   439,   154,    36,
      43,   169,   391,   364,   175,   191,    74,   339,   252,    65,
      93,   409,   364,   462,   303,   308,   428,   202,   309,   429,
     409,   259,    40,   310,   430,    40,   260,   392,   291,   232,
     241,   233,    40,    40,   337,   392,    93,   121,   122,   123,
     412,   121,   122,   123,   392,    93,   203,   358,    36,   327,
     206,   419,   359,   211,   252,   252,   234,   236,   354,   303,
     235,    93,   453,   121,   122,   123,   124,   125,   126,   127,
      36,   128,   237,   244,   281,   282,    40,   171,    40,   172,
     194,   257,   288,   263,   264,   273,   274,   277,   129,   285,
     337,   295,   296,   318,   321,   194,   266,   331,   328,   332,
     453,   152,   130,   344,   345,   252,   346,   347,   252,   348,
     121,   122,   123,   124,   125,   303,   303,    36,   128,   356,
     152,   357,   366,   342,   241,   373,   374,   121,   122,   123,
     124,   125,   126,   127,    36,   128,   386,   387,   395,   121,
     122,   123,   124,   125,   126,   127,   240,   128,   397,   398,
     401,   410,   129,   402,    36,   414,   440,   445,   424,   303,
     438,   303,   212,   456,   129,   459,   130,   461,    70,   213,
      84,   214,   422,   464,   465,    58,   466,   215,   130,   216,
     210,   371,   217,   396,   370,   218,   279,     1,   276,   204,
     385,   131,   418,   132,   477,   133,   322,   134,   417,   431,
     135,   136,   137,   283,   343,   433,   189,   246,     0,     2,
       0,     3,     0,     4,     0,     0,     0,     5,     0,     6,
       7,     8,     0,     9,   425,     0,     0,    10,     0,    11,
       0,   241,     0,   288,    12,    13,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   131,     0,     0,     0,   133,
     443,   134,   444,     0,   135,   136,   137,   131,     0,     0,
     219,   133,     0,   134,   275,     0,   135,   136,   137,     2,
       0,     3,     0,     4,     0,     0,     0,     5,     0,     6,
       7,     8,     0,     9,     0,     0,     0,    10,     0,    11,
       0,     0,     0,     0,    12,    13,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       7,   294,     9,    10,     4,    13,    74,    80,    13,    13,
      20,    21,   305,    13,    15,    43,    13,    13,   134,   332,
      88,   137,    95,   284,    92,    25,     4,   137,   101,    43,
     302,    15,     4,    11,    39,    42,     2,     3,     4,   217,
       6,    13,    13,    39,   297,    11,    12,    13,   361,   336,
      13,    50,    59,    15,    61,    54,    53,    28,    13,    58,
     353,    21,   355,    13,    35,    36,   379,    23,   329,    58,
      25,    37,    13,    39,    13,   140,   251,    10,    23,   340,
      46,   129,    48,    49,    50,    35,    36,   359,    54,    20,
      21,    36,    58,   271,    11,   348,    62,    33,    97,   147,
     387,   129,    33,   416,   358,   133,   399,    46,    97,    20,
      70,    26,     0,   406,    80,   129,    27,   430,    29,   133,
       3,    99,   238,   298,   131,   438,    59,    42,   133,    95,
     140,    97,   404,    99,   133,   101,   136,   133,   211,   147,
     106,   129,   435,   147,   151,     0,   147,   154,   402,   137,
     266,    20,   465,   466,   138,   394,   140,   164,    27,   166,
      29,   160,   169,   147,   136,   130,   239,   133,   134,   140,
      13,   137,   134,   136,   247,   248,   351,    13,   140,    62,
     138,   136,    25,    19,   140,   147,   447,    13,   137,   147,
     140,   198,   134,     4,   160,   140,   269,   163,    35,   206,
      37,   462,    13,   169,   146,   147,   172,   129,   215,   140,
     217,   133,    49,    39,    25,   222,   455,    53,   129,   129,
     336,    13,   133,   106,   463,   129,   134,   337,   129,    21,
     129,   137,   142,   472,   129,   129,    28,   244,    30,   147,
     206,   142,   129,   142,    36,   211,    38,   142,   142,    41,
     257,   132,    44,   137,    13,   142,   222,   147,   265,   375,
     129,   377,   129,   147,   271,     9,   232,    26,    27,   337,
      29,   387,   238,   239,    18,   142,    78,   284,    37,    13,
      14,   247,   248,    42,   394,   251,   169,    46,    28,   172,
      30,   257,    51,   313,   314,    54,   131,   263,    38,   277,
     266,   129,   131,   269,   131,   137,   141,   131,   131,   137,
     309,   277,   141,   136,   141,   147,   138,   141,   120,   318,
     129,   129,   329,   206,   129,   147,   394,   140,   131,   131,
     131,   297,   298,   340,   136,   301,   302,   129,   141,   222,
     141,   133,   138,   309,   310,   455,   131,   129,   464,   232,
       4,   147,   318,   463,   427,   333,   141,   138,   132,    13,
       5,   132,   472,   362,   129,   135,   147,   333,   251,    14,
     336,   362,   371,   446,   257,   129,   129,   139,   132,   132,
     371,   132,   348,   137,   137,   351,   137,   455,   395,   131,
     192,   133,   358,   359,   277,   463,   362,     6,     7,     8,
     366,     6,     7,     8,   472,   371,   139,   131,    13,   375,
     140,   377,   136,   131,   297,   298,   129,   131,   301,   302,
     135,   387,   429,     6,     7,     8,     9,    10,    11,    12,
      13,    14,   139,   138,   236,   237,   402,   129,   404,   131,
     447,   138,   244,   137,    51,    13,    13,    23,    31,   137,
     333,    13,   141,   132,    11,   462,   140,   137,    14,   137,
     467,   427,    45,   139,   131,   348,    13,    13,   351,   131,
       6,     7,     8,     9,    10,   358,   359,    13,    14,    13,
     446,   139,   137,   285,   286,   129,    14,     6,     7,     8,
       9,    10,    11,    12,    13,    14,   137,   140,   138,     6,
       7,     8,     9,    10,    11,    12,    13,    14,   129,   129,
      13,    13,    31,   131,    13,   139,   129,   129,   137,   402,
     137,   404,    21,   139,    31,   139,    45,   129,    19,    28,
      46,    30,   142,   137,   137,     9,   137,    36,    45,    38,
     166,   318,    41,   345,   316,    44,   227,     0,   222,   160,
     333,   134,   377,   136,   467,   138,   265,   140,   375,   393,
     143,   144,   145,   238,   286,   395,   137,   198,    -1,    22,
      -1,    24,    -1,    26,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    -1,    36,   386,    -1,    -1,    40,    -1,    42,
      -1,   393,    -1,   395,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   134,    -1,    -1,    -1,   138,
     422,   140,   424,    -1,   143,   144,   145,   134,    -1,    -1,
     129,   138,    -1,   140,   133,    -1,   143,   144,   145,    22,
      -1,    24,    -1,    26,    -1,    -1,    -1,    30,    -1,    32,
      33,    34,    -1,    36,    -1,    -1,    -1,    40,    -1,    42,
      -1,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     0,    22,    24,    26,    30,    32,    33,    34,    36,
      40,    42,    47,    48,    49,    50,   129,   149,   150,   151,
     152,   154,   155,   156,   158,   164,   167,   168,   170,   192,
     194,   195,   196,   202,   246,   254,    13,   255,   256,   162,
     255,   255,   258,   170,   193,   165,   166,   255,    20,    21,
     140,   171,   172,   173,   174,   175,   257,    11,   174,    33,
     171,   258,   255,   159,   255,   170,     9,    18,     0,     0,
     154,   130,   153,   155,   147,   169,   255,   255,   134,    15,
     137,   247,   257,   129,   165,   255,   255,   255,    46,   162,
     176,   177,   245,   255,    53,   178,   245,    19,   178,   197,
     171,   137,   203,   257,   157,   162,   132,   160,   129,   129,
      26,    27,    29,    37,    42,    46,    51,    54,   256,   129,
     137,     6,     7,     8,     9,    10,    11,    12,    14,    31,
      45,   134,   136,   138,   140,   143,   144,   145,   163,   216,
     227,   228,   229,   237,   238,   239,   240,   243,   244,   255,
     258,   259,   255,   259,   132,   248,   129,   138,   138,   256,
     131,   141,   256,   146,   259,   178,   258,   204,   259,   132,
     205,   129,   131,   161,   162,   129,   227,   227,   241,   242,
     257,   245,   230,   231,   237,   245,   227,   216,   237,   238,
     255,   135,   140,   232,   257,    43,   129,   133,   249,   250,
     251,   257,   139,   139,   177,   255,   140,   179,   180,   257,
     179,   131,    21,    28,    30,    36,    38,    41,    44,   129,
     133,   162,   206,   207,   208,   209,   210,   211,   212,   219,
     257,   162,   131,   133,   129,   135,   131,   139,   131,   141,
      13,   227,   233,   234,   138,   133,   250,    26,    42,   252,
     253,   136,   162,   181,   182,   183,   257,   138,   184,   132,
     137,   198,   259,   137,    51,    23,   140,   220,   221,   222,
     257,    36,   220,    13,    13,   133,   207,    23,    28,   212,
     162,   227,   227,   231,   259,   137,   131,   141,   227,   235,
     236,   257,   259,   259,   182,    13,   141,   131,   136,   188,
     257,    25,   136,   162,   185,   186,   187,   257,   129,   132,
     137,   189,    20,    27,    29,   129,   199,   200,   132,   201,
     255,    11,   221,   223,   224,   225,   237,   255,    14,   259,
     220,   137,   137,    28,    35,    36,   140,   162,   213,   255,
     258,   232,   227,   234,   139,   131,    13,    13,   131,   188,
       4,   136,   183,   182,   162,   186,    13,   139,   131,   136,
     188,    39,   190,   191,   245,   255,   137,   201,   201,   133,
     200,   190,   138,   129,    14,   131,   141,   137,   129,   137,
     232,   226,   227,   226,    35,   213,   137,   140,   230,   214,
     215,   216,   256,   140,   232,   138,   227,   129,   129,   182,
     188,    13,   131,   188,   136,   187,   186,   226,   133,   191,
      13,   129,   255,   133,   139,   129,   137,   224,   225,   255,
     226,   129,   142,   129,   137,   227,   230,   141,   129,   132,
     137,   233,   214,   235,   188,   186,   188,   129,   137,   138,
     129,   226,   129,   227,   227,   129,   141,   259,    35,    37,
      49,   217,   218,   257,   226,   141,   139,   188,   226,   139,
     129,   129,   259,   232,   137,   137,   137,   129,   133,   129,
     214,   129,   232,   214,   237,   226,   226,   218,   214
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   148,   149,   149,   150,   150,   150,   151,   152,   153,
     154,   154,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   156,   157,   157,   158,
     159,   160,   161,   161,   162,   162,   163,   163,   164,   164,
     165,   166,   167,   168,   169,   170,   171,   171,   171,   172,
     173,   174,   174,   175,   176,   176,   177,   177,   178,   178,
     179,   179,   180,   181,   181,   181,   181,   181,   182,   182,
     183,   184,   185,   185,   185,   185,   185,   186,   186,   187,
     187,   188,   188,   189,   189,   189,   190,   190,   191,   191,
     192,   193,   194,   194,   195,   196,   197,   197,   198,   198,
     199,   199,   200,   200,   200,   200,   201,   202,   203,   203,
     204,   204,   205,   205,   206,   206,   207,   207,   207,   207,
     207,   207,   207,   207,   208,   208,   209,   209,   209,   209,
     210,   210,   211,   211,   212,   212,   212,   213,   213,   213,
     213,   213,   214,   214,   214,   215,   215,   216,   217,   217,
     218,   218,   218,   218,   219,   220,   220,   220,   220,   221,
     222,   222,   223,   223,   224,   224,   225,   225,   226,   226,
     227,   227,   228,   228,   228,   228,   228,   228,   228,   228,
     228,   228,   228,   229,   229,   230,   230,   231,   231,   232,
     232,   233,   233,   234,   234,   235,   235,   236,   236,   237,
     237,   237,   238,   238,   239,   239,   240,   241,   241,   242,
     242,   243,   243,   244,   244,   244,   244,   245,   245,   246,
     247,   247,   248,   248,   249,   249,   250,   250,   250,   251,
     251,   252,   253,   254,   255,   255,   255,   255,   255,   255,
     255,   255,   255,   255,   256,   257,   258,   259,   259
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     2,     3,     3,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     3,     4,
       1,     3,     1,     3,     1,     4,     1,     1,     3,     4,
       1,     1,     6,     4,     1,     2,     1,     1,     6,     4,
       4,     1,     1,     3,     1,     3,     2,     2,     1,     1,
       1,     1,     3,     1,     2,     3,     4,     5,     1,     3,
       2,     3,     1,     2,     3,     4,     5,     1,     3,     2,
       3,     1,     2,     1,     3,     3,     1,     2,     3,     5,
       2,     1,     2,     3,     2,     6,     1,     2,     2,     3,
       1,     2,     1,     2,     2,     5,     3,     4,     1,     2,
       1,     3,     3,     2,     1,     2,     1,     1,     1,     2,
       2,     3,     6,     1,     5,     5,     3,     4,     6,     7,
       1,     1,     1,     2,     1,     1,     1,     7,     6,     3,
       5,     2,     4,     4,     2,     1,     1,     1,     1,     3,
       3,     3,     3,     1,     4,     3,     5,     4,     6,     3,
       1,     3,     1,     3,     3,     1,     1,     1,     1,     3,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     8,     5,     1,     3,     1,     1,     1,
       3,     1,     3,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     2,     2,     2,     2,     1,     3,     4,
       1,     2,     2,     3,     1,     2,     2,     2,     1,     1,
       1,     4,     4,     2,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     0,     2,     1,     1
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
#line 281 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, T ( (yyvsp[0]) ) ); }
#line 2005 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 282 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2011 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 286 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[0]) ) ); }
#line 2017 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 287 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2023 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 288 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2029 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_1_0, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2035 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 296 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_2, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2041 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 300 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2047 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 306 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_1_0, P ( (yyvsp[0]) ) ); }
#line 2053 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 307 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 2059 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 311 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2065 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 312 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2071 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2077 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 314 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2083 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 315 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2089 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 316 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2095 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 317 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2101 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 318 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2107 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 319 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2113 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 320 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2119 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2125 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 322 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2131 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2137 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 324 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2143 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 334 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDEF, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2149 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 338 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2155 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 339 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P((yyvsp[-2])), T((yyvsp[-1])), P((yyvsp[0])) ); }
#line 2161 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 346 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESET, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2167 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 350 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2173 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 355 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESETDEF, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2179 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2185 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 362 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2191 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 366 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2197 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2203 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 371 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2209 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2215 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 379 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2221 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 381 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2227 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 385 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2233 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 389 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2239 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 397 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2245 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 404 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ALIAS, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2251 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 408 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2257 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 414 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCTION, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2263 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 418 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2269 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 419 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2275 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 426 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCDECL, P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2281 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 431 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNTYPED, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2287 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 436 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ROWLENGTH, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2293 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 440 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2299 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 441 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2305 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 445 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMASIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2311 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 449 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2317 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 450 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 454 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2329 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 455 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2335 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 459 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, T ( (yyvsp[0]) ) ); }
#line 2341 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, P ( (yyvsp[0]) ) ); }
#line 2347 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 464 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2353 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2359 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 469 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2365 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 473 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2371 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 475 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2377 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 477 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2383 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 479 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2389 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 481 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2395 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 485 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2401 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 486 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2407 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 490 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2413 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 494 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2419 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 498 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2425 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 500 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2431 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 502 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2437 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 504 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2443 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 506 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2449 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 510 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2455 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 511 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2461 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 515 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2467 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 517 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2473 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 521 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2479 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 522 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ELLIPSIS, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2485 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 527 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[0]) ) ); }
#line 2491 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 529 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2497 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 531 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2503 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 535 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2509 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 536 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2515 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 540 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURN, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2521 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 542 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2527 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 549 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EXTERN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2533 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 553 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2539 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 561 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2545 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 563 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2551 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 571 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VALIDATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2557 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 583 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSICAL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2563 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 587 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2569 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 588 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NOHEADER, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2575 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 593 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2581 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 595 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2587 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 599 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2593 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 600 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2599 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 605 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[0]) ) ); }
#line 2605 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 607 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2611 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 609 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2617 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 611 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2623 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 616 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSSTMT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2629 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 626 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2635 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 630 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2641 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 631 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2647 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 635 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2653 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 636 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2659 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 641 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2665 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 643 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2671 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 647 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2677 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 648 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2683 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 652 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2689 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 653 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2695 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 654 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2701 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 655 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2707 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 656 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2713 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 657 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2719 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 658 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUNTYPED, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2725 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 659 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2731 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 664 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2737 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 666 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODTRIGGER, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2743 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 671 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2749 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 673 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2755 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 675 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2761 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 677 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2767 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2773 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 682 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2779 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 686 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2785 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2791 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 691 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLMODIFIER, T ( (yyvsp[0]) ) ); }
#line 2797 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLMODIFIER, T ( (yyvsp[0]) ) ); }
#line 2803 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLMODIFIER, T ( (yyvsp[0]) ) ); }
#line 2809 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 698 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, T ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2815 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 700 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2821 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 702 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2827 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 704 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2833 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 706 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2839 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 711 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2845 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 713 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2851 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 715 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2857 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 719 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2863 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 720 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2869 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 724 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 2875 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 728 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2881 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 729 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2887 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 733 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2893 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 734 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2899 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 735 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2905 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 736 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2911 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DFLTVIEW, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2917 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 746 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2923 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 748 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2929 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 750 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2935 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 752 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2941 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 757 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOLDEF, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2947 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 761 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2953 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 762 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2959 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 766 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2965 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 767 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2971 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 771 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2977 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 772 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2983 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 776 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2989 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 777 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2995 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 784 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3001 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 785 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3007 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 789 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3013 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 790 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CASTEXPR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3019 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 794 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3025 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 795 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3031 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 796 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3037 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 797 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3043 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 798 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3049 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 799 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3055 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 800 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3061 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 801 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3067 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 802 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3073 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 803 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3079 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNARYPLUS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3085 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 816 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, T ( (yyvsp[-7]) ), P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3091 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 822 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3097 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 826 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3103 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 827 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3109 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 831 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3115 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 832 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3121 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 836 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3127 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 837 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3133 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 841 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3139 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 842 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3145 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 846 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3151 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 847 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMNAMED, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[-1]) ) ); }
#line 3157 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 851 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3163 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 852 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3169 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 856 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3175 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 857 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3181 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 861 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3187 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 862 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3193 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 863 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3199 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 867 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3205 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 868 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3211 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 872 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3217 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 873 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3223 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 877 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONSTVECT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3229 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 881 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3235 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 882 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3241 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 886 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3247 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 887 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3253 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 891 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3259 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 892 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3265 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 896 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3271 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 897 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3277 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 898 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3283 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 899 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3289 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 903 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3295 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 904 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1])), P ( (yyvsp[0]) ) ); }
#line 3301 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 914 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2])), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3307 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 918 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 919 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBDAD, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3319 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 923 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3325 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 924 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3331 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 928 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3337 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 929 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3343 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 933 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASEMEMBER, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3349 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 934 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEMEMBER, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3355 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 939 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 940 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 945 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBMEMBER, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3373 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 950 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TBLMEMBER, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3379 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 957 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3385 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 965 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FQN, P ( (yyvsp[0]) ) ); }
#line 3391 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 966 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3397 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 968 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3403 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 969 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3409 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 970 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3415 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 971 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3421 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 972 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3427 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 973 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3433 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 974 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3439 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 975 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3445 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 979 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 3451 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 983 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EMPTY ); }
#line 3457 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 987 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSNAME, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3463 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 991 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3469 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 992 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3475 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 3479 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
