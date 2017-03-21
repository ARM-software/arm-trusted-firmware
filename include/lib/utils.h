/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

/* Compute the number of elements in the given array */
#define ARRAY_SIZE(a)				\
	(sizeof(a) / sizeof((a)[0]))

#define IS_POWER_OF_TWO(x)			\
	(((x) & ((x) - 1)) == 0)

#define SIZE_FROM_LOG2_WORDS(n)		(4 << (n))

#define BIT(nr)				(1UL << (nr))

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
 * For those constants to be shared between C and other sources, apply a 'ull'
 * suffix to the argument only in C, to avoid undefined or unintended behaviour.
 *
 * The GNU assembler and linker do not support the 'ull' suffix (it causes the
 * build process to fail) therefore the suffix is omitted when used in linker
 * scripts and assembler files.
*/
#if defined(__LINKER__) || defined(__ASSEMBLY__)
# define ULL(_x)	(_x)
#else
# define ULL(_x)	(_x##ull)
#endif

/*
 * C code should be put in this part of the header to avoid breaking ASM files
 * or linker scripts including it.
 */
#if !(defined(__LINKER__) || defined(__ASSEMBLY__))

#include <types.h>

/*
 * Fill a region of normal memory of size "length" in bytes with zero bytes.
 *
 * WARNING: This function can only operate on normal memory. This means that
 *          the MMU must be enabled when using this function. Otherwise, use
 *          zeromem.
 */
void zero_normalmem(void *mem, u_register_t length);

/*
 * Fill a region of memory of size "length" in bytes with null bytes.
 *
 * Unlike zero_normalmem, this function has no restriction on the type of
 * memory targeted and can be used for any device memory as well as normal
 * memory. This function must be used instead of zero_normalmem when MMU is
 * disabled.
 *
 * NOTE: When data cache and MMU are enabled, prefer zero_normalmem for faster
 *       zeroing.
 */
void zeromem(void *mem, u_register_t length);
#endif /* !(defined(__LINKER__) || defined(__ASSEMBLY__)) */

#endif /* __UTILS_H__ */
