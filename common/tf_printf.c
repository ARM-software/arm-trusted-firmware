/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>

/***********************************************************
 * The tf_printf implementation for all BL stages
 ***********************************************************/

#define get_num_va_args(args, lcount) \
	(((lcount) > 1) ? va_arg(args, long long int) :	\
	((lcount) ? va_arg(args, long int) : va_arg(args, int)))

#define get_unum_va_args(args, lcount) \
	(((lcount) > 1) ? va_arg(args, unsigned long long int) :	\
	((lcount) ? va_arg(args, unsigned long int) : va_arg(args, unsigned int)))

void tf_string_print(const char *str)
{
	assert(str);

	while (*str)
		putchar(*str++);
}

static void unsigned_num_print(unsigned long long int unum, unsigned int radix,
			       char padc, int padn)
{
	/* Just need enough space to store 64 bit decimal integer */
	unsigned char num_buf[20];
	int i = 0, rem;

	do {
		rem = unum % radix;
		if (rem < 0xa)
			num_buf[i++] = '0' + rem;
		else
			num_buf[i++] = 'a' + (rem - 0xa);
	} while (unum /= radix);

	if (padn > 0) {
		while (i < padn--) {
			putchar(padc);
		}
	}

	while (--i >= 0)
		putchar(num_buf[i]);
}

/*******************************************************************
 * Reduced format print for Trusted firmware.
 * The following type specifiers are supported by this print
 * %x - hexadecimal format
 * %s - string format
 * %d or %i - signed decimal format
 * %u - unsigned decimal format
 * %p - pointer format
 *
 * The following length specifiers are supported by this print
 * %l - long int (64-bit on AArch64)
 * %ll - long long int (64-bit on AArch64)
 * %z - size_t sized integer formats (64 bit on AArch64)
 *
 * The following padding specifiers are supported by this print
 * %0NN - Left-pad the number with 0s (NN is a decimal number)
 *
 * The print exits on all other formats specifiers other than valid
 * combinations of the above specifiers.
 *******************************************************************/
void tf_vprintf(const char *fmt, va_list args)
{
	int l_count;
	long long int num;
	unsigned long long int unum;
	char *str;
	char padc = 0; /* Padding character */
	int padn; /* Number of characters to pad */

	while (*fmt) {
		l_count = 0;
		padn = 0;

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier */
loop:
			switch (*fmt) {
			case 'i': /* Fall through to next one */
			case 'd':
				num = get_num_va_args(args, l_count);
				if (num < 0) {
					putchar('-');
					unum = (unsigned long long int)-num;
					padn--;
				} else
					unum = (unsigned long long int)num;

				unsigned_num_print(unum, 10, padc, padn);
				break;
			case 's':
				str = va_arg(args, char *);
				tf_string_print(str);
				break;
			case 'p':
				unum = (uintptr_t)va_arg(args, void *);
				if (unum) {
					tf_string_print("0x");
					padn -= 2;
				}

				unsigned_num_print(unum, 16, padc, padn);
				break;
			case 'x':
				unum = get_unum_va_args(args, l_count);
				unsigned_num_print(unum, 16, padc, padn);
				break;
			case 'z':
				if (sizeof(size_t) == 8)
					l_count = 2;

				fmt++;
				goto loop;
			case 'l':
				l_count++;
				fmt++;
				goto loop;
			case 'u':
				unum = get_unum_va_args(args, l_count);
				unsigned_num_print(unum, 10, padc, padn);
				break;
			case '0':
				padc = '0';
				padn = 0;
				fmt++;

				while (1) {
					char ch = *fmt;
					if (ch < '0' || ch > '9') {
						goto loop;
					}
					padn = (padn * 10) + (ch - '0');
					fmt++;
				}
			default:
				/* Exit on any other format specifier */
				return;
			}
			fmt++;
			continue;
		}
		putchar(*fmt++);
	}
}

void tf_printf(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	tf_vprintf(fmt, va);
	va_end(va);
}
