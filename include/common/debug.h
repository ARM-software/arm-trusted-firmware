/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

/* If building the project with DEBUG disabled the INFO and WARN macros
 * won't produce any output. The ERROR macro is always enabled.
 * The format expected is the same as for printf().
 * INFO("Info %s.\n", "message")    -> INFO: Info message.
 * WARN("Warning %s.\n", "message") -> WARN: Warning message.
 * ERROR("Error %s.\n", "message")  -> ERROR: Error message.
 *
 * TODO : add debug levels.
 */
#if DEBUG
 #define INFO(...)	printf("INFO: " __VA_ARGS__)
 #define WARN(...)	printf("WARN: " __VA_ARGS__)
#else
 #define INFO(...)
 #define WARN(...)
#endif

#define ERROR(...)	printf("ERROR: " __VA_ARGS__)


/* For the moment this Panic function is very basic, Report an error and
 * spin. This can be expanded in the future to provide more information.
 */
#if DEBUG
void __dead2 do_panic(const char *file, int line);
#define panic()	do_panic(__FILE__, __LINE__)

#else
void __dead2 do_panic(void);
#define panic()	do_panic()

#endif

void print_string_value(char *s, unsigned long *mem);

#endif /* __DEBUG_H__ */
