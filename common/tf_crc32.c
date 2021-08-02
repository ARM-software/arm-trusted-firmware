/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <assert.h>

#include <arm_acle.h>
#include <common/debug.h>
#include <common/tf_crc32.h>

/* compute CRC using Arm intrinsic function
 *
 * This function is useful for the platforms with the CPU ARMv8.0
 * (with CRC instructions supported), and onwards.
 * Platforms with CPU ARMv8.0 should make sure to add a compile switch
 * '-march=armv8-a+crc" for successful compilation of this file.
 *
 * @crc: previous accumulated CRC
 * @buf: buffer base address
 * @size: the size of the buffer
 *
 * Return calculated CRC value
 */
uint32_t tf_crc32(uint32_t crc, const unsigned char *buf, size_t size)
{
	assert(buf != NULL);

	uint32_t calc_crc = ~crc;
	const unsigned char *local_buf = buf;
	size_t local_size = size;

	/*
	 * calculate CRC over byte data
	 */
	while (local_size != 0UL) {
		calc_crc = __crc32b(calc_crc, *local_buf);
		local_buf++;
		local_size--;
	}

	return ~calc_crc;
}
