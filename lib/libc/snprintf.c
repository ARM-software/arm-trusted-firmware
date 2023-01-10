/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define get_num_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, long int) :		\
			    va_arg(_args, int)))

#define get_unum_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, unsigned long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, unsigned long int) :		\
			    va_arg(_args, unsigned int)))

#define CHECK_AND_PUT_CHAR(buf, size, chars_printed, ch)	\
	do {						\
		if ((chars_printed) < (size)) {		\
			*(buf) = (ch);			\
			(buf)++;			\
		}					\
		(chars_printed)++;			\
	} while (false)

static void string_print(char **s, size_t n, size_t *chars_printed,
			 const char *str)
{
	while (*str != '\0') {
		CHECK_AND_PUT_CHAR(*s, n, *chars_printed, *str);
		str++;
	}
}

static void unsigned_num_print(char **s, size_t n, size_t *chars_printed,
			      unsigned long long int unum,
			      unsigned int radix, char padc, int padn,
			      bool capitalise)
{
	/* Just need enough space to store 64 bit decimal integer */
	char num_buf[20];
	int i = 0;
	int width;
	unsigned int rem;
	char ascii_a = capitalise ? 'A' : 'a';

	/* num_buf is only large enough for radix >= 10 */
	if (radix < 10) {
		assert(0);
		return;
	}

	do {
		rem = unum % radix;
		if (rem < 10U) {
			num_buf[i] = '0' + rem;
		} else {
			num_buf[i] = ascii_a + (rem - 10U);
		}
		i++;
		unum /= radix;
	} while (unum > 0U);

	width = i;
	for (i = padn - width; i > 0; i--) {
		CHECK_AND_PUT_CHAR(*s, n, *chars_printed, padc);
	}
	for (i = width; i > 0; i--) {
		CHECK_AND_PUT_CHAR(*s, n, *chars_printed, num_buf[i - 1]);
	}
	for (i = width + padn; i < 0; i++) {
		CHECK_AND_PUT_CHAR(*s, n, *chars_printed, padc);
	}
}

/*******************************************************************
 * Reduced vsnprintf to be used for Trusted firmware.
 * The following type specifiers are supported:
 *
 * %x (or %X) - hexadecimal format
 * %d or %i - signed decimal format
 * %s - string format
 * %u - unsigned decimal format
 * %p - pointer format
 *
 * The following length specifiers are supported by this print
 * %l - long int
 * %ll - long long int
 * %z - size_t sized integer formats
 *
 * The following padding specifiers are supported by this print
 * %0NN - Left-pad the number with 0s (NN is a decimal number)
 * %NN - Left-pad the number or string with spaces (NN is a decimal number)
 * %-NN - Right-pad the number or string with spaces (NN is a decimal number)
 *
 * The function panics on all other formats specifiers.
 *
 * It returns the number of characters that would be written if the
 * buffer was big enough. If it returns a value lower than n, the
 * whole string has been written.
 *******************************************************************/
int vsnprintf(char *s, size_t n, const char *fmt, va_list args)
{
	int num;
	unsigned long long int unum;
	char *str;
	char padc;		/* Padding character */
	int padn;		/* Number of characters to pad */
	bool left;
	bool capitalise;
	size_t chars_printed = 0U;
	unsigned int l_count;

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

	while (*fmt != '\0') {
		left = false;
		padc ='\0';
		padn = 0;
		capitalise = false;
		l_count = 0;

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier. */
loop:
			switch (*fmt) {
			case '%':
				CHECK_AND_PUT_CHAR(s, n, chars_printed, '%');
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				padc = (*fmt == '0') ? '0' : ' ';
				for (padn = 0; *fmt >= '0' && *fmt <= '9'; fmt++) {
					padn = (padn * 10) + (*fmt - '0');
				}
				if (left) {
					padn = -padn;
				}
				goto loop;
			case '-':
				left = true;
				fmt++;
				goto loop;

			case 'i':
			case 'd':
				num = get_num_va_args(args, l_count);

				if (num < 0) {
					CHECK_AND_PUT_CHAR(s, n, chars_printed,
						'-');
					unum = (unsigned int)-num;
				} else {
					unum = (unsigned int)num;
				}

				unsigned_num_print(&s, n, &chars_printed,
						   unum, 10, padc, padn, false);
				break;
			case 's':
				str = va_arg(args, char *);
				string_print(&s, n, &chars_printed, str);
				break;
			case 'u':
				unum = get_unum_va_args(args, l_count);
				unsigned_num_print(&s, n, &chars_printed,
						   unum, 10, padc, padn, false);
				break;
			case 'z':
				l_count = 1;
				fmt++;
				goto loop;
			case 'l':
				l_count++;
				fmt++;
				goto loop;
			case 'p':
				unum = (uintptr_t)va_arg(args, void *);
				if (unum > 0U) {
					string_print(&s, n, &chars_printed, "0x");
					padn -= 2;
				}
				unsigned_num_print(&s, n, &chars_printed,
						   unum, 16, padc, padn, false);
				break;
			case 'X':
				capitalise = true;
				/* fallthrough */
			case 'x':
				unum = get_unum_va_args(args, l_count);
				unsigned_num_print(&s, n, &chars_printed,
						   unum, 16, padc, padn,
						   capitalise);
				break;

			default:
				CHECK_AND_PUT_CHAR(s, n, chars_printed, '%');
				CHECK_AND_PUT_CHAR(s, n, chars_printed, *fmt);
			}
			fmt++;
			continue;
		}

		CHECK_AND_PUT_CHAR(s, n, chars_printed, *fmt);

		fmt++;
	}

	if (n > 0U) {
		*s = '\0';
	}

	return (int)chars_printed;
}

/*******************************************************************
 * Reduced snprintf to be used for Trusted firmware.
 * The following type specifiers are supported:
 *
 * %x (or %X) - hexadecimal format
 * %d or %i - signed decimal format
 * %s - string format
 * %u - unsigned decimal format
 * %p - pointer format
 *
 * The following padding specifiers are supported by this print
 * %0NN - Left-pad the number with 0s (NN is a decimal number)
 * %NN - Left-pad the number or string with spaces (NN is a decimal number)
 * %-NN - Right-pad the number or string with spaces (NN is a decimal number)
 *
 * The function panics on all other formats specifiers.
 *
 * It returns the number of characters that would be written if the
 * buffer was big enough. If it returns a value lower than n, the
 * whole string has been written.
 *******************************************************************/
int snprintf(char *s, size_t n, const char *fmt, ...)
{
	int count;
	va_list all_args;

	va_start(all_args, fmt);
	count = vsnprintf(s, n, fmt, all_args);
	va_end(all_args);

	return count;
}
