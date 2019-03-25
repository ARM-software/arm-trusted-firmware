/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* LLC driver is the Last Level Cache (L3C) driver
 * for Marvell SoCs in AP806, AP807, and AP810
 */

#ifndef CACHE_LLC_H
#define CACHE_LLC_H

#define LLC_CTRL(ap)			(MVEBU_LLC_BASE(ap) + 0x100)
#define LLC_SYNC(ap)			(MVEBU_LLC_BASE(ap) + 0x700)
#define LLC_BANKED_MNT_AHR(ap)		(MVEBU_LLC_BASE(ap) + 0x724)
#define LLC_INV_WAY(ap)			(MVEBU_LLC_BASE(ap) + 0x77C)
#define LLC_BLK_ALOC(ap)		(MVEBU_LLC_BASE(ap) + 0x78c)
#define LLC_CLEAN_WAY(ap)		(MVEBU_LLC_BASE(ap) + 0x7BC)
#define LLC_CLEAN_INV_WAY(ap)		(MVEBU_LLC_BASE(ap) + 0x7FC)
#define LLC_TC0_LOCK(ap)		(MVEBU_LLC_BASE(ap) + 0x920)

#define MASTER_LLC_CTRL			LLC_CTRL(MVEBU_AP0)
#define MASTER_LLC_INV_WAY		LLC_INV_WAY(MVEBU_AP0)
#define MASTER_LLC_TC0_LOCK		LLC_TC0_LOCK(MVEBU_AP0)

#define LLC_CTRL_EN			1
#define LLC_EXCLUSIVE_EN		0x100
#define LLC_ALL_WAYS_MASK		0xFFFFFFFF

/* AP806/AP807 - 1MB 8-ways LLC */
#define LLC_WAYS			8
#define LLC_WAY_MASK			((1 << LLC_WAYS) - 1)
#define LLC_SIZE			(1024 * 1024)
#define LLC_WAY_SIZE			(LLC_SIZE / LLC_WAYS)


#ifndef __ASSEMBLER__
void llc_cache_sync(int ap_index);
void llc_flush_all(int ap_index);
void llc_clean_all(int ap_index);
void llc_inv_all(int ap_index);
void llc_disable(int ap_index);
void llc_enable(int ap_index, int excl_mode);
int llc_is_exclusive(int ap_index);
void llc_runtime_enable(int ap_index);
#endif

#endif /* CACHE_LLC_H */
