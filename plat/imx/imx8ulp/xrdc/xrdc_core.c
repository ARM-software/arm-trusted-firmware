/*
 * Copyright 2020-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "xrdc_config.h"

#define XRDC_ADDR	0x292f0000
#define MRC_OFFSET	0x2000
#define MRC_STEP	0x200

#define XRDC_MGR_PAC_ID	  U(0)
#define XRDC_MGR_PAC_SLOT U(47)

enum xrdc_comp_type {
	MDA_TYPE = (1 << 16),
	MRC_TYPE = (2 << 16),
	PAC_TYPE = (3 << 16),
	MSC_TYPE = (4 << 16),
};

enum xrdc_pd_type {
	XRDC_AD_PD,
	XRDC_HIFI_PD,
	XRDC_AV_PD,
};

#define XRDC_TYPE_MASK (0x7 << 16)
#define XRDC_ID_MASK 0xFFFF
#define XRDC_ID(id) ((id) & XRDC_ID_MASK)

typedef bool (*xrdc_check_func)(enum xrdc_comp_type type, uint16_t id);

/* Access below XRDC needs enable PS 8
 * and HIFI clocks and release HIFI firstly
 */
uint32_t hifi_xrdc_list[] = {
	(MDA_TYPE | XRDC_ID(9)),
	(MRC_TYPE | XRDC_ID(7)),
	(MRC_TYPE | XRDC_ID(9)),
	(MRC_TYPE | XRDC_ID(11)),
};

/* Access below XRDC needs enable PS 16 firstly */
uint32_t av_periph_xrdc_list[] = {
	(MDA_TYPE | XRDC_ID(10)),
	(MDA_TYPE | XRDC_ID(11)),
	(MDA_TYPE | XRDC_ID(12)),
	(MDA_TYPE | XRDC_ID(13)),
	(MDA_TYPE | XRDC_ID(14)),
	(MDA_TYPE | XRDC_ID(15)),
	(MDA_TYPE | XRDC_ID(16)),

	(PAC_TYPE | XRDC_ID(2)),

	(MRC_TYPE | XRDC_ID(6)),
	(MRC_TYPE | XRDC_ID(8)),
	(MRC_TYPE | XRDC_ID(10)),

	(MSC_TYPE | XRDC_ID(1)),
	(MSC_TYPE | XRDC_ID(2)),
};

uint32_t imx8ulp_pac_slots[] = {
	61, 23, 53
};

uint32_t imx8ulp_msc_slots[] = {
	2, 1, 7
};

static int xrdc_config_mrc_w0_w1(uint32_t mrc_con, uint32_t region, uint32_t w0, uint32_t size)
{

	uint32_t w0_addr, w1_addr;

	w0_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20;
	w1_addr = w0_addr + 4;

	if ((size % 32) != 0) {
		return -EINVAL;
	}

	mmio_write_32(w0_addr, w0 & ~0x1f);
	mmio_write_32(w1_addr, w0 + size - 1);

	return 0;
}

static int xrdc_config_mrc_w2(uint32_t mrc_con, uint32_t region, uint32_t dxsel_all)
{
	uint32_t w2_addr;

	w2_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0x8;

	mmio_write_32(w2_addr, dxsel_all);

	return 0;
}

static int xrdc_config_mrc_w3_w4(uint32_t mrc_con, uint32_t region, uint32_t w3, uint32_t w4)
{
	uint32_t w3_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0xC;
	uint32_t w4_addr = w3_addr + 4;

	mmio_write_32(w3_addr, w3);
	mmio_write_32(w4_addr, w4);

	return 0;
}

static int xrdc_config_pac(uint32_t pac, uint32_t index, uint32_t dxacp)
{
	uint32_t w0_addr;
	uint32_t val;

	if (pac > 2U) {
		return -EINVAL;
	}

	/* Skip the PAC slot for XRDC MGR, use Sentinel configuration */
	if (pac == XRDC_MGR_PAC_ID && index == XRDC_MGR_PAC_SLOT) {
		return 0;
	}

	w0_addr = XRDC_ADDR + 0x1000 + 0x400 * pac + 0x8 * index;

	mmio_write_32(w0_addr, dxacp);

	val = mmio_read_32(w0_addr + 4);
	mmio_write_32(w0_addr + 4, val | BIT_32(31));

	return 0;
}

static int xrdc_config_msc(uint32_t msc, uint32_t index, uint32_t dxacp)
{
	uint32_t w0_addr;
	uint32_t val;

	if (msc > 2) {
		return -EINVAL;
	}

	w0_addr = XRDC_ADDR + 0x4000 + 0x400 * msc + 0x8 * index;

	mmio_write_32(w0_addr, dxacp);

	val = mmio_read_32(w0_addr + 4);
	mmio_write_32(w0_addr + 4, val | BIT_32(31));

	return 0;
}

static int xrdc_config_mda(uint32_t mda_con, uint32_t dom, enum xrdc_mda_sa sa)
{
	uint32_t w0_addr;
	uint32_t val;

	w0_addr = XRDC_ADDR + 0x800 + mda_con * 0x20;

	val = mmio_read_32(w0_addr);

	if (val & BIT_32(29)) {
		mmio_write_32(w0_addr, (val & (~0xFF)) | dom |
			      BIT_32(31) | 0x20 | ((sa & 0x3) << 6));
	} else {
		mmio_write_32(w0_addr, dom | BIT_32(31));
		mmio_write_32(w0_addr + 0x4, dom | BIT_32(31));
	}

	return 0;
}

static bool xrdc_check_pd(enum xrdc_comp_type type,
			  uint16_t id, enum xrdc_pd_type pd)
{
	unsigned int i, size;
	uint32_t item = type | XRDC_ID(id);
	uint32_t *list;

	if (pd == XRDC_HIFI_PD) {
		size = ARRAY_SIZE(hifi_xrdc_list);
		list = hifi_xrdc_list;
	} else if (pd == XRDC_AV_PD) {
		size = ARRAY_SIZE(av_periph_xrdc_list);
		list = av_periph_xrdc_list;
	} else {
		return false;
	}

	for (i = 0U; i < size; i++) {
		if (item == list[i]) {
			return true;
		}
	}

	return false;
}

static bool xrdc_check_lpav(enum xrdc_comp_type type, uint16_t id)
{
	return xrdc_check_pd(type, id, XRDC_AV_PD);
}

static bool xrdc_check_hifi(enum xrdc_comp_type type, uint16_t id)
{
	return xrdc_check_pd(type, id, XRDC_HIFI_PD);
}

static bool xrdc_check_ad(enum xrdc_comp_type type, uint16_t id)
{
	return (!xrdc_check_pd(type, id, XRDC_HIFI_PD) &&
			!xrdc_check_pd(type, id, XRDC_AV_PD));
}

static int xrdc_apply_config(xrdc_check_func check_func)
{
	unsigned int i, j;
	uint32_t val;

	for (i = 0U; i < ARRAY_SIZE(imx8ulp_mda); i++) {
		if (check_func(MDA_TYPE, imx8ulp_mda[i].mda_id)) {
			xrdc_config_mda(imx8ulp_mda[i].mda_id,
					imx8ulp_mda[i].did, imx8ulp_mda[i].sa);
		}
	}

	for (i = 0U; i < ARRAY_SIZE(imx8ulp_mrc); i++) {
		if (check_func(MRC_TYPE, imx8ulp_mrc[i].mrc_id)) {
			xrdc_config_mrc_w0_w1(imx8ulp_mrc[i].mrc_id,
					      imx8ulp_mrc[i].region_id,
					      imx8ulp_mrc[i].region_start,
					      imx8ulp_mrc[i].region_size);

			val = 0;
			for (j = 0U; j < DID_MAX; j++) {
				val |= imx8ulp_mrc[i].dsel[j] << (3 * j);
			}

			xrdc_config_mrc_w2(imx8ulp_mrc[i].mrc_id, imx8ulp_mrc[i].region_id, val);
			xrdc_config_mrc_w3_w4(imx8ulp_mrc[i].mrc_id, imx8ulp_mrc[i].region_id,
				0, imx8ulp_mrc[i].accset[0] | (imx8ulp_mrc[i].accset[1] << 16) | BIT_32(31));
		}
	}

	for (i = 0U; i < ARRAY_SIZE(imx8ulp_pdac); i++) {
		if (check_func(PAC_TYPE, imx8ulp_pdac[i].pac_msc_id)) {
			val = 0;
			for (j = 0U; j < DID_MAX; j++) {
				val |= imx8ulp_pdac[i].dsel[j] << (3 * j);
			}

			if (imx8ulp_pdac[i].slot_id == PAC_SLOT_ALL) {
				/* Apply to all slots*/
				for (j = 0U; j < imx8ulp_pac_slots[imx8ulp_pdac[i].pac_msc_id]; j++) {
					xrdc_config_pac(imx8ulp_pdac[i].pac_msc_id, j, val);
				}
			} else {
				if (imx8ulp_pdac[i].slot_id >= imx8ulp_pac_slots[imx8ulp_pdac[i].pac_msc_id]) {
					return -EINVAL;
				}

				xrdc_config_pac(imx8ulp_pdac[i].pac_msc_id, imx8ulp_pdac[i].slot_id, val);
			}
		}
	}

	for (i = 0U; i < ARRAY_SIZE(imx8ulp_msc); i++) {
		if (check_func(MSC_TYPE, imx8ulp_msc[i].pac_msc_id)) {
			val = 0;
			for (j = 0U; j < DID_MAX; j++) {
				val |= imx8ulp_msc[i].dsel[j] << (3 * j);
			}

			if (imx8ulp_msc[i].slot_id == MSC_SLOT_ALL) {
				/* Apply to all slots*/
				for (j = 0U; j < imx8ulp_msc_slots[imx8ulp_msc[i].pac_msc_id]; j++) {
					xrdc_config_msc(imx8ulp_msc[i].pac_msc_id, j, val);
				}
			} else {
				if (imx8ulp_msc[i].slot_id >= imx8ulp_msc_slots[imx8ulp_msc[i].pac_msc_id]) {
					return -EINVAL;
				}

				xrdc_config_msc(imx8ulp_msc[i].pac_msc_id, imx8ulp_msc[i].slot_id, val);
			}
		}
	}

	return 0;
}

int xrdc_apply_lpav_config(void)
{
	/* Configure PAC2 to allow to access PCC5 */
	xrdc_config_pac(2, 39, 0xe00000);

	/* Enable the eDMA2 MP clock for MDA16 access */
	mmio_write_32(IMX_PCC5_BASE + 0x0, 0xc0000000);
	return xrdc_apply_config(xrdc_check_lpav);
}

int xrdc_apply_hifi_config(void)
{
	return xrdc_apply_config(xrdc_check_hifi);
}

int xrdc_apply_apd_config(void)
{
	return xrdc_apply_config(xrdc_check_ad);
}

void xrdc_enable(void)
{
	mmio_write_32(XRDC_ADDR, BIT(14) | BIT(15) | BIT(0));
}
