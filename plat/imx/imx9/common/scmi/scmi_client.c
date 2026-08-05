/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

#include <drivers/arm/css/scmi.h>
#include <drivers/scmi.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <scmi_imx9.h>

void *imx9_scmi_handle;

/* The SCMI channel global object */
static scmi_channel_t channel;

spinlock_t imx95_scmi_lock;
#define IMX95_SCMI_LOCK_GET_INSTANCE	(&imx95_scmi_lock)

static void mu_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	uint32_t db = mmio_read_32(plat_info->db_reg_addr) &
		      (plat_info->db_preserve_mask);

	mmio_write_32(plat_info->db_reg_addr, db | plat_info->db_modify_mask);
}

static scmi_channel_plat_info_t sq_scmi_plat_info = {
		.scmi_mbx_mem = IMX9_SCMI_PAYLOAD_BASE,
		.db_reg_addr = IMX9_MU1_BASE + MU_GCR_OFF,
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mu_ring_doorbell,
};

static int imx9_scmi_protocol_init(scmi_channel_t *ch)
{
	int ret;

	ret = scmi_sys_pwr_init(ch);
	if (ret != 0) {
		return ret;
	}

	return scmi_core_init(ch);
}

void plat_imx9_scmi_setup(void)
{
	channel.info = &sq_scmi_plat_info;
	channel.lock = IMX95_SCMI_LOCK_GET_INSTANCE;

	imx9_scmi_handle = scmi_init(&channel, imx9_scmi_protocol_init);
	if (imx9_scmi_handle == NULL) {
		ERROR("SCMI Initialization failed\n");
		panic();
	}
}
