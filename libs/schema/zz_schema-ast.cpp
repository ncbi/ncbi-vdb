/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

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

/* First part of user prologue.  */

    #define YYDEBUG 1

    #include <stdio.h>

    #include "ASTBuilder.hpp"
    using namespace ncbi::SchemaParser;

    #include "schema-ast.hpp"

    #define AST_lex NextToken
    static int NextToken ( YYSTYPE * p_token, ParseTreeScanner & p_sb )
    {
        return p_sb . NextToken ( p_token -> tok );
    }

    void AST_error ( ctx_t ctx, void * p_parser, ASTBuilder & p_builder, ParseTreeScanner & p_sb, const char * p_msg )
    {
        INTERNAL_ERROR ( xcUnexpected, "%s: %s", p_sb . GetSourceFileName (), p_msg );
    }



# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_AST_HPP_INCLUDED
# define YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_AST_HPP_INCLUDED
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



int AST_parse (ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb);

#endif /* !YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_AST_HPP_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

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
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
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
#define YYLAST   1057

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  155
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  113
/* YYNRULES -- Number of rules.  */
#define YYNRULES  244
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  859

#define YYUNDEFTOK  2
#define YYMAXUTOK   390


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
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
static const yytype_int16 yyrline[] =
{
       0,   262,   262,   263,   267,   268,   269,   273,   277,   281,
     282,   286,   291,   292,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   316,
     321,   322,   326,   327,   331,   335,   336,   340,   345,   346,
     350,   351,   355,   359,   363,   367,   368,   369,   374,   375,
     379,   380,   384,   385,   389,   390,   394,   395,   399,   403,
     404,   405,   406,   407,   411,   415,   416,   420,   421,   425,
     426,   430,   431,   432,   436,   440,   441,   445,   446,   450,
     451,   455,   458,   459,   463,   468,   477,   482,   483,   486,
     488,   493,   494,   498,   500,   502,   504,   511,   516,   517,
     521,   522,   526,   527,   531,   532,   536,   537,   538,   539,
     540,   541,   542,   543,   544,   548,   552,   553,   557,   558,
     562,   563,   564,   568,   569,   573,   575,   576,   580,   581,
     582,   586,   587,   591,   592,   596,   597,   601,   602,   603,
     604,   608,   610,   612,   614,   619,   623,   624,   628,   629,
     633,   634,   638,   639,   644,   648,   649,   652,   653,   657,
     658,   659,   660,   664,   666,   671,   672,   678,   684,   688,
     689,   693,   694,   695,   696,   697,   698,   699,   700,   701,
     702,   703,   704,   705,   706,   710,   715,   716,   719,   720,
     724,   725,   726,   730,   731,   735,   739,   740,   744,   745,
     749,   750,   754,   755,   756,   760,   761,   765,   766,   770,
     774,   775,   779,   782,   786,   787,   791,   792,   804,   806,
     821,   825,   826,   830,   834,   835,   839,   844,   850,   851,
     855,   859,   860,   864,   865,   869,   870,   871,   875,   876,
     880,   881,   885,   890,   891
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
static const yytype_int16 yytoknum[] =
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

#define YYPACT_NINF (-675)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     112,   -41,   179,    53,  -675,    44,    54,   244,  -675,   221,
      70,    93,   169,   175,  -675,   111,   255,   195,  -675,   290,
     302,   401,   202,   222,   220,  -675,   348,   354,   242,   246,
     251,   257,   258,   272,   297,   298,   299,   300,   301,   303,
     305,   307,  -675,   230,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
     401,  -675,   228,   306,   404,   399,   400,   417,   426,   429,
     423,   421,   415,   407,   422,   418,   433,   410,  -675,  -675,
     263,   325,   328,   456,   403,   403,   403,     8,   403,   394,
     309,    16,   405,   268,   355,   355,   355,  -675,  -675,  -675,
     344,   346,   430,   419,   -18,   351,   403,  -675,  -675,   403,
     353,   361,   364,   365,   366,   414,   368,   369,   371,  -675,
     372,   -23,   373,   101,    88,   356,  -675,   440,   374,   375,
     376,   380,   403,   377,   381,  -675,   493,   500,   268,  -675,
     386,   268,  -675,   488,   378,   389,   390,  -675,   355,   403,
    -675,   391,   432,  -675,   392,   413,   477,   397,   121,     8,
     393,  -675,   398,   406,   185,   408,   403,   403,   453,  -675,
     453,   460,   483,    20,   521,   208,   527,   409,   412,   411,
     416,   420,   425,   532,  -675,   484,   236,  -675,   495,  -675,
      91,  -675,  -675,  -675,  -675,  -675,  -675,  -675,   427,   428,
     431,   434,   435,   436,   439,   442,  -675,   424,  -675,  -675,
    -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
     443,   444,   -22,   -22,   445,   446,   447,   450,  -675,   451,
     453,  -675,   454,   457,   455,   498,   448,   -22,   452,  -675,
     438,   459,   458,   449,     8,   461,  -675,   462,  -675,   384,
     437,   463,   464,   465,   467,   490,   490,   468,   470,   471,
    -675,   208,  -675,   208,   268,   478,  -675,   403,  -675,   472,
     473,   466,   474,  -675,   481,   504,   476,   -19,   482,   265,
    -675,  -675,   553,   485,  -675,     8,   486,   487,   489,   491,
     511,   -22,    -7,   514,   185,   185,   494,     7,   480,  -675,
     492,   496,   515,   515,   453,   501,   270,  -675,   497,  -675,
     213,   267,  -675,   -22,   502,   499,  -675,   503,   505,   -22,
     506,   438,   507,  -675,  -675,   274,  -675,  -675,  -675,  -675,
     509,   139,   510,   513,   517,   518,   564,   519,   490,   545,
    -675,  -675,   516,   523,   534,   -22,    -8,   512,   478,   403,
    -675,   522,   520,   546,   530,   277,  -675,   261,   524,   210,
    -675,   490,     2,  -675,  -675,   525,     8,    22,  -675,   526,
     528,  -675,   185,   185,  -675,  -675,   529,  -675,   531,   535,
     533,   536,   537,   538,  -675,   540,   541,   208,   490,   490,
     542,  -675,   543,   -12,   544,   547,   548,   549,   -22,   551,
     552,   554,   555,   556,   557,  -675,  -675,   129,  -675,  -675,
     558,   560,   561,  -675,   177,  -675,   562,  -675,   565,   508,
     563,  -675,  -675,   278,  -675,  -675,  -675,   539,   172,   566,
    -675,   281,  -675,  -675,   185,    10,   213,   238,  -675,   559,
    -675,   515,   567,   568,  -675,  -675,   570,   572,   -33,  -675,
     550,   569,  -675,  -675,    -7,   587,    32,   591,   232,   611,
    -675,  -675,  -675,   133,   133,   571,  -675,  -675,  -675,   573,
     181,   575,   576,    22,   579,   574,  -675,   580,  -675,   578,
     185,   141,  -675,   583,   490,   585,   588,   581,   403,   523,
    -675,  -675,   597,   -17,  -675,   590,   582,   572,   593,   592,
      67,  -675,   586,   490,   490,  -675,   595,   594,    39,   599,
      18,   589,  -675,  -675,   614,   608,   596,   602,   600,  -675,
     598,   601,  -675,  -675,   603,   185,   604,   607,  -675,  -675,
     185,  -675,   609,   610,  -675,   612,   613,   615,   617,   284,
    -675,   618,   577,   572,   -17,  -675,    12,   605,  -675,   620,
     619,   621,   341,   622,   623,   625,   624,   626,   628,   599,
     630,   403,   -22,   -22,  -675,   627,    65,   631,  -675,   185,
     285,  -675,  -675,  -675,  -675,  -675,   308,   633,   634,     1,
    -675,   597,  -675,   660,   -17,   635,   629,   616,   632,   636,
     638,   308,   545,   545,  -675,  -675,  -675,   315,  -675,    13,
    -675,   637,   185,    21,  -675,   639,  -675,   641,   490,   490,
     646,   643,   644,  -675,    83,  -675,  -675,  -675,   288,  -675,
     645,    98,  -675,  -675,  -675,   647,     8,   653,  -675,   648,
     649,  -675,   403,   651,   652,  -675,  -675,   157,   654,   655,
     657,  -675,   659,   -43,   -43,   185,   658,   584,   661,   676,
    -675,   662,   663,   664,   667,   289,  -675,     8,   685,  -675,
    -675,   642,   185,   670,   606,  -675,  -675,   720,   668,  -675,
    -675,   671,   640,   665,   656,   669,   672,  -675,   -14,   674,
     675,   677,   678,   666,   679,   722,    97,   106,   680,   682,
     683,   -22,   684,   646,   490,     8,   686,  -675,   545,   687,
     688,  -675,   689,   690,   699,   691,  -675,  -675,   702,   697,
     139,    47,    47,  -675,  -675,   692,  -675,   127,  -675,   681,
     -22,   693,   545,   694,  -675,  -675,   695,  -675,  -675,   698,
     490,  -675,   696,  -675,  -675,   700,   703,   704,   701,   706,
     707,   708,  -675,   174,  -675,   705,  -675,   709,   545,   710,
     139,  -675,   711,  -675,   735,   545,   712,   714,  -675,   308,
     308,  -675,    22,  -675,  -675,   715,   193,   545,  -675,   716,
     736,   718,   721,   717,   723,   545,  -675,   173,   187,   292,
    -675,   724,  -675,   719,   726,   725,   729,  -675,  -675,   490,
     730,   728,   727,   731,   732,    59,   733,   737,  -675,    89,
    -675,   293,  -675,   739,   740,   741,   -22,   744,  -675,   117,
    -675,  -675,  -675,   745,   -16,  -675,  -675,  -675,  -675,   738,
     490,  -675,   746,   747,   139,   204,  -675,  -675,   403,   742,
    -675,   749,  -675,  -675,  -675,   750,   743,   748,   751,   752,
     753,  -675,  -675,   673,   545,   545,     4,  -675,   734,   754,
     756,   758,   761,   762,   763,  -675,  -675,  -675,  -675
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,     0,     0,     0,     2,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     3,     0,
       0,     0,     0,     0,     0,     4,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,     0,    12,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     5,
       0,     6,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     9,    13,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,     7,     8,
       0,     0,     0,     0,     0,     0,     0,    33,    32,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,   167,     0,     0,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,    44,
       0,     0,    82,     0,     0,     0,     0,    87,     0,     0,
      98,     0,     0,   155,     0,     0,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   220,     0,     0,    30,     0,    41,
       0,   205,   206,   207,   208,   172,   211,   210,     0,     0,
       0,     0,     0,     0,     0,     0,   173,     0,   174,   175,
     176,   177,   178,   179,   180,   182,   183,   184,   171,    43,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
       0,    56,     0,     0,     0,     0,     0,     0,     0,   154,
       0,     0,     0,     0,     0,     0,    36,     0,    35,     0,
     186,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,   225,     0,     0,     0,    54,     0,    55,     0,
       0,     0,     0,   226,     0,   102,     0,     0,     0,     0,
     228,   221,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    42,
       0,     0,     0,     0,     0,     0,     0,    50,     0,    88,
       0,     0,    86,     0,     0,     0,   156,     0,     0,     0,
       0,     0,     0,    29,    38,     0,    34,   202,   204,   203,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      45,    46,     0,     0,     0,     0,     0,     0,     0,     0,
      59,     0,     0,     0,     0,     0,   100,     0,     0,     0,
     158,     0,     0,   229,   222,     0,     0,     0,   193,     0,
       0,   194,     0,     0,   212,   181,     0,   217,     0,     0,
       0,     0,     0,     0,    73,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   114,   106,     0,   104,   107,
       0,     0,     0,   160,     0,   159,     0,   238,     0,     0,
       0,    39,   191,     0,   188,   192,   190,     0,     0,     0,
     200,     0,   223,   216,     0,     0,     0,     0,    47,     0,
      84,     0,     0,     0,    49,   215,     0,     0,     0,    57,
       0,     0,    99,   101,     0,     0,     0,     0,     0,     0,
     103,   105,    97,     0,     0,     0,   162,   161,   230,     0,
       0,     0,     0,     0,     0,     0,   198,     0,   213,     0,
       0,     0,   169,     0,     0,     0,     0,     0,     0,     0,
      52,    53,     0,     0,    69,     0,     0,     0,     0,     0,
       0,    91,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,   166,   165,     0,     0,     0,     0,     0,   231,
       0,     0,   187,   189,     0,     0,     0,     0,   201,   168,
       0,   219,     0,     0,    71,     0,     0,     0,     0,     0,
      65,     0,     0,     0,     0,    60,     0,     0,    92,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   157,     0,     0,     0,    37,     0,
       0,   185,   209,   170,   218,    58,     0,     0,     0,     0,
      64,     0,    61,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   120,   121,   122,     0,   118,     0,
     115,     0,     0,     0,   111,     0,   110,     0,     0,     0,
       0,     0,     0,   237,     0,   233,   227,   196,     0,   199,
       0,     0,    75,    78,    72,     0,     0,     0,    66,     0,
       0,    62,     0,     0,     0,    93,    90,     0,     0,     0,
       0,   119,     0,     0,     0,     0,     0,     0,     0,     0,
     112,     0,     0,     0,     0,     0,   240,     0,     0,   232,
     234,     0,     0,     0,     0,    76,    85,     0,     0,    70,
      63,     0,     0,     0,     0,     0,     0,   117,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   197,     0,     0,
       0,    67,     0,     0,     0,     0,    79,    80,     0,     0,
       0,     0,     0,   123,   124,     0,   108,     0,   146,     0,
       0,     0,     0,     0,   163,   164,     0,   239,   241,     0,
       0,   195,     0,    74,    68,     0,     0,     0,     0,     0,
       0,     0,   132,     0,   131,     0,   109,     0,     0,     0,
       0,   141,     0,   113,     0,     0,     0,     0,    96,     0,
       0,    89,     0,   127,   126,     0,     0,     0,   142,     0,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
     130,     0,   133,     0,     0,     0,     0,   145,   143,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   144,     0,
     243,     0,   235,     0,     0,     0,     0,     0,   137,     0,
     135,   128,   129,     0,     0,   148,   151,   153,   152,     0,
       0,   236,     0,     0,     0,     0,   134,   136,     0,     0,
     149,     0,   244,    94,    95,     0,     0,     0,     0,     0,
       0,   242,   125,     0,     0,     0,     0,   147,     0,     0,
       0,     0,     0,     0,     0,   150,   140,   138,   139
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -675,  -675,  -675,  -675,  -675,   798,  -675,   768,    36,  -675,
    -675,   -82,  -355,  -675,  -675,  -675,  -675,  -675,  -675,   759,
    -675,  -106,  -675,   323,  -139,  -235,  -280,   324,  -413,  -675,
     233,  -455,   249,   479,  -575,  -607,  -293,  -675,  -675,   755,
    -675,  -675,  -675,  -675,  -675,   333,  -675,  -675,  -675,  -675,
    -675,   650,  -675,  -675,  -675,   237,  -675,  -675,   192,   125,
    -675,  -675,    40,  -415,   227,  -675,  -675,    46,    23,  -675,
    -675,  -675,  -675,   469,   441,  -675,  -574,  -675,  -149,  -675,
    -675,   130,   475,  -674,   194,  -675,  -675,   379,  -356,  -675,
    -675,  -675,  -675,  -675,  -675,  -279,  -675,  -675,   -84,  -675,
    -246,  -220,   -88,  -675,  -675,   713,  -675,  -675,   294,  -675,
    -675,   209,  -675
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    15,    16,    17,    24,    43,    44,    45,
     186,   228,   107,   247,    46,   325,    47,    48,    49,    50,
     114,   121,   306,   307,   147,   233,   343,   352,   448,   539,
     540,   498,   383,   384,   621,   622,   623,    51,    52,   117,
      53,    54,   148,   272,   500,   501,    55,   152,   355,   315,
     407,   408,   409,   507,   597,   598,   554,   644,   681,   743,
     783,   809,   810,   558,   649,   720,   814,   815,   816,    56,
     155,   181,   414,   415,   514,    57,   380,   481,   430,   208,
     291,   423,   424,   370,   371,   618,   477,   431,   209,   210,
     211,   212,   213,   214,   215,   229,   216,   217,   108,   158,
     744,   261,   262,    58,   279,   280,   520,   614,   615,   419,
     655,   656,   801
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     102,   103,   104,   263,   109,   106,   123,   124,   125,   297,
     298,   425,   422,   332,   665,   207,   637,   276,   638,   639,
     131,   337,   133,   344,   483,   134,   302,   626,   303,   222,
     665,   223,   168,   586,   493,   170,   741,   317,   541,   388,
     587,   248,   588,   101,   446,   647,   494,   101,   163,   101,
     115,   101,   198,     5,   495,   559,   505,   101,   101,   145,
     175,   742,   494,   679,   406,   176,   101,   389,   555,   101,
     495,   331,   198,   101,   226,   556,   771,   187,   101,    79,
     218,   417,   220,   221,   544,   101,   680,   101,   506,   585,
     198,   269,   378,   356,   116,   560,     3,   813,   146,   361,
     191,   192,   193,   194,   122,   195,   218,   496,   227,   227,
     497,     6,   122,   296,   406,   416,    79,   425,   422,   105,
     105,   130,   829,   196,   732,   387,   318,   557,   105,   630,
     584,   447,   648,   708,   418,   105,   227,   197,   260,   260,
     105,   642,   442,   443,   605,   334,   335,   105,   752,   105,
     835,   589,   441,   260,   101,   611,   101,   198,   304,   198,
     338,   489,   284,   484,   499,   345,   807,   153,   612,     1,
     665,   665,    13,   611,   769,   503,   718,   512,   453,     4,
     150,   774,     8,   308,   777,   778,   612,   620,   399,   400,
      14,     9,   199,   784,   191,   192,   193,   194,   808,   195,
     151,   791,   813,   324,   613,   547,   154,   260,    18,   719,
     218,   218,   513,   200,   201,   202,   203,   196,   368,   399,
     400,   659,   613,   429,   807,   204,   205,   206,   475,   260,
      19,   197,   401,   402,   246,   260,   664,   518,   532,   403,
     836,   369,   837,   404,    10,   721,   620,   399,   400,   781,
     101,   476,   722,   198,   838,   826,   808,   550,   551,   184,
     519,   260,   620,   399,   400,   392,   747,   460,   405,   509,
     849,   850,   782,   748,   510,   185,   620,   399,   400,   529,
      11,    12,    13,   426,   421,   482,   199,   231,   218,   218,
     530,   232,   350,    28,    29,   674,   411,   412,   351,    30,
      14,    31,    32,    33,    34,    35,    20,   200,   201,   202,
     203,   792,    21,   765,   260,   465,   466,    23,   766,   204,
     205,   206,    36,    37,    38,   793,    28,    29,    39,   411,
     412,   528,    30,    25,    31,    32,    33,    34,    35,   640,
      59,    26,   608,   609,   594,    40,   595,   119,   120,   413,
     218,   399,   400,    27,   596,    36,    37,    38,    61,    60,
      41,    39,   652,   653,   401,   402,    62,    81,    78,    42,
     594,   403,   595,    63,   243,   404,   244,   486,    40,    64,
     596,   573,   487,    65,   488,   111,   112,   113,    66,   426,
     287,   288,   289,    41,    67,    68,   218,   620,   399,   400,
     405,    97,    42,   320,   536,   321,   425,   422,   347,    69,
     348,   353,   365,   354,   366,   397,   472,   398,   473,   479,
     617,   480,   580,   619,   581,   480,   661,   692,   662,   693,
     794,   819,   473,   820,    70,    71,    72,    73,    74,    83,
      75,   218,    76,   817,    77,    82,   218,    84,   729,    85,
      86,    87,    88,   646,    89,    90,    91,    92,   817,    93,
      95,    94,    96,    98,    28,    29,    99,   100,   101,    35,
      30,   726,    31,    32,    33,    34,    35,   607,   260,   260,
     118,   122,   126,   127,   756,   218,   128,   848,   132,   129,
     817,   135,   116,    36,    37,    38,   683,   627,   136,    39,
     750,   137,   138,   156,   139,   141,   157,   142,   143,   144,
     149,   159,   160,   697,   161,   166,    40,   643,   218,   162,
     165,   167,   171,   164,   169,   172,   173,   174,   177,   179,
     180,    41,   178,   182,   183,   145,   189,   188,   224,   225,
      42,   230,   234,   800,   667,   241,   219,   190,   671,   236,
     242,   245,   271,   238,   274,   235,   296,   237,   239,   305,
     314,   218,   240,   257,   249,   250,   322,   330,   251,   278,
     333,   252,   253,   254,   832,   694,   255,   376,   218,   256,
     258,   259,   264,   265,   290,   266,   824,   267,   283,   268,
     710,   270,   275,   273,   709,   282,   277,   281,   285,   342,
     292,   379,   395,   311,   286,   293,   299,   260,   300,   301,
     309,   382,   312,   730,   316,   508,   294,   295,   313,   319,
     310,   339,   385,   323,   326,   327,   396,   328,   446,   329,
     340,   336,   504,   511,   341,   524,   260,   535,   346,   357,
     359,   562,   549,   360,   358,   364,   367,   499,   372,   349,
     373,   563,   470,   381,   362,   374,   375,   377,   565,   393,
     390,   601,   410,   427,   629,   428,   668,   432,   394,   433,
     420,   391,   434,   436,   437,   435,   438,   439,   426,   440,
     444,   445,   449,   538,   450,   451,   474,   452,   454,   455,
     687,   456,   457,   458,   459,   648,   462,   463,   464,   553,
     468,   471,   469,   487,   478,   490,   491,   492,   557,   695,
     698,   525,   633,   502,   521,   818,   516,   583,   526,   517,
     527,   531,   260,   533,   522,   543,   534,   542,   634,   546,
     818,   545,   552,   700,   564,   561,   717,   566,   537,   568,
     569,   198,   571,   567,   839,   572,   736,   574,   575,   576,
     590,   699,   577,   578,   579,   737,   582,   591,   739,   599,
     485,   600,   818,   603,   610,   592,   604,   593,   606,   616,
     602,   624,   625,   631,   635,   632,   636,   650,   651,   654,
     657,   658,   663,   645,   703,   666,   669,   670,   672,   673,
     696,   773,   786,   675,   676,   677,   678,   684,   686,   369,
     688,   705,   689,   690,   691,   715,   701,   706,   702,   704,
     707,   711,   712,    22,   628,   713,   714,   716,   749,   723,
     724,   725,   727,   386,   731,   733,   734,   735,    80,   738,
     746,   751,   753,   548,   641,   757,   682,   745,   758,   761,
     759,   760,   754,   762,   755,   763,   764,   768,   110,   827,
     772,   767,   776,   780,   789,   785,   787,   770,   775,   788,
     830,   795,   790,   798,   796,   797,   799,   803,   802,   851,
     140,   811,   804,   852,   685,   812,   805,   821,   822,   823,
     806,   825,   828,   467,   833,   834,   831,   841,   842,   843,
     840,   847,   779,   853,   844,   854,   855,   845,   846,   856,
     857,   858,   728,   740,   570,   515,     0,     0,   660,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   523,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   363,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   461
};

static const yytype_int16 yycheck[] =
{
      84,    85,    86,   223,    88,    87,    94,    95,    96,   255,
     256,   367,   367,   292,   621,   164,   591,   237,   592,   593,
     104,    14,   106,   303,    14,   109,   261,    26,   263,   168,
     637,   170,   138,    21,   447,   141,   710,    56,   493,    47,
      28,   190,    30,    65,    56,    24,    79,    65,   132,    65,
      34,    65,    68,     0,    87,    37,    24,    65,    65,    82,
     148,    14,    79,   106,   357,   149,    65,   346,    29,    65,
      87,   291,    68,    65,    54,    36,   750,   159,    65,    43,
     164,    79,   166,   167,   497,    65,   129,    65,    56,   544,
      68,   230,   338,   313,    78,   510,   137,   113,   121,   319,
       9,    10,    11,    12,   126,    14,   190,   140,   116,   116,
     143,    58,   126,    66,   407,   361,    80,   473,   473,   127,
     127,   139,   138,    32,   698,   345,   145,   109,   127,   584,
     543,   143,   111,   147,   132,   127,   116,    46,   222,   223,
     127,   128,   388,   389,   559,   294,   295,   127,   722,   127,
     824,   139,   387,   237,    65,    90,    65,    68,   264,    68,
     153,   441,   244,   153,    97,   304,   107,    79,   103,    57,
     777,   778,    61,    90,   748,   454,    79,    44,   398,     0,
      79,   755,   138,   267,   759,   760,   103,    89,    90,    91,
      79,   137,   101,   767,     9,    10,    11,    12,   139,    14,
      99,   775,   113,   285,   139,   138,   118,   291,   138,   112,
     294,   295,    79,   122,   123,   124,   125,    32,    79,    90,
      91,   138,   139,   372,   107,   134,   135,   136,    56,   313,
     137,    46,   103,   104,   143,   319,   138,    56,   484,   110,
      36,   102,    38,   114,     0,   139,    89,    90,    91,    56,
      65,    79,   146,    68,    50,   138,   139,   503,   504,   138,
      79,   345,    89,    90,    91,   349,   139,   138,   139,    37,
     844,   845,    79,   146,    42,   154,    89,    90,    91,   138,
      59,    60,    61,   367,   366,   434,   101,    79,   372,   373,
     149,    83,    79,    63,    64,   138,   119,   120,    85,    69,
      79,    71,    72,    73,    74,    75,   137,   122,   123,   124,
     125,   138,   137,   139,   398,   138,   139,    62,   144,   134,
     135,   136,    92,    93,    94,   138,    63,    64,    98,   119,
     120,   480,    69,   138,    71,    72,    73,    74,    75,    24,
     138,    51,   562,   563,    29,   115,    31,    79,    80,   139,
     434,    90,    91,    51,    39,    92,    93,    94,   138,   137,
     130,    98,   608,   609,   103,   104,    18,   139,   138,   139,
      29,   110,    31,    19,   138,   114,   140,   139,   115,   137,
      39,   530,   144,   137,   146,    76,    77,    78,   137,   473,
       6,     7,     8,   130,   137,   137,   480,    89,    90,    91,
     139,   138,   139,   138,   488,   140,   762,   762,   138,   137,
     140,   144,   138,   146,   140,   138,   138,   140,   140,   138,
     569,   140,   138,   138,   140,   140,   138,   138,   140,   140,
     138,   138,   140,   140,   137,   137,   137,   137,   137,    35,
     137,   525,   137,   799,   137,   139,   530,    48,   694,    49,
      33,    25,    23,   602,    31,    34,    41,    50,   814,    37,
      27,    43,    52,   138,    63,    64,   138,    11,    65,    75,
      69,   691,    71,    72,    73,    74,    75,   561,   562,   563,
      75,   126,   138,   137,   730,   569,    56,   843,   137,    70,
     846,   138,    78,    92,    93,    94,   645,   579,   137,    98,
     720,   137,   137,   147,   138,   137,    66,   138,   137,   137,
     137,   137,   137,   662,   138,    22,   115,   599,   602,   139,
     139,    21,    34,   146,   138,   147,   137,   137,   137,   137,
     117,   130,   100,    56,   137,    82,   138,   144,    78,    56,
     139,    20,    15,   789,   626,    13,   138,   141,   632,   137,
      66,    56,    95,   137,    56,   146,    66,   146,   138,    81,
      56,   645,   137,   139,   137,   137,    13,    56,   137,   131,
      56,   137,   137,   137,   820,   657,   137,    13,   662,   137,
     137,   137,   137,   137,   147,   138,   806,   137,   139,   138,
     678,   137,   144,   138,   678,   137,   144,   138,   137,    84,
     137,    56,    56,   137,   142,   141,   138,   691,   138,   138,
     138,    88,   138,   695,   138,    24,   151,   150,   137,   137,
     147,   141,    88,   138,   138,   138,    96,   138,    56,   138,
     138,   137,    45,    22,   138,    56,   720,    56,   137,   137,
     137,    27,    56,   138,   145,   138,   137,    97,   138,   152,
     137,    43,   144,   137,   148,   138,   138,   138,    56,   137,
     148,    36,   138,   137,     4,   137,    13,   138,   148,   138,
     145,   348,   137,   137,   137,   142,   138,   137,   762,   138,
     138,   138,   138,    86,   137,   137,   147,   138,   137,   137,
      14,   137,   137,   137,   137,   111,   138,   137,   137,   105,
     138,   138,   137,   144,   138,   138,   138,   137,   109,    24,
      40,   137,    96,   144,   139,   799,   145,   140,   138,   146,
     142,   138,   806,   138,   148,   143,   138,   137,    96,   137,
     814,   138,   137,    13,   138,   146,    14,   137,   489,   138,
     137,    68,   138,   145,   828,   138,    56,   138,   138,   137,
     145,   145,   139,   138,   137,    56,   138,   137,    56,   137,
     436,   138,   846,   137,   137,   146,   138,   146,   138,   138,
     146,   138,   138,   138,   138,   146,   138,   138,   137,   133,
     137,   137,   137,   146,   144,   138,   138,   138,   137,   137,
     148,    56,    56,   139,   139,   138,   137,   139,   137,   102,
     138,   145,   139,   139,   137,   139,   138,   138,   137,   144,
     138,   137,   137,    15,   581,   138,   138,   138,   137,   139,
     138,   138,   138,   344,   138,   138,   138,   138,    60,   138,
     138,   138,   138,   500,   597,   139,   644,   712,   138,   138,
     137,   137,   147,   137,   146,   138,   138,   138,    89,   809,
     139,   146,   138,   138,   137,   139,   138,   147,   146,   138,
     814,   137,   139,   138,   145,   139,   137,   139,   138,   846,
     115,   138,   145,   139,   647,   138,   145,   138,   138,   138,
     148,   137,   137,   414,   138,   138,   148,   138,   138,   146,
     148,   138,   762,   139,   146,   139,   138,   146,   146,   138,
     138,   138,   693,   709,   525,   464,    -1,    -1,   614,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   473,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   321,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   407
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    57,   156,   137,     0,     0,    58,   157,   138,   137,
       0,    59,    60,    61,    79,   158,   159,   160,   138,   137,
     137,   137,   160,    62,   161,   138,    51,    51,    63,    64,
      69,    71,    72,    73,    74,    75,    92,    93,    94,    98,
     115,   130,   139,   162,   163,   164,   169,   171,   172,   173,
     174,   192,   193,   195,   196,   201,   224,   230,   258,   138,
     137,   138,    18,    19,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   138,   163,
     162,   139,   139,    35,    48,    49,    33,    25,    23,    31,
      34,    41,    50,    37,    43,    27,    52,   138,   138,   138,
      11,    65,   253,   253,   253,   127,   166,   167,   253,   253,
     174,    76,    77,    78,   175,    34,    78,   194,    75,    79,
      80,   176,   126,   257,   257,   257,   138,   137,    56,    70,
     139,   253,   137,   253,   253,   138,   137,   137,   137,   138,
     194,   137,   138,   137,   137,    82,   121,   179,   197,   137,
      79,    99,   202,    79,   118,   225,   147,    66,   254,   137,
     137,   138,   139,   253,   146,   139,    22,    21,   176,   138,
     176,    34,   147,   137,   137,   257,   253,   137,   100,   137,
     117,   226,    56,   137,   138,   154,   165,   166,   144,   138,
     141,     9,    10,    11,    12,    14,    32,    46,    68,   101,
     122,   123,   124,   125,   134,   135,   136,   233,   234,   243,
     244,   245,   246,   247,   248,   249,   251,   252,   253,   138,
     253,   253,   179,   179,    78,    56,    54,   116,   166,   250,
      20,    79,    83,   180,    15,   146,   137,   146,   137,   138,
     137,    13,    66,   138,   140,    56,   143,   168,   233,   137,
     137,   137,   137,   137,   137,   137,   137,   139,   137,   137,
     253,   256,   257,   256,   137,   137,   138,   137,   138,   179,
     137,    95,   198,   138,    56,   144,   256,   144,   131,   259,
     260,   138,   137,   139,   166,   137,   142,     6,     7,     8,
     147,   235,   137,   141,   151,   150,    66,   255,   255,   138,
     138,   138,   180,   180,   176,    81,   177,   178,   253,   138,
     147,   137,   138,   137,    56,   204,   138,    56,   145,   137,
     138,   140,    13,   138,   166,   170,   138,   138,   138,   138,
      56,   256,   250,    56,   233,   233,   137,    14,   153,   141,
     138,   138,    84,   181,   181,   179,   137,   138,   140,   152,
      79,    85,   182,   144,   146,   203,   256,   137,   145,   137,
     138,   256,   148,   260,   138,   138,   140,   137,    79,   102,
     238,   239,   138,   137,   138,   138,    13,   138,   255,    56,
     231,   137,    88,   187,   188,    88,   188,   256,    47,   250,
     148,   178,   253,   137,   148,    56,    96,   138,   140,    90,
      91,   103,   104,   110,   114,   139,   191,   205,   206,   207,
     138,   119,   120,   139,   227,   228,   255,    79,   132,   264,
     145,   166,   167,   236,   237,   243,   253,   137,   137,   233,
     233,   242,   138,   138,   137,   142,   137,   137,   138,   137,
     138,   180,   255,   255,   138,   138,    56,   143,   183,   138,
     137,   137,   138,   256,   137,   137,   137,   137,   137,   137,
     138,   206,   138,   137,   137,   138,   139,   228,   138,   137,
     144,   138,   138,   140,   147,    56,    79,   241,   138,   138,
     140,   232,   233,    14,   153,   182,   139,   144,   146,   181,
     138,   138,   137,   183,    79,    87,   140,   143,   186,    97,
     199,   200,   144,   250,    45,    24,    56,   208,    24,    37,
      42,    22,    44,    79,   229,   229,   145,   146,    56,    79,
     261,   139,   148,   237,    56,   137,   138,   142,   233,   138,
     149,   138,   255,   138,   138,    56,   253,   187,    86,   184,
     185,   186,   137,   143,   183,   138,   137,   138,   200,    56,
     255,   255,   137,   105,   211,    29,    36,   109,   218,    37,
     218,   146,    27,    43,   138,    56,   137,   145,   138,   137,
     242,   138,   138,   233,   138,   138,   137,   139,   138,   137,
     138,   140,   138,   140,   183,   186,    21,    28,    30,   139,
     145,   137,   146,   146,    29,    31,    39,   209,   210,   137,
     138,    36,   146,   137,   138,   218,   138,   253,   256,   256,
     137,    90,   103,   139,   262,   263,   138,   233,   240,   138,
      89,   189,   190,   191,   138,   138,    26,   166,   185,     4,
     186,   138,   146,    96,    96,   138,   138,   189,   231,   231,
      24,   210,   128,   166,   212,   146,   233,    24,   111,   219,
     138,   137,   255,   255,   133,   265,   266,   137,   137,   138,
     263,   138,   140,   137,   138,   190,   138,   166,    13,   138,
     138,   253,   137,   137,   138,   139,   139,   138,   137,   106,
     129,   213,   213,   233,   139,   219,   137,    14,   138,   139,
     139,   137,   138,   140,   166,    24,   148,   233,    40,   145,
      13,   138,   137,   144,   144,   145,   138,   138,   147,   253,
     257,   137,   137,   138,   138,   139,   138,    14,    79,   112,
     220,   139,   146,   139,   138,   138,   256,   138,   266,   255,
     166,   138,   231,   138,   138,   138,    56,    56,   138,    56,
     239,   238,    14,   214,   255,   214,   138,   139,   146,   137,
     256,   138,   231,   138,   147,   146,   255,   139,   138,   137,
     137,   138,   137,   138,   138,   139,   144,   146,   138,   231,
     147,   238,   139,    56,   231,   146,   138,   189,   189,   236,
     138,    56,    79,   215,   231,   139,    56,   138,   138,   137,
     139,   231,   138,   138,   138,   137,   145,   139,   138,   137,
     255,   267,   138,   139,   145,   145,   148,   107,   139,   216,
     217,   138,   138,   113,   221,   222,   223,   243,   253,   138,
     140,   138,   138,   138,   256,   137,   138,   217,   137,   138,
     222,   148,   255,   138,   138,   238,    36,    38,    50,   253,
     148,   138,   138,   146,   146,   146,   146,   138,   243,   231,
     231,   223,   139,   139,   139,   138,   138,   138,   138
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int16 yyr1[] =
{
       0,   155,   156,   156,   157,   157,   157,   158,   159,   160,
     160,   161,   162,   162,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   164,
     165,   165,   166,   166,   167,   168,   168,   169,   170,   170,
     171,   171,   172,   173,   174,   175,   175,   175,   176,   176,
     177,   177,   178,   178,   179,   179,   180,   180,   181,   182,
     182,   182,   182,   182,   183,   184,   184,   185,   185,   186,
     186,   187,   187,   187,   188,   189,   189,   190,   190,   191,
     191,   192,   193,   193,   194,   195,   196,   197,   197,   198,
     198,   199,   199,   200,   200,   200,   200,   201,   202,   202,
     203,   203,   204,   204,   205,   205,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   207,   208,   208,   209,   209,
     210,   210,   210,   211,   211,   212,   212,   212,   213,   213,
     213,   214,   214,   215,   215,   216,   216,   217,   217,   217,
     217,   218,   218,   218,   218,   219,   220,   220,   221,   221,
     222,   222,   223,   223,   224,   225,   225,   226,   226,   227,
     227,   227,   227,   228,   228,   229,   229,   230,   231,   232,
     232,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   234,   235,   235,   236,   236,
     237,   237,   237,   238,   238,   239,   240,   240,   241,   241,
     242,   242,   243,   243,   243,   244,   244,   245,   245,   246,
     247,   247,   248,   249,   250,   250,   251,   251,   252,   252,
     253,   254,   254,   255,   256,   256,   257,   258,   259,   259,
     260,   261,   261,   262,   262,   263,   263,   263,   264,   264,
     265,   265,   266,   267,   267
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     4,     5,     4,     5,     5,     6,     6,     4,
       1,     4,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,    10,
       1,     3,     1,     1,     7,     1,     1,    15,     1,     3,
       6,     7,     9,     7,     5,     7,     7,     9,     1,     9,
       1,     3,     5,     5,     4,     4,     1,     6,     6,     1,
       5,     6,     7,     8,     4,     1,     3,     5,     6,     1,
       5,     4,     6,     1,     9,     1,     2,     6,     1,     8,
       8,     5,     5,     6,     9,    17,     9,     1,     5,    13,
       9,     1,     2,     4,    13,    13,     8,    12,     1,     8,
       1,     3,     0,     4,     1,     2,     1,     1,     8,     9,
       5,     5,     6,     9,     1,     5,     1,     5,     1,     2,
       1,     1,     1,     5,     5,    11,     5,     5,     7,     7,
       5,     1,     1,     1,     4,     1,     2,     1,     7,     7,
       7,     6,     7,     8,     9,     6,     1,     9,     1,     2,
       6,     1,     1,     1,     7,     1,     5,     9,     5,     1,
       1,     2,     2,     8,     8,     1,     1,     5,     4,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     5,     1,     1,     1,     9,     0,     6,     1,     3,
       1,     1,     1,     1,     1,     9,     1,     3,     1,     4,
       1,     3,     4,     4,     4,     1,     1,     1,     1,     9,
       1,     1,     5,     7,     1,     6,     6,     5,     9,     8,
       4,     4,     6,     4,     1,     1,     5,    15,     1,     3,
       5,     1,     4,     1,     2,     8,     9,     1,     1,     8,
       1,     3,    10,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        yyerror (ctx, p_ast, p_builder, p_sb, YY_("syntax error: cannot back up")); \
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
                  Type, Value, ctx, p_ast, p_builder, p_sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (ctx);
  YYUSE (p_ast);
  YYUSE (p_builder);
  YYUSE (p_sb);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep, ctx, p_ast, p_builder, p_sb);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              , ctx, p_ast, p_builder, p_sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, ctx, p_ast, p_builder, p_sb); \
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
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
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
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
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
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

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
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
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
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
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
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
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
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  YYUSE (yyvaluep);
  YYUSE (ctx);
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
yyparse (ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
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

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

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
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
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
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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
                                                    { p_ast = AST :: Make ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 3:
                                                    { p_ast = (yyvsp[-2].node); }
    break;

  case 4:
                                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 5:
                                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[-2].tok) ); }
    break;

  case 6:
                                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[-2].tok) ); }
    break;

  case 7:
                                                        { (yyval.tok) = (yyvsp[-5].tok); }
    break;

  case 8:
                                                       { (yyval.tok) = (yyvsp[-5].tok); }
    break;

  case 9:
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-1].node) ); }
    break;

  case 10:
                                                    { (yyval.node) = AST :: Make ( ctx, PT_SCHEMA_1_0 ); }
    break;

  case 11:
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-1].node) ); }
    break;

  case 12:
                                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 13:
                                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
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
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 28:
                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 29:
                                            { (yyval.node) = p_builder . TypeDef ( ctx, (yyvsp[-9].tok), (yyvsp[-6].fqn), (yyvsp[-3].node) ); }
    break;

  case 30:
                                       { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 31:
                                       { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 32:
                { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 33:
                { (yyval.node) = (yyvsp[0].node); }
    break;

  case 34:
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
    break;

  case 35:
             { (yyval.node) = (yyvsp[0].expr); }
    break;

  case 36:
             { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 37:
                { (yyval.node) = p_builder . TypeSet ( ctx, (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-5].node) ); }
    break;

  case 38:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 39:
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 40:
                                                { (yyval.node) = p_builder . FmtDef ( ctx, (yyvsp[-5].tok), (yyvsp[-2].fqn), 0 ); }
    break;

  case 41:
                                                { (yyval.node) = p_builder . FmtDef ( ctx, (yyvsp[-6].tok), (yyvsp[-2].fqn), (yyvsp[-3].fqn) ); }
    break;

  case 42:
                                                                 { (yyval.node) = p_builder . ConstDef ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 43:
                                             { (yyval.node) = p_builder . AliasDef ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
    break;

  case 44:
                                                { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 45:
                                                       { (yyval.node) = p_builder . UntypedFunctionDecl ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
    break;

  case 46:
                                                       { (yyval.node) = p_builder . RowlenFunctionDecl ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
    break;

  case 47:
                { (yyval.node) = p_builder . FunctionDecl ( ctx, (yyvsp[-8].tok), false, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 48:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 49:
                                                                        { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 50:
                                        { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 51:
                                        { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 52:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-1].fqn) ); }
    break;

  case 53:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].fqn) ); }
    break;

  case 54:
                                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 55:
                                        { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 56:
                                                 { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 57:
                                                 { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 58:
                                                  { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 59:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 60:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), AST :: Make ( ctx ), (yyvsp[-1].node) ); }
    break;

  case 61:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), AST :: Make ( ctx ), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 62:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 63:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 64:
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 65:
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 66:
                            { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 67:
                                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), AST :: Make ( ctx, (yyvsp[-1].tok) ), AST :: Make ( ctx ) ); }
    break;

  case 68:
                                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-2].node), AST :: Make ( ctx, (yyvsp[-1].tok) ), AST :: Make ( ctx, (yyvsp[-3].tok) ) ); }
    break;

  case 69:
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 70:
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok) ); }
    break;

  case 71:
                                            { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 72:
                                            { (yyval.node) = (yyvsp[-2].fqn); }
    break;

  case 74:
                                                                        { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 75:
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 76:
                                { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 77:
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-2].expr) ); }
    break;

  case 78:
                                                    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 79:
                                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 80:
                                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 81:
                                              { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 82:
                                                          { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 83:
                                                          { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 84:
                { (yyval.node) = p_builder . FunctionDecl ( ctx, (yyvsp[-8].tok), true, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 85:
        { (yyval.node) = p_builder . FunctionDecl ( ctx, (yyvsp[-16].tok), false, (yyvsp[-8].node), (yyvsp[-7].node), (yyvsp[-6].fqn), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].node) ); }
    break;

  case 86:
            { (yyval.node) = p_builder . PhysicalDecl ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 87:
                                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 88:
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-1].node) ); }
    break;

  case 89:
                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-9].tok), (yyvsp[-4].node) ); }
    break;

  case 90:
                { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 91:
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 92:
                                { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 93:
        { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 94:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-10].tok), (yyvsp[-4].node) ) ; }
    break;

  case 95:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-10].tok), (yyvsp[-4].node) ); }
    break;

  case 96:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].fqn) ); }
    break;

  case 97:
                { (yyval.node) = p_builder . TableDef ( ctx, (yyvsp[-11].tok), (yyvsp[-8].fqn), (yyvsp[-7].node), (yyvsp[-3].node) ); }
    break;

  case 98:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 99:
                                                                    { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 100:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 101:
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 102:
                                    { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 103:
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 104:
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 105:
                            { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 106:
                                                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 107:
                                                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 108:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-2].expr) ); }
    break;

  case 109:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-2].expr) ); }
    break;

  case 110:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-2].tok), (yyvsp[-1].node) ); }
    break;

  case 111:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-2].tok), (yyvsp[-1].node) ); }
    break;

  case 112:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-1].node) ); }
    break;

  case 113:
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-4].fqn) ); }
    break;

  case 114:
                                                                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 115:
                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 116:
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 117:
                                                    { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 118:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 119:
                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 120:
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 121:
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 122:
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 123:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 124:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 125:
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-10].tok), (yyvsp[-5].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 126:
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 127:
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 128:
                                                            {  (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node) ); }
    break;

  case 129:
                                                            {  (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
    break;

  case 130:
                                                            {  (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node) ); }
    break;

  case 131:
                                { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 132:
                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 133:
                                    { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 134:
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 135:
                        { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 136:
                        { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 137:
                                                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 138:
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 139:
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 140:
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 141:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].node), AST :: Make ( ctx, (yyvsp[-2].tok) ) ); }
    break;

  case 142:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-3].node), AST :: Make ( ctx, (yyvsp[-2].tok) ) ); }
    break;

  case 143:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 144:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 145:
                                                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 146:
                                                                            { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 147:
                                                                            { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 148:
                                        { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 149:
                                        { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 150:
                                                            { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].node) ); }
    break;

  case 151:
                                                            { (yyval.node) = (yyvsp[0].node); }
    break;

  case 152:
                { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 153:
                { (yyval.node) = (yyvsp[0].expr); }
    break;

  case 154:
                                                                { (yyval.node) = p_builder . DatabaseDef ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 155:
                                           { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 156:
                                           { (yyval.node) = (yyvsp[-1].fqn); }
    break;

  case 157:
                                                                { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 158:
                                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok) ); }
    break;

  case 159:
                              { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 160:
                                { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 161:
                              { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 162:
                                { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 163:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
    break;

  case 164:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
    break;

  case 165:
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 166:
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 167:
                                           { (yyval.node) = p_builder . Include ( ctx, (yyvsp[-2].tok), (yyvsp[-1].tok) ); }
    break;

  case 168:
                                    { (yyval.expr) = (yyvsp[-1].expr); }
    break;

  case 169:
                             { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].expr) ); }
    break;

  case 170:
                             { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 171:
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 172:
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 173:
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
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
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 181:
                                    { (yyval.expr) = (yyvsp[-1].expr); }
    break;

  case 182:
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 183:
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 184:
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 185:
        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-5].fqn) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-4].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-2].node) ); }
    break;

  case 186:
                                                { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 187:
                                                { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 188:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 189:
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 190:
                    { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 191:
                    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 192:
                    { (yyval.node) = (yyvsp[0].expr); }
    break;

  case 193:
                            { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 194:
                            { (yyval.node) = (yyvsp[0].node); }
    break;

  case 195:
                                                                        { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 196:
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 197:
                                { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 198:
                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 199:
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 200:
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 201:
                            { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 202:
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 203:
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 204:
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 205:
                 { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 206:
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 207:
                        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 208:
                        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 209:
                                                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].node) ); }
    break;

  case 210:
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 211:
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 212:
                                 { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-4].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].expr) ); }
    break;

  case 213:
                                                  { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-6].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].expr) ); }
    break;

  case 214:
                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 215:
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }
    break;

  case 216:
                                                            { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-5].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].fqn) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].fqn) ); }
    break;

  case 217:
        {   /* remove leading '.'*/
            (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-4].tok) );
            (yyval.expr) -> AddNode ( ctx, (yyvsp[-2].fqn) );
            AST * ident = AST :: Make ( ctx, PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( ctx, & t );
            (yyval.expr) -> AddNode ( ctx, ident );
        }
    break;

  case 218:
        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-6].fqn) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-4].expr) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].fqn) ); }
    break;

  case 219:
        {   /* remove leading '.'*/
            (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-7].tok) );
            (yyval.expr) -> AddNode ( ctx, (yyvsp[-5].fqn) );
            (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].expr) );
            AST * ident = AST :: Make ( ctx, PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( ctx, & t );
            (yyval.expr) -> AddNode ( ctx, ident );
        }
    break;

  case 220:
                                { (yyval.fqn) = (yyvsp[-1].fqn); }
    break;

  case 221:
                                                        { (yyval.fqn) = AST_FQN :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 222:
                                                        { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 223:
                                        { (yyval.fqn) = AST_FQN :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 224:
                { (yyval.fqn) = (yyvsp[0].fqn); }
    break;

  case 225:
                { (yyval.fqn) = (yyvsp[0].fqn); }
    break;

  case 226:
                                        { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }
    break;

  case 227:
        { (yyval.node) = p_builder . ViewDef ( ctx, (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-2].node) ); }
    break;

  case 228:
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 229:
                                { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 230:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].fqn) ); }
    break;

  case 231:
                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 232:
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 233:
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 234:
                            { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 235:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 236:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 237:
                                                                    { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 238:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 239:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-2].node) ); }
    break;

  case 240:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 241:
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 242:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-9].tok), (yyvsp[-7].fqn), (yyvsp[-3].node) ); }
    break;

  case 243:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 244:
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
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
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

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
      yyerror (ctx, p_ast, p_builder, p_sb, YY_("syntax error"));
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
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
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
        yyerror (ctx, p_ast, p_builder, p_sb, yymsgp);
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
                      yytoken, &yylval, ctx, p_ast, p_builder, p_sb);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

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
                  yystos[yystate], yyvsp, ctx, p_ast, p_builder, p_sb);
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
  yyerror (ctx, p_ast, p_builder, p_sb, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, ctx, p_ast, p_builder, p_sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp, ctx, p_ast, p_builder, p_sb);
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
