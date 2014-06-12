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
static void signed_dec_print(long int num)
{
	/* Just need enough space to store 64 bit decimal integer */
	unsigned char dec_buf[19];
	unsigned long int unum = 0;
	int i = 0;

	if (num < 0) {
		putchar('-');
		unum = (unsigned long int)-num;
	} else
		unum = (unsigned long int)num;

	do {
		dec_buf[i++] = '0' + unum % 10;
	} while (unum /= 10);

	while (--i >= 0)
		putchar(dec_buf[i]);
}

static void string_print(const char *str)
{
	while (*str)
		putchar(*str++);
}

static void hex_print(unsigned long int val)
{
	/* Just need enough space to store 64 bit hex */
	unsigned char hex_buf[16];
	int i = 0;
	unsigned char temp;

	do {
		temp = val & 0xf;
		if (temp < 0xa)
			hex_buf[i++] = '0' + temp;
		else
			hex_buf[i++] = 'a' + (temp - 0xa);
	} while (val >>= 4);

	while (--i >= 0)
		putchar(hex_buf[i]);
}

/*******************************************************************
 * Reduced format print for Trusted firmware.
 * The following formats are supported by this print
 * %x - 32 bit hexadecimal format
 * %llx and %lx -64 bit hexadecimal format
 * %s - string format
 * %d or %i - signed 32 bit decimal format
 * %ld and %lld - signed 64 bit decimal format
 * All other formats are ignored.
 *******************************************************************/

void tf_printf(const char *fmt, ...)
{
	va_list args;
	int bit64;
	int64_t num;
	uint64_t hex;
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

				signed_dec_print(num);
				break;
			case 's':
				str = va_arg(args, char *);
				string_print(str);
				break;
			case 'x':
				if (bit64)
					hex = va_arg(args, uint64_t);
				else
					hex = va_arg(args, uint32_t);

				hex_print(hex);
				break;
			case 'l':
				bit64 = 1;
				fmt++;
				goto loop;
			default:
				/* Ignore any other format specifier */
				break;
			}
			fmt++;
			continue;
		}
		putchar(*fmt++);
	}
	va_end(args);
}
