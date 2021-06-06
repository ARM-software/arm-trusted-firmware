/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/css/sds.h>
#include <lib/mmio.h>
#include <lib/utils.h>

#include "n1sdp_def.h"
#include <plat/arm/common/plat_arm.h>

struct n1sdp_plat_info {
	bool multichip_mode;
	uint8_t secondary_count;
	uint8_t local_ddr_size;
	uint8_t remote_ddr_size;
} __packed;

/*
 * N1SDP platform supports RDIMMs with ECC capability. To use the ECC
 * capability, the entire DDR memory space has to be zeroed out before
 * enabling the ECC bits in DMC620. Zeroing out several gigabytes of
 * memory from SCP is quite time consuming so the following function
 * is added to zero out the DDR memory from application processor which is
 * much faster compared to SCP.
 */

void dmc_ecc_setup(uint8_t ddr_size_gb)
{
	uint64_t dram2_size;

	dram2_size = (ddr_size_gb * 1024UL * 1024UL * 1024UL) -
			ARM_DRAM1_SIZE;

	INFO("Zeroing DDR memories\n");
	zero_normalmem((void *)ARM_DRAM1_BASE, ARM_DRAM1_SIZE);
	flush_dcache_range(ARM_DRAM1_BASE, ARM_DRAM1_SIZE);
	zero_normalmem((void *)ARM_DRAM2_BASE, dram2_size);
	flush_dcache_range(ARM_DRAM2_BASE, dram2_size);

	INFO("Enabling ECC on DMCs\n");
	/* Set DMCs to CONFIG state before writing ERR0CTLR0 register */
	mmio_write_32(N1SDP_DMC0_MEMC_CMD_REG, N1SDP_DMC_MEMC_CMD_CONFIG);
	mmio_write_32(N1SDP_DMC1_MEMC_CMD_REG, N1SDP_DMC_MEMC_CMD_CONFIG);

	/* Enable ECC in DMCs */
	mmio_setbits_32(N1SDP_DMC0_ERR0CTLR0_REG, N1SDP_DMC_ERR0CTLR0_ECC_EN);
	mmio_setbits_32(N1SDP_DMC1_ERR0CTLR0_REG, N1SDP_DMC_ERR0CTLR0_ECC_EN);

	/* Set DMCs to READY state */
	mmio_write_32(N1SDP_DMC0_MEMC_CMD_REG, N1SDP_DMC_MEMC_CMD_READY);
	mmio_write_32(N1SDP_DMC1_MEMC_CMD_REG, N1SDP_DMC_MEMC_CMD_READY);
}

void bl2_platform_setup(void)
{
	int ret;
	struct n1sdp_plat_info plat_info;

	ret = sds_init();
	if (ret != SDS_OK) {
		ERROR("SDS initialization failed\n");
		panic();
	}

	ret = sds_struct_read(N1SDP_SDS_PLATFORM_INFO_STRUCT_ID,
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

	dmc_ecc_setup(plat_info.local_ddr_size);
	arm_bl2_platform_setup();
}
