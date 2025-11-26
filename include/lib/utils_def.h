/*
 * Copyright (c) 2016-2025, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UTILS_DEF_H
#define UTILS_DEF_H

#include <export/lib/utils_def_exp.h>

/* Compute the number of elements in the given array */
#define ARRAY_SIZE(a)				\
	(sizeof(a) / sizeof((a)[0]))

#define IS_POWER_OF_TWO(x)			\
	(((x) & ((x) - 1)) == 0)

#define SIZE_FROM_LOG2_WORDS(n)		(U(4) << (n))

#if defined(__LINKER__) || defined(__ASSEMBLER__)
#define BIT_32(nr)			(U(1) << (nr))
#define BIT_64(nr)			(ULL(1) << (nr))
#else
#define BIT_32(nr)			(((uint32_t)(1U)) << (nr))
#define BIT_64(nr)			(((uint64_t)(1ULL)) << (nr))
#endif

#ifdef __aarch64__
#define BIT				BIT_64
#else
#define BIT				BIT_32
#endif

/*
 * Create a contiguous bitmask starting at bit position @low and ending at
 * position @high. For example
 * GENMASK_64(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#if defined(__LINKER__) || defined(__ASSEMBLER__)
#define GENMASK_32(high, low) \
	(((0xFFFFFFFF) << (low)) & (0xFFFFFFFF >> (32 - 1 - (high))))

#define GENMASK_64(high, low) \
	((~0 << (low)) & (~0 >> (64 - 1 - (high))))
#else
#define GENMASK_32(high, low) \
	((~UINT32_C(0) >> (32U - 1U - (high))) ^ ((BIT_32(low) - 1U)))

#define GENMASK_64(high, low) \
	((~UINT64_C(0) >> (64U - 1U - (high))) ^ ((BIT_64(low) - 1U)))
#endif

#ifdef __aarch64__
#define GENMASK				GENMASK_64
#else
#define GENMASK				GENMASK_32
#endif

/*
 * Similar to GENMASK_64 but uses a named register field to compute the mask.
 * For a register field REG_FIELD, the macros REG_FIELD_WIDTH and
 * REG_FIELD_SHIFT must be defined.
 */
#define MASK(regfield)							\
	((~0ULL >> (64ULL - (regfield##_WIDTH))) << (regfield##_SHIFT))

#define HI(addr)			(addr >> 32)
#define LO(addr)			(addr & 0xffffffff)

#define HI_64(addr)			(addr >> 64)
#define LO_64(addr)			(addr & 0xffffffffffffffff)

/**
 * EXTRACT_FIELD - Extracts a specific bit field from a value.
 *
 * @reg:      The input value containing the field.

 * @regfield: A bitmask representing the field. For a register field REG_FIELD,
 *            the macros REG_FIELD_WIDTH and REG_FIELD_SHIFT must be defined.

 * The result of this macro is the contents of the field right shifted to the
 * least significant bit positions, with the rest being zero.
 */
#define EXTRACT(regfield, reg) \
	(((reg) & MASK(regfield)) >> (regfield##_SHIFT))

#define UPDATE_REG_FIELD(regfield, reg, val) \
	do { \
		(reg) &= ~(MASK(regfield)); \
		(reg) |= ((uint64_t)(val) << (regfield##_SHIFT)); \
	} while (0)

/*
 * This variant of div_round_up can be used in macro definition but should not
 * be used in C code as the `div` parameter is evaluated twice.
 */
#define DIV_ROUND_UP_2EVAL(n, d)	(((n) + (d) - 1) / (d))

/* round `n` up to a multiple of `r` */
#define ROUND_UP_2EVAL(n, r)		((((n) + (r) - 1) / (r)) * (r))

#define div_round_up(val, div) __extension__ ({	\
	__typeof__(div) _div = (div);		\
	((val) + _div - (__typeof__(div)) 1) / _div;		\
})

#define MIN(x, y) __extension__ ({	\
	__typeof__(x) _x = (x);		\
	__typeof__(y) _y = (y);		\
	(void)(&_x == &_y);		\
	(_x < _y) ? _x : _y;		\
})

#define MAX(x, y) __extension__ ({	\
	__typeof__(x) _x = (x);		\
	__typeof__(y) _y = (y);		\
	(void)(&_x == &_y);		\
	(_x > _y) ? _x : _y;		\
})

#define CLAMP(x, min, max) __extension__ ({ \
	__typeof__(x) _x = (x); \
	__typeof__(min) _min = (min); \
	__typeof__(max) _max = (max); \
	(void)(&_x == &_min); \
	(void)(&_x == &_max); \
	((_x > _max) ? _max : ((_x < _min) ? _min : _x)); \
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
	((__typeof__(value))((boundary) - ((__typeof__(value))1U)))

#define round_up(value, boundary)		\
	((((value) - ((__typeof__(value))1U)) | round_boundary(value, boundary)) + ((__typeof__(value))1U))

#define round_down(value, boundary)		\
	((value) & ~round_boundary(value, boundary))

/* add operation together with checking whether the operation overflowed
 * The result is '*res',
 * return 0 on success and 1 on overflow
 */
#define add_overflow(a, b, res) __builtin_add_overflow((a), (b), (res))

/*
 * Round up a value to align with a given size and
 * check whether overflow happens.
 * The rounduped value is '*res',
 * return 0 on success and 1 on overflow
 */
#define round_up_overflow(v, size, res) (__extension__({ \
	typeof(res) __res = res; \
	typeof(*(__res)) __roundup_tmp = 0; \
	typeof(v) __roundup_mask = (typeof(v))(size) - 1; \
	\
	add_overflow((v), __roundup_mask, &__roundup_tmp) ? 1 : \
		(void)(*(__res) = __roundup_tmp & ~__roundup_mask), 0; \
}))

/*
 * Add a with b, then round up the result to align with a given size and
 * check whether overflow happens.
 * The rounduped value is '*res',
 * return 0 on success and 1 on overflow
 */
#define add_with_round_up_overflow(a, b, size, res) (__extension__({ \
	typeof(a) __a = (a); \
	typeof(__a) __add_res = 0; \
	\
	add_overflow((__a), (b), &__add_res) ? 1 : \
		round_up_overflow(__add_res, (size), (res)) ? 1 : 0; \
}))

/**
 * Helper macro to ensure a value lies on a given boundary.
 */
#define is_aligned(value, boundary)			\
	(round_up((uintptr_t) value, boundary) ==	\
	 round_down((uintptr_t) value, boundary))

/*
 * Evaluates to 1 if (ptr + inc) overflows, 0 otherwise.
 * Both arguments must be unsigned pointer values (i.e. uintptr_t).
 */
#define check_uptr_overflow(_ptr, _inc)		\
	((_ptr) > (UINTPTR_MAX - (_inc)))

/*
 * Evaluates to 1 if (u32 + inc) overflows, 0 otherwise.
 * Both arguments must be 32-bit unsigned integers (i.e. effectively uint32_t).
 */
#define check_u32_overflow(_u32, _inc) \
	((_u32) > (UINT32_MAX - (_inc)))

/* Register size of the current architecture. */
#ifdef __aarch64__
#define REGSZ		U(8)
#else
#define REGSZ		U(4)
#endif

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

#define COMPILER_BARRIER() __asm__ volatile ("" ::: "memory")

/* Compiler builtin of GCC >= 9 and planned in llvm */
#ifdef __HAVE_SPECULATION_SAFE_VALUE
# define SPECULATION_SAFE_VALUE(var) __builtin_speculation_safe_value(var)
#else
# define SPECULATION_SAFE_VALUE(var) var
#endif

/*
 * Ticks elapsed in one second with a signal of 1 MHz
 */
#define MHZ_TICKS_PER_SEC	U(1000000)

/*
 * Ticks elapsed in one second with a signal of 1 KHz
 */
#define KHZ_TICKS_PER_SEC U(1000)

#endif /* UTILS_DEF_H */
