/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TZC_DMC500_H__
#define __TZC_DMC500_H__

#include <tzc_common.h>

#define SI_STATUS_OFFSET				0x000
#define SI_STATE_CTRL_OFFSET				0x030
#define SI_FLUSH_CTRL_OFFSET				0x034
#define SI_INT_CONTROL_OFFSET				0x048

#define SI_INT_STATUS_OFFSET				0x004
#define SI_TZ_FAIL_ADDRESS_LOW_OFFSET			0x008
#define SI_TZ_FAIL_ADDRESS_HIGH_OFFSET			0x00c
#define SI_FAIL_CONTROL_OFFSET				0x010
#define SI_FAIL_ID_OFFSET				0x014
#define SI_INT_CLR_OFFSET				0x04c

/*
 * DMC-500 has 2 system interfaces each having a similar set of regs
 * to configure each interface.
 */
#define SI0_BASE					0x0000
#define SI1_BASE					0x0200

/* Bit positions of SIx_SI_STATUS */
#define SI_EMPTY_SHIFT					0x01
#define SI_STALL_ACK_SHIFT				0x00
#define SI_EMPTY_MASK					0x01
#define SI_STALL_ACK_MASK				0x01

/* Bit positions of SIx_SI_INT_STATUS */
#define PMU_REQ_INT_OVERFLOW_STATUS_SHIFT		18
#define FAILED_ACCESS_INT_OVERFLOW_STATUS_SHIFT		16
#define PMU_REQ_INT_STATUS_SHIFT			2
#define FAILED_ACCESS_INT_INFO_TZ_OVERLAP_STATUS_SHIFT	1
#define FAILED_ACCESS_INT_STATUS_SHIFT			0
#define PMU_REQ_INT_OVERFLOW_STATUS_MASK		0x1
#define FAILED_ACCESS_INT_OVERFLOW_STATUS_MASK		0x1
#define PMU_REQ_INT_STATUS_MASK				0x1
#define FAILED_ACCESS_INT_INFO_TZ_OVERLAP_STATUS_MASK	0x1
#define FAILED_ACCESS_INT_STATUS_MASK			0x1

/* Bit positions of SIx_TZ_FAIL_CONTROL */
#define DIRECTION_SHIFT					24
#define NON_SECURE_SHIFT				21
#define PRIVILEGED_SHIFT				20
#define FAILED_ACCESS_INT_INFO_RANK_MASKED_SHIFT	3
#define FAILED_ACCESS_INT_INFO_UNMAPPED_SHIFT		2
#define FAILED_ACCESS_INT_TZ_FAIL_SHIFT			0x1
#define FAILED_ACCESS_INT_INFO_OUTSIDE_DEFAULT_SHIFT	0
#define DIRECTION_MASK					0x1
#define NON_SECURE_MASK					0x1
#define PRIVILEGED_MASK					0x1
#define FAILED_ACCESS_INT_INFO_RANK_MASKED_MASK		0x1
#define FAILED_ACCESS_INT_INFO_UNMAPPED_MASK		0x1
#define FAILED_ACCESS_INT_TZ_FAIL_MASK			1
#define FAILED_ACCESS_INT_INFO_OUTSIDE_DEFAULT_MASK	0x1

/* Bit positions of SIx_FAIL_STATUS */
#define FAIL_ID_VNET_SHIFT				24
#define FAIL_ID_ID_SHIFT				0
#define FAIL_ID_VNET_MASK				0xf
#define FAIL_ID_ID_MASK					0xffffff

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
#define PMU_REQ_INT_EN_MASK				0x1
#define OVERLAP_DETECT_INT_EN_MASK			0x1
#define FAILED_ACCESS_INT_EN_MASK			0x1
#define PMU_REQ_INT_EN					0x1
#define OVERLAP_DETECT_INT_EN				0x1
#define FAILED_ACCESS_INT_EN				0x1

/* Bit positions of SIx_SI_INT_CLR */
#define PMU_REQ_OFLOW_CLR_SHIFT				18
#define FAILED_ACCESS_OFLOW_CLR_SHIFT			16
#define PMU_REQ_INT_CLR_SHIFT				2
#define FAILED_ACCESS_INT_CLR_SHIFT			0
#define PMU_REQ_OFLOW_CLR_MASK				0x1
#define FAILED_ACCESS_OFLOW_CLR_MASK			0x1
#define PMU_REQ_INT_CLR_MASK				0x1
#define FAILED_ACCESS_INT_CLR_MASK			0x1
#define PMU_REQ_OFLOW_CLR				0x1
#define FAILED_ACCESS_OFLOW_CLR				0x1
#define PMU_REQ_INT_CLR					0x1
#define FAILED_ACCESS_INT_CLR				0x1

/* Macro to get the correct base register for a system interface */
#define IFACE_OFFSET(sys_if)	((sys_if) ? SI1_BASE : SI0_BASE)

#define MAX_SYS_IF_COUNT				2
#define MAX_REGION_VAL					8

/* DMC-500 supports striping across a max of 4 DMC instances */
#define MAX_DMC_COUNT					4

/* Consist of part_number_1 and part_number_0 */
#define DMC500_PERIPHERAL_ID				0x0450

/* Filter enable bits in a TZC */
#define TZC_DMC500_REGION_ATTR_F_EN_MASK		0x1

/* Length of registers for configuring each region */
#define TZC_DMC500_REGION_SIZE				0x018

#ifndef __ASSEMBLY__

#include <stdint.h>

/*
 * Contains the base addresses of all the DMC instances.
 */
typedef struct tzc_dmc500_driver_data {
	uintptr_t dmc_base[MAX_DMC_COUNT];
	int dmc_count;
} tzc_dmc500_driver_data_t;

void tzc_dmc500_driver_init(const tzc_dmc500_driver_data_t *plat_driver_data);
void tzc_dmc500_configure_region0(tzc_region_attributes_t sec_attr,
				unsigned int nsaid_permissions);
void tzc_dmc500_configure_region(int region_no,
				unsigned long long region_base,
				unsigned long long region_top,
				tzc_region_attributes_t sec_attr,
				unsigned int nsaid_permissions);
void tzc_dmc500_set_action(tzc_action_t action);
void tzc_dmc500_config_complete(void);
int tzc_dmc500_verify_complete(void);


#endif /* __ASSEMBLY__ */
#endif /* __TZC_DMC500_H__ */

