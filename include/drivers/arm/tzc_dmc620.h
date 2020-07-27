/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZC_DMC620_H
#define TZC_DMC620_H

#include <lib/utils_def.h>

/* DMC-620 memc register offsets */
#define DMC620_MEMC_STATUS	U(0x0000)
#define DMC620_MEMC_CMD		U(0x0008)

/* Mask value to check the status of memc_cmd register */
#define DMC620_MEMC_CMD_MASK	U(0x00000007)

/* memc_cmd register's action values */
#define DMC620_MEMC_CMD_GO	U(0x00000003)
#define DMC620_MEMC_CMD_EXECUTE	U(0x00000004)

/* Address offsets of access address next region 0 registers */
#define DMC620_ACC_ADDR_MIN_31_00_NEXT_BASE	U(0x0080)
#define DMC620_ACC_ADDR_MIN_47_32_NEXT_BASE	U(0x0084)
#define DMC620_ACC_ADDR_MAX_31_00_NEXT_BASE	U(0x0088)
#define DMC620_ACC_ADDR_MAX_47_32_NEXT_BASE	U(0x008c)

/* Length of one block of access address next register region */
#define DMC620_ACC_ADDR_NEXT_SIZE		U(0x0010)

/* Address offsets of access address next registers */
#define DMC620_ACC_ADDR_MIN_31_00_NEXT(region_no)	\
		(DMC620_ACC_ADDR_MIN_31_00_NEXT_BASE +	\
			((region_no) * DMC620_ACC_ADDR_NEXT_SIZE))
#define DMC620_ACC_ADDR_MIN_47_32_NEXT(region_no)	\
		(DMC620_ACC_ADDR_MIN_47_32_NEXT_BASE +	\
			((region_no) * DMC620_ACC_ADDR_NEXT_SIZE))
#define DMC620_ACC_ADDR_MAX_31_00_NEXT(region_no)	\
		(DMC620_ACC_ADDR_MAX_31_00_NEXT_BASE +	\
			((region_no) * DMC620_ACC_ADDR_NEXT_SIZE))
#define DMC620_ACC_ADDR_MAX_47_32_NEXT(region_no)	\
		(DMC620_ACC_ADDR_MAX_47_32_NEXT_BASE +	\
			((region_no) * DMC620_ACC_ADDR_NEXT_SIZE))

/* Number of TZC address regions in DMC-620 */
#define DMC620_ACC_ADDR_COUNT	U(8)
/* Width of access address registers */
#define DMC620_ACC_ADDR_WIDTH	U(32)

/* Peripheral ID registers offsets */
#define DMC620_PERIPHERAL_ID_0		U(0x1fe0)

/* Default values in id registers */
#define DMC620_PERIPHERAL_ID_0_VALUE	U(0x00000054)

/* Secure access region attributes. */
#define TZC_DMC620_REGION_NS_RD		U(0x00000001)
#define TZC_DMC620_REGION_NS_WR		U(0x00000002)
#define TZC_DMC620_REGION_NS_RDWR	\
	(TZC_DMC620_REGION_NS_RD | TZC_DMC620_REGION_NS_WR)
#define TZC_DMC620_REGION_S_RD		U(0x00000004)
#define TZC_DMC620_REGION_S_WR		U(0x00000008)
#define TZC_DMC620_REGION_S_RDWR	\
	(TZC_DMC620_REGION_S_RD | TZC_DMC620_REGION_S_WR)
#define TZC_DMC620_REGION_S_NS_RDWR	\
	(TZC_DMC620_REGION_NS_RDWR | TZC_DMC620_REGION_S_RDWR)

/*
 * Contains pointer to the base addresses of all the DMC-620 instances.
 * 'dmc_count' specifies the number of DMC base addresses contained in the
 * array pointed to by dmc_base.
 */
typedef struct tzc_dmc620_driver_data {
	const uintptr_t *dmc_base;
	const unsigned int dmc_count;
} tzc_dmc620_driver_data_t;

/*
 * Contains region base, region top addresses and corresponding attributes
 * for configuring TZC access region registers.
 */
typedef struct tzc_dmc620_acc_addr_data {
	const unsigned long long region_base;
	const unsigned long long region_top;
	const unsigned int sec_attr;
} tzc_dmc620_acc_addr_data_t;

/*
 * Contains platform specific data for configuring TZC region base and
 * region top address. 'acc_addr_count' specifies the number of
 * valid entries in 'plat_acc_addr_data' array.
 */
typedef struct tzc_dmc620_config_data {
	const tzc_dmc620_driver_data_t *plat_drv_data;
	const tzc_dmc620_acc_addr_data_t *plat_acc_addr_data;
	const uint8_t acc_addr_count;
} tzc_dmc620_config_data_t;

/* Function prototypes */
void arm_tzc_dmc620_setup(const tzc_dmc620_config_data_t *plat_config_data);

#endif /* TZC_DMC620_H */

