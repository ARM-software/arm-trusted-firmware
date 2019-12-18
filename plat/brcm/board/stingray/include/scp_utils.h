/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_UTILS_H
#define SCP_UTILS_H

#include <common/bl_common.h>
#include <lib/mmio.h>

#include <m0_cfg.h>

int plat_bcm_bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info);

bool is_crmu_alive(void);
bool bcm_scp_issue_sys_reset(void);

#define SCP_READ_CFG(cfg) mmio_read_32(CRMU_CFG_BASE + \
		offsetof(M0CFG, cfg))
#define SCP_WRITE_CFG(cfg, value) mmio_write_32(CRMU_CFG_BASE + \
		offsetof(M0CFG, cfg), value)

#define SCP_READ_CFG16(cfg) mmio_read_16(CRMU_CFG_BASE + \
		offsetof(M0CFG, cfg))
#define SCP_WRITE_CFG16(cfg, value) mmio_write_16(CRMU_CFG_BASE + \
		offsetof(M0CFG, cfg), value)

#define SCP_READ_CFG8(cfg) mmio_read_8(CRMU_CFG_BASE + \
		offsetof(M0CFG, cfg))
#define SCP_WRITE_CFG8(cfg, value) mmio_write_8(CRMU_CFG_BASE + \
		offsetof(M0CFG, cfg), value)
#endif
