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
    PT_PHYSENCEXPR = 382,
    PT_PHYSENCREF = 383,
    PT_TYPEDCOLEXPR = 384
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

#line 361 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:358  */

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
#define YYLAST   637

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  149
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  113
/* YYNRULES -- Number of rules.  */
#define YYNRULES  248
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  473

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   384

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,   131,     2,     2,     2,
     139,   140,   137,   145,   132,   146,     2,   147,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   148,   130,
     141,   138,   142,     2,   144,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   135,     2,   136,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   133,   143,   134,     2,     2,     2,     2,
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
     125,   126,   127,   128,   129
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   282,   282,   283,   287,   288,   289,   293,   297,   301,
     307,   308,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   331,   339,   340,   346,
     351,   355,   362,   363,   367,   368,   372,   373,   379,   381,
     386,   390,   397,   404,   409,   415,   419,   420,   421,   431,
     436,   441,   442,   446,   450,   451,   455,   456,   460,   461,
     465,   466,   470,   474,   475,   477,   479,   481,   486,   487,
     491,   495,   499,   500,   502,   504,   506,   511,   512,   516,
     517,   522,   523,   527,   529,   531,   536,   537,   541,   542,
     549,   554,   561,   563,   571,   578,   588,   589,   593,   595,
     600,   601,   605,   607,   609,   611,   616,   623,   631,   632,
     636,   637,   641,   643,   648,   649,   653,   654,   655,   657,
     659,   660,   661,   662,   663,   665,   669,   671,   676,   677,
     681,   682,   686,   687,   688,   692,   694,   699,   701,   703,
     708,   710,   712,   717,   718,   722,   726,   727,   731,   732,
     736,   737,   738,   739,   743,   748,   750,   752,   754,   759,
     764,   765,   769,   770,   774,   775,   779,   780,   787,   788,
     792,   793,   797,   798,   799,   800,   801,   802,   803,   804,
     805,   806,   807,   811,   816,   825,   830,   831,   835,   836,
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
  "PT_AT", "PT_PHYSENCEXPR", "PT_PHYSENCREF", "PT_TYPEDCOLEXPR", "';'",
  "'$'", "','", "'{'", "'}'", "'['", "']'", "'*'", "'='", "'('", "')'",
  "'<'", "'>'", "'|'", "'@'", "'+'", "'-'", "'/'", "':'", "$accept",
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
  "col_1_0_modifiers", "col_1_0_modifier_seq", "col_1_0_modifier",
  "col_1_0_decl", "phys_enc_ref", "typed_column_decl_1_0", "col_ident",
  "phys_ident", "column_body_1_0_opt", "column_body_1_0",
  "column_stmt_1_0", "default_view_1_0_decl", "physmbr_1_0_decl",
  "phys_coldef_1_0", "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
  "col_schema_parm_1_0", "col_schema_value_1_0", "cond_expr_1_0",
  "expression_1_0", "primary_expr_1_0", "func_expr_1_0", "phys_enc_expr",
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
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
      59,    36,    44,   123,   125,    91,    93,    42,    61,    40,
      41,    60,    62,   124,    64,    43,    45,    47,    58
};
# endif

#define YYPACT_NINF -295

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-295)))

#define YYTABLE_NINF -248

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     456,  -295,     3,     3,     3,   133,     3,    21,    77,    28,
      43,     3,     3,     3,   133,   116,  -295,   173,   229,   507,
     101,   507,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,   -12,  -295,     3,
      71,    -9,    98,  -295,  -295,   125,     3,    -6,     3,     3,
     159,  -295,  -295,  -295,    67,  -295,  -295,  -295,    82,    21,
    -295,   138,   -12,   166,   182,  -295,   203,   204,  -295,  -295,
     507,  -295,  -295,  -295,   275,   243,   182,  -101,   331,  -295,
       3,   244,  -295,  -295,   255,   182,   148,   207,     3,  -295,
      33,  -295,     3,    93,  -295,     3,  -295,    67,  -295,     3,
    -295,     3,   259,  -295,   237,  -295,     3,   280,  -295,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
     422,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,   422,  -295,     3,   431,  -295,   422,   394,   269,  -295,
    -295,  -295,  -295,   272,  -295,  -295,  -295,  -295,  -295,  -295,
       8,  -295,   274,    -9,  -295,   -18,  -295,  -295,   273,   281,
    -295,   159,  -295,  -295,     3,   282,  -295,   282,   292,  -295,
     472,  -295,  -295,     3,   264,  -295,  -295,   295,  -295,   291,
     308,  -295,   301,   115,  -295,  -295,    76,  -295,  -295,  -295,
    -295,   182,  -295,   422,   422,   303,  -295,  -295,  -295,  -295,
     -16,  -295,   168,  -295,  -295,  -295,  -295,  -295,  -295,   182,
      18,   304,  -295,  -295,    97,     3,   307,    46,   395,  -295,
     -15,  -295,    55,     3,  -295,  -295,     3,   494,  -295,  -295,
      11,   100,  -295,  -295,  -295,     3,  -295,  -295,  -295,   422,
     422,   431,     3,   331,  -295,   309,   315,  -295,  -295,   135,
     422,  -295,  -295,     3,     3,     3,   435,   310,   216,  -295,
    -295,    13,   257,    69,   317,  -295,  -295,     3,   416,   316,
     444,   319,   431,  -295,   447,     3,  -295,   -15,  -295,   320,
     324,  -295,  -295,   431,   194,  -295,   194,    35,   274,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,   274,   327,  -295,   422,
     422,  -295,   325,     3,     3,   332,  -295,  -295,    58,     3,
    -295,  -295,     3,   201,   458,   329,   251,  -295,  -295,  -295,
     118,     3,  -295,   335,   317,   317,  -295,    70,  -295,   118,
    -295,   226,   336,   422,   349,   467,   165,  -295,  -295,  -295,
     176,   -19,   274,  -295,   422,   422,   190,  -295,   284,  -295,
    -295,  -295,   422,  -295,  -295,  -295,  -295,  -295,  -295,   353,
     354,   362,  -295,  -295,     3,  -295,   332,   474,   359,  -295,
    -295,    54,   201,  -295,   422,    47,  -295,     3,   -21,     3,
    -295,  -295,  -295,  -295,    53,   357,   422,   364,  -295,   175,
     431,  -295,   431,  -295,   422,  -295,   103,  -295,   120,     3,
    -295,    68,   422,   217,  -295,  -295,   332,  -295,  -295,   184,
    -295,   201,  -295,   359,   122,  -295,  -295,   361,  -295,   233,
    -295,   371,   379,  -295,  -295,   422,  -295,  -295,   182,   123,
    -295,   422,  -295,   274,   373,   374,   376,  -295,   384,    68,
    -295,  -295,   126,  -295,  -295,   359,  -295,  -295,   422,   380,
    -295,  -295,   141,  -295,  -295,  -295,   344,   422,   422,  -295,
    -295,  -295,  -295,   143,  -295,  -295,   389,   155,   163,  -295,
    -295,  -295,  -295
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
       0,   199,   201,   200,   202,   203,   204,   205,   145,   212,
     211,   245,    37,     0,     0,   174,     0,     0,     0,   173,
      36,   170,   175,     0,   176,   177,   178,   179,   180,   181,
     172,   248,   245,   247,   221,   245,   219,    39,     0,     0,
      56,     0,    53,    57,     0,   245,    97,   245,   109,   110,
       0,   107,    26,     0,     0,    32,    29,     0,   209,     0,
     208,   207,     0,     0,   186,   190,   188,   182,   214,   215,
     216,   213,    35,   245,     0,     0,   191,   230,   228,   222,
     245,   224,     0,   226,   227,   229,    50,    49,    55,   218,
     245,     0,    61,    60,     0,     0,     0,   128,   132,   133,
     245,   134,   245,     0,   125,   113,     0,     0,   114,   116,
       0,     0,   130,   123,    28,     0,    31,    42,   206,     0,
       0,     0,     0,     0,   197,     0,   196,   195,   193,     0,
     245,   223,   225,     0,     0,     0,     0,     0,   245,    68,
      63,   245,     0,     0,     0,    95,   111,     0,     0,     0,
       0,   245,     0,   121,     0,     0,   160,   245,   120,     0,
       0,   112,   115,     0,     0,   117,     0,    34,   245,   129,
     132,   131,    33,   210,   171,   187,   245,     0,   183,     0,
       0,   192,     0,     0,     0,   245,    70,    62,     0,     0,
      64,    81,     0,     0,     0,     0,   245,    77,    72,    83,
       0,     0,    48,     0,     0,     0,   102,     0,   100,     0,
      98,     0,     0,     0,     0,     0,     0,   162,   165,   167,
     166,     0,   245,   122,     0,     0,     0,   135,     0,   144,
     143,   136,     0,   138,   185,   189,   198,   194,   184,     0,
       0,     0,    65,    82,     0,    69,   245,     0,   245,    79,
      71,     0,     0,    73,     0,     0,    86,     0,     0,     0,
     103,   104,    99,   101,     0,     0,     0,     0,   154,     0,
       0,   161,     0,   155,     0,   159,     0,   168,     0,     0,
     142,   245,     0,     0,   231,   232,   245,    66,    80,     0,
      74,     0,    78,   245,     0,    85,    87,     0,    84,     0,
     106,     0,     0,   118,   157,     0,   163,   164,   166,     0,
     127,     0,   126,   245,     0,     0,     0,   153,     0,   147,
     148,   146,     0,   139,    67,   245,    75,    88,     0,     0,
     124,   119,     0,   156,   169,   137,     0,     0,     0,   140,
     149,   141,    76,     0,   105,   158,     0,     0,     0,    89,
     152,   150,   151
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -295,  -295,  -295,  -295,  -295,  -295,   502,    24,  -295,  -295,
    -295,  -295,  -295,  -295,    48,   283,  -295,   477,  -295,  -295,
    -295,  -295,   375,    62,  -295,  -295,   516,  -295,  -295,   366,
     -24,   367,  -295,  -295,  -172,  -268,  -295,  -295,  -265,  -294,
    -284,  -295,   199,     9,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,  -295,   209,   -14,  -295,  -295,  -295,  -295,  -295,   318,
    -295,  -295,  -295,   311,  -295,  -295,   258,  -295,  -133,  -295,
    -295,   107,  -295,  -192,   277,  -295,  -295,   160,   161,   -94,
     -61,  -295,  -295,  -295,   268,   321,  -274,   200,   314,  -295,
     -98,   421,  -295,  -295,  -295,  -295,  -295,  -295,   162,  -295,
    -295,  -295,  -295,   360,  -295,  -295,  -295,  -295,     7,   -59,
      -7,     1,   -52
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    72,    21,    22,    23,   104,
      24,    63,   107,   174,    89,   138,    25,    45,    46,    26,
      27,    75,    28,    51,    52,    53,    54,    55,    90,    91,
      95,   211,   212,   257,   258,   259,   262,   315,   316,   317,
     310,   322,   375,   376,    29,    44,    30,    31,    32,    99,
     265,   327,   328,   330,    33,   102,   168,   171,   227,   228,
     229,   230,   231,   232,   285,   286,   347,   348,   139,   438,
     439,   440,   233,   273,   274,   275,   336,   337,   338,   396,
     397,   141,   142,   143,   183,   184,   195,   249,   245,   246,
     144,   145,   146,   147,   179,   180,   148,   149,   377,    34,
      81,   156,   200,   201,   202,   203,   204,    35,   150,    38,
     311,   151,   152
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,    36,    56,    56,   188,    42,    79,   -41,   271,    37,
      40,    41,    61,    47,   353,   118,    36,   140,    41,    62,
      64,   362,   354,    79,    36,   197,    36,   197,   154,   160,
     278,    36,   373,   163,    98,    82,   185,   120,   312,   189,
     365,    48,    49,   165,    76,    73,    77,    74,   368,   169,
      79,    39,    56,    85,   103,    86,    87,    93,   363,   177,
      36,    93,   363,    48,    49,    93,    36,    36,   395,    40,
     178,    36,    60,   166,   268,   187,    59,   412,   271,   312,
      36,   269,   407,   305,   410,   343,   374,   153,    57,   323,
     323,   277,   374,   365,    73,    36,   324,   324,   325,   325,
     167,    97,   153,   434,    93,   435,    41,   413,   153,   418,
     105,   393,   198,    40,   198,   412,   199,   436,   251,   394,
      94,   100,   444,   289,   181,    66,   272,    74,   290,   446,
     219,    36,   244,   248,    67,    94,    74,   366,   221,    74,
      93,   186,    74,   185,   191,   196,   445,  -247,   205,  -247,
     313,   349,   283,   349,   175,   255,    74,   374,   213,   455,
     213,   462,    50,   266,   279,   161,     7,   280,    93,    50,
      78,   209,    36,    68,   339,   162,   352,    40,   293,   294,
      40,   415,   140,    74,    50,   185,   247,   420,   363,   244,
     296,   411,   406,   205,   253,   364,   272,    36,   437,   326,
     326,   303,   304,   260,   382,    88,    78,    36,   128,   312,
     254,   243,    92,   276,    36,   276,    96,    40,   226,    74,
      96,   234,   153,   342,    74,   350,   312,   350,    78,    69,
     263,   288,    71,   430,    40,   264,    80,   287,   356,   357,
     164,    74,    40,   247,   359,   360,   431,   241,   186,   153,
     432,   398,   447,   453,   318,    83,   461,   242,   256,    96,
     153,   153,    40,   431,   276,   431,   431,   300,    40,   431,
     276,   465,   387,   469,   331,   226,   101,   301,   284,   340,
     414,   196,   153,   292,   431,   471,   431,   158,    36,   196,
     186,   248,   339,   472,   339,   182,    74,   390,   431,   106,
     429,   110,   111,   256,   112,   424,   431,   391,   442,   314,
     380,   381,   113,   425,   392,    40,    40,   114,   417,    40,
      40,   115,   241,    92,    74,   422,   116,    93,   378,   117,
      74,   452,   399,   108,   109,   196,    93,   121,   122,   123,
     124,   125,   126,   127,    36,   128,   159,   433,   308,   300,
     121,   122,   123,   309,   463,    74,   256,   256,   466,   443,
     367,   314,   129,   467,   468,   385,   363,   172,    40,   173,
     454,    40,   449,   119,    74,    36,   130,   155,    40,    40,
      43,    74,    93,   371,   416,   157,   419,   319,   372,    65,
     320,    93,   170,   416,   441,   321,   235,   340,   236,   428,
     121,   122,   123,   124,   125,   192,   153,    36,   128,   256,
     176,   193,   256,   206,   400,   194,    40,   401,    40,   314,
     314,   207,   402,   210,   215,   237,   196,   238,   121,   122,
     123,   124,   125,   126,   127,    36,   128,   121,   122,   123,
     239,   240,   250,   261,    36,   267,   270,   299,   306,   298,
     329,   332,   307,   129,   333,   334,     1,   314,   344,   314,
     272,   341,   345,   355,   361,   358,   131,   130,   132,   370,
     133,   369,   134,   379,   386,   135,   136,   137,     2,   388,
       3,   389,     4,   404,   405,    36,     5,   408,     6,     7,
       8,   409,     9,   216,   423,   217,    10,   421,    11,   448,
     218,   450,   219,    12,    13,    14,    15,    36,   220,   451,
     221,   456,   457,   222,   458,   216,   223,   217,   459,   470,
     464,    70,   218,    84,   219,    58,   297,   208,   384,     2,
     220,     3,   221,     4,   214,   222,   383,     5,   223,     6,
       7,     8,   291,     9,   351,   282,   460,    10,   335,    11,
     426,   346,   403,   427,    12,    13,    14,   131,   190,     0,
     252,   133,   295,   134,   302,     0,   135,   136,   137,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,     0,   281,     0,
       0,     0,     0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       7,    13,     9,    10,   137,     4,    15,    13,    23,     2,
       3,     4,    11,     6,   288,    74,    13,    78,    11,    12,
      13,   305,   296,    15,    13,    43,    13,    43,    80,    88,
     222,    13,   316,    92,    58,    42,   134,   138,    25,   137,
     308,    20,    21,    95,    37,    21,    39,   148,   313,   101,
      15,     3,    59,    46,    61,    48,    49,    50,     4,   120,
      13,    54,     4,    20,    21,    58,    13,    13,   342,    62,
     131,    13,    10,    97,    28,   136,    33,   371,    23,    25,
      13,    35,   366,   255,   368,   277,    39,    80,    11,    20,
      20,    36,    39,   361,    70,    13,    27,    27,    29,    29,
      99,    19,    95,    35,    97,    37,    99,   372,   101,   130,
      62,   130,   130,   106,   130,   409,   134,    49,   134,   138,
      53,    59,   406,    23,   131,     9,   141,   148,    28,   413,
      30,    13,   193,   194,    18,    53,   148,   309,    38,   148,
     133,   134,   148,   241,   137,   152,   411,   139,   155,   141,
     137,   284,   141,   286,   106,   137,   148,    39,   165,   433,
     167,   445,   141,   215,   223,   132,    33,   226,   161,   141,
     135,   164,    13,     0,   272,   142,   141,   170,   239,   240,
     173,   134,   243,   148,   141,   283,   193,   134,     4,   250,
     242,   137,   364,   200,    26,   137,   141,    13,   130,   130,
     130,   253,   254,   210,   134,    46,   135,    13,    14,    25,
      42,   135,    50,   220,    13,   222,    54,   210,   170,   148,
      58,   173,   215,   275,   148,   284,    25,   286,   135,     0,
     133,   230,   131,   130,   227,   138,   138,   230,   299,   300,
     147,   148,   235,   250,   303,   304,   143,   132,   241,   242,
     130,   345,   130,   130,   261,   130,   130,   142,   210,    97,
     253,   254,   255,   143,   271,   143,   143,   132,   261,   143,
     277,   130,   333,   130,   267,   227,   138,   142,   230,   272,
     374,   288,   275,   235,   143,   130,   143,   139,    13,   296,
     283,   352,   390,   130,   392,   133,   148,   132,   143,   133,
     394,    26,    27,   255,    29,   130,   143,   142,   402,   261,
     324,   325,    37,   138,   138,   308,   309,    42,   377,   312,
     313,    46,   132,   161,   148,   386,    51,   320,   321,    54,
     148,   425,   142,   130,   130,   342,   329,     6,     7,     8,
       9,    10,    11,    12,    13,    14,   139,   399,   132,   132,
       6,     7,     8,   137,   448,   148,   308,   309,   456,   142,
     312,   313,    31,   457,   458,   139,     4,   130,   361,   132,
     431,   364,   139,   130,   148,    13,    45,   133,   371,   372,
       5,   148,   375,   132,   375,   130,   379,   130,   137,    14,
     133,   384,   133,   384,   401,   138,   132,   390,   134,   392,
       6,     7,     8,     9,    10,   136,   399,    13,    14,   361,
     130,   139,   364,   140,   130,   141,   409,   133,   411,   371,
     372,   140,   138,   141,   132,   130,   433,   136,     6,     7,
       8,     9,    10,    11,    12,    13,    14,     6,     7,     8,
     132,   140,   139,   139,    13,   138,    51,   132,    13,   140,
     133,    35,   142,    31,   138,    11,     0,   409,   138,   411,
     141,    14,   138,   136,   132,   140,   135,    45,   137,   140,
     139,    13,   141,   138,   138,   144,   145,   146,    22,   130,
      24,    14,    26,   130,   130,    13,    30,    13,    32,    33,
      34,   132,    36,    21,   130,    23,    40,   140,    42,   138,
      28,   130,    30,    47,    48,    49,    50,    13,    36,   130,
      38,   138,   138,    41,   138,    21,    44,    23,   134,   130,
     140,    19,    28,    46,    30,     9,   243,   161,   329,    22,
      36,    24,    38,    26,   167,    41,   327,    30,    44,    32,
      33,    34,   231,    36,   286,   227,   439,    40,   271,    42,
     390,   283,   352,   392,    47,    48,    49,   135,   137,    -1,
     200,   139,   241,   141,   250,    -1,   144,   145,   146,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,    -1,   134,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     0,    22,    24,    26,    30,    32,    33,    34,    36,
      40,    42,    47,    48,    49,    50,   130,   150,   151,   152,
     153,   155,   156,   157,   159,   165,   168,   169,   171,   193,
     195,   196,   197,   203,   248,   256,    13,   257,   258,   163,
     257,   257,   260,   171,   194,   166,   167,   257,    20,    21,
     141,   172,   173,   174,   175,   176,   259,    11,   175,    33,
     172,   260,   257,   160,   257,   171,     9,    18,     0,     0,
     155,   131,   154,   156,   148,   170,   257,   257,   135,    15,
     138,   249,   259,   130,   166,   257,   257,   257,    46,   163,
     177,   178,   247,   257,    53,   179,   247,    19,   179,   198,
     172,   138,   204,   259,   158,   163,   133,   161,   130,   130,
      26,    27,    29,    37,    42,    46,    51,    54,   258,   130,
     138,     6,     7,     8,     9,    10,    11,    12,    14,    31,
      45,   135,   137,   139,   141,   144,   145,   146,   164,   217,
     229,   230,   231,   232,   239,   240,   241,   242,   245,   246,
     257,   260,   261,   257,   261,   133,   250,   130,   139,   139,
     258,   132,   142,   258,   147,   261,   179,   260,   205,   261,
     133,   206,   130,   132,   162,   163,   130,   229,   229,   243,
     244,   259,   247,   233,   234,   239,   257,   229,   217,   239,
     240,   257,   136,   139,   141,   235,   259,    43,   130,   134,
     251,   252,   253,   254,   255,   259,   140,   140,   178,   257,
     141,   180,   181,   259,   180,   132,    21,    23,    28,    30,
      36,    38,    41,    44,   130,   134,   163,   207,   208,   209,
     210,   211,   212,   221,   163,   132,   134,   130,   136,   132,
     140,   132,   142,   135,   229,   237,   238,   259,   229,   236,
     139,   134,   252,    26,    42,   137,   163,   182,   183,   184,
     259,   139,   185,   133,   138,   199,   261,   138,    28,    35,
      51,    23,   141,   222,   223,   224,   259,    36,   222,   258,
     258,   134,   208,   141,   163,   213,   214,   257,   260,    23,
      28,   212,   163,   229,   229,   234,   261,   164,   140,   132,
     132,   142,   237,   261,   261,   183,    13,   142,   132,   137,
     189,   259,    25,   137,   163,   186,   187,   188,   259,   130,
     133,   138,   190,    20,    27,    29,   130,   200,   201,   133,
     202,   257,    35,   138,    11,   223,   225,   226,   227,   239,
     257,    14,   261,   222,   138,   138,   233,   215,   216,   217,
     258,   215,   141,   235,   235,   136,   229,   229,   140,   258,
     258,   132,   189,     4,   137,   184,   183,   163,   187,    13,
     140,   132,   137,   189,    39,   191,   192,   247,   257,   138,
     202,   202,   134,   201,   191,   139,   138,   229,   130,    14,
     132,   142,   138,   130,   138,   235,   228,   229,   228,   142,
     130,   133,   138,   236,   130,   130,   183,   189,    13,   132,
     189,   137,   188,   187,   228,   134,   192,   258,   130,   257,
     134,   140,   229,   130,   130,   138,   226,   227,   257,   228,
     130,   143,   130,   261,    35,    37,    49,   130,   218,   219,
     220,   259,   228,   142,   189,   187,   189,   130,   138,   139,
     130,   130,   228,   130,   229,   235,   138,   138,   138,   134,
     220,   130,   189,   228,   140,   130,   239,   228,   228,   130,
     130,   130,   130
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   149,   150,   150,   151,   151,   151,   152,   153,   154,
     155,   155,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   157,   158,   158,   159,
     160,   161,   162,   162,   163,   163,   164,   164,   165,   165,
     166,   167,   168,   169,   170,   171,   172,   172,   172,   173,
     174,   175,   175,   176,   177,   177,   178,   178,   179,   179,
     180,   180,   181,   182,   182,   182,   182,   182,   183,   183,
     184,   185,   186,   186,   186,   186,   186,   187,   187,   188,
     188,   189,   189,   190,   190,   190,   191,   191,   192,   192,
     193,   194,   195,   195,   196,   197,   198,   198,   199,   199,
     200,   200,   201,   201,   201,   201,   202,   203,   204,   204,
     205,   205,   206,   206,   207,   207,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   209,   209,   210,   210,
     211,   211,   212,   212,   212,   213,   213,   214,   214,   214,
     215,   215,   215,   216,   216,   217,   218,   218,   219,   219,
     220,   220,   220,   220,   221,   222,   222,   222,   222,   223,
     224,   224,   225,   225,   226,   226,   227,   227,   228,   228,
     229,   229,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   231,   231,   232,   233,   233,   234,   234,
     234,   235,   235,   236,   236,   237,   237,   238,   238,   239,
     239,   239,   240,   240,   241,   241,   242,   243,   243,   244,
     244,   245,   245,   246,   246,   246,   246,   247,   247,   248,
     249,   249,   250,   250,   251,   251,   252,   252,   252,   253,
     253,   254,   255,   256,   257,   257,   257,   257,   257,   257,
     257,   257,   257,   257,   258,   259,   260,   261,   261
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
       1,     1,     2,     4,     5,     5,     1,     3,     1,     4,
       1,     1,     3,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       3,     1,     1,     2,     2,     2,     2,     1,     3,     4,
       1,     2,     2,     3,     1,     2,     1,     1,     1,     1,
       1,     5,     5,     2,     1,     3,     3,     3,     3,     3,
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
#line 282 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, T ( (yyvsp[0]) ) ); }
#line 1984 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 283 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { *root =  MakeTree ( PT_PARSE, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 1990 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 287 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[0]) ) ); }
#line 1996 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 288 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2002 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 289 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SOURCE, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2008 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 293 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_1_0, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2014 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 297 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSION_2, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2020 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 301 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2026 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 307 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA_1_0, P ( (yyvsp[0]) ) ); }
#line 2032 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 308 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ) , P ( (yyvsp[0]) ) ); }
#line 2038 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 312 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2044 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 313 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2050 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 314 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2056 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 315 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2062 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 316 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2068 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 317 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2074 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 318 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2080 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 319 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2086 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 320 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2092 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 321 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2098 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 322 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2104 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 323 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2110 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 324 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2116 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 325 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2122 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 335 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDEF, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2128 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 339 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2134 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 340 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P((yyvsp[-2])), T((yyvsp[-1])), P((yyvsp[0])) ); }
#line 2140 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 347 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESET, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2146 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 351 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2152 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 356 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPESETDEF, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2158 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 362 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2164 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 363 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2170 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 367 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2176 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 368 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2182 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 372 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2188 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 373 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2194 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 380 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2200 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 382 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMAT, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2206 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 386 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2212 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 390 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2218 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 398 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONST, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2224 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 405 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ALIAS, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2230 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 409 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2236 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 415 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCTION, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2242 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 419 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2248 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 420 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2254 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 427 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCDECL, P ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2260 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 432 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNTYPED, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2266 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 437 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ROWLENGTH, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2272 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 441 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2278 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 442 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2284 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 446 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMASIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2290 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 450 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2296 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 451 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2302 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 455 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2308 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 456 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2314 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, T ( (yyvsp[0]) ) ); }
#line 2320 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 461 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURNTYPE, P ( (yyvsp[0]) ) ); }
#line 2326 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 465 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2332 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 466 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2338 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 470 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2344 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 474 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2350 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 476 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2356 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 478 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2362 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 480 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2368 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 482 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2374 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 486 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2380 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 487 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2386 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 491 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2392 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 495 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCSIG, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2398 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 499 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2404 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 501 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2410 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 503 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2416 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 505 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2422 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 507 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPARAMS, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2428 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 511 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2434 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 512 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2440 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 516 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2446 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 518 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FORMALPARAM, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2452 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 522 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2458 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 523 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ELLIPSIS, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2464 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 528 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[0]) ) ); }
#line 2470 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 530 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2476 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 532 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2482 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 536 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2488 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 537 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2494 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 541 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_RETURN, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2500 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 543 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2506 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 550 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EXTERN, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2512 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 554 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2518 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 562 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2524 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 564 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_SCHEMA, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2530 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 572 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VALIDATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2536 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 584 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSICAL, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2542 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 588 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2548 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 589 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NOHEADER, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2554 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 594 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2560 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 596 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2566 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 600 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2572 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 601 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2578 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 606 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[0]) ) ); }
#line 2584 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 608 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2590 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 610 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2596 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 612 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2602 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 617 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSSTMT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2608 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 627 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2614 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 631 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2620 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 632 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEPARENTS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2626 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 636 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2632 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 637 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2638 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 642 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2644 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 644 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TABLEBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2650 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 648 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2656 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 649 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2662 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 653 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2668 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 654 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMN, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2674 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 656 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2680 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 658 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUMNEXPR, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2686 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 659 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2692 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 660 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2698 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 661 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOL, T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2704 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 662 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2710 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 664 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLUNTYPED, T ( (yyvsp[-5]) ), T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2716 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 665 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2722 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 670 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODSTMT, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2728 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 672 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PRODTRIGGER, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2734 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 676 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2740 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 677 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); AddToList ( P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2746 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 681 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2752 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 682 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2758 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 686 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2764 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 687 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2770 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 688 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2776 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 693 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2782 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 695 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLDECL, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2788 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 700 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2794 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 702 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2800 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 704 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCREF, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2806 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 709 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2812 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 711 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOLEXPR, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2818 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 713 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEDCOL, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2824 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 717 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2830 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 718 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2836 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 722 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2842 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 726 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2848 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 727 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2854 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 731 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2860 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 732 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2866 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 736 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2872 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 737 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2878 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 738 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSTMT, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2884 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 739 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 2890 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 744 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DFLTVIEW, T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2896 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 749 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2902 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 751 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2908 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 753 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2914 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 755 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSMBR, T ( (yyvsp[-5]) ), P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2920 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 760 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSCOLDEF, P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2926 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 764 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2932 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 765 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 2938 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 769 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2944 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 770 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2950 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 774 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2956 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 775 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2962 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 779 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2968 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 780 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2974 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 787 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 2980 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 788 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2986 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 792 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2992 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 793 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CASTEXPR, T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 2998 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 797 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3004 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 798 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3010 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 799 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3016 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 800 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3022 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 801 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3028 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 802 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3034 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 803 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3040 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 804 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3046 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 805 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3052 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 806 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3058 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 807 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UNARYPLUS, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3064 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 815 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3070 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 821 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FUNCEXPR, P ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3076 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 826 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_PHYSENCEXPR, T ( (yyvsp[-4]) ), P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3082 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 830 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3088 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 831 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3094 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 835 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3100 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 836 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_ARRAY, P ( (yyvsp[-3]) ), T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3106 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 837 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3112 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 841 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3118 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 842 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FACTPARMS, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3124 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 846 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3130 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 847 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3136 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 851 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3142 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 852 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3148 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 856 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3154 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 857 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3160 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 861 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3166 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 862 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3172 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 863 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_UINT, T ( (yyvsp[0]) ) ); }
#line 3178 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 867 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3184 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 868 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3190 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 872 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3196 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 873 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3202 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 877 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_CONSTVECT, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3208 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 881 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3214 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 882 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3220 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 886 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3226 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 887 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3232 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 891 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3238 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 892 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3244 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 896 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3250 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 897 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3256 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 898 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3262 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 899 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_NEGATE, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3268 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 903 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3274 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 904 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TYPEEXPR, P ( (yyvsp[-2]) ), T ( (yyvsp[-1])), P ( (yyvsp[0]) ) ); }
#line 3280 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 914 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DATABASE, T ( (yyvsp[-3]) ), P ( (yyvsp[-2])), P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3286 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 918 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3292 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 919 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBDAD, T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3298 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 923 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3304 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 924 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBBODY, T ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3310 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 928 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeList ( (yyvsp[0]) ); }
#line 3316 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 929 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3322 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 933 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3328 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 934 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3334 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 935 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3340 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 939 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3346 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 940 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = T ( (yyvsp[0]) ); }
#line 3352 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 945 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_DBMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3358 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 950 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_TBLMEMBER, P ( (yyvsp[-4]) ), T ( (yyvsp[-3]) ), P ( (yyvsp[-2]) ), P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3364 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 957 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_INCLUDE, T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3370 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 965 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_FQN, P ( (yyvsp[0]) ) ); }
#line 3376 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 966 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), P ( (yyvsp[0]) ) ); }
#line 3382 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 968 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3388 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 969 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3394 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 970 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3400 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 971 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3406 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 972 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3412 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 973 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3418 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 242:
#line 974 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3424 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 243:
#line 975 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = AddToList ( P ( (yyvsp[-2]) ), T ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3430 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 244:
#line 979 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_IDENT, T ( (yyvsp[0]) ) ); }
#line 3436 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 245:
#line 983 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_EMPTY ); }
#line 3442 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 246:
#line 987 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) . subtree = MakeTree ( PT_VERSNAME, P ( (yyvsp[-1]) ), T ( (yyvsp[0]) ) ); }
#line 3448 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 247:
#line 991 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3454 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;

  case 248:
#line 992 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3460 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
    break;


#line 3464 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-grammar.cpp" /* yacc.c:1646  */
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
