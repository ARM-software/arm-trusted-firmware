/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include "win_posix.h"

/*
 * This variable is set by getopt to the index of the next element of the
 * argv array to be processed. Once getopt has found all of the option
 * arguments, you can use this variable to determine where the remaining
 * non-option arguments begin. The initial value of this variable is 1.
 */
int optind = 1;

/*
 * If the value of this variable is nonzero, then getopt prints an error
 * message to the standard error stream if it encounters an unknown option
 * default character or an option with a missing required argument.
 * If you set this variable to zero, getopt does not print any messages,
 * but it still returns the character ? to indicate an error.
 */
const int opterr; /* = 0; */
/* const because we do not implement error printing.*/
/* Not initialised to conform with the coding standard. */

/*
 * When getopt encounters an unknown option character or an option with a
 * missing required argument, it stores that option character in this
 * variable.
 */
int optopt;	/* = 0; */

/*
 * This variable is set by getopt to point at the value of the option
 * argument, for those options that accept arguments.
 */
char *optarg;	/* = 0; */

enum return_flags {
	RET_ERROR = -1,
	RET_END_OPT_LIST = -1,
	RET_NO_PARAM = '?',
	RET_NO_PARAM2 = ':',
	RET_UNKNOWN_OPT = '?'
};

/*
 * Common initialisation on entry.
 */
static
void getopt_init(void)
{
	optarg = (char *)0;
	optopt = 0;
	/* optind may be zero with some POSIX uses.
	 * For our purposes we just change it to 1.
	 */
	if (optind == 0)
		optind = 1;
}

/*
 * Common handling for a single letter option.
 */
static
int getopt_1char(int argc,
		 char *const argv[],
		 const char *const opstring,
		 const int optchar)
{
	size_t nlen = (opstring == 0) ? 0 : strlen(opstring);
	size_t loptn;

	for (loptn = 0; loptn < nlen; loptn++) {
		if (optchar == opstring[loptn]) {
			if (opstring[loptn + 1] == ':') {
				/* Option has argument */
				if (optind < argc) {
					/* Found argument. */
					assert(argv != 0);
					optind++;
					optarg = argv[optind++];
					return optchar;
				}
				/* Missing argument. */
				if (opstring[loptn + 2] == ':') {
					/* OK if optional "x::". */
					optind++;
					return optchar;
				}
				/* Actual missing value. */
				optopt = optchar;
				return ((opstring[0] == ':')
					? RET_NO_PARAM2
					: RET_NO_PARAM);
			}
			/* No argument, just return option char */
			optind++;
			return optchar;
		}
	}
	/*
	 * If getopt finds an option character in argv that was not included in
	 * options, ... it returns '?' and sets the external variable optopt to
	 * the actual option character.
	 */
	optopt = optchar;
	return RET_UNKNOWN_OPT;
}

int getopt(int argc,
	   char *argv[],
	   char *opstring)
{
	int result = RET_END_OPT_LIST;
	size_t argn = 0;
	size_t nlen = strlen(opstring);

	getopt_init();
	/* If we have an argument left to play with */
	if ((argc > optind) && (argv != 0)) {
		const char *arg = (const char *)argv[optind];

		if ((arg != 0) && (arg[0] == '-'))
			result = getopt_1char(argc, argv, opstring, arg[1]);
	}

	return result;
}

/*
 * Match an argument value against an option name.
 * Note that we only match over the shorter length of the pair, to allow
 * for abbreviation or say --match=value
 * Long option names may be abbreviated if the abbreviation is unique or an
 * exact match for some defined option.
 * A long option may take a parameter, of the form --opt=param or --opt param.
*/
static
int optmatch(const char *argval, const char *optname)
{
	int result = 0;

	while ((result == 0) && (*optname != 0) && (*argval != 0))
		result = (*argval++) - (*optname++);
	return result;
}

/* Handling for a single long option. */
static
int getopt_1long(const int argc,
		 char *const argv[],
		 const struct option *const longopts,
		 const char *const optname,
		 int *const indexptr)
{
	int result = RET_UNKNOWN_OPT;
	size_t loptn = 0;

	while (longopts[loptn].name != 0) {
		if (optmatch(optname, longopts[loptn].name) == 0) {
			/* We found a match. */
			result = longopts[loptn].val;
			if (indexptr != 0)
				*indexptr = loptn;
			switch (longopts[loptn].has_arg) {
			case required_argument:
				if ((optind + 1) >= argc) {
					/* Missing argument. */
					optopt = result;
					return RET_NO_PARAM;
				}
				/* Fallthrough to get option value. */

			case optional_argument:
				if ((argc - optind) > 0) {
					/* Found argument. */
					optarg = argv[++optind];
				}
				/* Fallthrough to handle flag. */

			case no_argument:
				optind++;
				if (longopts[loptn].flag != 0) {
					*longopts[loptn].flag = result;
					result = 0;
				}
				break;

			}
			return result;
		}
		++loptn;
	}
	/*
	 * If getopt finds an option character in argv that was not included
	 * in options, ... it returns '?' and sets the external variable
	 * optopt to the actual option character.
	 */
	return RET_UNKNOWN_OPT;
}

/*
 * getopt_long gets the next option argument from the argument list
 * specified by the argv and argc arguments.  Options may be either short
 * (single letter) as for getopt, or longer names (preceded by --).
 */
int getopt_long(int argc,
		char *argv[],
		const char *shortopts,
		const struct option *longopts,
		int *indexptr)
{
	int result = RET_END_OPT_LIST;

	getopt_init();
	/* If we have an argument left to play with */
	if ((argc > optind) && (argv != 0)) {
		const char *arg = argv[optind];

		if ((arg != 0) && (arg[0] == '-')) {
			if (arg[1] == '-') {
				/* Looks like a long option. */
				result = getopt_1long(argc,
						      argv,
						      longopts,
						      &arg[2],
						      indexptr);
			} else {
				result = getopt_1char(argc,
						      argv,
						      shortopts,
						      arg[1]);
			}
		}
	}
	return result;
}

/*
 * getopt_long_only gets the next option argument from the argument list
 * specified by the argv and argc arguments.  Options may be either short
 * or long as for getopt_long, but the long names may have a single '-'
 * prefix too.
 */
int getopt_long_only(int argc,
		     char *argv[],
		     const char *shortopts,
		     const struct option *longopts,
		     int *indexptr)
{
	int result = RET_END_OPT_LIST;

	getopt_init();
	/* If we have an argument left to play with */
	if ((argc > optind) && (argv != 0)) {
		const char *arg = argv[optind];

		if ((arg != 0) && (arg[0] == '-')) {
			if (arg[1] == '-') {
				/* Looks like a long option. */
				result = getopt_1long(argc,
						      argv,
						      longopts,
						      &arg[2],
						      indexptr);
			} else {
				result = getopt_1long(argc,
						      argv,
						      longopts,
						      &arg[1],
						      indexptr);
				if (result == RET_UNKNOWN_OPT) {
					result = getopt_1char(argc,
							      argv,
							      shortopts,
							      arg[1]);
				}
			}
		}
	}
	return result;
}
