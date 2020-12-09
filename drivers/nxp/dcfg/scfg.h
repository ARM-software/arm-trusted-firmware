/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SCFG_H
#define SCFG_H

#ifdef CONFIG_CHASSIS_2

/* SCFG register offsets */
#define SCFG_CORE0_SFT_RST_OFFSET	0x0130
#define SCFG_SNPCNFGCR_OFFSET		0x01A4
#define SCFG_CORESRENCR_OFFSET		0x0204
#define SCFG_RVBAR0_0_OFFSET		0x0220
#define SCFG_RVBAR0_1_OFFSET		0x0224
#define SCFG_COREBCR_OFFSET		0x0680
#define SCFG_RETREQCR_OFFSET		0x0424

#define SCFG_COREPMCR_OFFSET		0x042C
#define COREPMCR_WFIL2			0x1

#define SCFG_GIC400_ADDR_ALIGN_OFFSET	0x0188
#define SCFG_BOOTLOCPTRH_OFFSET		0x0600
#define SCFG_BOOTLOCPTRL_OFFSET		0x0604
#define SCFG_SCRATCHRW2_OFFSET		0x0608
#define SCFG_SCRATCHRW3_OFFSET		0x060C

/* SCFG bit fields */
#define SCFG_SNPCNFGCR_SECRDSNP		0x80000000
#define SCFG_SNPCNFGCR_SECWRSNP         0x40000000
#endif /* CONFIG_CHASSIS_2 */

#ifndef __ASSEMBLER__
#include <endian.h>
#include <lib/mmio.h>

#ifdef NXP_SCFG_BE
#define scfg_in32(a)		bswap32(mmio_read_32((uintptr_t)(a)))
#define scfg_out32(a, v)	mmio_write_32((uintptr_t)(a), bswap32(v))
#define scfg_setbits32(a, v)	mmio_setbits_32((uintptr_t)(a), v)
#define scfg_clrbits32(a, v)	mmio_clrbits_32((uintptr_t)(a), v)
#define scfg_clrsetbits32(a, clear, set)	\
				mmio_clrsetbits_32((uintptr_t)(a), clear, set)
#elif defined(NXP_GUR_LE)
#define scfg_in32(a)		mmio_read_32((uintptr_t)(a))
#define scfg_out32(a, v)	mmio_write_32((uintptr_t)(a), v)
#define scfg_setbits32(a, v)	mmio_setbits_32((uintptr_t)(a), v)
#define scfg_clrbits32(a, v)	mmio_clrbits_32((uintptr_t)(a), v)
#define scfg_clrsetbits32(a, clear, set)	\
				mmio_clrsetbits_32((uintptr_t)(a), clear, set)
#else
#error Please define CCSR SCFG register endianness
#endif
#endif	/*	__ASSEMBLER__	*/

#endif	/* SCFG_H */
