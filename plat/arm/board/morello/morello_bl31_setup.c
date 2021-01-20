/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/arm/css/sds.h>
#include <lib/cassert.h>
#include <plat/arm/common/plat_arm.h>

#include "morello_def.h"
#include <platform_def.h>

/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size which is an information about multichip setup
 *	- Local DDR size in bytes, DDR memory in master board
 *	- Remote DDR size in bytes, DDR memory in slave board
 *	- slave_count
 *	- multichip mode
 */
struct morello_plat_info {
	uint64_t local_ddr_size;
	uint64_t remote_ddr_size;
	uint8_t slave_count;
	bool multichip_mode;
} __packed;

/* Compile time assertion to ensure the size of structure is 18 bytes */
CASSERT(sizeof(struct morello_plat_info) == MORELLO_SDS_PLATFORM_INFO_SIZE,
		assert_invalid_plat_info_size);
/*
 * BL33 image information structure stored in SDS.
 * This structure holds the source & destination addresses and
 * the size of the BL33 image which will be loaded by BL31.
 */
struct morello_bl33_info {
	uint32_t bl33_src_addr;
	uint32_t bl33_dst_addr;
	uint32_t bl33_size;
};

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

static void copy_bl33(uint32_t src, uint32_t dst, uint32_t size)
{
	unsigned int i;

	INFO("Copying BL33 to DDR memory...\n");
	for (i = 0U; i < size; (i = i + 8U))
		mmio_write_64((dst + i), mmio_read_64(src + i));

	for (i = 0U; i < size; (i = i + 8U)) {
		if (mmio_read_64(src + i) != mmio_read_64(dst + i)) {
			ERROR("Copy failed!\n");
			panic();
		}
	}
	INFO("done\n");
}

void bl31_platform_setup(void)
{
	int ret;
	struct morello_plat_info plat_info;
	struct morello_bl33_info bl33_info;
	struct morello_plat_info *copy_dest;

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
	if ((plat_info.local_ddr_size == 0U)
		|| (plat_info.local_ddr_size > MORELLO_MAX_DDR_CAPACITY)
		|| (plat_info.remote_ddr_size > MORELLO_MAX_DDR_CAPACITY)
		|| (plat_info.slave_count > MORELLO_MAX_SLAVE_COUNT)) {
		ERROR("platform info SDS is corrupted\n");
		panic();
	}

	arm_bl31_platform_setup();

	ret = sds_struct_read(MORELLO_SDS_BL33_INFO_STRUCT_ID,
				MORELLO_SDS_BL33_INFO_OFFSET,
				&bl33_info,
				MORELLO_SDS_BL33_INFO_SIZE,
				SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Error getting BL33 info from SDS. ret:%d\n", ret);
		panic();
	}
	copy_bl33(bl33_info.bl33_src_addr,
			bl33_info.bl33_dst_addr,
			bl33_info.bl33_size);
	/*
	 * Pass platform information to BL33. This method is followed as
	 * currently there is no BL1/BL2 involved in boot flow of MORELLO.
	 * When TBBR is implemented for MORELLO, this method should be removed
	 * and platform information should be passed to BL33 using NT_FW_CONFIG
	 * passing mechanism.
	 */
	copy_dest = (struct morello_plat_info *)MORELLO_PLATFORM_INFO_BASE;
	*copy_dest = plat_info;
}
