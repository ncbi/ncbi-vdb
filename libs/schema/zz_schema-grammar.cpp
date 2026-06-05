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
#define YYSTYPE         SCHEMA_STYPE
#define YYLTYPE         SCHEMA_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         Schema_parse
#define yylex           Schema_lex
#define yyerror         Schema_error
#define yydebug         Schema_debug
#define yynerrs         Schema_nerrs

/* First part of user prologue.  */

    #define YYDEBUG 1

    #include <stdio.h>

    #include "ParseTree.hpp"
    #include "ErrorReport.hpp"

    using namespace ncbi::SchemaParser;

    #include "schema-grammar.hpp"
    #include "schema-lex.h"
    #define Schema_lex SchemaScan_yylex

    void Schema_error ( YYLTYPE *                   p_llocp,
                        ctx_t                       ctx,
                        ParseTree **                p_root,
                        ErrorReport *               p_errors,
                        struct SchemaScanBlock *    p_sb,
                        const char *                p_msg )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        /* send message to the C++ parser for proper display and recovery */
        Token :: Location loc ( p_sb -> file_name, p_llocp -> first_line, p_llocp -> first_column );
        p_errors -> ReportError ( ctx, loc, "%s", p_msg);
    }

    extern "C"
    {
        extern enum Schema_tokentype SchemaScan_yylex ( SCHEMA_STYPE *lvalp, SCHEMA_LTYPE *llocp, SchemaScanBlock* sb );
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
    T ( ctx_t ctx, SchemaToken & p_term )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );//TODO: catch
        assert ( p_term . subtree == 0 );
        return ParseTree::Make ( ctx, p_term );
    }

    /* Create production node */
    static
    ParseTree *
    MakeTree ( ctx_t ctx,
               int p_token,
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
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        ParseTree * ret = ParseTree :: Make ( ctx, Token ( p_token ) );
        if ( p_ch1 != 0 ) ret -> AddChild ( ctx, p_ch1 );
        if ( p_ch2 != 0 ) ret -> AddChild ( ctx, p_ch2 );
        if ( p_ch3 != 0 ) ret -> AddChild ( ctx, p_ch3 );
        if ( p_ch4 != 0 ) ret -> AddChild ( ctx, p_ch4 );
        if ( p_ch5 != 0 ) ret -> AddChild ( ctx, p_ch5 );
        if ( p_ch6 != 0 ) ret -> AddChild ( ctx, p_ch6 );
        if ( p_ch7 != 0 ) ret -> AddChild ( ctx, p_ch7 );
        if ( p_ch8 != 0 ) ret -> AddChild ( ctx, p_ch8 );
        if ( p_ch9 != 0 ) ret -> AddChild ( ctx, p_ch9 );
        return ret;
    }

    /* Create a flat list */
    static
    ParseTree *
    MakeList ( ctx_t ctx, SchemaToken & p_prod )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        ParseTree * ret = ParseTree :: Make ( ctx, Token ( PT_ASTLIST ) );
        ret -> AddChild ( ctx, P ( p_prod ) );
        return ret;
    }

    /* Add to a flat list node */
    static
    ParseTree *
    AddToList ( ctx_t ctx, ParseTree * p_root, ParseTree * p_br1, ParseTree * p_br2 = 0 )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        assert ( p_br1 != 0 );
        p_root -> AddChild ( ctx, p_br1 );
        if ( p_br2 != 0 )
        {
            p_root -> AddChild ( ctx, p_br2 );
        }
        return p_root;
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
#ifndef YY_SCHEMA_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_GRAMMAR_HPP_INCLUDED
# define YY_SCHEMA_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_GRAMMAR_HPP_INCLUDED
/* Debug traces.  */
#ifndef SCHEMA_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define SCHEMA_DEBUG 1
#  else
#   define SCHEMA_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define SCHEMA_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined SCHEMA_DEBUG */
#if SCHEMA_DEBUG
extern int Schema_debug;
#endif

/* Token type.  */
#ifndef SCHEMA_TOKENTYPE
# define SCHEMA_TOKENTYPE
  enum Schema_tokentype
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
#if ! defined SCHEMA_STYPE && ! defined SCHEMA_STYPE_IS_DECLARED
typedef SchemaToken SCHEMA_STYPE;
# define SCHEMA_STYPE_IS_TRIVIAL 1
# define SCHEMA_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined SCHEMA_LTYPE && ! defined SCHEMA_LTYPE_IS_DECLARED
typedef struct SCHEMA_LTYPE SCHEMA_LTYPE;
struct SCHEMA_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define SCHEMA_LTYPE_IS_DECLARED 1
# define SCHEMA_LTYPE_IS_TRIVIAL 1
#endif



int Schema_parse (ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb);

#endif /* !YY_SCHEMA_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_GRAMMAR_HPP_INCLUDED  */



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
         || (defined SCHEMA_LTYPE_IS_TRIVIAL && SCHEMA_LTYPE_IS_TRIVIAL \
             && defined SCHEMA_STYPE_IS_TRIVIAL && SCHEMA_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
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
#define YYFINAL  68
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   837

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  157
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  129
/* YYNRULES -- Number of rules.  */
#define YYNRULES  280
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  540

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
     146,   147,   144,   153,   139,   152,   155,   154,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   156,   138,
     148,   145,   149,     2,   151,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   142,     2,   143,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   140,   150,   141,     2,     2,     2,     2,
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

#if SCHEMA_DEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   316,   316,   317,   321,   322,   323,   329,   333,   334,
     338,   339,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   362,   370,   371,   377,
     382,   386,   393,   394,   398,   399,   403,   404,   410,   412,
     417,   421,   428,   435,   440,   446,   450,   451,   452,   462,
     467,   472,   473,   477,   481,   482,   486,   487,   491,   492,
     496,   497,   501,   505,   506,   508,   510,   512,   517,   518,
     522,   526,   530,   531,   533,   535,   537,   542,   543,   547,
     548,   553,   554,   558,   560,   562,   567,   568,   572,   573,
     580,   585,   592,   594,   602,   609,   619,   620,   624,   626,
     631,   632,   636,   638,   640,   642,   647,   654,   662,   663,
     667,   668,   672,   674,   679,   680,   684,   685,   686,   688,
     690,   691,   692,   693,   694,   696,   700,   702,   707,   708,
     712,   713,   717,   718,   719,   723,   725,   730,   732,   734,
     739,   741,   743,   748,   749,   753,   757,   758,   762,   763,
     767,   768,   769,   770,   774,   779,   781,   783,   785,   790,
     795,   796,   800,   801,   805,   806,   810,   811,   818,   819,
     823,   824,   828,   829,   830,   831,   832,   833,   834,   835,
     836,   837,   838,   839,   840,   844,   851,   860,   861,   865,
     866,   867,   871,   872,   876,   880,   881,   885,   886,   890,
     891,   895,   896,   897,   901,   902,   906,   907,   911,   915,
     916,   920,   921,   925,   926,   930,   931,   935,   936,   940,
     942,   949,   957,   958,   962,   963,   967,   968,   972,   973,
     974,   975,   979,   980,   984,   989,   994,   999,  1004,  1005,
    1012,  1020,  1021,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1034,  1038,  1042,  1046,  1047,  1053,  1057,  1058,  1062,
    1063,  1067,  1068,  1072,  1077,  1078,  1082,  1086,  1087,  1091,
    1092,  1096,  1098,  1100,  1104,  1105,  1109,  1110,  1114,  1119,
    1120
};
#endif

#if SCHEMA_DEBUG || YYERROR_VERBOSE || 1
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
  "PT_ALIASMEMBER", "PT_VIEWSPEC", "';'", "','", "'{'", "'}'", "'['",
  "']'", "'*'", "'='", "'('", "')'", "'<'", "'>'", "'|'", "'@'", "'-'",
  "'+'", "'/'", "'.'", "':'", "$accept", "parse", "source", "version_1_0",
  "schema_1_0_opt", "schema_1_0", "schema_1_0_decl", "typedef_1_0_decl",
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
  "phys_ident", "opt_column_body_1_0", "column_body_1_0",
  "column_stmt_1_0", "default_view_1_0_decl", "physmbr_1_0_decl",
  "phys_coldef_1_0", "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
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
  "table_member_1_0", "alias_member_1_0", "view_spec", "view_parms",
  "include_directive", "fqn_1_0", "ident_1_0", "empty", "fqn_vers",
  "fqn_opt_vers", "version_2_0", "schema_2_0_opt", "schema_2_0",
  "schema_2_0_decl", "view_2_0_decl", "view_parm", "opt_view_body",
  "view_body", "view_member", "opt_view_parents", "view_parents",
  "view_parent", "view_parent_parms", YY_NULLPTR
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
     385,   386,   387,   388,   389,   390,   391,   392,    59,    44,
     123,   125,    91,    93,    42,    61,    40,    41,    60,    62,
     124,    64,    45,    43,    47,    46,    58
};
# endif

#define YYPACT_NINF (-369)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-255)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     521,  -369,     8,     8,     8,    98,     8,    59,   168,    57,
      52,     8,     8,     8,    98,   392,  -369,   244,   248,   699,
     699,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,   635,  -369,    -8,  -369,     8,
     -49,    -9,   118,  -369,  -369,   114,     8,    -6,     8,     8,
      12,  -369,  -369,  -369,    76,  -369,  -369,  -369,   200,    59,
    -369,   120,    -8,   149,   125,  -369,   173,   220,  -369,  -369,
    -369,   699,  -369,  -369,     8,  -369,  -369,  -369,   635,  -369,
    -369,   597,   232,   125,   -14,   107,  -369,     8,   187,  -369,
    -369,   278,   125,    42,    89,     8,  -369,   189,  -369,     8,
      80,  -369,     8,  -369,    76,  -369,     8,  -369,     8,   195,
    -369,   -40,  -369,     8,   281,  -369,  -369,   193,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,   486,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
     486,  -369,     8,   386,  -369,   486,   486,   212,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,    34,
       2,  -369,   277,    -9,  -369,    61,  -369,  -369,   280,   299,
    -369,    12,  -369,  -369,     8,   282,  -369,   282,   313,  -369,
     279,  -369,  -369,     8,   -98,  -369,  -369,     8,   318,  -369,
     323,   328,  -369,   321,   203,  -369,  -369,   -33,  -369,  -369,
    -369,  -369,   486,     8,   486,   325,  -369,  -369,     8,  -369,
    -369,  -369,    78,  -369,    36,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,   125,    29,   326,  -369,  -369,   -12,     8,   324,
     378,   421,  -369,    27,  -369,    38,     8,  -369,  -369,     8,
     578,  -369,  -369,    19,   291,  -369,  -369,  -369,     8,  -369,
     217,   218,     8,  -369,  -369,  -369,   486,   486,   386,     8,
     107,   272,  -369,  -369,  -369,   222,   486,     8,   329,  -369,
    -369,     8,     8,     8,   461,   327,   141,  -369,  -369,    45,
     295,    66,   338,  -369,  -369,     8,   443,   335,   470,   334,
     386,  -369,   474,     8,  -369,    27,  -369,   345,   357,  -369,
    -369,   386,   430,  -369,   430,    18,   277,  -369,  -369,  -369,
    -369,     8,   359,  -369,  -369,  -369,  -369,   277,   363,    -1,
     486,   486,  -369,  -369,   360,   369,  -369,   371,     8,     8,
       8,   372,  -369,  -369,    53,     8,  -369,  -369,     8,   182,
     499,   366,   257,  -369,  -369,  -369,    84,     8,  -369,   370,
     338,   338,  -369,    -4,  -369,    84,  -369,   216,   375,   486,
     376,   502,   230,  -369,  -369,  -369,    86,   300,   277,  -369,
     486,   486,   234,  -369,   296,  -369,  -369,  -369,  -369,  -369,
     237,  -369,     8,  -369,   377,   380,  -369,  -369,     8,  -369,
    -369,  -369,   486,  -369,   242,   385,   389,   122,  -369,  -369,
       8,  -369,   372,   511,   390,  -369,  -369,    14,   182,  -369,
     486,    21,  -369,     8,    87,     8,  -369,  -369,  -369,  -369,
      51,   381,   486,   393,  -369,   317,   386,  -369,   386,  -369,
     486,  -369,   112,   128,     8,  -369,     0,   486,   382,   394,
    -369,     7,   486,  -369,  -369,  -369,  -369,  -369,   372,  -369,
    -369,   211,  -369,   182,  -369,   390,   134,  -369,  -369,   397,
    -369,   241,  -369,   396,   399,  -369,  -369,   486,  -369,  -369,
     125,   136,  -369,  -369,   277,   400,   402,   406,  -369,   412,
       0,  -369,  -369,   156,     8,     8,     8,  -369,     8,  -369,
     416,     7,  -369,   391,  -369,   390,  -369,  -369,   486,   413,
    -369,  -369,   162,  -369,  -369,   457,   486,   486,  -369,  -369,
    -369,  -369,   251,  -369,     8,   414,  -369,  -369,  -369,  -369,
     175,  -369,  -369,   423,   196,   209,     8,  -369,   418,   486,
    -369,  -369,  -369,  -369,  -369,   486,   210,   213,  -369,  -369
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   252,     0,   252,
     252,     0,     0,     0,     0,     0,    25,     0,     0,   252,
       4,    10,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   252,   251,     0,   241,     0,
      34,     0,   252,    91,    90,     0,     0,    40,     0,     0,
       0,    45,    46,    47,     0,    52,    51,   240,     0,   252,
      92,   252,     0,     0,    30,    94,     0,     0,     1,     3,
       5,     8,     9,    11,     0,   261,   258,     6,   257,   259,
     262,     0,     0,    44,     0,     0,   253,     0,     0,   222,
      38,     0,    40,     0,     0,     0,   215,     0,    54,     0,
      34,    58,     0,    59,     0,    96,     0,    93,     0,     0,
     108,     0,    27,     0,     0,     7,   256,     0,   260,   243,
     244,   245,   246,   247,   248,   249,   250,   242,    43,     0,
     201,   203,   202,   204,   205,   206,   207,   145,   214,   213,
     252,    37,     0,     0,   174,     0,     0,     0,   173,    36,
     170,   175,   176,   177,   178,   179,   180,   183,   184,   172,
     241,   255,   252,   254,   223,   252,   221,    39,     0,     0,
      56,     0,    53,    57,     0,   252,    97,   252,   109,   110,
       0,   107,    26,     0,     0,    32,    29,     0,     0,   211,
       0,   210,   209,     0,     0,   187,   191,   189,   181,   182,
      35,   218,     0,     0,     0,     0,   193,   192,     0,   233,
     231,   224,   252,   226,     0,   228,   229,   230,   232,    50,
      49,    55,   216,   252,     0,    61,    60,     0,     0,     0,
     128,   132,   133,   252,   134,   252,     0,   125,   113,     0,
       0,   114,   116,     0,     0,   130,   123,    28,     0,    31,
       0,   241,     0,   264,    42,   208,     0,     0,     0,     0,
       0,     0,   168,   217,   195,     0,   252,     0,     0,   225,
     227,     0,     0,     0,     0,     0,   252,    68,    63,   252,
       0,     0,     0,    95,   111,     0,     0,     0,     0,   252,
       0,   121,     0,     0,   160,   252,   120,     0,     0,   112,
     115,     0,     0,   117,     0,    34,   252,   129,   132,   131,
      33,     0,   252,   266,   212,   171,   188,   252,     0,     0,
       0,     0,   194,   199,     0,   198,   197,     0,     0,     0,
       0,   252,    70,    62,     0,     0,    64,    81,     0,     0,
       0,     0,   252,    77,    72,    83,     0,     0,    48,     0,
       0,     0,   102,     0,   100,     0,    98,     0,     0,     0,
       0,     0,     0,   162,   165,   167,   166,     0,   252,   122,
       0,     0,     0,   135,     0,   144,   143,   136,   139,   138,
     241,   265,     0,   274,     0,     0,   190,   220,     0,   169,
     196,   186,     0,   236,     0,     0,     0,     0,    65,    82,
       0,    69,   252,     0,   252,    79,    71,     0,     0,    73,
       0,     0,    86,     0,     0,     0,   103,   104,    99,   101,
       0,     0,     0,     0,   154,     0,     0,   161,     0,   155,
       0,   159,     0,     0,     0,   142,   252,     0,     0,   275,
     276,   252,   252,   219,   200,   237,   234,   235,   252,    66,
      80,     0,    74,     0,    78,   252,     0,    85,    87,     0,
      84,     0,   106,     0,     0,   118,   157,     0,   163,   164,
     166,     0,   127,   126,   252,     0,     0,     0,   153,     0,
     147,   148,   146,     0,     0,     0,     0,   273,     0,   267,
       0,   268,   269,     0,    67,   252,    75,    88,     0,     0,
     124,   119,     0,   156,   137,     0,     0,     0,   140,   149,
     141,   279,     0,   277,     0,     0,   263,   270,   185,    76,
       0,   105,   158,     0,     0,     0,     0,   278,     0,     0,
      89,   152,   150,   151,   280,     0,     0,     0,   271,   272
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -369,  -369,  -369,  -369,  -369,   546,   197,  -369,  -369,  -369,
    -369,  -369,  -369,    50,   306,  -369,   522,  -369,  -369,  -369,
    -369,   262,    44,  -369,  -369,   558,  -369,  -369,   403,   -35,
     398,  -369,  -369,  -206,  -289,  -369,  -369,  -262,  -368,  -312,
    -369,   221,  -106,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,   224,   110,  -369,  -369,  -369,  -369,  -369,   333,  -369,
    -369,  -369,   336,  -369,  -369,   275,  -369,   -93,  -369,  -369,
     101,  -369,  -213,   293,  -369,  -369,   163,   158,  -342,    83,
    -369,  -369,   287,   332,  -303,   288,  -369,   150,  -369,  -131,
    -369,  -369,  -369,  -369,  -369,  -369,   160,  -369,  -369,  -369,
    -369,  -369,  -369,   383,  -369,  -369,  -369,  -369,  -369,   268,
    -369,    -2,   283,   142,     4,   -32,  -369,  -369,  -369,   519,
    -369,   290,  -369,  -369,   108,  -369,  -369,   113,  -369
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    70,    20,    21,    22,   111,    23,
      63,   114,   184,    96,   147,    24,    45,    46,    25,    26,
      82,    27,    51,    52,    53,    54,    55,    97,    98,   102,
     224,   225,   275,   276,   277,   280,   341,   342,   343,   336,
     348,   411,   412,    28,    44,    29,    30,    31,   106,   283,
     353,   354,   356,    32,   109,   178,   181,   240,   241,   242,
     243,   244,   245,   303,   304,   373,   374,   148,   479,   480,
     481,   246,   291,   292,   293,   362,   363,   364,   261,   262,
     150,   151,   194,   195,   205,   206,   265,   324,   325,   152,
     153,   154,   155,   190,   191,   156,   413,   157,   158,    33,
      88,   166,   212,   213,   214,   215,   216,   217,   267,   250,
      34,   159,    38,   337,   161,   162,    35,    77,    78,    79,
      80,   253,   490,   491,   492,   384,   439,   440,   512
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,    40,    41,   379,    47,    36,    86,   -41,    42,    41,
      62,    64,   196,   387,   385,    61,   201,   349,   399,   398,
      36,    36,   296,   105,   350,    36,   351,    36,   432,   433,
     409,   486,    36,    86,    36,    83,   475,    84,   476,   454,
     338,   248,    36,   249,    92,   401,    93,    94,   100,    86,
     477,   289,   100,    39,    60,   164,   100,   399,    36,    95,
      40,   410,   289,   271,    36,   431,    36,   331,   456,   176,
     175,   338,    41,    48,    49,   295,   179,   404,   117,   272,
      48,    49,   369,   454,   208,   163,    59,   349,   471,    36,
     449,   410,   452,    85,   350,   483,   351,    36,   182,   183,
     163,   208,   100,   107,    41,   209,   163,    81,   401,   260,
     177,    40,   112,   130,   131,   132,   133,   134,   135,   136,
      36,   137,   209,    81,   410,   502,   399,   196,   281,   402,
     101,   129,     7,   282,   352,    36,   494,   418,   478,   138,
     100,   197,    81,   496,   202,   487,   455,    81,    81,    56,
      81,    56,    56,   139,   388,   252,   520,   203,   453,   365,
      85,    72,   457,   185,   524,   525,   204,   301,   149,   100,
     196,   504,   222,   273,    81,   290,   268,    76,    40,    57,
    -254,    40,  -254,   519,    89,   163,   290,   536,   168,   339,
      81,   495,   462,   537,   448,    36,   284,   400,    81,   210,
      50,    56,   211,   110,   352,    50,   163,    50,   338,   375,
      99,   375,   188,    36,   103,   399,   210,    73,   103,   269,
     104,    40,    85,   189,    36,   460,   163,   317,   198,   199,
     239,   428,    75,   247,   174,   169,    81,   338,    40,   329,
     330,   305,    81,    81,    68,    81,    40,   306,    69,   140,
     472,   141,    90,   142,   101,   143,   197,   163,   144,   145,
     146,   368,   320,    87,   103,   108,   473,    43,    73,   163,
     163,    40,   497,   274,   503,    75,    65,    40,   320,   252,
     334,    81,   192,   357,   320,   335,   320,   264,   366,   113,
     239,   163,    36,   302,   510,   365,   252,   365,   310,   197,
     522,   229,   193,   230,   207,   458,   320,   218,   231,   163,
     232,   115,   320,   530,   458,   307,   233,   226,   234,   226,
     308,   235,   232,   274,   236,   320,   163,   165,   171,   340,
     234,    99,    40,    40,   532,   180,    40,    40,   172,   314,
     315,   187,   258,   149,   100,   414,   320,   533,   538,   323,
     438,   539,   259,   100,   218,   200,   311,  -238,   116,   320,
     320,   321,   421,   320,   127,   278,   312,  -238,   160,   426,
     128,   322,    81,   258,   523,   294,  -239,   294,   170,   427,
     163,   311,   173,   434,   274,   274,  -239,   499,   403,   340,
     526,   445,   130,   131,   132,    40,   407,    81,    40,    36,
     527,   408,   474,   389,   390,    40,    40,   286,   326,   100,
      66,    67,   160,   461,   287,   319,   167,   237,   100,   186,
     238,   344,   320,   160,   366,   204,   470,   219,   160,   160,
     223,   294,   163,   345,   435,   346,   436,   294,   429,    40,
     347,   437,   423,    36,   137,   430,   220,   274,   207,    40,
     274,    40,   228,   438,   383,   466,   254,   340,   340,   207,
     416,   417,   467,   130,   131,   132,   255,   256,   257,   285,
     251,   266,   279,   288,   332,   444,   333,   328,   355,   358,
     359,   360,   290,   163,    40,   160,   263,   160,   367,    40,
     370,   488,   130,   131,   132,   133,   134,   135,   136,    36,
     137,   340,   371,   340,   382,   464,   386,   391,   392,   393,
     207,   397,   405,   406,   424,   415,   425,   441,   138,   297,
     422,     1,   298,   446,   450,   323,   442,   447,   463,   451,
     484,   465,   139,   485,   500,   313,   514,   501,   518,   160,
     160,   488,   498,   160,     2,   505,     3,   506,     4,   160,
     327,   507,     5,   508,     6,     7,     8,   516,     9,   529,
     521,   531,    10,   535,    11,    71,   318,    58,    91,    12,
      13,    14,    15,   300,   221,   227,   420,   419,   482,   377,
     309,   509,   361,   489,   326,   376,   469,   376,   372,   468,
     316,    36,   493,   378,   380,   270,   394,   118,   513,   517,
     229,   381,   230,   160,   160,     0,     0,   231,     0,   232,
      36,   251,   395,   396,     0,   233,   207,   234,     0,     0,
     235,     0,     0,   236,   119,   120,     0,   121,   140,     0,
       0,     0,   142,     0,   143,   122,     0,   144,   145,   146,
     123,     0,   160,     0,   124,     0,     0,     0,     0,   125,
       0,     0,   126,   160,   160,     0,     0,     0,     2,    16,
       3,     0,     4,     0,     0,     0,     5,     0,     6,     7,
       8,   443,     9,     0,     0,   160,    10,     0,    11,     0,
       0,     0,     0,    12,    13,    14,     0,    74,     0,     0,
       0,     0,     0,   160,     0,     0,   459,     0,     0,     0,
       0,     0,     0,     0,     0,   160,     0,     0,     0,     0,
       0,     0,     0,   160,     0,     0,   237,     0,     0,   299,
     160,     0,     2,     0,     3,   160,     4,     0,     0,     0,
       5,     0,     6,     7,     8,     0,     9,     0,     0,     0,
      10,     0,    11,     0,     0,     0,     0,    12,    13,    14,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   511,     0,     0,
       0,   515,     0,    16,     0,     0,     0,     0,     0,     0,
       0,   160,     0,     0,     0,     0,     0,     0,     0,   160,
     160,     0,     0,     0,     0,     0,     0,   528,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   534,
       0,     0,   160,     0,     0,     0,     0,     0,   160,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       2,     3,     4,   306,     6,    13,    15,    13,     4,    11,
      12,    13,   143,    14,   317,    11,    14,    21,     4,   331,
      13,    13,   235,    58,    28,    13,    30,    13,   370,   371,
     342,    24,    13,    15,    13,    37,    36,    39,    38,   407,
      26,   139,    13,   141,    46,   334,    48,    49,    50,    15,
      50,    24,    54,     3,    10,    87,    58,     4,    13,    47,
      62,    40,    24,    27,    13,   368,    13,   273,   410,   104,
     102,    26,    74,    21,    22,    37,   108,   339,    74,    43,
      21,    22,   295,   451,    23,    87,    34,    21,   430,    13,
     402,    40,   404,   142,    28,   437,    30,    13,   138,   139,
     102,    23,   104,    59,   106,    44,   108,   156,   397,   142,
     106,   113,    62,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    44,   156,    40,   467,     4,   258,   140,   335,
      54,   145,    34,   145,   138,    13,   448,   141,   138,    32,
     142,   143,   156,   455,   142,   138,   408,   156,   156,     7,
     156,     9,    10,    46,   155,   187,   498,   155,   144,   290,
     142,    19,   141,   113,   506,   507,   148,   148,    85,   171,
     301,   474,   174,   144,   156,   148,   208,    35,   180,    11,
     146,   183,   148,   495,    42,   187,   148,   529,   146,   144,
     156,   453,   141,   535,   400,    13,   228,   144,   156,   138,
     148,    59,   141,    61,   138,   148,   208,   148,    26,   302,
      50,   304,   129,    13,    54,     4,   138,    20,    58,   141,
      20,   223,   142,   140,    13,   138,   228,   259,   145,   146,
     180,   145,    35,   183,   154,   146,   156,    26,   240,   271,
     272,   243,   156,   156,     0,   156,   248,   243,     0,   142,
     138,   144,   138,   146,    54,   148,   258,   259,   151,   152,
     153,   293,   150,   145,   104,   145,   138,     5,    71,   271,
     272,   273,   138,   223,   138,    78,    14,   279,   150,   311,
     139,   156,   140,   285,   150,   144,   150,   204,   290,   140,
     240,   293,    13,   243,   138,   426,   328,   428,   248,   301,
     138,    22,   142,    24,   162,   411,   150,   165,    29,   311,
      31,   138,   150,   138,   420,    24,    37,   175,    39,   177,
      29,    42,    31,   273,    45,   150,   328,   140,   139,   279,
      39,   171,   334,   335,   138,   140,   338,   339,   149,   256,
     257,   148,   139,   260,   346,   347,   150,   138,   138,   266,
     382,   138,   149,   355,   212,   143,   139,   139,   138,   150,
     150,   139,   146,   150,    81,   223,   149,   149,    85,   139,
     138,   149,   156,   139,   505,   233,   139,   235,    95,   149,
     382,   139,    99,   149,   334,   335,   149,   146,   338,   339,
     139,   149,     6,     7,     8,   397,   139,   156,   400,    13,
     149,   144,   434,   320,   321,   407,   408,    29,   266,   411,
      18,    19,   129,   415,    36,   143,   138,   138,   420,   138,
     141,   279,   150,   140,   426,   148,   428,   147,   145,   146,
     148,   289,   434,   138,   138,   140,   140,   295,   138,   441,
     145,   145,   359,    13,    14,   145,   147,   397,   306,   451,
     400,   453,   139,   485,   312,   138,   138,   407,   408,   317,
     350,   351,   145,     6,     7,     8,   143,   139,   147,   145,
     187,   146,   146,    52,    13,   392,   149,   148,   140,    36,
     145,    11,   148,   485,   486,   202,   203,   204,    14,   491,
     145,   441,     6,     7,     8,     9,    10,    11,    12,    13,
      14,   451,   145,   453,   145,   422,   143,   147,   139,   138,
     368,   139,    13,   147,   138,   145,    14,   140,    32,   236,
     145,     0,   239,   138,    13,   442,   146,   138,   147,   139,
     148,   138,    46,   139,   138,   252,   486,   138,   147,   256,
     257,   491,   145,   260,    23,   145,    25,   145,    27,   266,
     267,   145,    31,   141,    33,    34,    35,   141,    37,   145,
     147,   138,    41,   145,    43,    19,   260,     9,    46,    48,
      49,    50,    51,   240,   171,   177,   355,   353,   436,   304,
     244,   480,   289,   441,   442,   302,   428,   304,   301,   426,
     258,    13,   442,   305,   311,   212,   328,    78,   485,   491,
      22,   311,    24,   320,   321,    -1,    -1,    29,    -1,    31,
      13,   328,   329,   330,    -1,    37,   474,    39,    -1,    -1,
      42,    -1,    -1,    45,    27,    28,    -1,    30,   142,    -1,
      -1,    -1,   146,    -1,   148,    38,    -1,   151,   152,   153,
      43,    -1,   359,    -1,    47,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    55,   370,   371,    -1,    -1,    -1,    23,   138,
      25,    -1,    27,    -1,    -1,    -1,    31,    -1,    33,    34,
      35,   388,    37,    -1,    -1,   392,    41,    -1,    43,    -1,
      -1,    -1,    -1,    48,    49,    50,    -1,    52,    -1,    -1,
      -1,    -1,    -1,   410,    -1,    -1,   413,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   422,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   430,    -1,    -1,   138,    -1,    -1,   141,
     437,    -1,    23,    -1,    25,   442,    27,    -1,    -1,    -1,
      31,    -1,    33,    34,    35,    -1,    37,    -1,    -1,    -1,
      41,    -1,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
     467,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   484,    -1,    -1,
      -1,   488,    -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   498,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   506,
     507,    -1,    -1,    -1,    -1,    -1,    -1,   514,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   526,
      -1,    -1,   529,    -1,    -1,    -1,    -1,    -1,   535,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,     0,    23,    25,    27,    31,    33,    34,    35,    37,
      41,    43,    48,    49,    50,    51,   138,   158,   159,   160,
     162,   163,   164,   166,   172,   175,   176,   178,   200,   202,
     203,   204,   210,   256,   267,   273,    13,   268,   269,   170,
     268,   268,   271,   178,   201,   173,   174,   268,    21,    22,
     148,   179,   180,   181,   182,   183,   270,    11,   182,    34,
     179,   271,   268,   167,   268,   178,    18,    19,     0,     0,
     161,   162,   270,   163,    52,   163,   270,   274,   275,   276,
     277,   156,   177,   268,   268,   142,    15,   145,   257,   270,
     138,   173,   268,   268,   268,    47,   170,   184,   185,   253,
     268,    54,   186,   253,    20,   186,   205,   179,   145,   211,
     270,   165,   170,   140,   168,   138,   138,   271,   276,    27,
      28,    30,    38,    43,    47,    52,    55,   269,   138,   145,
       6,     7,     8,     9,    10,    11,    12,    14,    32,    46,
     142,   144,   146,   148,   151,   152,   153,   171,   224,   236,
     237,   238,   246,   247,   248,   249,   252,   254,   255,   268,
     269,   271,   272,   268,   272,   140,   258,   138,   146,   146,
     269,   139,   149,   269,   154,   272,   186,   271,   212,   272,
     140,   213,   138,   139,   169,   170,   138,   148,   236,   236,
     250,   251,   270,   253,   239,   240,   246,   268,   236,   236,
     143,    14,   142,   155,   148,   241,   242,   270,    23,    44,
     138,   141,   259,   260,   261,   262,   263,   264,   270,   147,
     147,   185,   268,   148,   187,   188,   270,   187,   139,    22,
      24,    29,    31,    37,    39,    42,    45,   138,   141,   170,
     214,   215,   216,   217,   218,   219,   228,   170,   139,   141,
     266,   269,   272,   278,   138,   143,   139,   147,   139,   149,
     142,   235,   236,   269,   236,   243,   146,   265,   272,   141,
     260,    27,    43,   144,   170,   189,   190,   191,   270,   146,
     192,   140,   145,   206,   272,   145,    29,    36,    52,    24,
     148,   229,   230,   231,   270,    37,   229,   269,   269,   141,
     215,   148,   170,   220,   221,   268,   271,    24,    29,   219,
     170,   139,   149,   269,   236,   236,   240,   272,   171,   143,
     150,   139,   149,   236,   244,   245,   270,   269,   148,   272,
     272,   190,    13,   149,   139,   144,   196,   270,    26,   144,
     170,   193,   194,   195,   270,   138,   140,   145,   197,    21,
      28,    30,   138,   207,   208,   140,   209,   268,    36,   145,
      11,   230,   232,   233,   234,   246,   268,    14,   272,   229,
     145,   145,   239,   222,   223,   224,   269,   222,   242,   241,
     269,   278,   145,   270,   282,   241,   143,    14,   155,   236,
     236,   147,   139,   138,   266,   269,   269,   139,   196,     4,
     144,   191,   190,   170,   194,    13,   147,   139,   144,   196,
      40,   198,   199,   253,   268,   145,   209,   209,   141,   208,
     198,   146,   145,   236,   138,    14,   139,   149,   145,   138,
     145,   241,   235,   235,   149,   138,   140,   145,   272,   283,
     284,   140,   146,   269,   236,   149,   138,   138,   190,   196,
      13,   139,   196,   144,   195,   194,   235,   141,   199,   269,
     138,   268,   141,   147,   236,   138,   138,   145,   233,   234,
     268,   235,   138,   138,   272,    36,    38,    50,   138,   225,
     226,   227,   270,   235,   148,   139,    24,   138,   170,   270,
     279,   280,   281,   244,   196,   194,   196,   138,   145,   146,
     138,   138,   235,   138,   241,   145,   145,   145,   141,   227,
     138,   269,   285,   284,   170,   269,   141,   281,   147,   196,
     235,   147,   138,   246,   235,   235,   139,   149,   269,   145,
     138,   138,   138,   138,   269,   145,   235,   235,   138,   138
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int16 yyr1[] =
{
       0,   157,   158,   158,   159,   159,   159,   160,   161,   161,
     162,   162,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   164,   165,   165,   166,
     167,   168,   169,   169,   170,   170,   171,   171,   172,   172,
     173,   174,   175,   176,   177,   178,   179,   179,   179,   180,
     181,   182,   182,   183,   184,   184,   185,   185,   186,   186,
     187,   187,   188,   189,   189,   189,   189,   189,   190,   190,
     191,   192,   193,   193,   193,   193,   193,   194,   194,   195,
     195,   196,   196,   197,   197,   197,   198,   198,   199,   199,
     200,   201,   202,   202,   203,   204,   205,   205,   206,   206,
     207,   207,   208,   208,   208,   208,   209,   210,   211,   211,
     212,   212,   213,   213,   214,   214,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   216,   216,   217,   217,
     218,   218,   219,   219,   219,   220,   220,   221,   221,   221,
     222,   222,   222,   223,   223,   224,   225,   225,   226,   226,
     227,   227,   227,   227,   228,   229,   229,   229,   229,   230,
     231,   231,   232,   232,   233,   233,   234,   234,   235,   235,
     236,   236,   237,   237,   237,   237,   237,   237,   237,   237,
     237,   237,   237,   237,   237,   238,   238,   239,   239,   240,
     240,   240,   241,   241,   242,   243,   243,   244,   244,   245,
     245,   246,   246,   246,   247,   247,   248,   248,   249,   250,
     250,   251,   251,   252,   252,   253,   253,   254,   254,   255,
     255,   256,   257,   257,   258,   258,   259,   259,   260,   260,
     260,   260,   261,   261,   262,   263,   264,   265,   266,   266,
     267,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   269,   270,   271,   272,   272,   273,   274,   274,   275,
     275,   276,   276,   277,   266,   266,   278,   279,   279,   280,
     280,   281,   281,   281,   282,   282,   283,   283,   284,   285,
     285
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     2,     3,     1,     1,
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
       1,     2,     1,     1,     1,     2,     2,     5,     2,     2,
       4,     4,     2,     1,     1,     1,     1,     1,     1,     2,
       4,     4,     4,     1,     4,     3,     5,     4,     6,     3,
       1,     3,     1,     3,     3,     1,     1,     1,     1,     3,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     1,     1,     8,     5,     1,     3,     1,
       4,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     1,     3,     3,     2,     6,
       5,     4,     1,     2,     2,     3,     1,     2,     1,     1,
       1,     1,     1,     1,     5,     5,     4,     4,     1,     3,
       2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     0,     2,     1,     1,     3,     1,     1,     1,
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
        yyerror (&yylloc, ctx, root, errors, sb, YY_("syntax error: cannot back up")); \
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
#if SCHEMA_DEBUG

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
# if defined SCHEMA_LTYPE_IS_TRIVIAL && SCHEMA_LTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
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
                  Type, Value, Location, ctx, root, errors, sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (yylocationp);
  YYUSE (ctx);
  YYUSE (root);
  YYUSE (errors);
  YYUSE (sb);
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
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yytype, yyvaluep, yylocationp, ctx, root, errors, sb);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , ctx, root, errors, sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, ctx, root, errors, sb); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !SCHEMA_DEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !SCHEMA_DEBUG */


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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (ctx);
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
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 3: /* UNRECOGNIZED  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 4: /* ELLIPSIS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 5: /* INCREMENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 6: /* DECIMAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 7: /* OCTAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 8: /* HEX  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 9: /* FLOAT_  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 10: /* EXP_FLOAT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 11: /* STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 12: /* ESCAPED_STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 13: /* IDENTIFIER_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 14: /* PHYSICAL_IDENTIFIER_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 15: /* VERSION  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 16: /* UNTERM_STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 17: /* UNTERM_ESCAPED_STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 18: /* VERS_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 19: /* VERS_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 20: /* KW___no_header  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 21: /* KW___row_length  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 22: /* KW___untyped  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 23: /* KW_alias  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 24: /* KW_column  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 25: /* KW_const  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 26: /* KW_control  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 27: /* KW_database  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 28: /* KW_decode  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 29: /* KW_default  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 30: /* KW_encode  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 31: /* KW_extern  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 32: /* KW_false  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 33: /* KW_fmtdef  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 34: /* KW_function  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 35: /* KW_include  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 36: /* KW_limit  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 37: /* KW_physical  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 38: /* KW_read  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 39: /* KW_readonly  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 40: /* KW_return  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 41: /* KW_schema  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 42: /* KW_static  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 43: /* KW_table  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 44: /* KW_template  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 45: /* KW_trigger  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 46: /* KW_true  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 47: /* KW_type  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 48: /* KW_typedef  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 49: /* KW_typeset  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 50: /* KW_validate  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 51: /* KW_version  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 52: /* KW_view  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 53: /* KW_virtual  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 54: /* KW_void  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 55: /* KW_write  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 56: /* PT_ASTLIST  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 57: /* PT_PARSE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 58: /* PT_SOURCE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 59: /* PT_VERSION_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 60: /* PT_VERSION_2  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 61: /* PT_SCHEMA_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 62: /* PT_SCHEMA_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 63: /* PT_INCLUDE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 64: /* PT_TYPEDEF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 65: /* PT_FQN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 66: /* PT_IDENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 67: /* PT_PHYSIDENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 68: /* PT_UINT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 69: /* PT_TYPESET  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 70: /* PT_TYPESETDEF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 71: /* PT_FORMAT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 72: /* PT_CONST  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 73: /* PT_ALIAS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 74: /* PT_EXTERN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 75: /* PT_FUNCTION  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 76: /* PT_UNTYPED  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 77: /* PT_ROWLENGTH  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 78: /* PT_FUNCDECL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 79: /* PT_EMPTY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 80: /* PT_SCHEMASIG  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 81: /* PT_SCHEMAFORMAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 82: /* PT_RETURNTYPE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 83: /* PT_FACTSIG  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 84: /* PT_FUNCSIG  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 85: /* PT_FUNCPARAMS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 86: /* PT_FORMALPARAM  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 87: /* PT_ELLIPSIS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 88: /* PT_FUNCPROLOGUE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 89: /* PT_RETURN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 90: /* PT_PRODSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 91: /* PT_PRODTRIGGER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 92: /* PT_SCHEMA  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 93: /* PT_VALIDATE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 94: /* PT_PHYSICAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 95: /* PT_PHYSPROLOGUE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 96: /* PT_PHYSSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 97: /* PT_PHYSBODYSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 98: /* PT_TABLE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 99: /* PT_TABLEPARENTS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 100: /* PT_TABLEBODY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 101: /* PT_FUNCEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 102: /* PT_FACTPARMS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 103: /* PT_COLUMN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 104: /* PT_COLUMNEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 105: /* PT_COLDECL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 106: /* PT_TYPEDCOL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 107: /* PT_COLSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 108: /* PT_DFLTVIEW  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 109: /* PT_PHYSMBR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 110: /* PT_PHYSCOL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 111: /* PT_PHYSCOLDEF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 112: /* PT_COLSCHEMAPARMS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 113: /* PT_COLSCHEMAPARAM  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 114: /* PT_COLUNTYPED  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 115: /* PT_DATABASE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 116: /* PT_TYPEEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 117: /* PT_DBBODY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 118: /* PT_DBDAD  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 119: /* PT_DBMEMBER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 120: /* PT_TBLMEMBER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 121: /* PT_NOHEADER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 122: /* PT_CASTEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 123: /* PT_CONSTVECT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 124: /* PT_NEGATE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 125: /* PT_UNARYPLUS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 126: /* PT_VERSNAME  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 127: /* PT_ARRAY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 128: /* PT_PHYSENCREF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 129: /* PT_TYPEDCOLEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 130: /* PT_VIEW  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 131: /* PT_VIEWPARAM  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 132: /* PT_VIEWPARENTS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 133: /* PT_VIEWPARENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 134: /* PT_MEMBEREXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 135: /* PT_JOINEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 136: /* PT_ALIASMEMBER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 137: /* PT_VIEWSPEC  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 138: /* ';'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 139: /* ','  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 140: /* '{'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 141: /* '}'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 142: /* '['  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 143: /* ']'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 144: /* '*'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 145: /* '='  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 146: /* '('  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 147: /* ')'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 148: /* '<'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 149: /* '>'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 150: /* '|'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 151: /* '@'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 152: /* '-'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 153: /* '+'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 154: /* '/'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 155: /* '.'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 156: /* ':'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 158: /* parse  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 159: /* source  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 160: /* version_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 161: /* schema_1_0_opt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 162: /* schema_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 163: /* schema_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 164: /* typedef_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 165: /* typedef_1_0_new_name_list  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 166: /* typeset_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 167: /* typeset_1_0_new_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 168: /* typeset_1_0_def  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 169: /* typespec_1_0_list  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 170: /* typespec_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 171: /* dim_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 172: /* format_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 173: /* format_1_0_new_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 174: /* format_1_0_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 175: /* const_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 176: /* alias_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 177: /* alias_1_0_new_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 178: /* function_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 179: /* func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 180: /* untyped_func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 181: /* row_length_func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 182: /* opt_func_1_0_schema_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 183: /* func_1_0_schema_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 184: /* func_1_0_schema_formals  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 185: /* func_1_0_schema_formal  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 186: /* func_1_0_return_type  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 187: /* opt_func_1_0_fact_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 188: /* func_1_0_fact_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 189: /* func_1_0_fact_signature  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 190: /* func_1_0_fact_params  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 191: /* fact_param_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 192: /* func_1_0_param_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 193: /* func_1_0_param_signature  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 194: /* func_1_0_formal_params  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 195: /* formal_param_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 196: /* func_1_0_vararg_formals  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 197: /* func_1_0_prologue  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 198: /* script_1_0_stmt_seq  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 199: /* script_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 200: /* extern_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 201: /* ext_func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 202: /* script_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 203: /* validate_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 204: /* physical_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 205: /* phys_1_0_return_type  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 206: /* phys_1_0_prologue  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 207: /* phys_1_0_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 208: /* phys_1_0_body_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 209: /* phys_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 210: /* table_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 211: /* opt_tbl_1_0_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 212: /* tbl_1_0_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 213: /* tbl_1_0_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 214: /* tbl_1_0_stmt_seq  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 215: /* tbl_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 216: /* production_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 217: /* col_1_0_modifiers  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 218: /* col_1_0_modifier_seq  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 219: /* col_1_0_modifier  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 220: /* col_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 221: /* phys_enc_ref  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 222: /* typed_column_decl_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 223: /* col_ident  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 224: /* phys_ident  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 225: /* opt_column_body_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 226: /* column_body_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 227: /* column_stmt_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 228: /* default_view_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 229: /* physmbr_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 230: /* phys_coldef_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 231: /* opt_col_schema_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 232: /* col_schema_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 233: /* col_schema_parm_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 234: /* col_schema_value_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 235: /* cond_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 236: /* expression_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 237: /* primary_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 238: /* func_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 239: /* schema_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 240: /* schema_parm_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 241: /* opt_factory_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 242: /* factory_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 243: /* factory_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 244: /* opt_func_1_0_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 245: /* func_1_0_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 246: /* uint_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 247: /* float_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 248: /* string_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 249: /* const_vect_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 250: /* opt_const_vect_exprlist_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 251: /* const_vect_exprlist_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 252: /* bool_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 253: /* type_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 254: /* member_expr_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 255: /* join_expr_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 256: /* database_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 257: /* opt_database_dad_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 258: /* database_body_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 259: /* database_members_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 260: /* database_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 261: /* opt_template_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 262: /* db_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 263: /* table_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 264: /* alias_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 265: /* view_spec  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 266: /* view_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 267: /* include_directive  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 268: /* fqn_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 269: /* ident_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 270: /* empty  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 271: /* fqn_vers  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 272: /* fqn_opt_vers  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 273: /* version_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 274: /* schema_2_0_opt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 275: /* schema_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 276: /* schema_2_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 277: /* view_2_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 278: /* view_parm  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 279: /* opt_view_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 280: /* view_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 281: /* view_member  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 282: /* opt_view_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 283: /* view_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 284: /* view_parent  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case 285: /* view_parent_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
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
yyparse (ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
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
# if defined SCHEMA_LTYPE_IS_TRIVIAL && SCHEMA_LTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYPTRDIFF_T yystacksize;

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
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
                                { *root = MakeTree ( ctx, PT_PARSE, T ( ctx, yyvsp[0] ) );              yyval . subtree = 0; yyval . leading_ws = 0;  }
    break;

  case 3:
                                { *root = MakeTree ( ctx, PT_PARSE, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) );    yyval . subtree = 0; yyval . leading_ws = 0; }
    break;

  case 4:
                                    { yyval . subtree = MakeTree ( ctx, PT_SOURCE, P ( yyvsp[0] ) ); }
    break;

  case 5:
                                    { yyval . subtree = MakeTree ( ctx, PT_SOURCE, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 6:
                                    { yyval . subtree = MakeTree ( ctx, PT_SOURCE, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 7:
                                { yyval . subtree = MakeTree ( ctx, PT_VERSION_1_0, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 8:
                    { yyval . subtree = P ( yyvsp[0] ); }
    break;

  case 9:
                    { yyval = yyvsp[0]; }
    break;

  case 10:
                                   { yyval . subtree = MakeTree ( ctx, PT_SCHEMA_1_0, P ( yyvsp[0] ) ); }
    break;

  case 11:
                                   { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ) , P ( yyvsp[0] ) ); }
    break;

  case 12:
                            { yyval = yyvsp[0]; }
    break;

  case 13:
                            { yyval = yyvsp[0]; }
    break;

  case 14:
                            { yyval = yyvsp[0]; }
    break;

  case 15:
                            { yyval = yyvsp[0]; }
    break;

  case 16:
                            { yyval = yyvsp[0]; }
    break;

  case 17:
                            { yyval = yyvsp[0]; }
    break;

  case 18:
                            { yyval = yyvsp[0]; }
    break;

  case 19:
                            { yyval = yyvsp[0]; }
    break;

  case 20:
                            { yyval = yyvsp[0]; }
    break;

  case 21:
                            { yyval = yyvsp[0]; }
    break;

  case 22:
                            { yyval = yyvsp[0]; }
    break;

  case 23:
                            { yyval = yyvsp[0]; }
    break;

  case 24:
                            { yyval = yyvsp[0]; }
    break;

  case 25:
                            { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 26:
                            { yyval . subtree = MakeTree ( ctx, PT_TYPEDEF, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 27:
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 28:
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 29:
                            { yyval . subtree = MakeTree ( ctx, PT_TYPESET, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 30:
                            { yyval = yyvsp[0]; }
    break;

  case 31:
            { yyval . subtree = MakeTree ( ctx, PT_TYPESETDEF, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 32:
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 33:
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 34:
                                { yyval = yyvsp[0]; }
    break;

  case 35:
                                { yyval . subtree = MakeTree ( ctx, PT_ARRAY, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 36:
                        { yyval = yyvsp[0]; }
    break;

  case 37:
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 38:
                                    { yyval . subtree = MakeTree ( ctx, PT_FORMAT, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 39:
                                    { yyval . subtree = MakeTree ( ctx, PT_FORMAT, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 40:
                                    { yyval = yyvsp[0]; }
    break;

  case 41:
                                    { yyval = yyvsp[0]; }
    break;

  case 42:
            { yyval . subtree = MakeTree ( ctx, PT_CONST, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 43:
            { yyval . subtree = MakeTree ( ctx, PT_ALIAS, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 44:
                                    { yyval = yyvsp[0]; }
    break;

  case 45:
                                     { yyval . subtree = MakeTree ( ctx, PT_FUNCTION, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 46:
                                     { yyval = yyvsp[0]; }
    break;

  case 47:
                                     { yyval = yyvsp[0]; }
    break;

  case 48:
            { yyval . subtree = MakeTree ( ctx, PT_FUNCDECL, P ( yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 49:
            { yyval . subtree = MakeTree ( ctx, PT_UNTYPED, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 50:
            { yyval . subtree = MakeTree ( ctx, PT_ROWLENGTH, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 51:
                                { yyval = yyvsp[0]; }
    break;

  case 52:
                                { yyval = yyvsp[0]; }
    break;

  case 53:
                                            { yyval . subtree = MakeTree ( ctx, PT_SCHEMASIG, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 54:
                                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 55:
                                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 56:
                                { yyval . subtree = MakeTree ( ctx, PT_SCHEMAFORMAL, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 57:
                                { yyval . subtree = MakeTree ( ctx, PT_SCHEMAFORMAL, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 58:
                        { yyval . subtree = MakeTree ( ctx, PT_RETURNTYPE, T ( ctx, yyvsp[0] ) ); }
    break;

  case 59:
                        { yyval . subtree = MakeTree ( ctx, PT_RETURNTYPE, P ( yyvsp[0] ) ); }
    break;

  case 60:
                                { yyval = yyvsp[0]; }
    break;

  case 61:
                                { yyval = yyvsp[0]; }
    break;

  case 62:
                                       { yyval . subtree = MakeTree ( ctx, PT_FACTSIG, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 63:
                            { yyval = yyvsp[0]; }
    break;

  case 64:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 65:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 66:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 67:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 68:
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 69:
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 70:
                                        { yyval . subtree = MakeTree ( ctx, PT_FORMALPARAM, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 71:
                                        { yyval . subtree = MakeTree ( ctx, PT_FUNCSIG, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 72:
                            { yyval = yyvsp[0]; }
    break;

  case 73:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 74:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 75:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 76:
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 77:
                                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 78:
                                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 79:
                                        { yyval . subtree = MakeTree ( ctx, PT_FORMALPARAM, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 80:
                                        { yyval . subtree = MakeTree ( ctx, PT_FORMALPARAM, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 81:
                                { yyval = yyvsp[0]; }
    break;

  case 82:
                                { yyval . subtree = MakeTree ( ctx, PT_ELLIPSIS, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 83:
                                { yyval . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, yyvsp[0] ) ); }
    break;

  case 84:
                                { yyval . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 85:
                                { yyval . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 86:
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 87:
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 88:
                                    { yyval . subtree = MakeTree ( ctx, PT_RETURN, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 89:
                                     { yyval . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 90:
            { yyval . subtree = MakeTree ( ctx, PT_EXTERN, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 91:
                                    { yyval = yyvsp[0]; }
    break;

  case 92:
                            { yyval . subtree = MakeTree ( ctx, PT_SCHEMA, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 93:
                            { yyval . subtree = MakeTree ( ctx, PT_SCHEMA, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 94:
                            { yyval . subtree = MakeTree ( ctx, PT_VALIDATE, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 95:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSICAL, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 96:
                                            { yyval = yyvsp[0]; }
    break;

  case 97:
                                            { yyval . subtree = MakeTree ( ctx, PT_NOHEADER, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 98:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSPROLOGUE, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 99:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSPROLOGUE, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 100:
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 101:
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 102:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 103:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 104:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 105:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 106:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSSTMT, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 107:
            { yyval . subtree = MakeTree ( ctx, PT_TABLE, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 108:
                                { yyval = yyvsp[0]; }
    break;

  case 109:
                                { yyval . subtree = MakeTree ( ctx, PT_TABLEPARENTS, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 110:
                                          { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 111:
                                          { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 112:
            { yyval . subtree = MakeTree ( ctx, PT_TABLEBODY, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 113:
            { yyval . subtree = MakeTree ( ctx, PT_TABLEBODY, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 114:
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 115:
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 116:
                                                { yyval = yyvsp[0]; }
    break;

  case 117:
                                                { yyval . subtree = MakeTree ( ctx, PT_COLUMN, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 118:
                        { yyval . subtree = MakeTree ( ctx, PT_COLUMNEXPR, T ( ctx, yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 119:
                        { yyval . subtree = MakeTree ( ctx, PT_COLUMNEXPR, T ( ctx, yyvsp[-5] ), T ( ctx, yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 120:
                                                { yyval . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 121:
                                                { yyval . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 122:
                                                { yyval . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 123:
                                                { yyval = yyvsp[0]; }
    break;

  case 124:
                        { yyval . subtree = MakeTree ( ctx, PT_COLUNTYPED, T ( ctx, yyvsp[-5] ), T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 125:
                                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 126:
                                     { yyval . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 127:
                                     { yyval . subtree = MakeTree ( ctx, PT_PRODTRIGGER, T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 128:
                                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 129:
                                        { yyval = yyvsp[-1]; AddToList ( ctx, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 130:
                                                { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 131:
                                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 132:
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 133:
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 134:
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 135:
            { yyval . subtree = MakeTree ( ctx, PT_COLDECL, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 136:
            { yyval . subtree = MakeTree ( ctx, PT_COLDECL, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 137:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSENCREF, T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 138:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSENCREF, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 139:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSENCREF, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 140:
            { yyval . subtree = MakeTree ( ctx, PT_TYPEDCOL, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 141:
            { yyval . subtree = MakeTree ( ctx, PT_TYPEDCOLEXPR, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 142:
            { yyval . subtree = MakeTree ( ctx, PT_TYPEDCOL, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 143:
                                    { yyval = yyvsp[0]; }
    break;

  case 144:
                                    { yyval = yyvsp[0]; }
    break;

  case 145:
                                    { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 146:
                        { yyval = yyvsp[0]; }
    break;

  case 147:
                        { yyval = yyvsp[0]; }
    break;

  case 148:
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 149:
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 150:
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 151:
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 152:
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 153:
                                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 154:
            { yyval . subtree = MakeTree ( ctx, PT_DFLTVIEW, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 155:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 156:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 157:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 158:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 159:
            { yyval . subtree = MakeTree ( ctx, PT_PHYSCOLDEF, P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 160:
                                        { yyval = yyvsp[0]; }
    break;

  case 161:
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSCHEMAPARMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 162:
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 163:
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 164:
                                            { yyval . subtree = MakeTree ( ctx, PT_COLSCHEMAPARAM, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 165:
                                            { yyval = yyvsp[0]; }
    break;

  case 166:
                                            { yyval = yyvsp[0]; }
    break;

  case 167:
                                            { yyval = yyvsp[0]; }
    break;

  case 168:
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 169:
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 170:
                                            { yyval = yyvsp[0]; }
    break;

  case 171:
                                            { yyval . subtree = MakeTree ( ctx, PT_CASTEXPR, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 172:
                                { yyval = yyvsp[0]; }
    break;

  case 173:
                                { yyval = yyvsp[0]; }
    break;

  case 174:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 175:
                                { yyval = yyvsp[0]; }
    break;

  case 176:
                                { yyval = yyvsp[0]; }
    break;

  case 177:
                                { yyval = yyvsp[0]; }
    break;

  case 178:
                                { yyval = yyvsp[0]; }
    break;

  case 179:
                                { yyval = yyvsp[0]; }
    break;

  case 180:
                                { yyval = yyvsp[0]; }
    break;

  case 181:
                                { yyval . subtree = MakeTree ( ctx, PT_NEGATE, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 182:
                                { yyval . subtree = MakeTree ( ctx, PT_UNARYPLUS, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 183:
                                { yyval = yyvsp[0]; }
    break;

  case 184:
                                { yyval = yyvsp[0]; }
    break;

  case 185:
             { yyval . subtree = MakeTree ( ctx, PT_FUNCEXPR, T ( ctx, yyvsp[-7] ), P ( yyvsp[-6] ), T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 186:
             { yyval . subtree = MakeTree ( ctx, PT_FUNCEXPR, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 187:
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 188:
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 189:
                                { yyval = yyvsp[0]; }
    break;

  case 190:
                                { yyval . subtree = MakeTree ( ctx, PT_ARRAY, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 191:
                                { yyval = yyvsp[0]; }
    break;

  case 192:
                        { yyval = yyvsp[0]; }
    break;

  case 193:
                        { yyval = yyvsp[0]; }
    break;

  case 194:
                            { yyval . subtree = MakeTree ( ctx, PT_FACTPARMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 195:
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 196:
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 197:
                                            { yyval = yyvsp[0]; }
    break;

  case 198:
                                            { yyval = yyvsp[0]; }
    break;

  case 199:
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 200:
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 201:
                                            { yyval . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 202:
                                            { yyval . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 203:
                                            { yyval . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 204:
                                 { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 205:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 206:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 207:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 208:
                                               { yyval . subtree = MakeTree ( ctx, PT_CONSTVECT, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 209:
                                { yyval = yyvsp[0]; }
    break;

  case 210:
                                { yyval = yyvsp[0]; }
    break;

  case 211:
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 212:
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 213:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 214:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 215:
                                { yyval = yyvsp[0]; }
    break;

  case 216:
                                { yyval . subtree = MakeTree ( ctx, PT_TYPEEXPR, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1]), P ( yyvsp[0] ) ); }
    break;

  case 217:
                                        { yyval . subtree = MakeTree ( ctx, PT_MEMBEREXPR, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 218:
                                        { yyval . subtree = MakeTree ( ctx, PT_MEMBEREXPR, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 219:
        { yyval . subtree = MakeTree ( ctx, PT_JOINEXPR, P ( yyvsp[-5] ), T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P (yyvsp[0] ) ); }
    break;

  case 220:
        { yyval . subtree = MakeTree ( ctx, PT_JOINEXPR, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 221:
            { yyval . subtree = MakeTree ( ctx, PT_DATABASE, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2]), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 222:
                                    { yyval = yyvsp[0]; }
    break;

  case 223:
                                    { yyval . subtree = MakeTree ( ctx, PT_DBDAD, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 224:
                                    { yyval . subtree = MakeTree ( ctx, PT_DBBODY, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 225:
                                    { yyval . subtree = MakeTree ( ctx, PT_DBBODY, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 226:
                                                { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 227:
                                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 228:
                                { yyval = yyvsp[0]; }
    break;

  case 229:
                                { yyval = yyvsp[0]; }
    break;

  case 230:
                                { yyval = yyvsp[0]; }
    break;

  case 231:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 232:
                                { yyval = yyvsp[0]; }
    break;

  case 233:
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 234:
            { yyval . subtree = MakeTree ( ctx, PT_DBMEMBER, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 235:
            { yyval . subtree = MakeTree ( ctx, PT_TBLMEMBER, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 236:
            { yyval . subtree = MakeTree ( ctx, PT_ALIASMEMBER, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 237:
        { yyval . subtree = MakeTree ( ctx, PT_VIEWSPEC, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 238:
                               { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 239:
                               { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 240:
                            { yyval . subtree = MakeTree ( ctx, PT_INCLUDE, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 241:
                                { yyval . subtree = MakeTree ( ctx, PT_FQN, P ( yyvsp[0] ) ); }
    break;

  case 242:
                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 243:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 244:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 245:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 246:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 247:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 248:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 249:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 250:
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 251:
                        { yyval . subtree = MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 252:
                { yyval . subtree = MakeTree ( ctx, PT_EMPTY ); }
    break;

  case 253:
                            { yyval . subtree = MakeTree ( ctx, PT_VERSNAME, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 254:
                     { yyval = yyvsp[0]; }
    break;

  case 255:
                     { yyval = yyvsp[0]; }
    break;

  case 256:
                                   { yyval . subtree = MakeTree ( ctx, PT_VERSION_2, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 257:
                    { yyval . subtree = P ( yyvsp[0] ); }
    break;

  case 258:
                    { yyval = yyvsp[0]; }
    break;

  case 259:
                                   { yyval . subtree = MakeTree ( ctx, PT_SCHEMA_2_0, P ( yyvsp[0] ) ); }
    break;

  case 260:
                                   { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ) , P ( yyvsp[0] ) ); }
    break;

  case 261:
                        { yyval = yyvsp[0]; }
    break;

  case 262:
                        { yyval = yyvsp[0]; }
    break;

  case 263:
        { yyval . subtree = MakeTree ( ctx, PT_VIEW, T ( ctx, yyvsp[-8] ), P ( yyvsp[-7] ), T ( ctx, yyvsp[-6] ), P ( yyvsp[-5] ), T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 264:
                                { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 265:
                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 266:
                                { yyval . subtree = MakeTree ( ctx, PT_VIEWPARAM, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 267:
                { yyval = yyvsp[0]; }
    break;

  case 268:
                { yyval = yyvsp[0]; }
    break;

  case 269:
                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 270:
                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 271:
            { yyval . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 272:
            { yyval . subtree = MakeTree ( ctx, PT_COLUMN, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 273:
            { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 274:
                                { yyval = yyvsp[0]; }
    break;

  case 275:
                                { yyval . subtree = MakeTree ( ctx, PT_VIEWPARENTS, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 276:
                                   { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 277:
                                   { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 278:
        { yyval . subtree = MakeTree ( ctx, PT_VIEWPARENT, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 279:
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 280:
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
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
  *++yylsp = yyloc;

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
      yyerror (&yylloc, ctx, root, errors, sb, YY_("syntax error"));
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
        yyerror (&yylloc, ctx, root, errors, sb, yymsgp);
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
                      yytoken, &yylval, &yylloc, ctx, root, errors, sb);
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, ctx, root, errors, sb);
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
  yyerror (&yylloc, ctx, root, errors, sb, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, ctx, root, errors, sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp, yylsp, ctx, root, errors, sb);
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
