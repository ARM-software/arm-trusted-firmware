/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WIN_POSIX_H
#define WIN_POSIX_H

#define _CRT_SECURE_NO_WARNINGS

#include <direct.h>
#include <io.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "uuid.h"

/* Derive or provide Windows equivalents of Posix/GCC/Unix stuff. */
#ifndef PATH_MAX
# ifdef MAX_PATH
#  define PATH_MAX MAX_PATH
# else
#  ifdef _MAX_PATH
#   define MAX_PATH _MAX_PATH
#   define PATH_MAX _MAX_PATH
#  else
#   define PATH_MAX 260
#  endif
# endif
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS 1
#endif

/*
 * Platform specific names.
 *
 * Visual Studio deprecates a number of POSIX functions and only provides
 * ISO C++ compliant alternatives (distinguished by their '_' prefix).
 * These macros help provide a stopgap for that.
 */

/* fileno cannot be an inline function, because _fileno is a macro. */
#define fileno(fileptr) _fileno(fileptr)

/* _fstat uses the _stat structure, not stat. */
#define BLD_PLAT_STAT	_stat

/* Define flag values for _access. */
#define F_OK	0


/* getopt implementation for Windows: Data. */

/* Legitimate values for option.has_arg. */
enum has_arg_values {
	no_argument,		/* No argument value required */
	required_argument,	/* value must be specified. */
	optional_argument	/* value may be specified. */
};

/* Long option table entry for get_opt_long. */
struct option {
	/* The name of the option. */
	const char *name;

	/*
	 * Indicates whether the option takes an argument.
	 * Possible values: see has_arg_values above.
	 */
	int has_arg;

	/* If not null, when option present, *flag is set to val. */
	int *flag;

	/*
	 * The value associated with this option to return
	 * (and save in *flag when not null)
	 */
	int val;
};

/*
 * This variable is set by getopt to point at the value of the option
 * argument, for those options that accept arguments.
 */
extern char *optarg;

/*
 * When this variable is not zero, getopt emits an error message to stderr
 * if it encounters an unspecified option, or a missing argument.
 * Otherwise no message is reported.
 */
extern const int opterr;	/* const as NOT used in this implementation. */

/*
 * This variable is set by getopt to the index of the next element of the
 * argv array to be processed. Once getopt has found all of the option
 * arguments, you can use this variable to determine where the remaining
 * non-option arguments begin. The initial value of this variable is 1.
 */
extern int optind;

/*
 * When getopt encounters an unknown option character or an option with a
 * missing required argument, it stores that option character in this
 * variable.
 */
extern int optopt;


/*
 * Platform specific names.
 *
 * Visual Studio deprecates a number of POSIX functions and only provides
 * ISO C++ compliant alternatives (distinguished by their '_' prefix).
 * These inline functions provide a stopgap for that.
 */

inline int access(const char *path, int mode)
{
	return _access(path, mode);
}

inline int chdir(const char *s)
{
	return _chdir(s);
}

inline int fstat(int fd, struct _stat *buffer)
{
	return _fstat(fd, buffer);
}

inline char *strdup(const char *s)
{
	return _strdup(s);
}

/*
 * getopt implementation for Windows: Functions.
 *
 * Windows does not have the getopt family of functions, as it normally
 * uses '/' instead of '-' as the command line option delimiter.
 * These functions provide a Windows version that  uses '-', which precludes
 * using '-' as the intial letter of a program argument.
 * This is not seen as a problem in the specific instance of fiptool,
 * and enables existing makefiles to work on a Windows build environment.
 */

/*
 * The getopt function gets the next option argument from the argument list
 * specified by the argv and argc arguments.
 */
int getopt(int argc,
	   char *argv[],
	   char *options);

/*
 * getopt_long gets the next option argument from the argument list
 * specified by the argv and argc arguments.  Options may be either short
 * (single letter) as for getopt, or longer names (preceded by --).
 */
int getopt_long(int argc,
		char *argv[],
		const char *shortopts,
		const struct option *longopts,
		int *indexptr);

/*
 * getopt_long_only gets the next option argument from the argument list
 * specified by the argv and argc arguments.  Options may be either short
 * or long as for getopt_long, but the long names may have a single '-'
 * prefix, too.
 */
int getopt_long_only(int argc,
			   char *argv[],
			   const char *shortopts,
			   const struct option *longopts,
			   int *indexptr);

#endif /* WIN_POSIX_H */
