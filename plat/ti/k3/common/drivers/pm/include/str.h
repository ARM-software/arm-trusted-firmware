/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STR_H
#define STR_H

#include <string.h>
#include <limits.h>
#include <ctype.h>

/**
 * streq - Are two strings equal?
 * @a: first string
 * @b: first string
 *
 * This macro is arguably more readable than "!strcmp(a, b)".
 *
 * Example:
 *	if (streq(somestring, ""))
 *		printf("String is empty!\n");
 */
#define streq(a, b) (strcmp((a), (b)) == 0)

/**
 * strstarts - Does this string start with this prefix?
 * @str: string to test
 * @prefix: prefix to look for at start of str
 *
 * Example:
 *	if (strstarts(somestring, "foo"))
 *		printf("String %s begins with 'foo'!\n", somestring);
 */
#define strstarts(str, prefix) (strncmp((str), (prefix), strlen(prefix)) == 0)

/**
 * strends - Does this string end with this postfix?
 * @str: string to test
 * @postfix: postfix to look for at end of str
 *
 * Example:
 *	if (strends(somestring, "foo"))
 *		printf("String %s end with 'foo'!\n", somestring);
 */
static inline bool strends(const char *str, const char *postfix)
{
	bool ret;

	if (strlen(str) < strlen(postfix)) {
		ret = false;
	} else {
		ret = streq(str + strlen(str) - strlen(postfix), postfix);
	}
	return ret;
}

/**
 * stringify - Turn expression into a string literal
 * @expr: any C expression
 *
 * Example:
 *	#define PRINT_COND_IF_FALSE(cond) \
 *		((cond) || printf("%s is false!", stringify(cond)))
 */
#define stringify(expr)		stringify_1(expr)
/* Double-indirection required to stringify expansions */
#define stringify_1(expr)	#expr

/**
 * STR_MAX_CHARS - Maximum possible size of numeric string for this type.
 * @type_or_expr: a pointer or integer type or expression.
 *
 * This provides enough space for a nul-terminated string which represents the
 * largest possible value for the type or expression.
 *
 * Note: The implementation adds extra space so hex values or negative
 * values will fit (eg. sprintf(... "%p"). )
 *
 * Example:
 *	char str[STR_MAX_CHARS(int)];
 *
 *	sprintf(str, "%i", 7);
 */
#define STR_MAX_CHARS(type_or_expr)				\
	((sizeof(type_or_expr) * CHAR_BIT + 8) / 9 * 3 + 2	\
	 + STR_MAX_CHARS_TCHECK_(type_or_expr))

/* Only a simple type can have 0 assigned, so test that. */
#define STR_MAX_CHARS_TCHECK_(type_or_expr)		\
	({ typeof(type_or_expr)x = 0; (void) x; 0; })

#endif /* STR_H */
