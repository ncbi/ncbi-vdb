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
#line 27 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:339  */

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



#line 173 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:339  */

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
#ifndef YY_SCHEMA_2T_DATA_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED
# define YY_SCHEMA_2T_DATA_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED
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

#endif /* !YY_SCHEMA_2T_DATA_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 374 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   872

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  155
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  125
/* YYNRULES -- Number of rules.  */
#define YYNRULES  271
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  525

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
     898,   902,   903,   907,   908,   912,   913,   917,   919,   926,
     934,   935,   939,   940,   944,   945,   949,   950,   951,   955,
     956,   960,   965,   973,   981,   982,   984,   985,   986,   987,
     988,   989,   990,   991,   995,   999,  1003,  1007,  1008,  1014,
    1018,  1019,  1023,  1024,  1028,  1033,  1034,  1038,  1042,  1043,
    1047,  1048,  1052,  1054,  1056,  1060,  1061,  1065,  1066,  1070,
    1075,  1076
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

#define YYPACT_NINF -331

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-331)))

#define YYTABLE_NINF -248

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     573,  -331,   166,   166,   166,   -20,   166,    47,    61,    13,
      56,   166,   166,   166,   -20,   111,  -331,   227,   248,   736,
     736,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,   708,  -331,    -5,  -331,   166,
     -19,     4,   188,  -331,  -331,   118,   166,     3,   166,   166,
      11,  -331,  -331,  -331,    62,  -331,  -331,  -331,   119,    47,
    -331,   198,    -5,   174,   194,  -331,   208,   220,  -331,  -331,
     736,  -331,   166,  -331,   708,  -331,  -331,   429,   223,   194,
      53,   502,  -331,   166,   224,  -331,  -331,   236,   194,    64,
      69,   166,  -331,   134,  -331,   166,    60,  -331,   166,  -331,
      62,  -331,   166,  -331,   166,   241,  -331,   230,  -331,   166,
     253,  -331,  -331,   252,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,   529,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,  -331,   529,  -331,   166,   357,
    -331,   529,   529,   260,  -331,  -331,  -331,  -331,  -331,  -331,
    -331,  -331,  -331,  -331,  -331,    17,    27,  -331,   256,     4,
    -331,   -14,  -331,  -331,   237,   261,  -331,    11,  -331,  -331,
     166,   263,  -331,   263,   270,  -331,   545,  -331,  -331,   166,
     246,  -331,  -331,   166,   275,  -331,   272,   280,  -331,   274,
     148,  -331,  -331,    52,  -331,  -331,  -331,  -331,   529,   166,
     529,   276,  -331,  -331,  -331,  -331,  -331,    55,  -331,    77,
    -331,  -331,  -331,  -331,  -331,  -331,   194,    42,   277,  -331,
    -331,   129,   166,   279,   146,   371,  -331,    18,  -331,    58,
     166,  -331,  -331,   166,   676,  -331,  -331,     2,   213,  -331,
    -331,  -331,   166,  -331,   166,   150,  -331,  -331,  -331,   529,
     529,   357,   166,   502,   163,  -331,  -331,  -331,   152,   529,
    -331,  -331,   166,   166,   166,   413,   281,   190,  -331,  -331,
       0,   200,    59,   289,  -331,  -331,   166,   393,   292,   427,
     293,   357,  -331,   426,   166,  -331,    18,  -331,   300,   301,
    -331,  -331,   357,   380,  -331,   380,     6,   256,  -331,  -331,
    -331,  -331,  -331,   166,   303,  -331,  -331,  -331,   256,   304,
       9,   529,   529,  -331,  -331,   305,   310,  -331,   166,   166,
     314,  -331,  -331,    41,   166,  -331,  -331,   166,   115,   440,
     309,   193,  -331,  -331,  -331,    51,   166,  -331,   312,   289,
     289,  -331,     8,  -331,    51,  -331,   149,   318,   529,   326,
     450,   159,  -331,  -331,  -331,   104,    74,   256,  -331,   529,
     529,   161,  -331,   217,  -331,  -331,  -331,  -331,  -331,  -331,
     166,  -331,   327,   329,  -331,  -331,   166,  -331,  -331,  -331,
     529,   334,   335,   287,  -331,  -331,   166,  -331,   314,   462,
     341,  -331,  -331,    36,   115,  -331,   529,    46,  -331,   166,
     -44,   166,  -331,  -331,  -331,  -331,    48,   340,   529,   343,
    -331,   204,   357,  -331,   357,  -331,   529,  -331,  -109,   -24,
     166,  -331,    73,   529,   342,   350,  -331,     7,   529,  -331,
    -331,  -331,  -331,   314,  -331,  -331,   114,  -331,   115,  -331,
     341,    40,  -331,  -331,   337,  -331,   169,  -331,   353,   354,
    -331,  -331,   529,  -331,  -331,   194,    63,  -331,  -331,   256,
     348,   351,   352,  -331,   358,    73,  -331,  -331,    80,   166,
     166,   166,  -331,   166,  -331,   360,     7,  -331,   355,  -331,
     341,  -331,  -331,   529,   359,  -331,  -331,    88,  -331,  -331,
     370,   529,   529,  -331,  -331,  -331,  -331,   192,  -331,   166,
     362,  -331,  -331,  -331,  -331,    97,  -331,  -331,   366,    98,
     105,   166,  -331,   363,   529,  -331,  -331,  -331,  -331,  -331,
     529,   107,   127,  -331,  -331
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   245,     0,   245,
     245,     0,     0,     0,     0,     0,    23,     0,     0,     0,
       4,     8,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,   244,     0,   234,     0,
      32,     0,   245,    89,    88,     0,     0,    38,     0,     0,
       0,    43,    44,    45,     0,    50,    49,   233,     0,   245,
      90,   245,     0,     0,    28,    92,     0,     0,     1,     3,
       5,     9,     0,   252,     6,   250,   253,     0,     0,    42,
       0,     0,   246,     0,     0,   220,    36,     0,    38,     0,
       0,     0,   213,     0,    52,     0,    32,    56,     0,    57,
       0,    94,     0,    91,     0,     0,   106,     0,    25,     0,
       0,     7,   249,     0,   251,   236,   237,   238,   239,   240,
     241,   242,   243,   235,    41,     0,   199,   201,   200,   202,
     203,   204,   205,   143,   212,   211,   245,    35,     0,     0,
     172,     0,     0,     0,   171,    34,   168,   173,   174,   175,
     176,   177,   178,   181,   182,   170,   234,   248,   245,   247,
     221,   245,   219,    37,     0,     0,    54,     0,    51,    55,
       0,   245,    95,   245,   107,   108,     0,   105,    24,     0,
       0,    30,    27,     0,     0,   209,     0,   208,   207,     0,
       0,   185,   189,   187,   179,   180,    33,   216,     0,     0,
       0,     0,   191,   190,   230,   228,   222,   245,   224,     0,
     226,   227,   229,    48,    47,    53,   214,   245,     0,    59,
      58,     0,     0,     0,   126,   130,   131,   245,   132,   245,
       0,   123,   111,     0,     0,   112,   114,     0,     0,   128,
     121,    26,     0,    29,     0,     0,   255,    40,   206,     0,
       0,     0,     0,     0,     0,   166,   215,   193,     0,   245,
     223,   225,     0,     0,     0,     0,     0,   245,    66,    61,
     245,     0,     0,     0,    93,   109,     0,     0,     0,     0,
     245,     0,   119,     0,     0,   158,   245,   118,     0,     0,
     110,   113,     0,     0,   115,     0,    32,   245,   127,   130,
     129,    31,   257,     0,   245,   210,   169,   186,   245,     0,
       0,     0,     0,   192,   197,     0,   196,   195,     0,     0,
     245,    68,    60,     0,     0,    62,    79,     0,     0,     0,
       0,   245,    75,    70,    81,     0,     0,    46,     0,     0,
       0,   100,     0,    98,     0,    96,     0,     0,     0,     0,
       0,     0,   160,   163,   165,   164,     0,   245,   120,     0,
       0,     0,   133,     0,   142,   141,   134,   137,   136,   256,
       0,   265,     0,     0,   188,   218,     0,   167,   194,   184,
       0,     0,     0,     0,    63,    80,     0,    67,   245,     0,
     245,    77,    69,     0,     0,    71,     0,     0,    84,     0,
       0,     0,   101,   102,    97,    99,     0,     0,     0,     0,
     152,     0,     0,   159,     0,   153,     0,   157,     0,     0,
       0,   140,   245,     0,     0,   266,   267,   245,   245,   217,
     198,   231,   232,   245,    64,    78,     0,    72,     0,    76,
     245,     0,    83,    85,     0,    82,     0,   104,     0,     0,
     116,   155,     0,   161,   162,   164,     0,   125,   124,   245,
       0,     0,     0,   151,     0,   145,   146,   144,     0,     0,
       0,     0,   264,     0,   258,     0,   259,   260,     0,    65,
     245,    73,    86,     0,     0,   122,   117,     0,   154,   135,
       0,     0,     0,   138,   147,   139,   270,     0,   268,     0,
       0,   254,   261,   183,    74,     0,   103,   156,     0,     0,
       0,     0,   269,     0,     0,    87,   150,   148,   149,   271,
       0,     0,     0,   262,   263
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -331,  -331,  -331,  -331,   477,    31,  -331,  -331,  -331,  -331,
    -331,  -331,    22,   250,  -331,   461,  -331,  -331,  -331,  -331,
     302,    24,  -331,  -331,   508,  -331,  -331,   356,   -15,   345,
    -331,  -331,  -231,  -266,  -331,  -331,  -300,  -330,  -314,  -331,
     175,   -88,  -331,  -331,  -331,  -331,  -331,  -331,  -331,  -331,
     178,    57,  -331,  -331,  -331,  -331,  -331,   288,  -331,  -331,
    -331,   283,  -331,  -331,   229,  -331,    95,  -331,  -331,    65,
    -331,  -207,   245,  -331,  -331,   117,   113,  -226,   -28,  -331,
    -331,   239,   282,  -290,   251,  -331,   121,  -331,  -136,  -331,
    -331,  -331,  -331,  -331,  -331,   219,  -331,  -331,  -331,  -331,
    -331,  -331,   325,  -331,  -331,  -331,  -331,    -2,   233,   144,
       1,   -33,  -331,  -331,   472,  -331,  -331,   247,  -331,  -331,
      78,  -331,  -331,    83,  -331
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    22,   107,    23,    63,
     110,   180,    92,   143,    24,    45,    46,    25,    26,    78,
      27,    51,    52,    53,    54,    55,    93,    94,    98,   218,
     219,   266,   267,   268,   271,   330,   331,   332,   325,   337,
     397,   398,    28,    44,    29,    30,    31,   102,   274,   342,
     343,   345,    32,   105,   174,   177,   234,   235,   236,   237,
     238,   239,   294,   295,   362,   363,   144,   464,   465,   466,
     240,   282,   283,   284,   351,   352,   353,   254,   255,   146,
     147,   190,   191,   201,   202,   258,   315,   316,   148,   149,
     150,   151,   186,   187,   152,   399,   153,   154,    33,    84,
     162,   207,   208,   209,   210,   211,    34,   155,    38,   326,
     157,   158,    35,    74,    75,    76,   245,   246,   475,   476,
     477,   372,   425,   426,   497
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,    40,    41,   192,    47,    42,   384,   368,    36,    41,
      62,    64,    61,    36,     7,    36,   -39,   395,   373,    82,
      36,    82,   287,   375,    36,    39,   327,   457,   390,   338,
     204,   471,    82,   320,    60,    79,   339,    80,   340,   311,
     385,   197,   280,   101,    88,   385,    89,    90,    96,    36,
     160,    71,    96,   145,    36,    36,    96,   387,    91,    36,
      40,    36,   327,   439,    36,   171,    73,   417,    48,    49,
      41,   175,    57,   113,   434,    36,   437,    48,    49,   358,
     338,   159,   280,   103,   108,   172,   396,   339,   396,   340,
      59,   396,   445,   388,   440,   286,   159,   184,    96,   204,
      41,    71,   159,   173,   262,    73,   439,    40,   185,   460,
      77,   461,   458,   194,   195,   192,    97,   387,   385,   479,
     263,    81,   205,   462,   311,   206,   481,    36,    36,    66,
      67,   181,    36,   418,   419,    77,    96,   193,   480,   100,
     327,   327,   328,   472,   341,   354,    81,   404,   292,    77,
     244,    56,   200,    56,    56,   433,   192,    77,    77,    50,
      77,  -247,   376,  -247,   281,    96,   504,   198,   216,   489,
     441,    77,   257,    97,    40,   277,   482,    40,   438,    36,
     199,   159,   278,   386,   264,   442,    85,   447,   311,   275,
     456,   205,   253,    50,   260,   341,   125,   468,   233,   488,
      81,   241,    50,    56,   281,   106,    77,    77,   164,   463,
     415,   311,   170,   165,    77,    40,   495,   416,    77,   308,
     159,   305,   306,    77,   507,   145,   487,    68,   311,   318,
     319,   314,    40,   515,   517,   296,   311,   298,   297,   265,
      40,   518,   299,   523,   226,   311,   311,   414,    69,   193,
     159,   357,   228,   311,    86,   311,   233,   505,    77,   293,
     159,   159,    40,   524,   301,   509,   510,   272,    40,    95,
     244,   167,   273,    99,   346,   311,   354,    99,   354,   355,
     188,   168,   159,   377,   378,   251,   265,   303,   521,   312,
     193,   385,   329,   407,   522,   252,   412,   304,   251,   313,
      36,   159,   203,    77,   310,   212,   413,    43,   420,   443,
     123,   311,   109,   484,   156,   220,    65,   220,   443,    99,
     409,    40,    40,    77,   166,    40,    40,   323,   169,   511,
     393,    83,   324,    96,   400,   394,   334,   424,   335,   512,
     451,   104,    96,   336,   111,   265,   265,   452,    77,   389,
     329,   212,   430,   421,   508,   422,   112,   189,   156,   124,
     423,   269,   161,   126,   127,   128,   178,   179,   159,   156,
      36,   285,   163,   285,   156,   156,   126,   127,   128,   176,
     449,    40,   213,   242,    40,   243,    95,   459,   364,   182,
     364,    40,    40,    36,   133,    96,   402,   403,   183,   446,
     314,   196,   200,   317,    96,   265,   214,   222,   265,   217,
     355,   247,   455,   248,   333,   329,   329,   249,   159,   250,
     259,   270,   276,   279,   285,    40,   321,   344,   322,   347,
     285,   156,   256,   156,    40,   348,    40,   424,   349,   281,
     356,   203,    36,   359,   360,   374,   370,   380,   371,   473,
     379,   383,   203,   391,   392,   401,   115,   116,   329,   117,
     329,   408,   410,   288,   411,   427,   289,   118,   159,    40,
     431,   432,   119,   428,    40,   435,   120,   302,   436,   450,
     483,   121,   156,   156,   122,   448,   156,   470,   469,   485,
     486,   490,   156,   499,   491,   492,    70,   493,   473,   501,
     503,   203,   516,   309,   506,   514,   520,    87,   126,   127,
     128,   129,   130,   131,   132,    36,   133,    58,   221,   406,
     405,   300,   291,   215,   366,   350,   365,   454,   365,   453,
     494,   361,   261,   307,   134,   126,   127,   128,   129,   130,
     131,   132,    36,   133,   156,   156,   114,   367,   135,   478,
     369,   381,   382,   498,   502,     0,     0,     0,    36,     0,
       0,   134,     0,     0,     0,     0,   467,   223,     0,   224,
       0,   474,   317,     1,   225,   135,   226,     0,     0,     0,
       0,   156,   227,     0,   228,     0,     0,   229,     0,     0,
     230,     0,   156,   156,     0,     0,     2,     0,     3,     0,
       4,     0,     0,   203,     5,     0,     6,     7,     8,   429,
       9,     0,     0,   156,    10,     0,    11,     0,     0,     0,
       0,    12,    13,    14,    15,     0,     0,     0,     0,   156,
       0,     0,   444,     0,     0,     0,     0,     0,     0,     0,
       0,   156,   136,     0,   137,     0,   138,     0,   139,   156,
       0,   140,   141,   142,     0,     0,   156,     0,     0,     0,
       0,   156,     0,     0,     0,     0,     0,     0,     0,   136,
       0,     0,     0,   138,     0,   139,     0,     0,   140,   141,
     142,   231,     0,     0,   232,   156,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,   223,     0,
     224,     0,   496,     0,     0,   225,   500,   226,     0,    16,
       0,     0,     0,   227,     0,   228,   156,     0,   229,     0,
       0,   230,     0,     0,   156,   156,     0,     0,     0,     0,
       0,     2,   513,     3,     0,     4,     0,     0,     0,     5,
       0,     6,     7,     8,   519,     9,     0,   156,     0,    10,
       0,    11,     0,   156,     0,     0,    12,    13,    14,     2,
      72,     3,     0,     4,     0,     0,     0,     5,     0,     6,
       7,     8,     0,     9,     0,     0,     0,    10,     0,    11,
       0,     0,     0,     0,    12,    13,    14,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   231,     0,     0,   290,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    16,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       2,     3,     4,   139,     6,     4,   320,   297,    13,    11,
      12,    13,    11,    13,    34,    13,    13,   331,   308,    15,
      13,    15,   229,    14,    13,     3,    26,   136,   328,    21,
      44,    24,    15,   264,    10,    37,    28,    39,    30,   148,
       4,    14,    24,    58,    46,     4,    48,    49,    50,    13,
      83,    20,    54,    81,    13,    13,    58,   323,    47,    13,
      62,    13,    26,   393,    13,    98,    35,   357,    21,    22,
      72,   104,    11,    72,   388,    13,   390,    21,    22,   286,
      21,    83,    24,    59,    62,   100,    40,    28,    40,    30,
      34,    40,   136,   324,   394,    37,    98,   125,   100,    44,
     102,    70,   104,   102,    27,    74,   436,   109,   136,    36,
     154,    38,   136,   141,   142,   251,    54,   383,     4,   433,
      43,   140,   136,    50,   148,   139,   440,    13,    13,    18,
      19,   109,    13,   359,   360,   154,   138,   139,   438,    20,
      26,    26,   142,   136,   136,   281,   140,   139,   146,   154,
     183,     7,   146,     9,    10,   386,   292,   154,   154,   146,
     154,   144,   153,   146,   146,   167,   480,   140,   170,   459,
     396,   154,   200,    54,   176,    29,   136,   179,   142,    13,
     153,   183,    36,   142,   142,   139,    42,   139,   148,   222,
     416,   136,   140,   146,   139,   136,   143,   423,   176,   136,
     140,   179,   146,    59,   146,    61,   154,   154,   144,   136,
     136,   148,   152,   144,   154,   217,   136,   143,   154,   252,
     222,   249,   250,   154,   136,   253,   452,     0,   148,   262,
     263,   259,   234,   136,   136,   237,   148,    24,   237,   217,
     242,   136,    29,   136,    31,   148,   148,   143,     0,   251,
     252,   284,    39,   148,   136,   148,   234,   483,   154,   237,
     262,   263,   264,   136,   242,   491,   492,   138,   270,    50,
     303,   137,   143,    54,   276,   148,   412,    58,   414,   281,
     136,   147,   284,   311,   312,   137,   264,   137,   514,   137,
     292,     4,   270,   144,   520,   147,   137,   147,   137,   147,
      13,   303,   158,   154,   141,   161,   147,     5,   147,   397,
      77,   148,   138,   144,    81,   171,    14,   173,   406,   100,
     348,   323,   324,   154,    91,   327,   328,   137,    95,   137,
     137,   143,   142,   335,   336,   142,   136,   370,   138,   147,
     136,   143,   344,   143,   136,   323,   324,   143,   154,   327,
     328,   207,   380,   136,   490,   138,   136,   138,   125,   136,
     143,   217,   138,     6,     7,     8,   136,   137,   370,   136,
      13,   227,   136,   229,   141,   142,     6,     7,     8,   138,
     408,   383,   145,   137,   386,   139,   167,   420,   293,   136,
     295,   393,   394,    13,    14,   397,   339,   340,   146,   401,
     428,   141,   146,   259,   406,   383,   145,   137,   386,   146,
     412,   136,   414,   141,   270,   393,   394,   137,   420,   145,
     144,   144,   143,    52,   280,   427,    13,   138,   147,    36,
     286,   198,   199,   200,   436,   143,   438,   470,    11,   146,
      14,   297,    13,   143,   143,   141,   143,   137,   304,   427,
     145,   137,   308,    13,   145,   143,    27,    28,   436,    30,
     438,   143,   136,   230,    14,   138,   233,    38,   470,   471,
     136,   136,    43,   144,   476,    13,    47,   244,   137,   136,
     143,    52,   249,   250,    55,   145,   253,   137,   146,   136,
     136,   143,   259,   471,   143,   143,    19,   139,   476,   139,
     145,   357,   136,   253,   145,   143,   143,    46,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     9,   173,   344,
     342,   238,   234,   167,   295,   280,   293,   414,   295,   412,
     465,   292,   207,   251,    32,     6,     7,     8,     9,    10,
      11,    12,    13,    14,   311,   312,    74,   296,    46,   428,
     303,   318,   319,   470,   476,    -1,    -1,    -1,    13,    -1,
      -1,    32,    -1,    -1,    -1,    -1,   422,    22,    -1,    24,
      -1,   427,   428,     0,    29,    46,    31,    -1,    -1,    -1,
      -1,   348,    37,    -1,    39,    -1,    -1,    42,    -1,    -1,
      45,    -1,   359,   360,    -1,    -1,    23,    -1,    25,    -1,
      27,    -1,    -1,   459,    31,    -1,    33,    34,    35,   376,
      37,    -1,    -1,   380,    41,    -1,    43,    -1,    -1,    -1,
      -1,    48,    49,    50,    51,    -1,    -1,    -1,    -1,   396,
      -1,    -1,   399,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   408,   140,    -1,   142,    -1,   144,    -1,   146,   416,
      -1,   149,   150,   151,    -1,    -1,   423,    -1,    -1,    -1,
      -1,   428,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
      -1,    -1,    -1,   144,    -1,   146,    -1,    -1,   149,   150,
     151,   136,    -1,    -1,   139,   452,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    -1,
      24,    -1,   469,    -1,    -1,    29,   473,    31,    -1,   136,
      -1,    -1,    -1,    37,    -1,    39,   483,    -1,    42,    -1,
      -1,    45,    -1,    -1,   491,   492,    -1,    -1,    -1,    -1,
      -1,    23,   499,    25,    -1,    27,    -1,    -1,    -1,    31,
      -1,    33,    34,    35,   511,    37,    -1,   514,    -1,    41,
      -1,    43,    -1,   520,    -1,    -1,    48,    49,    50,    23,
      52,    25,    -1,    27,    -1,    -1,    -1,    31,    -1,    33,
      34,    35,    -1,    37,    -1,    -1,    -1,    41,    -1,    43,
      -1,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,   139,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136
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
     143,   264,   276,   238,   141,    14,   153,   233,   233,   145,
     137,   263,   263,   137,   193,     4,   142,   188,   187,   167,
     191,    13,   145,   137,   142,   193,    40,   195,   196,   250,
     262,   143,   206,   206,   139,   205,   195,   144,   143,   233,
     136,    14,   137,   147,   143,   136,   143,   238,   232,   232,
     147,   136,   138,   143,   266,   277,   278,   138,   144,   263,
     233,   136,   136,   187,   193,    13,   137,   193,   142,   192,
     191,   232,   139,   196,   263,   136,   262,   139,   145,   233,
     136,   136,   143,   230,   231,   262,   232,   136,   136,   266,
      36,    38,    50,   136,   222,   223,   224,   264,   232,   146,
     137,    24,   136,   167,   264,   273,   274,   275,   241,   193,
     191,   193,   136,   143,   144,   136,   136,   232,   136,   238,
     143,   143,   143,   139,   224,   136,   263,   279,   278,   167,
     263,   139,   275,   145,   193,   232,   145,   136,   243,   232,
     232,   137,   147,   263,   143,   136,   136,   136,   136,   263,
     143,   232,   232,   136,   136
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
     248,   249,   249,   250,   250,   251,   251,   252,   252,   253,
     254,   254,   255,   255,   256,   256,   257,   257,   257,   258,
     258,   259,   260,   261,   262,   262,   262,   262,   262,   262,
     262,   262,   262,   262,   263,   264,   265,   266,   266,   267,
     268,   268,   269,   269,   270,   271,   271,   272,   273,   273,
     274,   274,   275,   275,   275,   276,   276,   277,   277,   278,
     279,   279
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
       3,     1,     1,     1,     3,     3,     2,     6,     5,     4,
       1,     2,     2,     3,     1,     2,     1,     1,     1,     1,
       1,     5,     5,     2,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     0,     2,     1,     1,     3,
       1,     2,     1,     1,     9,     1,     3,     2,     1,     1,
       1,     2,     5,     6,     1,     1,     2,     1,     3,     4,
       1,     3
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
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1799 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 3: /* UNRECOGNIZED  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1807 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 4: /* ELLIPSIS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1815 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 5: /* INCREMENT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1823 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 6: /* DECIMAL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1831 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 7: /* OCTAL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1839 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 8: /* HEX  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1847 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 9: /* FLOAT_  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1855 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 10: /* EXP_FLOAT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1863 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 11: /* STRING  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1871 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 12: /* ESCAPED_STRING  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1879 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 13: /* IDENTIFIER_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1887 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 14: /* PHYSICAL_IDENTIFIER_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1895 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 15: /* VERSION  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1903 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 16: /* UNTERM_STRING  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1911 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 17: /* UNTERM_ESCAPED_STRING  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1919 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 18: /* VERS_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1927 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 19: /* VERS_2_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1935 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 20: /* KW___no_header  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1943 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 21: /* KW___row_length  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1951 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 22: /* KW___untyped  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1959 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 23: /* KW_alias  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1967 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 24: /* KW_column  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1975 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 25: /* KW_const  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1983 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 26: /* KW_control  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1991 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 27: /* KW_database  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1999 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 28: /* KW_decode  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2007 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 29: /* KW_default  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2015 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 30: /* KW_encode  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2023 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 31: /* KW_extern  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2031 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 32: /* KW_false  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2039 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 33: /* KW_fmtdef  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2047 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 34: /* KW_function  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2055 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 35: /* KW_include  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2063 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 36: /* KW_limit  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2071 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 37: /* KW_physical  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2079 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 38: /* KW_read  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2087 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 39: /* KW_readonly  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2095 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 40: /* KW_return  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2103 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 41: /* KW_schema  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2111 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 42: /* KW_static  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2119 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 43: /* KW_table  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2127 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 44: /* KW_template  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2135 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 45: /* KW_trigger  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2143 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 46: /* KW_true  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2151 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 47: /* KW_type  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2159 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 48: /* KW_typedef  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2167 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 49: /* KW_typeset  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2175 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 50: /* KW_validate  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2183 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 51: /* KW_version  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2191 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 52: /* KW_view  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2199 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 53: /* KW_virtual  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2207 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 54: /* KW_void  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2215 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 55: /* KW_write  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2223 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 56: /* PT_ASTLIST  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2231 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 57: /* PT_PARSE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2239 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 58: /* PT_SOURCE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2247 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 59: /* PT_VERSION_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2255 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 60: /* PT_VERSION_2  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2263 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 61: /* PT_SCHEMA_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2271 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 62: /* PT_SCHEMA_2_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2279 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 63: /* PT_INCLUDE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2287 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 64: /* PT_TYPEDEF  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2295 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 65: /* PT_FQN  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2303 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 66: /* PT_IDENT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2311 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 67: /* PT_PHYSIDENT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2319 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 68: /* PT_UINT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2327 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 69: /* PT_TYPESET  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2335 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 70: /* PT_TYPESETDEF  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2343 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 71: /* PT_FORMAT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2351 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 72: /* PT_CONST  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2359 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 73: /* PT_ALIAS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2367 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 74: /* PT_EXTERN  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2375 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 75: /* PT_FUNCTION  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2383 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 76: /* PT_UNTYPED  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2391 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 77: /* PT_ROWLENGTH  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2399 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 78: /* PT_FUNCDECL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2407 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 79: /* PT_EMPTY  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2415 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 80: /* PT_SCHEMASIG  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2423 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 81: /* PT_SCHEMAFORMAL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2431 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 82: /* PT_RETURNTYPE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2439 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 83: /* PT_FACTSIG  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2447 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 84: /* PT_FUNCSIG  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2455 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 85: /* PT_FUNCPARAMS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2463 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 86: /* PT_FORMALPARAM  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2471 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 87: /* PT_ELLIPSIS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2479 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 88: /* PT_FUNCPROLOGUE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2487 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 89: /* PT_RETURN  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2495 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 90: /* PT_PRODSTMT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2503 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 91: /* PT_PRODTRIGGER  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2511 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 92: /* PT_SCHEMA  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2519 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 93: /* PT_VALIDATE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2527 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 94: /* PT_PHYSICAL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2535 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 95: /* PT_PHYSPROLOGUE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2543 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 96: /* PT_PHYSSTMT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2551 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 97: /* PT_PHYSBODYSTMT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2559 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 98: /* PT_TABLE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2567 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 99: /* PT_TABLEPARENTS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2575 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 100: /* PT_TABLEBODY  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2583 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 101: /* PT_FUNCEXPR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2591 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 102: /* PT_FACTPARMS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2599 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 103: /* PT_COLUMN  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2607 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 104: /* PT_COLUMNEXPR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2615 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 105: /* PT_COLDECL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2623 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 106: /* PT_TYPEDCOL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2631 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 107: /* PT_COLSTMT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2639 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 108: /* PT_DFLTVIEW  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2647 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 109: /* PT_PHYSMBR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2655 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 110: /* PT_PHYSCOL  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2663 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 111: /* PT_PHYSCOLDEF  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2671 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 112: /* PT_COLSCHEMAPARMS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2679 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 113: /* PT_COLSCHEMAPARAM  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2687 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 114: /* PT_COLUNTYPED  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2695 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 115: /* PT_DATABASE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2703 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 116: /* PT_TYPEEXPR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2711 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 117: /* PT_DBBODY  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2719 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 118: /* PT_DBDAD  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2727 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 119: /* PT_DBMEMBER  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2735 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 120: /* PT_TBLMEMBER  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2743 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 121: /* PT_NOHEADER  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2751 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 122: /* PT_CASTEXPR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2759 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 123: /* PT_CONSTVECT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2767 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 124: /* PT_NEGATE  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2775 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 125: /* PT_UNARYPLUS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2783 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 126: /* PT_VERSNAME  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2791 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 127: /* PT_ARRAY  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2799 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 128: /* PT_PHYSENCREF  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2807 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 129: /* PT_TYPEDCOLEXPR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2815 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 130: /* PT_VIEW  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2823 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 131: /* PT_VIEWPARAM  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2831 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 132: /* PT_VIEWPARENTS  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2839 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 133: /* PT_VIEWPARENT  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2847 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 134: /* PT_MEMBEREXPR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2855 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 135: /* PT_JOINEXPR  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2863 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 136: /* ';'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2871 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 137: /* ','  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2879 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 138: /* '{'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2887 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 139: /* '}'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2895 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 140: /* '['  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2903 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 141: /* ']'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2911 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 142: /* '*'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2919 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 143: /* '='  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2927 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 144: /* '('  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2935 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 145: /* ')'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2943 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 146: /* '<'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2951 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 147: /* '>'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2959 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 148: /* '|'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2967 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 149: /* '@'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2975 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 150: /* '-'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2983 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 151: /* '+'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2991 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 152: /* '/'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2999 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 153: /* '.'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3007 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 154: /* ':'  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3015 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 156: /* parse  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3023 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 157: /* source  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3031 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 158: /* version_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3039 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 159: /* schema_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3047 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 160: /* schema_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3055 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 161: /* typedef_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3063 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 162: /* typedef_1_0_new_name_list  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3071 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 163: /* typeset_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3079 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 164: /* typeset_1_0_new_name  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3087 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 165: /* typeset_1_0_def  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3095 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 166: /* typespec_1_0_list  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3103 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 167: /* typespec_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3111 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 168: /* dim_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3119 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 169: /* format_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3127 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 170: /* format_1_0_new_name  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3135 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 171: /* format_1_0_name  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3143 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 172: /* const_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3151 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 173: /* alias_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3159 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 174: /* alias_1_0_new_name  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3167 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 175: /* function_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3175 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 176: /* func_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3183 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 177: /* untyped_func_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3191 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 178: /* row_length_func_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3199 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 179: /* opt_func_1_0_schema_sig  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3207 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 180: /* func_1_0_schema_sig  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3215 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 181: /* func_1_0_schema_formals  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3223 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 182: /* func_1_0_schema_formal  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3231 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 183: /* func_1_0_return_type  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3239 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 184: /* opt_func_1_0_fact_sig  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3247 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 185: /* func_1_0_fact_sig  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3255 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 186: /* func_1_0_fact_signature  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3263 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 187: /* func_1_0_fact_params  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3271 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 188: /* fact_param_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3279 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 189: /* func_1_0_param_sig  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3287 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 190: /* func_1_0_param_signature  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3295 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 191: /* func_1_0_formal_params  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3303 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 192: /* formal_param_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3311 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 193: /* func_1_0_vararg_formals  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3319 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 194: /* func_1_0_prologue  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3327 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 195: /* script_1_0_stmt_seq  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3335 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 196: /* script_1_0_stmt  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3343 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 197: /* extern_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3351 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 198: /* ext_func_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3359 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 199: /* script_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3367 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 200: /* validate_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3375 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 201: /* physical_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3383 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 202: /* phys_1_0_return_type  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3391 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 203: /* phys_1_0_prologue  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3399 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 204: /* phys_1_0_body  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3407 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 205: /* phys_1_0_body_stmt  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3415 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 206: /* phys_1_0_stmt  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3423 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 207: /* table_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3431 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 208: /* opt_tbl_1_0_parents  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3439 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 209: /* tbl_1_0_parents  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3447 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 210: /* tbl_1_0_body  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3455 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 211: /* tbl_1_0_stmt_seq  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3463 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 212: /* tbl_1_0_stmt  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3471 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 213: /* production_1_0_stmt  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3479 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 214: /* col_1_0_modifiers  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3487 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 215: /* col_1_0_modifier_seq  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3495 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 216: /* col_1_0_modifier  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3503 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 217: /* col_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3511 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 218: /* phys_enc_ref  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3519 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 219: /* typed_column_decl_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3527 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 220: /* col_ident  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3535 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 221: /* phys_ident  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3543 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 222: /* opt_column_body_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3551 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 223: /* column_body_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3559 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 224: /* column_stmt_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3567 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 225: /* default_view_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3575 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 226: /* physmbr_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3583 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 227: /* phys_coldef_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3591 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 228: /* opt_col_schema_parms_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3599 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 229: /* col_schema_parms_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3607 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 230: /* col_schema_parm_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3615 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 231: /* col_schema_value_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3623 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 232: /* cond_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3631 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 233: /* expression_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3639 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 234: /* primary_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3647 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 235: /* func_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3655 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 236: /* schema_parms_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3663 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 237: /* schema_parm_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3671 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 238: /* opt_factory_parms_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3679 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 239: /* factory_parms_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3687 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 240: /* factory_parms  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3695 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 241: /* opt_func_1_0_parms  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3703 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 242: /* func_1_0_parms  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3711 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 243: /* uint_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3719 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 244: /* float_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3727 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 245: /* string_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3735 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 246: /* const_vect_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3743 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 247: /* opt_const_vect_exprlist_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3751 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 248: /* const_vect_exprlist_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3759 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 249: /* bool_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3767 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 250: /* type_expr_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3775 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 251: /* member_expr_2_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3783 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 252: /* join_expr_2_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3791 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 253: /* database_1_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3799 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 254: /* opt_database_dad_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3807 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 255: /* database_body_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3815 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 256: /* database_members_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3823 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 257: /* database_member_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3831 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 258: /* opt_template_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3839 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 259: /* db_member_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3847 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 260: /* table_member_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3855 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 261: /* include_directive  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3863 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 262: /* fqn_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3871 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 263: /* ident_1_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3879 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 264: /* empty  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3887 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 265: /* fqn_vers  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3895 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 266: /* fqn_opt_vers  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3903 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 267: /* version_2_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3911 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 268: /* schema_2_0  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3919 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 269: /* schema_2_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3927 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 270: /* view_2_0_decl  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3935 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 271: /* view_parms  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3943 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 272: /* view_parm  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3951 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 273: /* opt_view_body  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3959 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 274: /* view_body  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3967 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 275: /* view_member  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3975 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 276: /* opt_view_parents  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3983 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 277: /* view_parents  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3991 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 278: /* view_parent  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3999 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 279: /* view_parent_parms  */
#line 138 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 4007 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
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
#line 298 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root = MakeTree ( PT_PARSE, T ( (yyvsp[0]) ) );              (yyval) . subtree = 0; (yyval) . leading_ws = 0;  }
#line 4301 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 299 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root = MakeTree ( PT_PARSE, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) );    (yyval) . subtree = 0; (yyval) . leading_ws = 0; }
#line 4307 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 303 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[0]) ) ); }
#line 4313 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 304 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4319 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 305 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4325 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 311 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_1_0, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4331 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 315 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_1_0, P ( (yyvsp[0]) ) ); }
#line 4337 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 316 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 4343 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 320 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4349 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 321 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4355 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 322 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4361 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 323 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4367 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 324 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4373 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 325 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4379 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 326 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4385 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 327 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4391 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 328 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4397 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 329 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4403 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 330 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4409 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 331 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4415 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 332 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4421 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 333 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4427 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 343 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDEF, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4433 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 347 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4439 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 348 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P((yyvsp[-2])), T((yyvsp[-1])), P((yyvsp[0])) ); }
#line 4445 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 355 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESET, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4451 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 359 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4457 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 364 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESETDEF, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4463 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 370 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4469 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 371 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4475 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 375 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4481 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 376 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4487 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 380 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4493 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 381 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4499 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 388 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4505 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 390 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4511 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 394 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4517 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 398 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4523 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 406 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4529 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 413 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ALIAS, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4535 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 417 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4541 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 423 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCTION, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4547 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 427 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4553 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 428 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4559 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 435 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCDECL, P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4565 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 440 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNTYPED, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4571 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 445 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ROWLENGTH, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4577 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 449 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4583 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 450 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4589 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 454 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMASIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4595 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 458 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4601 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 459 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4607 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 463 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4613 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 464 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4619 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 468 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, T ( (yyvsp[0]) ) ); }
#line 4625 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 469 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, P ( (yyvsp[0]) ) ); }
#line 4631 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 473 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4637 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 474 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4643 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 478 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4649 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 482 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4655 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 484 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4661 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 486 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4667 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 488 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4673 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 490 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4679 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 494 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4685 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 495 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4691 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 499 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4697 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 503 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4703 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 507 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4709 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 509 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4715 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 511 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4721 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 513 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4727 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 515 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4733 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 519 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4739 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 520 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4745 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 524 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4751 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 526 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4757 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 530 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4763 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 531 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ELLIPSIS, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4769 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 536 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[0]) ) ); }
#line 4775 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 538 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4781 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 540 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4787 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 544 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4793 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 545 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4799 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 549 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURN, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4805 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 551 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4811 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 558 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EXTERN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4817 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 562 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4823 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 570 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4829 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 572 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4835 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 580 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VALIDATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4841 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 592 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSICAL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4847 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 596 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4853 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 597 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NOHEADER, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4859 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 602 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4865 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 604 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4871 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 608 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4877 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 609 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4883 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 614 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[0]) ) ); }
#line 4889 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 616 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4895 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 618 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4901 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 620 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4907 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 625 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSSTMT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4913 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 635 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4919 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 639 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4925 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 640 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4931 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 644 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4937 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 645 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4943 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 650 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4949 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 652 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4955 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 656 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4961 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 657 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4967 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 661 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4973 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 662 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4979 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 664 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4985 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 666 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4991 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 667 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4997 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 668 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5003 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 669 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5009 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 670 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5015 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 672 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUNTYPED, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5021 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 673 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5027 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 678 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5033 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 680 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODTRIGGER, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5039 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 684 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5045 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 685 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); AddToList ( P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5051 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 689 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5057 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 690 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5063 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 694 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5069 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 695 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5075 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 696 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5081 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 701 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5087 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 703 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5093 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 708 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5099 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 710 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5105 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 712 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5111 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 717 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5117 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 719 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOLEXPR, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5123 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 721 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5129 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 725 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5135 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 726 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5141 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 730 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5147 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 734 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5153 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 735 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5159 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 739 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5165 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 740 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5171 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 744 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5177 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 745 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5183 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 746 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5189 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 747 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5195 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 752 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DFLTVIEW, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5201 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 757 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5207 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 759 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5213 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 761 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5219 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 763 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5225 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 768 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOLDEF, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5231 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 772 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5237 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 773 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5243 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 777 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5249 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 778 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5255 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 782 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5261 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 783 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5267 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 787 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5273 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 788 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5279 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 795 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5285 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 796 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5291 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 800 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5297 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 801 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CASTEXPR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5303 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 805 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5309 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 806 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5315 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 807 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5321 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 808 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5327 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 809 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5333 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 810 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5339 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 811 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5345 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 812 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5351 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 813 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5357 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 814 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5363 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 815 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNARYPLUS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5369 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 816 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5375 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 817 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5381 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 827 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, T ( (yyvsp[-7]) ), P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5387 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 833 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5393 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 837 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5399 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 838 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5405 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 842 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5411 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 843 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5417 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 844 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5423 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 848 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5429 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 849 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5435 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 853 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5441 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 857 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5447 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 858 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5453 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 862 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5459 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 863 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5465 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 867 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5471 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 868 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5477 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 872 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5483 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 873 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5489 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 874 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5495 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 878 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5501 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 879 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5507 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 883 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5513 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 884 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5519 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 888 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONSTVECT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5525 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 892 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5531 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 893 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5537 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 897 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5543 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 898 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5549 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 902 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5555 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 903 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5561 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 907 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5567 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 908 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1])), P ( (yyvsp[0]) ) ); }
#line 5573 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 912 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_MEMBEREXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5579 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 913 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_MEMBEREXPR, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5585 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 918 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_JOINEXPR, P ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ((yyvsp[0]) ) ); }
#line 5591 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 920 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_JOINEXPR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5597 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 930 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2])), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5603 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 934 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5609 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 935 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBDAD, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5615 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 939 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5621 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 940 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5627 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 944 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5633 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 945 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5639 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 949 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5645 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 950 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5651 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 951 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5657 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 955 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5663 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 956 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5669 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 961 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5675 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 966 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TBLMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5681 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 973 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5687 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 981 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FQN, P ( (yyvsp[0]) ) ); }
#line 5693 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 982 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5699 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 984 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5705 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 985 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5711 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 986 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5717 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 987 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5723 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 988 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5729 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 989 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5735 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 990 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5741 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 991 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5747 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 995 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 5753 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 999 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EMPTY ); }
#line 5759 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1003 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSNAME, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5765 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1007 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5771 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1008 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5777 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1014 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_2, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5783 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1018 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_2_0, P ( (yyvsp[0]) ) ); }
#line 5789 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1019 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 5795 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1023 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5801 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1024 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5807 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1029 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEW, T ( (yyvsp[-8]) ), P ( (yyvsp[-7]) ), T ( (yyvsp[-6]) ), P ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5813 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1033 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5819 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1034 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5825 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1038 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARAM, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5831 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1042 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5837 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1043 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5843 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1047 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5849 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 261:
#line 1048 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5855 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 262:
#line 1053 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5861 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 263:
#line 1055 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5867 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 264:
#line 1056 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5873 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 265:
#line 1060 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5879 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 266:
#line 1061 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5885 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 267:
#line 1065 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5891 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 268:
#line 1066 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5897 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 269:
#line 1071 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARENT, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5903 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 270:
#line 1075 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5909 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 271:
#line 1076 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5915 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 5919 "/2T/data/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
