/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>

#include <common/debug.h>
#include <plat/common/platform.h>

static void string_print(char **s, size_t n, size_t *chars_printed,
			 const char *str)
{
	while (*str != '\0') {
		if (*chars_printed < n) {
			*(*s) = *str;
			(*s)++;
		}

		(*chars_printed)++;
		str++;
	}
}

static void unsigned_dec_print(char **s, size_t n, size_t *chars_printed,
			       unsigned int unum)
{
	/* Enough for a 32-bit unsigned decimal integer (4294967295). */
	char num_buf[10];
	int i = 0;
	unsigned int rem;

	do {
		rem = unum % 10U;
		num_buf[i++] = '0' + rem;
		unum /= 10U;
	} while (unum > 0U);

	while (--i >= 0) {
		if (*chars_printed < n) {
			*(*s) = num_buf[i];
			(*s)++;
		}

		(*chars_printed)++;
	}
}

/*******************************************************************
 * Reduced snprintf to be used for Trusted firmware.
 * The following type specifiers are supported:
 *
 * %d or %i - signed decimal format
 * %s - string format
 * %u - unsigned decimal format
 *
 * The function panics on all other formats specifiers.
 *
 * It returns the number of characters that would be written if the
 * buffer was big enough. If it returns a value lower than n, the
 * whole string has been written.
 *******************************************************************/
int snprintf(char *s, size_t n, const char *fmt, ...)
{
	va_list args;
	int num;
	unsigned int unum;
	char *str;
	size_t chars_printed = 0U;

	if (n == 0U) {
		/* There isn't space for anything. */
	} else if (n == 1U) {
		/* Buffer is too small to actually write anything else. */
		*s = '\0';
		n = 0U;
	} else {
		/* Reserve space for the terminator character. */
		n--;
	}

	va_start(args, fmt);
	while (*fmt != '\0') {

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier. */
			switch (*fmt) {
			case 'i':
			case 'd':
				num = va_arg(args, int);

				if (num < 0) {
					if (chars_printed < n) {
						*s = '-';
						s++;
					}
					chars_printed++;

					unum = (unsigned int)-num;
				} else {
					unum = (unsigned int)num;
				}

				unsigned_dec_print(&s, n, &chars_printed, unum);
				break;
			case 's':
				str = va_arg(args, char *);
				string_print(&s, n, &chars_printed, str);
				break;
			case 'u':
				unum = va_arg(args, unsigned int);
				unsigned_dec_print(&s, n, &chars_printed, unum);
				break;
			default:
				/* Panic on any other format specifier. */
				ERROR("snprintf: specifier with ASCII code '%d' not supported.",
				      *fmt);
				plat_panic_handler();
				assert(0); /* Unreachable */
			}
			fmt++;
			continue;
		}

		if (chars_printed < n) {
			*s = *fmt;
			s++;
		}

		fmt++;
		chars_printed++;
	}

	va_end(args);

	if (n > 0U)
		*s = '\0';

	return (int)chars_printed;
}
