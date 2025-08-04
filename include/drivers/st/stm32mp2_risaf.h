/*
 * Copyright (c) 2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_RISAF_H
#define STM32MP2_RISAF_H

#include <stdint.h>

#include <lib/utils_def.h>

/* RISAF general registers (base relative) */
#define _RISAF_CR			U(0x00)
#define _RISAF_SR			U(0x04)
#define _RISAF_KEYR			U(0x30)
#define _RISAF_HWCFGR			U(0xFF0)

/* RISAF general register field description */
/* _RISAF_CR register fields */
#define _RISAF_CR_GLOCK			BIT_32(0)
/* _RISAF_SR register fields */
#define _RISAF_SR_KEYVALID		BIT_32(0)
#define _RISAF_SR_KEYRDY		BIT_32(1)
#define _RISAF_SR_ENCDIS		BIT_32(2)
/* _RISAF_HWCFGR register fields */
#define _RISAF_HWCFGR_CFG1_SHIFT	0
#define _RISAF_HWCFGR_CFG1_MASK		GENMASK_32(7, 0)
#define _RISAF_HWCFGR_CFG2_SHIFT	8
#define _RISAF_HWCFGR_CFG2_MASK		GENMASK_32(15, 8)
#define _RISAF_HWCFGR_CFG3_SHIFT	16
#define _RISAF_HWCFGR_CFG3_MASK		GENMASK_32(23, 16)
#define _RISAF_HWCFGR_CFG4_SHIFT	24
#define _RISAF_HWCFGR_CFG4_MASK		GENMASK_32(31, 24)

/* RISAF region registers (base relative) */
#define _RISAF_REG_BASE			U(0x40)
#define _RISAF_REG_SIZE			U(0x40)
#define _RISAF_REG(n)			(_RISAF_REG_BASE + (((n) - 1) * _RISAF_REG_SIZE))
#define _RISAF_REG_CFGR_OFFSET		U(0x0)
#define _RISAF_REG_CFGR(n)		(_RISAF_REG(n) + _RISAF_REG_CFGR_OFFSET)
#define _RISAF_REG_STARTR_OFFSET	U(0x4)
#define _RISAF_REG_STARTR(n)		(_RISAF_REG(n) + _RISAF_REG_STARTR_OFFSET)
#define _RISAF_REG_ENDR_OFFSET		U(0x8)
#define _RISAF_REG_ENDR(n)		(_RISAF_REG(n) + _RISAF_REG_ENDR_OFFSET)
#define _RISAF_REG_CIDCFGR_OFFSET	U(0xC)
#define _RISAF_REG_CIDCFGR(n)		(_RISAF_REG(n) + _RISAF_REG_CIDCFGR_OFFSET)

/* RISAF region register field description */
/* _RISAF_REG_CFGR(n) register fields */
#define _RISAF_REG_CFGR_BREN_SHIFT	0
#define _RISAF_REG_CFGR_BREN		BIT_32(_RISAF_REG_CFGR_BREN_SHIFT)
#define _RISAF_REG_CFGR_SEC_SHIFT	8
#define _RISAF_REG_CFGR_SEC		BIT_32(_RISAF_REG_CFGR_SEC_SHIFT)
#define _RISAF_REG_CFGR_ENC_SHIFT	15
#define _RISAF_REG_CFGR_ENC		BIT_32(_RISAF_REG_CFGR_ENC_SHIFT)
#define _RISAF_REG_CFGR_PRIVC_SHIFT	16
#define _RISAF_REG_CFGR_PRIVC_MASK	GENMASK_32(23, 16)
#define _RISAF_REG_CFGR_ALL_MASK	(_RISAF_REG_CFGR_BREN | _RISAF_REG_CFGR_SEC | \
					 _RISAF_REG_CFGR_ENC | _RISAF_REG_CFGR_PRIVC_MASK)
/* _RISAF_REG_CIDCFGR(n) register fields */
#define _RISAF_REG_CIDCFGR_RDENC_SHIFT		0
#define _RISAF_REG_CIDCFGR_RDENC_MASK		GENMASK_32(7, 0)
#define _RISAF_REG_CIDCFGR_WRENC_SHIFT		16
#define _RISAF_REG_CIDCFGR_WRENC_MASK		GENMASK_32(23, 16)
#define _RISAF_REG_CIDCFGR_ALL_MASK		(_RISAF_REG_CIDCFGR_RDENC_MASK | \
						 _RISAF_REG_CIDCFGR_WRENC_MASK)

/* Device Tree related definitions */
#define DT_RISAF_COMPAT			"st,stm32-risaf"
#define DT_RISAF_REG_ID_MASK		U(0xF)
#define DT_RISAF_EN_SHIFT		5
#define DT_RISAF_EN_MASK		BIT_32(DT_RISAF_EN_SHIFT)
#define DT_RISAF_SEC_SHIFT		6
#define DT_RISAF_SEC_MASK		BIT_32(DT_RISAF_SEC_SHIFT)
#define DT_RISAF_ENC_SHIFT		7
#define DT_RISAF_ENC_MASK		BIT_32(DT_RISAF_ENC_SHIFT)
#define DT_RISAF_PRIV_SHIFT		8
#define DT_RISAF_PRIV_MASK		GENMASK_32(15, 8)
#define DT_RISAF_READ_SHIFT		16
#define DT_RISAF_READ_MASK		GENMASK_32(23, 16)
#define DT_RISAF_WRITE_SHIFT		24
#define DT_RISAF_WRITE_MASK		GENMASK_32(31, 24)

/* RISAF max properties */
#define RISAF_REGION_REG_SIZE		(4 * sizeof(uint32_t))
#define RISAF_REGION_PROTREG_SIZE	(1 * sizeof(uint32_t))
#define RISAF_TIMEOUT_1MS_IN_US		U(1000)

/* RISAF encryption key size in bytes */
#define RISAF_ENCRYPTION_KEY_SIZE_IN_BYTES	U(16)

struct stm32mp2_risaf_region {
	int instance;
	uint32_t cfg;
	uintptr_t addr;
	size_t len;
};

struct stm32mp2_risaf_platdata {
	uintptr_t *base;
	unsigned long *clock;
	uint32_t *granularity;
	struct stm32mp2_risaf_region *region;
	int nregions;
};

int stm32mp2_risaf_write_encryption_key(int instance, uint8_t *key);
int stm32mp2_risaf_lock(int instance);
int stm32mp2_risaf_is_locked(int instance, bool *state);
int stm32mp2_risaf_init(void);

#endif /* STM32MP2_RISAF_H */
