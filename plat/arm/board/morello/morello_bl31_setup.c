/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/css/sds.h>
#include <lib/cassert.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>

#include "morello_def.h"
#include <platform_def.h>

#ifdef TARGET_PLATFORM_FVP
/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size
 *	- Local DDR size in bytes, DDR memory in main board
 */
struct morello_plat_info {
	uint64_t local_ddr_size;
} __packed;
#else
/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size which is an information about multichip setup
 *	- Local DDR size in bytes, DDR memory in main board
 *	- Remote DDR size in bytes, DDR memory in remote board
 *	- remote_chip_count
 *	- multichip mode
 *	- scc configuration
 */
struct morello_plat_info {
	uint64_t local_ddr_size;
	uint64_t remote_ddr_size;
	uint8_t remote_chip_count;
	bool multichip_mode;
	uint32_t scc_config;
} __packed;
#endif

/* Compile time assertion to ensure the size of structure is 18 bytes */
CASSERT(sizeof(struct morello_plat_info) == MORELLO_SDS_PLATFORM_INFO_SIZE,
		assert_invalid_plat_info_size);

static scmi_channel_plat_info_t morello_scmi_plat_info = {
	.scmi_mbx_mem = MORELLO_SCMI_PAYLOAD_BASE,
	.db_reg_addr = PLAT_CSS_MHU_BASE + CSS_SCMI_MHU_DB_REG_OFF,
	.db_preserve_mask = 0xfffffffe,
	.db_modify_mask = 0x1,
	.ring_doorbell = &mhu_ring_doorbell
};

scmi_channel_plat_info_t *plat_css_get_scmi_info(int channel_id)
{
	return &morello_scmi_plat_info;
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}

#ifdef TARGET_PLATFORM_SOC
/*
 * Morello platform supports RDIMMs with ECC capability. To use the ECC
 * capability, the entire DDR memory space has to be zeroed out before
 * enabling the ECC bits in DMC-Bing. Zeroing out several gigabytes of
 * memory from SCP is quite time consuming so the following function
 * is added to zero out the DDR memory from application processor which is
 * much faster compared to SCP.
 */

static void dmc_ecc_setup(struct morello_plat_info *plat_info)
{
	uint64_t dram2_size;
	uint32_t val;
	uint64_t tag_mem_base;
	uint64_t usable_mem_size;

	INFO("Total DIMM size: %uGB\n",
			(uint32_t)(plat_info->local_ddr_size / 0x40000000));

	assert(plat_info->local_ddr_size > ARM_DRAM1_SIZE);
	dram2_size = plat_info->local_ddr_size - ARM_DRAM1_SIZE;

	INFO("Zeroing DDR memory range 0x%llx - 0x%llx\n",
		ARM_DRAM2_BASE, ARM_DRAM2_BASE + dram2_size);
	zero_normalmem((void *)ARM_DRAM2_BASE, dram2_size);
	flush_dcache_range(ARM_DRAM2_BASE, dram2_size);

	/* Clear previous ECC errors while zeroing out the memory */
	val = mmio_read_32(MORELLO_DMC0_ERR2STATUS_REG);
	mmio_write_32(MORELLO_DMC0_ERR2STATUS_REG, val);

	val = mmio_read_32(MORELLO_DMC1_ERR2STATUS_REG);
	mmio_write_32(MORELLO_DMC1_ERR2STATUS_REG, val);

	/* Set DMCs to CONFIG state before writing ERR0CTLR0 register */
	mmio_write_32(MORELLO_DMC0_MEMC_CMD_REG, MORELLO_DMC_MEMC_CMD_CONFIG);
	mmio_write_32(MORELLO_DMC1_MEMC_CMD_REG, MORELLO_DMC_MEMC_CMD_CONFIG);

	while ((mmio_read_32(MORELLO_DMC0_MEMC_STATUS_REG) &
			MORELLO_DMC_MEMC_STATUS_MASK) !=
			MORELLO_DMC_MEMC_CMD_CONFIG) {
		continue;
	}

	while ((mmio_read_32(MORELLO_DMC1_MEMC_STATUS_REG) &
			MORELLO_DMC_MEMC_STATUS_MASK) !=
			MORELLO_DMC_MEMC_CMD_CONFIG) {
		continue;
	}

	/* Configure Bing client/server mode based on SCC configuration */
	if (plat_info->scc_config & MORELLO_SCC_CLIENT_MODE_MASK) {
		INFO("Configuring DMC Bing in client mode\n");
		usable_mem_size = plat_info->local_ddr_size -
			(plat_info->local_ddr_size / 128ULL);

		/* Linear DDR address */
		tag_mem_base = usable_mem_size;
		tag_mem_base = tag_mem_base / 4;

		/* Reverse translation */
		if (tag_mem_base < ARM_DRAM1_BASE) {
			tag_mem_base += ARM_DRAM1_BASE;
		} else {
			tag_mem_base = tag_mem_base - ARM_DRAM1_BASE +
				ARM_DRAM2_BASE;
		}

		mmio_write_32(MORELLO_DMC0_CAP_CTRL_REG, 0x1);
		mmio_write_32(MORELLO_DMC1_CAP_CTRL_REG, 0x1);
		mmio_write_32(MORELLO_DMC0_TAG_CACHE_CFG, 0x1);
		mmio_write_32(MORELLO_DMC1_TAG_CACHE_CFG, 0x1);

		if (plat_info->scc_config & MORELLO_SCC_C1_TAG_CACHE_EN_MASK) {
			mmio_setbits_32(MORELLO_DMC0_TAG_CACHE_CFG, 0x2);
			mmio_setbits_32(MORELLO_DMC1_TAG_CACHE_CFG, 0x2);
			INFO("C1 Tag Cache Enabled\n");
		}

		if (plat_info->scc_config & MORELLO_SCC_C2_TAG_CACHE_EN_MASK) {
			mmio_setbits_32(MORELLO_DMC0_TAG_CACHE_CFG, 0x4);
			mmio_setbits_32(MORELLO_DMC1_TAG_CACHE_CFG, 0x4);
			INFO("C2 Tag Cache Enabled\n");
		}

		mmio_write_32(MORELLO_DMC0_MEM_ADDR_CTL,
				(uint32_t)tag_mem_base);
		mmio_write_32(MORELLO_DMC1_MEM_ADDR_CTL,
				(uint32_t)tag_mem_base);
		mmio_write_32(MORELLO_DMC0_MEM_ADDR_CTL2,
				(uint32_t)(tag_mem_base >> 32));
		mmio_write_32(MORELLO_DMC1_MEM_ADDR_CTL2,
				(uint32_t)(tag_mem_base >> 32));

		mmio_setbits_32(MORELLO_DMC0_MEM_ACCESS_CTL,
				MORELLO_DMC_MEM_ACCESS_DIS);
		mmio_setbits_32(MORELLO_DMC1_MEM_ACCESS_CTL,
				MORELLO_DMC_MEM_ACCESS_DIS);

		INFO("Tag base set to 0x%lx\n", tag_mem_base);
		plat_info->local_ddr_size = usable_mem_size;
	} else {
		INFO("Configuring DMC Bing in server mode\n");
		mmio_write_32(MORELLO_DMC0_CAP_CTRL_REG, 0x0);
		mmio_write_32(MORELLO_DMC1_CAP_CTRL_REG, 0x0);
	}

	INFO("Enabling ECC on DMCs\n");
	/* Enable ECC in DMCs */
	mmio_setbits_32(MORELLO_DMC0_ERR0CTLR0_REG,
		MORELLO_DMC_ERR0CTLR0_ECC_EN);
	mmio_setbits_32(MORELLO_DMC1_ERR0CTLR0_REG,
		MORELLO_DMC_ERR0CTLR0_ECC_EN);

	/* Set DMCs to READY state */
	mmio_write_32(MORELLO_DMC0_MEMC_CMD_REG, MORELLO_DMC_MEMC_CMD_READY);
	mmio_write_32(MORELLO_DMC1_MEMC_CMD_REG, MORELLO_DMC_MEMC_CMD_READY);

	while ((mmio_read_32(MORELLO_DMC0_MEMC_STATUS_REG) &
			MORELLO_DMC_MEMC_STATUS_MASK) !=
			MORELLO_DMC_MEMC_CMD_READY) {
		continue;
	}

	while ((mmio_read_32(MORELLO_DMC1_MEMC_STATUS_REG) &
			MORELLO_DMC_MEMC_STATUS_MASK) !=
			MORELLO_DMC_MEMC_CMD_READY) {
		continue;
	}
}
#endif

void bl31_platform_setup(void)
{
	int ret;
	struct morello_plat_info plat_info;

	ret = sds_init();
	if (ret != SDS_OK) {
		ERROR("SDS initialization failed. ret:%d\n", ret);
		panic();
	}

	ret = sds_struct_read(MORELLO_SDS_PLATFORM_INFO_STRUCT_ID,
				MORELLO_SDS_PLATFORM_INFO_OFFSET,
				&plat_info,
				MORELLO_SDS_PLATFORM_INFO_SIZE,
				SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Error getting platform info from SDS. ret:%d\n", ret);
		panic();
	}

	/* Validate plat_info SDS */
#ifdef TARGET_PLATFORM_FVP
	if (plat_info.local_ddr_size == 0U) {
#else
	if ((plat_info.local_ddr_size == 0U)
		|| (plat_info.local_ddr_size > MORELLO_MAX_DDR_CAPACITY)
		|| (plat_info.remote_ddr_size > MORELLO_MAX_DDR_CAPACITY)
		|| (plat_info.remote_chip_count > MORELLO_MAX_REMOTE_CHIP_COUNT)
		) {
#endif
		ERROR("platform info SDS is corrupted\n");
		panic();
	}

	arm_bl31_platform_setup();

#ifdef TARGET_PLATFORM_SOC
	dmc_ecc_setup(&plat_info);
#endif
}
