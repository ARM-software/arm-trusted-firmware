/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_H
#define SPM_H

/**************************************
 * Define and Declare
 **************************************/

#define POWERON_CONFIG_EN              (SPM_BASE + 0x000)
#define SPM_POWER_ON_VAL0              (SPM_BASE + 0x004)
#define SPM_POWER_ON_VAL1              (SPM_BASE + 0x008)
#define SPM_CLK_CON                    (SPM_BASE + 0x00C)
#define SPM_CLK_SETTLE                 (SPM_BASE + 0x010)
#define SPM_AP_STANDBY_CON             (SPM_BASE + 0x014)
#define PCM_CON0                       (SPM_BASE + 0x018)
#define PCM_CON1                       (SPM_BASE + 0x01C)
#define PCM_IM_PTR                     (SPM_BASE + 0x020)
#define PCM_IM_LEN                     (SPM_BASE + 0x024)
#define PCM_REG_DATA_INI               (SPM_BASE + 0x028)
#define PCM_PWR_IO_EN                  (SPM_BASE + 0x02C)
#define PCM_TIMER_VAL                  (SPM_BASE + 0x030)
#define PCM_WDT_VAL                    (SPM_BASE + 0x034)
#define PCM_IM_HOST_RW_PTR             (SPM_BASE + 0x038)
#define PCM_IM_HOST_RW_DAT             (SPM_BASE + 0x03C)
#define PCM_EVENT_VECTOR0              (SPM_BASE + 0x040)
#define PCM_EVENT_VECTOR1              (SPM_BASE + 0x044)
#define PCM_EVENT_VECTOR2              (SPM_BASE + 0x048)
#define PCM_EVENT_VECTOR3              (SPM_BASE + 0x04C)
#define PCM_EVENT_VECTOR4              (SPM_BASE + 0x050)
#define PCM_EVENT_VECTOR5              (SPM_BASE + 0x054)
#define PCM_EVENT_VECTOR6              (SPM_BASE + 0x058)
#define PCM_EVENT_VECTOR7              (SPM_BASE + 0x05C)
#define PCM_EVENT_VECTOR8              (SPM_BASE + 0x060)
#define PCM_EVENT_VECTOR9              (SPM_BASE + 0x064)
#define PCM_EVENT_VECTOR10             (SPM_BASE + 0x068)
#define PCM_EVENT_VECTOR11             (SPM_BASE + 0x06C)
#define PCM_EVENT_VECTOR12             (SPM_BASE + 0x070)
#define PCM_EVENT_VECTOR13             (SPM_BASE + 0x074)
#define PCM_EVENT_VECTOR14             (SPM_BASE + 0x078)
#define PCM_EVENT_VECTOR15             (SPM_BASE + 0x07C)
#define PCM_EVENT_VECTOR_EN            (SPM_BASE + 0x080)
#define SPM_SRAM_RSV_CON               (SPM_BASE + 0x088)
#define SPM_SWINT                      (SPM_BASE + 0x08C)
#define SPM_SWINT_SET                  (SPM_BASE + 0x090)
#define SPM_SWINT_CLR                  (SPM_BASE + 0x094)
#define SPM_SCP_MAILBOX                (SPM_BASE + 0x098)
#define SCP_SPM_MAILBOX                (SPM_BASE + 0x09C)
#define SPM_TWAM_CON                   (SPM_BASE + 0x0A0)
#define SPM_TWAM_WINDOW_LEN            (SPM_BASE + 0x0A4)
#define SPM_TWAM_IDLE_SEL              (SPM_BASE + 0x0A8)
#define SPM_SCP_IRQ                    (SPM_BASE + 0x0AC)
#define SPM_CPU_WAKEUP_EVENT           (SPM_BASE + 0x0B0)
#define SPM_IRQ_MASK                   (SPM_BASE + 0x0B4)
#define SPM_SRC_REQ                    (SPM_BASE + 0x0B8)
#define SPM_SRC_MASK                   (SPM_BASE + 0x0BC)
#define SPM_SRC2_MASK                  (SPM_BASE + 0x0C0)
#define SPM_WAKEUP_EVENT_MASK          (SPM_BASE + 0x0C4)
#define SPM_WAKEUP_EVENT_EXT_MASK      (SPM_BASE + 0x0C8)
#define SPM_TWAM_EVENT_CLEAR           (SPM_BASE + 0x0CC)
#define SCP_CLK_CON                    (SPM_BASE + 0x0D0)
#define PCM_DEBUG_CON                  (SPM_BASE + 0x0D4)
#define DDR_EN_DBC_LEN                 (SPM_BASE + 0x0D8)
#define AHB_BUS_CON                    (SPM_BASE + 0x0DC)
#define SPM_SRC3_MASK                  (SPM_BASE + 0x0E0)
#define DDR_EN_EMI_DBC_CON             (SPM_BASE + 0x0E4)
#define SSPM_CLK_CON                   (SPM_BASE + 0x0E8)
#define PCM_REG0_DATA                  (SPM_BASE + 0x100)
#define PCM_REG1_DATA                  (SPM_BASE + 0x104)
#define PCM_REG2_DATA                  (SPM_BASE + 0x108)
#define PCM_REG3_DATA                  (SPM_BASE + 0x10C)
#define PCM_REG4_DATA                  (SPM_BASE + 0x110)
#define PCM_REG5_DATA                  (SPM_BASE + 0x114)
#define PCM_REG6_DATA                  (SPM_BASE + 0x118)
#define PCM_REG7_DATA                  (SPM_BASE + 0x11C)
#define PCM_REG8_DATA                  (SPM_BASE + 0x120)
#define PCM_REG9_DATA                  (SPM_BASE + 0x124)
#define PCM_REG10_DATA                 (SPM_BASE + 0x128)
#define PCM_REG11_DATA                 (SPM_BASE + 0x12C)
#define PCM_REG12_DATA                 (SPM_BASE + 0x130)
#define PCM_REG13_DATA                 (SPM_BASE + 0x134)
#define PCM_REG14_DATA                 (SPM_BASE + 0x138)
#define PCM_REG15_DATA                 (SPM_BASE + 0x13C)
#define PCM_REG12_MASK_B_STA           (SPM_BASE + 0x140)
#define PCM_REG12_EXT_DATA             (SPM_BASE + 0x144)
#define PCM_REG12_EXT_MASK_B_STA       (SPM_BASE + 0x148)
#define PCM_EVENT_REG_STA              (SPM_BASE + 0x14C)
#define PCM_TIMER_OUT                  (SPM_BASE + 0x150)
#define PCM_WDT_OUT                    (SPM_BASE + 0x154)
#define SPM_IRQ_STA                    (SPM_BASE + 0x158)
#define SPM_WAKEUP_STA                 (SPM_BASE + 0x15C)
#define SPM_WAKEUP_EXT_STA             (SPM_BASE + 0x160)
#define SPM_WAKEUP_MISC                (SPM_BASE + 0x164)
#define BUS_PROTECT_RDY                (SPM_BASE + 0x168)
#define BUS_PROTECT2_RDY               (SPM_BASE + 0x16C)
#define SUBSYS_IDLE_STA                (SPM_BASE + 0x170)
#define CPU_IDLE_STA                   (SPM_BASE + 0x174)
#define PCM_FSM_STA                    (SPM_BASE + 0x178)
#define SRC_REQ_STA                    (SPM_BASE + 0x17C)
#define PWR_STATUS                     (SPM_BASE + 0x180)
#define PWR_STATUS_2ND                 (SPM_BASE + 0x184)
#define CPU_PWR_STATUS                 (SPM_BASE + 0x188)
#define CPU_PWR_STATUS_2ND             (SPM_BASE + 0x18C)
#define MISC_STA                       (SPM_BASE + 0x190)
#define SPM_SRC_RDY_STA                (SPM_BASE + 0x194)
#define DRAMC_DBG_LATCH                (SPM_BASE + 0x19C)
#define SPM_TWAM_LAST_STA0             (SPM_BASE + 0x1A0)
#define SPM_TWAM_LAST_STA1             (SPM_BASE + 0x1A4)
#define SPM_TWAM_LAST_STA2             (SPM_BASE + 0x1A8)
#define SPM_TWAM_LAST_STA3             (SPM_BASE + 0x1AC)
#define SPM_TWAM_CURR_STA0             (SPM_BASE + 0x1B0)
#define SPM_TWAM_CURR_STA1             (SPM_BASE + 0x1B4)
#define SPM_TWAM_CURR_STA2             (SPM_BASE + 0x1B8)
#define SPM_TWAM_CURR_STA3             (SPM_BASE + 0x1BC)
#define SPM_TWAM_TIMER_OUT             (SPM_BASE + 0x1C0)
#define SPM_DVFS_STA                   (SPM_BASE + 0x1C8)
#define BUS_PROTECT3_RDY               (SPM_BASE + 0x1CC)
#define SRC_DDREN_STA                  (SPM_BASE + 0x1E0)
#define MCU_PWR_CON                    (SPM_BASE + 0x200)
#define MP0_CPUTOP_PWR_CON             (SPM_BASE + 0x204)
#define MP0_CPU0_PWR_CON               (SPM_BASE + 0x208)
#define MP0_CPU1_PWR_CON               (SPM_BASE + 0x20C)
#define MP0_CPU2_PWR_CON               (SPM_BASE + 0x210)
#define MP0_CPU3_PWR_CON               (SPM_BASE + 0x214)
#define MP1_CPUTOP_PWR_CON             (SPM_BASE + 0x218)
#define MP1_CPU0_PWR_CON               (SPM_BASE + 0x21C)
#define MP1_CPU1_PWR_CON               (SPM_BASE + 0x220)
#define MP1_CPU2_PWR_CON               (SPM_BASE + 0x224)
#define MP1_CPU3_PWR_CON               (SPM_BASE + 0x228)
#define MP0_CPUTOP_L2_PDN              (SPM_BASE + 0x240)
#define MP0_CPUTOP_L2_SLEEP_B          (SPM_BASE + 0x244)
#define MP0_CPU0_L1_PDN                (SPM_BASE + 0x248)
#define MP0_CPU1_L1_PDN                (SPM_BASE + 0x24C)
#define MP0_CPU2_L1_PDN                (SPM_BASE + 0x250)
#define MP0_CPU3_L1_PDN                (SPM_BASE + 0x254)
#define MP1_CPUTOP_L2_PDN              (SPM_BASE + 0x258)
#define MP1_CPUTOP_L2_SLEEP_B          (SPM_BASE + 0x25C)
#define MP1_CPU0_L1_PDN                (SPM_BASE + 0x260)
#define MP1_CPU1_L1_PDN                (SPM_BASE + 0x264)
#define MP1_CPU2_L1_PDN                (SPM_BASE + 0x268)
#define MP1_CPU3_L1_PDN                (SPM_BASE + 0x26C)
#define CPU_EXT_BUCK_ISO               (SPM_BASE + 0x290)
#define DUMMY1_PWR_CON                 (SPM_BASE + 0x2B0)
#define BYPASS_SPMC                    (SPM_BASE + 0x2B4)
#define SPMC_DORMANT_ENABLE            (SPM_BASE + 0x2B8)
#define ARMPLL_CLK_CON                 (SPM_BASE + 0x2BC)
#define SPMC_IN_RET                    (SPM_BASE + 0x2C0)
#define VDE_PWR_CON                    (SPM_BASE + 0x300)
#define VEN_PWR_CON                    (SPM_BASE + 0x304)
#define ISP_PWR_CON                    (SPM_BASE + 0x308)
#define DIS_PWR_CON                    (SPM_BASE + 0x30C)
#define MFG_CORE1_PWR_CON              (SPM_BASE + 0x310)
#define AUDIO_PWR_CON                  (SPM_BASE + 0x314)
#define IFR_PWR_CON                    (SPM_BASE + 0x318)
#define DPY_PWR_CON                    (SPM_BASE + 0x31C)
#define MD1_PWR_CON                    (SPM_BASE + 0x320)
#define VPU_TOP_PWR_CON                (SPM_BASE + 0x324)
#define CONN_PWR_CON                   (SPM_BASE + 0x32C)
#define VPU_CORE2_PWR_CON              (SPM_BASE + 0x330)
#define MFG_ASYNC_PWR_CON              (SPM_BASE + 0x334)
#define MFG_PWR_CON                    (SPM_BASE + 0x338)
#define VPU_CORE0_PWR_CON              (SPM_BASE + 0x33C)
#define VPU_CORE1_PWR_CON              (SPM_BASE + 0x340)
#define CAM_PWR_CON                    (SPM_BASE + 0x344)
#define MFG_2D_PWR_CON                 (SPM_BASE + 0x348)
#define MFG_CORE0_PWR_CON              (SPM_BASE + 0x34C)
#define SYSRAM_CON                     (SPM_BASE + 0x350)
#define SYSROM_CON                     (SPM_BASE + 0x354)
#define SSPM_SRAM_CON                  (SPM_BASE + 0x358)
#define SCP_SRAM_CON                   (SPM_BASE + 0x35C)
#define UFS_SRAM_CON                   (SPM_BASE + 0x36C)
#define DUMMY_SRAM_CON                 (SPM_BASE + 0x380)
#define MD_EXT_BUCK_ISO_CON            (SPM_BASE + 0x390)
#define MD_SRAM_ISO_CON                (SPM_BASE + 0x394)
#define MD_EXTRA_PWR_CON               (SPM_BASE + 0x398)
#define EXT_BUCK_CON                   (SPM_BASE + 0x3A0)
#define MBIST_EFUSE_REPAIR_ACK_STA     (SPM_BASE + 0x3D0)
#define SPM_DVFS_CON                   (SPM_BASE + 0x400)
#define SPM_MDBSI_CON                  (SPM_BASE + 0x404)
#define SPM_MAS_PAUSE_MASK_B           (SPM_BASE + 0x408)
#define SPM_MAS_PAUSE2_MASK_B          (SPM_BASE + 0x40C)
#define SPM_BSI_GEN                    (SPM_BASE + 0x410)
#define SPM_BSI_EN_SR                  (SPM_BASE + 0x414)
#define SPM_BSI_CLK_SR                 (SPM_BASE + 0x418)
#define SPM_BSI_D0_SR                  (SPM_BASE + 0x41C)
#define SPM_BSI_D1_SR                  (SPM_BASE + 0x420)
#define SPM_BSI_D2_SR                  (SPM_BASE + 0x424)
#define SPM_AP_SEMA                    (SPM_BASE + 0x428)
#define SPM_SPM_SEMA                   (SPM_BASE + 0x42C)
#define AP_MDSRC_REQ                   (SPM_BASE + 0x430)
#define SPM2MD_DVFS_CON                (SPM_BASE + 0x438)
#define MD2SPM_DVFS_CON                (SPM_BASE + 0x43C)
#define DRAMC_DPY_CLK_SW_CON_RSV       (SPM_BASE + 0x440)
#define DPY_LP_CON                     (SPM_BASE + 0x444)
#define CPU_DVFS_REQ                   (SPM_BASE + 0x448)
#define SPM_PLL_CON                    (SPM_BASE + 0x44C)
#define SPM_EMI_BW_MODE                (SPM_BASE + 0x450)
#define AP2MD_PEER_WAKEUP              (SPM_BASE + 0x454)
#define ULPOSC_CON                     (SPM_BASE + 0x458)
#define SPM2MM_CON                     (SPM_BASE + 0x45C)
#define DRAMC_DPY_CLK_SW_CON_SEL       (SPM_BASE + 0x460)
#define DRAMC_DPY_CLK_SW_CON           (SPM_BASE + 0x464)
#define SPM_S1_MODE_CH                 (SPM_BASE + 0x468)
#define EMI_SELF_REFRESH_CH_STA        (SPM_BASE + 0x46C)
#define DRAMC_DPY_CLK_SW_CON_SEL2      (SPM_BASE + 0x470)
#define DRAMC_DPY_CLK_SW_CON2          (SPM_BASE + 0x474)
#define DRAMC_DMYRD_CON                (SPM_BASE + 0x478)
#define SPM_DRS_CON                    (SPM_BASE + 0x47C)
#define SPM_SEMA_M0                    (SPM_BASE + 0x480)
#define SPM_SEMA_M1                    (SPM_BASE + 0x484)
#define SPM_SEMA_M2                    (SPM_BASE + 0x488)
#define SPM_SEMA_M3                    (SPM_BASE + 0x48C)
#define SPM_SEMA_M4                    (SPM_BASE + 0x490)
#define SPM_SEMA_M5                    (SPM_BASE + 0x494)
#define SPM_SEMA_M6                    (SPM_BASE + 0x498)
#define SPM_SEMA_M7                    (SPM_BASE + 0x49C)
#define SPM_MAS_PAUSE_MM_MASK_B        (SPM_BASE + 0x4A0)
#define SPM_MAS_PAUSE_MCU_MASK_B       (SPM_BASE + 0x4A4)
#define SRAM_DREQ_ACK                  (SPM_BASE + 0x4AC)
#define SRAM_DREQ_CON                  (SPM_BASE + 0x4B0)
#define SRAM_DREQ_CON_SET              (SPM_BASE + 0x4B4)
#define SRAM_DREQ_CON_CLR              (SPM_BASE + 0x4B8)
#define SPM2EMI_ENTER_ULPM             (SPM_BASE + 0x4BC)
#define SPM_SSPM_IRQ                   (SPM_BASE + 0x4C0)
#define SPM2PMCU_INT                   (SPM_BASE + 0x4C4)
#define SPM2PMCU_INT_SET               (SPM_BASE + 0x4C8)
#define SPM2PMCU_INT_CLR               (SPM_BASE + 0x4CC)
#define SPM2PMCU_MAILBOX_0             (SPM_BASE + 0x4D0)
#define SPM2PMCU_MAILBOX_1             (SPM_BASE + 0x4D4)
#define SPM2PMCU_MAILBOX_2             (SPM_BASE + 0x4D8)
#define SPM2PMCU_MAILBOX_3             (SPM_BASE + 0x4DC)
#define PMCU2SPM_INT                   (SPM_BASE + 0x4E0)
#define PMCU2SPM_INT_SET               (SPM_BASE + 0x4E4)
#define PMCU2SPM_INT_CLR               (SPM_BASE + 0x4E8)
#define PMCU2SPM_MAILBOX_0             (SPM_BASE + 0x4EC)
#define PMCU2SPM_MAILBOX_1             (SPM_BASE + 0x4F0)
#define PMCU2SPM_MAILBOX_2             (SPM_BASE + 0x4F4)
#define PMCU2SPM_MAILBOX_3             (SPM_BASE + 0x4F8)
#define PMCU2SPM_CFG                   (SPM_BASE + 0x4FC)
#define MP0_CPU0_IRQ_MASK              (SPM_BASE + 0x500)
#define MP0_CPU1_IRQ_MASK              (SPM_BASE + 0x504)
#define MP0_CPU2_IRQ_MASK              (SPM_BASE + 0x508)
#define MP0_CPU3_IRQ_MASK              (SPM_BASE + 0x50C)
#define MP1_CPU0_IRQ_MASK              (SPM_BASE + 0x510)
#define MP1_CPU1_IRQ_MASK              (SPM_BASE + 0x514)
#define MP1_CPU2_IRQ_MASK              (SPM_BASE + 0x518)
#define MP1_CPU3_IRQ_MASK              (SPM_BASE + 0x51C)
#define MP0_CPU0_WFI_EN                (SPM_BASE + 0x530)
#define MP0_CPU1_WFI_EN                (SPM_BASE + 0x534)
#define MP0_CPU2_WFI_EN                (SPM_BASE + 0x538)
#define MP0_CPU3_WFI_EN                (SPM_BASE + 0x53C)
#define MP1_CPU0_WFI_EN                (SPM_BASE + 0x540)
#define MP1_CPU1_WFI_EN                (SPM_BASE + 0x544)
#define MP1_CPU2_WFI_EN                (SPM_BASE + 0x548)
#define MP1_CPU3_WFI_EN                (SPM_BASE + 0x54C)
#define MP0_L2CFLUSH                   (SPM_BASE + 0x554)
#define MP1_L2CFLUSH                   (SPM_BASE + 0x558)
#define CPU_PTPOD2_CON                 (SPM_BASE + 0x560)
#define ROOT_CPUTOP_ADDR               (SPM_BASE + 0x570)
#define ROOT_CORE_ADDR                 (SPM_BASE + 0x574)
#define CPU_SPARE_CON                  (SPM_BASE + 0x580)
#define CPU_SPARE_CON_SET              (SPM_BASE + 0x584)
#define CPU_SPARE_CON_CLR              (SPM_BASE + 0x588)
#define SPM2SW_MAILBOX_0               (SPM_BASE + 0x5D0)
#define SPM2SW_MAILBOX_1               (SPM_BASE + 0x5D4)
#define SPM2SW_MAILBOX_2               (SPM_BASE + 0x5D8)
#define SPM2SW_MAILBOX_3               (SPM_BASE + 0x5DC)
#define SW2SPM_INT                     (SPM_BASE + 0x5E0)
#define SW2SPM_INT_SET                 (SPM_BASE + 0x5E4)
#define SW2SPM_INT_CLR                 (SPM_BASE + 0x5E8)
#define SW2SPM_MAILBOX_0               (SPM_BASE + 0x5EC)
#define SW2SPM_MAILBOX_1               (SPM_BASE + 0x5F0)
#define SW2SPM_MAILBOX_2               (SPM_BASE + 0x5F4)
#define SW2SPM_MAILBOX_3               (SPM_BASE + 0x5F8)
#define SW2SPM_CFG                     (SPM_BASE + 0x5FC)
#define SPM_SW_FLAG                    (SPM_BASE + 0x600)
#define SPM_SW_DEBUG                   (SPM_BASE + 0x604)
#define SPM_SW_RSV_0                   (SPM_BASE + 0x608)
#define SPM_SW_RSV_1                   (SPM_BASE + 0x60C)
#define SPM_SW_RSV_2                   (SPM_BASE + 0x610)
#define SPM_SW_RSV_3                   (SPM_BASE + 0x614)
#define SPM_SW_RSV_4                   (SPM_BASE + 0x618)
#define SPM_SW_RSV_5                   (SPM_BASE + 0x61C)
#define SPM_RSV_CON                    (SPM_BASE + 0x620)
#define SPM_RSV_STA                    (SPM_BASE + 0x624)
#define SPM_RSV_CON1                   (SPM_BASE + 0x628)
#define SPM_RSV_STA1                   (SPM_BASE + 0x62C)
#define SPM_PASR_DPD_0                 (SPM_BASE + 0x630)
#define SPM_PASR_DPD_1                 (SPM_BASE + 0x634)
#define SPM_PASR_DPD_2                 (SPM_BASE + 0x638)
#define SPM_PASR_DPD_3                 (SPM_BASE + 0x63C)
#define SPM_SPARE_CON                  (SPM_BASE + 0x640)
#define SPM_SPARE_CON_SET              (SPM_BASE + 0x644)
#define SPM_SPARE_CON_CLR              (SPM_BASE + 0x648)
#define SPM_SW_RSV_6                   (SPM_BASE + 0x64C)
#define SPM_SW_RSV_7                   (SPM_BASE + 0x650)
#define SPM_SW_RSV_8                   (SPM_BASE + 0x654)
#define SPM_SW_RSV_9                   (SPM_BASE + 0x658)
#define SPM_SW_RSV_10                  (SPM_BASE + 0x65C)
#define SPM_SW_RSV_18                  (SPM_BASE + 0x67C)
#define SPM_SW_RSV_19                  (SPM_BASE + 0x680)
#define DVFSRC_EVENT_MASK_CON          (SPM_BASE + 0x690)
#define DVFSRC_EVENT_FORCE_ON          (SPM_BASE + 0x694)
#define DVFSRC_EVENT_SEL               (SPM_BASE + 0x698)
#define SPM_DVFS_EVENT_STA             (SPM_BASE + 0x69C)
#define SPM_DVFS_EVENT_STA1            (SPM_BASE + 0x6A0)
#define SPM_DVFS_LEVEL                 (SPM_BASE + 0x6A4)
#define DVFS_ABORT_STA                 (SPM_BASE + 0x6A8)
#define DVFS_ABORT_OTHERS_MASK         (SPM_BASE + 0x6AC)
#define SPM_DFS_LEVEL                  (SPM_BASE + 0x6B0)
#define SPM_DVS_LEVEL                  (SPM_BASE + 0x6B4)
#define SPM_DVFS_MISC                  (SPM_BASE + 0x6B8)
#define SPARE_SRC_REQ_MASK             (SPM_BASE + 0x6C0)
#define SCP_VCORE_LEVEL                (SPM_BASE + 0x6C4)
#define SC_MM_CK_SEL_CON               (SPM_BASE + 0x6C8)
#define SPARE_ACK_STA                  (SPM_BASE + 0x6F0)
#define SPARE_ACK_MASK                 (SPM_BASE + 0x6F4)
#define SPM_DVFS_CON1                  (SPM_BASE + 0x700)
#define SPM_DVFS_CON1_STA              (SPM_BASE + 0x704)
#define SPM_DVFS_CMD0                  (SPM_BASE + 0x710)
#define SPM_DVFS_CMD1                  (SPM_BASE + 0x714)
#define SPM_DVFS_CMD2                  (SPM_BASE + 0x718)
#define SPM_DVFS_CMD3                  (SPM_BASE + 0x71C)
#define SPM_DVFS_CMD4                  (SPM_BASE + 0x720)
#define SPM_DVFS_CMD5                  (SPM_BASE + 0x724)
#define SPM_DVFS_CMD6                  (SPM_BASE + 0x728)
#define SPM_DVFS_CMD7                  (SPM_BASE + 0x72C)
#define SPM_DVFS_CMD8                  (SPM_BASE + 0x730)
#define SPM_DVFS_CMD9                  (SPM_BASE + 0x734)
#define SPM_DVFS_CMD10                 (SPM_BASE + 0x738)
#define SPM_DVFS_CMD11                 (SPM_BASE + 0x73C)
#define SPM_DVFS_CMD12                 (SPM_BASE + 0x740)
#define SPM_DVFS_CMD13                 (SPM_BASE + 0x744)
#define SPM_DVFS_CMD14                 (SPM_BASE + 0x748)
#define SPM_DVFS_CMD15                 (SPM_BASE + 0x74C)
#define WDT_LATCH_SPARE0_FIX           (SPM_BASE + 0x780)
#define WDT_LATCH_SPARE1_FIX           (SPM_BASE + 0x784)
#define WDT_LATCH_SPARE2_FIX           (SPM_BASE + 0x788)
#define WDT_LATCH_SPARE3_FIX           (SPM_BASE + 0x78C)
#define SPARE_ACK_IN_FIX               (SPM_BASE + 0x790)
#define DCHA_LATCH_RSV0_FIX            (SPM_BASE + 0x794)
#define DCHB_LATCH_RSV0_FIX            (SPM_BASE + 0x798)
#define PCM_WDT_LATCH_0                (SPM_BASE + 0x800)
#define PCM_WDT_LATCH_1                (SPM_BASE + 0x804)
#define PCM_WDT_LATCH_2                (SPM_BASE + 0x808)
#define PCM_WDT_LATCH_3                (SPM_BASE + 0x80C)
#define PCM_WDT_LATCH_4                (SPM_BASE + 0x810)
#define PCM_WDT_LATCH_5                (SPM_BASE + 0x814)
#define PCM_WDT_LATCH_6                (SPM_BASE + 0x818)
#define PCM_WDT_LATCH_7                (SPM_BASE + 0x81C)
#define PCM_WDT_LATCH_8                (SPM_BASE + 0x820)
#define PCM_WDT_LATCH_9                (SPM_BASE + 0x824)
#define WDT_LATCH_SPARE0               (SPM_BASE + 0x828)
#define WDT_LATCH_SPARE1               (SPM_BASE + 0x82C)
#define WDT_LATCH_SPARE2               (SPM_BASE + 0x830)
#define WDT_LATCH_SPARE3               (SPM_BASE + 0x834)
#define PCM_WDT_LATCH_10               (SPM_BASE + 0x838)
#define PCM_WDT_LATCH_11               (SPM_BASE + 0x83C)
#define DCHA_GATING_LATCH_0            (SPM_BASE + 0x840)
#define DCHA_GATING_LATCH_1            (SPM_BASE + 0x844)
#define DCHA_GATING_LATCH_2            (SPM_BASE + 0x848)
#define DCHA_GATING_LATCH_3            (SPM_BASE + 0x84C)
#define DCHA_GATING_LATCH_4            (SPM_BASE + 0x850)
#define DCHA_GATING_LATCH_5            (SPM_BASE + 0x854)
#define DCHA_GATING_LATCH_6            (SPM_BASE + 0x858)
#define DCHA_GATING_LATCH_7            (SPM_BASE + 0x85C)
#define DCHB_GATING_LATCH_0            (SPM_BASE + 0x860)
#define DCHB_GATING_LATCH_1            (SPM_BASE + 0x864)
#define DCHB_GATING_LATCH_2            (SPM_BASE + 0x868)
#define DCHB_GATING_LATCH_3            (SPM_BASE + 0x86C)
#define DCHB_GATING_LATCH_4            (SPM_BASE + 0x870)
#define DCHB_GATING_LATCH_5            (SPM_BASE + 0x874)
#define DCHB_GATING_LATCH_6            (SPM_BASE + 0x878)
#define DCHB_GATING_LATCH_7            (SPM_BASE + 0x87C)
#define DCHA_LATCH_RSV0                (SPM_BASE + 0x880)
#define DCHB_LATCH_RSV0                (SPM_BASE + 0x884)
#define PCM_WDT_LATCH_12               (SPM_BASE + 0x888)
#define PCM_WDT_LATCH_13               (SPM_BASE + 0x88C)
#define SPM_PC_TRACE_CON               (SPM_BASE + 0x8C0)
#define SPM_PC_TRACE_G0                (SPM_BASE + 0x8C4)
#define SPM_PC_TRACE_G1                (SPM_BASE + 0x8C8)
#define SPM_PC_TRACE_G2                (SPM_BASE + 0x8CC)
#define SPM_PC_TRACE_G3                (SPM_BASE + 0x8D0)
#define SPM_PC_TRACE_G4                (SPM_BASE + 0x8D4)
#define SPM_PC_TRACE_G5                (SPM_BASE + 0x8D8)
#define SPM_PC_TRACE_G6                (SPM_BASE + 0x8DC)
#define SPM_PC_TRACE_G7                (SPM_BASE + 0x8E0)
#define SPM_ACK_CHK_CON                (SPM_BASE + 0x900)
#define SPM_ACK_CHK_PC                 (SPM_BASE + 0x904)
#define SPM_ACK_CHK_SEL                (SPM_BASE + 0x908)
#define SPM_ACK_CHK_TIMER              (SPM_BASE + 0x90C)
#define SPM_ACK_CHK_STA                (SPM_BASE + 0x910)
#define SPM_ACK_CHK_LATCH              (SPM_BASE + 0x914)
#define SPM_ACK_CHK_CON2               (SPM_BASE + 0x920)
#define SPM_ACK_CHK_PC2                (SPM_BASE + 0x924)
#define SPM_ACK_CHK_SEL2               (SPM_BASE + 0x928)
#define SPM_ACK_CHK_TIMER2             (SPM_BASE + 0x92C)
#define SPM_ACK_CHK_STA2               (SPM_BASE + 0x930)
#define SPM_ACK_CHK_LATCH2             (SPM_BASE + 0x934)
#define SPM_ACK_CHK_CON3               (SPM_BASE + 0x940)
#define SPM_ACK_CHK_PC3                (SPM_BASE + 0x944)
#define SPM_ACK_CHK_SEL3               (SPM_BASE + 0x948)
#define SPM_ACK_CHK_TIMER3             (SPM_BASE + 0x94C)
#define SPM_ACK_CHK_STA3               (SPM_BASE + 0x950)
#define SPM_ACK_CHK_LATCH3             (SPM_BASE + 0x954)
#define SPM_ACK_CHK_CON4               (SPM_BASE + 0x960)
#define SPM_ACK_CHK_PC4                (SPM_BASE + 0x964)
#define SPM_ACK_CHK_SEL4               (SPM_BASE + 0x968)
#define SPM_ACK_CHK_TIMER4             (SPM_BASE + 0x96C)
#define SPM_ACK_CHK_STA4               (SPM_BASE + 0x970)
#define SPM_ACK_CHK_LATCH4             (SPM_BASE + 0x974)

/* POWERON_CONFIG_EN (0x10006000+0x000) */
#define BCLK_CG_EN_LSB                      (1U << 0)       /* 1b */
#define MD_BCLK_CG_EN_LSB                   (1U << 1)       /* 1b */
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
#define MD_DDR_EN_0_DBC_EN_LSB              (1U << 18)      /* 1b */
#define MD_DDR_EN_1_DBC_EN_LSB              (1U << 19)      /* 1b */
#define MD_MASK_B_LSB                       (1U << 20)      /* 2b */
#define SSPM_MASK_B_LSB                     (1U << 22)      /* 1b */
#define SCP_MASK_B_LSB                      (1U << 23)      /* 1b */
#define SRCCLKENI_MASK_B_LSB                (1U << 24)      /* 1b */
#define MD_APSRC_1_SEL_LSB                  (1U << 25)      /* 1b */
#define MD_APSRC_0_SEL_LSB                  (1U << 26)      /* 1b */
#define CONN_DDR_EN_DBC_EN_LSB              (1U << 27)      /* 1b */
#define CONN_MASK_B_LSB                     (1U << 28)      /* 1b */
#define CONN_APSRC_SEL_LSB                  (1U << 29)      /* 1b */
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
/* SPM_SRAM_RSV_CON (0x10006000+0x088) */
#define SPM_SRAM_SLEEP_B_ECO_EN_LSB         (1U << 0)       /* 1b */
/* SPM_SWINT (0x10006000+0x08C) */
#define SPM_SWINT_LSB                       (1U << 0)       /* 10b */
/* SPM_SWINT_SET (0x10006000+0x090) */
#define SPM_SWINT_SET_LSB                   (1U << 0)       /* 10b */
/* SPM_SWINT_CLR (0x10006000+0x094) */
#define SPM_SWINT_CLR_LSB                   (1U << 0)       /* 10b */
/* SPM_SCP_MAILBOX (0x10006000+0x098) */
#define SPM_SCP_MAILBOX_LSB                 (1U << 0)       /* 32b */
/* SCP_SPM_MAILBOX (0x10006000+0x09C) */
#define SCP_SPM_MAILBOX_LSB                 (1U << 0)       /* 32b */
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
/* SPM_SCP_IRQ (0x10006000+0x0AC) */
#define SPM_SCP_IRQ_LSB                     (1U << 0)       /* 1b */
#define SPM_SCP_IRQ_SEL_LSB                 (1U << 4)       /* 1b */
/* SPM_CPU_WAKEUP_EVENT (0x10006000+0x0B0) */
#define SPM_CPU_WAKEUP_EVENT_LSB            (1U << 0)       /* 1b */
/* SPM_IRQ_MASK (0x10006000+0x0B4) */
#define SPM_TWAM_IRQ_MASK_LSB               (1U << 2)       /* 1b */
#define PCM_IRQ_ROOT_MASK_LSB               (1U << 3)       /* 1b */
#define SPM_IRQ_MASK_LSB                    (1U << 8)       /* 10b */
/* SPM_SRC_REQ (0x10006000+0x0B8) */
#define SPM_APSRC_REQ_LSB                   (1U << 0)       /* 1b */
#define SPM_F26M_REQ_LSB                    (1U << 1)       /* 1b */
#define SPM_INFRA_REQ_LSB                   (1U << 3)       /* 1b */
#define SPM_VRF18_REQ_LSB                   (1U << 4)       /* 1b */
#define SPM_DDREN_REQ_LSB                   (1U << 7)       /* 1b */
#define SPM_RSV_SRC_REQ_LSB                 (1U << 8)       /* 3b */
#define SPM_DDREN_2_REQ_LSB                 (1U << 11)      /* 1b */
#define CPU_MD_DVFS_SOP_FORCE_ON_LSB        (1U << 16)      /* 1b */
/* SPM_SRC_MASK (0x10006000+0x0BC) */
#define CSYSPWREQ_MASK_LSB                  (1U << 0)       /* 1b */
#define CCIF0_MD_EVENT_MASK_B_LSB           (1U << 1)       /* 1b */
#define CCIF0_AP_EVENT_MASK_B_LSB           (1U << 2)       /* 1b */
#define CCIF1_MD_EVENT_MASK_B_LSB           (1U << 3)       /* 1b */
#define CCIF1_AP_EVENT_MASK_B_LSB           (1U << 4)       /* 1b */
#define CCIF2_MD_EVENT_MASK_B_LSB           (1U << 5)       /* 1b */
#define CCIF2_AP_EVENT_MASK_B_LSB           (1U << 6)       /* 1b */
#define CCIF3_MD_EVENT_MASK_B_LSB           (1U << 7)       /* 1b */
#define CCIF3_AP_EVENT_MASK_B_LSB           (1U << 8)       /* 1b */
#define MD_SRCCLKENA_0_INFRA_MASK_B_LSB     (1U << 9)       /* 1b */
#define MD_SRCCLKENA_1_INFRA_MASK_B_LSB     (1U << 10)      /* 1b */
#define CONN_SRCCLKENA_INFRA_MASK_B_LSB     (1U << 11)      /* 1b */
#define UFS_INFRA_REQ_MASK_B_LSB            (1U << 12)      /* 1b */
#define SRCCLKENI_INFRA_MASK_B_LSB          (1U << 13)      /* 1b */
#define MD_APSRC_REQ_0_INFRA_MASK_B_LSB     (1U << 14)      /* 1b */
#define MD_APSRC_REQ_1_INFRA_MASK_B_LSB     (1U << 15)      /* 1b */
#define CONN_APSRCREQ_INFRA_MASK_B_LSB      (1U << 16)      /* 1b */
#define UFS_SRCCLKENA_MASK_B_LSB            (1U << 17)      /* 1b */
#define MD_VRF18_REQ_0_MASK_B_LSB           (1U << 18)      /* 1b */
#define MD_VRF18_REQ_1_MASK_B_LSB           (1U << 19)      /* 1b */
#define UFS_VRF18_REQ_MASK_B_LSB            (1U << 20)      /* 1b */
#define GCE_VRF18_REQ_MASK_B_LSB            (1U << 21)      /* 1b */
#define CONN_INFRA_REQ_MASK_B_LSB           (1U << 22)      /* 1b */
#define GCE_APSRC_REQ_MASK_B_LSB            (1U << 23)      /* 1b */
#define DISP0_APSRC_REQ_MASK_B_LSB          (1U << 24)      /* 1b */
#define DISP1_APSRC_REQ_MASK_B_LSB          (1U << 25)      /* 1b */
#define MFG_REQ_MASK_B_LSB                  (1U << 26)      /* 1b */
#define VDEC_REQ_MASK_B_LSB                 (1U << 27)      /* 1b */
/* SPM_SRC2_MASK (0x10006000+0x0C0) */
#define MD_DDR_EN_0_MASK_B_LSB              (1U << 0)       /* 1b */
#define MD_DDR_EN_1_MASK_B_LSB              (1U << 1)       /* 1b */
#define CONN_DDR_EN_MASK_B_LSB              (1U << 2)       /* 1b */
#define DDREN_SSPM_APSRC_REQ_MASK_B_LSB     (1U << 3)       /* 1b */
#define DDREN_SCP_APSRC_REQ_MASK_B_LSB      (1U << 4)       /* 1b */
#define DISP0_DDREN_MASK_B_LSB              (1U << 5)       /* 1b */
#define DISP1_DDREN_MASK_B_LSB              (1U << 6)       /* 1b */
#define GCE_DDREN_MASK_B_LSB                (1U << 7)       /* 1b */
#define DDREN_EMI_SELF_REFRESH_CH0_MASK_B_LSB (1U << 8)       /* 1b */
#define DDREN_EMI_SELF_REFRESH_CH1_MASK_B_LSB (1U << 9)       /* 1b */
/* SPM_WAKEUP_EVENT_MASK (0x10006000+0x0C4) */
#define SPM_WAKEUP_EVENT_MASK_LSB           (1U << 0)       /* 32b */
/* SPM_WAKEUP_EVENT_EXT_MASK (0x10006000+0x0C8) */
#define SPM_WAKEUP_EVENT_EXT_MASK_LSB       (1U << 0)       /* 32b */
/* SPM_TWAM_EVENT_CLEAR (0x10006000+0x0CC) */
#define SPM_TWAM_EVENT_CLEAR_LSB            (1U << 0)       /* 1b */
/* SCP_CLK_CON (0x10006000+0x0D0) */
#define SCP_26M_CK_SEL_LSB                  (1U << 0)       /* 1b */
#define SCP_SECURE_V_REQ_MASK_LSB           (1U << 1)       /* 1b */
#define SCP_SLP_REQ_LSB                     (1U << 2)       /* 1b */
#define SCP_SLP_ACK_LSB                     (1U << 3)       /* 1b */
/* PCM_DEBUG_CON (0x10006000+0x0D4) */
#define PCM_DEBUG_OUT_ENABLE_LSB            (1U << 0)       /* 1b */
/* DDR_EN_DBC_LEN (0x10006000+0x0D8) */
#define MD_DDR_EN_0_DBC_LEN_LSB             (1U << 0)       /* 10b */
#define MD_DDR_EN_1_DBC_LEN_LSB             (1U << 10)      /* 10b */
#define CONN_DDR_EN_DBC_LEN_LSB             (1U << 20)      /* 10b */
/* AHB_BUS_CON (0x10006000+0x0DC) */
#define AHB_HADDR_EXT_LSB                   (1U << 0)       /* 2b */
#define REG_AHB_LOCK_LSB                    (1U << 8)       /* 1b */
/* SPM_SRC3_MASK (0x10006000+0x0E0) */
#define MD_DDR_EN_2_0_MASK_B_LSB            (1U << 0)       /* 1b */
#define MD_DDR_EN_2_1_MASK_B_LSB            (1U << 1)       /* 1b */
#define CONN_DDR_EN_2_MASK_B_LSB            (1U << 2)       /* 1b */
#define DDREN2_SSPM_APSRC_REQ_MASK_B_LSB    (1U << 3)       /* 1b */
#define DDREN2_SCP_APSRC_REQ_MASK_B_LSB     (1U << 4)       /* 1b */
#define DISP0_DDREN2_MASK_B_LSB             (1U << 5)       /* 1b */
#define DISP1_DDREN2_MASK_B_LSB             (1U << 6)       /* 1b */
#define GCE_DDREN2_MASK_B_LSB               (1U << 7)       /* 1b */
#define DDREN2_EMI_SELF_REFRESH_CH0_MASK_B_LSB (1U << 8)       /* 1b */
#define DDREN2_EMI_SELF_REFRESH_CH1_MASK_B_LSB (1U << 9)       /* 1b */
/* DDR_EN_EMI_DBC_CON (0x10006000+0x0E4) */
#define EMI_SELF_REFRESH_CH0_DBC_LEN_LSB    (1U << 0)       /* 10b */
#define EMI_SELF_REFRESH_CH0_DBC_EN_LSB     (1U << 10)      /* 1b */
#define EMI_SELF_REFRESH_CH1_DBC_LEN_LSB    (1U << 16)      /* 10b */
#define EMI_SELF_REFRESH_CH1_DBC_EN_LSB     (1U << 26)      /* 1b */
/* SSPM_CLK_CON (0x10006000+0x0E8) */
#define SSPM_26M_CK_SEL_LSB                 (1U << 0)       /* 1b */
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
#define SPM_ACK_CHK_WAKEUP_LSB              (1U << 1)       /* 1b */
#define TWAM_IRQ_LSB                        (1U << 2)       /* 1b */
#define PCM_IRQ_LSB                         (1U << 3)       /* 1b */
/* #define SPM_SWINT_LSB                    (1U << 4) */       /* 10b */
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
/* SRC_REQ_STA (0x10006000+0x17C) */
#define SRC_REQ_STA_LSB                     (1U << 0)       /* 32b */
/* PWR_STATUS (0x10006000+0x180) */
#define PWR_STATUS_LSB                      (1U << 0)       /* 32b */
/* PWR_STATUS_2ND (0x10006000+0x184) */
#define PWR_STATUS_2ND_LSB                  (1U << 0)       /* 32b */
/* CPU_PWR_STATUS (0x10006000+0x188) */
#define CPU_PWR_STATUS_LSB                  (1U << 0)       /* 32b */
/* CPU_PWR_STATUS_2ND (0x10006000+0x18C) */
#define CPU_PWR_STATUS_2ND_LSB              (1U << 0)       /* 32b */
/* MISC_STA (0x10006000+0x190) */
#define MM_DVFS_HALT_AF_MASK_LSB            (1U << 0)       /* 5b */
/* SPM_SRC_RDY_STA (0x10006000+0x194) */
#define SPM_INFRA_SRC_ACK_LSB               (1U << 0)       /* 1b */
#define SPM_VRF18_SRC_ACK_LSB               (1U << 1)       /* 1b */
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
/* SPM_DVFS_STA (0x10006000+0x1C8) */
#define MD_DVFS_ERROR_STATUS_LSB            (1U << 0)       /* 1b */
/* BUS_PROTECT3_RDY (0x10006000+0x1CC) */
#define BUS_PROTECT_MM_RDY_LSB              (1U << 0)       /* 16b */
#define BUS_PROTECT_MCU_RDY_LSB             (1U << 16)      /* 16b */
/* SRC_DDREN_STA (0x10006000+0x1E0) */
#define SRC_DDREN_STA_LSB                   (1U << 0)       /* 32b */
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
#define MP_EXT_BUCK_ISO_LSB                 (1U << 2)       /* 1b */
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
#define REG_SC_ARM_FHC_PAUSE_LSB            (1U << 0)       /* 3b */
#define REG_SC_ARM_CLK_OFF_LSB              (1U << 3)       /* 3b */
#define REG_SC_ARMPLLOUT_OFF_LSB            (1U << 6)       /* 3b */
#define REG_SC_ARMPLL_OFF_LSB               (1U << 9)       /* 3b */
#define REG_SC_ARMPLL_S_OFF_LSB             (1U << 12)      /* 3b */
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
/* MFG_CORE1_PWR_CON (0x10006000+0x310) */
#define MFG_CORE1_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define MFG_CORE1_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define MFG_CORE1_PWR_ON_LSB                (1U << 2)       /* 1b */
#define MFG_CORE1_PWR_ON_2ND_LSB            (1U << 3)       /* 1b */
#define MFG_CORE1_PWR_CLK_DIS_LSB           (1U << 4)       /* 1b */
#define MFG_CORE1_SRAM_PDN_LSB              (1U << 8)       /* 4b */
#define MFG_CORE1_SRAM_PDN_ACK_LSB          (1U << 12)      /* 4b */
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
/* VPU_TOP_PWR_CON (0x10006000+0x324) */
#define VPU_TOP_PWR_RST_B_LSB               (1U << 0)       /* 1b */
#define VPU_TOP_PWR_ISO_LSB                 (1U << 1)       /* 1b */
#define VPU_TOP_PWR_ON_LSB                  (1U << 2)       /* 1b */
#define VPU_TOP_PWR_ON_2ND_LSB              (1U << 3)       /* 1b */
#define VPU_TOP_PWR_CLK_DIS_LSB             (1U << 4)       /* 1b */
#define VPU_TOP_SRAM_CKISO_LSB              (1U << 5)       /* 1b */
#define VPU_TOP_SRAM_ISOINT_B_LSB           (1U << 6)       /* 1b */
#define VPU_TOP_SRAM_PDN_LSB                (1U << 8)       /* 4b */
#define VPU_TOP_SRAM_PDN_ACK_LSB            (1U << 12)      /* 4b */
#define VPU_TOP_SRAM_SLPB_LSB               (1U << 16)      /* 4b */
#define VPU_TOP_SRAM_SLPB_ACK_LSB           (1U << 28)      /* 4b */
/* CONN_PWR_CON (0x10006000+0x32C) */
#define CONN_PWR_RST_B_LSB                  (1U << 0)       /* 1b */
#define CONN_PWR_ISO_LSB                    (1U << 1)       /* 1b */
#define CONN_PWR_ON_LSB                     (1U << 2)       /* 1b */
#define CONN_PWR_ON_2ND_LSB                 (1U << 3)       /* 1b */
#define CONN_PWR_CLK_DIS_LSB                (1U << 4)       /* 1b */
#define CONN_SRAM_PDN_LSB                   (1U << 8)       /* 1b */
#define CONN_SRAM_PDN_ACK_LSB               (1U << 12)      /* 1b */
/* VPU_CORE2_PWR_CON (0x10006000+0x330) */
#define VPU_CORE2_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define VPU_CORE2_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define VPU_CORE2_PWR_ON_LSB                (1U << 2)       /* 1b */
#define VPU_CORE2_PWR_ON_2ND_LSB            (1U << 3)       /* 1b */
#define VPU_CORE2_PWR_CLK_DIS_LSB           (1U << 4)       /* 1b */
#define VPU_CORE2_SRAM_CKISO_LSB            (1U << 5)       /* 1b */
#define VPU_CORE2_SRAM_ISOINT_B_LSB         (1U << 6)       /* 1b */
#define VPU_CORE2_SRAM_PDN_LSB              (1U << 8)       /* 4b */
#define VPU_CORE2_SRAM_PDN_ACK_LSB          (1U << 12)      /* 4b */
#define VPU_CORE2_SRAM_SLPB_LSB             (1U << 16)      /* 4b */
#define VPU_CORE2_SRAM_SLPB_ACK_LSB         (1U << 28)      /* 4b */
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
#define MFG_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define MFG_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */
/* VPU_CORE0_PWR_CON (0x10006000+0x33C) */
#define VPU_CORE0_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define VPU_CORE0_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define VPU_CORE0_PWR_ON_LSB                (1U << 2)       /* 1b */
#define VPU_CORE0_ON_2ND_LSB                (1U << 3)       /* 1b */
#define VPU_CORE0_CLK_DIS_LSB               (1U << 4)       /* 1b */
#define VPU_CORE0_SRAM_CKISO_LSB            (1U << 5)       /* 1b */
#define VPU_CORE0_SRAM_ISOINT_B_LSB         (1U << 6)       /* 1b */
#define VPU_CORE0_SRAM_PDN_LSB              (1U << 8)       /* 4b */
#define VPU_CORE0_SRAM_PDN_ACK_LSB          (1U << 12)      /* 4b */
#define VPU_CORE0_SRAM_SLPB_LSB             (1U << 16)      /* 4b */
#define VPU_CORE0_SRAM_SLPB_ACK_LSB         (1U << 28)      /* 4b */
/* VPU_CORE1_PWR_CON (0x10006000+0x340) */
#define VPU_CORE1_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define VPU_CORE1_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define VPU_CORE1_PWR_ON_LSB                (1U << 2)       /* 1b */
#define VPU_CORE1_ON_2ND_LSB                (1U << 3)       /* 1b */
#define VPU_CORE1_CLK_DIS_LSB               (1U << 4)       /* 1b */
#define VPU_CORE1_SRAM_CKISO_LSB            (1U << 5)       /* 1b */
#define VPU_CORE1_SRAM_ISOINT_B_LSB         (1U << 6)       /* 1b */
#define VPU_CORE1_SRAM_PDN_LSB              (1U << 8)       /* 4b */
#define VPU_CORE1_SRAM_PDN_ACK_LSB          (1U << 12)      /* 4b */
#define VPU_CORE1_SRAM_SLPB_LSB             (1U << 16)      /* 4b */
#define VPU_CORE1_SRAM_SLPB_ACK_LSB         (1U << 28)      /* 4b */
/* CAM_PWR_CON (0x10006000+0x344) */
#define CAM_PWR_RST_B_LSB                   (1U << 0)       /* 1b */
#define CAM_PWR_ISO_LSB                     (1U << 1)       /* 1b */
#define CAM_PWR_ON_LSB                      (1U << 2)       /* 1b */
#define CAM_PWR_ON_2ND_LSB                  (1U << 3)       /* 1b */
#define CAM_PWR_CLK_DIS_LSB                 (1U << 4)       /* 1b */
#define CAM_SRAM_PDN_LSB                    (1U << 8)       /* 4b */
#define CAM_SRAM_PDN_ACK_LSB                (1U << 12)      /* 4b */
/* MFG_2D_PWR_CON (0x10006000+0x348) */
#define MFG_2D_PWR_RST_B_LSB                (1U << 0)       /* 1b */
#define MFG_2D_PWR_ISO_LSB                  (1U << 1)       /* 1b */
#define MFG_2D_PWR_ON_LSB                   (1U << 2)       /* 1b */
#define MFG_2D_PWR_ON_2ND_LSB               (1U << 3)       /* 1b */
#define MFG_2D_PWR_CLK_DIS_LSB              (1U << 4)       /* 1b */
#define MFG_2D_SRAM_PDN_LSB                 (1U << 8)       /* 4b */
#define MFG_2D_SRAM_PDN_ACK_LSB             (1U << 12)      /* 4b */
/* MFG_CORE0_PWR_CON (0x10006000+0x34C) */
#define MFG_CORE0_PWR_RST_B_LSB             (1U << 0)       /* 1b */
#define MFG_CORE0_PWR_ISO_LSB               (1U << 1)       /* 1b */
#define MFG_CORE0_PWR_ON_LSB                (1U << 2)       /* 1b */
#define MFG_CORE0_PWR_ON_2ND_LSB            (1U << 3)       /* 1b */
#define MFG_CORE0_PWR_CLK_DIS_LSB           (1U << 4)       /* 1b */
#define MFG_CORE0_SRAM_PDN_LSB              (1U << 8)       /* 4b */
#define MFG_CORE0_SRAM_PDN_ACK_LSB          (1U << 12)      /* 4b */
/* SYSRAM_CON (0x10006000+0x350) */
#define IFR_SRAMROM_SRAM_CKISO_LSB          (1U << 0)       /* 1b */
#define IFR_SRAMROM_SRAM_ISOINT_B_LSB       (1U << 1)       /* 1b */
#define IFR_SRAMROM_SRAM_SLEEP_B_LSB        (1U << 4)       /* 8b */
#define IFR_SRAMROM_SRAM_PDN_LSB            (1U << 16)      /* 8b */
/* SYSROM_CON (0x10006000+0x354) */
#define IFR_SRAMROM_ROM_PDN_LSB             (1U << 0)       /* 6b */
/* SSPM_SRAM_CON (0x10006000+0x358) */
#define SSPM_SRAM_CKISO_LSB                 (1U << 0)       /* 1b */
#define SSPM_SRAM_ISOINT_B_LSB              (1U << 1)       /* 1b */
#define SSPM_SRAM_SLEEP_B_LSB               (1U << 4)       /* 1b */
#define SSPM_SRAM_PDN_LSB                   (1U << 16)      /* 1b */
/* SCP_SRAM_CON (0x10006000+0x35C) */
#define SCP_SRAM_CKISO_LSB                  (1U << 0)       /* 1b */
#define SCP_SRAM_ISOINT_B_LSB               (1U << 1)       /* 1b */
#define SCP_SRAM_SLEEP_B_LSB                (1U << 4)       /* 1b */
#define SCP_SRAM_PDN_LSB                    (1U << 16)      /* 1b */
/* UFS_SRAM_CON (0x10006000+0x36C) */
#define UFS_SRAM_CKISO_LSB                  (1U << 0)       /* 1b */
#define UFS_SRAM_ISOINT_B_LSB               (1U << 1)       /* 1b */
#define UFS_SRAM_SLEEP_B_LSB                (1U << 4)       /* 5b */
#define UFS_SRAM_PDN_LSB                    (1U << 16)      /* 5b */
/* DUMMY_SRAM_CON (0x10006000+0x380) */
#define DUMMY_SRAM_CKISO_LSB                (1U << 0)       /* 1b */
#define DUMMY_SRAM_ISOINT_B_LSB             (1U << 1)       /* 1b */
#define DUMMY_SRAM_SLEEP_B_LSB              (1U << 4)       /* 8b */
#define DUMMY_SRAM_PDN_LSB                  (1U << 16)      /* 8b */
/* MD_EXT_BUCK_ISO_CON (0x10006000+0x390) */
#define VMODEM_BUCK_ELS_EN_LSB              (1U << 0)       /* 1b */
#define VMD_BUCK_ELS_EN_LSB                 (1U << 1)       /* 1b */
/* MD_SRAM_ISO_CON (0x10006000+0x394) */
#define MD1_SRAM_ISOINT_B_LSB               (1U << 0)       /* 1b */
/* MD_EXTRA_PWR_CON (0x10006000+0x398) */
#define MD1_PWR_PROT_REQ_STA_LSB            (1U << 0)       /* 1b */
#define MD2_PWR_PROT_REQ_STA_LSB            (1U << 1)       /* 1b */
/* EXT_BUCK_CON (0x10006000+0x3A0) */
#define RG_VA09_ON_LSB                      (1U << 0)       /* 1b */
/* MBIST_EFUSE_REPAIR_ACK_STA (0x10006000+0x3D0) */
#define MBIST_EFUSE_REPAIR_ACK_STA_LSB      (1U << 0)       /* 32b */
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
/* AP_MDSRC_REQ (0x10006000+0x430) */
#define AP_MDSMSRC_REQ_LSB                  (1U << 0)       /* 1b */
#define AP_L1SMSRC_REQ_LSB                  (1U << 1)       /* 1b */
#define AP_MD2SRC_REQ_LSB                   (1U << 2)       /* 1b */
#define AP_MDSMSRC_ACK_LSB                  (1U << 4)       /* 1b */
#define AP_L1SMSRC_ACK_LSB                  (1U << 5)       /* 1b */
#define AP_MD2SRC_ACK_LSB                   (1U << 6)       /* 1b */
/* SPM2MD_DVFS_CON (0x10006000+0x438) */
#define SPM2MD_DVFS_CON_LSB                 (1U << 0)       /* 32b */
/* MD2SPM_DVFS_CON (0x10006000+0x43C) */
#define MD2SPM_DVFS_CON_LSB                 (1U << 0)       /* 32b */
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
#define CPU_DVFS_REQ_LSB                    (1U << 0)       /* 32b */
/* SPM_PLL_CON (0x10006000+0x44C) */
#define SC_MAINPLLOUT_OFF_LSB               (1U << 0)       /* 1b */
#define SC_UNIPLLOUT_OFF_LSB                (1U << 1)       /* 1b */
#define SC_MAINPLL_OFF_LSB                  (1U << 4)       /* 1b */
#define SC_UNIPLL_OFF_LSB                   (1U << 5)       /* 1b */
#define SC_MAINPLL_S_OFF_LSB                (1U << 8)       /* 1b */
#define SC_UNIPLL_S_OFF_LSB                 (1U << 9)       /* 1b */
#define SC_SMI_CK_OFF_LSB                   (1U << 16)      /* 1b */
#define SC_SSPMK_CK_OFF_LSB                 (1U << 17)      /* 1b */
/* SPM_EMI_BW_MODE (0x10006000+0x450) */
#define EMI_BW_MODE_LSB                     (1U << 0)       /* 1b */
#define EMI_BOOST_MODE_LSB                  (1U << 1)       /* 1b */
#define EMI_BW_MODE_2_LSB                   (1U << 2)       /* 1b */
#define EMI_BOOST_MODE_2_LSB                (1U << 3)       /* 1b */
/* AP2MD_PEER_WAKEUP (0x10006000+0x454) */
#define AP2MD_PEER_WAKEUP_LSB               (1U << 0)       /* 1b */
/* ULPOSC_CON (0x10006000+0x458) */
#define ULPOSC_EN_LSB                       (1U << 0)       /* 1b */
#define ULPOSC_RST_LSB                      (1U << 1)       /* 1b */
#define ULPOSC_CG_EN_LSB                    (1U << 2)       /* 1b */
#define ULPOSC_CLK_SEL_LSB                  (1U << 3)       /* 1b */
/* SPM2MM_CON (0x10006000+0x45C) */
#define SPM2MM_FORCE_ULTRA_LSB              (1U << 0)       /* 1b */
#define SPM2MM_DBL_OSTD_ACT_LSB             (1U << 1)       /* 1b */
#define SPM2MM_ULTRAREQ_LSB                 (1U << 2)       /* 1b */
#define SPM2MD_ULTRAREQ_LSB                 (1U << 3)       /* 1b */
#define SPM2ISP_ULTRAREQ_LSB                (1U << 4)       /* 1b */
#define MM2SPM_FORCE_ULTRA_ACK_LSB          (1U << 16)      /* 1b */
#define MM2SPM_DBL_OSTD_ACT_ACK_LSB         (1U << 17)      /* 1b */
#define SPM2ISP_ULTRAACK_D2T_LSB            (1U << 18)      /* 1b */
#define SPM2MM_ULTRAACK_D2T_LSB             (1U << 19)      /* 1b */
#define SPM2MD_ULTRAACK_D2T_LSB             (1U << 20)      /* 1b */
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
/* SPM_S1_MODE_CH (0x10006000+0x468) */
#define SPM_S1_MODE_CH_LSB                  (1U << 0)       /* 2b */
#define S1_EMI_CK_SWITCH_LSB                (1U << 8)       /* 2b */
/* EMI_SELF_REFRESH_CH_STA (0x10006000+0x46C) */
#define EMI_SELF_REFRESH_CH_LSB             (1U << 0)       /* 2b */
/* DRAMC_DPY_CLK_SW_CON_SEL2 (0x10006000+0x470) */
#define SW_PHYPLL_SHU_EN_SEL_LSB            (1U << 0)       /* 1b */
#define SW_PHYPLL2_SHU_EN_SEL_LSB           (1U << 1)       /* 1b */
#define SW_PHYPLL_MODE_SW_SEL_LSB           (1U << 2)       /* 1b */
#define SW_PHYPLL2_MODE_SW_SEL_LSB          (1U << 3)       /* 1b */
#define SW_DR_SHORT_QUEUE_SEL_LSB           (1U << 4)       /* 1b */
#define SW_DR_SHU_EN_SEL_LSB                (1U << 5)       /* 1b */
#define SW_DR_SHU_LEVEL_SEL_LSB             (1U << 6)       /* 1b */
#define SW_DPY_BCLK_ENABLE_SEL_LSB          (1U << 8)       /* 2b */
#define SW_SHU_RESTORE_SEL_LSB              (1U << 10)      /* 2b */
#define SW_DPHY_PRECAL_UP_SEL_LSB           (1U << 12)      /* 2b */
#define SW_DPHY_RXDLY_TRACK_EN_SEL_LSB      (1U << 14)      /* 2b */
#define SW_TX_TRACKING_DIS_SEL_LSB          (1U << 16)      /* 2b */
/* DRAMC_DPY_CLK_SW_CON2 (0x10006000+0x474) */
#define SW_PHYPLL_SHU_EN_LSB                (1U << 0)       /* 1b */
#define SW_PHYPLL2_SHU_EN_LSB               (1U << 1)       /* 1b */
#define SW_PHYPLL_MODE_SW_LSB               (1U << 2)       /* 1b */
#define SW_PHYPLL2_MODE_SW_LSB              (1U << 3)       /* 1b */
#define SW_DR_SHORT_QUEUE_LSB               (1U << 4)       /* 1b */
#define SW_DR_SHU_EN_LSB                    (1U << 5)       /* 1b */
#define SW_DR_SHU_LEVEL_LSB                 (1U << 6)       /* 2b */
#define SW_DPY_BCLK_ENABLE_LSB              (1U << 8)       /* 2b */
#define SW_SHU_RESTORE_LSB                  (1U << 10)      /* 2b */
#define SW_DPHY_PRECAL_UP_LSB               (1U << 12)      /* 2b */
#define SW_DPHY_RXDLY_TRACK_EN_LSB          (1U << 14)      /* 2b */
#define SW_TX_TRACKING_DIS_LSB              (1U << 16)      /* 2b */
/* DRAMC_DMYRD_CON (0x10006000+0x478) */
#define DRAMC_DMYRD_EN_CH0_LSB              (1U << 0)       /* 1b */
#define DRAMC_DMYRD_INTV_SEL_CH0_LSB        (1U << 1)       /* 1b */
#define DRAMC_DMYRD_EN_MOD_SEL_CH0_LSB      (1U << 2)       /* 1b */
#define DRAMC_DMYRD_EN_CH1_LSB              (1U << 8)       /* 1b */
#define DRAMC_DMYRD_INTV_SEL_CH1_LSB        (1U << 9)       /* 1b */
#define DRAMC_DMYRD_EN_MOD_SEL_CH1_LSB      (1U << 10)      /* 1b */
/* SPM_DRS_CON (0x10006000+0x47C) */
#define SPM_DRS_DIS_REQ_CH0_LSB             (1U << 0)       /* 1b */
#define SPM_DRS_DIS_REQ_CH1_LSB             (1U << 1)       /* 1b */
#define SPM_DRS_DIS_ACK_CH0_LSB             (1U << 8)       /* 1b */
#define SPM_DRS_DIS_ACK_CH1_LSB             (1U << 9)       /* 1b */
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
/* SPM_MAS_PAUSE_MM_MASK_B (0x10006000+0x4A0) */
#define SPM_MAS_PAUSE_MM_MASK_B_LSB         (1U << 0)       /* 16b */
/* SPM_MAS_PAUSE_MCU_MASK_B (0x10006000+0x4A4) */
#define SPM_MAS_PAUSE_MCU_MASK_B_LSB        (1U << 0)       /* 16b */
/* SRAM_DREQ_ACK (0x10006000+0x4AC) */
#define SRAM_DREQ_ACK_LSB                   (1U << 0)       /* 16b */
/* SRAM_DREQ_CON (0x10006000+0x4B0) */
#define SRAM_DREQ_CON_LSB                   (1U << 0)       /* 16b */
/* SRAM_DREQ_CON_SET (0x10006000+0x4B4) */
#define SRAM_DREQ_CON_SET_LSB               (1U << 0)       /* 16b */
/* SRAM_DREQ_CON_CLR (0x10006000+0x4B8) */
#define SRAM_DREQ_CON_CLR_LSB               (1U << 0)       /* 16b */
/* SPM2EMI_ENTER_ULPM (0x10006000+0x4BC) */
#define SPM2EMI_ENTER_ULPM_LSB              (1U << 0)       /* 1b */
/* SPM_SSPM_IRQ (0x10006000+0x4C0) */
#define SPM_SSPM_IRQ_LSB                    (1U << 0)       /* 1b */
#define SPM_SSPM_IRQ_SEL_LSB                (1U << 4)       /* 1b */
/* SPM2PMCU_INT (0x10006000+0x4C4) */
#define SPM2PMCU_INT_LSB                    (1U << 0)       /* 4b */
/* SPM2PMCU_INT_SET (0x10006000+0x4C8) */
#define SPM2PMCU_INT_SET_LSB                (1U << 0)       /* 4b */
/* SPM2PMCU_INT_CLR (0x10006000+0x4CC) */
#define SPM2PMCU_INT_CLR_LSB                (1U << 0)       /* 4b */
/* SPM2PMCU_MAILBOX_0 (0x10006000+0x4D0) */
#define SPM2PMCU_MAILBOX_0_LSB              (1U << 0)       /* 32b */
/* SPM2PMCU_MAILBOX_1 (0x10006000+0x4D4) */
#define SPM2PMCU_MAILBOX_1_LSB              (1U << 0)       /* 32b */
/* SPM2PMCU_MAILBOX_2 (0x10006000+0x4D8) */
#define SPM2PMCU_MAILBOX_2_LSB              (1U << 0)       /* 32b */
/* SPM2PMCU_MAILBOX_3 (0x10006000+0x4DC) */
#define SPM2PMCU_MAILBOX_3_LSB              (1U << 0)       /* 32b */
/* PMCU2SPM_INT (0x10006000+0x4E0) */
#define PMCU2SPM_INT_LSB                    (1U << 0)       /* 4b */
/* PMCU2SPM_INT_SET (0x10006000+0x4E4) */
#define PMCU2SPM_INT_SET_LSB                (1U << 0)       /* 4b */
/* PMCU2SPM_INT_CLR (0x10006000+0x4E8) */
#define PMCU2SPM_INT_CLR_LSB                (1U << 0)       /* 4b */
/* PMCU2SPM_MAILBOX_0 (0x10006000+0x4EC) */
#define PMCU2SPM_MAILBOX_0_LSB              (1U << 0)       /* 32b */
/* PMCU2SPM_MAILBOX_1 (0x10006000+0x4F0) */
#define PMCU2SPM_MAILBOX_1_LSB              (1U << 0)       /* 32b */
/* PMCU2SPM_MAILBOX_2 (0x10006000+0x4F4) */
#define PMCU2SPM_MAILBOX_2_LSB              (1U << 0)       /* 32b */
/* PMCU2SPM_MAILBOX_3 (0x10006000+0x4F8) */
#define PMCU2SPM_MAILBOX_3_LSB              (1U << 0)       /* 32b */
/* PMCU2SPM_CFG (0x10006000+0x4FC) */
#define PMCU2SPM_INT_MASK_B_LSB             (1U << 0)       /* 4b */
#define SPM_PMCU_MAILBOX_REQ_LSB            (1U << 8)       /* 1b */
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
/* MP0_L2CFLUSH (0x10006000+0x554) */
#define MP0_L2CFLUSH_REQ_LSB                (1U << 0)       /* 1b */
#define MP0_L2CFLUSH_DONE_LSB               (1U << 4)       /* 1b */
/* MP1_L2CFLUSH (0x10006000+0x558) */
#define MP1_L2CFLUSH_REQ_LSB                (1U << 0)       /* 1b */
#define MP1_L2CFLUSH_DONE_LSB               (1U << 4)       /* 1b */
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
/* SPM2SW_MAILBOX_0 (0x10006000+0x5D0) */
#define SPM2SW_MAILBOX_0_LSB                (1U << 0)       /* 32b */
/* SPM2SW_MAILBOX_1 (0x10006000+0x5D4) */
#define SPM2SW_MAILBOX_1_LSB                (1U << 0)       /* 32b */
/* SPM2SW_MAILBOX_2 (0x10006000+0x5D8) */
#define SPM2SW_MAILBOX_2_LSB                (1U << 0)       /* 32b */
/* SPM2SW_MAILBOX_3 (0x10006000+0x5DC) */
#define SPM2SW_MAILBOX_3_LSB                (1U << 0)       /* 32b */
/* SW2SPM_INT (0x10006000+0x5E0) */
#define SW2SPM_INT_LSB                      (1U << 0)       /* 4b */
/* SW2SPM_INT_SET (0x10006000+0x5E4) */
#define SW2SPM_INT_SET_LSB                  (1U << 0)       /* 4b */
/* SW2SPM_INT_CLR (0x10006000+0x5E8) */
#define SW2SPM_INT_CLR_LSB                  (1U << 0)       /* 4b */
/* SW2SPM_MAILBOX_0 (0x10006000+0x5EC) */
#define SW2SPM_MAILBOX_0_LSB                (1U << 0)       /* 32b */
/* SW2SPM_MAILBOX_1 (0x10006000+0x5F0) */
#define SW2SPM_MAILBOX_1_LSB                (1U << 0)       /* 32b */
/* SW2SPM_MAILBOX_2 (0x10006000+0x5F4) */
#define SW2SPM_MAILBOX_2_LSB                (1U << 0)       /* 32b */
/* SW2SPM_MAILBOX_3 (0x10006000+0x5F8) */
#define SW2SPM_MAILBOX_3_LSB                (1U << 0)       /* 32b */
/* SW2SPM_CFG (0x10006000+0x5FC) */
#define SWU2SPM_INT_MASK_B_LSB              (1U << 0)       /* 4b */
#define SPM_SW_MAILBOX_REQ_LSB              (1U << 8)       /* 1b */
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
/* SPM_RSV_CON1 (0x10006000+0x628) */
#define SPM_RSV_CON1_LSB                    (1U << 0)       /* 16b */
/* SPM_RSV_STA1 (0x10006000+0x62C) */
#define SPM_RSV_STA1_LSB                    (1U << 0)       /* 16b */
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
/* SPM_SW_RSV_18 (0x10006000+0x67C) */
#define SPM_SW_RSV_18_LSB                   (1U << 0)       /* 32b */
/* SPM_SW_RSV_19 (0x10006000+0x680) */
#define SPM_SW_RSV_19_LSB                   (1U << 0)       /* 32b */
/* DVFSRC_EVENT_MASK_CON (0x10006000+0x690) */
#define DVFSRC_EVENT_MASK_B_LSB             (1U << 0)       /* 16b */
#define DVFSRC_EVENT_TRIGGER_MASK_B_LSB     (1U << 16)      /* 1b */
/* DVFSRC_EVENT_FORCE_ON (0x10006000+0x694) */
#define DVFSRC_EVENT_FORCE_ON_LSB           (1U << 0)       /* 16b */
#define DVFSRC_EVENT_TRIGGER_FORCE_ON_LSB   (1U << 16)      /* 1b */
/* DVFSRC_EVENT_SEL (0x10006000+0x698) */
#define DVFSRC_EVENT_SEL_LSB                (1U << 0)       /* 16b */
/* SPM_DVFS_EVENT_STA (0x10006000+0x69C) */
#define SPM_DVFS_EVENT_STA_LSB              (1U << 0)       /* 32b */
/* SPM_DVFS_EVENT_STA1 (0x10006000+0x6A0) */
#define SPM_DVFS_EVENT_STA1_LSB             (1U << 0)       /* 32b */
/* SPM_DVFS_LEVEL (0x10006000+0x6A4) */
#define SPM_DVFS_LEVEL_LSB                  (1U << 0)       /* 16b */
/* DVFS_ABORT_STA (0x10006000+0x6A8) */
#define RC2SPM_EVENT_ABORT_D2T_LSB          (1U << 0)       /* 16b */
#define RC2SPM_EVENT_ABORT_MASK_OR_LSB      (1U << 16)      /* 1b */
/* DVFS_ABORT_OTHERS_MASK (0x10006000+0x6AC) */
#define DVFS_ABORT_OTHERS_MASK_B_LSB        (1U << 0)       /* 16b */
/* SPM_DFS_LEVEL (0x10006000+0x6B0) */
#define SPM_DFS_LEVEL_LSB                   (1U << 0)       /* 4b */
/* SPM_DVS_LEVEL (0x10006000+0x6B4) */
#define SPM_VCORE_LEVEL_LSB                 (1U << 0)       /* 8b */
#define SPM_VSRAM_LEVEL_LSB                 (1U << 8)       /* 8b */
#define SPM_VMODEM_LEVEL_LSB                (1U << 16)      /* 8b */
/* SPM_DVFS_MISC (0x10006000+0x6B8) */
#define MSDC_DVFS_REQUEST_LSB               (1U << 0)       /* 1b */
#define MSDC_DVFS_LEVEL_LSB                 (1U << 1)       /* 4b */
#define SDIO_READY_TO_SPM_LSB               (1U << 7)       /* 1b */
#define MD2AP_CENTRAL_BUCK_GEAR_REQ_D2T_LSB (1U << 8)       /* 1b */
#define MD2AP_CENTRAL_BUCK_GEAR_RDY_D2T_LSB (1U << 9)       /* 1b */
/* SPARE_SRC_REQ_MASK (0x10006000+0x6C0) */
#define SPARE1_DDREN_MASK_B_LSB             (1U << 0)       /* 1b */
#define SPARE1_APSRC_REQ_MASK_B_LSB         (1U << 1)       /* 1b */
#define SPARE1_VRF18_REQ_MASK_B_LSB         (1U << 2)       /* 1b */
#define SPARE1_INFRA_REQ_MASK_B_LSB         (1U << 3)       /* 1b */
#define SPARE1_SRCCLKENA_MASK_B_LSB         (1U << 4)       /* 1b */
#define SPARE1_DDREN_2_MASK_B_LSB           (1U << 5)       /* 1b */
#define SPARE2_DDREN_MASK_B_LSB             (1U << 8)       /* 1b */
#define SPARE2_APSRC_REQ_MASK_B_LSB         (1U << 9)       /* 1b */
#define SPARE2_VRF18_REQ_MASK_B_LSB         (1U << 10)      /* 1b */
#define SPARE2_INFRA_REQ_MASK_B_LSB         (1U << 11)      /* 1b */
#define SPARE2_SRCCLKENA_MASK_B_LSB         (1U << 12)      /* 1b */
#define SPARE2_DDREN_2_MASK_B_LSB           (1U << 13)      /* 1b */
/* SCP_VCORE_LEVEL (0x10006000+0x6C4) */
#define SCP_VCORE_LEVEL_LSB                 (1U << 0)       /* 8b */
/* SC_MM_CK_SEL_CON (0x10006000+0x6C8) */
#define SC_MM_CK_SEL_LSB                    (1U << 0)       /* 4b */
#define SC_MM_CK_SEL_EN_LSB                 (1U << 4)       /* 1b */
/* SPARE_ACK_STA (0x10006000+0x6F0) */
#define SPARE_ACK_SYNC_LSB                  (1U << 0)       /* 32b */
/* SPARE_ACK_MASK (0x10006000+0x6F4) */
#define SPARE_ACK_MASK_B_LSB                (1U << 0)       /* 32b */
/* SPM_DVFS_CON1 (0x10006000+0x700) */
#define SPM_DVFS_CON1_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CON1_STA (0x10006000+0x704) */
#define SPM_DVFS_CON1_STA_LSB               (1U << 0)       /* 32b */
/* SPM_DVFS_CMD0 (0x10006000+0x710) */
#define SPM_DVFS_CMD0_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD1 (0x10006000+0x714) */
#define SPM_DVFS_CMD1_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD2 (0x10006000+0x718) */
#define SPM_DVFS_CMD2_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD3 (0x10006000+0x71C) */
#define SPM_DVFS_CMD3_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD4 (0x10006000+0x720) */
#define SPM_DVFS_CMD4_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD5 (0x10006000+0x724) */
#define SPM_DVFS_CMD5_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD6 (0x10006000+0x728) */
#define SPM_DVFS_CMD6_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD7 (0x10006000+0x72C) */
#define SPM_DVFS_CMD7_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD8 (0x10006000+0x730) */
#define SPM_DVFS_CMD8_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD9 (0x10006000+0x734) */
#define SPM_DVFS_CMD9_LSB                   (1U << 0)       /* 32b */
/* SPM_DVFS_CMD10 (0x10006000+0x738) */
#define SPM_DVFS_CMD10_LSB                  (1U << 0)       /* 32b */
/* SPM_DVFS_CMD11 (0x10006000+0x73C) */
#define SPM_DVFS_CMD11_LSB                  (1U << 0)       /* 32b */
/* SPM_DVFS_CMD12 (0x10006000+0x740) */
#define SPM_DVFS_CMD12_LSB                  (1U << 0)       /* 32b */
/* SPM_DVFS_CMD13 (0x10006000+0x744) */
#define SPM_DVFS_CMD13_LSB                  (1U << 0)       /* 32b */
/* SPM_DVFS_CMD14 (0x10006000+0x748) */
#define SPM_DVFS_CMD14_LSB                  (1U << 0)       /* 32b */
/* SPM_DVFS_CMD15 (0x10006000+0x74C) */
#define SPM_DVFS_CMD15_LSB                  (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE0_FIX (0x10006000+0x780) */
#define WDT_LATCH_SPARE0_FIX_LSB            (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE1_FIX (0x10006000+0x784) */
#define WDT_LATCH_SPARE1_FIX_LSB            (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE2_FIX (0x10006000+0x788) */
#define WDT_LATCH_SPARE2_FIX_LSB            (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE3_FIX (0x10006000+0x78C) */
#define WDT_LATCH_SPARE3_FIX_LSB            (1U << 0)       /* 32b */
/* SPARE_ACK_IN_FIX (0x10006000+0x790) */
#define SPARE_ACK_IN_FIX_LSB                (1U << 0)       /* 32b */
/* DCHA_LATCH_RSV0_FIX (0x10006000+0x794) */
#define DCHA_LATCH_RSV0_FIX_LSB             (1U << 0)       /* 32b */
/* DCHB_LATCH_RSV0_FIX (0x10006000+0x798) */
#define DCHB_LATCH_RSV0_FIX_LSB             (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_0 (0x10006000+0x800) */
#define PCM_WDT_LATCH_0_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_1 (0x10006000+0x804) */
#define PCM_WDT_LATCH_1_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_2 (0x10006000+0x808) */
#define PCM_WDT_LATCH_2_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_3 (0x10006000+0x80C) */
#define PCM_WDT_LATCH_3_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_4 (0x10006000+0x810) */
#define PCM_WDT_LATCH_4_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_5 (0x10006000+0x814) */
#define PCM_WDT_LATCH_5_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_6 (0x10006000+0x818) */
#define PCM_WDT_LATCH_6_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_7 (0x10006000+0x81C) */
#define PCM_WDT_LATCH_7_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_8 (0x10006000+0x820) */
#define PCM_WDT_LATCH_8_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_9 (0x10006000+0x824) */
#define PCM_WDT_LATCH_9_LSB                 (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE0 (0x10006000+0x828) */
#define WDT_LATCH_SPARE0_LSB                (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE1 (0x10006000+0x82C) */
#define WDT_LATCH_SPARE1_LSB                (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE2 (0x10006000+0x830) */
#define WDT_LATCH_SPARE2_LSB                (1U << 0)       /* 32b */
/* WDT_LATCH_SPARE3 (0x10006000+0x834) */
#define WDT_LATCH_SPARE3_LSB                (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_10 (0x10006000+0x838) */
#define PCM_WDT_LATCH_10_LSB                (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_11 (0x10006000+0x83C) */
#define PCM_WDT_LATCH_11_LSB                (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_0 (0x10006000+0x840) */
#define DCHA_GATING_LATCH_0_LSB             (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_1 (0x10006000+0x844) */
#define DCHA_GATING_LATCH_1_LSB             (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_2 (0x10006000+0x848) */
#define DCHA_GATING_LATCH_2_LSB             (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_3 (0x10006000+0x84C) */
#define DCHA_GATING_LATCH_3_LSB             (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_4 (0x10006000+0x850) */
#define DCHA_GATING_LATCH_4_LSB             (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_5 (0x10006000+0x854) */
#define DCHA_GATING_LATCH_5_LSB             (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_6 (0x10006000+0x858) */
#define DCHA_GATING_LATCH_6_LSB             (1U << 0)       /* 32b */
/* DCHA_GATING_LATCH_7 (0x10006000+0x85C) */
#define DCHA_GATING_LATCH_7_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_0 (0x10006000+0x860) */
#define DCHB_GATING_LATCH_0_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_1 (0x10006000+0x864) */
#define DCHB_GATING_LATCH_1_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_2 (0x10006000+0x868) */
#define DCHB_GATING_LATCH_2_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_3 (0x10006000+0x86C) */
#define DCHB_GATING_LATCH_3_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_4 (0x10006000+0x870) */
#define DCHB_GATING_LATCH_4_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_5 (0x10006000+0x874) */
#define DCHB_GATING_LATCH_5_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_6 (0x10006000+0x878) */
#define DCHB_GATING_LATCH_6_LSB             (1U << 0)       /* 32b */
/* DCHB_GATING_LATCH_7 (0x10006000+0x87C) */
#define DCHB_GATING_LATCH_7_LSB             (1U << 0)       /* 32b */
/* DCHA_LATCH_RSV0 (0x10006000+0x880) */
#define DCHA_LATCH_RSV0_LSB                 (1U << 0)       /* 32b */
/* DCHB_LATCH_RSV0 (0x10006000+0x884) */
#define DCHB_LATCH_RSV0_LSB                 (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_12 (0x10006000+0x888) */
#define PCM_WDT_LATCH_12_LSB                (1U << 0)       /* 32b */
/* PCM_WDT_LATCH_13 (0x10006000+0x88C) */
#define PCM_WDT_LATCH_13_LSB                (1U << 0)       /* 32b */
/* SPM_PC_TRACE_CON (0x10006000+0x8C0) */
#define SPM_PC_TRACE_OFFSET_LSB             (1U << 0)       /* 12b */
#define SPM_PC_TRACE_HW_EN_LSB              (1U << 16)      /* 1b */
#define SPM_PC_TRACE_SW_LSB                 (1U << 17)      /* 1b */
/* SPM_PC_TRACE_G0 (0x10006000+0x8C4) */
#define SPM_PC_TRACE0_LSB                   (1U << 0)       /* 12b */
#define SPM_PC_TRACE1_LSB                   (1U << 16)      /* 12b */
/* SPM_PC_TRACE_G1 (0x10006000+0x8C8) */
#define SPM_PC_TRACE2_LSB                   (1U << 0)       /* 12b */
#define SPM_PC_TRACE3_LSB                   (1U << 16)      /* 12b */
/* SPM_PC_TRACE_G2 (0x10006000+0x8CC) */
#define SPM_PC_TRACE4_LSB                   (1U << 0)       /* 12b */
#define SPM_PC_TRACE5_LSB                   (1U << 16)      /* 12b */
/* SPM_PC_TRACE_G3 (0x10006000+0x8D0) */
#define SPM_PC_TRACE6_LSB                   (1U << 0)       /* 12b */
#define SPM_PC_TRACE7_LSB                   (1U << 16)      /* 12b */
/* SPM_PC_TRACE_G4 (0x10006000+0x8D4) */
#define SPM_PC_TRACE8_LSB                   (1U << 0)       /* 12b */
#define SPM_PC_TRACE9_LSB                   (1U << 16)      /* 12b */
/* SPM_PC_TRACE_G5 (0x10006000+0x8D8) */
#define SPM_PC_TRACE10_LSB                  (1U << 0)       /* 12b */
#define SPM_PC_TRACE11_LSB                  (1U << 16)      /* 12b */
/* SPM_PC_TRACE_G6 (0x10006000+0x8DC) */
#define SPM_PC_TRACE12_LSB                  (1U << 0)       /* 12b */
#define SPM_PC_TRACE13_LSB                  (1U << 16)      /* 12b */
/* SPM_PC_TRACE_G7 (0x10006000+0x8E0) */
#define SPM_PC_TRACE14_LSB                  (1U << 0)       /* 12b */
#define SPM_PC_TRACE15_LSB                  (1U << 16)      /* 12b */
/* SPM_ACK_CHK_CON (0x10006000+0x900) */
#define SPM_ACK_CHK_SW_EN_LSB               (1U << 0)       /* 1b */
#define SPM_ACK_CHK_CLR_ALL_LSB             (1U << 1)       /* 1b */
#define SPM_ACK_CHK_CLR_TIMER_LSB           (1U << 2)       /* 1b */
#define SPM_ACK_CHK_CLR_IRQ_LSB             (1U << 3)       /* 1b */
#define SPM_ACK_CHK_STA_EN_LSB              (1U << 4)       /* 1b */
#define SPM_ACK_CHK_WAKEUP_EN_LSB           (1U << 5)       /* 1b */
#define SPM_ACK_CHK_WDT_EN_LSB              (1U << 6)       /* 1b */
#define SPM_ACK_CHK_LOCK_PC_TRACE_EN_LSB    (1U << 7)       /* 1b */
#define SPM_ACK_CHK_HW_EN_LSB               (1U << 8)       /* 1b */
#define SPM_ACK_CHK_HW_MODE_LSB             (1U << 9)       /* 3b */
#define SPM_ACK_CHK_FAIL_LSB                (1U << 15)      /* 1b */
#define SPM_ACK_CHK_SWINT_EN_LSB            (1U << 16)      /* 16b */
/* SPM_ACK_CHK_PC (0x10006000+0x904) */
#define SPM_ACK_CHK_HW_TRIG_PC_VAL_LSB      (1U << 0)       /* 16b */
#define SPM_ACK_CHK_HW_TARG_PC_VAL_LSB      (1U << 16)      /* 16b */
/* SPM_ACK_CHK_SEL (0x10006000+0x908) */
#define SPM_ACK_CHK_HW_TRIG_SIGNAL_SEL_LSB  (1U << 0)       /* 5b */
#define SPM_ACK_CHK_HW_TRIG_GROUP_SEL_LSB   (1U << 5)       /* 3b */
#define SPM_ACK_CHK_HW_TARG_SIGNAL_SEL_LSB  (1U << 16)      /* 5b */
#define SPM_ACK_CHK_HW_TARG_GROUP_SEL_LSB   (1U << 21)      /* 3b */
/* SPM_ACK_CHK_TIMER (0x10006000+0x90C) */
#define SPM_ACK_CHK_TIMER_VAL_LSB           (1U << 0)       /* 16b */
#define SPM_ACK_CHK_TIMER_LSB               (1U << 16)      /* 16b */
/* SPM_ACK_CHK_STA (0x10006000+0x910) */
#define SPM_ACK_CHK_STA_LSB                 (1U << 0)       /* 32b */
/* SPM_ACK_CHK_LATCH (0x10006000+0x914) */
#define SPM_ACK_CHK_LATCH_LSB               (1U << 0)       /* 32b */
/* SPM_ACK_CHK_CON2 (0x10006000+0x920) */
#define SPM_ACK_CHK_SW_EN2_LSB              (1U << 0)       /* 1b */
#define SPM_ACK_CHK_CLR_ALL2_LSB            (1U << 1)       /* 1b */
#define SPM_ACK_CHK_CLR_TIMER2_LSB          (1U << 2)       /* 1b */
#define SPM_ACK_CHK_CLR_IRQ2_LSB            (1U << 3)       /* 1b */
#define SPM_ACK_CHK_STA_EN2_LSB             (1U << 4)       /* 1b */
#define SPM_ACK_CHK_WAKEUP_EN2_LSB          (1U << 5)       /* 1b */
#define SPM_ACK_CHK_WDT_EN2_LSB             (1U << 6)       /* 1b */
#define SPM_ACK_CHK_LOCK_PC_TRACE_EN2_LSB   (1U << 7)       /* 1b */
#define SPM_ACK_CHK_HW_EN2_LSB              (1U << 8)       /* 1b */
#define SPM_ACK_CHK_HW_MODE2_LSB            (1U << 9)       /* 3b */
#define SPM_ACK_CHK_FAIL2_LSB               (1U << 15)      /* 1b */
#define SPM_ACK_CHK_SWINT_EN2_LSB           (1U << 16)      /* 16b */
/* SPM_ACK_CHK_PC2 (0x10006000+0x924) */
#define SPM_ACK_CHK_HW_TRIG_PC_VAL2_LSB     (1U << 0)       /* 16b */
#define SPM_ACK_CHK_HW_TARG_PC_VAL2_LSB     (1U << 16)      /* 16b */
/* SPM_ACK_CHK_SEL2 (0x10006000+0x928) */
#define SPM_ACK_CHK_HW_TRIG_SIGNAL_SEL2_LSB (1U << 0)       /* 5b */
#define SPM_ACK_CHK_HW_TRIG_GROUP_SEL2_LSB  (1U << 5)       /* 3b */
#define SPM_ACK_CHK_HW_TARG_SIGNAL_SEL2_LSB (1U << 16)      /* 5b */
#define SPM_ACK_CHK_HW_TARG_GROUP_SEL2_LSB  (1U << 21)      /* 3b */
/* SPM_ACK_CHK_TIMER2 (0x10006000+0x92C) */
#define SPM_ACK_CHK_TIMER_VAL2_LSB          (1U << 0)       /* 16b */
#define SPM_ACK_CHK_TIMER2_LSB              (1U << 16)      /* 16b */
/* SPM_ACK_CHK_STA2 (0x10006000+0x930) */
#define SPM_ACK_CHK_STA2_LSB                (1U << 0)       /* 32b */
/* SPM_ACK_CHK_LATCH2 (0x10006000+0x934) */
#define SPM_ACK_CHK_LATCH2_LSB              (1U << 0)       /* 32b */
/* SPM_ACK_CHK_CON3 (0x10006000+0x940) */
#define SPM_ACK_CHK_SW_EN3_LSB              (1U << 0)       /* 1b */
#define SPM_ACK_CHK_CLR_ALL3_LSB            (1U << 1)       /* 1b */
#define SPM_ACK_CHK_CLR_TIMER3_LSB          (1U << 2)       /* 1b */
#define SPM_ACK_CHK_CLR_IRQ3_LSB            (1U << 3)       /* 1b */
#define SPM_ACK_CHK_STA_EN3_LSB             (1U << 4)       /* 1b */
#define SPM_ACK_CHK_WAKEUP_EN3_LSB          (1U << 5)       /* 1b */
#define SPM_ACK_CHK_WDT_EN3_LSB             (1U << 6)       /* 1b */
#define SPM_ACK_CHK_LOCK_PC_TRACE_EN3_LSB   (1U << 7)       /* 1b */
#define SPM_ACK_CHK_HW_EN3_LSB              (1U << 8)       /* 1b */
#define SPM_ACK_CHK_HW_MODE3_LSB            (1U << 9)       /* 3b */
#define SPM_ACK_CHK_FAIL3_LSB               (1U << 15)      /* 1b */
#define SPM_ACK_CHK_SWINT_EN3_LSB           (1U << 16)      /* 16b */
/* SPM_ACK_CHK_PC3 (0x10006000+0x944) */
#define SPM_ACK_CHK_HW_TRIG_PC_VAL3_LSB     (1U << 0)       /* 16b */
#define SPM_ACK_CHK_HW_TARG_PC_VAL3_LSB     (1U << 16)      /* 16b */
/* SPM_ACK_CHK_SEL3 (0x10006000+0x948) */
#define SPM_ACK_CHK_HW_TRIG_SIGNAL_SEL3_LSB (1U << 0)       /* 5b */
#define SPM_ACK_CHK_HW_TRIG_GROUP_SEL3_LSB  (1U << 5)       /* 3b */
#define SPM_ACK_CHK_HW_TARG_SIGNAL_SEL3_LSB (1U << 16)      /* 5b */
#define SPM_ACK_CHK_HW_TARG_GROUP_SEL3_LSB  (1U << 21)      /* 3b */
/* SPM_ACK_CHK_TIMER3 (0x10006000+0x94C) */
#define SPM_ACK_CHK_TIMER_VAL3_LSB          (1U << 0)       /* 16b */
#define SPM_ACK_CHK_TIMER3_LSB              (1U << 16)      /* 16b */
/* SPM_ACK_CHK_STA3 (0x10006000+0x950) */
#define SPM_ACK_CHK_STA3_LSB                (1U << 0)       /* 32b */
/* SPM_ACK_CHK_LATCH3 (0x10006000+0x954) */
#define SPM_ACK_CHK_LATCH3_LSB              (1U << 0)       /* 32b */
/* SPM_ACK_CHK_CON4 (0x10006000+0x960) */
#define SPM_ACK_CHK_SW_EN4_LSB              (1U << 0)       /* 1b */
#define SPM_ACK_CHK_CLR_ALL4_LSB            (1U << 1)       /* 1b */
#define SPM_ACK_CHK_CLR_TIMER4_LSB          (1U << 2)       /* 1b */
#define SPM_ACK_CHK_CLR_IRQ4_LSB            (1U << 3)       /* 1b */
#define SPM_ACK_CHK_STA_EN4_LSB             (1U << 4)       /* 1b */
#define SPM_ACK_CHK_WAKEUP_EN4_LSB          (1U << 5)       /* 1b */
#define SPM_ACK_CHK_WDT_EN4_LSB             (1U << 6)       /* 1b */
#define SPM_ACK_CHK_LOCK_PC_TRACE_EN4_LSB   (1U << 7)       /* 1b */
#define SPM_ACK_CHK_HW_EN4_LSB              (1U << 8)       /* 1b */
#define SPM_ACK_CHK_HW_MODE4_LSB            (1U << 9)       /* 3b */
#define SPM_ACK_CHK_FAIL4_LSB               (1U << 15)      /* 1b */
#define SPM_ACK_CHK_SWINT_EN4_LSB           (1U << 16)      /* 16b */
/* SPM_ACK_CHK_PC4 (0x10006000+0x964) */
#define SPM_ACK_CHK_HW_TRIG_PC_VAL4_LSB     (1U << 0)       /* 16b */
#define SPM_ACK_CHK_HW_TARG_PC_VAL4_LSB     (1U << 16)      /* 16b */
/* SPM_ACK_CHK_SEL4 (0x10006000+0x968) */
#define SPM_ACK_CHK_HW_TRIG_SIGNAL_SEL4_LSB (1U << 0)       /* 5b */
#define SPM_ACK_CHK_HW_TRIG_GROUP_SEL4_LSB  (1U << 5)       /* 3b */
#define SPM_ACK_CHK_HW_TARG_SIGNAL_SEL4_LSB (1U << 16)      /* 5b */
#define SPM_ACK_CHK_HW_TARG_GROUP_SEL4_LSB  (1U << 21)      /* 3b */
/* SPM_ACK_CHK_TIMER4 (0x10006000+0x96C) */
#define SPM_ACK_CHK_TIMER_VAL4_LSB          (1U << 0)       /* 16b */
#define SPM_ACK_CHK_TIMER4_LSB              (1U << 16)      /* 16b */
/* SPM_ACK_CHK_STA4 (0x10006000+0x970) */
#define SPM_ACK_CHK_STA4_LSB                (1U << 0)       /* 32b */
/* SPM_ACK_CHK_LATCH4 (0x10006000+0x974) */
#define SPM_ACK_CHK_LATCH4_LSB              (1U << 0)       /* 32b */

/* --- SPM Flag Define --- */
#define SPM_FLAG_DIS_CPU_PDN                  (1U << 0)
#define SPM_FLAG_DIS_INFRA_PDN                (1U << 1)
#define SPM_FLAG_DIS_DDRPHY_PDN               (1U << 2)
#define SPM_FLAG_DIS_VCORE_DVS                (1U << 3)
#define SPM_FLAG_DIS_VCORE_DFS                (1U << 4)
#define SPM_FLAG_DIS_COMMON_SCENARIO          (1U << 5)
#define SPM_FLAG_DIS_BUS_CLOCK_OFF            (1U << 6)
#define SPM_FLAG_DIS_ATF_ABORT                (1U << 7)
#define SPM_FLAG_KEEP_CSYSPWRUPACK_HIGH       (1U << 8)
#define SPM_FLAG_DIS_VPROC_VSRAM_DVS          (1U << 9)
#define SPM_FLAG_RUN_COMMON_SCENARIO          (1U << 10)
#define SPM_FLAG_EN_MET_DEBUG_USAGE           (1U << 11)
#define SPM_FLAG_SODI_CG_MODE                 (1U << 12)
#define SPM_FLAG_SODI_NO_EVENT                (1U << 13)
#define SPM_FLAG_ENABLE_SODI3                 (1U << 14)
#define SPM_FLAG_DISABLE_MMSYS_DVFS           (1U << 15)
#define SPM_FLAG_DIS_SYSRAM_SLEEP             (1U << 16)
#define SPM_FLAG_DIS_SSPM_SRAM_SLEEP          (1U << 17)
#define SPM_FLAG_DIS_VMODEM_DVS               (1U << 18)
#define SPM_FLAG_SUSPEND_OPTION               (1U << 19)
#define SPM_FLAG_DEEPIDLE_OPTION              (1U << 20)
#define SPM_FLAG_SODI_OPTION                  (1U << 21)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT22    (1U << 22)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT23    (1U << 23)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT24    (1U << 24)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT25    (1U << 25)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT26    (1U << 26)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT27    (1U << 27)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT28    (1U << 28)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT29    (1U << 29)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT30    (1U << 30)
#define SPM_FLAG_SPM_FLAG_DONT_TOUCH_BIT31    (1U << 31)

/* --- SPM Flag1 Define --- */
#define SPM_FLAG1_RESERVED_BIT0               (1U << 0)
#define SPM_FLAG1_ENABLE_CPU_DORMANT          (1U << 1)
#define SPM_FLAG1_ENABLE_CPU_SLEEP_VOLT       (1U << 2)
#define SPM_FLAG1_DISABLE_PWRAP_CLK_SWITCH    (1U << 3)
#define SPM_FLAG1_DISABLE_ULPOSC_OFF          (1U << 4)
#define SPM_FLAG1_VCORE_LP_0P7V               (1U << 5)
#define SPM_FLAG1_DISABLE_MCDSR               (1U << 6)
#define SPM_FLAG1_DISABLE_NO_RESUME           (1U << 7)
#define SPM_FLAG1_BIG_BUCK_OFF_ENABLE         (1U << 8)
#define SPM_FLAG1_BIG_BUCK_ON_ENABLE          (1U << 9)
#define SPM_FLAG1_RESERVED_BIT10              (1U << 10)
#define SPM_FLAG1_RESERVED_BIT11              (1U << 11)
#define SPM_FLAG1_RESERVED_BIT12              (1U << 12)
#define SPM_FLAG1_RESERVED_BIT13              (1U << 13)
#define SPM_FLAG1_RESERVED_BIT14              (1U << 14)
#define SPM_FLAG1_DIS_ARMPLL_OFF              (1U << 15)
#define SPM_FLAG1_DIS_AXI_BUS_TO_26M          (1U << 16)
#define SPM_FLAG1_DIS_IMP_DIS                 (1U << 17)
#define SPM_FLAG1_DIS_IMP_COPY                (1U << 18)
#define SPM_FLAG1_DIS_EMI_TOGGLE_WORKAROUND   (1U << 19)
#define SPM_FLAG1_DIS_DRAM_ENTER_SREF         (1U << 20)
#define SPM_FLAG1_DIS_DRAM_DLL_OFF            (1U << 21)
#define SPM_FLAG1_DIS_PHYPLL_OFF              (1U << 22)
#define SPM_FLAG1_DIS_MPLL_OFF                (1U << 23)
#define SPM_FLAG1_DIS_SYSPLL_OFF              (1U << 24)
#define SPM_FLAG1_DIS_TOP_AXI_CLK_OFF         (1U << 25)
#define SPM_FLAG1_DIS_PCM_26M_SWITCH          (1U << 26)
#define SPM_FLAG1_DIS_CKSQ_OFF                (1U << 27)
#define SPM_FLAG1_DIS_SRCVOLTEN_OFF           (1U << 28)
#define SPM_FLAG1_DIS_CHB_CG_FREE_EN          (1U << 29)
#define SPM_FLAG1_DIS_CHA_DCM_RES             (1U << 30)
#define SPM_FLAG1_DIS_SW_MR4                  (1U << 31)

/* --- SPM DEBUG Define --- */
#define SPM_DBG_DEBUG_IDX_26M_WAKE            (1U << 0)
#define SPM_DBG_DEBUG_IDX_26M_SLEEP           (1U << 1)
#define SPM_DBG_DEBUG_IDX_INFRA_WAKE          (1U << 2)
#define SPM_DBG_DEBUG_IDX_INFRA_SLEEP         (1U << 3)
#define SPM_DBG_DEBUG_IDX_APSRC_WAKE          (1U << 4)
#define SPM_DBG_DEBUG_IDX_APSRC_SLEEP         (1U << 5)
#define SPM_DBG_DEBUG_IDX_VRF18_WAKE          (1U << 6)
#define SPM_DBG_DEBUG_IDX_VRF18_SLEEP         (1U << 7)
#define SPM_DBG_DEBUG_IDX_DDREN_WAKE          (1U << 8)
#define SPM_DBG_DEBUG_IDX_DDREN_SLEEP         (1U << 9)
#define SPM_DBG_DEBUG_IDX_NFC_CKBUF_ON        (1U << 10)
#define SPM_DBG_DEBUG_IDX_NFC_CKBUF_OFF       (1U << 11)
#define SPM_DBG_DEBUG_IDX_CPU_PDN             (1U << 12)
#define SPM_DBG_DEBUG_IDX_DPD                 (1U << 13)
#define SPM_DBG_DEBUG_IDX_CONN_CKBUF_ON       (1U << 14)
#define SPM_DBG_DEBUG_IDX_CONN_CKBUF_OFF      (1U << 15)
#define SPM_DBG_DEBUG_IDX_VCORE_DVFS_START    (1U << 16)
#define SPM_DBG_DEBUG_IDX_DDREN2_WAKE         (1U << 17)
#define SPM_DBG_DEBUG_IDX_DDREN2_SLEEP        (1U << 18)
#define SPM_DBG_DEBUG_IDX_SSPM_WFI            (1U << 19)
#define SPM_DBG_DEBUG_IDX_SSPM_SRAM_SLP       (1U << 20)
#define SPM_DBG_RESERVED_BIT21                (1U << 21)
#define SPM_DBG_RESERVED_BIT22                (1U << 22)
#define SPM_DBG_RESERVED_BIT23                (1U << 23)
#define SPM_DBG_RESERVED_BIT24                (1U << 24)
#define SPM_DBG_RESERVED_BIT25                (1U << 25)
#define SPM_DBG_RESERVED_BIT26                (1U << 26)
#define SPM_DBG_SODI1_FLAG                    (1U << 27)
#define SPM_DBG_SODI3_FLAG                    (1U << 28)
#define SPM_DBG_VCORE_DVFS_FLAG               (1U << 29)
#define SPM_DBG_DEEPIDLE_FLAG                 (1U << 30)
#define SPM_DBG_SUSPEND_FLAG                  (1U << 31)

/* --- SPM DEBUG1 Define --- */
#define SPM_DBG1_DRAM_SREF_ACK_TO             (1U << 0)
#define SPM_DBG1_PWRAP_SLEEP_ACK_TO           (1U << 1)
#define SPM_DBG1_PWRAP_SPI_ACK_TO             (1U << 2)
#define SPM_DBG1_DRAM_GATE_ERR_DDREN_WAKEUP   (1U << 3)
#define SPM_DBG1_DRAM_GATE_ERR_LEAVE_LP_SCN   (1U << 4)
#define SPM_DBG1_RESERVED_BIT5                (1U << 5)
#define SPM_DBG1_RESERVED_BIT6                (1U << 6)
#define SPM_DBG1_RESERVED_BIT7                (1U << 7)
#define SPM_DBG1_RESERVED_BIT8                (1U << 8)
#define SPM_DBG1_RESERVED_BIT9                (1U << 9)
#define SPM_DBG1_RESERVED_BIT10               (1U << 10)
#define SPM_DBG1_RESERVED_BIT11               (1U << 11)
#define SPM_DBG1_RESERVED_BIT12               (1U << 12)
#define SPM_DBG1_RESERVED_BIT13               (1U << 13)
#define SPM_DBG1_RESERVED_BIT14               (1U << 14)
#define SPM_DBG1_RESERVED_BIT15               (1U << 15)
#define SPM_DBG1_RESERVED_BIT16               (1U << 16)
#define SPM_DBG1_RESERVED_BIT17               (1U << 17)
#define SPM_DBG1_RESERVED_BIT18               (1U << 18)
#define SPM_DBG1_RESERVED_BIT19               (1U << 19)
#define SPM_DBG1_RESERVED_BIT20               (1U << 20)
#define SPM_DBG1_RESERVED_BIT21               (1U << 21)
#define SPM_DBG1_RESERVED_BIT22               (1U << 22)
#define SPM_DBG1_RESERVED_BIT23               (1U << 23)
#define SPM_DBG1_RESERVED_BIT24               (1U << 24)
#define SPM_DBG1_RESERVED_BIT25               (1U << 25)
#define SPM_DBG1_RESERVED_BIT26               (1U << 26)
#define SPM_DBG1_RESERVED_BIT27               (1U << 27)
#define SPM_DBG1_RESERVED_BIT28               (1U << 28)
#define SPM_DBG1_RESERVED_BIT29               (1U << 29)
#define SPM_DBG1_RESERVED_BIT30               (1U << 30)
#define SPM_DBG1_RESERVED_BIT31               (1U << 31)

/* --- R0 Define --- */
#define R0_SC_26M_CK_OFF                      (1U << 0)
#define R0_BIT1                               (1U << 1)
#define R0_SC_MEM_CK_OFF                      (1U << 2)
#define R0_SC_AXI_CK_OFF                      (1U << 3)
#define R0_SC_DR_GATE_RETRY_EN_PCM            (1U << 4)
#define R0_SC_MD26M_CK_OFF                    (1U << 5)
#define R0_SC_DPY_MODE_SW_PCM                 (1U << 6)
#define R0_SC_DMSUS_OFF_PCM                   (1U << 7)
#define R0_SC_DPY_2ND_DLL_EN_PCM              (1U << 8)
#define R0_BIT9                               (1U << 9)
#define R0_SC_MPLLOUT_OFF                     (1U << 10)
#define R0_SC_TX_TRACKING_DIS                 (1U << 11)
#define R0_SC_DPY_DLL_EN_PCM                  (1U << 12)
#define R0_SC_DPY_DLL_CK_EN_PCM               (1U << 13)
#define R0_SC_DPY_VREF_EN_PCM                 (1U << 14)
#define R0_SC_PHYPLL_EN_PCM                   (1U << 15)
#define R0_SC_DDRPHY_FB_CK_EN_PCM             (1U << 16)
#define R0_SC_DPY_BCLK_ENABLE                 (1U << 17)
#define R0_SC_MPLL_OFF                        (1U << 18)
#define R0_SC_SHU_RESTORE                     (1U << 19)
#define R0_SC_CKSQ0_OFF                       (1U << 20)
#define R0_SC_CKSQ1_OFF                       (1U << 21)
#define R0_SC_DR_SHU_EN_PCM                   (1U << 22)
#define R0_SC_DPHY_PRECAL_UP                  (1U << 23)
#define R0_SC_MPLL_S_OFF                      (1U << 24)
#define R0_SC_DPHY_RXDLY_TRACK_EN             (1U << 25)
#define R0_SC_PHYPLL_SHU_EN_PCM               (1U << 26)
#define R0_SC_PHYPLL2_SHU_EN_PCM              (1U << 27)
#define R0_SC_PHYPLL_MODE_SW_PCM              (1U << 28)
#define R0_SC_PHYPLL2_MODE_SW_PCM             (1U << 29)
#define R0_SC_DR_SHU_LEVEL_PCM0               (1U << 30)
#define R0_SC_DR_SHU_LEVEL_PCM1               (1U << 31)

/* --- R7 Define --- */
#define R7_PWRAP_SLEEP_REQ                    (1U << 0)
#define R7_EMI_CLK_OFF_REQ                    (1U << 1)
#define R7_TOP_MAS_PAU_REQ                    (1U << 2)
#define R7_SPM2CKSYS_MEM_CK_MUX_UPDATE        (1U << 3)
#define R7_PCM_CK_SEL0                        (1U << 4)
#define R7_PCM_CK_SEL1                        (1U << 5)
#define R7_SPM2RC_DVS_DONE                    (1U << 6)
#define R7_FREQH_PAUSE_MPLL                   (1U << 7)
#define R7_SC_26M_CK_SEL                      (1U << 8)
#define R7_PCM_TIMER_SET                      (1U << 9)
#define R7_PCM_TIMER_CLR                      (1U << 10)
#define R7_SRCVOLTEN                          (1U << 11)
#define R7_CSYSPWRUPACK                       (1U << 12)
#define R7_IM_SLEEP_ENABLE                    (1U << 13)
#define R7_SRCCLKENO_0                        (1U << 14)
#define R7_SYSRST                             (1U << 15)
#define R7_MD_APSRC_ACK                       (1U << 16)
#define R7_CPU_SYS_TIMER_CLK_SEL              (1U << 17)
#define R7_SC_AXI_DCM_DIS                     (1U << 18)
#define R7_FREQH_PAUSE_MAIN                   (1U << 19)
#define R7_FREQH_PAUSE_MEM                    (1U << 20)
#define R7_SRCCLKENO_1                        (1U << 21)
#define R7_WDT_KICK_P                         (1U << 22)
#define R7_SPM2RC_EVENT_ABORT_ACK             (1U << 23)
#define R7_WAKEUP_EXT_W_SEL                   (1U << 24)
#define R7_WAKEUP_EXT_R_SEL                   (1U << 25)
#define R7_PMIC_IRQ_REQ_EN                    (1U << 26)
#define R7_FORCE_26M_WAKE                     (1U << 27)
#define R7_FORCE_APSRC_WAKE                   (1U << 28)
#define R7_FORCE_INFRA_WAKE                   (1U << 29)
#define R7_FORCE_VRF18_WAKE                   (1U << 30)
#define R7_SC_DR_SHORT_QUEUE_PCM              (1U << 31)

/* --- R12 Define --- */
#define R12_PCM_TIMER                         (1U << 0)
#define R12_SSPM_WDT_EVENT_B                  (1U << 1)
#define R12_KP_IRQ_B                          (1U << 2)
#define R12_APWDT_EVENT_B                     (1U << 3)
#define R12_APXGPT1_EVENT_B                   (1U << 4)
#define R12_CONN2AP_SPM_WAKEUP_B              (1U << 5)
#define R12_EINT_EVENT_B                      (1U << 6)
#define R12_CONN_WDT_IRQ_B                    (1U << 7)
#define R12_CCIF0_EVENT_B                     (1U << 8)
#define R12_LOWBATTERY_IRQ_B                  (1U << 9)
#define R12_SSPM_SPM_IRQ_B                    (1U << 10)
#define R12_SCP_SPM_IRQ_B                     (1U << 11)
#define R12_SCP_WDT_EVENT_B                   (1U << 12)
#define R12_PCM_WDT_WAKEUP_B                  (1U << 13)
#define R12_USB_CDSC_B                        (1U << 14)
#define R12_USB_POWERDWN_B                    (1U << 15)
#define R12_SYS_TIMER_EVENT_B                 (1U << 16)
#define R12_EINT_EVENT_SECURE_B               (1U << 17)
#define R12_CCIF1_EVENT_B                     (1U << 18)
#define R12_UART0_IRQ_B                       (1U << 19)
#define R12_AFE_IRQ_MCU_B                     (1U << 20)
#define R12_THERM_CTRL_EVENT_B                (1U << 21)
#define R12_SYS_CIRQ_IRQ_B                    (1U << 22)
#define R12_MD2AP_PEER_EVENT_B                (1U << 23)
#define R12_CSYSPWREQ_B                       (1U << 24)
#define R12_MD1_WDT_B                         (1U << 25)
#define R12_CLDMA_EVENT_B                     (1U << 26)
#define R12_SEJ_WDT_GPT_B                     (1U << 27)
#define R12_ALL_SSPM_WAKEUP_B                 (1U << 28)
#define R12_CPU_IRQ_B                         (1U << 29)
#define R12_CPU_WFI_AND_B                     (1U << 30)
#define R12_MCUSYS_IDLE_TO_EMI_ALL_B          (1U << 31)

/* --- R12ext Define --- */
#define R12EXT_26M_WAKE                       (1U << 0)
#define R12EXT_26M_SLEEP                      (1U << 1)
#define R12EXT_INFRA_WAKE                     (1U << 2)
#define R12EXT_INFRA_SLEEP                    (1U << 3)
#define R12EXT_APSRC_WAKE                     (1U << 4)
#define R12EXT_APSRC_SLEEP                    (1U << 5)
#define R12EXT_VRF18_WAKE                     (1U << 6)
#define R12EXT_VRF18_SLEEP                    (1U << 7)
#define R12EXT_DVFS_ALL_STATE                 (1U << 8)
#define R12EXT_DVFS_LEVEL_STATE0              (1U << 9)
#define R12EXT_DVFS_LEVEL_STATE1              (1U << 10)
#define R12EXT_DVFS_LEVEL_STATE2              (1U << 11)
#define R12EXT_DDREN_WAKE                     (1U << 12)
#define R12EXT_DDREN_SLEEP                    (1U << 13)
#define R12EXT_NFC_CLK_BUF_WAKE               (1U << 14)
#define R12EXT_NFC_CLK_BUF_SLEEP              (1U << 15)
#define R12EXT_CONN_CLK_BUF_WAKE              (1U << 16)
#define R12EXT_CONN_CLK_BUF_SLEEP             (1U << 17)
#define R12EXT_MD_DVFS_ERROR_STATUS           (1U << 18)
#define R12EXT_DVFS_LEVEL_STATE3              (1U << 19)
#define R12EXT_DVFS_LEVEL_STATE4              (1U << 20)
#define R12EXT_DVFS_LEVEL_STATE5              (1U << 21)
#define R12EXT_DVFS_LEVEL_STATE6              (1U << 22)
#define R12EXT_DVFS_LEVEL_STATE7              (1U << 23)
#define R12EXT_DVFS_LEVEL_STATE8              (1U << 24)
#define R12EXT_DVFS_LEVEL_STATE9              (1U << 25)
#define R12EXT_DVFS_LEVEL_STATE_G0            (1U << 26)
#define R12EXT_DVFS_LEVEL_STATE_G1            (1U << 27)
#define R12EXT_DVFS_LEVEL_STATE_G2            (1U << 28)
#define R12EXT_DVFS_LEVEL_STATE_G3            (1U << 29)
#define R12EXT_HYBRID_DDREN_SLEEP             (1U << 30)
#define R12EXT_HYBRID_DDREN_WAKE              (1U << 31)

/* --- R13 Define --- */
#define R13_EXT_SRCCLKENI_0                   (1U << 0)
#define R13_EXT_SRCCLKENI_1                   (1U << 1)
#define R13_MD1_SRCCLKENA                     (1U << 2)
#define R13_MD1_APSRC_REQ                     (1U << 3)
#define R13_CONN_DDR_EN                       (1U << 4)
#define R13_MD2_SRCCLKENA                     (1U << 5)
#define R13_SSPM_SRCCLKENA                    (1U << 6)
#define R13_SSPM_APSRC_REQ                    (1U << 7)
#define R13_MD_STATE                          (1U << 8)
#define R13_EMI_CLK_OFF_2_ACK                 (1U << 9)
#define R13_MM_STATE                          (1U << 10)
#define R13_SSPM_STATE                        (1U << 11)
#define R13_MD_DDR_EN                         (1U << 12)
#define R13_CONN_STATE                        (1U << 13)
#define R13_CONN_SRCCLKENA                    (1U << 14)
#define R13_CONN_APSRC_REQ                    (1U << 15)
#define R13_SLEEP_EVENT_STA                   (1U << 16)
#define R13_WAKE_EVENT_STA                    (1U << 17)
#define R13_EMI_IDLE                          (1U << 18)
#define R13_CSYSPWRUPREQ                      (1U << 19)
#define R13_PWRAP_SLEEP_ACK                   (1U << 20)
#define R13_EMI_CLK_OFF_ACK_ALL               (1U << 21)
#define R13_TOP_MAS_PAU_ACK                   (1U << 22)
#define R13_SW_DMDRAMCSHU_ACK_ALL             (1U << 23)
#define R13_RC2SPM_EVENT_ABORT_MASK_OR        (1U << 24)
#define R13_DR_SHORT_QUEUE_ACK_ALL            (1U << 25)
#define R13_INFRA_AUX_IDLE                    (1U << 26)
#define R13_DVFS_ALL_STATE                    (1U << 27)
#define R13_RC2SPM_EVENT_ABORT_OR             (1U << 28)
#define R13_DRAMC_SPCMD_APSRC_REQ             (1U << 29)
#define R13_MD1_VRF18_REQ                     (1U << 30)
#define R13_C2K_VRF18_REQ                     (1U << 31)

#define is_cpu_pdn(flags)		(!((flags) & SPM_FLAG_DIS_CPU_PDN))
#define is_infra_pdn(flags)		(!((flags) & SPM_FLAG_DIS_INFRA_PDN))
#define is_ddrphy_pdn(flags)		(!((flags) & SPM_FLAG_DIS_DDRPHY_PDN))

#define MP0_SPMC_SRAM_DORMANT_EN	(1<<0)
#define MP1_SPMC_SRAM_DORMANT_EN	(1<<1)
#define MP2_SPMC_SRAM_DORMANT_EN	(1<<2)

#define EVENT_VEC(event, resume, imme, pc)	\
	(((pc) << 16) |				\
	 (!!(imme) << 7) |			\
	 (!!(resume) << 6) |			\
	 ((event) & 0x3f))

#define SPM_PROJECT_CODE	0xb16
#define SPM_REGWR_CFG_KEY	(SPM_PROJECT_CODE << 16)

/**************************************
 * Config and Parameter
 **************************************/
#define POWER_ON_VAL1_DEF		0x00015800
#define PCM_FSM_STA_DEF			0x00108490
#define SPM_WAKEUP_EVENT_MASK_DEF	0xF0F92218
#define PCM_WDT_TIMEOUT			(30 * 32768)	/* 30s */
#define PCM_TIMER_MAX			(0xffffffff - PCM_WDT_TIMEOUT)

/**************************************
 * Define and Declare
 **************************************/
/* PCM_PWR_IO_EN */
#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

/* SPM_SWINT */
#define PCM_SW_INT0		(1U << 0)
#define PCM_SW_INT1		(1U << 1)
#define PCM_SW_INT2		(1U << 2)
#define PCM_SW_INT3		(1U << 3)
#define PCM_SW_INT4		(1U << 4)
#define PCM_SW_INT5		(1U << 5)
#define PCM_SW_INT6		(1U << 6)
#define PCM_SW_INT7		(1U << 7)
#define PCM_SW_INT8		(1U << 8)
#define PCM_SW_INT9		(1U << 9)
#define PCM_SW_INT_ALL		(PCM_SW_INT9 | PCM_SW_INT8 | PCM_SW_INT7 | \
				 PCM_SW_INT6 | PCM_SW_INT5 | PCM_SW_INT4 | \
				 PCM_SW_INT3 | PCM_SW_INT2 | PCM_SW_INT1 | \
				 PCM_SW_INT0)
/* SPM_IRQ_MASK */
#define ISRM_TWAM		(1U << 2)
#define ISRM_PCM_RETURN		(1U << 3)
#define ISRM_RET_IRQ0		(1U << 8)
#define ISRM_RET_IRQ1		(1U << 9)
#define ISRM_RET_IRQ2		(1U << 10)
#define ISRM_RET_IRQ3		(1U << 11)
#define ISRM_RET_IRQ4		(1U << 12)
#define ISRM_RET_IRQ5		(1U << 13)
#define ISRM_RET_IRQ6		(1U << 14)
#define ISRM_RET_IRQ7		(1U << 15)
#define ISRM_RET_IRQ8		(1U << 16)
#define ISRM_RET_IRQ9		(1U << 17)
#define ISRM_RET_IRQ_AUX	(ISRM_RET_IRQ9 | ISRM_RET_IRQ8 | \
				 ISRM_RET_IRQ7 | ISRM_RET_IRQ6 | \
				 ISRM_RET_IRQ5 | ISRM_RET_IRQ4 | \
				 ISRM_RET_IRQ3 | ISRM_RET_IRQ2 | \
				 ISRM_RET_IRQ1)
#define ISRM_ALL_EXC_TWAM	(ISRM_RET_IRQ_AUX)
#define ISRM_ALL		(ISRM_ALL_EXC_TWAM | ISRM_TWAM)

/* SPM_IRQ_STA */
#define ISRS_TWAM		(1U << 2)
#define ISRS_PCM_RETURN		(1U << 3)
#define ISRS_SW_INT0		(1U << 4)
#define ISRC_TWAM		ISRS_TWAM
#define ISRC_ALL_EXC_TWAM	ISRS_PCM_RETURN
#define ISRC_ALL		(ISRC_ALL_EXC_TWAM | ISRC_TWAM)

/* SPM_WAKEUP_MISC */
#define WAKE_MISC_TWAM		(1U << 18)
#define WAKE_MISC_PCM_TIMER	(1U << 19)
#define WAKE_MISC_CPU_WAKE	(1U << 20)

enum SPM_WAKE_SRC_LIST {
	WAKE_SRC_R12_PCM_TIMER = (1U << 0),
	WAKE_SRC_R12_SSPM_WDT_EVENT_B = (1U << 1),
	WAKE_SRC_R12_KP_IRQ_B = (1U << 2),
	WAKE_SRC_R12_APWDT_EVENT_B = (1U << 3),
	WAKE_SRC_R12_APXGPT1_EVENT_B = (1U << 4),
	WAKE_SRC_R12_CONN2AP_SPM_WAKEUP_B = (1U << 5),
	WAKE_SRC_R12_EINT_EVENT_B = (1U << 6),
	WAKE_SRC_R12_CONN_WDT_IRQ_B = (1U << 7),
	WAKE_SRC_R12_CCIF0_EVENT_B = (1U << 8),
	WAKE_SRC_R12_LOWBATTERY_IRQ_B = (1U << 9),
	WAKE_SRC_R12_SSPM_SPM_IRQ_B = (1U << 10),
	WAKE_SRC_R12_SCP_SPM_IRQ_B = (1U << 11),
	WAKE_SRC_R12_SCP_WDT_EVENT_B = (1U << 12),
	WAKE_SRC_R12_PCM_WDT_WAKEUP_B = (1U << 13),
	WAKE_SRC_R12_USB_CDSC_B = (1U << 14),
	WAKE_SRC_R12_USB_POWERDWN_B = (1U << 15),
	WAKE_SRC_R12_SYS_TIMER_EVENT_B = (1U << 16),
	WAKE_SRC_R12_EINT_EVENT_SECURE_B = (1U << 17),
	WAKE_SRC_R12_CCIF1_EVENT_B = (1U << 18),
	WAKE_SRC_R12_UART0_IRQ_B = (1U << 19),
	WAKE_SRC_R12_AFE_IRQ_MCU_B = (1U << 20),
	WAKE_SRC_R12_THERM_CTRL_EVENT_B = (1U << 21),
	WAKE_SRC_R12_SYS_CIRQ_IRQ_B = (1U << 22),
	WAKE_SRC_R12_MD2AP_PEER_EVENT_B = (1U << 23),
	WAKE_SRC_R12_CSYSPWREQ_B = (1U << 24),
	WAKE_SRC_R12_MD1_WDT_B = (1U << 25),
	WAKE_SRC_R12_CLDMA_EVENT_B = (1U << 26),
	WAKE_SRC_R12_SEJ_WDT_GPT_B = (1U << 27),
	WAKE_SRC_R12_ALL_SSPM_WAKEUP_B = (1U << 28),
	WAKE_SRC_R12_CPU_IRQ_B = (1U << 29),
	WAKE_SRC_R12_CPU_WFI_AND_B = (1U << 30),
};

struct pcm_desc {
	const char *version;
	const uint32_t *base;
	const uint32_t base_dma;
	const uint32_t size;
	const uint32_t sess;
	const uint32_t replace;
	const uint32_t addr_2nd;
	const uint32_t reserved;

	uint32_t vec0;
	uint32_t vec1;
	uint32_t vec2;
	uint32_t vec3;
	uint32_t vec4;
	uint32_t vec5;
	uint32_t vec6;
	uint32_t vec7;
	uint32_t vec8;
	uint32_t vec9;
	uint32_t vec10;
	uint32_t vec11;
	uint32_t vec12;
	uint32_t vec13;
	uint32_t vec14;
	uint32_t vec15;
};

struct pwr_ctrl {
	uint32_t pcm_flags;
	uint32_t pcm_flags1;
	uint32_t timer_val;
	uint32_t wake_src;

	/* SPM_AP_STANDBY_CON */
	uint8_t wfi_op;
	uint8_t mp0_cputop_idle_mask;
	uint8_t mp1_cputop_idle_mask;
	uint8_t mcusys_idle_mask;
	uint8_t mm_mask_b;
	uint8_t md_ddr_en_0_dbc_en;
	uint8_t md_ddr_en_1_dbc_en;
	uint8_t md_mask_b;
	uint8_t sspm_mask_b;
	uint8_t scp_mask_b;
	uint8_t srcclkeni_mask_b;
	uint8_t md_apsrc_1_sel;
	uint8_t md_apsrc_0_sel;
	uint8_t conn_ddr_en_dbc_en;
	uint8_t conn_mask_b;
	uint8_t conn_apsrc_sel;

	/* SPM_SRC_REQ */
	uint8_t spm_apsrc_req;
	uint8_t spm_f26m_req;
	uint8_t spm_infra_req;
	uint8_t spm_vrf18_req;
	uint8_t spm_ddren_req;
	uint8_t spm_rsv_src_req;
	uint8_t spm_ddren_2_req;
	uint8_t cpu_md_dvfs_sop_force_on;

	/* SPM_SRC_MASK */
	uint8_t csyspwreq_mask;
	uint8_t ccif0_md_event_mask_b;
	uint8_t ccif0_ap_event_mask_b;
	uint8_t ccif1_md_event_mask_b;
	uint8_t ccif1_ap_event_mask_b;
	uint8_t ccif2_md_event_mask_b;
	uint8_t ccif2_ap_event_mask_b;
	uint8_t ccif3_md_event_mask_b;
	uint8_t ccif3_ap_event_mask_b;
	uint8_t md_srcclkena_0_infra_mask_b;
	uint8_t md_srcclkena_1_infra_mask_b;
	uint8_t conn_srcclkena_infra_mask_b;
	uint8_t ufs_infra_req_mask_b;
	uint8_t srcclkeni_infra_mask_b;
	uint8_t md_apsrc_req_0_infra_mask_b;
	uint8_t md_apsrc_req_1_infra_mask_b;
	uint8_t conn_apsrcreq_infra_mask_b;
	uint8_t ufs_srcclkena_mask_b;
	uint8_t md_vrf18_req_0_mask_b;
	uint8_t md_vrf18_req_1_mask_b;
	uint8_t ufs_vrf18_req_mask_b;
	uint8_t gce_vrf18_req_mask_b;
	uint8_t conn_infra_req_mask_b;
	uint8_t gce_apsrc_req_mask_b;
	uint8_t disp0_apsrc_req_mask_b;
	uint8_t disp1_apsrc_req_mask_b;
	uint8_t mfg_req_mask_b;
	uint8_t vdec_req_mask_b;

	/* SPM_SRC2_MASK */
	uint8_t md_ddr_en_0_mask_b;
	uint8_t md_ddr_en_1_mask_b;
	uint8_t conn_ddr_en_mask_b;
	uint8_t ddren_sspm_apsrc_req_mask_b;
	uint8_t ddren_scp_apsrc_req_mask_b;
	uint8_t disp0_ddren_mask_b;
	uint8_t disp1_ddren_mask_b;
	uint8_t gce_ddren_mask_b;
	uint8_t ddren_emi_self_refresh_ch0_mask_b;
	uint8_t ddren_emi_self_refresh_ch1_mask_b;

	/* SPM_WAKEUP_EVENT_MASK */
	uint32_t spm_wakeup_event_mask;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	uint32_t spm_wakeup_event_ext_mask;

	/* SPM_SRC3_MASK */
	uint8_t md_ddr_en_2_0_mask_b;
	uint8_t md_ddr_en_2_1_mask_b;
	uint8_t conn_ddr_en_2_mask_b;
	uint8_t ddren2_sspm_apsrc_req_mask_b;
	uint8_t ddren2_scp_apsrc_req_mask_b;
	uint8_t disp0_ddren2_mask_b;
	uint8_t disp1_ddren2_mask_b;
	uint8_t gce_ddren2_mask_b;
	uint8_t ddren2_emi_self_refresh_ch0_mask_b;
	uint8_t ddren2_emi_self_refresh_ch1_mask_b;

	uint8_t mp0_cpu0_wfi_en;
	uint8_t mp0_cpu1_wfi_en;
	uint8_t mp0_cpu2_wfi_en;
	uint8_t mp0_cpu3_wfi_en;

	uint8_t mp1_cpu0_wfi_en;
	uint8_t mp1_cpu1_wfi_en;
	uint8_t mp1_cpu2_wfi_en;
	uint8_t mp1_cpu3_wfi_en;
};

struct wake_status {
	uint32_t assert_pc;
	uint32_t r12;
	uint32_t r12_ext;
	uint32_t raw_sta;
	uint32_t raw_ext_sta;
	uint32_t wake_misc;
	uint32_t timer_out;
	uint32_t r13;
	uint32_t r15;
	uint32_t idle_sta;
	uint32_t req_sta;
	uint32_t debug_flag;
	uint32_t debug_flag1;
	uint32_t event_reg;
	uint32_t isr;
	uint32_t sw_flag;
	uint32_t sw_flag1;
	uint32_t log_index;
};

typedef struct spm_data {
	unsigned int cmd;
	union {
		struct {
			unsigned int sys_timestamp_l;
			unsigned int sys_timestamp_h;
			unsigned int sys_src_clk_l;
			unsigned int sys_src_clk_h;
			unsigned int spm_opt;
		} suspend;
		struct {
			unsigned int args1;
			unsigned int args2;
			unsigned int args3;
			unsigned int args4;
			unsigned int args5;
			unsigned int args6;
			unsigned int args7;
		} args;
	} u;
} spm_data_t;

enum {
	SPM_SUSPEND,
	SPM_RESUME
};

extern void spm_disable_pcm_timer(void);
extern void spm_set_bootaddr(unsigned long bootaddr);
extern void spm_set_cpu_status(int cpu);
extern void spm_set_power_control(const struct pwr_ctrl *pwrctrl);
extern void spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);
extern void spm_set_pcm_flags(const struct pwr_ctrl *pwrctrl);
extern void spm_send_cpu_wakeup_event(void);
extern void spm_get_wakeup_status(struct wake_status *wakesta);
extern void spm_clean_after_wakeup(void);
extern void spm_output_wake_reason(struct wake_status *wakesta,
				   const char *scenario);
extern void spm_set_pcm_wdt(int en);
extern void spm_lock_get(void);
extern void spm_lock_release(void);
extern void spm_boot_init(void);
extern const char *spm_get_firmware_version(void);

#endif /* SPM_H */
