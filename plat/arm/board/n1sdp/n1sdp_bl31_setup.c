/*
 * Copyright (c) 2018-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/css/sds.h>
#include <drivers/arm/gic600_multichip.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>

#include "n1sdp_def.h"
#include "n1sdp_private.h"
#include <platform_def.h>

#define RT_OWNER 0

/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size which will be used to zero out the memory before
 * enabling the ECC capability as well as information
 * about multichip setup
 * 	- multichip mode
 * 	- secondary_count
 * 	- Local DDR size in GB, DDR memory in master board
 * 	- Remote DDR size in GB, DDR memory in secondary board
 */
struct n1sdp_plat_info {
	bool multichip_mode;
	uint8_t secondary_count;
	uint8_t local_ddr_size;
	uint8_t remote_ddr_size;
} __packed;

static scmi_channel_plat_info_t n1sdp_scmi_plat_info = {
	.scmi_mbx_mem = N1SDP_SCMI_PAYLOAD_BASE,
	.db_reg_addr = PLAT_CSS_MHU_BASE + CSS_SCMI_MHU_DB_REG_OFF,
	.db_preserve_mask = 0xfffffffe,
	.db_modify_mask = 0x1,
	.ring_doorbell = &mhu_ring_doorbell
};

static struct gic600_multichip_data n1sdp_multichip_data __init = {
	.base_addrs = {
		PLAT_ARM_GICD_BASE
	},
	.rt_owner = RT_OWNER,
	.chip_count = 1,
	.chip_addrs = {
		[RT_OWNER] = {
			PLAT_ARM_GICD_BASE >> 16,
			PLAT_ARM_GICD_BASE >> 16
		}
	},
	.spi_ids = {
		{PLAT_ARM_GICD_BASE, 32, 511},
		{PLAT_ARM_GICD_BASE, 512, 991}
	}
};

static uintptr_t n1sdp_multichip_gicr_frames[3] = {
	PLAT_ARM_GICR_BASE,
	PLAT_ARM_GICR_BASE + PLAT_ARM_REMOTE_CHIP_OFFSET,
	0
};

scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id)
{
	return &n1sdp_scmi_plat_info;
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	ops->pwr_domain_off = n1sdp_pwr_domain_off;
	return css_scmi_override_pm_ops(ops);
}

/*
 * N1SDP platform supports RDIMMs with ECC capability. To use the ECC
 * capability, the entire DDR memory space has to be zeroed out before
 * enabling the ECC bits in DMC620. Zeroing out several gigabytes of
 * memory from SCP is quite time consuming so the following function
 * is added to zero out the DDR memory from application processor which is
 * much faster compared to SCP. Local DDR memory is zeroed out during BL2
 * stage. If remote chip is connected, it's DDR memory is zeroed out here.
 */

void remote_dmc_ecc_setup(uint8_t remote_ddr_size)
{
	uint64_t remote_dram2_size;

	remote_dram2_size = (remote_ddr_size * 1024UL * 1024UL * 1024UL) -
				N1SDP_REMOTE_DRAM1_SIZE;
	/* multichip setup */
	INFO("Zeroing remote DDR memories\n");
	zero_normalmem((void *)N1SDP_REMOTE_DRAM1_BASE,
			N1SDP_REMOTE_DRAM1_SIZE);
	flush_dcache_range(N1SDP_REMOTE_DRAM1_BASE, N1SDP_REMOTE_DRAM1_SIZE);
	zero_normalmem((void *)N1SDP_REMOTE_DRAM2_BASE, remote_dram2_size);
	flush_dcache_range(N1SDP_REMOTE_DRAM2_BASE, remote_dram2_size);

	INFO("Enabling ECC on remote DMCs\n");
	/* Set DMCs to CONFIG state before writing ERR0CTLR0 register */
	mmio_write_32(N1SDP_REMOTE_DMC0_MEMC_CMD_REG,
			N1SDP_DMC_MEMC_CMD_CONFIG);
	mmio_write_32(N1SDP_REMOTE_DMC1_MEMC_CMD_REG,
			N1SDP_DMC_MEMC_CMD_CONFIG);

	/* Enable ECC in DMCs */
	mmio_setbits_32(N1SDP_REMOTE_DMC0_ERR0CTLR0_REG,
			N1SDP_DMC_ERR0CTLR0_ECC_EN);
	mmio_setbits_32(N1SDP_REMOTE_DMC1_ERR0CTLR0_REG,
			N1SDP_DMC_ERR0CTLR0_ECC_EN);

	/* Set DMCs to READY state */
	mmio_write_32(N1SDP_REMOTE_DMC0_MEMC_CMD_REG, N1SDP_DMC_MEMC_CMD_READY);
	mmio_write_32(N1SDP_REMOTE_DMC1_MEMC_CMD_REG, N1SDP_DMC_MEMC_CMD_READY);
}

void n1sdp_bl31_multichip_setup(void)
{
	plat_arm_override_gicr_frames(n1sdp_multichip_gicr_frames);
	gic600_multichip_init(&n1sdp_multichip_data);
}

void bl31_platform_setup(void)
{
	int ret;
	struct n1sdp_plat_info plat_info;

	ret = sds_init(SDS_SCP_AP_REGION_ID);
	if (ret != SDS_OK) {
		ERROR("SDS initialization failed\n");
		panic();
	}

	ret = sds_struct_read(SDS_SCP_AP_REGION_ID,
				N1SDP_SDS_PLATFORM_INFO_STRUCT_ID,
				N1SDP_SDS_PLATFORM_INFO_OFFSET,
				&plat_info,
				N1SDP_SDS_PLATFORM_INFO_SIZE,
				SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Error getting platform info from SDS\n");
		panic();
	}
	/* Validate plat_info SDS */
	if ((plat_info.local_ddr_size == 0)
		|| (plat_info.local_ddr_size > N1SDP_MAX_DDR_CAPACITY_GB)
		|| (plat_info.remote_ddr_size > N1SDP_MAX_DDR_CAPACITY_GB)
		|| (plat_info.secondary_count > N1SDP_MAX_SECONDARY_COUNT)) {
		ERROR("platform info SDS is corrupted\n");
		panic();
	}

	if (plat_info.multichip_mode) {
		n1sdp_multichip_data.chip_count = plat_info.secondary_count + 1;
		n1sdp_bl31_multichip_setup();
	}
	arm_bl31_platform_setup();

	/* Check if remote memory is present */
	if ((plat_info.multichip_mode) && (plat_info.remote_ddr_size != 0))
		remote_dmc_ecc_setup(plat_info.remote_ddr_size);
}

#if defined(SPD_spmd) && (SPMC_AT_EL3 == 0)
/*
 * A dummy implementation of the platform handler for Group0 secure interrupt.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	(void)intid;
	return -1;
}
#endif /*defined(SPD_spmd) && (SPMC_AT_EL3 == 0)*/
