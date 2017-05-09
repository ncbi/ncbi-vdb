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
        p_errors -> ReportError ("Line %i pos %i: %s", p_llocp -> first_line, p_llocp -> first_column, p_msg);
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



#line 172 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:339  */

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

#endif /* !YY_SCHEMA_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 366 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   625

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  148
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  112
/* YYNRULES -- Number of rules.  */
#define YYNRULES  247
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  472

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
       0,   287,   287,   288,   292,   293,   294,   298,   302,   306,
     312,   313,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   336,   344,   345,   351,
     356,   360,   367,   368,   372,   373,   377,   378,   384,   386,
     391,   395,   402,   409,   414,   420,   424,   425,   426,   436,
     441,   446,   447,   451,   455,   456,   460,   461,   465,   466,
     470,   471,   475,   479,   480,   482,   484,   486,   491,   492,
     496,   500,   504,   505,   507,   509,   511,   516,   517,   521,
     522,   527,   528,   532,   534,   536,   541,   542,   546,   547,
     554,   559,   566,   568,   576,   583,   593,   594,   598,   600,
     605,   606,   610,   612,   614,   616,   621,   628,   636,   637,
     641,   642,   646,   648,   653,   654,   658,   659,   660,   662,
     664,   665,   666,   667,   668,   670,   674,   676,   681,   682,
     686,   687,   691,   692,   693,   697,   699,   704,   706,   708,
     713,   715,   717,   722,   723,   727,   731,   732,   736,   737,
     741,   742,   743,   744,   748,   753,   755,   757,   759,   764,
     769,   770,   774,   775,   779,   780,   784,   785,   792,   793,
     797,   798,   802,   803,   804,   805,   806,   807,   808,   809,
     810,   811,   812,   816,   823,   832,   833,   837,   838,   839,
     843,   844,   848,   849,   853,   854,   858,   859,   863,   864,
     865,   869,   870,   874,   875,   879,   883,   884,   888,   889,
     893,   894,   898,   899,   900,   901,   905,   906,   912,   920,
     921,   925,   926,   930,   931,   935,   936,   937,   941,   942,
     946,   951,   959,   967,   968,   970,   971,   972,   973,   974,
     975,   976,   977,   981,   985,   989,   993,   994
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
  "PT_AT", "PT_PHYSENCREF", "PT_TYPEDCOLEXPR", "';'", "'$'", "','", "'{'",
  "'}'", "'['", "']'", "'*'", "'='", "'('", "')'", "'<'", "'>'", "'|'",
  "'@'", "'+'", "'-'", "'/'", "':'", "$accept", "parse", "source",
  "version_1_0", "version_2_x", "schema_2_x", "schema_1_0",
  "schema_1_0_decl", "typedef_1_0_decl", "typedef_1_0_new_name_list",
  "typeset_1_0_decl", "typeset_1_0_new_name", "typeset_1_0_def",
  "typespec_1_0_list", "typespec_1_0", "dim_1_0", "format_1_0_decl",
  "format_1_0_new_name", "format_1_0_name", "const_1_0_decl",
  "alias_1_0_decl", "alias_1_0_new_name", "function_1_0_decl",
  "func_1_0_decl", "untyped_func_1_0_decl", "row_length_func_1_0_decl",
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
  "column_body_1_0_opt", "column_body_1_0", "column_stmt_1_0",
  "default_view_1_0_decl", "physmbr_1_0_decl", "phys_coldef_1_0",
  "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
  "col_schema_parm_1_0", "col_schema_value_1_0", "cond_expr_1_0",
  "expression_1_0", "primary_expr_1_0", "func_expr_1_0",
  "schema_parms_1_0", "schema_parm_1_0", "opt_factory_parms_1_0",
  "factory_parms_1_0", "opt_func_1_0_parms", "func_1_0_parms",
  "uint_expr_1_0", "float_expr_1_0", "string_expr_1_0",
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

#define YYPACT_NINF -295

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-295)))

#define YYTABLE_NINF -247

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     441,  -295,    64,    64,    64,    20,    64,    -4,   151,    42,
      24,    64,    64,    64,    20,    91,  -295,   134,   194,   496,
      80,   496,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,    -6,  -295,    64,
      40,    16,    69,  -295,  -295,   127,    64,    -2,    64,    64,
       9,  -295,  -295,  -295,    33,  -295,  -295,  -295,    68,    -4,
    -295,   125,    -6,   142,   148,  -295,   185,   187,  -295,  -295,
     496,  -295,  -295,  -295,   534,   188,   148,   -22,   210,  -295,
      64,   219,  -295,  -295,   229,   148,    39,    65,    64,  -295,
     140,  -295,    64,   -17,  -295,    64,  -295,    33,  -295,    64,
    -295,    64,   237,  -295,    99,  -295,    64,   252,  -295,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
     424,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,   424,  -295,    64,   404,  -295,   424,   365,   255,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,    -5,
    -295,   242,    16,  -295,    76,  -295,  -295,   254,   264,  -295,
       9,  -295,  -295,    64,   265,  -295,   265,   276,  -295,   457,
    -295,  -295,    64,   146,  -295,  -295,   284,  -295,   279,   285,
    -295,   287,   163,  -295,  -295,    41,  -295,  -295,  -295,  -295,
     148,  -295,   424,   290,  -295,  -295,  -295,  -295,   118,  -295,
      81,  -295,  -295,  -295,  -295,  -295,  -295,   148,     7,   301,
    -295,  -295,   141,    64,   288,   306,   373,  -295,    38,  -295,
      57,    64,  -295,  -295,    64,   483,  -295,  -295,     0,   215,
    -295,  -295,  -295,    64,  -295,  -295,  -295,   424,   424,   404,
      64,   210,  -295,   165,   424,  -295,  -295,    64,    64,    64,
     427,   302,   184,  -295,  -295,     1,   233,    75,   310,  -295,
    -295,    64,   409,   309,   434,   307,   404,  -295,   435,    64,
    -295,    38,  -295,   311,   314,  -295,  -295,   404,   286,  -295,
     286,    26,   242,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
     242,   317,   424,  -295,  -295,   315,   325,  -295,    64,    64,
     326,  -295,  -295,    15,    64,  -295,  -295,    64,   158,   445,
     320,   204,  -295,  -295,  -295,    71,    64,  -295,   327,   310,
     310,  -295,    36,  -295,    71,  -295,    95,   329,   424,   339,
     448,   166,  -295,  -295,  -295,   171,   164,   242,  -295,   424,
     424,   182,  -295,   267,  -295,  -295,  -295,   424,  -295,   334,
    -295,  -295,  -295,   424,   347,   350,   308,  -295,  -295,    64,
    -295,   326,   469,   353,  -295,  -295,    17,   158,  -295,   424,
      35,  -295,    64,   -41,    64,  -295,  -295,  -295,  -295,    37,
     355,   424,   357,  -295,   247,   404,  -295,   404,  -295,   424,
    -295,    47,  -295,    50,    64,  -295,     2,   424,   225,   424,
    -295,  -295,  -295,   326,  -295,  -295,   101,  -295,   158,  -295,
     353,    56,  -295,  -295,   360,  -295,   195,  -295,   363,   370,
    -295,  -295,   424,  -295,  -295,   148,    97,  -295,   424,  -295,
     242,   366,   368,   371,  -295,   374,     2,  -295,  -295,    98,
    -295,   375,  -295,   353,  -295,  -295,   424,   376,  -295,  -295,
     102,  -295,  -295,  -295,   414,   424,   424,  -295,  -295,  -295,
    -295,  -295,   123,  -295,  -295,   380,   138,   143,  -295,  -295,
    -295,  -295
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   244,     0,   244,
     244,     0,     0,     0,     0,     0,    25,     0,     0,     0,
       0,     4,    10,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,   243,     0,   233,     0,
      34,     0,   244,    91,    90,     0,     0,    40,     0,     0,
       0,    45,    46,    47,     0,    52,    51,   232,     0,   244,
      92,   244,     0,     0,    30,    94,     0,     0,     1,     3,
       5,     9,     6,    11,     0,     0,    44,     0,     0,   245,
       0,     0,   219,    38,     0,    40,     0,     0,     0,   216,
       0,    54,     0,    34,    58,     0,    59,     0,    96,     0,
      93,     0,     0,   108,     0,    27,     0,     0,     8,     7,
     235,   236,   237,   238,   239,   240,   241,   242,   234,    43,
       0,   198,   200,   199,   201,   202,   203,   204,   145,   211,
     210,   244,    37,     0,     0,   174,     0,     0,     0,   173,
      36,   170,   175,   176,   177,   178,   179,   180,   181,   172,
     247,   244,   246,   220,   244,   218,    39,     0,     0,    56,
       0,    53,    57,     0,   244,    97,   244,   109,   110,     0,
     107,    26,     0,     0,    32,    29,     0,   208,     0,   207,
     206,     0,     0,   185,   189,   187,   182,   213,   214,   215,
     212,    35,     0,     0,   190,   229,   227,   221,   244,   223,
       0,   225,   226,   228,    50,    49,    55,   217,   244,     0,
      61,    60,     0,     0,     0,   128,   132,   133,   244,   134,
     244,     0,   125,   113,     0,     0,   114,   116,     0,     0,
     130,   123,    28,     0,    31,    42,   205,     0,     0,     0,
       0,     0,   192,     0,   244,   222,   224,     0,     0,     0,
       0,     0,   244,    68,    63,   244,     0,     0,     0,    95,
     111,     0,     0,     0,     0,   244,     0,   121,     0,     0,
     160,   244,   120,     0,     0,   112,   115,     0,     0,   117,
       0,    34,   244,   129,   132,   131,    33,   209,   171,   186,
     244,     0,     0,   191,   196,     0,   195,   194,     0,     0,
     244,    70,    62,     0,     0,    64,    81,     0,     0,     0,
       0,   244,    77,    72,    83,     0,     0,    48,     0,     0,
       0,   102,     0,   100,     0,    98,     0,     0,     0,     0,
       0,     0,   162,   165,   167,   166,     0,   244,   122,     0,
       0,     0,   135,     0,   144,   143,   136,     0,   138,     0,
     188,   193,   184,     0,     0,     0,     0,    65,    82,     0,
      69,   244,     0,   244,    79,    71,     0,     0,    73,     0,
       0,    86,     0,     0,     0,   103,   104,    99,   101,     0,
       0,     0,     0,   154,     0,     0,   161,     0,   155,     0,
     159,     0,   168,     0,     0,   142,   244,     0,     0,   244,
     197,   230,   231,   244,    66,    80,     0,    74,     0,    78,
     244,     0,    85,    87,     0,    84,     0,   106,     0,     0,
     118,   157,     0,   163,   164,   166,     0,   127,     0,   126,
     244,     0,     0,     0,   153,     0,   147,   148,   146,     0,
     139,     0,    67,   244,    75,    88,     0,     0,   124,   119,
       0,   156,   169,   137,     0,     0,     0,   140,   149,   141,
     183,    76,     0,   105,   158,     0,     0,     0,    89,   152,
     150,   151
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -295,  -295,  -295,  -295,  -295,  -295,   491,    28,  -295,  -295,
    -295,  -295,  -295,  -295,    94,   271,  -295,   470,  -295,  -295,
    -295,  -295,   333,    54,  -295,  -295,   508,  -295,  -295,   377,
     -15,   359,  -295,  -295,  -231,  -265,  -295,  -295,  -200,  -294,
    -271,  -295,   199,   -11,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,  -295,   209,    44,  -295,  -295,  -295,  -295,  -295,   316,
    -295,  -295,  -295,   304,  -295,  -295,   259,  -295,  -132,  -295,
    -295,   104,  -295,  -205,   269,  -295,  -295,   150,   155,   -37,
     -42,  -295,  -295,   272,   312,  -278,   201,   147,  -295,  -128,
     413,  -295,  -295,  -295,  -295,  -295,  -295,   234,  -295,  -295,
    -295,  -295,   354,  -295,  -295,  -295,  -295,    21,    11,    -7,
      -3,   -33
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    72,    21,    22,    23,   104,
      24,    63,   107,   173,    89,   138,    25,    45,    46,    26,
      27,    75,    28,    51,    52,    53,    54,    55,    90,    91,
      95,   209,   210,   251,   252,   253,   256,   310,   311,   312,
     305,   317,   370,   371,    29,    44,    30,    31,    32,    99,
     259,   322,   323,   325,    33,   102,   167,   170,   225,   226,
     227,   228,   229,   230,   279,   280,   342,   343,   139,   435,
     436,   437,   231,   267,   268,   269,   331,   332,   333,   391,
     392,   141,   142,   182,   183,   193,   243,   295,   296,   143,
     144,   145,   146,   178,   179,   147,   148,   372,    34,    81,
     155,   198,   199,   200,   201,   202,    35,   149,    38,   306,
     150,   151
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,    42,    56,    56,   348,   187,   184,    36,    61,   188,
      79,   -41,   349,    36,    36,   272,    48,    49,   300,   358,
      36,   358,    36,    37,    40,    41,   307,    47,    36,   357,
      36,    79,    41,    62,    64,    82,   140,   431,   360,   432,
     368,    79,   307,    98,    48,    49,    36,   153,    36,    73,
      36,   433,    56,     7,   103,    88,   318,    59,    76,   390,
      77,   265,   164,   319,    60,   320,   338,    85,   168,    86,
      87,    93,   409,   361,   369,    93,   369,    36,   176,    93,
     265,    36,   165,    40,    36,   118,    94,    97,   415,   177,
     404,   360,   407,   271,   186,   318,   166,    39,    73,   159,
      66,   152,   319,   162,   320,   358,    74,   247,   363,    67,
     369,   184,   409,   100,    36,   120,   152,    78,    93,   195,
      41,    94,   152,   248,   180,    74,   307,    40,   403,   163,
      74,   434,   442,  -246,    68,  -246,    50,   308,   334,   444,
     277,    74,    74,   249,   194,    74,   344,   203,   344,   184,
     242,   359,   453,   408,    93,   185,   105,   211,   190,   211,
      78,   195,    57,    74,    50,   321,   347,   410,   412,   377,
     417,    36,   461,    74,    78,   241,   427,   157,   266,   429,
     260,    93,    50,   307,   207,   445,    74,    74,    74,   428,
      40,   203,   428,    40,    69,   287,   288,   266,   428,   140,
     174,   254,   294,   158,   321,   196,    80,   290,   443,   197,
      71,   270,    74,   270,   298,   299,   121,   122,   123,   124,
     125,   126,   127,    36,   128,   282,   451,   459,   171,    40,
     172,   464,   273,   380,   152,   274,   337,   297,   283,   428,
     428,   129,    74,   284,   428,   217,    40,   196,   313,   281,
     351,   245,   468,   219,    40,   130,    83,   334,   270,   334,
     185,   152,   101,   224,   270,   428,   232,   470,   152,   152,
      40,   160,   471,   257,   106,   194,    40,   233,   258,   234,
     428,   161,   326,   194,    92,   428,   382,   335,    96,   345,
     152,   345,    96,   388,   239,    74,   292,   385,   185,    36,
     128,   389,   250,   393,   240,   242,   293,   386,   387,   354,
     355,   400,   358,   239,   108,   303,   109,   119,    74,   224,
     304,    36,   278,   394,    40,    40,   465,   286,    40,    40,
     194,    96,   411,   447,   262,   366,    93,   373,    43,   419,
     367,   263,    74,   250,   131,    93,   132,    65,   133,   309,
     134,   154,   426,   135,   136,   137,   292,   294,   156,   413,
     439,   430,   314,   375,   376,   315,   440,   181,   413,   169,
     316,   121,   122,   123,   124,   125,   421,    40,    36,   128,
      40,   175,   192,   414,   422,   450,   452,    40,    40,   438,
     191,    93,   297,   204,    92,   416,   395,   250,   250,   396,
      93,   362,   309,   205,   397,   208,   335,   213,   425,   462,
     121,   122,   123,   235,   236,   152,   237,    36,   466,   467,
     121,   122,   123,   194,   264,   261,   238,    40,   244,    40,
     121,   122,   123,   124,   125,   126,   127,    36,   128,   255,
     301,     1,   324,   302,   327,   329,   328,   266,   339,   336,
     250,   340,   350,   250,   352,   129,   353,   356,   364,   365,
     309,   309,   384,     2,   374,     3,   381,     4,   383,   130,
      36,     5,   399,     6,     7,     8,   401,     9,   214,   402,
     215,    10,   405,    11,   406,   216,   420,   217,    12,    13,
      14,    15,   448,   218,   418,   219,    36,   446,   220,   449,
     309,   221,   309,   454,   214,   455,   215,   457,   456,   469,
      70,   216,   291,   217,   460,   463,    84,    58,     2,   218,
       3,   219,     4,   379,   220,   212,     5,   221,     6,     7,
       8,   378,     9,   285,   330,   423,    10,   206,    11,   346,
     458,   276,   424,    12,    13,    14,   441,    36,   398,   341,
     189,   289,   246,     0,     0,     0,     0,     0,   131,     0,
     110,   111,   133,   112,   134,     0,     0,   135,   136,   137,
      16,   113,     0,     0,     0,     0,   114,     0,     0,     0,
     115,     0,     0,     0,     0,   116,   222,     0,   117,     0,
     223,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   222,     0,     0,     0,   275,     0,     0,     0,
       0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       7,     4,     9,    10,   282,   137,   134,    13,    11,   137,
      15,    13,   290,    13,    13,   220,    20,    21,   249,     4,
      13,     4,    13,     2,     3,     4,    25,     6,    13,   300,
      13,    15,    11,    12,    13,    42,    78,    35,   303,    37,
     311,    15,    25,    58,    20,    21,    13,    80,    13,    21,
      13,    49,    59,    33,    61,    46,    20,    33,    37,   337,
      39,    23,    95,    27,    10,    29,   271,    46,   101,    48,
      49,    50,   366,   304,    39,    54,    39,    13,   120,    58,
      23,    13,    97,    62,    13,    74,    53,    19,   129,   131,
     361,   356,   363,    36,   136,    20,    99,     3,    70,    88,
       9,    80,    27,    92,    29,     4,   147,    26,   308,    18,
      39,   239,   406,    59,    13,   137,    95,   134,    97,    43,
      99,    53,   101,    42,   131,   147,    25,   106,   359,   146,
     147,   129,   403,   138,     0,   140,   140,   136,   266,   410,
     140,   147,   147,   136,   151,   147,   278,   154,   280,   277,
     192,   136,   430,   136,   133,   134,    62,   164,   137,   166,
     134,    43,    11,   147,   140,   129,   140,   367,   133,   133,
     133,    13,   443,   147,   134,   134,   129,   138,   140,   129,
     213,   160,   140,    25,   163,   129,   147,   147,   147,   142,
     169,   198,   142,   172,     0,   237,   238,   140,   142,   241,
     106,   208,   244,   138,   129,   129,   137,   240,   408,   133,
     130,   218,   147,   220,   247,   248,     6,     7,     8,     9,
      10,    11,    12,    13,    14,   228,   129,   129,   129,   208,
     131,   129,   221,   138,   213,   224,   269,   244,    23,   142,
     142,    31,   147,    28,   142,    30,   225,   129,   255,   228,
     292,   133,   129,    38,   233,    45,   129,   385,   265,   387,
     239,   240,   137,   169,   271,   142,   172,   129,   247,   248,
     249,   131,   129,   132,   132,   282,   255,   131,   137,   133,
     142,   141,   261,   290,    50,   142,   328,   266,    54,   278,
     269,   280,    58,   129,   131,   147,   131,   131,   277,    13,
      14,   137,   208,   340,   141,   347,   141,   141,   137,   298,
     299,   353,     4,   131,   129,   131,   129,   129,   147,   225,
     136,    13,   228,   141,   303,   304,   454,   233,   307,   308,
     337,    97,   369,   138,    28,   131,   315,   316,     5,   381,
     136,    35,   147,   249,   134,   324,   136,    14,   138,   255,
     140,   132,   389,   143,   144,   145,   131,   399,   129,   370,
     397,   394,   129,   319,   320,   132,   141,   133,   379,   132,
     137,     6,     7,     8,     9,    10,   129,   356,    13,    14,
     359,   129,   140,   372,   137,   422,   428,   366,   367,   396,
     135,   370,   399,   139,   160,   374,   129,   303,   304,   132,
     379,   307,   308,   139,   137,   140,   385,   131,   387,   446,
       6,     7,     8,   129,   135,   394,   131,    13,   455,   456,
       6,     7,     8,   430,    51,   137,   139,   406,   138,   408,
       6,     7,     8,     9,    10,    11,    12,    13,    14,   138,
      13,     0,   132,   141,    35,    11,   137,   140,   137,    14,
     356,   137,   135,   359,   139,    31,   131,   131,    13,   139,
     366,   367,    14,    22,   137,    24,   137,    26,   129,    45,
      13,    30,   138,    32,    33,    34,   129,    36,    21,   129,
      23,    40,    13,    42,   131,    28,   129,    30,    47,    48,
      49,    50,   129,    36,   139,    38,    13,   137,    41,   129,
     406,    44,   408,   137,    21,   137,    23,   133,   137,   129,
      19,    28,   241,    30,   139,   139,    46,     9,    22,    36,
      24,    38,    26,   324,    41,   166,    30,    44,    32,    33,
      34,   322,    36,   229,   265,   385,    40,   160,    42,   280,
     436,   225,   387,    47,    48,    49,   399,    13,   347,   277,
     137,   239,   198,    -1,    -1,    -1,    -1,    -1,   134,    -1,
      26,    27,   138,    29,   140,    -1,    -1,   143,   144,   145,
     129,    37,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      46,    -1,    -1,    -1,    -1,    51,   129,    -1,    54,    -1,
     133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,    -1,    -1,    -1,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129
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
     228,   229,   230,   237,   238,   239,   240,   243,   244,   255,
     258,   259,   255,   259,   132,   248,   129,   138,   138,   256,
     131,   141,   256,   146,   259,   178,   258,   204,   259,   132,
     205,   129,   131,   161,   162,   129,   228,   228,   241,   242,
     257,   245,   231,   232,   237,   255,   228,   216,   237,   238,
     255,   135,   140,   233,   257,    43,   129,   133,   249,   250,
     251,   252,   253,   257,   139,   139,   177,   255,   140,   179,
     180,   257,   179,   131,    21,    23,    28,    30,    36,    38,
      41,    44,   129,   133,   162,   206,   207,   208,   209,   210,
     211,   220,   162,   131,   133,   129,   135,   131,   139,   131,
     141,   134,   228,   234,   138,   133,   250,    26,    42,   136,
     162,   181,   182,   183,   257,   138,   184,   132,   137,   198,
     259,   137,    28,    35,    51,    23,   140,   221,   222,   223,
     257,    36,   221,   256,   256,   133,   207,   140,   162,   212,
     213,   255,   258,    23,    28,   211,   162,   228,   228,   232,
     259,   163,   131,   141,   228,   235,   236,   257,   259,   259,
     182,    13,   141,   131,   136,   188,   257,    25,   136,   162,
     185,   186,   187,   257,   129,   132,   137,   189,    20,    27,
      29,   129,   199,   200,   132,   201,   255,    35,   137,    11,
     222,   224,   225,   226,   237,   255,    14,   259,   221,   137,
     137,   231,   214,   215,   216,   256,   214,   140,   233,   233,
     135,   228,   139,   131,   256,   256,   131,   188,     4,   136,
     183,   182,   162,   186,    13,   139,   131,   136,   188,    39,
     190,   191,   245,   255,   137,   201,   201,   133,   200,   190,
     138,   137,   228,   129,    14,   131,   141,   137,   129,   137,
     233,   227,   228,   227,   141,   129,   132,   137,   234,   138,
     228,   129,   129,   182,   188,    13,   131,   188,   136,   187,
     186,   227,   133,   191,   256,   129,   255,   133,   139,   228,
     129,   129,   137,   225,   226,   255,   227,   129,   142,   129,
     259,    35,    37,    49,   129,   217,   218,   219,   257,   227,
     141,   235,   188,   186,   188,   129,   137,   138,   129,   129,
     227,   129,   228,   233,   137,   137,   137,   133,   219,   129,
     139,   188,   227,   139,   129,   237,   227,   227,   129,   129,
     129,   129
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
     207,   207,   207,   207,   207,   207,   208,   208,   209,   209,
     210,   210,   211,   211,   211,   212,   212,   213,   213,   213,
     214,   214,   214,   215,   215,   216,   217,   217,   218,   218,
     219,   219,   219,   219,   220,   221,   221,   221,   221,   222,
     223,   223,   224,   224,   225,   225,   226,   226,   227,   227,
     228,   228,   229,   229,   229,   229,   229,   229,   229,   229,
     229,   229,   229,   230,   230,   231,   231,   232,   232,   232,
     233,   233,   234,   234,   235,   235,   236,   236,   237,   237,
     237,   238,   238,   239,   239,   240,   241,   241,   242,   242,
     243,   243,   244,   244,   244,   244,   245,   245,   246,   247,
     247,   248,   248,   249,   249,   250,   250,   250,   251,   251,
     252,   253,   254,   255,   255,   255,   255,   255,   255,   255,
     255,   255,   255,   256,   257,   258,   259,   259
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
       1,     3,     3,     2,     1,     2,     1,     2,     5,     6,
       2,     2,     3,     1,     6,     1,     5,     5,     1,     2,
       1,     2,     1,     1,     1,     2,     2,     5,     2,     4,
       4,     4,     2,     1,     1,     1,     1,     1,     1,     2,
       4,     4,     4,     1,     4,     3,     5,     4,     6,     3,
       1,     3,     1,     3,     3,     1,     1,     1,     1,     3,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     8,     5,     1,     3,     1,     4,     1,
       1,     3,     1,     3,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     3,
       1,     1,     2,     2,     2,     2,     1,     3,     4,     1,
       2,     2,     3,     1,     2,     1,     1,     1,     1,     1,
       5,     5,     2,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     0,     2,     1,     1
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
  YYUSE (yytype);
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
#line 287 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, T ( (yyvsp[0]) ) ); }
#line 1989 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 288 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 1995 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[0]) ) ); }
#line 2001 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 293 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2007 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 294 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2013 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 298 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_1_0, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2019 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 302 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_2, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2025 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 306 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2031 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 312 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_1_0, P ( (yyvsp[0]) ) ); }
#line 2037 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 2043 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 317 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2049 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 318 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2055 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 319 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2061 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 320 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2067 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2073 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 322 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2079 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2085 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 324 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2091 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 325 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2097 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 326 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2103 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 327 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2109 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 328 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2115 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 329 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2121 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 330 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2127 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 340 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDEF, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2133 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 344 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2139 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 345 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P((yyvsp[-2])), T((yyvsp[-1])), P((yyvsp[0])) ); }
#line 2145 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 352 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESET, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2151 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 356 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2157 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESETDEF, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2163 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2169 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 368 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2175 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2181 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 373 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2187 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 377 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2193 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 378 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2199 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 385 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2205 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 387 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2211 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 391 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2217 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 395 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2223 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 403 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2229 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 410 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ALIAS, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2235 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 414 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2241 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 420 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCTION, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2247 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 424 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2253 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 425 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2259 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 432 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCDECL, P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2265 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 437 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNTYPED, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2271 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 442 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ROWLENGTH, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2277 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 446 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2283 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 447 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2289 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 451 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMASIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2295 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 455 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2301 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 456 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2307 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 461 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2319 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, T ( (yyvsp[0]) ) ); }
#line 2325 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 466 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, P ( (yyvsp[0]) ) ); }
#line 2331 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 470 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2337 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 471 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2343 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 475 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2349 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 479 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2355 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 481 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 483 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 485 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2373 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 487 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2379 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 491 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2385 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 492 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2391 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 496 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2397 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 500 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2403 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 504 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2409 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 506 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2415 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 508 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2421 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 510 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2427 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 512 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2433 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 516 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2439 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 517 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2445 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 521 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2451 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 523 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2457 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 527 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2463 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 528 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ELLIPSIS, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2469 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 533 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[0]) ) ); }
#line 2475 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 535 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2481 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 537 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2487 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 541 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2493 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 542 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2499 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 546 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURN, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2505 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 548 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2511 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 555 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EXTERN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2517 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 559 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2523 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 567 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2529 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 569 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2535 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 577 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VALIDATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2541 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 589 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSICAL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2547 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 593 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2553 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 594 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NOHEADER, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2559 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 599 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2565 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 601 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2571 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 605 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2577 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 606 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2583 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 611 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[0]) ) ); }
#line 2589 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 613 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2595 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 615 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2601 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 617 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2607 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 622 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSSTMT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2613 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 632 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2619 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 636 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2625 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 637 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2631 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 641 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2637 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 642 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2643 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 647 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2649 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 649 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2655 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 653 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2661 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 654 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2667 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 658 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2673 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 659 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2679 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 661 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2685 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 663 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2691 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 664 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2697 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 665 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2703 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 666 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2709 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 667 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2715 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 669 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUNTYPED, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2721 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 670 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2727 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 675 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2733 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 677 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODTRIGGER, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2739 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2745 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 682 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); AddToList ( P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2751 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 686 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2757 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2763 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 691 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2769 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2775 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2781 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 698 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2787 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 700 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2793 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 705 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2799 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 707 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2805 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 709 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2811 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 714 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2817 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 716 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOLEXPR, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2823 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 718 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2829 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 722 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2835 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 723 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2841 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 727 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2847 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 731 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2853 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 732 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2859 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 736 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2865 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 737 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2871 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2877 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 742 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2883 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 743 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2889 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 744 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2895 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 749 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DFLTVIEW, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2901 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 754 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2907 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 756 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2913 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 758 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2919 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 760 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2925 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 765 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOLDEF, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2931 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 769 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2937 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 770 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2943 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 774 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2949 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 775 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2955 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 779 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2961 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 780 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2967 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 784 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2973 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 785 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2979 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 792 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2985 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 793 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2991 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 797 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2997 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 798 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CASTEXPR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3003 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 802 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3009 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 803 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3015 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3021 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 805 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3027 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 806 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3033 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 807 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3039 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 808 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3045 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 809 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3051 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 810 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3057 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 811 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3063 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 812 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNARYPLUS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3069 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 822 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, T ( (yyvsp[-7]) ), P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3075 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 828 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3081 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 832 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3087 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 833 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3093 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 837 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3099 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 838 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3105 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 839 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3111 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 843 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3117 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 844 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3123 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 848 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3129 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 849 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3135 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 853 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3141 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 854 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3147 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 858 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3153 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 859 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3159 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 863 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3165 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 864 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3171 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 865 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3177 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 869 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3183 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 870 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3189 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 874 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3195 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 875 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3201 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 879 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONSTVECT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3207 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 883 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3213 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 884 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3219 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 888 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3225 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 889 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3231 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 893 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3237 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 894 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3243 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 898 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3249 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 899 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3255 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 900 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3261 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 901 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3267 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 905 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3273 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 906 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1])), P ( (yyvsp[0]) ) ); }
#line 3279 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 916 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2])), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3285 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 920 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3291 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 921 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBDAD, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 925 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3303 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 926 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3309 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 930 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3315 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 931 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 935 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3327 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 936 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3333 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 937 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3339 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 941 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3345 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 942 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3351 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 947 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3357 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 952 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TBLMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3363 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 959 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3369 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 967 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FQN, P ( (yyvsp[0]) ) ); }
#line 3375 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 968 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3381 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 970 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3387 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 971 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3393 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 972 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3399 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 973 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3405 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 974 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3411 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 975 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3417 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 976 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3423 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 977 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3429 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 981 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 3435 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 985 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EMPTY ); }
#line 3441 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 989 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSNAME, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3447 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 993 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3453 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 994 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3459 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 3463 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
