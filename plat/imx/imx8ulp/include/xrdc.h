/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8ULP_XRDC_H
#define IMX8ULP_XRDC_H

#define DID_MAX 8
#define PAC_SLOT_ALL 128
#define MSC_SLOT_ALL 8

enum xrdc_mda_sa {
	MDA_SA_S,
	MDA_SA_NS,
	MDA_SA_PT, /* pass through master's secure/nonsecure attribute */
};

struct xrdc_mda_config {
	uint16_t mda_id;
	uint16_t did;
	enum xrdc_mda_sa sa;
};

struct xrdc_pac_msc_config {
	uint16_t pac_msc_id;
	uint16_t slot_id;
	uint8_t dsel[DID_MAX];
};

struct xrdc_mrc_config {
	uint16_t mrc_id;
	uint16_t region_id;
	uint32_t region_start;
	uint32_t region_size;
	uint8_t dsel[DID_MAX];
	uint16_t accset[2];
};

/* APIs to apply and enable XRDC */
int xrdc_apply_lpav_config(void);
int xrdc_apply_hifi_config(void);
int xrdc_apply_apd_config(void);
void xrdc_enable(void);

#endif
