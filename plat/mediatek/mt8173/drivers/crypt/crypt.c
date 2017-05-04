/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt8173_def.h>
#include <mtk_sip_svc.h>

#define crypt_read32(offset)	\
	mmio_read_32((uintptr_t)(CRYPT_BASE+((offset) * 4)))

#define crypt_write32(offset, value)    \
	mmio_write_32((uintptr_t)(CRYPT_BASE + ((offset) * 4)), (uint32_t)value)

#define GET_L32(x) ((uint32_t)(x & 0xffffffff))
#define GET_H32(x) ((uint32_t)((x >> 32) & 0xffffffff))

#define REG_INIT 0
#define REG_MSC 4
#define REG_TRIG 256
#define REG_STAT 512
#define REG_CLR 513
#define REG_INT 514
#define REG_P68 768
#define REG_P69 769
#define REG_P70 770
#define REG_P71 771
#define REG_P72 772
#define REG_D20 820
#define KEY_SIZE 160
#define KEY_LEN 40

/* Wait until crypt is completed */
uint64_t crypt_wait(void)
{
	crypt_write32(REG_TRIG, 0);
	while (crypt_read32(REG_STAT) == 0)
		;
	udelay(100);
	crypt_write32(REG_CLR, crypt_read32(REG_STAT));
	crypt_write32(REG_INT, 0);
	return MTK_SIP_E_SUCCESS;
}

static uint32_t record[4];
/* Copy encrypted key to crypt engine */
uint64_t crypt_set_hdcp_key_ex(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint32_t i = (uint32_t)x1;
	uint32_t j = 0;

	if (i > KEY_LEN)
		return MTK_SIP_E_INVALID_PARAM;

	if (i < KEY_LEN) {
		crypt_write32(REG_MSC, 0x80ff3800);
		crypt_write32(REG_INIT, 0);
		crypt_write32(REG_INIT, 0xF);
		crypt_write32(REG_CLR, 1);
		crypt_write32(REG_INT, 0);

		crypt_write32(REG_P68, 0x70);
		crypt_write32(REG_P69, 0x1C0);
		crypt_write32(REG_P70, 0x30);
		crypt_write32(REG_P71, 0x4);
		crypt_wait();

		crypt_write32(REG_D20 + 4 * i, GET_L32(x2));
		crypt_write32(REG_D20 + 4 * i + 1, GET_H32(x2));
		crypt_write32(REG_D20 + 4 * i + 2, GET_L32(x3));
		crypt_write32(REG_D20 + 4 * i + 3, GET_H32(x3));

		crypt_write32(REG_P69, 0);
		crypt_write32(REG_P68, 0x20);
		crypt_write32(REG_P71, 0x34 + 4 * i);
		crypt_write32(REG_P72, 0x34 + 4 * i);
		crypt_wait();

		for (j = 0; j < 4; j++) {
			crypt_write32(REG_P68, 0x71);
			crypt_write32(REG_P69, 0x34 + 4 * i + j);
			crypt_write32(REG_P70, record[j]);
			crypt_wait();
		}
	}
	/* Prepare data for next iteration */
	record[0] = GET_L32(x2);
	record[1] = GET_H32(x2);
	record[2] = GET_L32(x3);
	record[3] = GET_H32(x3);
	return MTK_SIP_E_SUCCESS;
}

/* Set key to hdcp */
uint64_t crypt_set_hdcp_key_num(uint32_t num)
{
	if (num > KEY_LEN)
		return MTK_SIP_E_INVALID_PARAM;

	crypt_write32(REG_P68, 0x6A);
	crypt_write32(REG_P69, 0x34 + 4 * num);
	crypt_wait();
	return MTK_SIP_E_SUCCESS;
}

/* Clear key in crypt engine */
uint64_t crypt_clear_hdcp_key(void)
{
	uint32_t i;

	for (i = 0; i < KEY_SIZE; i++)
		crypt_write32(REG_D20 + i, 0);
	return MTK_SIP_E_SUCCESS;
}
