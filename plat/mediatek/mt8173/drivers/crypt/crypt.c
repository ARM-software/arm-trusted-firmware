/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt8173_def.h>

#define crypt_read32(offset)	\
	mmio_read_32((uintptr_t)(CRYPT_BASE+((offset) * 4)))

#define crypt_write32(offset, value)    \
	mmio_write_32((uintptr_t)(CRYPT_BASE + ((offset) * 4)), (uint32_t)value)

#define GET_L32(x) ((uint32_t)(x & 0xffffffff))
#define GET_H32(x) ((uint32_t)((x >> 32) & 0xffffffff))

#define REG_0 0
#define REG_4 4
#define REG_256 256
#define REG_512 512
#define REG_513 513
#define REG_514 514
#define REG_768 768
#define REG_769 769
#define REG_770 770
#define REG_771 771
#define REG_772 772
#define REG_820 820

uint64_t crypt_wait(void)
{
	crypt_write32(REG_256, 0);
	while(crypt_read32(REG_512) == 0);
	udelay(0x64);
	crypt_write32(REG_513, crypt_read32(REG_512));
	crypt_write32(REG_514, 0);

	return 0;
}

static uint32_t record[4];
uint64_t crypt_set_hdcp_key_ex(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint32_t i = (uint32_t)x1;
	uint32_t j = 0;

	if (i < 40) {
		crypt_write32(REG_4, 0x80ff3800);
		crypt_write32(REG_0, 0);
		crypt_write32(REG_0, 0xF);
		crypt_write32(REG_513, 1);
		crypt_write32(REG_514, 0);

		crypt_write32(REG_768, 0x70);
		crypt_write32(REG_769, 0x1C0);
		crypt_write32(REG_770, 0x30);
		crypt_write32(REG_771, 0x4);
		crypt_wait();

		crypt_write32(REG_820 + 4 * i, GET_L32(x2));
		crypt_write32(REG_820 + 4 * i + 1, GET_H32(x2));
		crypt_write32(REG_820 + 4 * i + 2, GET_L32(x3));
		crypt_write32(REG_820 + 4 * i + 3, GET_H32(x3));

		crypt_write32(REG_769, 0);

		crypt_write32(REG_768, 0x20);
		crypt_write32(REG_771, 0x34 + 4 * i);
		crypt_write32(REG_772, 0x34 + 4 * i);
		crypt_wait();

		for (j = 0; j < 4; j++) {
			crypt_write32(REG_768, 0x71);
			crypt_write32(REG_769, 0x34 + 4 * i + j);
			crypt_write32(REG_770, record[j]);
			crypt_wait();
		}
	}
	record[0] = GET_L32(x2);
	record[1] = GET_H32(x2);
	record[2] = GET_L32(x3);
	record[3] = GET_H32(x3);
	return 0;
}

uint64_t crypt_set_hdcp_key_num(uint32_t num)
{
	crypt_write32(REG_768, 0x6A);
	crypt_write32(REG_769, 0x34 + 4 * num);
	crypt_wait();
	return 0;
}

uint64_t crypt_clear_hdcp_key(void)
{
	uint32_t i;

	for (i = 0; i < 160; i++)
		crypt_write32(REG_820 + i, 0);
	return 0;
}
