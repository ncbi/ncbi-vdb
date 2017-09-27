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



#line 173 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:339  */

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

#line 371 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   748

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  151
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  120
/* YYNRULES -- Number of rules.  */
#define YYNRULES  260
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  504

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
     141,   142,   139,   148,   134,   147,     2,   149,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   150,   133,
     143,   140,   144,     2,   146,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   137,     2,   138,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   135,   145,   136,     2,     2,     2,     2,
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
       0,   295,   295,   296,   300,   301,   302,   308,   312,   313,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   336,   344,   345,   351,   356,   360,
     367,   368,   372,   373,   377,   378,   384,   386,   391,   395,
     402,   409,   414,   420,   424,   425,   426,   436,   441,   446,
     447,   451,   455,   456,   460,   461,   465,   466,   470,   471,
     475,   479,   480,   482,   484,   486,   491,   492,   496,   500,
     504,   505,   507,   509,   511,   516,   517,   521,   522,   527,
     528,   532,   534,   536,   541,   542,   546,   547,   554,   559,
     566,   568,   576,   583,   593,   594,   598,   600,   605,   606,
     610,   612,   614,   616,   621,   628,   636,   637,   641,   642,
     646,   648,   653,   654,   658,   659,   660,   662,   664,   665,
     666,   667,   668,   670,   674,   676,   681,   682,   686,   687,
     691,   692,   693,   697,   699,   704,   706,   708,   713,   715,
     717,   722,   723,   727,   731,   732,   736,   737,   741,   742,
     743,   744,   748,   753,   755,   757,   759,   764,   769,   770,
     774,   775,   779,   780,   784,   785,   792,   793,   797,   798,
     802,   803,   804,   805,   806,   807,   808,   809,   810,   811,
     812,   816,   823,   832,   833,   837,   838,   839,   843,   844,
     848,   852,   853,   857,   858,   862,   863,   867,   868,   869,
     873,   874,   878,   879,   883,   887,   888,   892,   893,   897,
     898,   902,   903,   909,   917,   918,   922,   923,   927,   928,
     932,   933,   934,   938,   939,   943,   948,   956,   964,   965,
     967,   968,   969,   970,   971,   972,   973,   974,   978,   982,
     986,   990,   991,   997,  1001,  1002,  1006,  1007,  1011,  1016,
    1017,  1021,  1025,  1026,  1030,  1032,  1034,  1038,  1039,  1043,
    1044
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
  "PT_VIEWPARENTS", "';'", "','", "'{'", "'}'", "'['", "']'", "'*'", "'='",
  "'('", "')'", "'<'", "'>'", "'|'", "'@'", "'-'", "'+'", "'/'", "':'",
  "$accept", "parse", "source", "version_1_0", "schema_1_0",
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
  "factory_parms_1_0", "factory_parms", "opt_func_1_0_parms",
  "func_1_0_parms", "uint_expr_1_0", "float_expr_1_0", "string_expr_1_0",
  "const_vect_expr_1_0", "opt_const_vect_exprlist_1_0",
  "const_vect_exprlist_1_0", "bool_expr_1_0", "type_expr_1_0",
  "database_1_0_decl", "opt_database_dad_1_0", "database_body_1_0",
  "database_members_1_0", "database_member_1_0", "opt_template_1_0",
  "db_member_1_0", "table_member_1_0", "include_directive", "fqn_1_0",
  "ident_1_0", "empty", "fqn_vers", "fqn_opt_vers", "version_2_0",
  "schema_2_0", "schema_2_0_decl", "view_2_0_decl", "view_parms",
  "view_parm", "view_body", "view_member", "opt_view_parents",
  "view_parents", YY_NULLPTR
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
     385,   386,   387,    59,    44,   123,   125,    91,    93,    42,
      61,    40,    41,    60,    62,   124,    64,    45,    43,    47,
      58
};
# endif

#define YYPACT_NINF -301

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-301)))

#define YYTABLE_NINF -242

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     482,  -301,    26,    26,    26,    45,    26,    36,   119,   -49,
      51,    26,    26,    26,    45,    46,  -301,   197,   201,   615,
     615,  -301,  -301,  -301,  -301,  -301,  -301,  -301,  -301,  -301,
    -301,  -301,  -301,  -301,  -301,   587,  -301,    -8,  -301,    26,
      87,     7,    29,  -301,  -301,    72,    26,    -5,    26,    26,
      42,  -301,  -301,  -301,    15,  -301,  -301,  -301,   102,    36,
    -301,    81,    -8,    96,    89,  -301,   146,   206,  -301,  -301,
     615,  -301,    26,  -301,   587,  -301,  -301,   632,   208,    89,
      56,   427,  -301,    26,   218,  -301,  -301,   232,    89,   127,
     178,    26,  -301,    78,  -301,    26,    86,  -301,    26,  -301,
      15,  -301,    26,  -301,    26,   238,  -301,   166,  -301,    26,
     235,  -301,  -301,   243,  -301,  -301,  -301,  -301,  -301,  -301,
    -301,  -301,  -301,  -301,  -301,   457,  -301,  -301,  -301,  -301,
    -301,  -301,  -301,  -301,  -301,  -301,   457,  -301,    26,   353,
    -301,   457,   457,   254,  -301,  -301,  -301,  -301,  -301,  -301,
    -301,  -301,  -301,     8,  -301,   252,     7,  -301,    66,  -301,
    -301,   115,   260,  -301,    42,  -301,  -301,    26,   263,  -301,
     263,   242,  -301,   498,  -301,  -301,    26,   220,  -301,  -301,
      26,   276,  -301,   274,   279,  -301,   278,   114,  -301,  -301,
     104,  -301,  -301,  -301,   457,   277,  -301,  -301,  -301,  -301,
    -301,    83,  -301,    44,  -301,  -301,  -301,  -301,  -301,  -301,
      89,    -6,   281,  -301,  -301,   158,    26,   283,    95,   372,
    -301,    39,  -301,    25,    26,  -301,  -301,    26,   547,  -301,
    -301,     3,   159,  -301,  -301,  -301,    26,  -301,    26,   169,
    -301,  -301,  -301,   457,   457,   353,    26,   427,  -301,   193,
     457,  -301,  -301,    26,    26,    26,   412,   284,   241,  -301,
    -301,    41,   229,    84,   292,  -301,  -301,    26,   394,   289,
     421,   299,   353,  -301,   429,    26,  -301,    39,  -301,   304,
     306,  -301,  -301,   353,   369,  -301,   369,    -3,   252,  -301,
    -301,  -301,  -301,  -301,    26,   307,  -301,  -301,  -301,   252,
     311,   457,  -301,  -301,   308,   317,  -301,    26,    26,   318,
    -301,  -301,    16,    26,  -301,  -301,    26,   234,   442,   319,
     250,  -301,  -301,  -301,   103,    26,  -301,   320,   292,   292,
    -301,    71,  -301,   103,  -301,   194,   332,   457,   341,   461,
     196,  -301,  -301,  -301,    80,    30,   252,  -301,   457,   457,
     198,  -301,   239,  -301,  -301,  -301,  -301,  -301,  -301,    26,
    -301,   342,   337,  -301,  -301,  -301,   457,   346,   347,   251,
    -301,  -301,    26,  -301,   318,   470,   350,  -301,  -301,    27,
     234,  -301,   457,     5,  -301,    26,    60,    26,  -301,  -301,
    -301,  -301,    28,   343,   457,   354,  -301,   142,   353,  -301,
     353,  -301,   457,  -301,    82,  -301,   116,    26,  -301,    70,
     457,  -301,   352,    19,   457,  -301,  -301,  -301,   318,  -301,
    -301,   113,  -301,   234,  -301,   350,   117,  -301,  -301,   348,
    -301,   205,  -301,   357,   359,  -301,  -301,   457,  -301,  -301,
      89,   139,  -301,   457,  -301,   252,   355,   356,   358,  -301,
     361,    70,  -301,  -301,   156,    26,    26,  -301,    26,    53,
    -301,   351,  -301,   350,  -301,  -301,   457,   360,  -301,  -301,
     162,  -301,  -301,  -301,   393,   457,   457,  -301,  -301,  -301,
    -301,    26,   364,  -301,  -301,  -301,  -301,   173,  -301,  -301,
     367,   175,   176,   366,   457,  -301,  -301,  -301,  -301,   457,
     184,   191,  -301,  -301
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   239,     0,   239,
     239,     0,     0,     0,     0,     0,    23,     0,     0,     0,
       4,     8,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,   238,     0,   228,     0,
      32,     0,   239,    89,    88,     0,     0,    38,     0,     0,
       0,    43,    44,    45,     0,    50,    49,   227,     0,   239,
      90,   239,     0,     0,    28,    92,     0,     0,     1,     3,
       5,     9,     0,   246,     6,   244,   247,     0,     0,    42,
       0,     0,   240,     0,     0,   214,    36,     0,    38,     0,
       0,     0,   211,     0,    52,     0,    32,    56,     0,    57,
       0,    94,     0,    91,     0,     0,   106,     0,    25,     0,
       0,     7,   243,     0,   245,   230,   231,   232,   233,   234,
     235,   236,   237,   229,    41,     0,   197,   199,   198,   200,
     201,   202,   203,   143,   210,   209,   239,    35,     0,     0,
     172,     0,     0,     0,   171,    34,   168,   173,   174,   175,
     176,   177,   178,   170,   242,   239,   241,   215,   239,   213,
      37,     0,     0,    54,     0,    51,    55,     0,   239,    95,
     239,   107,   108,     0,   105,    24,     0,     0,    30,    27,
       0,     0,   207,     0,   206,   205,     0,     0,   183,   187,
     185,   179,   180,    33,     0,     0,   189,   188,   224,   222,
     216,   239,   218,     0,   220,   221,   223,    48,    47,    53,
     212,   239,     0,    59,    58,     0,     0,     0,   126,   130,
     131,   239,   132,   239,     0,   123,   111,     0,     0,   112,
     114,     0,     0,   128,   121,    26,     0,    29,     0,     0,
     249,    40,   204,     0,     0,     0,     0,     0,   191,     0,
     239,   217,   219,     0,     0,     0,     0,     0,   239,    66,
      61,   239,     0,     0,     0,    93,   109,     0,     0,     0,
       0,   239,     0,   119,     0,     0,   158,   239,   118,     0,
       0,   110,   113,     0,     0,   115,     0,    32,   239,   127,
     130,   129,    31,   251,     0,   239,   208,   169,   184,   239,
       0,     0,   190,   195,     0,   194,   193,     0,     0,   239,
      68,    60,     0,     0,    62,    79,     0,     0,     0,     0,
     239,    75,    70,    81,     0,     0,    46,     0,     0,     0,
     100,     0,    98,     0,    96,     0,     0,     0,     0,     0,
       0,   160,   163,   165,   164,     0,   239,   120,     0,     0,
       0,   133,     0,   142,   141,   134,   137,   136,   250,     0,
     257,     0,     0,   186,   192,   182,     0,     0,     0,     0,
      63,    80,     0,    67,   239,     0,   239,    77,    69,     0,
       0,    71,     0,     0,    84,     0,     0,     0,   101,   102,
      97,    99,     0,     0,     0,     0,   152,     0,     0,   159,
       0,   153,     0,   157,     0,   166,     0,     0,   140,   239,
       0,   259,   258,     0,   239,   196,   225,   226,   239,    64,
      78,     0,    72,     0,    76,   239,     0,    83,    85,     0,
      82,     0,   104,     0,     0,   116,   155,     0,   161,   162,
     164,     0,   125,     0,   124,   239,     0,     0,     0,   151,
       0,   145,   146,   144,     0,     0,     0,   256,     0,     0,
     252,     0,    65,   239,    73,    86,     0,     0,   122,   117,
       0,   154,   167,   135,     0,     0,     0,   138,   147,   139,
     260,     0,     0,   248,   253,   181,    74,     0,   103,   156,
       0,     0,     0,     0,     0,    87,   150,   148,   149,     0,
       0,     0,   254,   255
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -301,  -301,  -301,  -301,   489,    55,  -301,  -301,  -301,  -301,
    -301,  -301,    35,   265,  -301,   455,  -301,  -301,  -301,  -301,
     329,   122,  -301,  -301,   505,  -301,  -301,   362,    -7,   371,
    -301,  -301,  -222,  -287,  -301,  -301,  -300,  -260,  -290,  -301,
     185,   -34,  -301,  -301,  -301,  -301,  -301,  -301,  -301,  -301,
     190,    88,  -301,  -301,  -301,  -301,  -301,   296,  -301,  -301,
    -301,   302,  -301,  -301,   253,  -301,   124,  -301,  -301,    77,
    -301,  -189,   271,  -301,  -301,   138,   145,  -299,   144,  -301,
    -301,   261,   301,  -285,   262,  -301,   133,  -301,  -124,  -301,
    -301,  -301,  -301,  -301,  -301,   233,  -301,  -301,  -301,  -301,
     349,  -301,  -301,  -301,  -301,    -2,    18,    17,     2,   -62,
    -301,  -301,   474,  -301,  -301,   257,  -301,    93,  -301,  -301
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    22,   107,    23,    63,
     110,   177,    92,   143,    24,    45,    46,    25,    26,    78,
      27,    51,    52,    53,    54,    55,    93,    94,    98,   212,
     213,   257,   258,   259,   262,   319,   320,   321,   314,   326,
     383,   384,    28,    44,    29,    30,    31,   102,   265,   331,
     332,   334,    32,   105,   171,   174,   228,   229,   230,   231,
     232,   233,   285,   286,   351,   352,   144,   450,   451,   452,
     234,   273,   274,   275,   340,   341,   342,   404,   405,   146,
     147,   187,   188,   195,   196,   249,   304,   305,   148,   149,
     150,   151,   183,   184,   152,   385,    33,    84,   159,   201,
     202,   203,   204,   205,    34,   153,    38,   315,   154,   155,
      35,    74,    75,    76,   239,   240,   459,   460,   361,   412
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,    40,    41,   357,    47,    36,    42,    36,   -39,    41,
      62,    64,    82,    61,   362,   189,    36,   376,    36,   370,
     371,   157,    82,    82,    56,   373,    56,    56,    36,    36,
     381,   371,    36,   309,   278,    79,   168,    80,    39,    36,
      36,    36,   172,   456,    88,   382,    89,    90,    96,   271,
     406,   101,    96,   316,    36,    36,    96,    48,    49,    85,
      40,   403,   277,   271,    66,    67,    36,   316,   382,    97,
      41,   253,    48,    49,   113,    71,    56,   456,   106,     7,
     425,   156,   373,   426,   419,    59,   422,   254,   347,    91,
      73,   374,   327,   169,    50,   123,   156,   108,    96,   328,
      41,   329,   156,   441,   170,   327,   446,    40,   447,   163,
     198,   454,   328,   166,   329,    36,    36,   371,   238,   424,
     448,   189,   100,   463,   268,    71,    36,   198,   462,    73,
      57,   269,    60,   255,    81,   464,    96,   190,   470,   316,
     194,   427,    77,   382,   178,    77,   283,    77,   343,  -241,
     418,  -241,   457,   185,   266,   372,    97,    77,    77,   189,
     473,   424,    96,   401,   432,   210,   423,   487,   272,    83,
     402,    40,   197,   486,    40,   206,   491,   492,   156,    50,
     317,   103,   272,   289,   299,   214,   457,   214,   290,   483,
     220,   307,   308,   430,    50,   500,   125,    68,   222,   199,
     501,    69,   200,   449,   330,    86,    77,   390,   227,    40,
      77,   235,   164,   346,   156,   442,   199,   330,   206,   251,
     400,   104,   165,    81,    81,   145,    40,   443,   260,   287,
      77,   109,   238,   288,    40,   167,    77,    77,   276,    77,
     276,   247,   279,   190,   156,   280,   256,    36,   245,   444,
     465,   156,   156,    40,    77,   371,   293,   207,   246,    40,
     316,   443,   443,   227,    36,   335,   284,   306,   161,   181,
     344,   292,   471,   156,   343,   436,   343,    77,   322,   111,
     182,   190,   437,    95,   443,   191,   192,    99,   276,   479,
     256,    99,   156,   263,   276,   489,   318,   411,   264,   175,
     176,   443,   354,   294,   354,   197,   495,   443,   497,   498,
      40,    40,   360,   295,    40,    40,   197,   502,   443,   162,
     443,   443,    96,   386,   503,   367,   368,   301,    77,   443,
     398,    96,   245,    99,    43,   393,   443,   302,   248,   112,
     399,   124,   407,    65,    77,   445,   467,   256,   256,   428,
     490,   375,   318,   158,   236,    77,   237,   156,   428,   126,
     127,   128,   323,   197,   324,   160,    36,    40,   179,   325,
      40,   186,   408,   173,   409,   312,   216,    40,    40,   410,
     313,    96,    36,   133,   379,   431,   180,   296,   297,   380,
      96,   145,   193,   480,   303,   194,   344,    95,   440,   126,
     127,   128,   208,   429,   256,   156,   211,   256,   353,   241,
     353,    40,   242,   243,   318,   318,   388,   389,   250,    40,
     244,    40,   261,   267,   270,   310,   453,   333,   311,   337,
     336,   306,   338,   126,   127,   128,   129,   130,   131,   132,
      36,   133,   272,   345,   348,   364,   349,   359,   458,   363,
     365,   366,   369,   156,    40,   377,   318,    40,   318,   134,
     387,   378,   197,   126,   127,   128,   129,   130,   131,   132,
      36,   133,   394,   135,   396,   397,   482,   413,   414,   416,
     417,   395,     1,   420,   421,   433,   455,   435,   466,   134,
     468,   481,   469,   485,   458,   474,   475,   477,   476,   493,
     496,    87,   488,   135,   494,     2,   499,     3,    70,     4,
     415,    36,   300,     5,    58,     6,     7,     8,   392,     9,
     217,   391,   218,    10,   282,    11,   209,   219,   478,   220,
      12,    13,    14,    15,   291,   221,   438,   222,   434,   355,
     223,   215,   339,   224,   350,   439,   298,   461,   114,   356,
     252,   358,   484,     0,     0,     0,     0,     0,   303,     0,
      36,     0,     0,     0,   136,     0,   137,     0,   138,   217,
     139,   218,     0,   140,   141,   142,   219,     0,   220,     0,
       0,     0,     0,     0,   221,     0,   222,   472,     0,   223,
       0,     0,   224,     0,   136,     0,     0,     0,   138,     0,
     139,     0,     0,   140,   141,   142,     0,     0,     0,     0,
       2,     0,     3,     0,     4,    16,     0,     0,     5,     0,
       6,     7,     8,     0,     9,     0,     0,     0,    10,     0,
      11,   225,     0,     0,   226,    12,    13,    14,     2,    72,
       3,     0,     4,     0,     0,    36,     5,     0,     6,     7,
       8,     0,     9,     0,     0,     0,    10,     0,    11,   115,
     116,     0,   117,    12,    13,    14,     0,     0,     0,     0,
     118,     0,     0,     0,     0,   119,     0,     0,     0,   120,
     225,     0,     0,   281,   121,     0,     0,   122,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       2,     3,     4,   288,     6,    13,     4,    13,    13,    11,
      12,    13,    15,    11,   299,   139,    13,   317,    13,   309,
       4,    83,    15,    15,     7,   312,     9,    10,    13,    13,
     320,     4,    13,   255,   223,    37,    98,    39,     3,    13,
      13,    13,   104,    24,    46,    40,    48,    49,    50,    24,
     349,    58,    54,    26,    13,    13,    58,    21,    22,    42,
      62,   346,    37,    24,    18,    19,    13,    26,    40,    54,
      72,    27,    21,    22,    72,    20,    59,    24,    61,    34,
     380,    83,   369,   382,   374,    34,   376,    43,   277,    47,
      35,   313,    21,   100,   143,    77,    98,    62,   100,    28,
     102,    30,   104,   402,   102,    21,    36,   109,    38,    91,
      44,   410,    28,    95,    30,    13,    13,     4,   180,   379,
      50,   245,    20,   423,    29,    70,    13,    44,   418,    74,
      11,    36,    10,   139,   137,   425,   138,   139,   437,    26,
     143,   136,   150,    40,   109,   150,   143,   150,   272,   141,
     372,   143,   133,   136,   216,   139,    54,   150,   150,   283,
     445,   421,   164,   133,   136,   167,   139,   466,   143,   140,
     140,   173,   155,   463,   176,   158,   475,   476,   180,   143,
     139,    59,   143,    24,   246,   168,   133,   170,    29,   136,
      31,   253,   254,   133,   143,   494,   140,     0,    39,   133,
     499,     0,   136,   133,   133,   133,   150,   136,   173,   211,
     150,   176,   134,   275,   216,   133,   133,   133,   201,   136,
     140,   140,   144,   137,   137,    81,   228,   145,   211,   231,
     150,   135,   294,   231,   236,   149,   150,   150,   221,   150,
     223,   137,   224,   245,   246,   227,   211,    13,   134,   133,
     133,   253,   254,   255,   150,     4,   238,   142,   144,   261,
      26,   145,   145,   228,    13,   267,   231,   250,   141,   125,
     272,   236,   133,   275,   398,   133,   400,   150,   261,   133,
     136,   283,   140,    50,   145,   141,   142,    54,   271,   133,
     255,    58,   294,   135,   277,   133,   261,   359,   140,   133,
     134,   145,   284,   134,   286,   288,   133,   145,   133,   133,
     312,   313,   295,   144,   316,   317,   299,   133,   145,   141,
     145,   145,   324,   325,   133,   307,   308,   134,   150,   145,
     134,   333,   134,   100,     5,   141,   145,   144,   194,   133,
     144,   133,   144,    14,   150,   407,   141,   312,   313,   383,
     474,   316,   317,   135,   134,   150,   136,   359,   392,     6,
       7,     8,   133,   346,   135,   133,    13,   369,   133,   140,
     372,   138,   133,   135,   135,   134,   134,   379,   380,   140,
     139,   383,    13,    14,   134,   387,   143,   243,   244,   139,
     392,   247,   138,   455,   250,   143,   398,   164,   400,     6,
       7,     8,   142,   385,   369,   407,   143,   372,   284,   133,
     286,   413,   138,   134,   379,   380,   328,   329,   141,   421,
     142,   423,   141,   140,    52,    13,   409,   135,   144,   140,
      36,   414,    11,     6,     7,     8,     9,    10,    11,    12,
      13,    14,   143,    14,   140,   301,   140,   140,   413,   138,
     142,   134,   134,   455,   456,    13,   421,   459,   423,    32,
     140,   142,   445,     6,     7,     8,     9,    10,    11,    12,
      13,    14,   140,    46,   133,    14,   458,   135,   141,   133,
     133,   337,     0,    13,   134,   142,   134,   133,   140,    32,
     133,   456,   133,   142,   459,   140,   140,   136,   140,   481,
     133,    46,   142,    46,   140,    23,   140,    25,    19,    27,
     366,    13,   247,    31,     9,    33,    34,    35,   333,    37,
      22,   331,    24,    41,   228,    43,   164,    29,   451,    31,
      48,    49,    50,    51,   232,    37,   398,    39,   394,   286,
      42,   170,   271,    45,   283,   400,   245,   414,    74,   287,
     201,   294,   459,    -1,    -1,    -1,    -1,    -1,   414,    -1,
      13,    -1,    -1,    -1,   137,    -1,   139,    -1,   141,    22,
     143,    24,    -1,   146,   147,   148,    29,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    37,    -1,    39,   443,    -1,    42,
      -1,    -1,    45,    -1,   137,    -1,    -1,    -1,   141,    -1,
     143,    -1,    -1,   146,   147,   148,    -1,    -1,    -1,    -1,
      23,    -1,    25,    -1,    27,   133,    -1,    -1,    31,    -1,
      33,    34,    35,    -1,    37,    -1,    -1,    -1,    41,    -1,
      43,   133,    -1,    -1,   136,    48,    49,    50,    23,    52,
      25,    -1,    27,    -1,    -1,    13,    31,    -1,    33,    34,
      35,    -1,    37,    -1,    -1,    -1,    41,    -1,    43,    27,
      28,    -1,    30,    48,    49,    50,    -1,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    47,
     133,    -1,    -1,   136,    52,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     0,    23,    25,    27,    31,    33,    34,    35,    37,
      41,    43,    48,    49,    50,    51,   133,   152,   153,   154,
     155,   156,   157,   159,   165,   168,   169,   171,   193,   195,
     196,   197,   203,   247,   255,   261,    13,   256,   257,   163,
     256,   256,   259,   171,   194,   166,   167,   256,    21,    22,
     143,   172,   173,   174,   175,   176,   258,    11,   175,    34,
     172,   259,   256,   160,   256,   171,    18,    19,     0,     0,
     155,   156,    52,   156,   262,   263,   264,   150,   170,   256,
     256,   137,    15,   140,   248,   258,   133,   166,   256,   256,
     256,    47,   163,   177,   178,   246,   256,    54,   179,   246,
      20,   179,   198,   172,   140,   204,   258,   158,   163,   135,
     161,   133,   133,   259,   263,    27,    28,    30,    38,    43,
      47,    52,    55,   257,   133,   140,     6,     7,     8,     9,
      10,    11,    12,    14,    32,    46,   137,   139,   141,   143,
     146,   147,   148,   164,   217,   229,   230,   231,   239,   240,
     241,   242,   245,   256,   259,   260,   256,   260,   135,   249,
     133,   141,   141,   257,   134,   144,   257,   149,   260,   179,
     259,   205,   260,   135,   206,   133,   134,   162,   163,   133,
     143,   229,   229,   243,   244,   258,   246,   232,   233,   239,
     256,   229,   229,   138,   143,   234,   235,   258,    44,   133,
     136,   250,   251,   252,   253,   254,   258,   142,   142,   178,
     256,   143,   180,   181,   258,   180,   134,    22,    24,    29,
      31,    37,    39,    42,    45,   133,   136,   163,   207,   208,
     209,   210,   211,   212,   221,   163,   134,   136,   260,   265,
     266,   133,   138,   134,   142,   134,   144,   137,   229,   236,
     141,   136,   251,    27,    43,   139,   163,   182,   183,   184,
     258,   141,   185,   135,   140,   199,   260,   140,    29,    36,
      52,    24,   143,   222,   223,   224,   258,    37,   222,   257,
     257,   136,   208,   143,   163,   213,   214,   256,   259,    24,
      29,   212,   163,   257,   134,   144,   229,   229,   233,   260,
     164,   134,   144,   229,   237,   238,   258,   260,   260,   183,
      13,   144,   134,   139,   189,   258,    26,   139,   163,   186,
     187,   188,   258,   133,   135,   140,   190,    21,    28,    30,
     133,   200,   201,   135,   202,   256,    36,   140,    11,   223,
     225,   226,   227,   239,   256,    14,   260,   222,   140,   140,
     232,   215,   216,   217,   257,   215,   235,   234,   266,   140,
     258,   269,   234,   138,   229,   142,   134,   257,   257,   134,
     189,     4,   139,   184,   183,   163,   187,    13,   142,   134,
     139,   189,    40,   191,   192,   246,   256,   140,   202,   202,
     136,   201,   191,   141,   140,   229,   133,    14,   134,   144,
     140,   133,   140,   234,   228,   229,   228,   144,   133,   135,
     140,   260,   270,   135,   141,   229,   133,   133,   183,   189,
      13,   134,   189,   139,   188,   187,   228,   136,   192,   257,
     133,   256,   136,   142,   229,   133,   133,   140,   226,   227,
     256,   228,   133,   145,   133,   260,    36,    38,    50,   133,
     218,   219,   220,   258,   228,   134,    24,   133,   163,   267,
     268,   237,   189,   187,   189,   133,   140,   141,   133,   133,
     228,   133,   229,   234,   140,   140,   140,   136,   220,   133,
     260,   163,   257,   136,   268,   142,   189,   228,   142,   133,
     239,   228,   228,   257,   140,   133,   133,   133,   133,   140,
     228,   228,   133,   133
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   151,   152,   152,   153,   153,   153,   154,   155,   155,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   157,   158,   158,   159,   160,   161,
     162,   162,   163,   163,   164,   164,   165,   165,   166,   167,
     168,   169,   170,   171,   172,   172,   172,   173,   174,   175,
     175,   176,   177,   177,   178,   178,   179,   179,   180,   180,
     181,   182,   182,   182,   182,   182,   183,   183,   184,   185,
     186,   186,   186,   186,   186,   187,   187,   188,   188,   189,
     189,   190,   190,   190,   191,   191,   192,   192,   193,   194,
     195,   195,   196,   197,   198,   198,   199,   199,   200,   200,
     201,   201,   201,   201,   202,   203,   204,   204,   205,   205,
     206,   206,   207,   207,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   208,   209,   209,   210,   210,   211,   211,
     212,   212,   212,   213,   213,   214,   214,   214,   215,   215,
     215,   216,   216,   217,   218,   218,   219,   219,   220,   220,
     220,   220,   221,   222,   222,   222,   222,   223,   224,   224,
     225,   225,   226,   226,   227,   227,   228,   228,   229,   229,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   231,   231,   232,   232,   233,   233,   233,   234,   234,
     235,   236,   236,   237,   237,   238,   238,   239,   239,   239,
     240,   240,   241,   241,   242,   243,   243,   244,   244,   245,
     245,   246,   246,   247,   248,   248,   249,   249,   250,   250,
     251,   251,   251,   252,   252,   253,   254,   255,   256,   256,
     256,   256,   256,   256,   256,   256,   256,   256,   257,   258,
     259,   260,   260,   261,   262,   262,   263,   263,   264,   265,
     265,   266,   267,   267,   268,   268,   268,   269,   269,   270,
     270
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
       2,     8,     5,     1,     3,     1,     4,     1,     1,     1,
       3,     1,     3,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     3,     4,     1,     2,     2,     3,     1,     2,
       1,     1,     1,     1,     1,     5,     5,     2,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     0,
       2,     1,     1,     3,     1,     2,     1,     1,     9,     1,
       3,     2,     1,     2,     5,     6,     1,     1,     2,     1,
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
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1756 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 3: /* UNRECOGNIZED  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1764 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 4: /* ELLIPSIS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1772 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 5: /* INCREMENT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1780 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 6: /* DECIMAL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1788 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 7: /* OCTAL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1796 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 8: /* HEX  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 9: /* FLOAT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1812 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 10: /* EXP_FLOAT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1820 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 11: /* STRING  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1828 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 12: /* ESCAPED_STRING  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1836 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 13: /* IDENTIFIER_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1844 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 14: /* PHYSICAL_IDENTIFIER_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1852 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 15: /* VERSION  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1860 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 16: /* UNTERM_STRING  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1868 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 17: /* UNTERM_ESCAPED_STRING  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1876 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 18: /* VERS_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1884 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 19: /* VERS_2_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1892 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 20: /* KW___no_header  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1900 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 21: /* KW___row_length  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1908 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 22: /* KW___untyped  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1916 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 23: /* KW_alias  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1924 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 24: /* KW_column  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1932 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 25: /* KW_const  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1940 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 26: /* KW_control  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1948 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 27: /* KW_database  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1956 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 28: /* KW_decode  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1964 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 29: /* KW_default  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1972 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 30: /* KW_encode  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1980 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 31: /* KW_extern  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1988 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 32: /* KW_false  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 1996 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 33: /* KW_fmtdef  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2004 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 34: /* KW_function  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2012 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 35: /* KW_include  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2020 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 36: /* KW_limit  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2028 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 37: /* KW_physical  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2036 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 38: /* KW_read  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2044 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 39: /* KW_readonly  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2052 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 40: /* KW_return  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2060 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 41: /* KW_schema  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2068 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 42: /* KW_static  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2076 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 43: /* KW_table  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2084 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 44: /* KW_template  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2092 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 45: /* KW_trigger  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2100 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 46: /* KW_true  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2108 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 47: /* KW_type  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2116 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 48: /* KW_typedef  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2124 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 49: /* KW_typeset  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2132 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 50: /* KW_validate  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2140 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 51: /* KW_version  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2148 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 52: /* KW_view  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2156 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 53: /* KW_virtual  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2164 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 54: /* KW_void  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2172 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 55: /* KW_write  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2180 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 56: /* PT_ASTLIST  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2188 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 57: /* PT_PARSE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2196 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 58: /* PT_SOURCE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2204 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 59: /* PT_VERSION_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2212 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 60: /* PT_VERSION_2  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2220 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 61: /* PT_SCHEMA_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2228 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 62: /* PT_SCHEMA_2_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2236 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 63: /* PT_INCLUDE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2244 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 64: /* PT_TYPEDEF  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2252 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 65: /* PT_FQN  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2260 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 66: /* PT_IDENT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2268 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 67: /* PT_PHYSIDENT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2276 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 68: /* PT_UINT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2284 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 69: /* PT_TYPESET  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 70: /* PT_TYPESETDEF  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2300 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 71: /* PT_FORMAT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2308 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 72: /* PT_CONST  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2316 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 73: /* PT_ALIAS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2324 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 74: /* PT_EXTERN  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2332 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 75: /* PT_FUNCTION  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2340 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 76: /* PT_UNTYPED  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2348 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 77: /* PT_ROWLENGTH  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2356 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 78: /* PT_FUNCDECL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2364 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 79: /* PT_EMPTY  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 80: /* PT_SCHEMASIG  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2380 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 81: /* PT_SCHEMAFORMAL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2388 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 82: /* PT_RETURNTYPE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2396 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 83: /* PT_FACTSIG  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2404 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 84: /* PT_FUNCSIG  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2412 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 85: /* PT_FUNCPARAMS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2420 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 86: /* PT_FORMALPARAM  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2428 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 87: /* PT_ELLIPSIS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2436 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 88: /* PT_FUNCPROLOGUE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2444 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 89: /* PT_RETURN  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2452 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 90: /* PT_PRODSTMT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 91: /* PT_PRODTRIGGER  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2468 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 92: /* PT_SCHEMA  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2476 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 93: /* PT_VALIDATE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2484 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 94: /* PT_PHYSICAL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2492 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 95: /* PT_PHYSPROLOGUE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2500 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 96: /* PT_PHYSSTMT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2508 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 97: /* PT_PHYSBODYSTMT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2516 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 98: /* PT_TABLE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2524 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 99: /* PT_TABLEPARENTS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2532 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 100: /* PT_TABLEBODY  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2540 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 101: /* PT_FUNCEXPR  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2548 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 102: /* PT_FACTPARMS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2556 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 103: /* PT_COLUMN  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2564 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 104: /* PT_COLUMNEXPR  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2572 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 105: /* PT_COLDECL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2580 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 106: /* PT_TYPEDCOL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2588 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 107: /* PT_COLSTMT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2596 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 108: /* PT_DFLTVIEW  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2604 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 109: /* PT_PHYSMBR  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2612 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 110: /* PT_PHYSCOL  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2620 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 111: /* PT_PHYSCOLDEF  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2628 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 112: /* PT_COLSCHEMAPARMS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2636 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 113: /* PT_COLSCHEMAPARAM  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2644 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 114: /* PT_COLUNTYPED  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2652 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 115: /* PT_DATABASE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2660 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 116: /* PT_TYPEEXPR  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2668 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 117: /* PT_DBBODY  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2676 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 118: /* PT_DBDAD  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2684 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 119: /* PT_DBMEMBER  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 120: /* PT_TBLMEMBER  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2700 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 121: /* PT_NOHEADER  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2708 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 122: /* PT_CASTEXPR  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2716 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 123: /* PT_CONSTVECT  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2724 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 124: /* PT_NEGATE  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2732 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 125: /* PT_UNARYPLUS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2740 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 126: /* PT_VERSNAME  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2748 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 127: /* PT_ARRAY  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2756 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 128: /* PT_PHYSENCREF  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2764 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 129: /* PT_TYPEDCOLEXPR  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2772 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 130: /* PT_VIEW  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2780 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 131: /* PT_VIEWPARAM  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2788 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 132: /* PT_VIEWPARENTS  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2796 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 133: /* ';'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 134: /* ','  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2812 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 135: /* '{'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2820 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 136: /* '}'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2828 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 137: /* '['  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2836 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 138: /* ']'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2844 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 139: /* '*'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2852 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 140: /* '='  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2860 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 141: /* '('  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2868 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 142: /* ')'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2876 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 143: /* '<'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2884 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 144: /* '>'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2892 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 145: /* '|'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2900 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 146: /* '@'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2908 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 147: /* '-'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2916 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 148: /* '+'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2924 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 149: /* '/'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2932 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 150: /* ':'  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2940 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 152: /* parse  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2948 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 153: /* source  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2956 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 154: /* version_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2964 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 155: /* schema_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2972 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 156: /* schema_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2980 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 157: /* typedef_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2988 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 158: /* typedef_1_0_new_name_list  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 2996 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 159: /* typeset_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3004 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 160: /* typeset_1_0_new_name  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3012 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 161: /* typeset_1_0_def  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3020 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 162: /* typespec_1_0_list  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3028 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 163: /* typespec_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3036 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 164: /* dim_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3044 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 165: /* format_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3052 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 166: /* format_1_0_new_name  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3060 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 167: /* format_1_0_name  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3068 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 168: /* const_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3076 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 169: /* alias_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3084 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 170: /* alias_1_0_new_name  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3092 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 171: /* function_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3100 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 172: /* func_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3108 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 173: /* untyped_func_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3116 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 174: /* row_length_func_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3124 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 175: /* opt_func_1_0_schema_sig  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3132 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 176: /* func_1_0_schema_sig  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3140 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 177: /* func_1_0_schema_formals  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3148 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 178: /* func_1_0_schema_formal  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3156 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 179: /* func_1_0_return_type  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3164 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 180: /* opt_func_1_0_fact_sig  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3172 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 181: /* func_1_0_fact_sig  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3180 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 182: /* func_1_0_fact_signature  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3188 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 183: /* func_1_0_fact_params  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3196 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 184: /* fact_param_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3204 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 185: /* func_1_0_param_sig  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3212 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 186: /* func_1_0_param_signature  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3220 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 187: /* func_1_0_formal_params  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3228 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 188: /* formal_param_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3236 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 189: /* func_1_0_vararg_formals  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3244 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 190: /* func_1_0_prologue  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3252 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 191: /* script_1_0_stmt_seq  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3260 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 192: /* script_1_0_stmt  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3268 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 193: /* extern_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3276 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 194: /* ext_func_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3284 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 195: /* script_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 196: /* validate_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3300 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 197: /* physical_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3308 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 198: /* phys_1_0_return_type  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3316 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 199: /* phys_1_0_prologue  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3324 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 200: /* phys_1_0_body  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3332 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 201: /* phys_1_0_body_stmt  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3340 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 202: /* phys_1_0_stmt  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3348 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 203: /* table_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3356 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 204: /* opt_tbl_1_0_parents  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3364 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 205: /* tbl_1_0_parents  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 206: /* tbl_1_0_body  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3380 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 207: /* tbl_1_0_stmt_seq  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3388 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 208: /* tbl_1_0_stmt  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3396 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 209: /* production_1_0_stmt  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3404 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 210: /* col_1_0_modifiers  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3412 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 211: /* col_1_0_modifier_seq  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3420 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 212: /* col_1_0_modifier  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3428 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 213: /* col_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3436 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 214: /* phys_enc_ref  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3444 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 215: /* typed_column_decl_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3452 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 216: /* col_ident  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 217: /* phys_ident  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3468 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 218: /* column_body_1_0_opt  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3476 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 219: /* column_body_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3484 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 220: /* column_stmt_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3492 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 221: /* default_view_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3500 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 222: /* physmbr_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3508 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 223: /* phys_coldef_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3516 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 224: /* opt_col_schema_parms_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3524 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 225: /* col_schema_parms_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3532 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 226: /* col_schema_parm_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3540 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 227: /* col_schema_value_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3548 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 228: /* cond_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3556 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 229: /* expression_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3564 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 230: /* primary_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3572 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 231: /* func_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3580 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 232: /* schema_parms_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3588 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 233: /* schema_parm_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3596 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 234: /* opt_factory_parms_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3604 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 235: /* factory_parms_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3612 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 236: /* factory_parms  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3620 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 237: /* opt_func_1_0_parms  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3628 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 238: /* func_1_0_parms  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3636 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 239: /* uint_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3644 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 240: /* float_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3652 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 241: /* string_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3660 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 242: /* const_vect_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3668 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 243: /* opt_const_vect_exprlist_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3676 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 244: /* const_vect_exprlist_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3684 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 245: /* bool_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 246: /* type_expr_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3700 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 247: /* database_1_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3708 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 248: /* opt_database_dad_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3716 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 249: /* database_body_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3724 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 250: /* database_members_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3732 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 251: /* database_member_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3740 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 252: /* opt_template_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3748 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 253: /* db_member_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3756 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 254: /* table_member_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3764 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 255: /* include_directive  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3772 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 256: /* fqn_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3780 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 257: /* ident_1_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3788 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 258: /* empty  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3796 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 259: /* fqn_vers  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 260: /* fqn_opt_vers  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3812 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 261: /* version_2_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3820 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 262: /* schema_2_0  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3828 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 263: /* schema_2_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3836 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 264: /* view_2_0_decl  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3844 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 265: /* view_parms  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3852 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 266: /* view_parm  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3860 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 267: /* view_body  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3868 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 268: /* view_member  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3876 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 269: /* opt_view_parents  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3884 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
        break;

    case 270: /* view_parents  */
#line 138 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1257  */
      {
    delete ( ParseTree * ) ( ((*yyvaluep)) . subtree );
}
#line 3892 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1257  */
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
#line 295 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root = MakeTree ( PT_PARSE, T ( (yyvsp[0]) ) );              (yyval) . subtree = 0; (yyval) . leading_ws = 0;  }
#line 4186 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 296 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root = MakeTree ( PT_PARSE, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) );    (yyval) . subtree = 0; (yyval) . leading_ws = 0; }
#line 4192 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 300 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[0]) ) ); }
#line 4198 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 301 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4204 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 302 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4210 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 308 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_1_0, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4216 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 312 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_1_0, P ( (yyvsp[0]) ) ); }
#line 4222 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 4228 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 317 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4234 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 318 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4240 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 319 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4246 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 320 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4252 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4258 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 322 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4264 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4270 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 324 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4276 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 325 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4282 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 326 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4288 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 327 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4294 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 328 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4300 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 329 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4306 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 330 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4312 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 340 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDEF, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4318 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 344 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4324 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 345 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P((yyvsp[-2])), T((yyvsp[-1])), P((yyvsp[0])) ); }
#line 4330 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 352 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESET, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4336 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 356 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4342 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESETDEF, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4348 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4354 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 368 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4360 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4366 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 373 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 377 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4378 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 378 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4384 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 385 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4390 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 387 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4396 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 391 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4402 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 395 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4408 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 403 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4414 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 410 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ALIAS, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4420 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 414 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4426 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 420 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCTION, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4432 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 424 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4438 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 425 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4444 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 432 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCDECL, P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4450 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 437 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNTYPED, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4456 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 442 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ROWLENGTH, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4462 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 446 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4468 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 447 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4474 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 451 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMASIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4480 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 455 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4486 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 456 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4492 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4498 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 461 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4504 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, T ( (yyvsp[0]) ) ); }
#line 4510 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 466 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, P ( (yyvsp[0]) ) ); }
#line 4516 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 470 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4522 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 471 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4528 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 475 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4534 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 479 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4540 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 481 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4546 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 483 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4552 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 485 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4558 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 487 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4564 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 491 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4570 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 492 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4576 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 496 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4582 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 500 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4588 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 504 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4594 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 506 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4600 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 508 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4606 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 510 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4612 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 512 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4618 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 516 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4624 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 517 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4630 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 521 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4636 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 523 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4642 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 527 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4648 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 528 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ELLIPSIS, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4654 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 533 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[0]) ) ); }
#line 4660 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 535 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4666 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 537 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4672 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 541 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4678 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 542 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4684 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 546 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURN, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4690 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 548 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4696 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 555 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EXTERN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4702 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 559 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4708 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 567 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4714 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 569 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4720 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 577 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VALIDATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4726 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 589 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSICAL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4732 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 593 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4738 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 594 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NOHEADER, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4744 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 599 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4750 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 601 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4756 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 605 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4762 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 606 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4768 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 611 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[0]) ) ); }
#line 4774 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 613 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4780 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 615 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4786 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 617 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4792 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 622 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSSTMT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4798 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 632 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 636 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4810 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 637 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4816 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 641 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4822 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 642 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4828 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 647 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4834 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 649 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4840 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 653 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4846 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 654 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4852 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 658 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4858 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 659 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4864 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 661 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4870 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 663 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4876 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 664 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4882 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 665 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4888 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 666 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4894 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 667 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4900 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 669 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUNTYPED, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4906 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 670 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4912 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 675 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4918 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 677 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODTRIGGER, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4924 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4930 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 682 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); AddToList ( P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 4936 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 686 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 4942 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4948 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 691 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4954 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4960 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 4966 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 698 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4972 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 700 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4978 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 705 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4984 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 707 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4990 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 709 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 4996 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 714 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5002 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 716 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOLEXPR, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5008 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 718 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5014 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 722 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5020 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 723 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5026 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 727 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5032 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 731 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5038 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 732 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5044 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 736 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5050 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 737 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5056 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 741 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5062 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 742 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5068 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 743 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5074 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 744 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5080 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 749 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DFLTVIEW, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5086 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 754 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5092 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 756 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5098 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 758 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5104 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 760 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5110 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 765 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOLDEF, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5116 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 769 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5122 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 770 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5128 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 774 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5134 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 775 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5140 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 779 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5146 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 780 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5152 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 784 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5158 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 785 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5164 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 792 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5170 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 793 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5176 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 797 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5182 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 798 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CASTEXPR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5188 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 802 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5194 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 803 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5200 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5206 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 805 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5212 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 806 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5218 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 807 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5224 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 808 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5230 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 809 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5236 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 810 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5242 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 811 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5248 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 812 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNARYPLUS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5254 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 822 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, T ( (yyvsp[-7]) ), P ( (yyvsp[-6]) ), T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5260 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 828 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5266 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 832 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5272 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 833 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5278 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 837 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5284 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 838 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5290 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 839 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5296 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 843 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5302 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 844 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5308 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 848 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5314 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 852 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5320 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 853 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5326 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 857 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5332 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 858 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5338 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 862 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5344 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 863 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5350 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 867 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5356 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 868 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5362 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 869 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 5368 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 873 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5374 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 874 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5380 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 878 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5386 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 879 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5392 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 883 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONSTVECT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5398 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 887 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5404 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 888 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5410 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 892 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5416 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 893 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5422 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 897 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5428 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 898 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5434 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 902 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5440 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 903 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1])), P ( (yyvsp[0]) ) ); }
#line 5446 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 913 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2])), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5452 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 917 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5458 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 918 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBDAD, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5464 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 922 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5470 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 923 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5476 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 927 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5482 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 928 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5488 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 932 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5494 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 933 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5500 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 934 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5506 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 938 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5512 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 939 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5518 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 944 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5524 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 949 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TBLMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5530 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 956 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5536 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 964 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FQN, P ( (yyvsp[0]) ) ); }
#line 5542 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 965 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5548 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 967 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5554 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 968 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5560 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 969 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5566 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 970 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5572 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 971 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5578 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 972 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5584 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 973 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5590 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 974 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyvsp[0]) . type = IDENTIFIER_1_0; (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ) ); }
#line 5596 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 978 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 5602 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 982 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EMPTY ); }
#line 5608 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 986 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSNAME, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5614 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 990 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5620 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 991 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5626 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 997 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_2, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5632 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 1001 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_2_0, P ( (yyvsp[0]) ) ); }
#line 5638 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 1002 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 5644 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 1006 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5650 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 1007 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5656 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 1012 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEW, T ( (yyvsp[-8]) ), P ( (yyvsp[-7]) ), T ( (yyvsp[-6]) ), P ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5662 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 249:
#line 1016 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5668 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 250:
#line 1017 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5674 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 251:
#line 1021 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARAM, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5680 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 252:
#line 1025 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5686 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 253:
#line 1026 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 254:
#line 1031 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5698 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 255:
#line 1033 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 5704 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 256:
#line 1034 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 5710 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 257:
#line 1038 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5716 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 258:
#line 1039 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VIEWPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5722 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 259:
#line 1043 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 5728 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 260:
#line 1044 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 5734 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 5738 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
