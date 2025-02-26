/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_RIFSC_REGS_H
#define STM32MP_RIFSC_REGS_H

/* RIFSC offset register */
#define _RIFSC_RISC_CR				U(0x0)
#define _RIFSC_RISC_SECCFGR(id)			(U(0x10) + U(0x4) * ((id) / 32))
#define _RIFSC_RISC_PRIVCFGR(id)		(U(0x30) + U(0x4) * ((id) / 32))
#define _RIFSC_RISC_RCFGLOCKR(id)		(U(0x50) + U(0x4) * ((id) / 32))
#define _RIFSC_RISC_PERy_CIDCFGR(id)		(U(0x100) + U(0x8) * (id))
#define _RIFSC_RISC_PERy_SEMCR(id)		(U(0x104) + U(0x8) * (id))
#define _RIFSC_RIMC_CR				U(0xC00)
#define _RIFSC_RIMC_SR				U(0xC04)
#define _RIFSC_RIMC_ATTR(x)			(U(0xC10) + U(0x4) * (x))
#define _RIFSC_PPSR(x)				(U(0xFB0) + U(0x4) * (x))
#define _RIFSC_HWCFGR3				U(0xFE8)
#define _RIFSC_HWCFGR2				U(0xFEC)
#define _RIFSC_HWCFGR1				U(0xFF0)
#define _RIFSC_VERR				U(0xFF4)
#define _RFISC_IPIDR				U(0xFF8)
#define _RFISC_SIDR				U(0xFFC)

/* RIFSC_RIMC_ATTRx register fields */
#define RIFSC_RIMC_ATTRx_CIDSEL			BIT_32(2)
#define RIFSC_RIMC_ATTRx_MCID_MASK		GENMASK_32(6, 4)
#define RIFSC_RIMC_ATTRx_MCID_SHIFT		4
#define RIFSC_RIMC_ATTRx_MSEC			BIT_32(8)
#define RIFSC_RIMC_ATTRx_MPRIV			BIT_32(9)


/* RIFSC_RISC_PERy_CIDCFGR register fields */
#define _RIFSC_CIDCFGR_CFEN			BIT_32(0)
#define _RIFSC_CIDCFGR_SEM_EN			BIT_32(1)
#define _RIFSC_CIDCFGR_SCID_SHIFT		U(4)
#define _RIFSC_CIDCFGR_SCID_MASK		GENMASK_32(6, 4)
#define _RIFSC_CIDCFGR_SEML_SHIFT		U(16)
#define _RIFSC_CIDCFGR_SEML_MASK		GENMASK_32(23, 16)

/* RIFSC_RISC_PERy_SEMCR register fields */
#define _RIFSC_SEMCR_SEM_MUTEX			BIT_32(0)
#define _RIFSC_SEMCR_SEMCID_SHIFT		U(4)
#define _RIFSC_SEMCR_SEMCID_MASK		GENMASK_32(6, 4)

#endif /* STM32MP_RIFSC_REGS_H */
