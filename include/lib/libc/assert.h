/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)assert.h	8.2 (Berkeley) 1/21/94
 * $FreeBSD$
 */
/*
 * Portions copyright (c) 2017, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <debug.h>
#include <platform_def.h>
#include <sys/cdefs.h>

#ifndef PLAT_LOG_LEVEL_ASSERT
#define PLAT_LOG_LEVEL_ASSERT	LOG_LEVEL
#endif

#if ENABLE_ASSERTIONS
#define	_assert(e)	assert(e)
# if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
#  define	assert(e)	((e) ? (void)0 : __assert(__FILE__, __LINE__, #e))
# elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
#  define	assert(e)	((e) ? (void)0 : __assert(__FILE__, __LINE__))
# else
#  define	assert(e)	((e) ? (void)0 : __assert())
# endif
#else
#define	assert(e)	((void)0)
#define	_assert(e)	((void)0)
#endif /* ENABLE_ASSERTIONS */

__BEGIN_DECLS
#if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
void __assert(const char *, unsigned int, const char *) __dead2;
#elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
void __assert(const char *, unsigned int) __dead2;
#else
void __assert(void) __dead2;
#endif
__END_DECLS

#endif /* !_ASSERT_H_ */
