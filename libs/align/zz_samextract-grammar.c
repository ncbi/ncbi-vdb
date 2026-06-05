/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         SAM_STYPE
/* Substitute the variable and function names.  */
#define yyparse         Sam_parse
#define yylex           Sam_lex
#define yyerror         Sam_error
#define yydebug         Sam_debug
#define yynerrs         Sam_nerrs
#define yylval          Sam_lval
#define yychar          Sam_char

/* First part of user prologue.  */

    #include "samextract.h"
    #include "samextract-pool.h"
    #include <align/samextract-lib.h>
    #include <klib/rc.h>
    int Sam_lex(SAMExtractor *);


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

#include "zz_samextract-grammar.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_HEADER = 3,                     /* HEADER  */
  YYSYMBOL_SEQUENCE = 4,                   /* SEQUENCE  */
  YYSYMBOL_READGROUP = 5,                  /* READGROUP  */
  YYSYMBOL_PROGRAM = 6,                    /* PROGRAM  */
  YYSYMBOL_COMMENT = 7,                    /* COMMENT  */
  YYSYMBOL_VALUE = 8,                      /* VALUE  */
  YYSYMBOL_QNAME = 9,                      /* QNAME  */
  YYSYMBOL_FLAG = 10,                      /* FLAG  */
  YYSYMBOL_RNAME = 11,                     /* RNAME  */
  YYSYMBOL_POS = 12,                       /* POS  */
  YYSYMBOL_MAPQ = 13,                      /* MAPQ  */
  YYSYMBOL_CIGAR = 14,                     /* CIGAR  */
  YYSYMBOL_RNEXT = 15,                     /* RNEXT  */
  YYSYMBOL_PNEXT = 16,                     /* PNEXT  */
  YYSYMBOL_TLEN = 17,                      /* TLEN  */
  YYSYMBOL_SEQ = 18,                       /* SEQ  */
  YYSYMBOL_QUAL = 19,                      /* QUAL  */
  YYSYMBOL_OPTTAG = 20,                    /* OPTTAG  */
  YYSYMBOL_OPTITAG = 21,                   /* OPTITAG  */
  YYSYMBOL_OPTZTAG = 22,                   /* OPTZTAG  */
  YYSYMBOL_OPTBTAG = 23,                   /* OPTBTAG  */
  YYSYMBOL_OPTATYPE = 24,                  /* OPTATYPE  */
  YYSYMBOL_OPTITYPE = 25,                  /* OPTITYPE  */
  YYSYMBOL_OPTFTYPE = 26,                  /* OPTFTYPE  */
  YYSYMBOL_OPTZTYPE = 27,                  /* OPTZTYPE  */
  YYSYMBOL_OPTHTYPE = 28,                  /* OPTHTYPE  */
  YYSYMBOL_OPTBTYPE = 29,                  /* OPTBTYPE  */
  YYSYMBOL_OPTAVALUE = 30,                 /* OPTAVALUE  */
  YYSYMBOL_OPTIVALUE = 31,                 /* OPTIVALUE  */
  YYSYMBOL_OPTFVALUE = 32,                 /* OPTFVALUE  */
  YYSYMBOL_OPTZVALUE = 33,                 /* OPTZVALUE  */
  YYSYMBOL_OPTHVALUE = 34,                 /* OPTHVALUE  */
  YYSYMBOL_OPTBVALUE = 35,                 /* OPTBVALUE  */
  YYSYMBOL_HDVN = 36,                      /* HDVN  */
  YYSYMBOL_HDSO = 37,                      /* HDSO  */
  YYSYMBOL_HDGO = 38,                      /* HDGO  */
  YYSYMBOL_RGID = 39,                      /* RGID  */
  YYSYMBOL_RGCN = 40,                      /* RGCN  */
  YYSYMBOL_RGDS = 41,                      /* RGDS  */
  YYSYMBOL_RGDT = 42,                      /* RGDT  */
  YYSYMBOL_RGFO = 43,                      /* RGFO  */
  YYSYMBOL_RGKS = 44,                      /* RGKS  */
  YYSYMBOL_RGLB = 45,                      /* RGLB  */
  YYSYMBOL_RGPG = 46,                      /* RGPG  */
  YYSYMBOL_RGPI = 47,                      /* RGPI  */
  YYSYMBOL_RGPL = 48,                      /* RGPL  */
  YYSYMBOL_RGPM = 49,                      /* RGPM  */
  YYSYMBOL_RGPU = 50,                      /* RGPU  */
  YYSYMBOL_RGSM = 51,                      /* RGSM  */
  YYSYMBOL_PGID = 52,                      /* PGID  */
  YYSYMBOL_PGPN = 53,                      /* PGPN  */
  YYSYMBOL_PGCL = 54,                      /* PGCL  */
  YYSYMBOL_PGPP = 55,                      /* PGPP  */
  YYSYMBOL_PGDS = 56,                      /* PGDS  */
  YYSYMBOL_PGVN = 57,                      /* PGVN  */
  YYSYMBOL_SQSN = 58,                      /* SQSN  */
  YYSYMBOL_SQLN = 59,                      /* SQLN  */
  YYSYMBOL_SQAS = 60,                      /* SQAS  */
  YYSYMBOL_SQM5 = 61,                      /* SQM5  */
  YYSYMBOL_SQSP = 62,                      /* SQSP  */
  YYSYMBOL_SQUR = 63,                      /* SQUR  */
  YYSYMBOL_TAB = 64,                       /* TAB  */
  YYSYMBOL_CONTROLCHAR = 65,               /* CONTROLCHAR  */
  YYSYMBOL_EOL = 66,                       /* EOL  */
  YYSYMBOL_YYACCEPT = 67,                  /* $accept  */
  YYSYMBOL_sam = 68,                       /* sam  */
  YYSYMBOL_line = 69,                      /* line  */
  YYSYMBOL_comment = 70,                   /* comment  */
  YYSYMBOL_header = 71,                    /* header  */
  YYSYMBOL_headerlist = 72,                /* headerlist  */
  YYSYMBOL_hdr = 73,                       /* hdr  */
  YYSYMBOL_sequence = 74,                  /* sequence  */
  YYSYMBOL_sequencelist = 75,              /* sequencelist  */
  YYSYMBOL_sq = 76,                        /* sq  */
  YYSYMBOL_program = 77,                   /* program  */
  YYSYMBOL_programlist = 78,               /* programlist  */
  YYSYMBOL_pg = 79,                        /* pg  */
  YYSYMBOL_readgroup = 80,                 /* readgroup  */
  YYSYMBOL_readgrouplist = 81,             /* readgrouplist  */
  YYSYMBOL_rg = 82,                        /* rg  */
  YYSYMBOL_alignment = 83,                 /* alignment  */
  YYSYMBOL_optlist = 84,                   /* optlist  */
  YYSYMBOL_opt = 85                        /* opt  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
typedef yytype_uint8 yy_state_t;

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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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

#if 1

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
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined SAM_STYPE_IS_TRIVIAL && SAM_STYPE_IS_TRIVIAL)))

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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   167

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  19
/* YYNRULES -- Number of rules.  */
#define YYNRULES  71
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  136

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   321


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      65,    66
};

#if SAM_DEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   136,   136,   137,   141,   142,   147,   148,   149,   150,
     151,   152,   156,   160,   178,   179,   182,   187,   191,   202,
     211,   229,   230,   234,   238,   248,   251,   256,   259,   262,
     269,   281,   282,   286,   290,   293,   296,   299,   302,   305,
     312,   325,   326,   329,   334,   353,   356,   359,   362,   367,
     370,   373,   376,   379,   382,   385,   388,   393,   397,   403,
     408,   415,   416,   420,   424,   428,   432,   436,   440,   444,
     448,   452
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "HEADER", "SEQUENCE",
  "READGROUP", "PROGRAM", "COMMENT", "VALUE", "QNAME", "FLAG", "RNAME",
  "POS", "MAPQ", "CIGAR", "RNEXT", "PNEXT", "TLEN", "SEQ", "QUAL",
  "OPTTAG", "OPTITAG", "OPTZTAG", "OPTBTAG", "OPTATYPE", "OPTITYPE",
  "OPTFTYPE", "OPTZTYPE", "OPTHTYPE", "OPTBTYPE", "OPTAVALUE", "OPTIVALUE",
  "OPTFVALUE", "OPTZVALUE", "OPTHVALUE", "OPTBVALUE", "HDVN", "HDSO",
  "HDGO", "RGID", "RGCN", "RGDS", "RGDT", "RGFO", "RGKS", "RGLB", "RGPG",
  "RGPI", "RGPL", "RGPM", "RGPU", "RGSM", "PGID", "PGPN", "PGCL", "PGPP",
  "PGDS", "PGVN", "SQSN", "SQLN", "SQAS", "SQM5", "SQSP", "SQUR", "TAB",
  "CONTROLCHAR", "EOL", "$accept", "sam", "line", "comment", "header",
  "headerlist", "hdr", "sequence", "sequencelist", "sq", "program",
  "programlist", "pg", "readgroup", "readgrouplist", "rg", "alignment",
  "optlist", "opt", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-46)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -46,     0,   -46,    -9,    44,    37,    15,   -46,    -2,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,     6,    12,
      14,   -46,   -12,   -46,    22,    23,    45,    50,    54,    55,
     -46,    31,   -46,    56,    65,    66,    67,    88,    90,    91,
      92,   107,   108,   109,   110,   111,   112,   -45,    -7,   -46,
     -46,   113,   114,   115,   116,   117,   118,    -6,   -46,   119,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,    61,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   -46,   120,
     -46,   121,   122,   123,   124,   125,   126,   127,   -10,    85,
     103,   102,   104,   -46,    -5,   -46,   101,   106,   128,   129,
     105,   100,   130,   131,   132,   -46,   -46,   -46,   -46,   -46,
     -46,   -46,   -46,   -46,   -46,   -46
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,    12,     0,     5,
       4,     3,     6,     7,     8,     9,    10,    11,     0,     0,
       0,    19,     0,    14,     0,     0,     0,     0,     0,     0,
      29,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    41,
      39,     0,     0,     0,     0,     0,     0,     0,    31,     0,
      16,    17,    18,    13,    15,    23,    24,    25,    26,    27,
      28,    20,    22,    56,    43,    45,    46,    47,    48,    49,
      50,    51,    52,    44,    53,    54,    55,     0,    58,    40,
      42,    33,    34,    35,    36,    37,    38,    30,    32,     0,
      57,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    59,     0,    61,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    60,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -46,   -46,   -46,   -46,   -46,   -46,   133,   -46,   -46,   134,
     -46,   -46,    84,   -46,   -46,    95,   -46,   -46,    33
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,    11,    12,    13,    22,    23,    14,    31,    32,
      15,    57,    58,    16,    48,    49,    17,   114,   115
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
       2,    33,    50,     3,     4,     5,     6,     7,    59,     8,
     109,   110,   111,   112,    60,   109,   110,   111,   112,    87,
      61,    88,    62,    50,    18,    19,    20,    18,    19,    20,
      65,    66,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    33,    51,    52,    53,    54,
      55,    56,    21,    67,    63,    21,   113,    47,    68,    89,
      97,   125,    69,    70,    73,     9,    10,    51,    52,    53,
      54,    55,    56,    74,    75,    76,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    24,
      25,    26,    27,    28,    29,    30,    77,    71,    78,    79,
      80,    47,    24,    25,    26,    27,    28,    29,    30,   116,
     117,   118,   119,   120,   121,    81,    82,    83,    84,    85,
      86,    91,    92,    93,    94,    95,    96,   100,   122,   123,
      99,   127,   101,   124,   102,   132,   103,   128,   104,   131,
     105,    98,   106,    90,   107,     0,   108,   126,     0,     0,
       0,     0,     0,     0,     0,    64,     0,     0,     0,     0,
     129,   133,   130,     0,   134,    72,     0,   135
};

static const yytype_int8 yycheck[] =
{
       0,     8,     8,     3,     4,     5,     6,     7,    10,     9,
      20,    21,    22,    23,     8,    20,    21,    22,    23,    64,
       8,    66,     8,     8,    36,    37,    38,    36,    37,    38,
       8,     8,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     8,    52,    53,    54,    55,
      56,    57,    64,     8,    66,    64,    66,    64,     8,    66,
      66,    66,     8,     8,     8,    65,    66,    52,    53,    54,
      55,    56,    57,     8,     8,     8,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    58,
      59,    60,    61,    62,    63,    64,     8,    66,     8,     8,
       8,    64,    58,    59,    60,    61,    62,    63,    64,    24,
      25,    26,    27,    28,    29,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,    66,    25,    27,
      11,    30,    12,    29,    13,    35,    14,    31,    15,    34,
      16,    57,    17,    48,    18,    -1,    19,   114,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    -1,
      32,    31,    33,    -1,    33,    31,    -1,    35
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    68,     0,     3,     4,     5,     6,     7,     9,    65,
      66,    69,    70,    71,    74,    77,    80,    83,    36,    37,
      38,    64,    72,    73,    58,    59,    60,    61,    62,    63,
      64,    75,    76,     8,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    64,    81,    82,
       8,    52,    53,    54,    55,    56,    57,    78,    79,    10,
       8,     8,     8,    66,    73,     8,     8,     8,     8,     8,
       8,    66,    76,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,    64,    66,    66,
      82,     8,     8,     8,     8,     8,     8,    66,    79,    11,
      66,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    66,    84,    85,    24,    25,    26,    27,
      28,    29,    25,    27,    29,    66,    85,    30,    31,    32,
      33,    34,    35,    31,    33,    35
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    67,    68,    68,    69,    69,    69,    69,    69,    69,
      69,    69,    70,    71,    72,    72,    73,    73,    73,    73,
      74,    75,    75,    76,    76,    76,    76,    76,    76,    76,
      77,    78,    78,    79,    79,    79,    79,    79,    79,    79,
      80,    81,    81,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    83,
      83,    84,    84,    85,    85,    85,    85,    85,    85,    85,
      85,    85
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     2,     2,     2,     1,
       3,     1,     2,     2,     2,     2,     2,     2,     2,     1,
       3,     1,     2,     2,     2,     2,     2,     2,     2,     1,
       3,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     3,     2,    12,
      13,     1,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = SAM_EMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == SAM_EMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (state, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use SAM_error or SAM_UNDEF. */
#define YYERRCODE SAM_UNDEF


/* Enable debugging if requested.  */
#if SAM_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, state); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, SAMExtractor * state)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (state);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, SAMExtractor * state)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, state);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, SAMExtractor * state)
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
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], state);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, state); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !SAM_DEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !SAM_DEBUG */


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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
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
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
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
# endif
#endif

#ifndef yytnamerr
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
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
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
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
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
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
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
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
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


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, SAMExtractor * state)
{
  YY_USE (yyvaluep);
  YY_USE (state);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (SAMExtractor * state)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = SAM_EMPTY; /* Cause a token to be read.  */

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
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == SAM_EMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (state);
    }

  if (yychar <= END)
    {
      yychar = END;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == SAM_error)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = SAM_UNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yychar = SAM_EMPTY;
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
  case 4: /* line: EOL  */
        { DBG("empty line"); }
    break;

  case 5: /* line: CONTROLCHAR  */
                 { ERR("CONTROLCHAR %d", (yyvsp[0].strval)[0]);
                   rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
                   state->rc=rc;
                   return END;
   }
    break;

  case 6: /* line: comment  */
             { DBG("comment"); }
    break;

  case 7: /* line: header  */
            { DBG("header done"); }
    break;

  case 8: /* line: sequence  */
              { DBG("sequence"); }
    break;

  case 9: /* line: program  */
             { DBG("program"); }
    break;

  case 10: /* line: readgroup  */
               { DBG("readgroup"); }
    break;

  case 11: /* line: alignment  */
               { DBG("alignment"); }
    break;

  case 12: /* comment: COMMENT  */
               { mark_headers(state,"CO"); }
    break;

  case 13: /* header: HEADER headerlist EOL  */
    {
        DBG("header list");
        if (!state->hashdvn)
        {
            ERR("VN tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        if (state->hashdso && state->hashdgo)
           WARN("Both SO and GO tags present");
        if (!state->hashdso && !state->hashdgo)
           WARN("neither SO or GO tags present");

        mark_headers(state,"HD"); }
    break;

  case 16: /* hdr: HDVN VALUE  */
                {
        state->hashdvn=true;
        if (process_header(state,"HD","VN",(yyvsp[0].strval))) return END;
        DBG("HDVN VALUE");
        }
    break;

  case 17: /* hdr: HDSO VALUE  */
                {
        state->hashdso=true;
        if (process_header(state,"HD","SO",(yyvsp[0].strval))) return END;
        }
    break;

  case 18: /* hdr: HDGO VALUE  */
                {
        state->hashdgo=true;
        if (process_header(state,"HD","GO",(yyvsp[0].strval))) return END;
        }
    break;

  case 19: /* hdr: TAB  */
        { ERR("empty HD tags");
        rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
        state->rc=rc;
        return END;}
    break;

  case 20: /* sequence: SEQUENCE sequencelist EOL  */
    {
        DBG("sequence");
        if (!state->hassqsn)
        {
            ERR("SN tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        if (!state->hassqln)
        {
            ERR("LN tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        mark_headers(state,"SQ"); }
    break;

  case 23: /* sq: SQSN VALUE  */
                 {
        state->hassqsn=true;
        if (process_header(state,"SQ",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        }
    break;

  case 24: /* sq: SQLN VALUE  */
                 {
        if (!inrange((yyvsp[0].strval),1,INT32_MAX))
        {
            ERR("SQ LN field not in range %s",(yyvsp[0].strval));
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        state->hassqln=true;
        if (process_header(state,"SQ",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        }
    break;

  case 25: /* sq: SQAS VALUE  */
                 {
        if (process_header(state,"SQ",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 26: /* sq: SQM5 VALUE  */
                 {
        if (!ismd5((yyvsp[0].strval)))
            WARN("M5 value not followed by MD5");
        if (process_header(state,"SQ",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 27: /* sq: SQSP VALUE  */
                 {
        if (process_header(state,"SQ",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 28: /* sq: SQUR VALUE  */
                 {
        if (process_header(state,"SQ",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 29: /* sq: TAB  */
          { ERR("Unexpected tab in sequence");
        rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
        state->rc=rc;
        return END;}
    break;

  case 30: /* program: PROGRAM programlist EOL  */
     {
        DBG("program");
        if (!state->haspgid)
        {
            ERR("ID tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }
        mark_headers(state,"PG"); }
    break;

  case 33: /* pg: PGID VALUE  */
                 {
        state->haspgid=true;
        if (process_header(state,"PG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 34: /* pg: PGPN VALUE  */
                 {
        if (process_header(state,"PG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 35: /* pg: PGCL VALUE  */
                 {
        if (process_header(state,"PG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 36: /* pg: PGPP VALUE  */
                 {
        if (process_header(state,"PG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 37: /* pg: PGDS VALUE  */
                 {
        if (process_header(state,"PG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 38: /* pg: PGVN VALUE  */
                 {
        if (process_header(state,"PG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 39: /* pg: VALUE  */
            {
        WARN("Bogus value in PG:%s",(yyvsp[0].strval));
         }
    break;

  case 40: /* readgroup: READGROUP readgrouplist EOL  */
    {
        DBG("readgroup ");
        if (!state->hasrgid)
        {
            ERR("ID tag not seen in header");
            rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
            state->rc=rc;
        }

        mark_headers(state,"RG"); }
    break;

  case 43: /* rg: RGID VALUE  */
                {
        state->hasrgid=true;
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 44: /* rg: RGPL VALUE  */
                {
        if (
            strcasecmp((yyvsp[0].strval),"illumina") && /* most frequent */
            strcmp((yyvsp[0].strval),"CAPILLARY") &&
            strcmp((yyvsp[0].strval),"LS454") &&
            strcasecmp((yyvsp[0].strval),"solid") &&
            strcmp((yyvsp[0].strval),"HELICOS") &&
            strcmp((yyvsp[0].strval),"IONTORRENT") &&
            strcmp((yyvsp[0].strval),"ONT") &&
            strcasecmp((yyvsp[0].strval),"pacbio") &&
            strcmp((yyvsp[0].strval),"\"Complete Genomics\"") && /* not compliant */
            strcmp((yyvsp[0].strval),"illumina Hiseq") && /* most frequent */
            strcmp((yyvsp[0].strval),"COMPLETEGENOMICS") && /* not compliant */
            strcmp((yyvsp[0].strval),"PacBio_SMRT") && /* not compliant */
            strcmp((yyvsp[0].strval),"PacBio_RS")  /* not compliant */
        )
            WARN("Invalid Platform %s", (yyvsp[0].strval));
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 45: /* rg: RGCN VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 46: /* rg: RGDS VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 47: /* rg: RGDT VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 48: /* rg: RGFO VALUE  */
                {
        if (!isfloworder((yyvsp[0].strval)))
            WARN("Flow order incorrec");
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 49: /* rg: RGKS VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 50: /* rg: RGLB VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 51: /* rg: RGPG VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 52: /* rg: RGPI VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 53: /* rg: RGPM VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 54: /* rg: RGPU VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 55: /* rg: RGSM VALUE  */
                {
        if (process_header(state,"RG",(yyvsp[-1].strval),(yyvsp[0].strval))) return END;
        ; }
    break;

  case 56: /* rg: VALUE VALUE  */
                 {
        WARN("Unknown readgroup (RG) tag:%s", (yyvsp[-1].strval));

        ;
        }
    break;

  case 57: /* rg: TAB TAB EOL  */
                 {
        ERR("empty RG tags");
        rc_t rc=RC(rcAlign,rcRow,rcParsing,rcData,rcInvalid);
        state->rc=rc; }
    break;

  case 58: /* rg: TAB EOL  */
             { WARN("empty tags"); }
    break;

  case 59: /* alignment: QNAME FLAG RNAME POS MAPQ CIGAR RNEXT PNEXT TLEN SEQ QUAL EOL  */
     {
        DBG("alignment record %s",(yyvsp[-11].strval));
        process_alignment(state,(yyvsp[-11].strval),(yyvsp[-10].strval),(yyvsp[-9].strval),(yyvsp[-8].strval),(yyvsp[-7].strval),(yyvsp[-6].strval),(yyvsp[-5].strval),(yyvsp[-4].strval),(yyvsp[-3].strval),(yyvsp[-2].strval),(yyvsp[-1].strval));
     }
    break;

  case 60: /* alignment: QNAME FLAG RNAME POS MAPQ CIGAR RNEXT PNEXT TLEN SEQ QUAL optlist EOL  */
     {
        DBG("alignment record with optional tags");
        process_alignment(state,(yyvsp[-12].strval),(yyvsp[-11].strval),(yyvsp[-10].strval),(yyvsp[-9].strval),(yyvsp[-8].strval),(yyvsp[-7].strval),(yyvsp[-6].strval),(yyvsp[-5].strval),(yyvsp[-4].strval),(yyvsp[-3].strval),(yyvsp[-2].strval));
     }
    break;

  case 61: /* optlist: opt  */
             { DBG("opt"); }
    break;

  case 62: /* optlist: optlist opt  */
                     { DBG(" opts"); }
    break;

  case 63: /* opt: OPTTAG OPTATYPE OPTAVALUE  */
    {
        DBG("?AA");
    }
    break;

  case 64: /* opt: OPTTAG OPTITYPE OPTIVALUE  */
    {
        DBG("?II");
    }
    break;

  case 65: /* opt: OPTTAG OPTFTYPE OPTFVALUE  */
    {
        DBG("?FF");
    }
    break;

  case 66: /* opt: OPTTAG OPTZTYPE OPTZVALUE  */
    {
        DBG("?ZZ");
    }
    break;

  case 67: /* opt: OPTTAG OPTHTYPE OPTHVALUE  */
    {
        DBG("?HH");
    }
    break;

  case 68: /* opt: OPTTAG OPTBTYPE OPTBVALUE  */
    {
        DBG("?BB");
    }
    break;

  case 69: /* opt: OPTITAG OPTITYPE OPTIVALUE  */
    {
        DBG("III");
    }
    break;

  case 70: /* opt: OPTZTAG OPTZTYPE OPTZVALUE  */
    {
        DBG("ZZZ");
    }
    break;

  case 71: /* opt: OPTBTAG OPTBTYPE OPTBVALUE  */
    {
        DBG("BBB");
    }
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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == SAM_EMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (state, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= END)
        {
          /* Return failure if at end of input.  */
          if (yychar == END)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, state);
          yychar = SAM_EMPTY;
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
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, state);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (state, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != SAM_EMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, state);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, state);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}



