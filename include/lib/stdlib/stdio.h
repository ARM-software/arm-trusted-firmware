/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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
 *	@(#)stdio.h	8.5 (Berkeley) 4/29/95
 * $FreeBSD$
 */

/*
 * Portions copyright (c) 2013-2014, ARM Limited and Contributors.
 * All rights reserved.
 */


#ifndef	_STDIO_H_
#define	_STDIO_H_

#include <sys/cdefs.h>
#include <sys/_null.h>
#include <sys/_types.h>

#ifndef _SIZE_T_DECLARED
typedef	__size_t	size_t;
#define	_SIZE_T_DECLARED
#endif

#ifndef _SSIZE_T_DECLARED
#define	_SSIZE_T_DECLARED
typedef	__ssize_t	ssize_t;
#endif

#define	EOF	(-1)

int	 printf(const char * __restrict, ...) __printflike(1, 2);
int	 putchar(int);
int	 puts(const char *);
int	 sprintf(char * __restrict, const char * __restrict, ...)
	__printflike(2, 3);
int	 vsprintf(char * __restrict, const char * __restrict,
	   __va_list) __printflike(2, 0);

int	 sscanf(const char *__restrict, char const *__restrict, ...);

#if __ISO_C_VISIBLE >= 1999
int	 snprintf(char * __restrict, size_t, const char * __restrict,
	   ...) __printflike(3, 4);
int	 vsnprintf(char * __restrict, size_t, const char * __restrict,
	   __va_list) __printflike(3, 0);
#endif

#endif /* !_STDIO_H_ */
