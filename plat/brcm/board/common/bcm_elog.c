/*
 * Copyright (c) 2018 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <plat/common/platform.h>

#include <bcm_elog.h>

/* error logging signature */
#define BCM_ELOG_SIG_OFFSET      0x0000
#define BCM_ELOG_SIG_VAL         0x75767971

/* current logging offset that points to where new logs should be added */
#define BCM_ELOG_OFF_OFFSET      0x0004

/* current logging length (excluding header) */
#define BCM_ELOG_LEN_OFFSET      0x0008

#define BCM_ELOG_HEADER_LEN      12

/*
 * @base: base address of memory where log is saved
 * @max_size: max size of memory reserved for logging
 * @is_active: indicates logging is currently active
 * @level: current logging level
 */
struct bcm_elog {
	uintptr_t base;
	uint32_t max_size;
	unsigned int is_active;
	unsigned int level;
};

static struct bcm_elog global_elog;

extern void memcpy16(void *dst, const void *src, unsigned int len);

/*
 * Log one character
 */
static void elog_putchar(struct bcm_elog *elog, unsigned char c)
{
	uint32_t offset, len;

	offset = mmio_read_32(elog->base + BCM_ELOG_OFF_OFFSET);
	len = mmio_read_32(elog->base + BCM_ELOG_LEN_OFFSET);
	mmio_write_8(elog->base + offset, c);
	offset++;

	/* log buffer is now full and need to wrap around */
	if (offset >= elog->max_size)
		offset = BCM_ELOG_HEADER_LEN;

	/* only increment length when log buffer is not full */
	if (len < elog->max_size - BCM_ELOG_HEADER_LEN)
		len++;

	mmio_write_32(elog->base + BCM_ELOG_OFF_OFFSET, offset);
	mmio_write_32(elog->base + BCM_ELOG_LEN_OFFSET, len);
}

static void elog_unsigned_num(struct bcm_elog *elog, unsigned long unum,
			      unsigned int radix)
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

	while (--i >= 0)
		elog_putchar(elog, num_buf[i]);
}

static void elog_string(struct bcm_elog *elog, const char *str)
{
	while (*str)
		elog_putchar(elog, *str++);
}

/*
 * Routine to initialize error logging
 */
int bcm_elog_init(void *base, uint32_t size, unsigned int level)
{
	struct bcm_elog *elog = &global_elog;
	uint32_t val;

	elog->base = (uintptr_t)base;
	elog->max_size = size;
	elog->is_active = 1;
	elog->level = level / 10;

	/*
	 * If a valid signature can be found, it means logs have been copied
	 * into designated memory by another software. In this case, we should
	 * not re-initialize the entry header in the designated memory
	 */
	val = mmio_read_32(elog->base + BCM_ELOG_SIG_OFFSET);
	if (val != BCM_ELOG_SIG_VAL) {
		mmio_write_32(elog->base + BCM_ELOG_SIG_OFFSET,
			      BCM_ELOG_SIG_VAL);
		mmio_write_32(elog->base + BCM_ELOG_OFF_OFFSET,
			      BCM_ELOG_HEADER_LEN);
		mmio_write_32(elog->base + BCM_ELOG_LEN_OFFSET, 0);
	}

	return 0;
}

/*
 * Routine to disable error logging
 */
void bcm_elog_exit(void)
{
	struct bcm_elog *elog = &global_elog;

	if (!elog->is_active)
		return;

	elog->is_active = 0;

	flush_dcache_range(elog->base, elog->max_size);
}

/*
 * Routine to copy error logs from current memory to 'dst' memory and continue
 * logging from the new 'dst' memory.
 * dst and base addresses must be 16-bytes aligned.
 */
int bcm_elog_copy_log(void *dst, uint32_t max_size)
{
	struct bcm_elog *elog = &global_elog;
	uint32_t offset, len;

	if (!elog->is_active || ((uintptr_t)dst == elog->base))
		return -1;

	/* flush cache before copying logs */
	flush_dcache_range(elog->base, max_size);

	/*
	 * If current offset exceeds the new max size, then that is considered
	 * as a buffer overflow situation. In this case, we reset the offset
	 * back to the beginning
	 */
	offset = mmio_read_32(elog->base + BCM_ELOG_OFF_OFFSET);
	if (offset >= max_size) {
		offset = BCM_ELOG_HEADER_LEN;
		mmio_write_32(elog->base + BCM_ELOG_OFF_OFFSET, offset);
	}

	/* note payload length does not include header */
	len = mmio_read_32(elog->base + BCM_ELOG_LEN_OFFSET);
	if (len > max_size - BCM_ELOG_HEADER_LEN) {
		len = max_size - BCM_ELOG_HEADER_LEN;
		mmio_write_32(elog->base + BCM_ELOG_LEN_OFFSET, len);
	}

	/* Need to copy everything including the header. */
	memcpy16(dst, (const void *)elog->base, len + BCM_ELOG_HEADER_LEN);
	elog->base = (uintptr_t)dst;
	elog->max_size = max_size;

	return 0;
}

/*
 * Main routine to save logs into memory
 */
void bcm_elog(const char *fmt, ...)
{
	va_list args;
	const char *prefix_str;
	int bit64;
	int64_t num;
	uint64_t unum;
	char *str;
	struct bcm_elog *elog = &global_elog;

	/* We expect the LOG_MARKER_* macro as the first character */
	unsigned int level = fmt[0];

	if (!elog->is_active || level > elog->level)
		return;

	prefix_str = plat_log_get_prefix(level);

	while (*prefix_str != '\0') {
		elog_putchar(elog, *prefix_str);
		prefix_str++;
	}

	va_start(args, fmt);
	fmt++;
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
					elog_putchar(elog, '-');
					unum = (unsigned long)-num;
				} else
					unum = (unsigned long)num;

				elog_unsigned_num(elog, unum, 10);
				break;
			case 's':
				str = va_arg(args, char *);
				elog_string(elog, str);
				break;
			case 'x':
				if (bit64)
					unum = va_arg(args, uint64_t);
				else
					unum = va_arg(args, uint32_t);

				elog_unsigned_num(elog, unum, 16);
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

				elog_unsigned_num(elog, unum, 10);
				break;
			default:
				/* Exit on any other format specifier */
				goto exit;
			}
			fmt++;
			continue;
		}
		elog_putchar(elog, *fmt++);
	}
exit:
	va_end(args);
}
