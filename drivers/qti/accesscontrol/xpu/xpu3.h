/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU3_H
#define XPU3_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MSA_DOMAIN QAD0_NS_DOMAIN
#define SPU_DOMAIN QAD1_NS_DOMAIN

#define XPU_ERR_SEC_CTX 0
#define XPU_ERR_NON_SEC_CTX 1
#define XPU3_RGn_GCR0_RG_SEC_SHIFT 0x8

#define XPU_RG_ALL 0x00ffffff
#define XPU_UMR_RG (XPU_RG_ALL - 1)

#define XPU_PROTECTION_STATIC 0x1

enum xpu {
	XPU_TYPE_NONE,
	XPU_TYPE_IPA_0_GSI_TOP,
	XPU_TYPE_SEC_CTRL_APU,
	XPU_TYPE_ANOC1_MPU,
	XPU_TYPE_ANOC2_MPU,
	XPU_TYPE_BIMC_MPU0,
	XPU_TYPE_MSS_NAV_MPU,
	XPU_TYPE_LLCC_BROADCAST_MPU,
	XPU_TYPE_AOSS_MPU,
	XPU_TYPE_GEMNOC_MS_MPU,
	XPU_TYPE_BOOT_ROM,
	XPU_TYPE_IMEM_MPU,
	XPU_TYPE_CNOC_SNOC_MPU,
	XPU_TYPE_MSS_Q6_MPU,
	XPU_TYPE_MSS_MPU,
	XPU_TYPE_BIMC_MPU1,
	XPU_TYPE_DC_NOC_NON_BROADCAST_MPU,
	XPU_TYPE_DC_NOC_SHRM_MPU,
	XPU_TYPE_PKA_APU,
	XPU_TYPE_IPC_MPU,
	XPU_TYPE_CNOC_GEMNOC_MPU,
	XPU_TYPE_CNOC2_SS_MPU,
	XPU_TYPE_WPSS_MPU,
	XPU_TYPE_AOSS_CNOC_MPU,
	XPU_TYPE_CNOC_SS_MPU,
	XPU_TYPE_COUNT,
};

enum domain_type {
	NO_DOMAIN = 0,
	APPS_NS_DOMAIN = BIT(0),
	APPS_S_DOMAIN = BIT(0) | BIT(XPU3_RGn_GCR0_RG_SEC_SHIFT),
	QAD0_NS_DOMAIN = BIT(1),
	QAD1_NS_DOMAIN = BIT(2),
};

enum device_type {
	DEVICE_MODEM = 30,
	DEVICE_MSS_NAV = 35,
};

struct rg_domain_ownership {
	uint32_t rg_num;
	enum domain_type owner_domain;
	uint32_t perm_r;
	uint32_t perm_w;
};

struct rg_partition_range {
	uint32_t rg_num;
	uintptr_t start_addr;
	uintptr_t end_addr;
};

struct xpu_instance {
	uintptr_t xpu_base_addr;
	uint64_t owner_arr_size;
	struct rg_domain_ownership *rg_owner;
	uint64_t part_range_arr_size;
	struct rg_partition_range *partition_range;
	enum xpu xpu_id;
	uint32_t flag;
};

struct mpu_ranges {
	const enum device_type device;
	uint8_t mpus_count;
	uint16_t device_prtn_cnt;
	struct xpu_instance *mpus;
};

struct xpu_err_pos_to_hal_map {
	uint32_t bit_mask;
	uint8_t xpu;
};

struct xpu_intr_reg_dtls {
	uintptr_t xpu_intr_reg_addr;
	uintptr_t xpu_intr_reg_mask;
};

struct xpu_base_addr_info {
	enum xpu e_xpu;
	uintptr_t base_addr;
	char *name;
};

void xpu_lock_down_assets(struct xpu_instance *xpus, uint8_t xpu_count);
int xpu_lock_down_assets_dynamic(struct xpu_instance *xpus, uint8_t xpu_count,
				 uint32_t xpu_id, uint32_t rg_num,
				 uint32_t perm_r, uint32_t perm_w);
void xpu_master_mpu_init(struct mpu_ranges *msm_mpu_ranges,
			 const uint32_t msm_mpu_ranges_count);

void xpu_print_log(void *ctx);

#endif /* XPU3_H */
