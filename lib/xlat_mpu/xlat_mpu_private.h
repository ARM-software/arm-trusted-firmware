/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_MPU_PRIVATE_H
#define XLAT_MPU_PRIVATE_H

#include <stdbool.h>

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <platform_def.h>

#if PLAT_XLAT_TABLES_DYNAMIC
/*
 * Private shifts and masks to access fields of an mmap attribute
 */
/* Dynamic or static */
#define MT_DYN_SHIFT		U(31)

/*
 * Memory mapping private attributes
 *
 * Private attributes not exposed in the public header.
 */

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

/* Calculate region-attributes byte for PRBAR part of MPU-region descriptor: */
uint64_t prbar_attr_value(uint32_t attr);
/* Calculate region-attributes byte for PRLAR part of MPU-region descriptor: */
uint64_t prlar_attr_value(uint32_t attr);
/* Calculates the attr value for a given PRBAR and PRLAR entry value: */
uint32_t region_attr(uint64_t prbar_attr, uint64_t prlar_attr);

#define PRBAR_PRLAR_ADDR_MASK	UL(0xffffffffffc0)
	/* mask for PRBAR & PRLAR MPU-region field */
/* MPU region attribute bit fields: */
#define PRBAR_SH_SHIFT		UL(4)
#define PRBAR_SH_MASK		UL(0x3)
#define PRBAR_AP_SHIFT		UL(2)
#define PRBAR_AP_MASK		UL(0x3)
#define PRBAR_XN_SHIFT		UL(1)
#define PRBAR_XN_MASK		UL(0x3)
#define PRLAR_NS_SHIFT		UL(4)
#define PRLAR_NS_MASK		UL(0x3)
#define PRBAR_ATTR_SHIFT	UL(0)
#define PRBAR_ATTR_MASK		UL(0x3f)
#define PRLAR_ATTR_SHIFT	UL(1)
#define PRLAR_ATTR_MASK		UL(0x7)
#define PRLAR_EN_SHIFT		UL(0)
#define PRLAR_EN_MASK		UL(0x1)
/* Aspects of the source attributes not defined elsewhere: */
#define MT_PERM_MASK		UL(0x1)
#define MT_SEC_MASK		UL(0x1)
#define MT_EXECUTE_MASK		UL(0x3)
#define MT_TYPE_SHIFT		UL(0)

extern uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

/*
 * Return the execute-never mask that will prevent instruction fetch at the
 * given translation regime.
 */
uint64_t xlat_arch_regime_get_xn_desc(int xlat_regime);

/* Print VA, PA, size and attributes of all regions in the mmap array. */
void xlat_mmap_print(const mmap_region_t *mmap);

/*
 * Print the current state of the translation tables by reading them from
 * memory.
 */
void xlat_tables_print(xlat_ctx_t *ctx);

/*
 * Returns a block/page table descriptor for the given level and attributes.
 */
uint64_t xlat_desc(const xlat_ctx_t *ctx, uint32_t attr,
		   unsigned long long addr_pa, unsigned int level);

/*
 * Architecture-specific initialization code.
 */

/* Returns the current Exception Level. The returned EL must be 1 or higher. */
unsigned int xlat_arch_current_el(void);

/*
 * Returns true if the MMU of the translation regime managed by the given
 * xlat_ctx_t is enabled, false otherwise.
 */
bool is_mpu_enabled_ctx(const xlat_ctx_t *ctx);

/*
 * Returns minimum virtual address space size supported by the architecture
 */
uintptr_t xlat_get_min_virt_addr_space_size(void);

#endif /* XLAT_MPU_PRIVATE_H */
