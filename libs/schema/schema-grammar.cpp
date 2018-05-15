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
#line 27 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:339  */

    #define YYDEBUG 1

    #include <stdio.h>

    #include "ParseTree.hpp"
    #include "ErrorReport.hpp"

    using namespace ncbi::SchemaParser;

    #include "schema-tokens.h"
    #include "schema-lex.h"
    #define Schema_lex SchemaScan_yylex

    void Schema_error ( YYLTYPE *                   p_llocp,
                        ParseTree **                p_root,
                        ErrorReport *               p_errors,
                        struct SchemaScanBlock *    p_sb,
                        const char *                p_msg )
    {
        /* send message to the C++ parser for proper display and recovery */
        Token :: Location loc ( p_sb -> file_name, p_llocp -> first_line, p_llocp -> first_column );
        p_errors -> ReportError ( loc, "%s", p_msg);
    }

    extern "C"
    {
        extern enum yytokentype SchemaScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, SchemaScanBlock* sb );
    }

    static
    ParseTree*
    P ( SchemaToken & p_prod )
    {
        assert ( p_prod . subtree );
        return ( ParseTree * ) p_prod . subtree;
    }

    static
    ParseTree*
    T ( SchemaToken & p_term )
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
               ParseTree * p_ch8 = 0,
               ParseTree * p_ch9 = 0
             )
    {
        ParseTree * ret = new ParseTree ( Token ( p_token ) );
        if ( p_ch1 != 0 ) ret -> AddChild ( p_ch1 );
        if ( p_ch2 != 0 ) ret -> AddChild ( p_ch2 );
        if ( p_ch3 != 0 ) ret -> AddChild ( p_ch3 );
        if ( p_ch4 != 0 ) ret -> AddChild ( p_ch4 );
        if ( p_ch5 != 0 ) ret -> AddChild ( p_ch5 );
        if ( p_ch6 != 0 ) ret -> AddChild ( p_ch6 );
        if ( p_ch7 != 0 ) ret -> AddChild ( p_ch7 );
        if ( p_ch8 != 0 ) ret -> AddChild ( p_ch8 );
        if ( p_ch9 != 0 ) ret -> AddChild ( p_ch9 );
        return ret;
    }

    /* Create a flat list */
    static
    ParseTree *
    MakeList ( SchemaToken & p_prod )
    {
        ParseTree * ret = new ParseTree ( Token ( PT_ASTLIST ) );
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



#line 173 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:339  */

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

#ifndef YY_SCHEMA_2T_DATA_ALTDEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED
# define YY_SCHEMA_2T_DATA_ALTDEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED

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



int Schema_parse (ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb);

#endif /* !YY_SCHEMA_2T_DATA_ALTDEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 374 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   856

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  155
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  125
/* YYNRULES -- Number of rules.  */
#define YYNRULES  270
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  524

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
     144,   145,   142,   151,   137,   150,   153,   152,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   154,   136,
     146,   143,   147,     2,   149,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   140,     2,   141,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   138,   148,   139,     2,     2,     2,     2,
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
       0,   298,   298,   299,   303,   304,   305,   311,   315,   316,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   339,   347,   348,   354,   359,   363,
     370,   371,   375,   376,   380,   381,   387,   389,   394,   398,
     405,   412,   417,   423,   427,   428,   429,   439,   444,   449,
     450,   454,   458,   459,   463,   464,   468,   469,   473,   474,
     478,   482,   483,   485,   487,   489,   494,   495,   499,   503,
     507,   508,   510,   512,   514,   519,   520,   524,   525,   530,
     531,   535,   537,   539,   544,   545,   549,   550,   557,   562,
     569,   571,   579,   586,   596,   597,   601,   603,   608,   609,
     613,   615,   617,   619,   624,   631,   639,   640,   644,   645,
     649,   651,   656,   657,   661,   662,   663,   665,   667,   668,
     669,   670,   671,   673,   677,   679,   684,   685,   689,   690,
     694,   695,   696,   700,   702,   707,   709,   711,   716,   718,
     720,   725,   726,   730,   734,   735,   739,   740,   744,   745,
     746,   747,   751,   756,   758,   760,   762,   767,   772,   773,
     777,   778,   782,   783,   787,   788,   795,   796,   800,   801,
     805,   806,   807,   808,   809,   810,   811,   812,   813,   814,
     815,   816,   817,   821,   828,   837,   838,   842,   843,   844,
     848,   849,   853,   857,   858,   862,   863,   867,   868,   872,
     873,   874,   878,   879,   883,   884,   888,   892,   893,   897,
     898,   902,   903,   907,   908,   912,   913,   917,   924,   932,
     933,   937,   938,   942,   943,   947,   948,   949,   953,   954,
     958,   963,   971,   979,   980,   982,   983,   984,   985,   986,
     987,   988,   989,   993,   997,  1001,  1005,  1006,  1012,  1016,
    1017,  1021,  1022,  1026,  1031,  1032,  1036,  1040,  1041,  1045,
    1046,  1050,  1052,  1054,  1058,  1059,  1063,  1064,  1068,  1073,
    1074
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
  "PT_VIEWPARENTS", "PT_VIEWPARENT", "PT_MEMBEREXPR", "PT_JOINEXPR", "';'",
  "','", "'{'", "'}'", "'['", "']'", "'*'", "'='", "'('", "')'", "'<'",
  "'>'", "'|'", "'@'", "'-'", "'+'", "'/'", "'.'", "':'", "$accept",
  "parse", "source", "version_1_0", "schema_1_0", "schema_1_0_decl",
  "typedef_1_0_decl", "typedef_1_0_new_name_list", "typeset_1_0_decl",
  "typeset_1_0_new_name", "typeset_1_0_def", "typespec_1_0_list",
  "typespec_1_0", "dim_1_0", "format_1_0_decl", "format_1_0_new_name",
  "format_1_0_name", "const_1_0_decl", "alias_1_0_decl",
  "alias_1_0_new_name", "function_1_0_decl", "func_1_0_decl",
  "untyped_func_1_0_decl", "row_length_func_1_0_decl",
  "opt_func_1_0_schema_sig", "func_1_0_schema_sig",
  "func_1_0_schema_formals", "func_1_0_schema_formal",
  "func_1_0_return_type", "opt_func_1_0_fact_sig", "func_1_0_fact_sig",
  "func_1_0_fact_signature", "func_1_0_fact_params", "fact_param_1_0",
  "func_1_0_param_sig", "func_1_0_param_signature",
  "func_1_0_formal_params", "formal_param_1_0", "func_1_0_vararg_formals",
  "func_1_0_prologue", "script_1_0_stmt_seq", "script_1_0_stmt",
  "extern_1_0_decl", "ext_func_1_0_decl", "script_1_0_decl",
  "validate_1_0_decl", "physical_1_0_decl", "phys_1_0_return_type",
  "phys_1_0_prologue", "phys_1_0_body", "phys_1_0_body_stmt",
  "phys_1_0_stmt", "table_1_0_decl", "opt_tbl_1_0_parents",
  "tbl_1_0_parents", "tbl_1_0_body", "tbl_1_0_stmt_seq", "tbl_1_0_stmt",
  "production_1_0_stmt", "col_1_0_modifiers", "col_1_0_modifier_seq",
  "col_1_0_modifier", "col_1_0_decl", "phys_enc_ref",
  "typed_column_decl_1_0", "col_ident", "phys_ident",
  "opt_column_body_1_0", "column_body_1_0", "column_stmt_1_0",
  "default_view_1_0_decl", "physmbr_1_0_decl", "phys_coldef_1_0",
  "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
  "col_schema_parm_1_0", "col_schema_value_1_0", "cond_expr_1_0",
  "expression_1_0", "primary_expr_1_0", "func_expr_1_0",
  "schema_parms_1_0", "schema_parm_1_0", "opt_factory_parms_1_0",
  "factory_parms_1_0", "factory_parms", "opt_func_1_0_parms",
  "func_1_0_parms", "uint_expr_1_0", "float_expr_1_0", "string_expr_1_0",
  "const_vect_expr_1_0", "opt_const_vect_exprlist_1_0",
  "const_vect_exprlist_1_0", "bool_expr_1_0", "type_expr_1_0",
  "member_expr_2_0", "join_expr_2_0", "database_1_0_decl",
  "opt_database_dad_1_0", "database_body_1_0", "database_members_1_0",
  "database_member_1_0", "opt_template_1_0", "db_member_1_0",
  "table_member_1_0", "include_directive", "fqn_1_0", "ident_1_0", "empty",
  "fqn_vers", "fqn_opt_vers", "version_2_0", "schema_2_0",
  "schema_2_0_decl", "view_2_0_decl", "view_parms", "view_parm",
  "opt_view_body", "view_body", "view_member", "opt_view_parents",
  "view_parents", "view_parent", "view_parent_parms", YY_NULLPTR
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
     385,   386,   387,   388,   389,   390,    59,    44,   123,   125,
      91,    93,    42,    61,    40,    41,    60,    62,   124,    64,
      45,    43,    47,    46,    58
};
# endif

#define YYPACT_NINF -350

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-350)))

#define YYTABLE_NINF -247

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     585,  -350,   121,   121,   121,    35,   121,     7,   144,    15,
      44,   121,   121,   121,    35,   374,  -350,   173,   179,   692,
     692,  -350,  -350,  -350,  -350,  -350,  -350,  -350,  -350,  -350,
    -350,  -350,  -350,  -350,  -350,   720,  -350,    -7,  -350,   121,
     -53,     5,    40,  -350,  -350,    70,   121,    -6,   121,   121,
      72,  -350,  -350,  -350,    25,  -350,  -350,  -350,   175,     7,
    -350,   137,    -7,    90,    98,  -350,   142,   160,  -350,  -350,
     692,  -350,   121,  -350,   720,  -350,  -350,   257,   166,    98,
     -82,   516,  -350,   121,   167,  -350,  -350,   180,    98,    93,
     113,   121,  -350,   104,  -350,   121,    56,  -350,   121,  -350,
      25,  -350,   121,  -350,   121,   182,  -350,   292,  -350,   121,
     207,  -350,  -350,   186,  -350,  -350,  -350,  -350,  -350,  -350,
    -350,  -350,  -350,  -350,  -350,   561,  -350,  -350,  -350,  -350,
    -350,  -350,  -350,  -350,  -350,  -350,   561,  -350,   121,   389,
    -350,   561,   561,   226,  -350,  -350,  -350,  -350,  -350,  -350,
    -350,  -350,  -350,  -350,  -350,     0,    -1,  -350,   224,     5,
    -350,    66,  -350,  -350,   240,   244,  -350,    72,  -350,  -350,
     121,   235,  -350,   235,   242,  -350,    75,  -350,  -350,   121,
     109,  -350,  -350,   121,   264,  -350,   285,   268,  -350,   267,
     152,  -350,  -350,   -32,  -350,  -350,  -350,  -350,   561,   121,
     561,   286,  -350,  -350,  -350,  -350,  -350,    85,  -350,   170,
    -350,  -350,  -350,  -350,  -350,  -350,    98,    20,   294,  -350,
    -350,   197,   121,   293,   288,   388,  -350,    -8,  -350,    43,
     121,  -350,  -350,   121,   410,  -350,  -350,     4,   234,  -350,
    -350,  -350,   121,  -350,   121,   192,  -350,  -350,  -350,   561,
     561,   389,   121,   516,   -15,  -350,  -350,  -350,   201,   561,
    -350,  -350,   121,   121,   121,   429,   306,   236,  -350,  -350,
      29,   220,   100,   318,  -350,  -350,   121,   422,   317,   453,
     320,   389,  -350,   455,   121,  -350,    -8,  -350,   327,   328,
    -350,  -350,   389,   437,  -350,   437,    38,   224,  -350,  -350,
    -350,  -350,  -350,   121,   329,  -350,  -350,  -350,   224,   333,
     322,   561,   561,  -350,  -350,   332,   341,  -350,   121,   121,
     343,  -350,  -350,    18,   121,  -350,  -350,   121,   196,   468,
     337,   271,  -350,  -350,  -350,    17,   121,  -350,   342,   318,
     318,  -350,    95,  -350,    17,  -350,   200,   344,   561,   350,
     474,   210,  -350,  -350,  -350,    -3,   223,   224,  -350,   561,
     561,   214,  -350,   263,  -350,  -350,  -350,  -350,  -350,  -350,
     121,  -350,   351,   347,  -350,   121,  -350,  -350,  -350,   561,
     357,   358,   128,  -350,  -350,   121,  -350,   343,   485,   363,
    -350,  -350,    28,   196,  -350,   561,    36,  -350,   121,   -96,
     121,  -350,  -350,  -350,  -350,    37,   356,   561,   366,  -350,
     233,   389,  -350,   389,  -350,   561,  -350,    64,    94,   121,
    -350,    55,   561,   360,   367,  -350,    51,   561,  -350,  -350,
    -350,  -350,   343,  -350,  -350,   181,  -350,   196,  -350,   363,
      97,  -350,  -350,   364,  -350,   208,  -350,   372,   373,  -350,
    -350,   561,  -350,  -350,    98,   107,  -350,  -350,   224,   368,
     369,   370,  -350,   371,    55,  -350,  -350,   118,   121,   121,
     121,  -350,   121,  -350,   376,    51,  -350,   386,  -350,   363,
    -350,  -350,   561,   387,  -350,  -350,   135,  -350,  -350,   412,
     561,   561,  -350,  -350,  -350,  -350,   218,  -350,   121,   375,
    -350,  -350,  -350,  -350,   140,  -350,  -350,   380,   155,   158,
     121,  -350,   378,   561,  -350,  -350,  -350,  -350,  -350,   561,
     162,   183,  -350,  -350
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   244,     0,   244,
     244,     0,     0,     0,     0,     0,    23,     0,     0,     0,
       4,     8,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,   243,     0,   233,     0,
      32,     0,   244,    89,    88,     0,     0,    38,     0,     0,
       0,    43,    44,    45,     0,    50,    49,   232,     0,   244,
      90,   244,     0,     0,    28,    92,     0,     0,     1,     3,
       5,     9,     0,   251,     6,   249,   252,     0,     0,    42,
       0,     0,   245,     0,     0,   219,    36,     0,    38,     0,
       0,     0,   213,     0,    52,     0,    32,    56,     0,    57,
       0,    94,     0,    91,     0,     0,   106,     0,    25,     0,
       0,     7,   248,     0,   250,   235,   236,   237,   238,   239,
     240,   241,   242,   234,    41,     0,   199,   201,   200,   202,
     203,   204,   205,   143,   212,   211,   244,    35,     0,     0,
     172,     0,     0,     0,   171,    34,   168,   173,   174,   175,
     176,   177,   178,   181,   182,   170,   233,   247,   244,   246,
     220,   244,   218,    37,     0,     0,    54,     0,    51,    55,
       0,   244,    95,   244,   107,   108,     0,   105,    24,     0,
       0,    30,    27,     0,     0,   209,     0,   208,   207,     0,
       0,   185,   189,   187,   179,   180,    33,   216,     0,     0,
       0,     0,   191,   190,   229,   227,   221,   244,   223,     0,
     225,   226,   228,    48,    47,    53,   214,   244,     0,    59,
      58,     0,     0,     0,   126,   130,   131,   244,   132,   244,
       0,   123,   111,     0,     0,   112,   114,     0,     0,   128,
     121,    26,     0,    29,     0,     0,   254,    40,   206,     0,
       0,     0,     0,     0,     0,   166,   215,   193,     0,   244,
     222,   224,     0,     0,     0,     0,     0,   244,    66,    61,
     244,     0,     0,     0,    93,   109,     0,     0,     0,     0,
     244,     0,   119,     0,     0,   158,   244,   118,     0,     0,
     110,   113,     0,     0,   115,     0,    32,   244,   127,   130,
     129,    31,   256,     0,   244,   210,   169,   186,   244,     0,
       0,     0,     0,   192,   197,     0,   196,   195,     0,     0,
     244,    68,    60,     0,     0,    62,    79,     0,     0,     0,
       0,   244,    75,    70,    81,     0,     0,    46,     0,     0,
       0,   100,     0,    98,     0,    96,     0,     0,     0,     0,
       0,     0,   160,   163,   165,   164,     0,   244,   120,     0,
       0,     0,   133,     0,   142,   141,   134,   137,   136,   255,
       0,   264,     0,     0,   188,     0,   167,   194,   184,     0,
       0,     0,     0,    63,    80,     0,    67,   244,     0,   244,
      77,    69,     0,     0,    71,     0,     0,    84,     0,     0,
       0,   101,   102,    97,    99,     0,     0,     0,     0,   152,
       0,     0,   159,     0,   153,     0,   157,     0,     0,     0,
     140,   244,     0,     0,   265,   266,   244,   244,   217,   198,
     230,   231,   244,    64,    78,     0,    72,     0,    76,   244,
       0,    83,    85,     0,    82,     0,   104,     0,     0,   116,
     155,     0,   161,   162,   164,     0,   125,   124,   244,     0,
       0,     0,   151,     0,   145,   146,   144,     0,     0,     0,
       0,   263,     0,   257,     0,   258,   259,     0,    65,   244,
      73,    86,     0,     0,   122,   117,     0,   154,   135,     0,
       0,     0,   138,   147,   139,   269,     0,   267,     0,     0,
     253,   260,   183,    74,     0,   103,   156,     0,     0,     0,
       0,   268,     0,     0,    87,   150,   148,   149,   270,     0,
       0,     0,   261,   262
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -350,  -350,  -350,  -350,   484,    39,  -350,  -350,  -350,  -350,
    -350,  -350,    22,   266,  -350,   471,  -350,  -350,  -350,  -350,
     212,    84,  -350,  -350,   524,  -350,  -350,   377,   -37,   361,
    -350,  -350,  -213,  -287,  -350,  -350,  -310,  -349,  -297,  -350,
     191,  -124,  -350,  -350,  -350,  -350,  -350,  -350,  -350,  -350,
     194,   123,  -350,  -350,  -350,  -350,  -350,   303,  -350,  -350,
    -350,   300,  -350,  -350,   245,  -350,    82,  -350,  -350,    78,
    -350,  -215,   265,  -350,  -350,   132,   134,  -333,   172,  -350,
    -350,   258,   301,  -289,   259,  -350,   126,  -350,  -136,  -350,
    -350,  -350,  -350,  -350,  -350,   169,  -350,  -350,  -350,  -350,
    -350,  -350,   349,  -350,  -350,  -350,  -350,    -2,   246,   157,
       1,   -59,  -350,  -350,   480,  -350,  -350,   256,  -350,  -350,
      86,  -350,  -350,    91,  -350
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    22,   107,    23,    63,
     110,   180,    92,   143,    24,    45,    46,    25,    26,    78,
      27,    51,    52,    53,    54,    55,    93,    94,    98,   218,
     219,   266,   267,   268,   271,   330,   331,   332,   325,   337,
     396,   397,    28,    44,    29,    30,    31,   102,   274,   342,
     343,   345,    32,   105,   174,   177,   234,   235,   236,   237,
     238,   239,   294,   295,   362,   363,   144,   463,   464,   465,
     240,   282,   283,   284,   351,   352,   353,   254,   255,   146,
     147,   190,   191,   201,   202,   258,   315,   316,   148,   149,
     150,   151,   186,   187,   152,   398,   153,   154,    33,    84,
     162,   207,   208,   209,   210,   211,    34,   155,    38,   326,
     157,   158,    35,    74,    75,    76,   245,   246,   474,   475,
     476,   372,   424,   425,   496
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,    40,    41,   192,    47,    42,    36,   -39,   368,    41,
      62,    64,    61,   197,   287,    82,   280,    36,   389,   373,
      82,   101,   384,   383,   160,    39,   417,   418,    48,    49,
      36,    36,   384,    36,   394,    79,   386,    80,    36,   171,
     444,    36,    36,   438,    88,   175,    89,    90,    96,    36,
      36,   320,    96,    82,   327,   327,    96,   395,    77,    71,
      40,   125,   440,   172,    36,    48,    49,   280,   416,     7,
      41,   358,    77,   113,    73,   470,   395,   395,    59,    97,
     286,   159,   455,   439,   108,    36,   438,    81,    36,   467,
     433,   459,   436,   460,    60,   386,   159,   223,    96,   224,
      41,    77,   159,   173,   225,   461,   226,    40,   253,    71,
     204,   387,   227,    73,   228,   192,   338,   229,   486,    91,
     230,   338,    77,   339,   244,   340,   310,   479,   339,   204,
     340,   181,   384,   311,    36,   478,    96,   193,   281,   198,
     413,    36,   480,   103,  -246,   354,  -246,    77,    77,   504,
     292,    77,   199,    50,    77,    57,   192,   508,   509,    77,
     385,    50,   264,   275,    56,    96,    56,    56,   216,   488,
     437,   328,   432,    68,    40,   441,   446,    40,    81,    69,
     520,   159,   503,    83,   200,   384,   521,   471,    36,   281,
      50,   462,    77,   308,    36,   100,    81,   262,   233,    85,
     456,   241,   205,   318,   319,   206,    86,   327,   170,    36,
      77,   231,   311,   263,   232,    40,    56,    43,   106,    95,
     159,   205,   327,    99,   260,   357,    65,    99,   109,    97,
     457,   341,    40,   481,   403,   296,   341,   164,   297,   265,
      40,   167,   311,   487,   244,   311,   242,    77,   243,   193,
     159,   168,    77,   145,   494,   311,   233,   165,   298,   293,
     159,   159,    40,   299,   301,   226,   311,    77,    40,    99,
      36,   506,   442,   228,   346,   354,   514,   354,   111,   355,
     104,   442,   159,   311,   115,   116,   265,   117,   311,   251,
     193,   516,   329,   188,   517,   118,   112,   184,   522,   252,
     119,   159,   124,   311,   120,   161,   311,   189,   185,   121,
     311,   423,   122,   194,   195,   203,   163,   277,   212,   523,
     176,    40,    40,   123,   278,    40,    40,   156,   220,   303,
     220,   311,   183,    96,   399,   272,    95,   166,   312,   304,
     273,   169,    96,   182,   406,   265,   265,   411,   313,   388,
     329,   251,   483,   507,    77,   510,   334,   412,   335,   414,
     458,   419,    77,   336,   212,   511,   415,   196,   159,   450,
     200,   156,   257,   323,   269,   364,   451,   364,   324,   222,
      40,   217,   156,    40,   285,   213,   285,   156,   156,   214,
      40,    40,    66,    67,    96,   126,   127,   128,   445,   420,
     247,   421,    36,    96,   265,   249,   422,   265,   392,   355,
     423,   454,   250,   393,   329,   329,   317,   159,   126,   127,
     128,   305,   306,    36,    40,   145,   248,   333,   178,   179,
     259,   314,   223,    40,   224,    40,   276,   285,   270,   225,
     279,   226,   321,   285,   156,   256,   156,   227,   472,   228,
      36,   133,   229,   322,   203,   230,   344,   329,   347,   329,
     348,   371,   401,   402,   349,   203,   281,   159,    40,   356,
     359,   360,   370,    40,   374,   375,   288,   378,   379,   289,
     382,   390,   391,   376,   377,   400,   409,   407,   410,   426,
     302,   427,   498,   430,   431,   156,   156,   472,   434,   156,
     435,   447,   449,    70,   469,   156,   468,   482,   484,   485,
     492,   489,   490,   491,   203,   500,   515,    87,   513,   309,
     408,   519,   126,   127,   128,   129,   130,   131,   132,    36,
     133,   502,   505,    58,   221,   405,   404,   291,   300,   365,
     366,   365,   493,   452,   215,   350,   231,   453,   134,   290,
     361,   429,   307,   477,   114,   367,   261,   156,   156,   369,
     497,   501,   135,     0,   380,   381,     0,   126,   127,   128,
     129,   130,   131,   132,    36,   133,     0,     0,   466,   448,
       0,     0,     0,   473,   317,     1,     0,     0,     0,     0,
       0,     0,     0,   134,   156,     0,     0,     0,     0,   314,
       0,     0,     0,     0,     0,   156,   156,   135,     2,     0,
       3,     0,     4,     0,     0,   203,     5,     0,     6,     7,
       8,   428,     9,     0,     0,   156,    10,     0,    11,     0,
       0,     0,     0,    12,    13,    14,    15,     0,     0,     0,
       0,   156,     0,     0,   443,     0,     0,     0,     0,     0,
       0,     0,     0,   156,     0,     0,   136,     0,   137,     0,
     138,   156,   139,     0,     0,   140,   141,   142,   156,     0,
       0,     0,     0,   156,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   156,     0,     0,
       0,   136,     0,     0,     0,   138,     0,   139,     0,     0,
     140,   141,   142,     0,   495,     2,     0,     3,   499,     4,
       0,    16,     0,     5,     0,     6,     7,     8,   156,     9,
       0,     0,     0,    10,     0,    11,   156,   156,     0,     0,
      12,    13,    14,     2,   512,     3,     0,     4,     0,     0,
       0,     5,     0,     6,     7,     8,   518,     9,     0,   156,
       0,    10,     0,    11,     0,   156,     0,     0,    12,    13,
      14,     0,    72,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       2,     3,     4,   139,     6,     4,    13,    13,   297,    11,
      12,    13,    11,    14,   229,    15,    24,    13,   328,   308,
      15,    58,     4,   320,    83,     3,   359,   360,    21,    22,
      13,    13,     4,    13,   331,    37,   323,    39,    13,    98,
     136,    13,    13,   392,    46,   104,    48,    49,    50,    13,
      13,   264,    54,    15,    26,    26,    58,    40,   154,    20,
      62,   143,   395,   100,    13,    21,    22,    24,   357,    34,
      72,   286,   154,    72,    35,    24,    40,    40,    34,    54,
      37,    83,   415,   393,    62,    13,   435,   140,    13,   422,
     387,    36,   389,    38,    10,   382,    98,    22,   100,    24,
     102,   154,   104,   102,    29,    50,    31,   109,   140,    70,
      44,   324,    37,    74,    39,   251,    21,    42,   451,    47,
      45,    21,   154,    28,   183,    30,   141,   437,    28,    44,
      30,   109,     4,   148,    13,   432,   138,   139,   146,   140,
     143,    13,   439,    59,   144,   281,   146,   154,   154,   482,
     146,   154,   153,   146,   154,    11,   292,   490,   491,   154,
     142,   146,   142,   222,     7,   167,     9,    10,   170,   458,
     142,   142,   385,     0,   176,   139,   139,   179,   140,     0,
     513,   183,   479,   143,   146,     4,   519,   136,    13,   146,
     146,   136,   154,   252,    13,    20,   140,    27,   176,    42,
     136,   179,   136,   262,   263,   139,   136,    26,   152,    13,
     154,   136,   148,    43,   139,   217,    59,     5,    61,    50,
     222,   136,    26,    54,   139,   284,    14,    58,   138,    54,
     136,   136,   234,   136,   139,   237,   136,   144,   237,   217,
     242,   137,   148,   136,   303,   148,   137,   154,   139,   251,
     252,   147,   154,    81,   136,   148,   234,   144,    24,   237,
     262,   263,   264,    29,   242,    31,   148,   154,   270,   100,
      13,   136,   396,    39,   276,   411,   136,   413,   136,   281,
     143,   405,   284,   148,    27,    28,   264,    30,   148,   137,
     292,   136,   270,   136,   136,    38,   136,   125,   136,   147,
      43,   303,   136,   148,    47,   138,   148,   138,   136,    52,
     148,   370,    55,   141,   142,   158,   136,    29,   161,   136,
     138,   323,   324,    77,    36,   327,   328,    81,   171,   137,
     173,   148,   146,   335,   336,   138,   167,    91,   137,   147,
     143,    95,   344,   136,   144,   323,   324,   137,   147,   327,
     328,   137,   144,   489,   154,   137,   136,   147,   138,   136,
     419,   147,   154,   143,   207,   147,   143,   141,   370,   136,
     146,   125,   200,   137,   217,   293,   143,   295,   142,   137,
     382,   146,   136,   385,   227,   145,   229,   141,   142,   145,
     392,   393,    18,    19,   396,     6,     7,     8,   400,   136,
     136,   138,    13,   405,   382,   137,   143,   385,   137,   411,
     469,   413,   145,   142,   392,   393,   259,   419,     6,     7,
       8,   249,   250,    13,   426,   253,   141,   270,   136,   137,
     144,   259,    22,   435,    24,   437,   143,   280,   144,    29,
      52,    31,    13,   286,   198,   199,   200,    37,   426,    39,
      13,    14,    42,   147,   297,    45,   138,   435,    36,   437,
     143,   304,   339,   340,    11,   308,   146,   469,   470,    14,
     143,   143,   143,   475,   141,   153,   230,   145,   137,   233,
     137,    13,   145,   311,   312,   143,   136,   143,    14,   138,
     244,   144,   470,   136,   136,   249,   250,   475,    13,   253,
     137,   145,   136,    19,   137,   259,   146,   143,   136,   136,
     139,   143,   143,   143,   357,   139,   136,    46,   143,   253,
     348,   143,     6,     7,     8,     9,    10,    11,    12,    13,
      14,   145,   145,     9,   173,   344,   342,   234,   238,   293,
     295,   295,   464,   411,   167,   280,   136,   413,    32,   139,
     292,   379,   251,   427,    74,   296,   207,   311,   312,   303,
     469,   475,    46,    -1,   318,   319,    -1,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    -1,    -1,   421,   407,
      -1,    -1,    -1,   426,   427,     0,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,   348,    -1,    -1,    -1,    -1,   427,
      -1,    -1,    -1,    -1,    -1,   359,   360,    46,    23,    -1,
      25,    -1,    27,    -1,    -1,   458,    31,    -1,    33,    34,
      35,   375,    37,    -1,    -1,   379,    41,    -1,    43,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,   395,    -1,    -1,   398,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   407,    -1,    -1,   140,    -1,   142,    -1,
     144,   415,   146,    -1,    -1,   149,   150,   151,   422,    -1,
      -1,    -1,    -1,   427,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   451,    -1,    -1,
      -1,   140,    -1,    -1,    -1,   144,    -1,   146,    -1,    -1,
     149,   150,   151,    -1,   468,    23,    -1,    25,   472,    27,
      -1,   136,    -1,    31,    -1,    33,    34,    35,   482,    37,
      -1,    -1,    -1,    41,    -1,    43,   490,   491,    -1,    -1,
      48,    49,    50,    23,   498,    25,    -1,    27,    -1,    -1,
      -1,    31,    -1,    33,    34,    35,   510,    37,    -1,   513,
      -1,    41,    -1,    43,    -1,   519,    -1,    -1,    48,    49,
      50,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     0,    23,    25,    27,    31,    33,    34,    35,    37,
      41,    43,    48,    49,    50,    51,   136,   156,   157,   158,
     159,   160,   161,   163,   169,   172,   173,   175,   197,   199,
     200,   201,   207,   253,   261,   267,    13,   262,   263,   167,
     262,   262,   265,   175,   198,   170,   171,   262,    21,    22,
     146,   176,   177,   178,   179,   180,   264,    11,   179,    34,
     176,   265,   262,   164,   262,   175,    18,    19,     0,     0,
     159,   160,    52,   160,   268,   269,   270,   154,   174,   262,
     262,   140,    15,   143,   254,   264,   136,   170,   262,   262,
     262,    47,   167,   181,   182,   250,   262,    54,   183,   250,
      20,   183,   202,   176,   143,   208,   264,   162,   167,   138,
     165,   136,   136,   265,   269,    27,    28,    30,    38,    43,
      47,    52,    55,   263,   136,   143,     6,     7,     8,     9,
      10,    11,    12,    14,    32,    46,   140,   142,   144,   146,
     149,   150,   151,   168,   221,   233,   234,   235,   243,   244,
     245,   246,   249,   251,   252,   262,   263,   265,   266,   262,
     266,   138,   255,   136,   144,   144,   263,   137,   147,   263,
     152,   266,   183,   265,   209,   266,   138,   210,   136,   137,
     166,   167,   136,   146,   233,   233,   247,   248,   264,   250,
     236,   237,   243,   262,   233,   233,   141,    14,   140,   153,
     146,   238,   239,   264,    44,   136,   139,   256,   257,   258,
     259,   260,   264,   145,   145,   182,   262,   146,   184,   185,
     264,   184,   137,    22,    24,    29,    31,    37,    39,    42,
      45,   136,   139,   167,   211,   212,   213,   214,   215,   216,
     225,   167,   137,   139,   266,   271,   272,   136,   141,   137,
     145,   137,   147,   140,   232,   233,   263,   233,   240,   144,
     139,   257,    27,    43,   142,   167,   186,   187,   188,   264,
     144,   189,   138,   143,   203,   266,   143,    29,    36,    52,
      24,   146,   226,   227,   228,   264,    37,   226,   263,   263,
     139,   212,   146,   167,   217,   218,   262,   265,    24,    29,
     216,   167,   263,   137,   147,   233,   233,   237,   266,   168,
     141,   148,   137,   147,   233,   241,   242,   264,   266,   266,
     187,    13,   147,   137,   142,   193,   264,    26,   142,   167,
     190,   191,   192,   264,   136,   138,   143,   194,    21,    28,
      30,   136,   204,   205,   138,   206,   262,    36,   143,    11,
     227,   229,   230,   231,   243,   262,    14,   266,   226,   143,
     143,   236,   219,   220,   221,   263,   219,   239,   238,   272,
     143,   264,   276,   238,   141,   153,   233,   233,   145,   137,
     263,   263,   137,   193,     4,   142,   188,   187,   167,   191,
      13,   145,   137,   142,   193,    40,   195,   196,   250,   262,
     143,   206,   206,   139,   205,   195,   144,   143,   233,   136,
      14,   137,   147,   143,   136,   143,   238,   232,   232,   147,
     136,   138,   143,   266,   277,   278,   138,   144,   263,   233,
     136,   136,   187,   193,    13,   137,   193,   142,   192,   191,
     232,   139,   196,   263,   136,   262,   139,   145,   233,   136,
     136,   143,   230,   231,   262,   232,   136,   136,   266,    36,
      38,    50,   136,   222,   223,   224,   264,   232,   146,   137,
      24,   136,   167,   264,   273,   274,   275,   241,   193,   191,
     193,   136,   143,   144,   136,   136,   232,   136,   238,   143,
     143,   143,   139,   224,   136,   263,   279,   278,   167,   263,
     139,   275,   145,   193,   232,   145,   136,   243,   232,   232,
     137,   147,   263,   143,   136,   136,   136,   136,   263,   143,
     232,   232,   136,   136
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   155,   156,   156,   157,   157,   157,   158,   159,   159,
     160,   160,   160,   160,   160,   160,   160,   160,   160,   160,
     160,   160,   160,   160,   161,   162,   162,   163,   164,   165,
     166,   166,   167,   167,   168,   168,   169,   169,   170,   171,
     172,   173,   174,   175,   176,   176,   176,   177,   178,   179,
     179,   180,   181,   181,   182,   182,   183,   183,   184,   184,
     185,   186,   186,   186,   186,   186,   187,   187,   188,   189,
     190,   190,   190,   190,   190,   191,   191,   192,   192,   193,
     193,   194,   194,   194,   195,   195,   196,   196,   197,   198,
     199,   199,   200,   201,   202,   202,   203,   203,   204,   204,
     205,   205,   205,   205,   206,   207,   208,   208,   209,   209,
     210,   210,   211,   211,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   213,   213,   214,   214,   215,   215,
     216,   216,   216,   217,   217,   218,   218,   218,   219,   219,
     219,   220,   220,   221,   222,   222,   223,   223,   224,   224,
     224,   224,   225,   226,   226,   226,   226,   227,   228,   228,
     229,   229,   230,   230,   231,   231,   232,   232,   233,   233,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   234,   234,   235,   235,   236,   236,   237,   237,   237,
     238,   238,   239,   240,   240,   241,   241,   242,   242,   243,
     243,   243,   244,   244,   245,   245,   246,   247,   247,   248,
     248,   249,   249,   250,   250,   251,   251,   252,   253,   254,
     254,   255,   255,   256,   256,   257,   257,   257,   258,   258,
     259,   260,   261,   262,   262,   262,   262,   262,   262,   262,
     262,   262,   262,   263,   264,   265,   266,   266,   267,   268,
     268,   269,   269,   270,   271,   271,   272,   273,   273,   274,
     274,   275,   275,   275,   276,   276,   277,   277,   278,   279,
     279
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     2,     3,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     3,     4,     1,     3,
       1,     3,     1,     4,     1,     1,     3,     4,     1,     1,
       6,     4,     1,     2,     1,     1,     6,     4,     4,     1,
       1,     3,     1,     3,     2,     2,     1,     1,     1,     1,
       3,     1,     2,     3,     4,     5,     1,     3,     2,     3,
       1,     2,     3,     4,     5,     1,     3,     2,     3,     1,
       2,     1,     3,     3,     1,     2,     3,     5,     2,     1,
       2,     3,     2,     6,     1,     2,     2,     3,     1,     2,
       1,     2,     2,     5,     3,     4,     1,     2,     1,     3,
       3,     2,     1,     2,     1,     2,     5,     6,     2,     2,
       3,     1,     6,     1,     5,     5,     1,     2,     1,     2,
       1,     1,     1,     2,     2,     5,     2,     2,     4,     4,
       2,     1,     1,     1,     1,     1,     1,     2,     4,     4,
       4,     1,     4,     3,     5,     4,     6,     3,     1,     3,
       1,     3,     3,     1,     1,     1,     1,     3,     1,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     1,     1,     8,     5,     1,     3,     1,     4,     1,
       1,     1,     3,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     1,     3,     3,     2,     6,     4,     1,
       2,     2,     3,     1,     2,     1,     1,     1,     1,     1,
       5,     5,     2,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     0,     2,     1,     1,     3,     1,
       2,     1,     1,     9,     1,     3,     2,     1,     1,     1,
       2,     5,     6,     1,     1,     2,     1,     3,     4,     1,
       3
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
      yyerror (&yylloc, root, errors, sb, YY_("syntax error: cannot back up")); \
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
                  Type, Value, Location, root, errors, sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (root);
  YYUSE (errors);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, root, errors, sb);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , root, errors, sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, root, errors, sb); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (root);
  YYUSE (errors);
  YYUSE (sb);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 0: /* "end of source"  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1795 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 3: /* UNRECOGNIZED  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1803 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 4: /* ELLIPSIS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1811 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 5: /* INCREMENT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1819 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 6: /* DECIMAL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1827 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 7: /* OCTAL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1835 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 8: /* HEX  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1843 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 9: /* FLOAT_  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1851 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 10: /* EXP_FLOAT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1859 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 11: /* STRING  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1867 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 12: /* ESCAPED_STRING  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1875 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 13: /* IDENTIFIER_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1883 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 14: /* PHYSICAL_IDENTIFIER_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1891 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 15: /* VERSION  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1899 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 16: /* UNTERM_STRING  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1907 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 17: /* UNTERM_ESCAPED_STRING  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1915 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 18: /* VERS_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1923 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 19: /* VERS_2_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1931 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 20: /* KW___no_header  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1939 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 21: /* KW___row_length  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1947 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 22: /* KW___untyped  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1955 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 23: /* KW_alias  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1963 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 24: /* KW_column  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1971 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 25: /* KW_const  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1979 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 26: /* KW_control  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1987 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 27: /* KW_database  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1995 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 28: /* KW_decode  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2003 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 29: /* KW_default  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2011 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 30: /* KW_encode  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2019 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 31: /* KW_extern  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2027 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 32: /* KW_false  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2035 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 33: /* KW_fmtdef  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2043 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 34: /* KW_function  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2051 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 35: /* KW_include  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2059 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 36: /* KW_limit  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2067 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 37: /* KW_physical  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2075 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 38: /* KW_read  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2083 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 39: /* KW_readonly  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2091 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 40: /* KW_return  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2099 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 41: /* KW_schema  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2107 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 42: /* KW_static  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2115 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 43: /* KW_table  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2123 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 44: /* KW_template  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2131 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 45: /* KW_trigger  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2139 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 46: /* KW_true  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2147 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 47: /* KW_type  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2155 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 48: /* KW_typedef  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2163 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 49: /* KW_typeset  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2171 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 50: /* KW_validate  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2179 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 51: /* KW_version  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2187 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 52: /* KW_view  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2195 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 53: /* KW_virtual  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2203 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 54: /* KW_void  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2211 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 55: /* KW_write  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2219 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 56: /* PT_ASTLIST  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2227 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 57: /* PT_PARSE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2235 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 58: /* PT_SOURCE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2243 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 59: /* PT_VERSION_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2251 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 60: /* PT_VERSION_2  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2259 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 61: /* PT_SCHEMA_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2267 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 62: /* PT_SCHEMA_2_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2275 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 63: /* PT_INCLUDE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2283 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 64: /* PT_TYPEDEF  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2291 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 65: /* PT_FQN  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2299 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 66: /* PT_IDENT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2307 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 67: /* PT_PHYSIDENT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2315 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 68: /* PT_UINT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2323 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 69: /* PT_TYPESET  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2331 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 70: /* PT_TYPESETDEF  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2339 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 71: /* PT_FORMAT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2347 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 72: /* PT_CONST  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2355 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 73: /* PT_ALIAS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2363 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 74: /* PT_EXTERN  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2371 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 75: /* PT_FUNCTION  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2379 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 76: /* PT_UNTYPED  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2387 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 77: /* PT_ROWLENGTH  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2395 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 78: /* PT_FUNCDECL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2403 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 79: /* PT_EMPTY  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2411 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 80: /* PT_SCHEMASIG  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2419 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 81: /* PT_SCHEMAFORMAL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2427 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 82: /* PT_RETURNTYPE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2435 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 83: /* PT_FACTSIG  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2443 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 84: /* PT_FUNCSIG  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2451 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 85: /* PT_FUNCPARAMS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2459 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 86: /* PT_FORMALPARAM  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2467 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 87: /* PT_ELLIPSIS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2475 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 88: /* PT_FUNCPROLOGUE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2483 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 89: /* PT_RETURN  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2491 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 90: /* PT_PRODSTMT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2499 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 91: /* PT_PRODTRIGGER  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2507 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 92: /* PT_SCHEMA  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2515 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 93: /* PT_VALIDATE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2523 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 94: /* PT_PHYSICAL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2531 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 95: /* PT_PHYSPROLOGUE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2539 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 96: /* PT_PHYSSTMT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2547 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 97: /* PT_PHYSBODYSTMT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2555 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 98: /* PT_TABLE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2563 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 99: /* PT_TABLEPARENTS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2571 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 100: /* PT_TABLEBODY  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2579 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 101: /* PT_FUNCEXPR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2587 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 102: /* PT_FACTPARMS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2595 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 103: /* PT_COLUMN  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2603 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 104: /* PT_COLUMNEXPR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2611 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 105: /* PT_COLDECL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2619 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 106: /* PT_TYPEDCOL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2627 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 107: /* PT_COLSTMT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2635 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 108: /* PT_DFLTVIEW  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2643 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 109: /* PT_PHYSMBR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2651 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 110: /* PT_PHYSCOL  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2659 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 111: /* PT_PHYSCOLDEF  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2667 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 112: /* PT_COLSCHEMAPARMS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2675 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 113: /* PT_COLSCHEMAPARAM  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2683 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 114: /* PT_COLUNTYPED  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2691 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 115: /* PT_DATABASE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2699 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 116: /* PT_TYPEEXPR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2707 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 117: /* PT_DBBODY  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2715 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 118: /* PT_DBDAD  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2723 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 119: /* PT_DBMEMBER  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2731 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 120: /* PT_TBLMEMBER  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2739 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 121: /* PT_NOHEADER  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2747 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 122: /* PT_CASTEXPR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2755 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 123: /* PT_CONSTVECT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2763 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 124: /* PT_NEGATE  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2771 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 125: /* PT_UNARYPLUS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2779 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 126: /* PT_VERSNAME  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2787 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 127: /* PT_ARRAY  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2795 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 128: /* PT_PHYSENCREF  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2803 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 129: /* PT_TYPEDCOLEXPR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2811 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 130: /* PT_VIEW  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2819 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 131: /* PT_VIEWPARAM  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2827 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 132: /* PT_VIEWPARENTS  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2835 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 133: /* PT_VIEWPARENT  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2843 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 134: /* PT_MEMBEREXPR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2851 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 135: /* PT_JOINEXPR  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2859 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 136: /* ';'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2867 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 137: /* ','  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2875 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 138: /* '{'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2883 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 139: /* '}'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2891 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 140: /* '['  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2899 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 141: /* ']'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2907 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 142: /* '*'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2915 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 143: /* '='  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2923 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 144: /* '('  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2931 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 145: /* ')'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2939 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 146: /* '<'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2947 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 147: /* '>'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2955 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 148: /* '|'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2963 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 149: /* '@'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2971 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 150: /* '-'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2979 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 151: /* '+'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2987 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 152: /* '/'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2995 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 153: /* '.'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3003 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 154: /* ':'  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3011 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 156: /* parse  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3019 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 157: /* source  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3027 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 158: /* version_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3035 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 159: /* schema_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3043 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 160: /* schema_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3051 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 161: /* typedef_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3059 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 162: /* typedef_1_0_new_name_list  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3067 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 163: /* typeset_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3075 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 164: /* typeset_1_0_new_name  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3083 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 165: /* typeset_1_0_def  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3091 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 166: /* typespec_1_0_list  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3099 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 167: /* typespec_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3107 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 168: /* dim_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3115 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 169: /* format_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3123 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 170: /* format_1_0_new_name  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3131 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 171: /* format_1_0_name  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3139 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 172: /* const_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3147 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 173: /* alias_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3155 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 174: /* alias_1_0_new_name  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3163 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 175: /* function_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3171 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 176: /* func_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3179 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 177: /* untyped_func_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3187 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 178: /* row_length_func_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3195 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 179: /* opt_func_1_0_schema_sig  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3203 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 180: /* func_1_0_schema_sig  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3211 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 181: /* func_1_0_schema_formals  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3219 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 182: /* func_1_0_schema_formal  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3227 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 183: /* func_1_0_return_type  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3235 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 184: /* opt_func_1_0_fact_sig  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3243 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 185: /* func_1_0_fact_sig  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3251 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 186: /* func_1_0_fact_signature  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3259 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 187: /* func_1_0_fact_params  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3267 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 188: /* fact_param_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3275 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 189: /* func_1_0_param_sig  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3283 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 190: /* func_1_0_param_signature  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3291 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 191: /* func_1_0_formal_params  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3299 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 192: /* formal_param_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3307 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 193: /* func_1_0_vararg_formals  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3315 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 194: /* func_1_0_prologue  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3323 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 195: /* script_1_0_stmt_seq  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3331 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 196: /* script_1_0_stmt  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3339 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 197: /* extern_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3347 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 198: /* ext_func_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3355 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 199: /* script_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3363 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 200: /* validate_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3371 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 201: /* physical_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3379 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 202: /* phys_1_0_return_type  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3387 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 203: /* phys_1_0_prologue  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3395 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 204: /* phys_1_0_body  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3403 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 205: /* phys_1_0_body_stmt  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3411 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 206: /* phys_1_0_stmt  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3419 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 207: /* table_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3427 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 208: /* opt_tbl_1_0_parents  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3435 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 209: /* tbl_1_0_parents  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3443 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 210: /* tbl_1_0_body  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3451 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 211: /* tbl_1_0_stmt_seq  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3459 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 212: /* tbl_1_0_stmt  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3467 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 213: /* production_1_0_stmt  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3475 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 214: /* col_1_0_modifiers  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3483 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 215: /* col_1_0_modifier_seq  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3491 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 216: /* col_1_0_modifier  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3499 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 217: /* col_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3507 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 218: /* phys_enc_ref  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3515 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 219: /* typed_column_decl_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3523 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 220: /* col_ident  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3531 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 221: /* phys_ident  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3539 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 222: /* opt_column_body_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3547 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 223: /* column_body_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3555 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 224: /* column_stmt_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3563 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 225: /* default_view_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3571 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 226: /* physmbr_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3579 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 227: /* phys_coldef_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3587 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 228: /* opt_col_schema_parms_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3595 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 229: /* col_schema_parms_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3603 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 230: /* col_schema_parm_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3611 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 231: /* col_schema_value_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3619 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 232: /* cond_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3627 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 233: /* expression_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3635 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 234: /* primary_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3643 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 235: /* func_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3651 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 236: /* schema_parms_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3659 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 237: /* schema_parm_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3667 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 238: /* opt_factory_parms_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3675 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 239: /* factory_parms_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3683 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 240: /* factory_parms  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3691 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 241: /* opt_func_1_0_parms  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3699 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 242: /* func_1_0_parms  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3707 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 243: /* uint_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3715 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 244: /* float_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3723 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 245: /* string_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3731 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 246: /* const_vect_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3739 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 247: /* opt_const_vect_exprlist_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3747 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 248: /* const_vect_exprlist_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3755 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 249: /* bool_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3763 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 250: /* type_expr_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3771 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 251: /* member_expr_2_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3779 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 252: /* join_expr_2_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3787 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 253: /* database_1_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3795 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 254: /* opt_database_dad_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3803 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 255: /* database_body_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3811 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 256: /* database_members_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3819 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 257: /* database_member_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3827 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 258: /* opt_template_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3835 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 259: /* db_member_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3843 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 260: /* table_member_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3851 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 261: /* include_directive  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3859 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 262: /* fqn_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3867 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 263: /* ident_1_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3875 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 264: /* empty  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3883 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 265: /* fqn_vers  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3891 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 266: /* fqn_opt_vers  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3899 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 267: /* version_2_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3907 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 268: /* schema_2_0  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3915 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 269: /* schema_2_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3923 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 270: /* view_2_0_decl  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3931 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 271: /* view_parms  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3939 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 272: /* view_parm  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3947 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 273: /* opt_view_body  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3955 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 274: /* view_body  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3963 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 275: /* view_member  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3971 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 276: /* opt_view_parents  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3979 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 277: /* view_parents  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3987 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 278: /* view_parent  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3995 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 279: /* view_parent_parms  */
#line 138 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 4003 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
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
#line 298 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root = MakeTree ( PT_PARSE, T ( (yyvsp[0]) ) );              (yyval) . subtree = 0; (yyval) . leading_ws = 0;  }
#line 4297 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 299 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root = MakeTree ( PT_PARSE, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) );    (yyval) . subtree = 0; (yyval) . leading_ws = 0; }
#line 4303 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 303 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[0]) ) ); }
#line 4309 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 304 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4315 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 305 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4321 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 311 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_1_0, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4327 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 315 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_1_0, P ( (yyvsp[0]) ) ); }
#line 4333 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 316 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 4339 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 320 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4345 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 321 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4351 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 322 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4357 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 323 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4363 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 324 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4369 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 325 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4375 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 326 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4381 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 327 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4387 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 328 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4393 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 329 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4399 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 330 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4405 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 331 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4411 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 332 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4417 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 333 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4423 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 343 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDEF, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4429 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 347 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4435 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 348 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P((yyvsp[-2])), T((yyvsp[-1])), P((yyvsp[0])) ); }
#line 4441 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 355 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESET, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4447 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 359 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4453 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 364 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESETDEF, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4459 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 370 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4465 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 371 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4471 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 375 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4477 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 376 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4483 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 380 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4489 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 381 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4495 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 388 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4501 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 390 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4507 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 394 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4513 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 398 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4519 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 406 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4525 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 413 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ALIAS, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4531 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 417 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4537 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 423 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCTION, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4543 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 427 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4549 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 428 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4555 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 435 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCDECL, P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4561 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 440 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNTYPED, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4567 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 445 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ROWLENGTH, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4573 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 449 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4579 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 450 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4585 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 454 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMASIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4591 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 458 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4597 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 459 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4603 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 463 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4609 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 464 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4615 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 468 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, T ( (yyvsp[0]) ) ); }
#line 4621 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 469 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, P ( (yyvsp[0]) ) ); }
#line 4627 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 473 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4633 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 474 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4639 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 478 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4645 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 482 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4651 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 484 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4657 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 486 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4663 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 488 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4669 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 490 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4675 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 494 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4681 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 495 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4687 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 499 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4693 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 503 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4699 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 507 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4705 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 509 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4711 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 511 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4717 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 513 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4723 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 515 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4729 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 519 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4735 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 520 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4741 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 524 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4747 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 526 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4753 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 530 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4759 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 531 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ELLIPSIS, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4765 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 536 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[0]) ) ); }
#line 4771 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 538 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4777 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 540 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4783 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 544 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4789 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 545 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4795 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 549 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURN, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4801 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 551 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4807 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 558 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EXTERN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4813 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 562 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4819 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 570 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4825 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 572 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4831 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 580 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VALIDATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4837 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 592 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSICAL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4843 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 596 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4849 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 597 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NOHEADER, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4855 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 602 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4861 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 604 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4867 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 608 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4873 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 609 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4879 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 614 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[0]) ) ); }
#line 4885 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 616 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4891 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 618 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4897 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 620 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4903 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 625 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSSTMT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4909 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 635 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4915 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 639 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4921 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 640 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4927 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 644 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4933 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 645 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4939 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 650 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4945 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 652 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4951 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 656 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4957 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 657 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4963 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 661 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4969 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 662 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4975 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 664 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4981 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 666 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4987 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 667 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4993 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 668 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4999 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 669 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5005 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 670 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5011 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 672 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUNTYPED, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5017 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 673 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5023 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 678 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5029 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 680 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODTRIGGER, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5035 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 684 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5041 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 685 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); AddToList ( P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5047 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 689 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5053 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 690 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5059 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 694 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5065 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 695 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5071 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 696 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5077 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 701 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5083 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 703 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5089 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 708 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5095 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 710 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5101 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 712 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5107 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 717 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5113 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 719 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOLEXPR, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5119 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 721 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5125 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 725 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5131 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 726 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5137 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 730 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5143 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 734 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5149 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 735 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5155 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 739 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5161 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 740 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5167 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 744 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5173 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 745 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5179 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 746 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5185 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 747 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5191 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 752 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DFLTVIEW, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5197 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 757 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5203 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 759 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5209 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 761 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5215 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 763 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5221 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 768 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOLDEF, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5227 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 772 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5233 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 773 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5239 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 777 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5245 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 778 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5251 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 782 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5257 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 783 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5263 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 787 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5269 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 788 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5275 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 795 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5281 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 796 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5287 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 800 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5293 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 801 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CASTEXPR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5299 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 805 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5305 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 806 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5311 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 807 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5317 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 808 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5323 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 809 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5329 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 810 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5335 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 811 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5341 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 812 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5347 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 813 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5353 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 814 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5359 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 815 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNARYPLUS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5365 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 816 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5371 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 817 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5377 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 827 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, T ( (yyvsp[-7]) ), P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5383 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 833 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5389 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 837 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5395 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 838 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5401 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 842 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5407 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 843 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5413 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 844 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5419 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 848 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5425 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 849 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5431 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 853 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5437 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 857 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5443 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 858 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5449 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 862 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5455 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 863 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5461 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 867 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5467 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 868 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5473 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 872 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5479 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 873 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5485 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 874 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5491 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 878 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5497 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 879 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5503 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 883 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5509 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 884 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5515 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 888 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONSTVECT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5521 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 892 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5527 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 893 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5533 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 897 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5539 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 898 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5545 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 902 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5551 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 903 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5557 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 907 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5563 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 908 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1])), P ( (yyvsp[0]) ) ); }
#line 5569 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 912 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_MEMBEREXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5575 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 913 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_MEMBEREXPR, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5581 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 918 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_JOINEXPR, P ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ((yyvsp[0]) ) ); }
#line 5587 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 928 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2])), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5593 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 932 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5599 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 933 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBDAD, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5605 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 937 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5611 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 938 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5617 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 942 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5623 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 943 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5629 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 947 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5635 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 948 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5641 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 949 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5647 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 953 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5653 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 954 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5659 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 959 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5665 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 964 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TBLMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5671 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 971 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5677 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 979 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FQN, P ( (yyvsp[0]) ) ); }
#line 5683 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 980 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5689 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 982 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5695 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 983 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5701 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 984 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5707 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 985 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5713 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 986 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5719 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 987 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5725 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 988 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5731 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 989 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5737 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 993 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 5743 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 997 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EMPTY ); }
#line 5749 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1001 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSNAME, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5755 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1005 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5761 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1006 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5767 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1012 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_2, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5773 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1016 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_2_0, P ( (yyvsp[0]) ) ); }
#line 5779 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1017 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 5785 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1021 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5791 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1022 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5797 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1027 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEW, T ( (yyvsp[-8]) ), P ( (yyvsp[-7]) ), T ( (yyvsp[-6]) ), P ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5803 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1031 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5809 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1032 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5815 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1036 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARAM, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5821 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1040 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5827 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1041 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5833 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1045 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5839 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1046 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5845 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1051 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5851 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1053 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5857 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1054 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5863 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1058 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5869 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1059 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5875 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1063 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5881 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1064 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5887 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1069 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARENT, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5893 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1073 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5899 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1074 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5905 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 5909 "/2T/data/altdevel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
      yyerror (&yylloc, root, errors, sb, YY_("syntax error"));
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
        yyerror (&yylloc, root, errors, sb, yymsgp);
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
                      yytoken, &yylval, &yylloc, root, errors, sb);
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
                  yystos[yystate], yyvsp, yylsp, root, errors, sb);
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
  yyerror (&yylloc, root, errors, sb, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, root, errors, sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, root, errors, sb);
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
