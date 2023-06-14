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

/* Substitute the type names.  */
#define YYSTYPE         AST_STYPE
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
#ifndef AST_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define AST_DEBUG 1
#  else
#   define AST_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define AST_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined AST_DEBUG */
#if AST_DEBUG
extern int AST_debug;
#endif

/* Token type.  */
#ifndef AST_TOKENTYPE
# define AST_TOKENTYPE
  enum AST_tokentype
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
    PT_JOINEXPR = 390,
    PT_ALIASMEMBER = 391,
    PT_VIEWSPEC = 392
  };
#endif

/* Value type.  */
#if ! defined AST_STYPE && ! defined AST_STYPE_IS_DECLARED
union AST_STYPE
{

  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;


};
typedef union AST_STYPE AST_STYPE;
# define AST_STYPE_IS_TRIVIAL 1
# define AST_STYPE_IS_DECLARED 1
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
         || (defined AST_STYPE_IS_TRIVIAL && AST_STYPE_IS_TRIVIAL)))

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
#define YYLAST   1204

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  157
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  114
/* YYNRULES -- Number of rules.  */
#define YYNRULES  248
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  878

#define YYUNDEFTOK  2
#define YYMAXUTOK   392


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
     139,   140,   145,   152,   142,   153,   155,   154,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   156,   141,
     149,   148,   150,     2,   138,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   143,     2,   144,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   146,   151,   147,     2,     2,     2,     2,
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
     135,   136,   137
};

#if AST_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   265,   265,   266,   270,   271,   272,   276,   280,   284,
     285,   289,   294,   295,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   319,
     324,   325,   329,   330,   334,   338,   339,   343,   348,   349,
     353,   354,   358,   362,   366,   370,   371,   372,   377,   378,
     382,   383,   387,   388,   392,   393,   397,   398,   402,   406,
     407,   408,   409,   410,   414,   418,   419,   423,   424,   428,
     429,   433,   434,   435,   439,   443,   444,   448,   449,   453,
     454,   458,   461,   462,   466,   471,   480,   485,   486,   489,
     491,   496,   497,   501,   503,   505,   507,   514,   519,   520,
     524,   525,   529,   530,   534,   535,   539,   540,   541,   542,
     543,   544,   545,   546,   547,   551,   555,   556,   560,   561,
     565,   566,   567,   571,   572,   576,   578,   579,   583,   584,
     585,   589,   590,   594,   595,   599,   600,   604,   605,   606,
     607,   611,   613,   615,   617,   622,   626,   627,   631,   632,
     636,   637,   641,   642,   647,   651,   652,   655,   656,   660,
     661,   662,   663,   667,   669,   671,   676,   677,   681,   686,
     687,   693,   699,   703,   704,   708,   709,   710,   711,   712,
     713,   714,   715,   716,   717,   718,   719,   720,   721,   725,
     730,   731,   734,   735,   739,   740,   741,   745,   746,   750,
     754,   755,   759,   760,   764,   765,   769,   770,   771,   775,
     776,   780,   781,   785,   789,   790,   794,   797,   801,   802,
     806,   807,   819,   821,   836,   840,   841,   845,   849,   850,
     854,   859,   865,   866,   870,   874,   875,   879,   880,   884,
     885,   886,   890,   891,   895,   896,   900,   905,   906
};
#endif

#if AST_DEBUG || YYERROR_VERBOSE || 1
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
  "PT_VIEWPARENTS", "PT_VIEWPARENT", "PT_MEMBEREXPR", "PT_JOINEXPR",
  "PT_ALIASMEMBER", "PT_VIEWSPEC", "'@'", "'('", "')'", "';'", "','",
  "'['", "']'", "'*'", "'{'", "'}'", "'='", "'<'", "'>'", "'|'", "'+'",
  "'-'", "'/'", "'.'", "':'", "$accept", "parse", "source", "version_1",
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
  "col_schema_parm", "col_schema_value", "database", "dbdad_opt", "dbbody",
  "db_members", "db_member", "template_opt", "view_spec", "view_parms",
  "include", "cond_expr", "cond_chain", "expr", "func_expr",
  "schema_parts_opt", "schema_parms", "schema_parm", "factory_parms_opt",
  "factory_parms_list", "factory_parms", "func_parms_opt", "expr_list",
  "uint_expr", "float_expr", "string_expr", "const_vect_expr", "bool_expr",
  "negate_expr", "cast_expr", "type_expr", "member_expr", "join_expr",
  "fqn", "qualnames", "ident", "fqn_opt_vers", "fqn_vers", "view",
  "view_parm", "view_body_opt", "view_body", "view_member",
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
     385,   386,   387,   388,   389,   390,   391,   392,    64,    40,
      41,    59,    44,    91,    93,    42,   123,   125,    61,    60,
      62,   124,    43,    45,    47,    46,    58
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
      18,   -54,   149,    84,  -675,    12,   126,   250,  -675,   238,
     182,   246,   251,   274,  -675,    15,   317,   196,  -675,   347,
     359,   396,   282,   291,   292,  -675,   413,   414,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   308,   310,
     311,   313,  -675,   240,  -675,  -675,  -675,  -675,  -675,  -675,
    -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
     396,  -675,   314,   316,   418,   406,   409,   428,   437,   441,
     435,   439,   433,   425,   440,   436,   449,   426,  -675,  -675,
     294,   345,   346,   476,   430,   430,   430,     7,   430,   416,
     270,   117,   421,   327,   371,   371,   371,  -675,  -675,  -675,
     361,   360,   447,   438,   -12,   367,   430,  -675,  -675,   430,
     370,   368,   374,   375,   376,   431,   378,   379,   381,  -675,
     382,    20,   383,   161,   101,   366,  -675,   452,   384,   389,
     390,   388,   430,   386,   391,  -675,   509,   514,   327,  -675,
     398,   327,  -675,   502,   392,   400,   401,  -675,   371,   430,
    -675,   403,   443,  -675,   405,   429,   489,   408,   111,     7,
     402,  -675,   410,   411,    32,   412,   430,   430,   467,  -675,
     467,   473,   499,   -15,   536,   277,   543,   415,   420,   417,
     422,   424,   427,   547,  -675,   501,   148,  -675,   512,  -675,
     206,  -675,  -675,  -675,  -675,  -675,  -675,  -675,   432,   434,
     442,   444,   445,   446,   448,   451,  -675,   450,  -675,  -675,
    -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,  -675,
     453,   454,    49,    49,   455,   457,   458,   460,  -675,   462,
     467,  -675,   461,   474,   463,   516,   464,    49,   465,  -675,
       1,   466,   468,   456,     7,   469,  -675,   470,  -675,   369,
     472,   477,   479,   423,   471,   508,   508,   475,   484,   485,
    -675,   277,  -675,   277,   327,   494,  -675,   430,  -675,   487,
     480,   478,   488,  -675,   491,   521,   492,   -29,   495,   496,
     197,  -675,  -675,  -675,   565,   493,  -675,     7,   497,   498,
     500,   503,   523,    49,    63,   524,    32,    32,     5,   504,
    -675,   505,   506,   520,   520,   467,   511,   249,  -675,   459,
    -675,   239,   173,  -675,    49,   513,   507,  -675,   517,   515,
     569,    49,   481,     1,   518,  -675,  -675,   254,  -675,  -675,
    -675,  -675,   522,   152,   519,   525,   526,   527,   528,   508,
     532,  -675,  -675,   530,   531,   548,    49,    21,   510,   494,
     430,  -675,   533,   529,   553,   545,   255,  -675,   179,   534,
     223,  -675,   535,   508,   -23,  -675,  -675,  -675,   537,     7,
      -6,  -675,   538,   539,  -675,    32,    32,  -675,  -675,  -675,
     540,   542,   541,   544,   549,   546,  -675,   550,   551,   277,
     508,   508,   552,  -675,   554,   -21,   555,   557,   558,   559,
      49,   561,   562,   563,   564,   566,   567,  -675,  -675,   132,
    -675,  -675,   568,   570,   571,   572,  -675,   165,  -675,  -675,
     573,  -675,   575,   574,   576,  -675,  -675,   259,  -675,  -675,
    -675,   490,    17,   577,  -675,   262,  -675,    32,     9,   239,
     204,  -675,   578,  -675,   520,   579,   581,  -675,  -675,   583,
     556,    26,  -675,   560,   580,  -675,  -675,    63,   597,   203,
     620,   312,   626,  -675,  -675,  -675,   142,   142,   628,   582,
    -675,  -675,  -675,   585,   177,   584,   586,    -6,   606,   588,
    -675,   590,  -675,   587,    32,   -59,  -675,   594,   508,   595,
     598,   607,   430,   531,  -675,  -675,   596,   168,  -675,   589,
     592,   556,   599,   602,   -36,  -675,   609,   508,   508,  -675,
     603,   593,   160,   614,    14,   601,  -675,  -675,   643,   610,
     608,   604,   615,   611,  -675,   600,   612,  -675,  -675,   616,
      32,   613,   617,  -675,  -675,    32,  -675,   618,   619,  -675,
     621,   605,   622,   625,   263,  -675,   627,   623,   556,   168,
    -675,    24,   624,  -675,   629,   630,   631,   296,   633,   634,
     637,   632,   636,   642,   614,   644,   430,    49,    49,   638,
     508,  -675,   646,    98,   647,  -675,    32,   272,  -675,  -675,
    -675,  -675,  -675,   293,   648,   649,    -2,  -675,   596,  -675,
     672,   168,   650,   635,   591,   655,   651,   652,   293,   532,
     532,  -675,  -675,  -675,   271,  -675,    45,  -675,   645,    32,
      16,  -675,   654,  -675,   656,   508,   508,    49,   657,   640,
     658,   660,  -675,    88,  -675,  -675,  -675,   278,  -675,   661,
     -20,  -675,  -675,  -675,   662,     7,   677,  -675,   663,   664,
    -675,   430,   666,   667,  -675,  -675,   109,   668,   669,   671,
    -675,   673,   128,   128,    32,   674,   659,   675,   690,  -675,
     676,   678,   679,   680,   681,   683,   279,  -675,     7,   688,
    -675,  -675,   682,    32,   703,   639,  -675,  -675,   694,   684,
    -675,  -675,   686,   685,   687,   670,   695,   696,  -675,   -27,
     689,   691,   697,   698,   693,   699,   701,   131,   100,   700,
     702,   704,   705,  -675,    49,   706,   640,   508,     7,   707,
    -675,   532,   708,   709,  -675,   710,   713,   720,   711,  -675,
    -675,   740,   716,   152,    77,    77,  -675,  -675,   712,  -675,
     120,  -675,   714,    49,   715,   532,   717,  -675,  -675,   719,
     718,  -675,  -675,   653,   508,  -675,   721,  -675,  -675,   723,
     722,   725,   726,   729,   730,   731,  -675,   232,  -675,   665,
    -675,   732,   532,   724,   152,  -675,   728,  -675,     1,   751,
     532,   692,   734,  -675,   293,   293,  -675,    -6,  -675,  -675,
     735,   202,   532,  -675,   736,   752,   738,   739,   283,   737,
     741,   532,  -675,   113,   135,   286,  -675,   742,  -675,   733,
     743,   745,   744,  -675,  -675,   746,   508,   747,   748,   750,
     753,   749,   104,   754,   755,  -675,   119,   758,  -675,   287,
    -675,   761,   762,   763,    49,   765,  -675,    67,  -675,  -675,
    -675,   766,   -10,  -675,  -675,  -675,  -675,  -675,   756,   508,
    -675,   767,   768,   152,   258,  -675,  -675,   430,   759,  -675,
     770,  -675,  -675,  -675,   771,   764,   769,   772,   773,   774,
    -675,  -675,   775,   532,   532,   212,  -675,   777,   778,   781,
     776,   783,   784,   785,  -675,  -675,  -675,  -675
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
       0,     0,     0,     0,     0,     0,   171,     0,     0,     0,
       0,     0,     0,     0,     0,    81,     0,     0,     0,    44,
       0,     0,    82,     0,     0,     0,     0,    87,     0,     0,
      98,     0,     0,   155,     0,     0,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,    30,     0,    41,
       0,   209,   210,   211,   212,   176,   215,   214,     0,     0,
       0,     0,     0,     0,     0,     0,   177,     0,   178,   179,
     180,   181,   182,   183,   184,   186,   187,   188,   175,    43,
       0,     0,     0,     0,     0,     0,     0,     0,   218,     0,
       0,    56,     0,     0,     0,     0,     0,     0,     0,   154,
       0,     0,     0,     0,     0,     0,    36,     0,    35,     0,
     190,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     228,     0,   229,     0,     0,     0,    54,     0,    55,     0,
       0,     0,     0,   230,     0,   102,     0,     0,     0,     0,
       0,   169,   232,   225,     0,     0,    31,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      42,     0,     0,     0,     0,     0,     0,     0,    50,     0,
      88,     0,     0,    86,     0,     0,     0,   156,     0,     0,
       0,     0,     0,     0,     0,    29,    38,     0,    34,   206,
     208,   207,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    45,    46,     0,     0,     0,     0,     0,     0,     0,
       0,    59,     0,     0,     0,     0,     0,   100,     0,     0,
       0,   158,     0,     0,     0,   170,   233,   226,     0,     0,
       0,   197,     0,     0,   198,     0,     0,   216,   185,   221,
       0,     0,     0,     0,     0,     0,    73,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   114,   106,     0,
     104,   107,     0,     0,     0,     0,   160,     0,   159,   227,
       0,   242,     0,     0,     0,    39,   195,     0,   192,   196,
     194,     0,     0,     0,   204,     0,   220,     0,     0,     0,
       0,    47,     0,    84,     0,     0,     0,    49,   219,     0,
       0,     0,    57,     0,     0,    99,   101,     0,     0,     0,
       0,     0,     0,   103,   105,    97,     0,     0,     0,     0,
     162,   161,   234,     0,     0,     0,     0,     0,     0,     0,
     202,     0,   217,     0,     0,     0,   173,     0,     0,     0,
       0,     0,     0,     0,    52,    53,     0,     0,    69,     0,
       0,     0,     0,     0,     0,    91,     0,     0,     0,   116,
       0,     0,     0,     0,     0,     0,   167,   166,     0,     0,
       0,     0,     0,     0,   235,     0,     0,   191,   193,     0,
       0,     0,     0,   205,   172,     0,   223,     0,     0,    71,
       0,     0,     0,     0,     0,    65,     0,     0,     0,     0,
      60,     0,     0,    92,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   157,     0,     0,     0,    37,     0,     0,   189,   213,
     174,   222,    58,     0,     0,     0,     0,    64,     0,    61,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   120,   121,   122,     0,   118,     0,   115,     0,     0,
       0,   111,     0,   110,     0,     0,     0,     0,     0,     0,
       0,     0,   241,     0,   237,   231,   200,     0,   203,     0,
       0,    75,    78,    72,     0,     0,     0,    66,     0,     0,
      62,     0,     0,     0,    93,    90,     0,     0,     0,     0,
     119,     0,     0,     0,     0,     0,     0,     0,     0,   112,
       0,     0,     0,     0,     0,     0,     0,   244,     0,     0,
     236,   238,     0,     0,     0,     0,    76,    85,     0,     0,
      70,    63,     0,     0,     0,     0,     0,     0,   117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   165,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,    67,     0,     0,     0,     0,    79,
      80,     0,     0,     0,     0,     0,   123,   124,     0,   108,
       0,   146,     0,     0,     0,     0,     0,   163,   164,     0,
       0,   243,   245,     0,     0,   199,     0,    74,    68,     0,
       0,     0,     0,     0,     0,     0,   132,     0,   131,     0,
     109,     0,     0,     0,     0,   141,     0,   113,     0,     0,
       0,     0,     0,    96,     0,     0,    89,     0,   127,   126,
       0,     0,     0,   142,     0,     0,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,   130,     0,   133,     0,
       0,     0,     0,   145,   143,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   144,     0,     0,   247,     0,
     239,     0,     0,     0,     0,     0,   137,     0,   135,   128,
     129,     0,     0,   148,   151,   153,   152,   168,     0,     0,
     240,     0,     0,     0,     0,   134,   136,     0,     0,   149,
       0,   248,    94,    95,     0,     0,     0,     0,     0,     0,
     246,   125,     0,     0,     0,     0,   147,     0,     0,     0,
       0,     0,     0,     0,   150,   140,   138,   139
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -675,  -675,  -675,  -675,  -675,   808,  -675,   794,    31,  -675,
    -675,   -82,  -361,  -675,  -675,  -675,  -675,  -675,  -675,   797,
    -675,  -105,  -675,   407,  -142,  -231,  -283,   315,  -403,  -675,
     130,  -460,   273,   482,  -582,  -612,  -265,  -675,  -675,   798,
    -675,  -675,  -675,  -675,  -675,   322,  -675,  -675,  -675,  -675,
    -675,   483,  -675,  -675,  -675,   241,  -675,  -675,   207,   134,
    -675,  -675,    29,  -457,   209,  -675,  -675,    56,    25,  -675,
    -675,  -675,  -675,   641,   486,  -675,   123,  -675,  -585,  -675,
    -161,  -675,  -675,   116,   727,  -674,   193,  -675,  -675,   397,
    -360,  -675,  -675,  -675,  -675,  -675,  -675,  -281,  -675,  -675,
     -84,  -675,  -244,  -206,   -88,  -675,   757,  -675,  -675,   307,
    -675,  -675,   220,  -675
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    15,    16,    17,    24,    43,    44,    45,
     186,   228,   107,   247,    46,   327,    47,    48,    49,    50,
     114,   121,   307,   308,   147,   233,   344,   353,   451,   544,
     545,   502,   385,   386,   630,   631,   632,    51,    52,   117,
      53,    54,   148,   272,   504,   505,    55,   152,   356,   316,
     409,   410,   411,   511,   604,   605,   559,   653,   692,   757,
     799,   827,   828,   563,   658,   733,   832,   833,   834,    56,
     155,   181,   417,   418,   518,   570,   280,    57,   382,   485,
     434,   208,   293,   427,   428,   373,   374,   627,   481,   435,
     209,   210,   211,   212,   213,   214,   215,   229,   216,   217,
     108,   158,   281,   261,   262,    58,   282,   525,   623,   624,
     423,   666,   667,   819
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     102,   103,   104,   207,   109,   106,   123,   124,   125,   426,
     429,   298,   299,   334,   647,   648,   646,   263,   676,   338,
     131,   345,   133,   487,   635,   134,   222,   318,   223,   248,
     303,   276,   304,   168,   676,   449,   170,   546,   101,   226,
     656,   191,   192,   193,   194,   593,   195,   497,   163,   755,
     101,   564,   594,   101,   595,   101,   421,   565,   198,   101,
     175,   503,   198,   101,   196,   176,   391,   278,   390,   629,
     401,   402,   101,   479,    79,     1,    13,   187,   197,   365,
     218,   534,   220,   221,     5,     3,   101,   333,   269,   592,
     786,   756,   535,   408,    14,   380,   480,   101,   549,   122,
     198,   227,   145,   831,   552,   498,   218,   612,   357,   422,
     101,    79,   105,   499,   101,   363,   426,   429,   319,   420,
     675,   105,   721,   562,   450,   105,   746,   657,   101,   130,
     848,   639,   279,   199,   105,   336,   337,   227,   260,   260,
     389,   146,     6,   278,   408,   591,   445,   446,   105,     4,
     766,   115,     8,   260,   200,   201,   202,   203,   444,   305,
     339,   493,   286,   346,   488,   596,   204,   205,   500,   854,
     206,   501,   105,   651,   825,   122,   507,   784,   620,   227,
     153,   676,   676,   309,   101,   790,   516,   198,   620,   560,
     105,   621,   793,   794,   456,   116,   561,   800,   629,   401,
     402,   621,   629,   401,   402,   326,   808,   845,   826,   260,
     731,   825,   218,   218,   433,   191,   192,   193,   194,   154,
     195,   517,   401,   402,   629,   401,   402,   509,   670,   622,
     260,   371,   831,   523,   690,   403,   404,   260,   196,   622,
     150,   734,   405,   732,   537,   826,   406,   498,   735,   685,
      10,   184,   197,   809,   372,   499,   524,   691,   797,   510,
     151,   761,   260,   555,   556,     9,   394,   185,   762,   401,
     402,   101,   463,   407,   198,   810,   486,   101,   868,   869,
     198,   798,   403,   404,   413,   414,   430,   425,   243,   405,
     244,   218,   218,   406,   855,   649,   856,    11,    12,    13,
     601,   415,   602,    28,    29,   469,   470,   199,   857,    30,
     603,    31,    32,    33,    34,    35,   260,    14,   351,   354,
     407,   355,    18,   533,   352,   601,   618,   602,   200,   201,
     202,   203,    36,    37,    38,   603,    25,   322,    39,   323,
     204,   205,   413,   414,   206,   490,   111,   112,   113,   513,
     491,   246,   492,   218,   514,    40,   231,    28,    29,   415,
     232,   615,   616,    30,   416,    31,    32,    33,    34,    35,
      41,   661,   662,   780,   580,   289,   290,   291,   781,    23,
      78,    42,   629,   401,   402,    19,    36,    37,    38,   348,
      20,   349,    39,   430,   368,   399,   369,   400,    26,   476,
     218,   477,   483,   587,   484,   588,   119,   120,   541,    40,
      27,   663,   628,    21,   484,   626,   426,   429,   672,   705,
     673,   706,    59,   805,    41,   323,   811,   838,   477,   839,
      60,    62,    61,    63,    97,    42,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,   218,    74,   655,    75,
      76,   218,    77,    83,    84,    81,   835,    82,    85,    28,
      29,    86,    87,   743,    88,    30,    89,    31,    32,    33,
      34,    35,   835,    90,    91,    92,    95,    93,    96,    94,
     758,   758,   614,   260,   260,    98,    99,   100,    36,    37,
      38,    35,   218,   694,    39,   101,   118,   122,   740,   127,
     771,   126,   867,   128,   636,   835,   132,   136,   129,   116,
     135,    40,   710,   137,   138,   156,   139,   141,   157,   142,
     143,   144,   149,   159,   652,   218,    41,   764,   160,   162,
     161,   166,   165,   260,   164,   167,   171,    42,   169,   173,
     174,   172,   177,   178,   179,   182,   180,   183,   188,   145,
     189,   224,   219,   678,   190,   225,   230,   682,   234,   236,
     241,   238,   818,   235,   239,   237,   240,   242,   245,   271,
     218,   249,   274,   250,   278,   306,   296,   315,   324,   332,
     335,   251,   362,   252,   253,   254,   707,   255,   381,   218,
     256,   257,   258,   259,   264,   851,   265,   285,   266,   267,
     270,   723,   268,   273,   343,   722,   283,   284,   287,   397,
     275,   277,   449,   350,   288,   300,   294,   312,   843,   384,
     260,   292,   295,   297,   301,   302,   744,   310,   313,   311,
     314,   364,   317,   325,   320,   321,   387,   328,   329,   478,
     330,   398,   508,   331,   512,   341,   342,   340,   515,   260,
     347,   520,   358,   568,   359,   361,   360,   503,   367,   375,
     392,   370,   529,   540,   376,   554,   377,   378,   379,   383,
     567,   572,   395,   608,   412,   419,   638,   431,   432,   396,
     436,   437,   543,   439,   424,   438,   441,   642,   440,   442,
     679,   443,   447,   430,   448,   452,   453,   454,   558,   455,
     457,   458,   459,   460,   698,   461,   462,   713,   465,   466,
     467,   468,   708,   472,   473,   730,   475,   482,   637,   494,
     474,   495,   496,   562,   491,   526,   506,   530,   547,   521,
     531,   532,   836,   522,   536,   538,   527,   548,   539,   550,
     260,   551,   557,   711,   571,   569,   584,   574,   836,   566,
     573,   643,   575,   578,   489,   576,   393,   579,   581,   582,
     583,   739,   585,   858,   586,   590,   542,   589,   598,   750,
     657,   597,   606,   665,   607,   610,   751,   617,   599,   600,
     609,   836,   611,   641,   613,   619,   712,   625,   633,   634,
     640,   644,   645,   654,   659,   660,   753,   668,   664,   669,
     674,   770,   677,   680,   681,   683,   684,   789,   802,   686,
     687,   688,   689,   782,   697,   695,   699,   718,   372,   700,
     701,   703,   704,    22,   714,   715,   553,   388,   724,   702,
     725,   716,   709,   717,   728,   719,   720,   726,   727,   729,
     791,   736,   737,   198,   738,   650,   741,   745,   747,   748,
     749,   752,   760,   763,    80,   765,   846,   767,   768,   759,
     693,   774,   772,   773,   775,   696,   776,   769,   777,   787,
     778,   779,   783,   785,   792,   796,   806,   801,   803,   804,
     813,   812,   807,   816,   814,   815,   110,   820,   849,   821,
     870,   788,   464,   795,   829,   830,   817,   822,   837,   824,
     823,   840,   841,   842,   844,   847,   850,   852,   853,   859,
     860,   861,   862,   140,   866,   754,   874,   863,   871,   872,
     864,   865,   873,   875,   876,   877,   742,   577,     0,     0,
     671,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   519,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   471,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     366,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,   528
};

static const yytype_int16 yycheck[] =
{
      84,    85,    86,   164,    88,    87,    94,    95,    96,   370,
     370,   255,   256,   294,   599,   600,   598,   223,   630,    14,
     104,   304,   106,    14,    26,   109,   168,    56,   170,   190,
     261,   237,   263,   138,   646,    56,   141,   497,    65,    54,
      24,     9,    10,    11,    12,    21,    14,   450,   132,   723,
      65,    37,    28,    65,    30,    65,    79,   514,    68,    65,
     148,    97,    68,    65,    32,   149,   347,    66,    47,    89,
      90,    91,    65,    56,    43,    57,    61,   159,    46,   323,
     164,   140,   166,   167,     0,   139,    65,   293,   230,   549,
     764,    14,   151,   358,    79,   339,    79,    65,   501,   126,
      68,   116,    82,   113,   140,    79,   190,   564,   314,   132,
      65,    80,   127,    87,    65,   321,   477,   477,   147,   363,
     140,   127,   149,   109,   145,   127,   711,   111,    65,   141,
     140,   591,   131,   101,   127,   296,   297,   116,   222,   223,
     346,   121,    58,    66,   409,   548,   390,   391,   127,     0,
     735,    34,   140,   237,   122,   123,   124,   125,   389,   264,
     155,   444,   244,   305,   155,   141,   134,   135,   142,   843,
     138,   145,   127,   128,   107,   126,   457,   762,    90,   116,
      79,   793,   794,   267,    65,   770,    44,    68,    90,    29,
     127,   103,   774,   775,   400,    78,    36,   782,    89,    90,
      91,   103,    89,    90,    91,   287,   791,   140,   141,   293,
      79,   107,   296,   297,   375,     9,    10,    11,    12,   118,
      14,    79,    90,    91,    89,    90,    91,    24,   140,   141,
     314,    79,   113,    56,   106,   103,   104,   321,    32,   141,
      79,   141,   110,   112,   488,   141,   114,    79,   148,   140,
       0,   140,    46,   140,   102,    87,    79,   129,    56,    56,
      99,   141,   346,   507,   508,   139,   350,   156,   148,    90,
      91,    65,   140,   141,    68,   140,   437,    65,   863,   864,
      68,    79,   103,   104,   119,   120,   370,   369,   140,   110,
     142,   375,   376,   114,    36,    24,    38,    59,    60,    61,
      29,   136,    31,    63,    64,   140,   141,   101,    50,    69,
      39,    71,    72,    73,    74,    75,   400,    79,    79,   146,
     141,   148,   140,   484,    85,    29,   570,    31,   122,   123,
     124,   125,    92,    93,    94,    39,   140,   140,    98,   142,
     134,   135,   119,   120,   138,   141,    76,    77,    78,    37,
     146,   145,   148,   437,    42,   115,    79,    63,    64,   136,
      83,   567,   568,    69,   141,    71,    72,    73,    74,    75,
     130,   615,   616,   141,   535,     6,     7,     8,   146,    62,
     140,   141,    89,    90,    91,   139,    92,    93,    94,   140,
     139,   142,    98,   477,   140,   140,   142,   142,    51,   140,
     484,   142,   140,   140,   142,   142,    79,    80,   492,   115,
      51,   617,   140,   139,   142,   576,   777,   777,   140,   140,
     142,   142,   140,   140,   130,   142,   140,   140,   142,   142,
     139,    18,   140,    19,   140,   141,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   530,   139,   609,   139,
     139,   535,   139,    35,    48,   141,   816,   141,    49,    63,
      64,    33,    25,   707,    23,    69,    31,    71,    72,    73,
      74,    75,   832,    34,    41,    50,    27,    37,    52,    43,
     724,   725,   566,   567,   568,   140,   140,    11,    92,    93,
      94,    75,   576,   654,    98,    65,    75,   126,   704,   139,
     744,   140,   862,    56,   586,   865,   139,   139,    70,    78,
     140,   115,   673,   139,   139,   149,   140,   139,    66,   140,
     139,   139,   139,   139,   606,   609,   130,   733,   139,   141,
     140,    22,   141,   617,   148,    21,    34,   141,   140,   139,
     139,   149,   139,   100,   139,    56,   117,   139,   146,    82,
     140,    78,   140,   635,   143,    56,    20,   641,    15,   139,
      13,   139,   806,   148,   140,   148,   139,    66,    56,    95,
     654,   139,    56,   139,    66,    81,   153,    56,    13,    56,
      56,   139,    13,   139,   139,   139,   668,   139,    56,   673,
     139,   141,   139,   139,   139,   839,   139,   141,   140,   139,
     139,   689,   140,   140,    84,   689,   140,   139,   139,    56,
     146,   146,    56,   154,   144,   140,   139,   139,   824,    88,
     704,   149,   143,   152,   140,   140,   708,   140,   140,   149,
     139,   150,   140,   140,   139,   139,    88,   140,   140,   149,
     140,    96,    45,   140,    24,   140,   140,   143,    22,   733,
     139,    23,   139,    43,   147,   140,   139,    97,   140,   140,
     150,   139,    56,    56,   139,    56,   140,   140,   140,   139,
      27,    56,   139,    36,   140,   140,     4,   139,   139,   150,
     140,   139,    86,   139,   147,   144,   140,    96,   139,   139,
      13,   140,   140,   777,   140,   140,   139,   139,   105,   140,
     139,   139,   139,   139,    14,   139,   139,    13,   140,   139,
     139,   139,    24,   140,   139,    14,   140,   140,   588,   140,
     146,   140,   139,   109,   146,   141,   146,   139,   139,   147,
     140,   144,   816,   148,   140,   140,   150,   145,   140,   140,
     824,   139,   139,    40,   140,   137,   141,   147,   832,   148,
     139,    96,   140,   140,   439,   139,   349,   140,   140,   140,
     139,    56,   140,   847,   139,   142,   493,   140,   139,    56,
     111,   147,   139,   133,   140,   139,    56,   139,   148,   148,
     148,   865,   140,   148,   140,   139,   147,   140,   140,   140,
     140,   140,   140,   148,   140,   139,    56,   139,   141,   139,
     139,   148,   140,   140,   140,   139,   139,    56,    56,   141,
     141,   140,   139,   148,   139,   141,   140,   147,   102,   141,
     141,   140,   139,    15,   140,   139,   504,   345,   139,   149,
     139,   146,   150,   146,   141,   140,   140,   140,   140,   140,
     148,   141,   140,    68,   140,   604,   140,   140,   140,   140,
     140,   140,   140,   139,    60,   140,   827,   140,   139,   725,
     653,   139,   141,   140,   139,   656,   140,   149,   139,   141,
     140,   140,   140,   149,   140,   140,   139,   141,   140,   140,
     147,   139,   141,   139,   141,   140,    89,   140,   832,   141,
     865,   768,   409,   777,   140,   140,   150,   147,   140,   150,
     147,   140,   140,   140,   139,   139,   150,   140,   140,   150,
     140,   140,   148,   115,   140,   722,   140,   148,   141,   141,
     148,   148,   141,   140,   140,   140,   706,   530,    -1,    -1,
     623,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   467,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   417,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     323,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   477
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    57,   158,   139,     0,     0,    58,   159,   140,   139,
       0,    59,    60,    61,    79,   160,   161,   162,   140,   139,
     139,   139,   162,    62,   163,   140,    51,    51,    63,    64,
      69,    71,    72,    73,    74,    75,    92,    93,    94,    98,
     115,   130,   141,   164,   165,   166,   171,   173,   174,   175,
     176,   194,   195,   197,   198,   203,   226,   234,   262,   140,
     139,   140,    18,    19,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   140,   165,
     164,   141,   141,    35,    48,    49,    33,    25,    23,    31,
      34,    41,    50,    37,    43,    27,    52,   140,   140,   140,
      11,    65,   257,   257,   257,   127,   168,   169,   257,   257,
     176,    76,    77,    78,   177,    34,    78,   196,    75,    79,
      80,   178,   126,   261,   261,   261,   140,   139,    56,    70,
     141,   257,   139,   257,   257,   140,   139,   139,   139,   140,
     196,   139,   140,   139,   139,    82,   121,   181,   199,   139,
      79,    99,   204,    79,   118,   227,   149,    66,   258,   139,
     139,   140,   141,   257,   148,   141,    22,    21,   178,   140,
     178,    34,   149,   139,   139,   261,   257,   139,   100,   139,
     117,   228,    56,   139,   140,   156,   167,   168,   146,   140,
     143,     9,    10,    11,    12,    14,    32,    46,    68,   101,
     122,   123,   124,   125,   134,   135,   138,   237,   238,   247,
     248,   249,   250,   251,   252,   253,   255,   256,   257,   140,
     257,   257,   181,   181,    78,    56,    54,   116,   168,   254,
      20,    79,    83,   182,    15,   148,   139,   148,   139,   140,
     139,    13,    66,   140,   142,    56,   145,   170,   237,   139,
     139,   139,   139,   139,   139,   139,   139,   141,   139,   139,
     257,   260,   261,   260,   139,   139,   140,   139,   140,   181,
     139,    95,   200,   140,    56,   146,   260,   146,    66,   131,
     233,   259,   263,   140,   139,   141,   168,   139,   144,     6,
       7,     8,   149,   239,   139,   143,   153,   152,   259,   259,
     140,   140,   140,   182,   182,   178,    81,   179,   180,   257,
     140,   149,   139,   140,   139,    56,   206,   140,    56,   147,
     139,   139,   140,   142,    13,   140,   168,   172,   140,   140,
     140,   140,    56,   260,   254,    56,   237,   237,    14,   155,
     143,   140,   140,    84,   183,   183,   181,   139,   140,   142,
     154,    79,    85,   184,   146,   148,   205,   260,   139,   147,
     139,   140,    13,   260,   150,   259,   263,   140,   140,   142,
     139,    79,   102,   242,   243,   140,   139,   140,   140,   140,
     259,    56,   235,   139,    88,   189,   190,    88,   190,   260,
      47,   254,   150,   180,   257,   139,   150,    56,    96,   140,
     142,    90,    91,   103,   104,   110,   114,   141,   193,   207,
     208,   209,   140,   119,   120,   136,   141,   229,   230,   140,
     259,    79,   132,   267,   147,   168,   169,   240,   241,   247,
     257,   139,   139,   237,   237,   246,   140,   139,   144,   139,
     139,   140,   139,   140,   182,   259,   259,   140,   140,    56,
     145,   185,   140,   139,   139,   140,   260,   139,   139,   139,
     139,   139,   139,   140,   208,   140,   139,   139,   139,   140,
     141,   230,   140,   139,   146,   140,   140,   142,   149,    56,
      79,   245,   140,   140,   142,   236,   237,    14,   155,   184,
     141,   146,   148,   183,   140,   140,   139,   185,    79,    87,
     142,   145,   188,    97,   201,   202,   146,   254,    45,    24,
      56,   210,    24,    37,    42,    22,    44,    79,   231,   231,
      23,   147,   148,    56,    79,   264,   141,   150,   241,    56,
     139,   140,   144,   237,   140,   151,   140,   259,   140,   140,
      56,   257,   189,    86,   186,   187,   188,   139,   145,   185,
     140,   139,   140,   202,    56,   259,   259,   139,   105,   213,
      29,    36,   109,   220,    37,   220,   148,    27,    43,   137,
     232,   140,    56,   139,   147,   140,   139,   246,   140,   140,
     237,   140,   140,   139,   141,   140,   139,   140,   142,   140,
     142,   185,   188,    21,    28,    30,   141,   147,   139,   148,
     148,    29,    31,    39,   211,   212,   139,   140,    36,   148,
     139,   140,   220,   140,   257,   260,   260,   139,   259,   139,
      90,   103,   141,   265,   266,   140,   237,   244,   140,    89,
     191,   192,   193,   140,   140,    26,   168,   187,     4,   188,
     140,   148,    96,    96,   140,   140,   191,   235,   235,    24,
     212,   128,   168,   214,   148,   237,    24,   111,   221,   140,
     139,   259,   259,   260,   141,   133,   268,   269,   139,   139,
     140,   266,   140,   142,   139,   140,   192,   140,   168,    13,
     140,   140,   257,   139,   139,   140,   141,   141,   140,   139,
     106,   129,   215,   215,   237,   141,   221,   139,    14,   140,
     141,   141,   149,   140,   139,   140,   142,   168,    24,   150,
     237,    40,   147,    13,   140,   139,   146,   146,   147,   140,
     140,   149,   257,   261,   139,   139,   140,   140,   141,   140,
      14,    79,   112,   222,   141,   148,   141,   140,   140,    56,
     260,   140,   269,   259,   168,   140,   235,   140,   140,   140,
      56,    56,   140,    56,   243,   242,    14,   216,   259,   216,
     140,   141,   148,   139,   260,   140,   235,   140,   139,   149,
     148,   259,   141,   140,   139,   139,   140,   139,   140,   140,
     141,   146,   148,   140,   235,   149,   242,   141,   233,    56,
     235,   148,   140,   191,   191,   240,   140,    56,    79,   217,
     235,   141,    56,   140,   140,   140,   139,   141,   235,   140,
     140,   140,   139,   147,   141,   140,   139,   150,   259,   270,
     140,   141,   147,   147,   150,   107,   141,   218,   219,   140,
     140,   113,   223,   224,   225,   247,   257,   140,   140,   142,
     140,   140,   140,   260,   139,   140,   219,   139,   140,   224,
     150,   259,   140,   140,   242,    36,    38,    50,   257,   150,
     140,   140,   148,   148,   148,   148,   140,   247,   235,   235,
     225,   141,   141,   141,   140,   140,   140,   140
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int16 yyr1[] =
{
       0,   157,   158,   158,   159,   159,   159,   160,   161,   162,
     162,   163,   164,   164,   165,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   166,
     167,   167,   168,   168,   169,   170,   170,   171,   172,   172,
     173,   173,   174,   175,   176,   177,   177,   177,   178,   178,
     179,   179,   180,   180,   181,   181,   182,   182,   183,   184,
     184,   184,   184,   184,   185,   186,   186,   187,   187,   188,
     188,   189,   189,   189,   190,   191,   191,   192,   192,   193,
     193,   194,   195,   195,   196,   197,   198,   199,   199,   200,
     200,   201,   201,   202,   202,   202,   202,   203,   204,   204,
     205,   205,   206,   206,   207,   207,   208,   208,   208,   208,
     208,   208,   208,   208,   208,   209,   210,   210,   211,   211,
     212,   212,   212,   213,   213,   214,   214,   214,   215,   215,
     215,   216,   216,   217,   217,   218,   218,   219,   219,   219,
     219,   220,   220,   220,   220,   221,   222,   222,   223,   223,
     224,   224,   225,   225,   226,   227,   227,   228,   228,   229,
     229,   229,   229,   230,   230,   230,   231,   231,   232,   233,
     233,   234,   235,   236,   236,   237,   237,   237,   237,   237,
     237,   237,   237,   237,   237,   237,   237,   237,   237,   238,
     239,   239,   240,   240,   241,   241,   241,   242,   242,   243,
     244,   244,   245,   245,   246,   246,   247,   247,   247,   248,
     248,   249,   249,   250,   251,   251,   252,   253,   254,   254,
     255,   255,   256,   256,   257,   258,   258,   259,   260,   260,
     261,   262,   233,   233,   263,   264,   264,   265,   265,   266,
     266,   266,   267,   267,   268,   268,   269,   270,   270
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
       1,     2,     2,     8,     8,     7,     1,     1,    10,     1,
       3,     5,     4,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     5,     1,     1,     1,     9,
       0,     6,     1,     3,     1,     1,     1,     1,     1,     9,
       1,     3,     1,     4,     1,     3,     4,     4,     4,     1,
       1,     1,     1,     9,     1,     1,     5,     7,     1,     6,
       6,     5,     9,     8,     4,     4,     6,     4,     1,     1,
       5,    15,     1,     3,     5,     1,     4,     1,     2,     8,
       9,     1,     1,     8,     1,     3,    10,     1,     3
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
#if AST_DEBUG

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
#else /* !AST_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !AST_DEBUG */


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
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-3].node), (yyvsp[-2].fqn) ); }
    break;

  case 166:
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 167:
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 168:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-9].tok) ); (yyval.node) -> AddNode ( ctx, (yyvsp[-7].fqn) ); (yyval.node) -> AddNode ( ctx, (yyvsp[-3].node) ); }
    break;

  case 169:
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 170:
                            { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 171:
                                           { (yyval.node) = p_builder . Include ( ctx, (yyvsp[-2].tok), (yyvsp[-1].tok) ); }
    break;

  case 172:
                                    { (yyval.expr) = (yyvsp[-1].expr); }
    break;

  case 173:
                             { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].expr) ); }
    break;

  case 174:
                             { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 175:
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 176:
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 177:
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
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
                                    { (yyval.expr) = (yyvsp[0].expr); }
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
                                    { (yyval.expr) = (yyvsp[-1].expr); }
    break;

  case 186:
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 187:
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 188:
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 189:
        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-5].fqn) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-4].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-2].node) ); }
    break;

  case 190:
                                                { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 191:
                                                { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 192:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 193:
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 194:
                    { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 195:
                    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 196:
                    { (yyval.node) = (yyvsp[0].expr); }
    break;

  case 197:
                            { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 198:
                            { (yyval.node) = (yyvsp[0].node); }
    break;

  case 199:
                                                                        { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 200:
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 201:
                                { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 202:
                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 203:
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 204:
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 205:
                            { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 206:
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 207:
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 208:
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 209:
                 { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 210:
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 211:
                        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 212:
                        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 213:
                                                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].node) ); }
    break;

  case 214:
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 215:
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 216:
                                 { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-4].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].expr) ); }
    break;

  case 217:
                                                  { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-6].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].expr) ); }
    break;

  case 218:
                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 219:
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }
    break;

  case 220:
                                                            { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-5].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].fqn) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].fqn) ); }
    break;

  case 221:
        {   /* remove leading '.'*/
            (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-4].tok) );
            (yyval.expr) -> AddNode ( ctx, (yyvsp[-2].fqn) );
            AST * ident = AST :: Make ( ctx, PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( ctx, & t );
            (yyval.expr) -> AddNode ( ctx, ident );
        }
    break;

  case 222:
        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-6].fqn) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-4].expr) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].fqn) ); }
    break;

  case 223:
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

  case 224:
                                { (yyval.fqn) = (yyvsp[-1].fqn); }
    break;

  case 225:
                                                        { (yyval.fqn) = AST_FQN :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 226:
                                                        { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 227:
                                        { (yyval.fqn) = AST_FQN :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 228:
                { (yyval.fqn) = (yyvsp[0].fqn); }
    break;

  case 229:
                { (yyval.fqn) = (yyvsp[0].fqn); }
    break;

  case 230:
                                        { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }
    break;

  case 231:
        { (yyval.node) = p_builder . ViewDef ( ctx, (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-2].node) ); }
    break;

  case 232:
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 233:
                                { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 234:
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].fqn) ); }
    break;

  case 235:
                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 236:
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 237:
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 238:
                            { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 239:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 240:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 241:
                                                                    { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 242:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 243:
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-2].node) ); }
    break;

  case 244:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 245:
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 246:
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-9].tok), (yyvsp[-7].fqn), (yyvsp[-3].node) ); }
    break;

  case 247:
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 248:
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
