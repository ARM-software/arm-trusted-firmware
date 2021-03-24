/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef BL31_DATA_H
#define	BL31_DATA_H

#define SECURE_DATA_BASE     NXP_OCRAM_ADDR
#define SECURE_DATA_SIZE     NXP_OCRAM_SIZE
#define SECURE_DATA_TOP      (SECURE_DATA_BASE + SECURE_DATA_SIZE)
#define SMC_REGION_SIZE      0x80
#define SMC_GLBL_BASE        (SECURE_DATA_TOP - SMC_REGION_SIZE)
#define BC_PSCI_DATA_SIZE    0xC0
#define BC_PSCI_BASE         (SMC_GLBL_BASE - BC_PSCI_DATA_SIZE)
#define SECONDARY_TOP        BC_PSCI_BASE

#define SEC_PSCI_DATA_SIZE   0xC0
#define SEC_REGION_SIZE      SEC_PSCI_DATA_SIZE

/* SMC global data */
#define BOOTLOC_OFFSET       0x0
#define BOOT_SVCS_OSET       0x8

/* offset to prefetch disable mask */
#define PREFETCH_DIS_OFFSET  0x10
/* must reference last smc global entry */
#define LAST_SMC_GLBL_OFFSET 0x18

#define SMC_TASK_OFFSET      0xC
#define TSK_START_OFFSET     0x0
#define TSK_DONE_OFFSET      0x4
#define TSK_CORE_OFFSET      0x8
#define SMC_TASK1_BASE       (SMC_GLBL_BASE + 32)
#define SMC_TASK2_BASE       (SMC_TASK1_BASE + SMC_TASK_OFFSET)
#define SMC_TASK3_BASE       (SMC_TASK2_BASE + SMC_TASK_OFFSET)
#define SMC_TASK4_BASE       (SMC_TASK3_BASE + SMC_TASK_OFFSET)

/* psci data area offsets */
#define CORE_STATE_DATA    0x0
#define SPSR_EL3_DATA      0x8
#define CNTXT_ID_DATA      0x10
#define START_ADDR_DATA    0x18
#define LINK_REG_DATA      0x20
#define GICC_CTLR_DATA     0x28
#define ABORT_FLAG_DATA    0x30
#define SCTLR_DATA         0x38
#define CPUECTLR_DATA      0x40
#define AUX_01_DATA        0x48  /* usage defined per SoC */
#define AUX_02_DATA        0x50  /* usage defined per SoC */
#define AUX_03_DATA        0x58  /* usage defined per SoC */
#define AUX_04_DATA        0x60  /* usage defined per SoC */
#define AUX_05_DATA        0x68  /* usage defined per SoC */
#define AUX_06_DATA        0x70  /* usage defined per SoC */
#define AUX_07_DATA        0x78  /* usage defined per SoC */
#define SCR_EL3_DATA       0x80
#define HCR_EL2_DATA       0x88

#endif /* BL31_DATA_H */
