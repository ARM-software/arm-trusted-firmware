/*
 * Copyright (C) 2018-2020 Marvell International Ltd.
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
#define LLC_SECURE_CTRL(ap)		(MVEBU_LLC_BASE(ap) + 0x10C)
#define LLC_SYNC(ap)			(MVEBU_LLC_BASE(ap) + 0x700)
#define LLC_BANKED_MNT_AHR(ap)		(MVEBU_LLC_BASE(ap) + 0x724)
#define LLC_INV_WAY(ap)			(MVEBU_LLC_BASE(ap) + 0x77C)
#define LLC_BLK_ALOC(ap)		(MVEBU_LLC_BASE(ap) + 0x78c)
#define LLC_CLEAN_WAY(ap)		(MVEBU_LLC_BASE(ap) + 0x7BC)
#define LLC_CLEAN_INV_WAY(ap)		(MVEBU_LLC_BASE(ap) + 0x7FC)
#define LLC_TCN_LOCK(ap, tc)		(MVEBU_LLC_BASE(ap) + 0x920 + 4 * (tc))

#define MASTER_LLC_CTRL			LLC_CTRL(MVEBU_AP0)
#define MASTER_LLC_INV_WAY		LLC_INV_WAY(MVEBU_AP0)
#define MASTER_LLC_TC0_LOCK		LLC_TCN_LOCK(MVEBU_AP0, 0)

#define LLC_CTRL_EN			1
#define LLC_EXCLUSIVE_EN		0x100
#define LLC_ALL_WAYS_MASK		0xFFFFFFFF

/* AP806/AP807 - 1MB 8-ways LLC */
#define LLC_WAYS			8
#define LLC_WAY_MASK			((1 << LLC_WAYS) - 1)
#define LLC_SIZE			(1024 * 1024)
#define LLC_WAY_SIZE			(LLC_SIZE / LLC_WAYS)
#define LLC_TC_NUM			15

#define LLC_BLK_ALOC_WAY_ID(way)	((way) & 0x1f)
#define LLC_BLK_ALOC_WAY_DATA_DSBL	(0x0 << 6)
#define LLC_BLK_ALOC_WAY_DATA_CLR	(0x1 << 6)
#define LLC_BLK_ALOC_WAY_DATA_SET	(0x3 << 6)
#define LLC_BLK_ALOC_BASE_ADDR(addr)	((addr) & ~(LLC_WAY_SIZE - 1))

#ifndef __ASSEMBLER__
void llc_cache_sync(int ap_index);
void llc_flush_all(int ap_index);
void llc_clean_all(int ap_index);
void llc_inv_all(int ap_index);
void llc_disable(int ap_index);
void llc_enable(int ap_index, int excl_mode);
int llc_is_exclusive(int ap_index);
void llc_runtime_enable(int ap_index);
#if LLC_SRAM
int llc_sram_enable(int ap_index, int size);
void llc_sram_disable(int ap_index);
int llc_sram_test(int ap_index, int size, char *msg);
#endif /* LLC_SRAM */
#endif /* __ASSEMBLER__ */

#endif /* CACHE_LLC_H */
