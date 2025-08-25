/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <drivers/arm/css/scmi.h>
#include <drivers/scmi.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define SCMI_CORE_PROTO_ID			0x82

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

static int scmi_ap_core_init(scmi_channel_t *ch)
{
	uint32_t version;
	int ret;

	ret = scmi_proto_version(ch, SCMI_CORE_PROTO_ID, &version);
	if (ret != SCMI_E_SUCCESS) {
		WARN("SCMI AP core protocol version message failed\n");
		return -1;
	}

	INFO("SCMI AP core protocol version 0x%x detected\n", version);

	return 0;
}

void plat_imx9_scmi_setup(void)
{
	channel.info = &sq_scmi_plat_info;
	channel.lock = IMX95_SCMI_LOCK_GET_INSTANCE;
	imx9_scmi_handle = scmi_init(&channel);
	if (imx9_scmi_handle == NULL) {
		ERROR("SCMI Initialization failed\n");
		panic();
	}
	if (scmi_ap_core_init(&channel) < 0) {
		ERROR("SCMI AP core protocol initialization failed\n");
		panic();
	}
}
