/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef BUILD_ASSERT_H
#define BUILD_ASSERT_H

/**
 * BUILD_ASSERT - assert a build-time dependency.
 * @cond: the compile-time condition which must be true.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can only be used within a function.
 *
 * Example:
 *	#include <stddef.h>
 *	...
 *	static char *foo_to_char(struct foo *foo)
 *	{
 *		// This code needs string to be at start of foo.
 *		BUILD_ASSERT(offsetof(struct foo, string) == 0);
 *		return (char *)foo;
 *	}
 */
#define BUILD_ASSERT(cond)						\
	do { (void) sizeof(char [1 - (2 * !(cond))]); } while (0)

/**
 * BUILD_ASSERT_OR_ZERO - assert a build-time dependency, as an expression.
 * @cond: the compile-time condition which must be true.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can be used in an expression: its value is "0".
 *
 * Example:
 *	#define foo_to_char(foo)					\
 *		 ((char *)(foo)						\
 *		  + BUILD_ASSERT_OR_ZERO(offsetof(struct foo, string) == 0))
 */
#define BUILD_ASSERT_OR_ZERO(cond)				\
	(int) (sizeof(char [1 - (2 * (!(cond) ? 1 : 0))]) - 1U)

/**
 * BUILD_ASSERT_GLOBAL - assert a build-time dependency, as a global.
 * @cond: the compile-time condition which must be true.
 * @name: A compile unit unique name for this condition.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can be used anywhere a global variable can be
 * declared. The global variable gets placed in a section to be discarded by
 * the linker.
 *
 * Example:
 *	BUILD_ASSERT_GLOBAL(offsetof(struct foo, string) == 0, foo_str_assert);
 */
#define BUILD_ASSERT_GLOBAL(cond, name)					\
	static const char name __section(".discard.asserts") __attribute__((unused, )) \
		= sizeof(char [1 - (2 * !(cond))]);
#endif /* BUILD_ASSERT_H */
