/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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
#include <debug.h>
#include <stdarg.h>
#include <stdint.h>

/***********************************************************
 * The tf_printf implementation for all BL stages
 ***********************************************************/
static void unsigned_num_print(unsigned long int unum, unsigned int radix)
{
	/* Just need enough space to store 64 bit decimal integer */
	unsigned char num_buf[20];
	int i = 0 , rem;

	do {
		rem = unum % radix;
		if (rem < 0xa)
			num_buf[i++] = '0' + rem;
		else
			num_buf[i++] = 'a' + (rem - 0xa);
	} while (unum /= radix);

	while (--i >= 0)
		putchar(num_buf[i]);
}

static void string_print(const char *str)
{
	while (*str)
		putchar(*str++);
}

/*******************************************************************
 * Reduced format print for Trusted firmware.
 * The following formats are supported by this print
 * %x - 32 bit hexadecimal format
 * %llx and %lx -64 bit hexadecimal format
 * %s - string format
 * %d or %i - signed 32 bit decimal format
 * %u - unsigned 32 bit decimal format
 * %ld and %lld - signed 64 bit decimal format
 * %lu and %llu - unsigned 64 bit decimal format
 * Exits on all other formats.
 *******************************************************************/

void tf_printf(const char *fmt, ...)
{
	va_list args;
	int bit64;
	int64_t num;
	uint64_t unum;
	char *str;

	va_start(args, fmt);
	while (*fmt) {
		bit64 = 0;

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier */
loop:
			switch (*fmt) {
			case 'i': /* Fall through to next one */
			case 'd':
				if (bit64)
					num = va_arg(args, int64_t);
				else
					num = va_arg(args, int32_t);

				if (num < 0) {
					putchar('-');
					unum = (unsigned long int)-num;
				} else
					unum = (unsigned long int)num;

				unsigned_num_print(unum, 10);
				break;
			case 's':
				str = va_arg(args, char *);
				string_print(str);
				break;
			case 'x':
				if (bit64)
					unum = va_arg(args, uint64_t);
				else
					unum = va_arg(args, uint32_t);

				unsigned_num_print(unum, 16);
				break;
			case 'l':
				bit64 = 1;
				fmt++;
				goto loop;
			case 'u':
				if (bit64)
					unum = va_arg(args, uint64_t);
				else
					unum = va_arg(args, uint32_t);

				unsigned_num_print(unum, 10);
				break;
			default:
				/* Exit on any other format specifier */
				goto exit;
			}
			fmt++;
			continue;
		}
		putchar(*fmt++);
	}
exit:
	va_end(args);
}
