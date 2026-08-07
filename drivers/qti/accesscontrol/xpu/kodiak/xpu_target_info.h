/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XPU_TARGET_INFO_H
#define XPU_TARGET_INFO_H

#include <xpu3.h>

#include <kodiak_def.h>

#define ACC_XPU_ERR_INT_REG_NUM 2
#define ACC_XPU_ERR_NUM_PER_REG 32

#define XPU_ADDR_TYPE(hwiosym) HWIO_##hwiosym##_ADDR

#define HWIO_OCIMEM_MPU_XPU3_GCR0_ADDR 0x061c000
#define HWIO_BOOT_ROM_XPU3_GCR0_ADDR 0x03fe000
#define HWIO_IPA_0_GSI_TOP_XPU3_GCR0_ADDR 0x01e00000
#define HWIO_SNOC_AGGRE1_MS_MPU_XPU3_GCR0_ADDR 0x016b0000
#define HWIO_SNOC_AGGRE2_MS_MPU_XPU3_GCR0_ADDR 0x016a8000
#define HWIO_LLCC0_LLCC_XPU3_GCR0_ADDR 0x0922e000
#define HWIO_LLCC1_LLCC_XPU3_GCR0_ADDR 0x092ae000
#define HWIO_MODEM_MS_MPU_XPU3_GCR0_ADDR 0x091fe000
#define HWIO_LLCC_BROADCAST_LLCC_XPU3_GCR0_ADDR 0x0962e000
#define HWIO_SNOC_NAV_MS_MPU_XPU3_GCR0_ADDR 0x016a0000
#define HWIO_AOSS_MPU_XPU3_GCR0_ADDR 0x01522000
#define HWIO_SEC_CTRL_APU_XPU3_GCR0_ADDR 0x0078e000
#define HWIO_MDSP_MS_MPU_XPU3_GCR0_ADDR 0x091fc000
#define HWIO_AOSS_CNOC_MPU_XPU3_GCR0_ADDR 0x0eff0000
#define HWIO_QHM_SHRM_MPU_XPU3_GCR0_ADDR 0x090c2000
#define HWIO_PKA_WRAPPER_XPU3_GCR0_ADDR 0x000f0000
#define HWIO_IPC_XPU3_GCR0_ADDR 0x004fe000
#define HWIO_QHS_NON_BROADCAST_MPU_XPU3_GCR0_ADDR 0x090c4000
#define HWIO_CNOC_GEMNOC_MPU_XPU3_GCR0_ADDR 0x0152c000
#define HWIO_CNOC2_SS_MPU_XPU3_GCR0_ADDR 0x01530000
#define HWIO_WPSS_MPU_XPU3_GCR0_ADDR 0x08b14000

#define TCSR_TCSR_REGS_REG_BASE (QTI_CORE_TOP_CSR_BASE + 0x000c0000)

#define HWIO_TCSR_XPU3_NON_SEC_IRQ_STATUS_REG_0_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00002000)
#define HWIO_TCSR_XPU3_NON_SEC_IRQ_STATUS_REG_0_RMSK 0xffffffff

#define HWIO_TCSR_XPU3_NON_SEC_IRQ_STATUS_REG_1_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00002004)
#define HWIO_TCSR_XPU3_NON_SEC_IRQ_STATUS_REG_1_RMSK 0xffffffff

#define HWIO_TCSR_XPU3_SEC_IRQ_STATUS_REG_0_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00004000)
#define HWIO_TCSR_XPU3_SEC_IRQ_STATUS_REG_0_RMSK 0xffffffff

#define HWIO_TCSR_XPU3_SEC_IRQ_STATUS_REG_1_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00004004)
#define HWIO_TCSR_XPU3_SEC_IRQ_STATUS_REG_1_RMSK 0xffffffff

#define HWIO_TCSR_XPU3_NON_SEC_IRQ_ENABLE_0_REG_0_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00002040)
#define HWIO_TCSR_XPU3_NON_SEC_IRQ_ENABLE_0_REG_0_RMSK 0xffffffff

#define HWIO_TCSR_XPU3_NON_SEC_IRQ_ENABLE_0_REG_1_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00002044)
#define HWIO_TCSR_XPU3_NON_SEC_IRQ_ENABLE_0_REG_1_RMSK 0xffffffff

#define HWIO_TCSR_XPU3_SEC_IRQ_ENABLE_0_REG_0_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00004040)
#define HWIO_TCSR_XPU3_SEC_IRQ_ENABLE_0_REG_0_RMSK 0xffffffff

#define HWIO_TCSR_XPU3_SEC_IRQ_ENABLE_0_REG_1_ADDR \
	(TCSR_TCSR_REGS_REG_BASE + 0x00004044)
#define HWIO_TCSR_XPU3_SEC_IRQ_ENABLE_0_REG_1_RMSK 0xffffffff

#define NUM_MODEM_MPU_PARTITIONS 7
#define NUM_MSS_NAV_MPU_PARTITIONS 8

extern struct xpu_err_pos_to_hal_map
	xpu_err_pos_to_hal_map[ACC_XPU_ERR_INT_REG_NUM][ACC_XPU_ERR_NUM_PER_REG];

extern const struct xpu_intr_reg_dtls
	xpu_non_sec_intr_status_reg[ACC_XPU_ERR_INT_REG_NUM];

extern const struct xpu_intr_reg_dtls
	xpu_sec_intr_status_reg[ACC_XPU_ERR_INT_REG_NUM];

extern const struct xpu_intr_reg_dtls
	xpu_non_sec_intr_en_reg[ACC_XPU_ERR_INT_REG_NUM];

extern const struct xpu_intr_reg_dtls
	xpu_sec_intr_en_reg[ACC_XPU_ERR_INT_REG_NUM];

extern struct xpu_instance msm_xpu_cfg[];
extern const uint32_t msm_xpu_cfg_count;

extern struct mpu_ranges msm_mpu_ranges[];
extern const uint32_t msm_mpu_ranges_count;

extern struct xpu_base_addr_info g_xpu_base_addr_array[];
extern uint32_t g_xpu_base_addr_array_count;

void xpu_configure_tz(void);

#endif /* XPU_TARGET_INFO_H */
