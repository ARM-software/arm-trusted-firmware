/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef COMPILER_H
#define COMPILER_H

#include <cdefs.h>

#ifndef COLD
/**
 * COLD - a function is unlikely to be called.
 *
 * Used to mark an unlikely code path and optimize appropriately.
 * It is usually used on logging or error routines.
 *
 * Example:
 * static void COLD moan(const char *reason)
 * {
 *	fprintf(stderr, "Error: %s (%s)\n", reason, strerror(errno));
 * }
 */
#define COLD __cold
#endif

#ifndef NORETURN
/**
 * NORETURN - a function does not return
 *
 * Used to mark a function which exits; useful for suppressing warnings.
 *
 * Example:
 * static void NORETURN fail(const char *reason)
 * {
 *	fprintf(stderr, "Error: %s (%s)\n", reason, strerror(errno));
 *	exit(1);
 * }
 */
#define NORETURN __noreturn
#endif

#ifndef PRINTF_FMT
/**
 * PRINTF_FMT - a function takes printf-style arguments
 * @nfmt: the 1-based number of the function's format argument.
 * @narg: the 1-based number of the function's first variable argument.
 *
 * This allows the compiler to check your parameters as it does for printf().
 *
 * Example:
 * void PRINTF_FMT(2,3) my_printf(const char *prefix, const char *fmt, ...);
 */
#define PRINTF_FMT(nfmt, narg)				\
	____printf__(nfmt, narg)
#endif

#ifndef CONST_FUNCTION
/**
 * CONST_FUNCTION - a function's return depends only on its argument
 *
 * This allows the compiler to assume that the function will return the exact
 * same value for the exact same arguments.  This implies that the function
 * must not use global variables, or dereference pointer arguments.
 */
#define CONST_FUNCTION __attribute__((const))
#endif

#ifndef UNNEEDED
/**
 * UNNEEDED - a variable/function may not be needed
 *
 * This suppresses warnings about unused variables or functions, but tells
 * the compiler that if it is unused it need not emit it into the source code.
 *
 * Example:
 * // With some preprocessor options, this is unnecessary.
 * static UNNEEDED int32_t counter;
 *
 * // With some preprocessor options, this is unnecessary.
 * static UNNEEDED void add_to_counter(int32_t add)
 * {
 *	counter += add;
 * }
 */
#define UNNEEDED __maybe_unused
#endif

#ifndef NEEDED
/**
 * NEEDED - a variable/function is needed
 *
 * This suppresses warnings about unused variables or functions, but tells
 * the compiler that it must exist even if it (seems) unused.
 *
 * Example:
 *	// Even if this is unused, these are vital for debugging.
 *	static NEEDED int32_t counter;
 *	static NEEDED void dump_counter(void)
 *	{
 *		printf("Counter is %i\n", counter);
 *	}
 */
#define NEEDED __used
#endif

#ifndef UNUSED
/**
 * UNUSED - a parameter is unused
 *
 * Some compilers (eg. gcc with -W or -Wunused) warn about unused
 * function parameters.	 This suppresses such warnings and indicates
 * to the reader that it's deliberate.
 *
 * Example:
 *	// This is used as a callback, so needs to have this prototype.
 *	static int32_t some_callback(void *unused UNUSED)
 *	{
 *		return 0;
 *	}
 */
#define UNUSED __maybe_unused
#endif

#ifndef IS_COMPILE_CONSTANT
/**
 * IS_COMPILE_CONSTANT - does the compiler know the value of this expression?
 * @expr: the expression to evaluate
 *
 * When an expression manipulation is complicated, it is usually better to
 * implement it in a function.	However, if the expression being manipulated is
 * known at compile time, it is better to have the compiler see the entire
 * expression so it can simply substitute the result.
 *
 * This can be done using the IS_COMPILE_CONSTANT() macro.
 *
 * Example:
 *	enum greek { ALPHA, BETA, GAMMA, DELTA, EPSILON };
 *
 *	// Out-of-line version.
 *	const char *greek_name(enum greek greek);
 *
 *	// Inline version.
 *	static inline const char *_greek_name(enum greek greek)
 *	{
 *		switch (greek) {
 *		case ALPHA: return "alpha";
 *		case BETA: return "beta";
 *		case GAMMA: return "gamma";
 *		case DELTA: return "delta";
 *		case EPSILON: return "epsilon";
 *		default: return "**INVALID**";
 *		}
 *	}
 *
 *	// Use inline if compiler knows answer.	 Otherwise call function
 *	// to avoid copies of the same code everywhere.
 *	#define greek_name(g)						\
 *		 (IS_COMPILE_CONSTANT(greek) ? _greek_name(g) : greek_name(g))
 */
#define IS_COMPILE_CONSTANT(expr) __builtin_constant_p(expr)
#endif

#ifndef WARN_UNUSED_RESULT
/**
 * WARN_UNUSED_RESULT - warn if a function return value is unused.
 *
 * Used to mark a function where it is extremely unlikely that the caller
 * can ignore the result, eg realloc().
 *
 * Example:
 * // buf param may be freed by this; need return value!
 * static char *WARN_UNUSED_RESULT enlarge(char *buf, unsigned *size)
 * {
 *	return realloc(buf, (*size) *= 2);
 * }
 */
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#endif

#endif /* COMPILER_H */
