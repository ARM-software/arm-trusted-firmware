/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __UTILS_DEF_H__
#define __UTILS_DEF_H__

/* Compute the number of elements in the given array */
#define ARRAY_SIZE(a)				\
	(sizeof(a) / sizeof((a)[0]))

#define IS_POWER_OF_TWO(x)			\
	(((x) & ((x) - 1)) == 0)

#define SIZE_FROM_LOG2_WORDS(n)		(4 << (n))

#define BIT(nr)				(ULL(1) << (nr))

/*
 * This variant of div_round_up can be used in macro definition but should not
 * be used in C code as the `div` parameter is evaluated twice.
 */
#define DIV_ROUND_UP_2EVAL(n, d)	(((n) + (d) - 1) / (d))

#define div_round_up(val, div) __extension__ ({	\
	__typeof__(div) _div = (div);		\
	((val) + _div - 1) / _div;		\
})

#define MIN(x, y) __extension__ ({	\
	__typeof__(x) _x = (x);		\
	__typeof__(y) _y = (y);		\
	(void)(&_x == &_y);		\
	_x < _y ? _x : _y;		\
})

#define MAX(x, y) __extension__ ({	\
	__typeof__(x) _x = (x);		\
	__typeof__(y) _y = (y);		\
	(void)(&_x == &_y);		\
	_x > _y ? _x : _y;		\
})

/*
 * The round_up() macro rounds up a value to the given boundary in a
 * type-agnostic yet type-safe manner. The boundary must be a power of two.
 * In other words, it computes the smallest multiple of boundary which is
 * greater than or equal to value.
 *
 * round_down() is similar but rounds the value down instead.
 */
#define round_boundary(value, boundary)		\
	((__typeof__(value))((boundary) - 1))

#define round_up(value, boundary)		\
	((((value) - 1) | round_boundary(value, boundary)) + 1)

#define round_down(value, boundary)		\
	((value) & ~round_boundary(value, boundary))

/*
 * Evaluates to 1 if (ptr + inc) overflows, 0 otherwise.
 * Both arguments must be unsigned pointer values (i.e. uintptr_t).
 */
#define check_uptr_overflow(ptr, inc)		\
	(((ptr) > UINTPTR_MAX - (inc)) ? 1 : 0)

/*
 * Evaluates to 1 if (u32 + inc) overflows, 0 otherwise.
 * Both arguments must be 32-bit unsigned integers (i.e. effectively uint32_t).
 */
#define check_u32_overflow(u32, inc) \
	((u32) > (UINT32_MAX - (inc)) ? 1 : 0)

/*
 * For those constants to be shared between C and other sources, apply a 'u'
 * or 'ull' suffix to the argument only in C, to avoid undefined or unintended
 * behaviour.
 *
 * The GNU assembler and linker do not support the 'u' and 'ull' suffix (it
 * causes the build process to fail) therefore the suffix is omitted when used
 * in linker scripts and assembler files.
*/
#if defined(__LINKER__) || defined(__ASSEMBLY__)
# define  U(_x)		(_x)
# define ULL(_x)	(_x)
#else
# define  U(_x)		(_x##U)
# define ULL(_x)	(_x##ULL)
#endif

/* Register size of the current architecture. */
#ifdef AARCH32
#define REGSZ		U(4)
#else
#define REGSZ		U(8)
#endif

/*
 * Test for the current architecture version to be at least the version
 * expected.
 */
#define ARM_ARCH_AT_LEAST(_maj, _min) \
	((ARM_ARCH_MAJOR > _maj) || \
	 ((ARM_ARCH_MAJOR == _maj) && (ARM_ARCH_MINOR >= _min)))

/*
 * Import an assembly or linker symbol as a C expression with the specified
 * type
 */
#define IMPORT_SYM(type, sym, name) \
	extern char sym[];\
	static const __attribute__((unused)) type name = (type) sym;

/*
 * When the symbol is used to hold a pointer, its alignment can be asserted
 * with this macro. For example, if there is a linker symbol that is going to
 * be used as a 64-bit pointer, the value of the linker symbol must also be
 * aligned to 64 bit. This macro makes sure this is the case.
 */
#define ASSERT_SYM_PTR_ALIGN(sym) assert(((size_t)(sym) % __alignof__(*(sym))) == 0)


#endif /* __UTILS_DEF_H__ */
