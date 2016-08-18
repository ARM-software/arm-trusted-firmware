/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#ifndef __ASM_MACROS_COMMON_S__
#define __ASM_MACROS_COMMON_S__

	/*
	 * This macro is used to create a function label and place the
	 * code into a separate text section based on the function name
	 * to enable elimination of unused code during linking
	 */
	.macro func _name
	.section .text.\_name, "ax"
	.type \_name, %function
	.func \_name
	\_name:
	.endm

	/*
	 * This macro is used to mark the end of a function.
	 */
	.macro endfunc _name
	.endfunc
	.size \_name, . - \_name
	.endm

	/*
	 * Theses macros are used to create function labels for deprecated
	 * APIs. If ERROR_DEPRECATED is non zero, the callers of these APIs
	 * will fail to link and cause build failure.
	 */
#if ERROR_DEPRECATED
	.macro func_deprecated _name
	func deprecated\_name
	.endm

	.macro endfunc_deprecated _name
	endfunc deprecated\_name
	.endm
#else
	.macro func_deprecated _name
	func \_name
	.endm

	.macro endfunc_deprecated _name
	endfunc \_name
	.endm
#endif

	/*
	 * Helper assembler macro to count trailing zeros. The output is
	 * populated in the `TZ_COUNT` symbol.
	 */
	.macro count_tz _value, _tz_count
	.if \_value
	  count_tz "(\_value >> 1)", "(\_tz_count + 1)"
	.else
	  .equ TZ_COUNT, (\_tz_count - 1)
	.endif
	.endm

	/*
	 * This macro declares an array of 1 or more stacks, properly
	 * aligned and in the requested section
	 */
#define DEFAULT_STACK_ALIGN	(1 << 6)   /* In case the caller doesnt provide alignment */

	.macro declare_stack _name, _section, _size, _count, _align=DEFAULT_STACK_ALIGN
	count_tz \_align, 0
	.if (\_align - (1 << TZ_COUNT))
	  .error "Incorrect stack alignment specified (Must be a power of 2)."
	.endif
	.if ((\_size & ((1 << TZ_COUNT) - 1)) <> 0)
	  .error "Stack size not correctly aligned"
	.endif
	.section    \_section, "aw", %nobits
	.align TZ_COUNT
	\_name:
	.space ((\_count) * (\_size)), 0
	.endm


#endif /* __ASM_MACROS_COMMON_S__ */
