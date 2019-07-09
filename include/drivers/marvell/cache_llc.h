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
#define L2X0_INV_WAY(ap)		(MVEBU_LLC_BASE(ap) + 0x77C)
#define L2X0_CLEAN_WAY(ap)		(MVEBU_LLC_BASE(ap) + 0x7BC)
#define L2X0_CLEAN_INV_WAY(ap)		(MVEBU_LLC_BASE(ap) + 0x7FC)
#define LLC_TC0_LOCK(ap)		(MVEBU_LLC_BASE(ap) + 0x920)

#define MASTER_LLC_CTRL			LLC_CTRL(MVEBU_AP0)
#define MASTER_L2X0_INV_WAY		L2X0_INV_WAY(MVEBU_AP0)
#define MASTER_LLC_TC0_LOCK		LLC_TC0_LOCK(MVEBU_AP0)

#define LLC_CTRL_EN			1
#define LLC_EXCLUSIVE_EN		0x100
#define LLC_WAY_MASK			0xFFFFFFFF

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
