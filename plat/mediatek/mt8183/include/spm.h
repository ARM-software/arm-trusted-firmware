/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_H
#define SPM_H

#define POWERON_CONFIG_EN		(SPM_BASE + 0x000)
#define SPM_POWER_ON_VAL0		(SPM_BASE + 0x004)
#define SPM_POWER_ON_VAL1		(SPM_BASE + 0x008)
#define SPM_CLK_CON			(SPM_BASE + 0x00C)
#define SPM_CLK_SETTLE			(SPM_BASE + 0x010)
#define SPM_AP_STANDBY_CON		(SPM_BASE + 0x014)
#define PCM_CON0			(SPM_BASE + 0x018)
#define PCM_CON1			(SPM_BASE + 0x01C)
#define PCM_IM_PTR			(SPM_BASE + 0x020)
#define PCM_IM_LEN			(SPM_BASE + 0x024)
#define PCM_REG_DATA_INI		(SPM_BASE + 0x028)
#define PCM_PWR_IO_EN			(SPM_BASE + 0x02C)
#define PCM_TIMER_VAL			(SPM_BASE + 0x030)
#define PCM_WDT_VAL			(SPM_BASE + 0x034)
#define PCM_IM_HOST_RW_PTR		(SPM_BASE + 0x038)
#define PCM_IM_HOST_RW_DAT		(SPM_BASE + 0x03C)
#define PCM_EVENT_VECTOR0		(SPM_BASE + 0x040)
#define PCM_EVENT_VECTOR1		(SPM_BASE + 0x044)
#define PCM_EVENT_VECTOR2		(SPM_BASE + 0x048)
#define PCM_EVENT_VECTOR3		(SPM_BASE + 0x04C)
#define PCM_EVENT_VECTOR4		(SPM_BASE + 0x050)
#define PCM_EVENT_VECTOR5		(SPM_BASE + 0x054)
#define PCM_EVENT_VECTOR6		(SPM_BASE + 0x058)
#define PCM_EVENT_VECTOR7		(SPM_BASE + 0x05C)
#define PCM_EVENT_VECTOR8		(SPM_BASE + 0x060)
#define PCM_EVENT_VECTOR9		(SPM_BASE + 0x064)
#define PCM_EVENT_VECTOR10		(SPM_BASE + 0x068)
#define PCM_EVENT_VECTOR11		(SPM_BASE + 0x06C)
#define PCM_EVENT_VECTOR12		(SPM_BASE + 0x070)
#define PCM_EVENT_VECTOR13		(SPM_BASE + 0x074)
#define PCM_EVENT_VECTOR14		(SPM_BASE + 0x078)
#define PCM_EVENT_VECTOR15		(SPM_BASE + 0x07C)
#define PCM_EVENT_VECTOR_EN		(SPM_BASE + 0x080)
#define SPM_SWINT			(SPM_BASE + 0x08C)
#define SPM_SWINT_SET			(SPM_BASE + 0x090)
#define SPM_SWINT_CLR			(SPM_BASE + 0x094)
#define SPM_SCP_MAILBOX			(SPM_BASE + 0x098)
#define SPM_SCP_IRQ			(SPM_BASE + 0x09C)
#define SPM_TWAM_CON			(SPM_BASE + 0x0A0)
#define SPM_TWAM_WINDOW_LEN		(SPM_BASE + 0x0A4)
#define SPM_TWAM_IDLE_SEL		(SPM_BASE + 0x0A8)
#define SPM_CPU_WAKEUP_EVENT		(SPM_BASE + 0x0B0)
#define SPM_IRQ_MASK			(SPM_BASE + 0x0B4)
#define SPM_SRC_REQ			(SPM_BASE + 0x0B8)
#define SPM_SRC_MASK			(SPM_BASE + 0x0BC)
#define SPM_SRC2_MASK			(SPM_BASE + 0x0C0)
#define SPM_WAKEUP_EVENT_MASK		(SPM_BASE + 0x0C4)
#define SPM_WAKEUP_EVENT_EXT_MASK	(SPM_BASE + 0x0C8)
#define SCP_CLK_CON			(SPM_BASE + 0x0D0)
#define PCM_DEBUG_CON			(SPM_BASE + 0x0D4)
#define PCM_REG0_DATA			(SPM_BASE + 0x100)
#define PCM_REG1_DATA			(SPM_BASE + 0x104)
#define PCM_REG2_DATA			(SPM_BASE + 0x108)
#define PCM_REG3_DATA			(SPM_BASE + 0x10C)
#define PCM_REG4_DATA			(SPM_BASE + 0x110)
#define PCM_REG5_DATA			(SPM_BASE + 0x114)
#define PCM_REG6_DATA			(SPM_BASE + 0x118)
#define PCM_REG7_DATA			(SPM_BASE + 0x11C)
#define PCM_REG8_DATA			(SPM_BASE + 0x120)
#define PCM_REG9_DATA			(SPM_BASE + 0x124)
#define PCM_REG10_DATA			(SPM_BASE + 0x128)
#define PCM_REG11_DATA			(SPM_BASE + 0x12C)
#define PCM_REG12_DATA			(SPM_BASE + 0x130)
#define PCM_REG13_DATA			(SPM_BASE + 0x134)
#define PCM_REG14_DATA			(SPM_BASE + 0x138)
#define PCM_REG15_DATA			(SPM_BASE + 0x13C)
#define PCM_REG12_MASK_B_STA		(SPM_BASE + 0x140)
#define PCM_REG12_EXT_DATA		(SPM_BASE + 0x144)
#define PCM_REG12_EXT_MASK_B_STA	(SPM_BASE + 0x148)
#define PCM_EVENT_REG_STA		(SPM_BASE + 0x14C)
#define PCM_TIMER_OUT			(SPM_BASE + 0x150)
#define PCM_WDT_OUT			(SPM_BASE + 0x154)
#define SPM_IRQ_STA			(SPM_BASE + 0x158)
#define SPM_WAKEUP_STA			(SPM_BASE + 0x15C)
#define SPM_WAKEUP_EXT_STA		(SPM_BASE + 0x160)
#define SPM_WAKEUP_MISC			(SPM_BASE + 0x164)
#define BUS_PROTECT_RDY			(SPM_BASE + 0x168)
#define BUS_PROTECT2_RDY		(SPM_BASE + 0x16C)
#define SUBSYS_IDLE_STA			(SPM_BASE + 0x170)
#define CPU_IDLE_STA			(SPM_BASE + 0x174)
#define PCM_FSM_STA			(SPM_BASE + 0x178)
#define PWR_STATUS			(SPM_BASE + 0x180)
#define PWR_STATUS_2ND			(SPM_BASE + 0x184)
#define CPU_PWR_STATUS			(SPM_BASE + 0x188)
#define CPU_PWR_STATUS_2ND		(SPM_BASE + 0x18C)
#define PCM_WDT_LATCH_0			(SPM_BASE + 0x190)
#define PCM_WDT_LATCH_1			(SPM_BASE + 0x194)
#define PCM_WDT_LATCH_2			(SPM_BASE + 0x198)
#define DRAMC_DBG_LATCH			(SPM_BASE + 0x19C)
#define SPM_TWAM_LAST_STA0		(SPM_BASE + 0x1A0)
#define SPM_TWAM_LAST_STA1		(SPM_BASE + 0x1A4)
#define SPM_TWAM_LAST_STA2		(SPM_BASE + 0x1A8)
#define SPM_TWAM_LAST_STA3		(SPM_BASE + 0x1AC)
#define SPM_TWAM_CURR_STA0		(SPM_BASE + 0x1B0)
#define SPM_TWAM_CURR_STA1		(SPM_BASE + 0x1B4)
#define SPM_TWAM_CURR_STA2		(SPM_BASE + 0x1B8)
#define SPM_TWAM_CURR_STA3		(SPM_BASE + 0x1BC)
#define SPM_TWAM_TIMER_OUT		(SPM_BASE + 0x1C0)
#define PCM_WDT_LATCH_3			(SPM_BASE + 0x1C4)
#define SPM_SRC_RDY_STA			(SPM_BASE + 0x1D0)
#define MISC_STA			(SPM_BASE + 0x1D4)
#define MCU_PWR_CON			(SPM_BASE + 0x200)
#define MP0_CPUTOP_PWR_CON		(SPM_BASE + 0x204)
#define MP0_CPU0_PWR_CON		(SPM_BASE + 0x208)
#define MP0_CPU1_PWR_CON		(SPM_BASE + 0x20C)
#define MP0_CPU2_PWR_CON		(SPM_BASE + 0x210)
#define MP0_CPU3_PWR_CON		(SPM_BASE + 0x214)
#define MP1_CPUTOP_PWR_CON		(SPM_BASE + 0x218)
#define MP1_CPU0_PWR_CON		(SPM_BASE + 0x21C)
#define MP1_CPU1_PWR_CON		(SPM_BASE + 0x220)
#define MP1_CPU2_PWR_CON		(SPM_BASE + 0x224)
#define MP1_CPU3_PWR_CON		(SPM_BASE + 0x228)
#define MP0_CPUTOP_L2_PDN		(SPM_BASE + 0x240)
#define MP0_CPUTOP_L2_SLEEP_B		(SPM_BASE + 0x244)
#define MP0_CPU0_L1_PDN			(SPM_BASE + 0x248)
#define MP0_CPU1_L1_PDN			(SPM_BASE + 0x24C)
#define MP0_CPU2_L1_PDN			(SPM_BASE + 0x250)
#define MP0_CPU3_L1_PDN			(SPM_BASE + 0x254)
#define MP1_CPUTOP_L2_PDN		(SPM_BASE + 0x258)
#define MP1_CPUTOP_L2_SLEEP_B		(SPM_BASE + 0x25C)
#define MP1_CPU0_L1_PDN			(SPM_BASE + 0x260)
#define MP1_CPU1_L1_PDN			(SPM_BASE + 0x264)
#define MP1_CPU2_L1_PDN			(SPM_BASE + 0x268)
#define MP1_CPU3_L1_PDN			(SPM_BASE + 0x26C)
#define CPU_EXT_BUCK_ISO		(SPM_BASE + 0x290)
#define DUMMY1_PWR_CON			(SPM_BASE + 0x2B0)
#define BYPASS_SPMC			(SPM_BASE + 0x2B4)
#define SPMC_DORMANT_ENABLE		(SPM_BASE + 0x2B8)
#define ARMPLL_CLK_CON			(SPM_BASE + 0x2BC)
#define SPMC_IN_RET			(SPM_BASE + 0x2C0)
#define VDE_PWR_CON			(SPM_BASE + 0x300)
#define VEN_PWR_CON			(SPM_BASE + 0x304)
#define ISP_PWR_CON			(SPM_BASE + 0x308)
#define DIS_PWR_CON			(SPM_BASE + 0x30C)
#define MJC_PWR_CON			(SPM_BASE + 0x310)
#define AUDIO_PWR_CON			(SPM_BASE + 0x314)
#define IFR_PWR_CON			(SPM_BASE + 0x318)
#define DPY_PWR_CON			(SPM_BASE + 0x31C)
#define MD1_PWR_CON			(SPM_BASE + 0x320)
#define MD2_PWR_CON			(SPM_BASE + 0x324)
#define C2K_PWR_CON			(SPM_BASE + 0x328)
#define CONN_PWR_CON			(SPM_BASE + 0x32C)
#define VCOREPDN_PWR_CON		(SPM_BASE + 0x330)
#define MFG_ASYNC_PWR_CON		(SPM_BASE + 0x334)
#define MFG_PWR_CON			(SPM_BASE + 0x338)
#define MFG_CORE0_PWR_CON		(SPM_BASE + 0x33C)
#define MFG_CORE1_PWR_CON		(SPM_BASE + 0x340)
#define CAM_PWR_CON			(SPM_BASE + 0x344)
#define SYSRAM_CON			(SPM_BASE + 0x350)
#define SYSROM_CON			(SPM_BASE + 0x354)
#define SCP_SRAM_CON			(SPM_BASE + 0x358)
#define GCPU_SRAM_CON			(SPM_BASE + 0x35C)
#define MDSYS_INTF_INFRA_PWR_CON	(SPM_BASE + 0x360)
#define MDSYS_INTF_MD1_PWR_CON		(SPM_BASE + 0x364)
#define MDSYS_INTF_C2K_PWR_CON		(SPM_BASE + 0x368)
#define BSI_TOP_SRAM_CON		(SPM_BASE + 0x370)
#define DVFSP_SRAM_CON			(SPM_BASE + 0x374)
#define MD_EXT_BUCK_ISO			(SPM_BASE + 0x390)
#define DUMMY2_PWR_CON			(SPM_BASE + 0x3B0)
#define MD1_OUTPUT_PISO_S_EN_IZ		(SPM_BASE + 0x3B4)
#define SPM_DVFS_CON			(SPM_BASE + 0x400)
#define SPM_MDBSI_CON			(SPM_BASE + 0x404)
#define SPM_MAS_PAUSE_MASK_B		(SPM_BASE + 0x408)
#define SPM_MAS_PAUSE2_MASK_B		(SPM_BASE + 0x40C)
#define SPM_BSI_GEN			(SPM_BASE + 0x410)
#define SPM_BSI_EN_SR			(SPM_BASE + 0x414)
#define SPM_BSI_CLK_SR			(SPM_BASE + 0x418)
#define SPM_BSI_D0_SR			(SPM_BASE + 0x41C)
#define SPM_BSI_D1_SR			(SPM_BASE + 0x420)
#define SPM_BSI_D2_SR			(SPM_BASE + 0x424)
#define SPM_AP_SEMA			(SPM_BASE + 0x428)
#define SPM_SPM_SEMA			(SPM_BASE + 0x42C)
#define AP2MD_CROSS_TRIGGER		(SPM_BASE + 0x430)
#define AP_MDSRC_REQ			(SPM_BASE + 0x434)
#define SPM2MD_DVFS_CON			(SPM_BASE + 0x438)
#define MD2SPM_DVFS_CON			(SPM_BASE + 0x43C)
#define DRAMC_DPY_CLK_SW_CON_RSV	(SPM_BASE + 0x440)
#define DPY_LP_CON			(SPM_BASE + 0x444)
#define CPU_DVFS_REQ			(SPM_BASE + 0x448)
#define SPM_PLL_CON			(SPM_BASE + 0x44C)
#define SPM_EMI_BW_MODE			(SPM_BASE + 0x450)
#define AP2MD_PEER_WAKEUP		(SPM_BASE + 0x454)
#define ULPOSC_CON			(SPM_BASE + 0x458)
#define DRAMC_DPY_CLK_SW_CON_SEL	(SPM_BASE + 0x460)
#define DRAMC_DPY_CLK_SW_CON		(SPM_BASE + 0x464)
#define DRAMC_DPY_CLK_SW_CON_SEL2	(SPM_BASE + 0x470)
#define DRAMC_DPY_CLK_SW_CON2		(SPM_BASE + 0x474)
#define SPM_SEMA_M0			(SPM_BASE + 0x480)
#define SPM_SEMA_M1			(SPM_BASE + 0x484)
#define SPM_SEMA_M2			(SPM_BASE + 0x488)
#define SPM_SEMA_M3			(SPM_BASE + 0x48C)
#define SPM_SEMA_M4			(SPM_BASE + 0x490)
#define SPM_SEMA_M5			(SPM_BASE + 0x494)
#define SPM_SEMA_M6			(SPM_BASE + 0x498)
#define SPM_SEMA_M7			(SPM_BASE + 0x49C)
#define SPM_SEMA_M8			(SPM_BASE + 0x4A0)
#define SPM_SEMA_M9			(SPM_BASE + 0x4A4)
#define SRAM_DREQ_ACK			(SPM_BASE + 0x4AC)
#define SRAM_DREQ_CON			(SPM_BASE + 0x4B0)
#define SRAM_DREQ_CON_SET		(SPM_BASE + 0x4B4)
#define SRAM_DREQ_CON_CLR		(SPM_BASE + 0x4B8)
#define MP0_CPU0_IRQ_MASK		(SPM_BASE + 0x500)
#define MP0_CPU1_IRQ_MASK		(SPM_BASE + 0x504)
#define MP0_CPU2_IRQ_MASK		(SPM_BASE + 0x508)
#define MP0_CPU3_IRQ_MASK		(SPM_BASE + 0x50C)
#define MP1_CPU0_IRQ_MASK		(SPM_BASE + 0x510)
#define MP1_CPU1_IRQ_MASK		(SPM_BASE + 0x514)
#define MP1_CPU2_IRQ_MASK		(SPM_BASE + 0x518)
#define MP1_CPU3_IRQ_MASK		(SPM_BASE + 0x51C)
#define MP0_CPU0_WFI_EN			(SPM_BASE + 0x530)
#define MP0_CPU1_WFI_EN			(SPM_BASE + 0x534)
#define MP0_CPU2_WFI_EN			(SPM_BASE + 0x538)
#define MP0_CPU3_WFI_EN			(SPM_BASE + 0x53C)
#define MP1_CPU0_WFI_EN			(SPM_BASE + 0x540)
#define MP1_CPU1_WFI_EN			(SPM_BASE + 0x544)
#define MP1_CPU2_WFI_EN			(SPM_BASE + 0x548)
#define MP1_CPU3_WFI_EN			(SPM_BASE + 0x54C)
#define CPU_PTPOD2_CON			(SPM_BASE + 0x560)
#define ROOT_CPUTOP_ADDR		(SPM_BASE + 0x570)
#define ROOT_CORE_ADDR			(SPM_BASE + 0x574)
#define CPU_SPARE_CON			(SPM_BASE + 0x580)
#define CPU_SPARE_CON_SET		(SPM_BASE + 0x584)
#define CPU_SPARE_CON_CLR		(SPM_BASE + 0x588)
#define SPM_SW_FLAG			(SPM_BASE + 0x600)
#define SPM_SW_DEBUG			(SPM_BASE + 0x604)
#define SPM_SW_RSV_0			(SPM_BASE + 0x608)
#define SPM_SW_RSV_1			(SPM_BASE + 0x60C)
#define SPM_SW_RSV_2			(SPM_BASE + 0x610)
#define SPM_SW_RSV_3			(SPM_BASE + 0x614)
#define SPM_SW_RSV_4			(SPM_BASE + 0x618)
#define SPM_SW_RSV_5			(SPM_BASE + 0x61C)
#define SPM_RSV_CON			(SPM_BASE + 0x620)
#define SPM_RSV_STA			(SPM_BASE + 0x624)
#define SPM_PASR_DPD_0			(SPM_BASE + 0x630)
#define SPM_PASR_DPD_1			(SPM_BASE + 0x634)
#define SPM_PASR_DPD_2			(SPM_BASE + 0x638)
#define SPM_PASR_DPD_3			(SPM_BASE + 0x63C)
#define SPM_SPARE_CON			(SPM_BASE + 0x640)
#define SPM_SPARE_CON_SET		(SPM_BASE + 0x644)
#define SPM_SPARE_CON_CLR		(SPM_BASE + 0x648)
#define SPM_SW_RSV_6			(SPM_BASE + 0x64C)
#define SPM_SW_RSV_7			(SPM_BASE + 0x650)
#define SPM_SW_RSV_8			(SPM_BASE + 0x654)
#define SPM_SW_RSV_9			(SPM_BASE + 0x658)
#define SPM_SW_RSV_10			(SPM_BASE + 0x65C)
#define SPM_SW_RSV_11			(SPM_BASE + 0x660)
#define SPM_SW_RSV_12			(SPM_BASE + 0x664)
#define SPM_SW_RSV_13			(SPM_BASE + 0x668)
#define SPM_SW_RSV_14			(SPM_BASE + 0x66C)
#define SPM_SW_RSV_15			(SPM_BASE + 0x670)
#define SPM_SW_RSV_16			(SPM_BASE + 0x674)
#define SPM_SW_RSV_17			(SPM_BASE + 0x678)
#define SPM_SW_RSV_18			(SPM_BASE + 0x67C)
#define SPM_SW_RSV_19			(SPM_BASE + 0x680)
#define SW_CRTL_EVENT			(SPM_BASE + 0x690)


#define MP1_CPU3_PWR_STA_MASK	(1U << 19)
#define MP1_CPU2_PWR_STA_MASK	(1U << 18)
#define MP1_CPU1_PWR_STA_MASK	(1U << 17)
#define MP1_CPU0_PWR_STA_MASK	(1U << 16)
#define MP1_CPUTOP_PWR_STA_MASK	(1U << 15)
#define MCU_PWR_STA_MASK	(1U << 14)
#define MP0_CPU3_PWR_STA_MASK	(1U << 12)
#define MP0_CPU2_PWR_STA_MASK	(1U << 11)
#define MP0_CPU1_PWR_STA_MASK	(1U << 10)
#define MP0_CPU0_PWR_STA_MASK	(1U << 9)
#define MP0_CPUTOP_PWR_STA_MASK	(1U << 8)


#define MP1_CPU3_STANDBYWFI	(1U << 17)
#define MP1_CPU2_STANDBYWFI	(1U << 16)
#define MP1_CPU1_STANDBYWFI	(1U << 15)
#define MP1_CPU0_STANDBYWFI	(1U << 14)
#define MP0_CPU3_STANDBYWFI	(1U << 13)
#define MP0_CPU2_STANDBYWFI	(1U << 12)
#define MP0_CPU1_STANDBYWFI	(1U << 11)
#define MP0_CPU0_STANDBYWFI	(1U << 10)

#define MP0_SPMC_SRAM_DORMANT_EN	(1<<0)
#define MP1_SPMC_SRAM_DORMANT_EN	(1<<1)
#define MP2_SPMC_SRAM_DORMANT_EN	(1<<2)

/* POWERON_CONFIG_EN (0x10006000+0x000) */
#define BCLK_CG_EN_LSB                      (1U << 0)       /* 1b */
#define PROJECT_CODE_LSB                    (1U << 16)      /* 16b */

/* SPM_POWER_ON_VAL0 (0x10006000+0x004) */
#define POWER_ON_VAL0_LSB                   (1U << 0)       /* 32b */

/* SPM_POWER_ON_VAL1 (0x10006000+0x008) */
#define POWER_ON_VAL1_LSB                   (1U << 0)       /* 32b */

/* SPM_CLK_CON (0x10006000+0x00C) */
#define SYSCLK0_EN_CTRL_LSB                 (1U << 0)       /* 2b */
#define SYSCLK1_EN_CTRL_LSB                 (1U << 2)       /* 2b */
#define SYS_SETTLE_SEL_LSB                  (1U << 4)       /* 1b */
#define SPM_LOCK_INFRA_DCM_LSB              (1U << 5)       /* 1b */
#define EXT_SRCCLKEN_MASK_LSB               (1U << 6)       /* 3b */
#define CXO32K_REMOVE_EN_MD1_LSB            (1U << 9)       /* 1b */
#define CXO32K_REMOVE_EN_MD2_LSB            (1U << 10)      /* 1b */
#define CLKSQ0_SEL_CTRL_LSB                 (1U << 11)      /* 1b */
#define CLKSQ1_SEL_CTRL_LSB                 (1U << 12)      /* 1b */
#define SRCLKEN0_EN_LSB                     (1U << 13)      /* 1b */
#define SRCLKEN1_EN_LSB                     (1U << 14)      /* 1b */
#define SCP_DCM_EN_LSB                      (1U << 15)      /* 1b */
#define SYSCLK0_SRC_MASK_B_LSB              (1U << 16)      /* 7b */
#define SYSCLK1_SRC_MASK_B_LSB              (1U << 23)      /* 7b */

/* SPM_CLK_SETTLE (0x10006000+0x010) */
#define SYSCLK_SETTLE_LSB                   (1U << 0)       /* 28b */

/* SPM_AP_STANDBY_CON (0x10006000+0x014) */
#define WFI_OP_LSB                          (1U << 0)       /* 1b */
#define MP0_CPUTOP_IDLE_MASK_LSB            (1U << 1)       /* 1b */
#define MP1_CPUTOP_IDLE_MASK_LSB            (1U << 2)       /* 1b */
#define MCUSYS_IDLE_MASK_LSB                (1U << 4)       /* 1b */
#define MM_MASK_B_LSB                       (1U << 16)      /* 2b */
#define MD_DDR_EN_DBC_EN_LSB                (1U << 18)      /* 1b */
#define MD_MASK_B_LSB                       (1U << 19)      /* 2b */
#define SCP_MASK_B_LSB                      (1U << 21)      /* 1b */
#define LTE_MASK_B_LSB                      (1U << 22)      /* 1b */
#define SRCCLKENI_MASK_B_LSB                (1U << 23)      /* 1b */
#define MD_APSRC_1_SEL_LSB                  (1U << 24)      /* 1b */
#define MD_APSRC_0_SEL_LSB                  (1U << 25)      /* 1b */
#define CONN_MASK_B_LSB                     (1U << 26)      /* 1b */
#define CONN_APSRC_SEL_LSB                  (1U << 27)      /* 1b */

/* PCM_CON0 (0x10006000+0x018) */
#define PCM_KICK_L_LSB                      (1U << 0)       /* 1b */
#define IM_KICK_L_LSB                       (1U << 1)       /* 1b */
#define PCM_CK_EN_LSB                       (1U << 2)       /* 1b */
#define EN_IM_SLEEP_DVS_LSB                 (1U << 3)       /* 1b */
#define IM_AUTO_PDN_EN_LSB                  (1U << 4)       /* 1b */
#define PCM_SW_RESET_LSB                    (1U << 15)      /* 1b */
#define PROJECT_CODE_LSB                    (1U << 16)      /* 16b */

/* PCM_CON1 (0x10006000+0x01C) */
#define IM_SLAVE_LSB                        (1U << 0)       /* 1b */
#define IM_SLEEP_LSB                        (1U << 1)       /* 1b */
#define MIF_APBEN_LSB                       (1U << 3)       /* 1b */
#define IM_PDN_LSB                          (1U << 4)       /* 1b */
#define PCM_TIMER_EN_LSB                    (1U << 5)       /* 1b */
#define IM_NONRP_EN_LSB                     (1U << 6)       /* 1b */
#define DIS_MIF_PROT_LSB                    (1U << 7)       /* 1b */
#define PCM_WDT_EN_LSB                      (1U << 8)       /* 1b */
#define PCM_WDT_WAKE_MODE_LSB               (1U << 9)       /* 1b */
#define SPM_SRAM_SLEEP_B_LSB                (1U << 10)      /* 1b */
#define SPM_SRAM_ISOINT_B_LSB               (1U << 11)      /* 1b */
#define EVENT_LOCK_EN_LSB                   (1U << 12)      /* 1b */
#define SRCCLKEN_FAST_RESP_LSB              (1U << 13)      /* 1b */
#define SCP_APB_INTERNAL_EN_LSB             (1U << 14)      /* 1b */
#define PROJECT_CODE_LSB                    (1U << 16)      /* 16b */

/* PCM_IM_PTR (0x10006000+0x020) */
#define PCM_IM_PTR_LSB                      (1U << 0)       /* 32b */

/* PCM_IM_LEN (0x10006000+0x024) */
#define PCM_IM_LEN_LSB                      (1U << 0)       /* 13b */

/* PCM_REG_DATA_INI (0x10006000+0x028) */
#define PCM_REG_DATA_INI_LSB                (1U << 0)       /* 32b */

/* PCM_PWR_IO_EN (0x10006000+0x02C) */
#define PCM_PWR_IO_EN_LSB                   (1U << 0)       /* 8b */
#define PCM_RF_SYNC_EN_LSB                  (1U << 16)      /* 8b */

/* PCM_TIMER_VAL (0x10006000+0x030) */
#define PCM_TIMER_VAL_LSB                   (1U << 0)       /* 32b */

/* PCM_WDT_VAL (0x10006000+0x034) */
#define PCM_WDT_VAL_LSB                     (1U << 0)       /* 32b */

/* PCM_IM_HOST_RW_PTR (0x10006000+0x038) */
#define PCM_IM_HOST_RW_PTR_LSB              (1U << 0)       /* 12b */
#define PCM_IM_HOST_W_EN_LSB                (1U << 30)      /* 1b */
#define PCM_IM_HOST_EN_LSB                  (1U << 31)      /* 1b */

/* PCM_IM_HOST_RW_DAT (0x10006000+0x03C) */
#define PCM_IM_HOST_RW_DAT_LSB              (1U << 0)       /* 32b */

/* PCM_EVENT_VECTOR0 (0x10006000+0x040) */
#define PCM_EVENT_VECTOR_0_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_0_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_0_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_0_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR1 (0x10006000+0x044) */
#define PCM_EVENT_VECTOR_1_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_1_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_1_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_1_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR2 (0x10006000+0x048) */
#define PCM_EVENT_VECTOR_2_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_2_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_2_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_2_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR3 (0x10006000+0x04C) */
#define PCM_EVENT_VECTOR_3_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_3_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_3_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_3_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR4 (0x10006000+0x050) */
#define PCM_EVENT_VECTOR_4_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_4_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_4_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_4_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR5 (0x10006000+0x054) */
#define PCM_EVENT_VECTOR_5_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_5_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_5_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_5_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR6 (0x10006000+0x058) */
#define PCM_EVENT_VECTOR_6_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_6_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_6_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_6_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR7 (0x10006000+0x05C) */
#define PCM_EVENT_VECTOR_7_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_7_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_7_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_7_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR8 (0x10006000+0x060) */
#define PCM_EVENT_VECTOR_8_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_8_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_8_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_8_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR9 (0x10006000+0x064) */
#define PCM_EVENT_VECTOR_9_LSB              (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_9_LSB              (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_9_LSB             (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_9_LSB              (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR10 (0x10006000+0x068) */
#define PCM_EVENT_VECTOR_10_LSB             (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_10_LSB             (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_10_LSB            (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_10_LSB             (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR11 (0x10006000+0x06C) */
#define PCM_EVENT_VECTOR_11_LSB             (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_11_LSB             (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_11_LSB            (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_11_LSB             (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR12 (0x10006000+0x070) */
#define PCM_EVENT_VECTOR_12_LSB             (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_12_LSB             (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_12_LSB            (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_12_LSB             (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR13 (0x10006000+0x074) */
#define PCM_EVENT_VECTOR_13_LSB             (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_13_LSB             (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_13_LSB            (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_13_LSB             (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR14 (0x10006000+0x078) */
#define PCM_EVENT_VECTOR_14_LSB             (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_14_LSB             (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_14_LSB            (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_14_LSB             (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR15 (0x10006000+0x07C) */
#define PCM_EVENT_VECTOR_15_LSB             (1U << 0)       /* 6b */
#define PCM_EVENT_RESUME_15_LSB             (1U << 6)       /* 1b */
#define PCM_EVENT_IMMEDIA_15_LSB            (1U << 7)       /* 1b */
#define PCM_EVENT_VECTPC_15_LSB             (1U << 16)      /* 11b */

/* PCM_EVENT_VECTOR_EN (0x10006000+0x080) */
#define PCM_EVENT_VECTOR_EN_LSB             (1U << 0)       /* 16b */

/* SPM_SWINT (0x10006000+0x08C) */
#define SPM_SWINT_LSB                       (1U << 0)       /* 10b */

/* SPM_SWINT_SET (0x10006000+0x090) */
#define SPM_SWINT_SET_LSB                   (1U << 0)       /* 10b */

/* SPM_SWINT_CLR (0x10006000+0x094) */
#define SPM_SWINT_CLR_LSB                   (1U << 0)       /* 10b */

/* SPM_SCP_MAILBOX (0x10006000+0x098) */
#define SPM_SCP_MAILBOX_LSB                 (1U << 0)       /* 32b */

/* SPM_SCP_IRQ (0x10006000+0x09C) */
#define SPM_SCP_IRQ_LSB                     (1U << 0)       /* 1b */
#define SPM_SCP_IRQ_SEL_LSB                 (1U << 4)       /* 1b */

/* SPM_TWAM_CON (0x10006000+0x0A0) */
#define TWAM_ENABLE_LSB                     (1U << 0)       /* 1b */
#define TWAM_SPEED_MODE_ENABLE_LSB          (1U << 1)       /* 1b */
#define TWAM_SW_RST_LSB                     (1U << 2)       /* 1b */
#define TWAM_MON_TYPE0_LSB                  (1U << 4)       /* 2b */
#define TWAM_MON_TYPE1_LSB                  (1U << 6)       /* 2b */
#define TWAM_MON_TYPE2_LSB                  (1U << 8)       /* 2b */
#define TWAM_MON_TYPE3_LSB                  (1U << 10)      /* 2b */
#define TWAM_SIGNAL_SEL0_LSB                (1U << 12)      /* 5b */
#define TWAM_SIGNAL_SEL1_LSB                (1U << 17)      /* 5b */
#define TWAM_SIGNAL_SEL2_LSB                (1U << 22)      /* 5b */
#define TWAM_SIGNAL_SEL3_LSB                (1U << 27)      /* 5b */

/* SPM_TWAM_WINDOW_LEN (0x10006000+0x0A4) */
#define TWAM_WINDOW_LEN_LSB                 (1U << 0)       /* 32b */

/* SPM_TWAM_IDLE_SEL (0x10006000+0x0A8) */
#define TWAM_IDLE_SEL_LSB                   (1U << 0)       /* 5b */

/* SPM_CPU_WAKEUP_EVENT (0x10006000+0x0B0) */
#define SPM_CPU_WAKEUP_EVENT_LSB            (1U << 0)       /* 1b */

/* SPM_IRQ_MASK (0x10006000+0x0B4) */
#define SPM_TWAM_IRQ_MASK_LSB               (1U << 2)       /* 1b */
#define PCM_IRQ_ROOT_MASK_LSB               (1U << 3)       /* 1b */
#define SPM_IRQ_MASK_LSB                    (1U << 8)       /* 10b */

/* SPM_SRC_REQ (0x10006000+0x0B8) */
#define SPM_APSRC_REQ_LSB                   (1U << 0)       /* 1b */
#define SPM_F26M_REQ_LSB                    (1U << 1)       /* 1b */
#define SPM_LTE_REQ_LSB                     (1U << 2)       /* 1b */
#define SPM_INFRA_REQ_LSB                   (1U << 3)       /* 1b */
#define SPM_VRF18_REQ_LSB                   (1U << 4)       /* 1b */
#define SPM_DVFS_REQ_LSB                    (1U << 5)       /* 1b */
#define SPM_DVFS_FORCE_DOWN_LSB             (1U << 6)       /* 1b */
#define SPM_DDREN_REQ_LSB                   (1U << 7)       /* 1b */
#define SPM_RSV_SRC_REQ_LSB                 (1U << 8)       /* 3b */
#define CPU_MD_DVFS_SOP_FORCE_ON_LSB        (1U << 16)      /* 1b */

/* SPM_SRC_MASK (0x10006000+0x0BC) */
#define CSYSPWREQ_MASK_LSB                  (1U << 0)       /* 1b */
#define CCIF0_MD_EVENT_MASK_B_LSB           (1U << 1)       /* 1b */
#define CCIF0_AP_EVENT_MASK_B_LSB           (1U << 2)       /* 1b */
#define CCIF1_MD_EVENT_MASK_B_LSB           (1U << 3)       /* 1b */
#define CCIF1_AP_EVENT_MASK_B_LSB           (1U << 4)       /* 1b */
#define CCIFMD_MD1_EVENT_MASK_B_LSB         (1U << 5)       /* 1b */
#define CCIFMD_MD2_EVENT_MASK_B_LSB         (1U << 6)       /* 1b */
#define DSI0_VSYNC_MASK_B_LSB               (1U << 7)       /* 1b */
#define DSI1_VSYNC_MASK_B_LSB               (1U << 8)       /* 1b */
#define DPI_VSYNC_MASK_B_LSB                (1U << 9)       /* 1b */
#define ISP0_VSYNC_MASK_B_LSB               (1U << 10)      /* 1b */
#define ISP1_VSYNC_MASK_B_LSB               (1U << 11)      /* 1b */
#define MD_SRCCLKENA_0_INFRA_MASK_B_LSB     (1U << 12)      /* 1b */
#define MD_SRCCLKENA_1_INFRA_MASK_B_LSB     (1U << 13)      /* 1b */
#define CONN_SRCCLKENA_INFRA_MASK_B_LSB     (1U << 14)      /* 1b */
#define MD32_SRCCLKENA_INFRA_MASK_B_LSB     (1U << 15)      /* 1b */
#define SRCCLKENI_INFRA_MASK_B_LSB          (1U << 16)      /* 1b */
#define MD_APSRC_REQ_0_INFRA_MASK_B_LSB     (1U << 17)      /* 1b */
#define MD_APSRC_REQ_1_INFRA_MASK_B_LSB     (1U << 18)      /* 1b */
#define CONN_APSRCREQ_INFRA_MASK_B_LSB      (1U << 19)      /* 1b */
#define MD32_APSRCREQ_INFRA_MASK_B_LSB      (1U << 20)      /* 1b */
#define MD_DDR_EN_0_MASK_B_LSB              (1U << 21)      /* 1b */
#define MD_DDR_EN_1_MASK_B_LSB              (1U << 22)      /* 1b */
#define MD_VRF18_REQ_0_MASK_B_LSB           (1U << 23)      /* 1b */
#define MD_VRF18_REQ_1_MASK_B_LSB           (1U << 24)      /* 1b */
#define MD1_DVFS_REQ_MASK_LSB               (1U << 25)      /* 2b */
#define CPU_DVFS_REQ_MASK_LSB               (1U << 27)      /* 1b */
#define EMI_BW_DVFS_REQ_MASK_LSB            (1U << 28)      /* 1b */
#define MD_SRCCLKENA_0_DVFS_REQ_MASK_B_LSB  (1U << 29)      /* 1b */
#define MD_SRCCLKENA_1_DVFS_REQ_MASK_B_LSB  (1U << 30)      /* 1b */
#define CONN_SRCCLKENA_DVFS_REQ_MASK_B_LSB  (1U << 31)      /* 1b */

/* SPM_SRC2_MASK (0x10006000+0x0C0) */
#define DVFS_HALT_MASK_B_LSB                (1U << 0)       /* 5b */
#define VDEC_REQ_MASK_B_LSB                 (1U << 6)       /* 1b */
#define GCE_REQ_MASK_B_LSB                  (1U << 7)       /* 1b */
#define CPU_MD_DVFS_REQ_MERGE_MASK_B_LSB    (1U << 8)       /* 1b */
#define MD_DDR_EN_DVFS_HALT_MASK_B_LSB      (1U << 9)       /* 2b */
#define DSI0_VSYNC_DVFS_HALT_MASK_B_LSB     (1U << 11)      /* 1b */
#define DSI1_VSYNC_DVFS_HALT_MASK_B_LSB     (1U << 12)      /* 1b */
#define DPI_VSYNC_DVFS_HALT_MASK_B_LSB      (1U << 13)      /* 1b */
#define ISP0_VSYNC_DVFS_HALT_MASK_B_LSB     (1U << 14)      /* 1b */
#define ISP1_VSYNC_DVFS_HALT_MASK_B_LSB     (1U << 15)      /* 1b */
#define CONN_DDR_EN_MASK_B_LSB              (1U << 16)      /* 1b */
#define DISP_REQ_MASK_B_LSB                 (1U << 17)      /* 1b */
#define DISP1_REQ_MASK_B_LSB                (1U << 18)      /* 1b */
#define MFG_REQ_MASK_B_LSB                  (1U << 19)      /* 1b */
#define C2K_PS_RCCIF_WAKE_MASK_B_LSB        (1U << 20)      /* 1b */
#define C2K_L1_RCCIF_WAKE_MASK_B_LSB        (1U << 21)      /* 1b */
#define PS_C2K_RCCIF_WAKE_MASK_B_LSB        (1U << 22)      /* 1b */
#define L1_C2K_RCCIF_WAKE_MASK_B_LSB        (1U << 23)      /* 1b */
#define SDIO_ON_DVFS_REQ_MASK_B_LSB         (1U << 24)      /* 1b */
#define EMI_BOOST_DVFS_REQ_MASK_B_LSB       (1U << 25)      /* 1b */
#define CPU_MD_EMI_DVFS_REQ_PROT_DIS_LSB    (1U << 26)      /* 1b */
#define DRAMC_SPCMD_APSRC_REQ_MASK_B_LSB    (1U << 27)      /* 1b */

/* SPM_WAKEUP_EVENT_MASK (0x10006000+0x0C4) */
#define SPM_WAKEUP_EVENT_MASK_LSB           (1U << 0)       /* 32b */

/* SPM_WAKEUP_EVENT_EXT_MASK (0x10006000+0x0C8) */
#define SPM_WAKEUP_EVENT_EXT_MASK_LSB       (1U << 0)       /* 32b */

/* SCP_CLK_CON (0x10006000+0x0D0) */
#define SCP_26M_CK_SEL_LSB                  (1U << 0)       /* 1b */

/* PCM_DEBUG_CON (0x10006000+0x0D4) */
#define PCM_DEBUG_OUT_ENABLE_LSB            (1U << 0)       /* 1b */

/* PCM_REG0_DATA (0x10006000+0x100) */
#define PCM_REG0_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG1_DATA (0x10006000+0x104) */
#define PCM_REG1_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG2_DATA (0x10006000+0x108) */
#define PCM_REG2_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG3_DATA (0x10006000+0x10C) */
#define PCM_REG3_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG4_DATA (0x10006000+0x110) */
#define PCM_REG4_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG5_DATA (0x10006000+0x114) */
#define PCM_REG5_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG6_DATA (0x10006000+0x118) */
#define PCM_REG6_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG7_DATA (0x10006000+0x11C) */
#define PCM_REG7_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG8_DATA (0x10006000+0x120) */
#define PCM_REG8_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG9_DATA (0x10006000+0x124) */
#define PCM_REG9_DATA_LSB                   (1U << 0)       /* 32b */

/* PCM_REG10_DATA (0x10006000+0x128) */
#define PCM_REG10_DATA_LSB                  (1U << 0)       /* 32b */

/* PCM_REG11_DATA (0x10006000+0x12C) */
#define PCM_REG11_DATA_LSB                  (1U << 0)       /* 32b */

/* PCM_REG12_DATA (0x10006000+0x130) */
#define PCM_REG12_DATA_LSB                  (1U << 0)       /* 32b */

/* PCM_REG13_DATA (0x10006000+0x134) */
#define PCM_REG13_DATA_LSB                  (1U << 0)       /* 32b */

/* PCM_REG14_DATA (0x10006000+0x138) */
#define PCM_REG14_DATA_LSB                  (1U << 0)       /* 32b */

/* PCM_REG15_DATA (0x10006000+0x13C) */
#define PCM_REG15_DATA_LSB                  (1U << 0)       /* 32b */

/* PCM_REG12_MASK_B_STA (0x10006000+0x140) */
#define PCM_REG12_MASK_B_STA_LSB            (1U << 0)       /* 32b */

/* PCM_REG12_EXT_DATA (0x10006000+0x144) */
#define PCM_REG12_EXT_DATA_LSB              (1U << 0)       /* 32b */

/* PCM_REG12_EXT_MASK_B_STA (0x10006000+0x148) */
#define PCM_REG12_EXT_MASK_B_STA_LSB        (1U << 0)       /* 32b */

/* PCM_EVENT_REG_STA (0x10006000+0x14C) */
#define PCM_EVENT_REG_STA_LSB               (1U << 0)       /* 32b */

/* PCM_TIMER_OUT (0x10006000+0x150) */
#define PCM_TIMER_OUT_LSB                   (1U << 0)       /* 32b */

/* PCM_WDT_OUT (0x10006000+0x154) */
#define PCM_WDT_OUT_LSB                     (1U << 0)       /* 32b */

/* SPM_IRQ_STA (0x10006000+0x158) */
#define TWAM_IRQ_LSB                        (1U << 2)       /* 1b */
#define PCM_IRQ_LSB                         (1U << 3)       /* 1b */
#define SPM_IRQ_SWINT_LSB                   (1U << 4)       /* 10b */

/* SPM_WAKEUP_STA (0x10006000+0x15C) */
#define SPM_WAKEUP_EVENT_STA_LSB            (1U << 0)       /* 32b */

/* SPM_WAKEUP_EXT_STA (0x10006000+0x160) */
#define SPM_WAKEUP_EVENT_EXT_STA_LSB        (1U << 0)       /* 32b */

/* SPM_WAKEUP_MISC (0x10006000+0x164) */
#define SPM_WAKEUP_EVENT_MISC_LSB           (1U << 0)       /* 30b */
#define SPM_PWRAP_IRQ_ACK_LSB               (1U << 30)      /* 1b */
#define SPM_PWRAP_IRQ_LSB                   (1U << 31)      /* 1b */

/* BUS_PROTECT_RDY (0x10006000+0x168) */
#define BUS_PROTECT_RDY_LSB                 (1U << 0)       /* 32b */

/* BUS_PROTECT2_RDY (0x10006000+0x16C) */
#define BUS_PROTECT2_RDY_LSB                (1U << 0)       /* 32b */

/* SUBSYS_IDLE_STA (0x10006000+0x170) */
#define SUBSYS_IDLE_STA_LSB                 (1U << 0)       /* 32b */

/* CPU_IDLE_STA (0x10006000+0x174) */
#define MP0_CPU0_STANDBYWFI_AFTER_SEL_LSB   (1U << 0)       /* 1b */
#define MP0_CPU1_STANDBYWFI_AFTER_SEL_LSB   (1U << 1)       /* 1b */
#define MP0_CPU2_STANDBYWFI_AFTER_SEL_LSB   (1U << 2)       /* 1b */
#define MP0_CPU3_STANDBYWFI_AFTER_SEL_LSB   (1U << 3)       /* 1b */
#define MP1_CPU0_STANDBYWFI_AFTER_SEL_LSB   (1U << 4)       /* 1b */
#define MP1_CPU1_STANDBYWFI_AFTER_SEL_LSB   (1U << 5)       /* 1b */
#define MP1_CPU2_STANDBYWFI_AFTER_SEL_LSB   (1U << 6)       /* 1b */
#define MP1_CPU3_STANDBYWFI_AFTER_SEL_LSB   (1U << 7)       /* 1b */
#define MP0_CPU0_STANDBYWFI_LSB             (1U << 10)      /* 1b */
#define MP0_CPU1_STANDBYWFI_LSB             (1U << 11)      /* 1b */
#define MP0_CPU2_STANDBYWFI_LSB             (1U << 12)      /* 1b */
#define MP0_CPU3_STANDBYWFI_LSB             (1U << 13)      /* 1b */
#define MP1_CPU0_STANDBYWFI_LSB             (1U << 14)      /* 1b */
#define MP1_CPU1_STANDBYWFI_LSB             (1U << 15)      /* 1b */
#define MP1_CPU2_STANDBYWFI_LSB             (1U << 16)      /* 1b */
#define MP1_CPU3_STANDBYWFI_LSB             (1U << 17)      /* 1b */
#define MP0_CPUTOP_IDLE_LSB                 (1U << 20)      /* 1b */
#define MP1_CPUTOP_IDLE_LSB                 (1U << 21)      /* 1b */
#define MCU_BIU_IDLE_LSB                    (1U << 22)      /* 1b */
#define MCUSYS_IDLE_LSB                     (1U << 23)      /* 1b */

/* PCM_FSM_STA (0x10006000+0x178) */
#define EXEC_INST_OP_LSB                    (1U << 0)       /* 4b */
#define PC_STATE_LSB                        (1U << 4)       /* 3b */
#define IM_STATE_LSB                        (1U << 7)       /* 3b */
#define MASTER_STATE_LSB                    (1U << 10)      /* 5b */
#define EVENT_FSM_LSB                       (1U << 15)      /* 3b */
#define PCM_CLK_SEL_STA_LSB                 (1U << 18)      /* 3b */
#define PCM_KICK_LSB                        (1U << 21)      /* 1b */
#define IM_KICK_LSB                         (1U << 22)      /* 1b */
#define EXT_SRCCLKEN_STA_LSB                (1U << 23)      /* 2b */
#define EXT_SRCVOLTEN_STA_LSB               (1U << 25)      /* 1b */

/* PWR_STATUS (0x10006000+0x180) */
#define PWR_STATUS_LSB                      (1U << 0)       /* 32b */

/* PWR_STATUS_2ND (0x10006000+0x184) */
#define PWR_STATUS_2ND_LSB                  (1U << 0)       /* 32b */

/* CPU_PWR_STATUS (0x10006000+0x188) */
#define CPU_PWR_STATUS_LSB                  (1U << 0)       /* 32b */

/* CPU_PWR_STATUS_2ND (0x10006000+0x18C) */
#define CPU_PWR_STATUS_2ND_LSB              (1U << 0)       /* 32b */

/* PCM_WDT_LATCH_0 (0x10006000+0x190) */
#define PCM_WDT_LATCH_0_LSB                 (1U << 0)       /* 32b */

/* PCM_WDT_LATCH_1 (0x10006000+0x194) */
#define PCM_WDT_LATCH_1_LSB                 (1U << 0)       /* 32b */

/* PCM_WDT_LATCH_2 (0x10006000+0x198) */
#define PCM_WDT_LATCH_2_LSB                 (1U << 0)       /* 32b */

/* DRAMC_DBG_LATCH (0x10006000+0x19C) */
#define DRAMC_DEBUG_LATCH_STATUS_LSB        (1U << 0)       /* 32b */

/* SPM_TWAM_LAST_STA0 (0x10006000+0x1A0) */
#define SPM_TWAM_LAST_STA0_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_LAST_STA1 (0x10006000+0x1A4) */
#define SPM_TWAM_LAST_STA1_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_LAST_STA2 (0x10006000+0x1A8) */
#define SPM_TWAM_LAST_STA2_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_LAST_STA3 (0x10006000+0x1AC) */
#define SPM_TWAM_LAST_STA3_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_CURR_STA0 (0x10006000+0x1B0) */
#define SPM_TWAM_CURR_STA0_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_CURR_STA1 (0x10006000+0x1B4) */
#define SPM_TWAM_CURR_STA1_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_CURR_STA2 (0x10006000+0x1B8) */
#define SPM_TWAM_CURR_STA2_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_CURR_STA3 (0x10006000+0x1BC) */
#define SPM_TWAM_CURR_STA3_LSB              (1U << 0)       /* 32b */

/* SPM_TWAM_TIMER_OUT (0x10006000+0x1C0) */
#define SPM_TWAM_TIMER_OUT_LSB              (1U << 0)       /* 32b */

/* PCM_WDT_LATCH_3 (0x10006000+0x1C4) */
#define PCM_WDT_LATCH_3_LSB                 (1U << 0)       /* 32b */

/* SPM_SRC_RDY_STA (0x10006000+0x1D0) */
#define SPM_INFRA_SRC_ACK_LSB               (1U << 0)       /* 1b */
#define SPM_VRF18_SRC_ACK_LSB               (1U << 1)       /* 1b */

/* MISC_STA (0x10006000+0x1D4) */
#define MM_DVFS_HALT_AF_MASK_LSB            (1U << 0)       /* 5b */

/* MCU_PWR_CON (0x10006000+0x200) */
#define MCU_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define MCU_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define MCU_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define MCU_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define MCU_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define MCU_SRAM_CKISO_LSB                  (1U << 5)       /* 1b */
#define MCU_SRAM_ISOINT_B_LSB               (1U << 6)       /* 1b */
#define MCU_SRAM_PD_SLPB_CLAMP_LSB          (1U << 7)       /* 1b */
#define MCU_SRAM_PDN_LSB                    (1U << 8)       /* 1b */
#define MCU_SRAM_SLEEP_B_LSB                (1U << 12)      /* 1b */
#define SC_MCU_SRAM_PDN_ACK_LSB             (1U << 24)      /* 1b */
#define SC_MCU_SRAM_SLEEP_B_ACK_LSB         (1U << 28)      /* 1b */

/* MP0_CPUTOP_PWR_CON (0x10006000+0x204) */
#define MP0_CPUTOP_PWR_RST_B_LSB            (1U << 0)       /* 1b */
#define MP0_CPUTOP_PWR_ISO_LSB              (1U << 1)       /* 1b */
#define MP0_CPUTOP_PWR_ON_LSB               (1U << 2)       /* 1b */
#define MP0_CPUTOP_PWR_ON_2ND_LSB           (1U << 3)       /* 1b */
#define MP0_CPUTOP_PWR_CLK_DIS_LSB          (1U << 4)       /* 1b */
#define MP0_CPUTOP_SRAM_CKISO_LSB           (1U << 5)       /* 1b */
#define MP0_CPUTOP_SRAM_ISOINT_B_LSB        (1U << 6)       /* 1b */
#define MP0_CPUTOP_SRAM_PD_SLPB_CLAMP_LSB   (1U << 7)       /* 1b */
#define MP0_CPUTOP_SRAM_PDN_LSB             (1U << 8)       /* 1b */
#define MP0_CPUTOP_SRAM_SLEEP_B_LSB         (1U << 12)      /* 1b */
#define SC_MP0_CPUTOP_SRAM_PDN_ACK_LSB      (1U << 24)      /* 1b */
#define SC_MP0_CPUTOP_SRAM_SLEEP_B_ACK_LSB  (1U << 28)      /* 1b */

/* MP0_CPU0_PWR_CON (0x10006000+0x208) */
#define MP0_CPU0_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP0_CPU0_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP0_CPU0_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP0_CPU0_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP0_CPU0_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP0_CPU0_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP0_CPU0_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP0_CPU0_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP0_CPU0_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP0_CPU0_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP0_CPU0_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP0_CPU0_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP0_CPU1_PWR_CON (0x10006000+0x20C) */
#define MP0_CPU1_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP0_CPU1_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP0_CPU1_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP0_CPU1_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP0_CPU1_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP0_CPU1_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP0_CPU1_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP0_CPU1_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP0_CPU1_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP0_CPU1_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP0_CPU1_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP0_CPU1_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP0_CPU2_PWR_CON (0x10006000+0x210) */
#define MP0_CPU2_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP0_CPU2_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP0_CPU2_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP0_CPU2_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP0_CPU2_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP0_CPU2_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP0_CPU2_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP0_CPU2_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP0_CPU2_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP0_CPU2_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP0_CPU2_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP0_CPU2_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP0_CPU3_PWR_CON (0x10006000+0x214) */
#define MP0_CPU3_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP0_CPU3_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP0_CPU3_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP0_CPU3_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP0_CPU3_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP0_CPU3_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP0_CPU3_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP0_CPU3_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP0_CPU3_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP0_CPU3_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP0_CPU3_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP0_CPU3_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP1_CPUTOP_PWR_CON (0x10006000+0x218) */
#define MP1_CPUTOP_PWR_RST_B_LSB            (1U << 0)       /* 1b */
#define MP1_CPUTOP_PWR_ISO_LSB              (1U << 1)       /* 1b */
#define MP1_CPUTOP_PWR_ON_LSB               (1U << 2)       /* 1b */
#define MP1_CPUTOP_PWR_ON_2ND_LSB           (1U << 3)       /* 1b */
#define MP1_CPUTOP_PWR_CLK_DIS_LSB          (1U << 4)       /* 1b */
#define MP1_CPUTOP_SRAM_CKISO_LSB           (1U << 5)       /* 1b */
#define MP1_CPUTOP_SRAM_ISOINT_B_LSB        (1U << 6)       /* 1b */
#define MP1_CPUTOP_SRAM_PD_SLPB_CLAMP_LSB   (1U << 7)       /* 1b */
#define MP1_CPUTOP_SRAM_PDN_LSB             (1U << 8)       /* 1b */
#define MP1_CPUTOP_SRAM_SLEEP_B_LSB         (1U << 12)      /* 1b */
#define SC_MP1_CPUTOP_SRAM_PDN_ACK_LSB      (1U << 24)      /* 1b */
#define SC_MP1_CPUTOP_SRAM_SLEEP_B_ACK_LSB  (1U << 28)      /* 1b */

/* MP1_CPU0_PWR_CON (0x10006000+0x21C) */
#define MP1_CPU0_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP1_CPU0_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP1_CPU0_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP1_CPU0_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP1_CPU0_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP1_CPU0_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP1_CPU0_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP1_CPU0_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP1_CPU0_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP1_CPU0_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP1_CPU0_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP1_CPU0_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP1_CPU1_PWR_CON (0x10006000+0x220) */
#define MP1_CPU1_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP1_CPU1_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP1_CPU1_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP1_CPU1_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP1_CPU1_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP1_CPU1_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP1_CPU1_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP1_CPU1_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP1_CPU1_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP1_CPU1_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP1_CPU1_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP1_CPU1_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP1_CPU2_PWR_CON (0x10006000+0x224) */
#define MP1_CPU2_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP1_CPU2_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP1_CPU2_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP1_CPU2_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP1_CPU2_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP1_CPU2_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP1_CPU2_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP1_CPU2_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP1_CPU2_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP1_CPU2_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP1_CPU2_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP1_CPU2_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP1_CPU3_PWR_CON (0x10006000+0x228) */
#define MP1_CPU3_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define MP1_CPU3_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define MP1_CPU3_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define MP1_CPU3_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define MP1_CPU3_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */
#define MP1_CPU3_SRAM_CKISO_LSB             (1U << 5)       /* 1b */
#define MP1_CPU3_SRAM_ISOINT_B_LSB          (1U << 6)       /* 1b */
#define MP1_CPU3_SRAM_PD_SLPB_CLAMP_LSB     (1U << 7)       /* 1b */
#define MP1_CPU3_SRAM_PDN_LSB               (1U << 8)       /* 1b */
#define MP1_CPU3_SRAM_SLEEP_B_LSB           (1U << 12)      /* 1b */
#define SC_MP1_CPU3_SRAM_PDN_ACK_LSB        (1U << 24)      /* 1b */
#define SC_MP1_CPU3_SRAM_SLEEP_B_ACK_LSB    (1U << 28)      /* 1b */

/* MP0_CPUTOP_L2_PDN (0x10006000+0x240) */
#define MP0_CPUTOP_L2_SRAM_PDN_LSB          (1U << 0)       /* 1b */
#define MP0_CPUTOP_L2_SRAM_PDN_ACK_LSB      (1U << 8)       /* 1b */

/* MP0_CPUTOP_L2_SLEEP_B (0x10006000+0x244) */
#define MP0_CPUTOP_L2_SRAM_SLEEP_B_LSB      (1U << 0)       /* 1b */
#define MP0_CPUTOP_L2_SRAM_SLEEP_B_ACK_LSB  (1U << 8)       /* 1b */

/* MP0_CPU0_L1_PDN (0x10006000+0x248) */
#define MP0_CPU0_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP0_CPU0_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* MP0_CPU1_L1_PDN (0x10006000+0x24C) */
#define MP0_CPU1_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP0_CPU1_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* MP0_CPU2_L1_PDN (0x10006000+0x250) */
#define MP0_CPU2_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP0_CPU2_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* MP0_CPU3_L1_PDN (0x10006000+0x254) */
#define MP0_CPU3_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP0_CPU3_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* MP1_CPUTOP_L2_PDN (0x10006000+0x258) */
#define MP1_CPUTOP_L2_SRAM_PDN_LSB          (1U << 0)       /* 1b */
#define MP1_CPUTOP_L2_SRAM_PDN_ACK_LSB      (1U << 8)       /* 1b */

/* MP1_CPUTOP_L2_SLEEP_B (0x10006000+0x25C) */
#define MP1_CPUTOP_L2_SRAM_SLEEP_B_LSB      (1U << 0)       /* 1b */
#define MP1_CPUTOP_L2_SRAM_SLEEP_B_ACK_LSB  (1U << 8)       /* 1b */

/* MP1_CPU0_L1_PDN (0x10006000+0x260) */
#define MP1_CPU0_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP1_CPU0_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* MP1_CPU1_L1_PDN (0x10006000+0x264) */
#define MP1_CPU1_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP1_CPU1_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* MP1_CPU2_L1_PDN (0x10006000+0x268) */
#define MP1_CPU2_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP1_CPU2_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* MP1_CPU3_L1_PDN (0x10006000+0x26C) */
#define MP1_CPU3_L1_PDN_LSB                 (1U << 0)       /* 1b */
#define MP1_CPU3_L1_PDN_ACK_LSB             (1U << 8)       /* 1b */

/* CPU_EXT_BUCK_ISO (0x10006000+0x290) */
#define MP0_EXT_BUCK_ISO_LSB                (1U << 0)       /* 1b */
#define MP1_EXT_BUCK_ISO_LSB                (1U << 1)       /* 1b */

/* DUMMY1_PWR_CON (0x10006000+0x2B0) */
#define DUMMY1_PWR_RST_B_LSB                (1U << 0)       /* 1b */
#define DUMMY1_PWR_ISO_LSB                  (1U << 1)       /* 1b */
#define DUMMY1_PWR_ON_LSB                   (1U << 2)       /* 1b */
#define DUMMY1_PWR_ON_2ND_LSB               (1U << 3)       /* 1b */
#define DUMMY1_PWR_CLK_DIS_LSB              (1U << 4)       /* 1b */

/* BYPASS_SPMC (0x10006000+0x2B4) */
#define BYPASS_CPU_SPMC_MODE_LSB            (1U << 0)       /* 1b */

/* SPMC_DORMANT_ENABLE (0x10006000+0x2B8) */
#define MP0_SPMC_SRAM_DORMANT_EN_LSB        (1U << 0)       /* 1b */
#define MP1_SPMC_SRAM_DORMANT_EN_LSB        (1U << 1)       /* 1b */

/* ARMPLL_CLK_CON (0x10006000+0x2BC) */
#define MUXSEL_SC_CCIPLL_LSB                (1U << 0)       /* 1b */
#define MUXSEL_SC_ARMPLL1_LSB               (1U << 1)       /* 1b */
#define MUXSEL_SC_ARMPLL2_LSB               (1U << 2)       /* 1b */
#define REG_SC_ARM_CLK_OFF_LSB              (1U << 8)       /* 4b */
#define REG_SC_ARMPLL_OFF_LSB               (1U << 12)      /* 4b */
#define REG_SC_ARMPLLOUT_OFF_LSB            (1U << 16)      /* 4b */
#define REG_SC_FHC_PAUSE_LSB                (1U << 20)      /* 4b */
#define REG_SC_ARMPLL_S_OFF_LSB             (1U << 24)      /* 4b */

/* SPMC_IN_RET (0x10006000+0x2C0) */
#define SPMC_STATUS_LSB                     (1U << 0)       /* 8b */

/* VDE_PWR_CON (0x10006000+0x300) */
#define VDE_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define VDE_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define VDE_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define VDE_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define VDE_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define VDE_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define VDE_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* VEN_PWR_CON (0x10006000+0x304) */
#define VEN_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define VEN_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define VEN_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define VEN_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define VEN_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define VEN_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define VEN_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* ISP_PWR_CON (0x10006000+0x308) */
#define ISP_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define ISP_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define ISP_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define ISP_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define ISP_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define ISP_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define ISP_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* DIS_PWR_CON (0x10006000+0x30C) */
#define DIS_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define DIS_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define DIS_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define DIS_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define DIS_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define DIS_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define DIS_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* MJC_PWR_CON (0x10006000+0x310) */
#define MJC_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define MJC_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define MJC_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define MJC_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define MJC_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define MJC_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define MJC_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* AUDIO_PWR_CON (0x10006000+0x314) */
#define AUD_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define AUD_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define AUD_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define AUD_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define AUD_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define AUD_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define AUD_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* IFR_PWR_CON (0x10006000+0x318) */
#define IFR_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define IFR_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define IFR_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define IFR_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define IFR_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define IFR_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define IFR_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* DPY_PWR_CON (0x10006000+0x31C) */
#define DPY_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define DPY_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define DPY_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define DPY_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define DPY_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define DPY_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define DPY_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* MD1_PWR_CON (0x10006000+0x320) */
#define MD1_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define MD1_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define MD1_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define MD1_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define MD1_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define MD1_SRAM_PDN_LSB                    (1U << 8)       /* 1b */

/* MD2_PWR_CON (0x10006000+0x324) */
#define MD2_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define MD2_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define MD2_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define MD2_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define MD2_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define MD2_SRAM_PDN_LSB                    (1U << 8)       /* 1b */

/* C2K_PWR_CON (0x10006000+0x328) */
#define C2K_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define C2K_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define C2K_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define C2K_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define C2K_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */

/* CONN_PWR_CON (0x10006000+0x32C) */
#define CONN_PWR_RST_B_LSB                  (1U << 0)       /* 1b */
#define CONN_PWR_ISO_LSB                    (1U << 1)       /* 1b */
#define CONN_PWR_ON_LSB                     (1U << 2)       /* 1b */
#define CONN_PWR_ON_2ND_LSB                 (1U << 3)       /* 1b */
#define CONN_PWR_CLK_DIS_LSB                (1U << 4)       /* 1b */
#define CONN_SRAM_PDN_LSB                   (1U << 8)       /* 1b */
#define CONN_SRAM_PDN_ACK_LSB               (1U << 12)      /* 1b */

/* VCOREPDN_PWR_CON (0x10006000+0x330) */
#define VCOREPDN_PWR_RST_B_LSB              (1U << 0)       /* 1b */
#define VCOREPDN_PWR_ISO_LSB                (1U << 1)       /* 1b */
#define VCOREPDN_PWR_ON_LSB                 (1U << 2)       /* 1b */
#define VCOREPDN_PWR_ON_2ND_LSB             (1U << 3)       /* 1b */
#define VCOREPDN_PWR_CLK_DIS_LSB            (1U << 4)       /* 1b */

/* MFG_ASYNC_PWR_CON (0x10006000+0x334) */
#define MFG_ASYNC_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define MFG_ASYNC_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define MFG_ASYNC_PWR_ON_LSB                (1U << 2)       /* 1b */
#define MFG_ASYNC_PWR_ON_2ND_LSB            (1U << 3)       /* 1b */
#define MFG_ASYNC_PWR_CLK_DIS_LSB           (1U << 4)       /* 1b */
#define MFG_ASYNC_SRAM_PDN_LSB              (1U << 8)       /* 4b */
#define MFG_ASYNC_SRAM_PDN_ACK_LSB          (1U << 12)      /* 4b */

/* MFG_PWR_CON (0x10006000+0x338) */
#define MFG_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define MFG_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define MFG_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define MFG_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define MFG_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define MFG_SRAM_PDN_LSB                    (1U << 8)       /* 6b */
#define MFG_SRAM_PDN_ACK_LSB                (1U << 16)      /* 6b */

/* MFG_CORE0_PWR_CON (0x10006000+0x33C) */
#define MFG_CORE0_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define MFG_CORE0_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define MFG_CORE0_PWR_ON_LSB                (1U << 2)       /* 1b */
#define MFG_CORE0_ON_2ND_LSB                (1U << 3)       /* 1b */
#define MFG_CORE0_CLK_DIS_LSB               (1U << 4)       /* 1b */
#define MFG_CORE0_SRAM_PDN_LSB              (1U << 5)       /* 1b */
#define MFG_CORE0_SRAM_PDN_ACK_LSB          (1U << 6)       /* 1b */

/* MFG_CORE1_PWR_CON (0x10006000+0x340) */
#define MFG_CORE1_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define MFG_CORE1_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define MFG_CORE1_PWR_ON_LSB                (1U << 2)       /* 1b */
#define MFG_CORE1_ON_2ND_LSB                (1U << 3)       /* 1b */
#define MFG_CORE1_CLK_DIS_LSB               (1U << 4)       /* 1b */
#define MFG_CORE1_SRAM_PDN_LSB              (1U << 5)       /* 1b */
#define MFG_CORE1_SRAM_PDN_ACK_LSB          (1U << 6)       /* 1b */

/* CAM_PWR_CON (0x10006000+0x344) */
#define CAM_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define CAM_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define CAM_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define CAM_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define CAM_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define CAM_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define CAM_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */

/* SYSRAM_CON (0x10006000+0x350) */
#define IFR_SRAMROM_SRAM_PDN_LSB            (1U << 0)       /* 8b */
#define IFR_SRAMROM_SRAM_CKISO_LSB          (1U << 8)       /* 8b */
#define IFR_SRAMROM_SRAM_SLEEP_B_LSB        (1U << 16)      /* 8b */
#define IFR_SRAMROM_SRAM_ISOINT_B_LSB       (1U << 24)      /* 8b */

/* SYSROM_CON (0x10006000+0x354) */
#define IFR_SRAMROM_ROM_PDN_LSB             (1U << 0)       /* 6b */

/* SCP_SRAM_CON (0x10006000+0x358) */
#define SCP_SRAM_PDN_LSB                    (1U << 0)       /* 1b */
#define SCP_SRAM_SLEEP_B_LSB                (1U << 4)       /* 1b */
#define SCP_SRAM_ISOINT_B_LSB               (1U << 8)       /* 1b */

/* GCPU_SRAM_CON (0x10006000+0x35C) */
#define GCPU_SRAM_PDN_LSB                   (1U << 0)       /* 4b */
#define GCPU_SRAM_CKISO_LSB                 (1U << 4)       /* 4b */
#define GCPU_SRAM_SLEEP_B_LSB               (1U << 8)       /* 4b */
#define GCPU_SRAM_ISOINT_B_LSB              (1U << 12)      /* 4b */

/* MDSYS_INTF_INFRA_PWR_CON (0x10006000+0x360) */
#define MDSYS_INTF_INFRA_PWR_RST_B_LSB      (1U << 0)       /* 1b */
#define MDSYS_INTF_INFRA_PWR_ISO_LSB        (1U << 1)       /* 1b */
#define MDSYS_INTF_INFRA_PWR_ON_LSB         (1U << 2)       /* 1b */
#define MDSYS_INTF_INFRA_PWR_ON_2ND_LSB     (1U << 3)       /* 1b */
#define MDSYS_INTF_INFRA_PWR_CLK_DIS_LSB    (1U << 4)       /* 1b */

/* MDSYS_INTF_MD1_PWR_CON (0x10006000+0x364) */
#define MDSYS_INTF_MD1_PWR_RST_B_LSB        (1U << 0)       /* 1b */
#define MDSYS_INTF_MD1_PWR_ISO_LSB          (1U << 1)       /* 1b */
#define MDSYS_INTF_MD1_PWR_ON_LSB           (1U << 2)       /* 1b */
#define MDSYS_INTF_MD1_PWR_ON_2ND_LSB       (1U << 3)       /* 1b */
#define MDSYS_INTF_MD1_PWR_CLK_DIS_LSB      (1U << 4)       /* 1b */

/* MDSYS_INTF_C2K_PWR_CON (0x10006000+0x368) */
#define MDSYS_INTF_C2K_PWR_RST_B_LSB        (1U << 0)       /* 1b */
#define MDSYS_INTF_C2K_PWR_ISO_LSB          (1U << 1)       /* 1b */
#define MDSYS_INTF_C2K_PWR_ON_LSB           (1U << 2)       /* 1b */
#define MDSYS_INTF_C2K_PWR_ON_2ND_LSB       (1U << 3)       /* 1b */
#define MDSYS_INTF_C2K_PWR_CLK_DIS_LSB      (1U << 4)       /* 1b */

/* BSI_TOP_SRAM_CON (0x10006000+0x370) */
#define BSI_TOP_SRAM_PDN_LSB                (1U << 0)       /* 7b */
#define BSI_TOP_SRAM_DSLP_LSB               (1U << 7)       /* 7b */
#define BSI_TOP_SRAM_SLEEP_B_LSB            (1U << 14)      /* 7b */
#define BSI_TOP_SRAM_ISOINT_B_LSB           (1U << 21)      /* 7b */
#define BSI_TOP_SRAM_ISO_EN_LSB             (1U << 28)      /* 2b */

/* DVFSP_SRAM_CON (0x10006000+0x374) */
#define DVFSP_SRAM_PDN_LSB                  (1U << 0)       /* 2b */
#define DVFSP_SRAM_SLEEP_B_LSB              (1U << 4)       /* 2b */
#define DVFSP_SRAM_ISOINT_B_LSB             (1U << 8)       /* 2b */

/* MD_EXT_BUCK_ISO (0x10006000+0x390) */
#define MD_EXT_BUCK_ISO_LSB                 (1U << 0)       /* 1b */

/* DUMMY2_PWR_CON (0x10006000+0x3B0) */
#define DUMMY2_PWR_RST_B_LSB                (1U << 0)       /* 1b */
#define DUMMY2_PWR_ISO_LSB                  (1U << 1)       /* 1b */
#define DUMMY2_PWR_ON_LSB                   (1U << 2)       /* 1b */
#define DUMMY2_PWR_ON_2ND_LSB               (1U << 3)       /* 1b */
#define DUMMY2_PWR_CLK_DIS_LSB              (1U << 4)       /* 1b */
#define DUMMY2_SRAM_PDN_LSB                 (1U << 8)       /* 4b */
#define DUMMY2_SRAM_PDN_ACK_LSB             (1U << 12)      /* 4b */

/* MD1_OUTPUT_PISO_S_EN_IZ (0x10006000+0x3B4) */
#define MD1_OUTPUT_PISO_S_EN_IZ_LSB         (1U << 0)       /* 1b */

/* SPM_DVFS_CON (0x10006000+0x400) */
#define SPM_DVFS_CON_LSB                    (1U << 0)       /* 4b */
#define SPM_DVFS_ACK_LSB                    (1U << 30)      /* 2b */

/* SPM_MDBSI_CON (0x10006000+0x404) */
#define SPM_MDBSI_CON_LSB                   (1U << 0)       /* 3b */

/* SPM_MAS_PAUSE_MASK_B (0x10006000+0x408) */
#define SPM_MAS_PAUSE_MASK_B_LSB            (1U << 0)       /* 32b */

/* SPM_MAS_PAUSE2_MASK_B (0x10006000+0x40C) */
#define SPM_MAS_PAUSE2_MASK_B_LSB           (1U << 0)       /* 32b */

/* SPM_BSI_GEN (0x10006000+0x410) */
#define SPM_BSI_START_LSB                   (1U << 0)       /* 1b */

/* SPM_BSI_EN_SR (0x10006000+0x414) */
#define SPM_BSI_EN_SR_LSB                   (1U << 0)       /* 32b */

/* SPM_BSI_CLK_SR (0x10006000+0x418) */
#define SPM_BSI_CLK_SR_LSB                  (1U << 0)       /* 32b */

/* SPM_BSI_D0_SR (0x10006000+0x41C) */
#define SPM_BSI_D0_SR_LSB                   (1U << 0)       /* 32b */

/* SPM_BSI_D1_SR (0x10006000+0x420) */
#define SPM_BSI_D1_SR_LSB                   (1U << 0)       /* 32b */

/* SPM_BSI_D2_SR (0x10006000+0x424) */
#define SPM_BSI_D2_SR_LSB                   (1U << 0)       /* 32b */

/* SPM_AP_SEMA (0x10006000+0x428) */
#define SPM_AP_SEMA_LSB                     (1U << 0)       /* 1b */

/* SPM_SPM_SEMA (0x10006000+0x42C) */
#define SPM_SPM_SEMA_LSB                    (1U << 0)       /* 1b */

/* AP2MD_CROSS_TRIGGER (0x10006000+0x430) */
#define AP2MD_CROSS_TRIGGER_REQ_LSB         (1U << 0)       /* 1b */
#define AP2MD_CROSS_TRIGGER_ACK_LSB         (1U << 1)       /* 1b */

/* AP_MDSRC_REQ (0x10006000+0x434) */
#define AP_MD1SRC_REQ_LSB                   (1U << 0)       /* 1b */
#define AP_MD2SRC_REQ_LSB                   (1U << 1)       /* 1b */
#define AP_MD1SRC_ACK_LSB                   (1U << 4)       /* 1b */
#define AP_MD2SRC_ACK_LSB                   (1U << 5)       /* 1b */

/* SPM2MD_DVFS_CON (0x10006000+0x438) */
#define SPM2MD_DVFS_CON_LSB                 (1U << 0)       /* 16b */

/* MD2SPM_DVFS_CON (0x10006000+0x43C) */
#define MD2SPM_DVFS_CON_LSB                 (1U << 0)       /* 16b */

/* DRAMC_DPY_CLK_SW_CON_RSV (0x10006000+0x440) */
#define SPM2DRAMC_SHUFFLE_START_LSB         (1U << 0)       /* 1b */
#define SPM2DRAMC_SHUFFLE_SWITCH_LSB        (1U << 1)       /* 1b */
#define SPM2DPY_DIV2_SYNC_LSB               (1U << 2)       /* 1b */
#define SPM2DPY_1PLL_SWITCH_LSB             (1U << 3)       /* 1b */
#define SPM2DPY_TEST_CK_MUX_LSB             (1U << 4)       /* 1b */
#define SPM2DPY_ASYNC_MODE_LSB              (1U << 5)       /* 1b */
#define SPM2TOP_ASYNC_MODE_LSB              (1U << 6)       /* 1b */

/* DPY_LP_CON (0x10006000+0x444) */
#define SC_DDRPHY_LP_SIGNALS_LSB            (1U << 0)       /* 3b */

/* CPU_DVFS_REQ (0x10006000+0x448) */
#define CPU_DVFS_REQ_LSB                    (1U << 0)       /* 16b */
#define DVFS_HALT_LSB                       (1U << 16)      /* 1b */
#define MD_DVFS_ERROR_STATUS_LSB            (1U << 17)      /* 1b */

/* SPM_PLL_CON (0x10006000+0x44C) */
#define SC_MPLLOUT_OFF_LSB                  (1U << 0)       /* 1b */
#define SC_UNIPLLOUT_OFF_LSB                (1U << 1)       /* 1b */
#define SC_MPLL_OFF_LSB                     (1U << 4)       /* 1b */
#define SC_UNIPLL_OFF_LSB                   (1U << 5)       /* 1b */
#define SC_MPLL_S_OFF_LSB                   (1U << 8)       /* 1b */
#define SC_UNIPLL_S_OFF_LSB                 (1U << 9)       /* 1b */
#define SC_SMI_CK_OFF_LSB                   (1U << 16)      /* 1b */
#define SC_MD32K_CK_OFF_LSB                 (1U << 17)      /* 1b */

/* SPM_EMI_BW_MODE (0x10006000+0x450) */
#define EMI_BW_MODE_LSB                     (1U << 0)       /* 1b */
#define EMI_BOOST_MODE_LSB                  (1U << 1)       /* 1b */

/* AP2MD_PEER_WAKEUP (0x10006000+0x454) */
#define AP2MD_PEER_WAKEUP_LSB               (1U << 0)       /* 1b */

/* ULPOSC_CON (0x10006000+0x458) */
#define ULPOSC_EN_LSB                       (1U << 0)       /* 1b */
#define ULPOSC_RST_LSB                      (1U << 1)       /* 1b */
#define ULPOSC_CG_EN_LSB                    (1U << 2)       /* 1b */

/* DRAMC_DPY_CLK_SW_CON_SEL (0x10006000+0x460) */
#define SW_DR_GATE_RETRY_EN_SEL_LSB         (1U << 0)       /* 2b */
#define SW_EMI_CLK_OFF_SEL_LSB              (1U << 2)       /* 2b */
#define SW_DPY_MODE_SW_SEL_LSB              (1U << 4)       /* 2b */
#define SW_DMSUS_OFF_SEL_LSB                (1U << 6)       /* 2b */
#define SW_MEM_CK_OFF_SEL_LSB               (1U << 8)       /* 2b */
#define SW_DPY_2ND_DLL_EN_SEL_LSB           (1U << 10)      /* 2b */
#define SW_DPY_DLL_EN_SEL_LSB               (1U << 12)      /* 2b */
#define SW_DPY_DLL_CK_EN_SEL_LSB            (1U << 14)      /* 2b */
#define SW_DPY_VREF_EN_SEL_LSB              (1U << 16)      /* 2b */
#define SW_PHYPLL_EN_SEL_LSB                (1U << 18)      /* 2b */
#define SW_DDRPHY_FB_CK_EN_SEL_LSB          (1U << 20)      /* 2b */
#define SEPERATE_PHY_PWR_SEL_LSB            (1U << 23)      /* 1b */
#define SW_DMDRAMCSHU_ACK_SEL_LSB           (1U << 24)      /* 2b */
#define SW_EMI_CLK_OFF_ACK_SEL_LSB          (1U << 26)      /* 2b */
#define SW_DR_SHORT_QUEUE_ACK_SEL_LSB       (1U << 28)      /* 2b */
#define SW_DRAMC_DFS_STA_SEL_LSB            (1U << 30)      /* 2b */

/* DRAMC_DPY_CLK_SW_CON (0x10006000+0x464) */
#define SW_DR_GATE_RETRY_EN_LSB             (1U << 0)       /* 2b */
#define SW_EMI_CLK_OFF_LSB                  (1U << 2)       /* 2b */
#define SW_DPY_MODE_SW_LSB                  (1U << 4)       /* 2b */
#define SW_DMSUS_OFF_LSB                    (1U << 6)       /* 2b */
#define SW_MEM_CK_OFF_LSB                   (1U << 8)       /* 2b */
#define SW_DPY_2ND_DLL_EN_LSB               (1U << 10)      /* 2b */
#define SW_DPY_DLL_EN_LSB                   (1U << 12)      /* 2b */
#define SW_DPY_DLL_CK_EN_LSB                (1U << 14)      /* 2b */
#define SW_DPY_VREF_EN_LSB                  (1U << 16)      /* 2b */
#define SW_PHYPLL_EN_LSB                    (1U << 18)      /* 2b */
#define SW_DDRPHY_FB_CK_EN_LSB              (1U << 20)      /* 2b */
#define SC_DR_SHU_EN_ACK_LSB                (1U << 24)      /* 2b */
#define EMI_CLK_OFF_ACK_LSB                 (1U << 26)      /* 2b */
#define SC_DR_SHORT_QUEUE_ACK_LSB           (1U << 28)      /* 2b */
#define SC_DRAMC_DFS_STA_LSB                (1U << 30)      /* 2b */

/* DRAMC_DPY_CLK_SW_CON_SEL2 (0x10006000+0x470) */
#define SW_PHYPLL_SHU_EN_SEL_LSB            (1U << 0)       /* 1b */
#define SW_PHYPLL2_SHU_EN_SEL_LSB           (1U << 1)       /* 1b */
#define SW_PHYPLL_MODE_SW_SEL_LSB           (1U << 2)       /* 1b */
#define SW_PHYPLL2_MODE_SW_SEL_LSB          (1U << 3)       /* 1b */
#define SW_DR_SHORT_QUEUE_SEL_LSB           (1U << 4)       /* 1b */
#define SW_DR_SHU_EN_SEL_LSB                (1U << 5)       /* 1b */
#define SW_DR_SHU_LEVEL_SEL_LSB             (1U << 6)       /* 1b */

/* DRAMC_DPY_CLK_SW_CON2 (0x10006000+0x474) */
#define SW_PHYPLL_SHU_EN_LSB                (1U << 0)       /* 1b */
#define SW_PHYPLL2_SHU_EN_LSB               (1U << 1)       /* 1b */
#define SW_PHYPLL_MODE_SW_LSB               (1U << 2)       /* 1b */
#define SW_PHYPLL2_MODE_SW_LSB              (1U << 3)       /* 1b */
#define SW_DR_SHORT_QUEUE_LSB               (1U << 4)       /* 1b */
#define SW_DR_SHU_EN_LSB                    (1U << 5)       /* 1b */
#define SW_DR_SHU_LEVEL_LSB                 (1U << 6)       /* 2b */
#define SPM2MM_ULTRAREQ_LSB                 (1U << 8)       /* 1b */
#define SPM2MD_ULTRAREQ_LSB                 (1U << 9)       /* 1b */
#define SPM2MM_ULTRAACK_D2T_LSB             (1U << 30)      /* 1b */
#define SPM2MD_ULTRAACK_D2T_LSB             (1U << 31)      /* 1b */

/* SPM_SEMA_M0 (0x10006000+0x480) */
#define SPM_SEMA_M0_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M1 (0x10006000+0x484) */
#define SPM_SEMA_M1_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M2 (0x10006000+0x488) */
#define SPM_SEMA_M2_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M3 (0x10006000+0x48C) */
#define SPM_SEMA_M3_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M4 (0x10006000+0x490) */
#define SPM_SEMA_M4_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M5 (0x10006000+0x494) */
#define SPM_SEMA_M5_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M6 (0x10006000+0x498) */
#define SPM_SEMA_M6_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M7 (0x10006000+0x49C) */
#define SPM_SEMA_M7_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M8 (0x10006000+0x4A0) */
#define SPM_SEMA_M8_LSB                     (1U << 0)       /* 8b */

/* SPM_SEMA_M9 (0x10006000+0x4A4) */
#define SPM_SEMA_M9_LSB                     (1U << 0)       /* 8b */

/* SRAM_DREQ_ACK (0x10006000+0x4AC) */
#define SRAM_DREQ_ACK_LSB                   (1U << 0)       /* 16b */

/* SRAM_DREQ_CON (0x10006000+0x4B0) */
#define SRAM_DREQ_CON_LSB                   (1U << 0)       /* 16b */

/* SRAM_DREQ_CON_SET (0x10006000+0x4B4) */
#define SRAM_DREQ_CON_SET_LSB               (1U << 0)       /* 16b */

/* SRAM_DREQ_CON_CLR (0x10006000+0x4B8) */
#define SRAM_DREQ_CON_CLR_LSB               (1U << 0)       /* 16b */

/* MP0_CPU0_IRQ_MASK (0x10006000+0x500) */
#define MP0_CPU0_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP0_CPU0_AUX_LSB                    (1U << 8)       /* 11b */

/* MP0_CPU1_IRQ_MASK (0x10006000+0x504) */
#define MP0_CPU1_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP0_CPU1_AUX_LSB                    (1U << 8)       /* 11b */

/* MP0_CPU2_IRQ_MASK (0x10006000+0x508) */
#define MP0_CPU2_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP0_CPU2_AUX_LSB                    (1U << 8)       /* 11b */

/* MP0_CPU3_IRQ_MASK (0x10006000+0x50C) */
#define MP0_CPU3_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP0_CPU3_AUX_LSB                    (1U << 8)       /* 11b */

/* MP1_CPU0_IRQ_MASK (0x10006000+0x510) */
#define MP1_CPU0_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP1_CPU0_AUX_LSB                    (1U << 8)       /* 11b */

/* MP1_CPU1_IRQ_MASK (0x10006000+0x514) */
#define MP1_CPU1_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP1_CPU1_AUX_LSB                    (1U << 8)       /* 11b */

/* MP1_CPU2_IRQ_MASK (0x10006000+0x518) */
#define MP1_CPU2_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP1_CPU2_AUX_LSB                    (1U << 8)       /* 11b */

/* MP1_CPU3_IRQ_MASK (0x10006000+0x51C) */
#define MP1_CPU3_IRQ_MASK_LSB               (1U << 0)       /* 1b */
#define MP1_CPU3_AUX_LSB                    (1U << 8)       /* 11b */

/* MP0_CPU0_WFI_EN (0x10006000+0x530) */
#define MP0_CPU0_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* MP0_CPU1_WFI_EN (0x10006000+0x534) */
#define MP0_CPU1_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* MP0_CPU2_WFI_EN (0x10006000+0x538) */
#define MP0_CPU2_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* MP0_CPU3_WFI_EN (0x10006000+0x53C) */
#define MP0_CPU3_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* MP1_CPU0_WFI_EN (0x10006000+0x540) */
#define MP1_CPU0_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* MP1_CPU1_WFI_EN (0x10006000+0x544) */
#define MP1_CPU1_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* MP1_CPU2_WFI_EN (0x10006000+0x548) */
#define MP1_CPU2_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* MP1_CPU3_WFI_EN (0x10006000+0x54C) */
#define MP1_CPU3_WFI_EN_LSB                 (1U << 0)       /* 1b */

/* CPU_PTPOD2_CON (0x10006000+0x560) */
#define MP0_PTPOD2_FBB_EN_LSB               (1U << 0)       /* 1b */
#define MP1_PTPOD2_FBB_EN_LSB               (1U << 1)       /* 1b */
#define MP0_PTPOD2_SPARK_EN_LSB             (1U << 2)       /* 1b */
#define MP1_PTPOD2_SPARK_EN_LSB             (1U << 3)       /* 1b */
#define MP0_PTPOD2_FBB_ACK_LSB              (1U << 4)       /* 1b */
#define MP1_PTPOD2_FBB_ACK_LSB              (1U << 5)       /* 1b */

/* ROOT_CPUTOP_ADDR (0x10006000+0x570) */
#define ROOT_CPUTOP_ADDR_LSB                (1U << 0)       /* 32b */

/* ROOT_CORE_ADDR (0x10006000+0x574) */
#define ROOT_CORE_ADDR_LSB                  (1U << 0)       /* 32b */

/* CPU_SPARE_CON (0x10006000+0x580) */
#define CPU_SPARE_CON_LSB                   (1U << 0)       /* 32b */

/* CPU_SPARE_CON_SET (0x10006000+0x584) */
#define CPU_SPARE_CON_SET_LSB               (1U << 0)       /* 32b */

/* CPU_SPARE_CON_CLR (0x10006000+0x588) */
#define CPU_SPARE_CON_CLR_LSB               (1U << 0)       /* 32b */

/* SPM_SW_FLAG (0x10006000+0x600) */
#define SPM_SW_FLAG_LSB                     (1U << 0)       /* 32b */

/* SPM_SW_DEBUG (0x10006000+0x604) */
#define SPM_SW_DEBUG_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_0 (0x10006000+0x608) */
#define SPM_SW_RSV_0_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_1 (0x10006000+0x60C) */
#define SPM_SW_RSV_1_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_2 (0x10006000+0x610) */
#define SPM_SW_RSV_2_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_3 (0x10006000+0x614) */
#define SPM_SW_RSV_3_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_4 (0x10006000+0x618) */
#define SPM_SW_RSV_4_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_5 (0x10006000+0x61C) */
#define SPM_SW_RSV_5_LSB                    (1U << 0)       /* 32b */

/* SPM_RSV_CON (0x10006000+0x620) */
#define SPM_RSV_CON_LSB                     (1U << 0)       /* 16b */

/* SPM_RSV_STA (0x10006000+0x624) */
#define SPM_RSV_STA_LSB                     (1U << 0)       /* 16b */

/* SPM_PASR_DPD_0 (0x10006000+0x630) */
#define SPM_PASR_DPD_0_LSB                  (1U << 0)       /* 32b */

/* SPM_PASR_DPD_1 (0x10006000+0x634) */
#define SPM_PASR_DPD_1_LSB                  (1U << 0)       /* 32b */

/* SPM_PASR_DPD_2 (0x10006000+0x638) */
#define SPM_PASR_DPD_2_LSB                  (1U << 0)       /* 32b */

/* SPM_PASR_DPD_3 (0x10006000+0x63C) */
#define SPM_PASR_DPD_3_LSB                  (1U << 0)       /* 32b */

/* SPM_SPARE_CON (0x10006000+0x640) */
#define SPM_SPARE_CON_LSB                   (1U << 0)       /* 32b */

/* SPM_SPARE_CON_SET (0x10006000+0x644) */
#define SPM_SPARE_CON_SET_LSB               (1U << 0)       /* 32b */

/* SPM_SPARE_CON_CLR (0x10006000+0x648) */
#define SPM_SPARE_CON_CLR_LSB               (1U << 0)       /* 32b */

/* SPM_SW_RSV_6 (0x10006000+0x64C) */
#define SPM_SW_RSV_6_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_7 (0x10006000+0x650) */
#define SPM_SW_RSV_7_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_8 (0x10006000+0x654) */
#define SPM_SW_RSV_8_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_9 (0x10006000+0x658) */
#define SPM_SW_RSV_9_LSB                    (1U << 0)       /* 32b */

/* SPM_SW_RSV_10 (0x10006000+0x65C) */
#define SPM_SW_RSV_10_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_11 (0x10006000+0x660) */
#define SPM_SW_RSV_11_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_12 (0x10006000+0x664) */
#define SPM_SW_RSV_12_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_13 (0x10006000+0x668) */
#define SPM_SW_RSV_13_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_14 (0x10006000+0x66C) */
#define SPM_SW_RSV_14_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_15 (0x10006000+0x670) */
#define SPM_SW_RSV_15_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_16 (0x10006000+0x674) */
#define SPM_SW_RSV_16_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_17 (0x10006000+0x678) */
#define SPM_SW_RSV_17_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_18 (0x10006000+0x67C) */
#define SPM_SW_RSV_18_LSB                   (1U << 0)       /* 32b */

/* SPM_SW_RSV_19 (0x10006000+0x680) */
#define SPM_SW_RSV_19_LSB                   (1U << 0)       /* 32b */

/* SW_CRTL_EVENT (0x10006000+0x690) */
#define SW_CRTL_EVENT_ON_LSB                (1U << 0)       /* 1b */

#define SPM_PROJECT_CODE	0xb16

#define SPM_REGWR_EN		(1U << 0)
#define SPM_REGWR_CFG_KEY	(SPM_PROJECT_CODE << 16)

#define SPM_CPU_PDN_DIS		(1U << 0)
#define SPM_INFRA_PDN_DIS	(1U << 1)
#define SPM_DDRPHY_PDN_DIS	(1U << 2)
#define SPM_DUALVCORE_PDN_DIS	(1U << 3)
#define SPM_PASR_DIS		(1U << 4)
#define SPM_DPD_DIS		(1U << 5)
#define SPM_SODI_DIS		(1U << 6)
#define SPM_MEMPLL_RESET	(1U << 7)
#define SPM_MAINPLL_PDN_DIS	(1U << 8)
#define SPM_CPU_DVS_DIS		(1U << 9)
#define SPM_CPU_DORMANT		(1U << 10)
#define SPM_EXT_VSEL_GPIO103	(1U << 11)
#define SPM_DDR_HIGH_SPEED	(1U << 12)
#define SPM_OPT			(1U << 13)

#define POWER_ON_VAL1_DEF	0x15820
#define PCM_FSM_STA_DEF		0x48490
#define PCM_END_FSM_STA_DEF	0x08490
#define PCM_END_FSM_STA_MASK	0x3fff0
#define PCM_HANDSHAKE_SEND1	0xbeefbeef

#define PCM_WDT_TIMEOUT		(30 * 32768)
#define PCM_TIMER_MAX		(0xffffffff - PCM_WDT_TIMEOUT)

#define CON0_PCM_KICK		(1U << 0)
#define CON0_IM_KICK		(1U << 1)
#define CON0_IM_SLEEP_DVS	(1U << 3)
#define CON0_PCM_SW_RESET	(1U << 15)
#define CON0_CFG_KEY		(SPM_PROJECT_CODE << 16)

#define CON1_IM_SLAVE		(1U << 0)
#define CON1_MIF_APBEN		(1U << 3)
#define CON1_PCM_TIMER_EN	(1U << 5)
#define CON1_IM_NONRP_EN	(1U << 6)
#define CON1_PCM_WDT_EN		(1U << 8)
#define CON1_PCM_WDT_WAKE_MODE	(1U << 9)
#define CON1_SPM_SRAM_SLP_B	(1U << 10)
#define CON1_SPM_SRAM_ISO_B	(1U << 11)
#define CON1_EVENT_LOCK_EN	(1U << 12)
#define CON1_CFG_KEY		(SPM_PROJECT_CODE << 16)

#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R2		(1U << 18)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

#define CC_SYSCLK0_EN_0		(1U << 0)
#define CC_SYSCLK0_EN_1		(1U << 1)
#define CC_SYSCLK1_EN_0		(1U << 2)
#define CC_SYSCLK1_EN_1		(1U << 3)
#define CC_SYSSETTLE_SEL	(1U << 4)
#define CC_LOCK_INFRA_DCM	(1U << 5)
#define CC_SRCLKENA_MASK_0	(1U << 6)
#define CC_CXO32K_RM_EN_MD1	(1U << 9)
#define CC_CXO32K_RM_EN_MD2	(1U << 10)
#define CC_CLKSQ1_SEL		(1U << 12)
#define CC_DISABLE_DORM_PWR	(1U << 14)
#define CC_MD32_DCM_EN		(1U << 18)

#define WFI_OP_AND		1
#define WFI_OP_OR		0

#define WAKE_MISC_PCM_TIMER	(1U << 19)
#define WAKE_MISC_CPU_WAKE	(1U << 20)

/* define WAKE_SRC_XXX */
#define WAKE_SRC_SPM_MERGE	(1 << 0)
#define WAKE_SRC_KP		(1 << 2)
#define WAKE_SRC_WDT		(1 << 3)
#define WAKE_SRC_GPT		(1 << 4)
#define WAKE_SRC_EINT		(1 << 6)
#define WAKE_SRC_LOW_BAT	(1 << 9)
#define WAKE_SRC_MD32		(1 << 10)
#define WAKE_SRC_USB_CD		(1 << 14)
#define WAKE_SRC_USB_PDN	(1 << 15)
#define WAKE_SRC_AFE		(1 << 20)
#define WAKE_SRC_THERM		(1 << 21)
#define WAKE_SRC_SYSPWREQ	(1 << 24)
#define WAKE_SRC_SEJ		(1 << 27)
#define WAKE_SRC_ALL_MD32	(1 << 28)
#define WAKE_SRC_CPU_IRQ	(1 << 29)

#define spm_read(addr)		mmio_read_32(addr)
#define spm_write(addr, val)	mmio_write_32(addr, val)

#endif /* SPM_H */
