/*
 * Copyright (c) 2014-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define get_num_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, long int) :		\
			    va_arg(_args, int)))

#define get_unum_va_args(_args, _lcount)				\
	(((_lcount) > 1)  ? va_arg(_args, unsigned long long int) :	\
	(((_lcount) == 1) ? va_arg(_args, unsigned long int) :		\
			    va_arg(_args, unsigned int)))

static int string_print(const char *str)
{
	int count = 0;

	assert(str != NULL);

	for ( ; *str != '\0'; str++) {
		(void)putchar(*str);
		count++;
	}

	return count;
}

static int unsigned_num_print(unsigned long long int unum, unsigned int radix,
			      char padc, int padn, bool uppercase)
{
	/* Just need enough space to store 64 bit decimal integer */
	char num_buf[20];
	int i = 0, count = 0;
	unsigned int rem;

	/* num_buf is only large enough for radix >= 10 */
	if (radix < 10U) {
		assert(0);
		return 0;
	}

	do {
		rem = (uint32_t)(unum % radix);
		if (rem < 0xaU) {
			num_buf[i] = '0' + rem;
		} else if (uppercase) {
			num_buf[i] = 'A' + (rem - 0xaU);
		} else {
			num_buf[i] = 'a' + (rem - 0xaU);
		}
		i++;
		unum /= radix;
	} while (unum > 0U);

	if (padn > 0) {
		while (i < padn) {
			(void)putchar((int32_t)padc);
			count++;
			padn--;
		}
	}

	while (--i >= 0) {
		(void)putchar((int32_t)num_buf[i]);
		count++;
	}

	return count;
}

/*******************************************************************
 * Reduced format print for Trusted firmware.
 * The following type specifiers are supported by this print
 * %x - hexadecimal format
 * %s - string format
 * %d or %i - signed decimal format
 * %c - character format
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
 * %NN - Left-pad the number with spaces (NN is a decimal number)
 *
 * The print exits on all other formats specifiers other than valid
 * combinations of the above specifiers.
 *******************************************************************/
int vprintf(const char *fmt, va_list args)
{
	int l_count;
	long long int num;
	unsigned long long int unum;
	const char *str;
	char padc = '\0'; /* Padding character */
	int padn; /* Number of characters to pad */
	int count = 0; /* Number of printed characters */
	bool uppercase; /* Print characters in uppercase */

	while (*fmt != '\0') {
		uppercase = false;
		l_count = 0;
		padn = 0;

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier */
loop:
			switch (*fmt) {
			case '%':
				(void)putchar((int32_t)'%');
				break;
			case 'i': /* Fall through to next one */
			case 'd':
				num = get_num_va_args(args, l_count);
				if (num < 0) {
					(void)putchar((int32_t)'-');
					unum = (unsigned long long int)-num;
					padn--;
				} else {
					unum = (unsigned long long int)num;
				}

				count += unsigned_num_print(unum, 10,
							    padc, padn, uppercase);
				break;
			case 'c':
				(void)putchar(va_arg(args, int));
				count++;
				break;
			case 's':
				str = va_arg(args, const char *);
				count += string_print(str);
				break;
			case 'p':
				unum = (uintptr_t)va_arg(args, void *);
				if (unum > 0U) {
					count += string_print("0x");
					padn -= 2;
				}

				count += unsigned_num_print(unum, 16,
							    padc, padn, uppercase);
				break;
			case 'X':
				uppercase = true;
				// fall through
			case 'x':
				unum = get_unum_va_args(args, l_count);
				count += unsigned_num_print(unum, 16,
							    padc, padn, uppercase);
				break;
			case 'z':
				if (sizeof(size_t) == 8U) {
					l_count = 2;
				}

				fmt++;
				goto loop;
			case 'l':
				l_count++;
				fmt++;
				goto loop;
			case 'u':
				unum = get_unum_va_args(args, l_count);
				count += unsigned_num_print(unum, 10,
							    padc, padn, uppercase);
				break;
			case '0':
				padc = '0';
				padn = 0;
				fmt++;

				for (;;) {
					char ch = *fmt;
					if ((ch < '0') || (ch > '9')) {
						goto loop;
					}
					padn = (padn * 10) + (ch - '0');
					fmt++;
				}
				assert(0); /* Unreachable */
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				padc = ' ';
				padn = 0;

				for (;;) {
					char ch = *fmt;
					if ((ch < '0') || (ch > '9')) {
						goto loop;
					}
					padn = (padn * 10) + (ch - '0');
					fmt++;
				}
				assert(0); /* Unreachable */
			default:
				/* Exit on any other format specifier */
				return -1;
			}
			fmt++;
			continue;
		}
		(void)putchar(*fmt);
		fmt++;
		count++;
	}

	return count;
}

int printf(const char *fmt, ...)
{
	int count;
	va_list va;

	va_start(va, fmt);
	count = vprintf(fmt, va);
	va_end(va);

	return count;
}
