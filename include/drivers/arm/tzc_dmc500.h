/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZC_DMC500_H
#define TZC_DMC500_H

#include <drivers/arm/tzc_common.h>
#include <lib/utils_def.h>

#define SI_STATUS_OFFSET				U(0x000)
#define SI_STATE_CTRL_OFFSET				U(0x030)
#define SI_FLUSH_CTRL_OFFSET				U(0x034)
#define SI_INT_CONTROL_OFFSET				U(0x048)

#define SI_INT_STATUS_OFFSET				U(0x004)
#define SI_TZ_FAIL_ADDRESS_LOW_OFFSET			U(0x008)
#define SI_TZ_FAIL_ADDRESS_HIGH_OFFSET			U(0x00c)
#define SI_FAIL_CONTROL_OFFSET				U(0x010)
#define SI_FAIL_ID_OFFSET				U(0x014)
#define SI_INT_CLR_OFFSET				U(0x04c)

/*
 * DMC-500 has 2 system interfaces each having a similar set of regs
 * to configure each interface.
 */
#define SI0_BASE					U(0x0000)
#define SI1_BASE					U(0x0200)

/* Bit positions of SIx_SI_STATUS */
#define SI_EMPTY_SHIFT					1
#define SI_STALL_ACK_SHIFT				0
#define SI_EMPTY_MASK					U(0x01)
#define SI_STALL_ACK_MASK				U(0x01)

/* Bit positions of SIx_SI_INT_STATUS */
#define PMU_REQ_INT_OVERFLOW_STATUS_SHIFT		18
#define FAILED_ACCESS_INT_OVERFLOW_STATUS_SHIFT		16
#define PMU_REQ_INT_STATUS_SHIFT			2
#define FAILED_ACCESS_INT_INFO_TZ_OVERLAP_STATUS_SHIFT	1
#define FAILED_ACCESS_INT_STATUS_SHIFT			0
#define PMU_REQ_INT_OVERFLOW_STATUS_MASK		U(0x1)
#define FAILED_ACCESS_INT_OVERFLOW_STATUS_MASK		U(0x1)
#define PMU_REQ_INT_STATUS_MASK				U(0x1)
#define FAILED_ACCESS_INT_INFO_TZ_OVERLAP_STATUS_MASK	U(0x1)
#define FAILED_ACCESS_INT_STATUS_MASK			U(0x1)

/* Bit positions of SIx_TZ_FAIL_CONTROL */
#define DIRECTION_SHIFT					24
#define NON_SECURE_SHIFT				21
#define PRIVILEGED_SHIFT				20
#define FAILED_ACCESS_INT_INFO_RANK_MASKED_SHIFT	3
#define FAILED_ACCESS_INT_INFO_UNMAPPED_SHIFT		2
#define FAILED_ACCESS_INT_TZ_FAIL_SHIFT			1
#define FAILED_ACCESS_INT_INFO_OUTSIDE_DEFAULT_SHIFT	0
#define DIRECTION_MASK					U(0x1)
#define NON_SECURE_MASK					U(0x1)
#define PRIVILEGED_MASK					U(0x1)
#define FAILED_ACCESS_INT_INFO_RANK_MASKED_MASK		U(0x1)
#define FAILED_ACCESS_INT_INFO_UNMAPPED_MASK		U(0x1)
#define FAILED_ACCESS_INT_TZ_FAIL_MASK			U(0x1)
#define FAILED_ACCESS_INT_INFO_OUTSIDE_DEFAULT_MASK	U(0x1)

/* Bit positions of SIx_FAIL_STATUS */
#define FAIL_ID_VNET_SHIFT				24
#define FAIL_ID_ID_SHIFT				0
#define FAIL_ID_VNET_MASK				U(0xf)
#define FAIL_ID_ID_MASK					U(0xffffff)

/* Bit positions of SIx_SI_STATE_CONTRL */
#define SI_STALL_REQ_GO					0x0
#define SI_STALL_REQ_STALL				0x1

/* Bit positions of SIx_SI_FLUSH_CONTROL */
#define SI_FLUSH_REQ_INACTIVE				0x0
#define SI_FLUSH_REQ_ACTIVE				0x1
#define SI_FLUSH_REQ_MASK				0x1

/* Bit positions of SIx_SI_INT_CONTROL */
#define PMU_REQ_INT_EN_SHIFT				2
#define OVERLAP_DETECT_INT_EN_SHIFT			1
#define FAILED_ACCESS_INT_EN_SHIFT			0
#define PMU_REQ_INT_EN_MASK				U(0x1)
#define OVERLAP_DETECT_INT_EN_MASK			U(0x1)
#define FAILED_ACCESS_INT_EN_MASK			U(0x1)
#define PMU_REQ_INT_EN					U(0x1)
#define OVERLAP_DETECT_INT_EN				U(0x1)
#define FAILED_ACCESS_INT_EN				U(0x1)

/* Bit positions of SIx_SI_INT_CLR */
#define PMU_REQ_OFLOW_CLR_SHIFT				18
#define FAILED_ACCESS_OFLOW_CLR_SHIFT			16
#define PMU_REQ_INT_CLR_SHIFT				2
#define FAILED_ACCESS_INT_CLR_SHIFT			0
#define PMU_REQ_OFLOW_CLR_MASK				U(0x1)
#define FAILED_ACCESS_OFLOW_CLR_MASK			U(0x1)
#define PMU_REQ_INT_CLR_MASK				U(0x1)
#define FAILED_ACCESS_INT_CLR_MASK			U(0x1)
#define PMU_REQ_OFLOW_CLR				U(0x1)
#define FAILED_ACCESS_OFLOW_CLR				U(0x1)
#define PMU_REQ_INT_CLR					U(0x1)
#define FAILED_ACCESS_INT_CLR				U(0x1)

/* Macro to get the correct base register for a system interface */
#define IFACE_OFFSET(sys_if)	((sys_if) ? SI1_BASE : SI0_BASE)

#define MAX_SYS_IF_COUNT				U(2)
#define MAX_REGION_VAL					8

/* DMC-500 supports striping across a max of 4 DMC instances */
#define MAX_DMC_COUNT					4

/* Consist of part_number_1 and part_number_0 */
#define DMC500_PERIPHERAL_ID				U(0x0450)

/* Filter enable bits in a TZC */
#define TZC_DMC500_REGION_ATTR_F_EN_MASK		U(0x1)

/* Length of registers for configuring each region */
#define TZC_DMC500_REGION_SIZE				U(0x018)

#ifndef __ASSEMBLER__

#include <stdint.h>

/*
 * Contains the base addresses of all the DMC instances.
 */
typedef struct tzc_dmc500_driver_data {
	uintptr_t dmc_base[MAX_DMC_COUNT];
	int dmc_count;
	unsigned int sys_if_count;
} tzc_dmc500_driver_data_t;

void tzc_dmc500_driver_init(const tzc_dmc500_driver_data_t *plat_driver_data);
void tzc_dmc500_configure_region0(unsigned int sec_attr,
				unsigned int nsaid_permissions);
void tzc_dmc500_configure_region(unsigned int region_no,
				unsigned long long region_base,
				unsigned long long region_top,
				unsigned int sec_attr,
				unsigned int nsaid_permissions);
void tzc_dmc500_set_action(unsigned int action);
void tzc_dmc500_config_complete(void);
int tzc_dmc500_verify_complete(void);


#endif /* __ASSEMBLER__ */
#endif /* TZC_DMC500_H */
