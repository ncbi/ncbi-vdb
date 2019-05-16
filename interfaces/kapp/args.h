/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _h_kapp_args_
#define _h_kapp_args_

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* this define is while adding the --option-file to standard options
 * and it might not be right yet */
#define USE_OPTFILE    1

/*
 * Terminology for this module:
 *
 * On the command line all things typed in are arguments
 * Those arguments that are string preceded by "-" or "--" are options
 * The -- preceded version of an Option is that Option's name.  A name
 * can have one or more aliases that are a single character.
 * The arguments that are not are parameters.
 *
 * This module treats all "strings" as if they were made of UTF-8 characters
 * that can be one or more bytes long.  An alias is a single Unicode character
 * in UTF-8 format.  7-bit ASCII is a true subset of UTF-8.  8-but ASCII might
 * not work.
 */

/*--------------------------------------------------------------------------
 * Args
 *  opaque class to build up option lists and parse the command line argc/argv
 *  not reference counted
 */
typedef struct Args Args;

/* ==========
 * Structure to define a command line option
 *
 * these are fed in one by one or through arrays to build up the
 * tables used to parse the caommand line argc/argv
 */

typedef void (CC * WhackParamFnP) (void * object);
typedef rc_t (CC * ConvertParamFnP) (const Args * self, uint32_t arg_index, const char * arg, size_t arg_len, void ** result, WhackParamFnP * whack);
    
typedef struct OptDef
{
    const char *  name;           	/* UTF8/ASCII NUL terminated long name */
    const char *  aliases;        	/* UTF8/ASCII NUL terminated set of single utf8/ASCII character names: may be NULL or "" */
    void (CC *    help_gen) (const char **);	/* function to generate help string */
    const char ** help;                	/* help-gen can treat these as non-const */
    uint16_t      max_count;      	/* maximum allowed; 0 is unlimited */
#define OPT_UNLIM 0
    bool          needs_value;    	/* does this require an argument value? */
    bool          required;             /* is this a required parameter?  Not supported yet. */
    ConvertParamFnP convert_fn;   /* function to convert option. can perform binary conversions. may be NULL */
} OptDef;
    
typedef struct ParamDef ParamDef;
struct ParamDef
{
    ConvertParamFnP convert_fn; /* function to convert option. can perform binary conversions. may be NULL */
};

extern OptDef StandardOptions [];

#define ALIAS_DEBUG     "+"
#define ALIAS_LOG_LEVEL "L"
#define ALIAS_HELP      "h?"
#define ALIAS_HELP1     "h"
#define ALIAS_VERSION   "V"
#define ALIAS_VERBOSE   "v"
#define ALIAS_QUIET     "q"
#if USE_OPTFILE
#define ALIAS_OPTFILE   ""
#endif

#define OPTION_DEBUG     "debug"
#define OPTION_LOG_LEVEL "log-level"
#define OPTION_HELP      "help"
#define OPTION_VERSION   "version"
#define OPTION_VERBOSE   "verbose"
#define OPTION_QUIET     "quiet"
#if USE_OPTFILE
#define OPTION_OPTFILE   "option-file"
#endif
#define OPTION_NO_USER_SETTINGS "no-user-settings"

#define ALIAS_REPORT    ""
#define OPTION_REPORT   "ncbi_error_report"

/* Make
 *  create the empty object
 */
rc_t CC ArgsMake ( Args ** pself );


/* Whack
 *  undo all object and owned object construction
 */
rc_t CC ArgsWhack ( Args * self );

#ifndef ArgsRelease
#define ArgsRelease(self) ArgsWhack(self)
#endif


/* AddOption
 *  takes the OptDef structure to add an option
 * AddLongOption
 *  splits the OptDef into simple parameters
 *
 *  "opt_short_names" [ IN, NULL OKAY ] - an optional list of single-characters used as
 *  aliases for the option.
 *
 *  "long_name" [ IN ] - a required long option name
 *
 *  "opt_param_names [ IN, NULL OKAY ] - an optional list of option parameter names
 *  currently limited to a single name. when not NULL/empty, implies that the option
 *  must have a parameter.
 *
 *  "help_text" [ IN ] - text for generating help info.
 *
 *  "max_count" [ IN, ZERO => INFINITE ] - sets an upper limit on the number of times
 *  the option can be specified on cmdline. the special value "0" is taken to mean
 *  as many times as the system will support.
 *
 *  "required" [ IN ] - when true, the option must be specified at least once.
 *  when false, the option is truly optional.
 */
rc_t CC ArgsAddOption ( Args * self, const OptDef * option );
rc_t CC ArgsAddLongOption ( Args * self, const char * opt_short_names, const char * long_name,
    const char * opt_param_names, const char * help_text, uint32_t max_count, bool required );


/* AddOptionArray
 *  helper function to call the ArgsAddOption() multiple times
 */
rc_t CC ArgsAddOptionArray ( Args * self, const OptDef * option, uint32_t count
#if ADD_SOMETIME_LATER
    , rc_t ( CC * header_fmt )( Args * args, const char * header ), const char * header
#endif
    );

/* AddParam
 *  adds a slot for a known parameter
 * AddLongParam
 *  adds a slot for a cmdline parameter along with some params of its own
 *
 *  "param_name" [ IN ] - for help display.
 *
 *  "help_text" [ IN ] - for help display.
 *
 *  "opt_cvt" [ IN, NULL OKAY ] - optional parameter conversion function
 */
rc_t CC ArgsAddParam ( Args * self, const ParamDef * param_def );
rc_t CC ArgsAddLongParam ( Args * self, const char * param_name, const char * help_text, ConvertParamFnP opt_cvt );

/* ArgsAddParamsArray
 *  adds parameter definitions for arguments parsing
 */
rc_t CC ArgsAddParamArray ( Args * self, const ParamDef * param, uint32_t count );

/* AddStandardOptions
 *  helper macro to add the array of internally defined
 *  "standard" options that we want all programs to support
 */
rc_t CC ArgsAddStandardOptions ( Args * self );


/* Parse
 *  parse the argc/argv as presented to KMain using the Args structure as built up
 */
rc_t CC ArgsParse ( Args * self, int argc, char *argv[] );


/* tokenizes a file into an user supplied argv array ( not the one from main() ! )
 * the result can be passed into ArgsParse(), enables commandline-options from a file
 *  caller has to free the created array via Args_free_token_argv()
 */
rc_t CC Args_tokenize_file_into_argv( const char * filename, int * argc, char *** argv );

rc_t CC Args_tokenize_file_and_progname_into_argv( const char * filename, const char * progname,
                                                   int * argc, char *** argv );

/* free's the array that was created by calling Args_tokenize_file_into_argv()
 */
void CC Args_free_token_argv( int argc, char * argv[] );


/* looks in args for file_option(s), if found loades the files, parses them
   if this results in more files to be parsed ( kind of includes ),
   the parsing continues recursivly
 */
rc_t CC Args_parse_inf_file( Args * args, const char * file_option );


/* looks in the unparsed original argv for the value of an option
   >>> this is a hack to enable special treatment for tools that do not use
       the standard args-parsing <<<
 */
rc_t CC Args_find_option_in_argv( int argc, char * argv[],
                                  const char * option_name,
                                  char * option, size_t option_len );

/* OptionCount
 *  how many times did this Option occur?
 */
rc_t CC ArgsOptionCount ( const Args * self, const char * option_name, uint32_t * count );


/* OptionValue
 *  what was the Nth value seen for this option?
 *  use OptionCount to know how many were seen.
 */
rc_t CC ArgsOptionValue ( const Args * self, const char * option_name,
    uint32_t iteration, const void ** value );

/* OptionValueExt
 *  What was the Nth value seen for this option?
 *  Use OptionCount to know how many were seen.
 *  Was this option called by long or short name (alias)?
 */
rc_t CC ArgsOptionValueExt ( const Args * self, const char * option_name,
    uint32_t iteration, const void ** value, bool * called_as_alias );

/*
 * ParamCount
 *  How many Parameters were seen?
 */
rc_t CC ArgsParamCount (const Args * self, uint32_t * count);

/*
 * ParamValue
 *  What was the Nth parameter seen?  Use ParamCount to know how many
 *  were seen.
 */
rc_t CC ArgsParamValue (const Args * self, uint32_t iteration, const void ** value);


/*
 * ArgvCount
 *  This is the original main() argument count (argc or ac)
 */
rc_t CC ArgsArgvCount (const Args * self, uint32_t * count);
rc_t CC ArgsArgc (const Args * self, uint32_t * count);
#define ArgsArgc ArgsArgvCount


/*
 * ArgvValue
 *  What was the Nth parameter seen?  Use ArgvCount to know how many
 *  were seen.
 *
 * The 0th value will as with the original argc.argv c system be the program name
 * as given to us by the O/S and c start up code.
 */
rc_t CC ArgsArgvValue (const Args * self, uint32_t iteration, const char ** value_string);



/* Utility functions to do common combinations and activities */

/*
 * MakeStandardOptions
 *  Calls both Make() and AddStandardOptions()
 *
 * This is probably the first thing to do in KMain(), then add other Options
 * via OptDef arracys and structures.  Then call parse.
 */
rc_t CC ArgsMakeStandardOptions (Args** pself);

rc_t CC ArgsHandleHelp (Args * self);
rc_t CC ArgsHandleVersion (Args * self);
rc_t CC ArgsHandleOptfile (Args * self);

/*
 * ArgsHandleLogLevel
 *  calls OptionCount and OptionValue to get parameters for the log-level
 *  option then uses that/those values to set the Log level for the log module
 */
rc_t CC ArgsHandleLogLevel (const Args * self);


rc_t CC ArgsHandleStatusLevel (const Args * self);
rc_t CC ArgsHandleDebug (const Args * self);
rc_t CC ArgsHandleStandardOptions (Args * self);

rc_t CC ArgsMakeAndHandle (Args ** pself, int argc, char ** argv, uint32_t table_count, ...);

/* the same as ArgsMakeAndHandle but also accepts params definitions */
rc_t CC ArgsMakeAndHandle2 (Args ** pself, int argc, char ** argv,
                            ParamDef * params, uint32_t param_count, uint32_t table_count, ...);

rc_t CC ArgsOptionSingleString (const Args * self, const char * option, const char ** value);

/* either fullpath or progname can be NULL, args can not */
rc_t CC ArgsProgram (const Args * args, const char ** fullpath, const char ** progname);


/* after arguments are parsed; check to see if any required arguments are missing */
rc_t CC ArgsCheckRequired (Args * args);


/*
 * Help Interface:
 * This interface is an extention os the Args Interface that is used for
 * generating usage outputs for a command line program,  This interface is provided
 * to standardize the look of these usage messages.
 */

/*
 * This function is defined per program and is the meat of the output
 * the the short-form usage and the first part of the long-form usage
 *
 * It should look something like this:
 *
 *
 *    rc_t UsageSummary (const char * progname)
 *    {
 *        return KOutMsg ("\n"
 *                        "Usage:\n"
 *                        "  %s [Options] [Parameters]\n"
 *                        "\n"
 *                        "Summary:\n"
 *                        "  Does something incredibly useful or we wouldn't have written it.\n"
 *                        "  I mean very very useful like pre-slicomg bread.\n",
 *                        "\n", progname);
 *    }
 *
 * More than one example line can be present if desired.
 */
rc_t CC UsageSummary (const char * prog_name);

/*
 * A program should define this which will be used only of the actual
 * program name as called is somehow irretrievable
 */
extern const char UsageDefaultName[];


/*
 * Version
 *   Generate the output for the -V or --version options.
 *   const char * fullpath:  The full argv[0] name for the program
 *   ver_t version:          the version for this program
 */
void CC HelpVersion (const char * fullpath, ver_t version);

void CC HelpOptionLine(const char * alias, const char * option, const char * param, const char ** msgs);

void CC HelpParamLine (const char * param, const char * const * msgs);

/*
 * OptionsStandard
 *   output the option lines for the standard options
 */
void CC HelpOptionsStandard (void);


/*
 * This Macro creates a default short form usage output typically
 * used when no options/parameters are given for a program
 *
 * It requires 'void summary (const char * program_name)' that is also
 * used in 'rc_t Usage (const Args* args)' that is the usage function
 * called when -? -h or --help is given as an option on the command line
 */

rc_t CC MiniUsage ( const Args * args );
rc_t CC Usage ( const Args * args );


uint32_t CC ArgsGetGlobalTries(bool *isSet);

bool CC Is32BitAndDisplayMessage( void );


/*
 * VDB-3772 - all dumpers suppose to support append mode output
 * so it is flag <--append|-A>
 * We use it as non-documented option
 * To add 'append' option, user should call ArgsAddAppendModeOption ()
 * fucntion
 */
#define OPTION_APPEND_OUTPUT    "append_output"

/*
 * Adding append mode option to arguments
 */
rc_t CC ArgsAddAppendModeOption ( Args * self );

/*
 * Handling append mode option from argument
 */
rc_t CC ArgsHandleAppendMode ( const Args * self );

/*
 * Checking/Setting ( if ) append mode ... lool
 */
bool CC ArgsIsAppendModeSet ( void );
void CC ArgsAppendModeSet ( bool AppendMode );

#ifdef __cplusplus
}
#endif

#endif /* _h_kapp_args_ */
