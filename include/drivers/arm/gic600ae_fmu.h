/*
 * Copyright (c) 2021-2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GIC600AE_FMU_H
#define GIC600AE_FMU_H

/*******************************************************************************
 * GIC600-AE FMU register offsets and constants
 ******************************************************************************/
#define GICFMU_ERRFR_LO		U(0x000)
#define GICFMU_ERRFR_HI		U(0x004)
#define GICFMU_ERRCTLR_LO	U(0x008)
#define GICFMU_ERRCTLR_HI	U(0x00C)
#define GICFMU_ERRSTATUS_LO	U(0x010)
#define GICFMU_ERRSTATUS_HI	U(0x014)
#define GICFMU_ERRGSR_LO	U(0xE00)
#define GICFMU_ERRGSR_HI	U(0xE04)
#define GICFMU_KEY		U(0xEA0)
#define GICFMU_PINGCTLR		U(0xEA4)
#define GICFMU_PINGNOW		U(0xEA8)
#define GICFMU_SMEN		U(0xEB0)
#define GICFMU_SMINJERR		U(0xEB4)
#define GICFMU_PINGMASK_LO	U(0xEC0)
#define GICFMU_PINGMASK_HI	U(0xEC4)
#define GICFMU_STATUS		U(0xF00)
#define GICFMU_ERRIDR		U(0xFC8)

/* ERRCTLR bits */
#define FMU_ERRCTLR_ED_BIT	BIT(0)
#define FMU_ERRCTLR_CE_EN_BIT	BIT(1)
#define FMU_ERRCTLR_UI_BIT	BIT(2)
#define FMU_ERRCTLR_CI_BIT	BIT(3)

/* SMEN constants */
#define FMU_SMEN_BLK_SHIFT	U(8)
#define FMU_SMEN_SMID_SHIFT	U(24)
#define FMU_SMEN_EN_BIT		BIT(0)

/* Error record IDs */
#define FMU_BLK_GICD		U(0)
#define FMU_BLK_SPICOL		U(1)
#define FMU_BLK_WAKERQ		U(2)
#define FMU_BLK_ITS0		U(4)
#define FMU_BLK_ITS1		U(5)
#define FMU_BLK_ITS2		U(6)
#define FMU_BLK_ITS3		U(7)
#define FMU_BLK_ITS4		U(8)
#define FMU_BLK_ITS5		U(9)
#define FMU_BLK_ITS6		U(10)
#define FMU_BLK_ITS7		U(11)
#define FMU_BLK_PPI0		U(12)
#define FMU_BLK_PPI1		U(13)
#define FMU_BLK_PPI2		U(14)
#define FMU_BLK_PPI3		U(15)
#define FMU_BLK_PPI4		U(16)
#define FMU_BLK_PPI5		U(17)
#define FMU_BLK_PPI6		U(18)
#define FMU_BLK_PPI7		U(19)
#define FMU_BLK_PPI8		U(20)
#define FMU_BLK_PPI9		U(21)
#define FMU_BLK_PPI10		U(22)
#define FMU_BLK_PPI11		U(23)
#define FMU_BLK_PPI12		U(24)
#define FMU_BLK_PPI13		U(25)
#define FMU_BLK_PPI14		U(26)
#define FMU_BLK_PPI15		U(27)
#define FMU_BLK_PPI16		U(28)
#define FMU_BLK_PPI17		U(29)
#define FMU_BLK_PPI18		U(30)
#define FMU_BLK_PPI19		U(31)
#define FMU_BLK_PPI20		U(32)
#define FMU_BLK_PPI21		U(33)
#define FMU_BLK_PPI22		U(34)
#define FMU_BLK_PPI23		U(35)
#define FMU_BLK_PPI24		U(36)
#define FMU_BLK_PPI25		U(37)
#define FMU_BLK_PPI26		U(38)
#define FMU_BLK_PPI27		U(39)
#define FMU_BLK_PPI28		U(40)
#define FMU_BLK_PPI29		U(41)
#define FMU_BLK_PPI30		U(42)
#define FMU_BLK_PPI31		U(43)
#define FMU_BLK_PRESENT_MASK	U(0xFFFFFFFFFFF)

/* Safety Mechamism limit */
#define FMU_SMID_GICD_MAX	U(33)
#define FMU_SMID_PPI_MAX	U(12)
#define FMU_SMID_ITS_MAX	U(14)
#define FMU_SMID_SPICOL_MAX	U(5)
#define FMU_SMID_WAKERQ_MAX	U(2)

/* MBIST Safety Mechanism ID */
#define GICD_MBIST_REQ_ERROR	U(23)
#define GICD_FMU_CLKGATE_ERROR	U(33)
#define PPI_MBIST_REQ_ERROR	U(10)
#define PPI_FMU_CLKGATE_ERROR	U(12)
#define ITS_MBIST_REQ_ERROR	U(13)
#define ITS_FMU_CLKGATE_ERROR	U(14)

/* ERRSTATUS bits */
#define FMU_ERRSTATUS_BLKID_SHIFT	U(32)
#define FMU_ERRSTATUS_BLKID_MASK	U(0xFF)
#define FMU_ERRSTATUS_V_BIT		BIT(30)
#define FMU_ERRSTATUS_UE_BIT		BIT(29)
#define FMU_ERRSTATUS_OV_BIT		BIT(27)
#define FMU_ERRSTATUS_CE_BITS		(BIT(25) | BIT(24))
#define FMU_ERRSTATUS_CLEAR		(FMU_ERRSTATUS_V_BIT | FMU_ERRSTATUS_UE_BIT | \
					 FMU_ERRSTATUS_OV_BIT | FMU_ERRSTATUS_CE_BITS)
#define FMU_ERRSTATUS_IERR_MASK		U(0xFF)
#define FMU_ERRSTATUS_IERR_SHIFT	U(8)
#define FMU_ERRSTATUS_SERR_MASK		U(0xFF)

/* PINGCTLR constants */
#define FMU_PINGCTLR_INTDIFF_SHIFT	U(16)
#define FMU_PINGCTLR_TIMEOUTVAL_SHIFT	U(4)
#define FMU_PINGCTLR_EN_BIT		BIT(0)

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <arch_helpers.h>

/*******************************************************************************
 * GIC600 FMU EL3 driver API
 ******************************************************************************/
uint64_t gic_fmu_read_errfr(uintptr_t base, unsigned int n);
uint64_t gic_fmu_read_errctlr(uintptr_t base, unsigned int n);
uint64_t gic_fmu_read_errstatus(uintptr_t base, unsigned int n);
uint64_t gic_fmu_read_errgsr(uintptr_t base);
uint32_t gic_fmu_read_pingctlr(uintptr_t base);
uint32_t gic_fmu_read_pingnow(uintptr_t base);
uint64_t gic_fmu_read_pingmask(uintptr_t base);
uint32_t gic_fmu_read_status(uintptr_t base);
uint32_t gic_fmu_read_erridr(uintptr_t base);
void gic_fmu_write_errctlr(uintptr_t base, unsigned int n, uint64_t val);
void gic_fmu_write_errstatus(uintptr_t base, unsigned int n, uint64_t val);
void gic_fmu_write_pingctlr(uintptr_t base, uint32_t val);
void gic_fmu_write_pingnow(uintptr_t base, uint32_t val);
void gic_fmu_write_smen(uintptr_t base, uint32_t val);
void gic_fmu_write_sminjerr(uintptr_t base, uint32_t val);
void gic_fmu_write_pingmask(uintptr_t base, uint64_t val);
void gic_fmu_disable_all_sm_blkid(uintptr_t base, unsigned int blkid);

void gic600_fmu_init(uint64_t base, uint64_t blk_present_mask, bool errctlr_ce_en, bool errctlr_ue_en);
void gic600_fmu_enable_ping(uint64_t base, uint64_t blk_present_mask,
		unsigned int timeout_val, unsigned int interval_diff);
void gic600_fmu_print_sm_info(uint64_t base, unsigned int blk, unsigned int smid);
int gic600_fmu_probe(uint64_t base, int *probe_data);
int gic600_fmu_ras_handler(uint64_t base, int probe_data);

#endif /* __ASSEMBLER__ */

#endif /* GIC600AE_FMU_H */
