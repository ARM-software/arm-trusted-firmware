/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_COMMON_V1_H
#define MT_SPM_COMMON_V1_H

#include <common/debug.h>

struct spm_hwcg_info {
	uint32_t pwr;
	uint32_t pwr_msb;
	uint32_t module_busy;
};

#define HWCG_INFO_INIT(_info) \
	({ _info.pwr = _info.pwr_msb = _info.module_busy = 0; })

#define DECLARE_HWCG_REG(_name_, _info) ({ \
		_info.pwr = REG_PWR_STATUS_##_name_##_REQ_MASK; \
		_info.pwr_msb = REG_PWR_STATUS_MSB_##_name_##_REQ_MASK; \
		_info.module_busy = REG_MODULE_BUSY_##_name_##_REQ_MASK; })

#define DECLARE_HWCG_DEFAULT(_name_, _info) ({ \
		_info.pwr = SPM_HWCG_##_name_##_PWR_MB; \
		_info.pwr_msb = SPM_HWCG_##_name_##_PWR_MSB_MB; \
		_info.module_busy = SPM_HWCG_##_name_##_MODULE_BUSY_MB; })

#define PERI_REQ_EN_INFO_INIT(_info)	({ _info.req_en = 0; })

#define PERI_REQ_STA_INFO_INIT(_info)	({ _info.req_sta = 0; })

#define DECLARE_PERI_REQ_EN_REG(_offset, _info) \
	({ _info.req_en = REG_PERI_REQ_EN(_offset); })

#define DECLARE_PERI_REQ_STA_REG(_offset, _info) \
	({ _info.req_sta = REG_PERI_REQ_STA(_offset); })

#define DECLARE_PERI_REQ_DEFAULT(_name_, _info) \
	({ _info.req_en = PERI_REQ_##_name_##_MB; })

#define CTRL0_SC_26M_CK_OFF			BIT(0)
#define CTRL0_SC_VLP_BUS_CK_OFF			BIT(1)
#define CTRL0_SC_PMIF_CK_OFF			BIT(2)
#define CTRL0_SC_AXI_CK_OFF			BIT(3)
#define CTRL0_SC_AXI_MEM_CK_OFF			BIT(4)
#define CTRL0_SC_MD26M_CK_OFF			BIT(5)
#define CTRL0_SC_MD32K_CK_OFF			BIT(6)
#define CTRL0_SC_VLP_26M_CLK_SEL		BIT(7)
#define CTRL0_SC_26M_CK_SEL			BIT(8)
#define CTRL0_SC_TOP_26M_CLK_SEL		BIT(9)
#define CTRL0_SC_SYS_TIMER_CLK_32K_SEL		BIT(10)
#define CTRL0_SC_CIRQ_CLK_32K_SEL		BIT(11)
#define CTRL0_SC_AXI_DCM_DIS			BIT(12)
#define CTRL0_SC_CKSQ0_OFF			BIT(13)
#define CTRL0_SC_CKSQ1_OFF			BIT(14)
#define CTRL0_VCORE_PWR_ISO			BIT(15)
#define CTRL0_VCORE_PWR_ISO_PRE			BIT(16)
#define CTRL0_VCORE_PWR_RST_B			BIT(17)
#define CTRL0_VCORE_RESTORE_ENABLE		BIT(18)
#define CTRL0_SC_TOP_RESTORE_26M_CLK_SEL	BIT(19)
#define CTRL0_AOC_VCORE_SRAM_ISO_DIN		BIT(20)
#define CTRL0_AOC_VCORE_SRAM_LATCH_ENB		BIT(21)
#define CTRL0_AOC_VCORE_ANA_ISO			BIT(22)
#define CTRL0_AOC_VCORE_ANA_ISO_PRE		BIT(23)
#define CTRL0_AOC_VLPTOP_SRAM_ISO_DIN		BIT(24)
#define CTRL0_AOC_VLPTOP_SRAM_LATCH_ENB		BIT(25)
#define CTRL0_AOC_VCORE_IO_ISO			BIT(26)
#define CTRL0_AOC_VCORE_IO_LATCH_ENB		BIT(27)
#define CTRL0_RTFF_VCORE_SAVE			BIT(28)
#define CTRL0_RTFF_VCORE_NRESTORE		BIT(29)
#define CTRL0_RTFF_VCORE_CLK_DIS		BIT(30)

/* MD32PCM_CTRL1 define */
#define CTRL1_PWRAP_SLEEP_REQ			BIT(0)
#define CTRL1_IM_SLP_EN				BIT(1)
#define CTRL1_SPM_LEAVE_VCORE_OFF_REQ		BIT(2)
#define CTRL1_SPM_CK_SEL0			BIT(4)
#define CTRL1_SPM_CK_SEL1			BIT(5)
#define CTRL1_TIMER_SET				BIT(6)
#define CTRL1_TIMER_CLR				BIT(7)
#define CTRL1_SPM_LEAVE_DEEPIDLE_REQ		BIT(8)
#define CTRL1_SPM_LEAVE_SUSPEND_REQ		BIT(9)
#define CTRL1_CSYSPWRUPACK			BIT(10)
#define CTRL1_SRCCLKENO0			BIT(11)
#define CTRL1_SRCCLKENO1			BIT(12)
#define CTRL1_SRCCLKENO2			BIT(13)
#define CTRL1_SPM_APSRC_INTERNAL_ACK		BIT(14)
#define CTRL1_SPM_EMI_INTERNAL_ACK		BIT(15)
#define CTRL1_SPM_DDREN_INTERNAL_ACK		BIT(16)
#define CTRL1_SPM_INFRA_INTERNAL_ACK		BIT(17)
#define CTRL1_SPM_VRF18_INTERNAL_ACK		BIT(18)
#define CTRL1_SPM_VCORE_INTERNAL_ACK		BIT(19)
#define CTRL1_SPM_VCORE_RESTORE_ACK		BIT(20)
#define CTRL1_SPM_PMIC_INTERNAL_ACK		BIT(21)
#define CTRL1_PMIC_IRQ_REQ_EN			BIT(22)
#define CTRL1_WDT_KICK_P			BIT(23)
#define CTRL1_FORCE_DDREN_WAKE			BIT(24)
#define CTRL1_FORCE_F26M_WAKE			BIT(25)
#define CTRL1_FORCE_APSRC_WAKE			BIT(26)
#define CTRL1_FORCE_INFRA_WAKE			BIT(27)
#define CTRL1_FORCE_VRF18_WAKE			BIT(28)
#define CTRL1_FORCE_VCORE_WAKE			BIT(29)
#define CTRL1_FORCE_EMI_WAKE			BIT(30)
#define CTRL1_FORCE_PMIC_WAKE			BIT(31)

/* MD32PCM_CTRL2 define (PCM_REG2_DATA) */
#define CTRL2_MD32PCM_IRQ_TRIG_BIT		BIT(31)

/* MD32PCM_STA0 define */
#define STA0_SRCCLKENI0				BIT(0)
#define STA0_SRCCLKENI1				BIT(1)
#define STA0_MD_SRCCLKENA			BIT(2)
#define STA0_MD_SRCCLKENA1			BIT(3)
#define STA0_MD_DDREN_REQ			BIT(4)
#define STA0_CONN_DDREN_REQ			BIT(5)
#define STA0_SSPM_SRCCLKENA			BIT(6)
#define STA0_SSPM_APSRC_REQ			BIT(7)
#define STA0_MD_STATE				BIT(8)
#define STA0_RC2SPM_SRCCLKENO_0_ACK		BIT(9)
#define STA0_MM_STATE				BIT(10)
#define STA0_SSPM_STATE				BIT(11)
#define STA0_CPUEB_STATE			BIT(12)
#define STA0_CONN_STATE				BIT(13)
#define STA0_CONN_VCORE_REQ			BIT(14)
#define STA0_CONN_SRCCLKENA			BIT(15)
#define STA0_CONN_SRCCLKENB			BIT(16)
#define STA0_CONN_APSRC_REQ			BIT(17)
#define STA0_SCP_STATE				BIT(18)
#define STA0_CSYSPWRUPREQ			BIT(19)
#define STA0_PWRAP_SLEEP_ACK			BIT(20)
#define STA0_DPM_STATE				BIT(21)
#define STA0_AUDIO_DSP_STATE			BIT(22)
#define STA0_PMIC_IRQ_ACK			BIT(23)
#define STA0_RESERVED_BIT_24			BIT(24)
#define STA0_RESERVED_BIT_25			BIT(25)
#define STA0_RESERVED_BIT_26			BIT(26)
#define STA0_DVFS_STATE				BIT(27)
#define STA0_RESERVED_BIT_28			BIT(28)
#define STA0_RESERVED_BIT_29			BIT(29)
#define STA0_SC_HW_S1_ACK_ALL			BIT(30)
#define STA0_DDREN_STATE			BIT(31)

#define R12_PCM_TIMER_B				BIT(0)
#define R12_TWAM_PMSR_DVFSRC_ALCO		BIT(1)
#define R12_KP_IRQ_B				BIT(2)
#define R12_APWDT_EVENT_B			BIT(3)
#define R12_APXGPT_EVENT_B			BIT(4)
#define R12_CONN2AP_WAKEUP_B			BIT(5)
#define R12_EINT_EVENT_B			BIT(6)
#define R12_CONN_WDT_IRQ_B			BIT(7)
#define R12_CCIF0_EVENT_B			BIT(8)
#define R12_CCIF1_EVENT_B			BIT(9)
#define R12_SSPM2SPM_WAKEUP_B			BIT(10)
#define R12_SCP2SPM_WAKEUP_B			BIT(11)
#define R12_ADSP2SPM_WAKEUP_B			BIT(12)
#define R12_PCM_WDT_WAKEUP_B			BIT(13)
#define R12_USB0_CDSC_B				BIT(14)
#define R12_USB0_POWERDWN_B			BIT(15)
#define R12_UART_EVENT_B			BIT(16)
#define R12_DEBUGTOP_FLAG_IRQ_B			BIT(17)
#define R12_SYS_TIMER_EVENT_B			BIT(18)
#define R12_EINT_EVENT_SECURE_B			BIT(19)
#define R12_AFE_IRQ_MCU_B			BIT(20)
#define R12_THERM_CTRL_EVENT_B			BIT(21)
#define R12_SYS_CIRQ_IRQ_B			BIT(22)
#define R12_PBUS_EVENT_B			BIT(23)
#define R12_CSYSPWREQ_B				BIT(24)
#define R12_MD_WDT_B				BIT(25)
#define R12_AP2AP_PEER_WAKEUP_B			BIT(26)
#define R12_SEJ_B				BIT(27)
#define R12_CPU_WAKEUP				BIT(28)
#define R12_APUSYS_WAKE_HOST_B			BIT(29)
#define R12_PCIE_WAKE_B				BIT(30)
#define R12_MSDC_WAKE_B				BIT(31)

/* PCM_PWR_IO_EN */
#define PCM_PWRIO_EN_R0		BIT(0)
#define PCM_PWRIO_EN_R7		BIT(7)
#define PCM_RF_SYNC_R0		BIT(16)
#define PCM_RF_SYNC_R6		BIT(22)
#define PCM_RF_SYNC_R7		BIT(23)

/* SPM_SWINT */
#define PCM_SW_INT0		BIT(0)
#define PCM_SW_INT1		BIT(1)
#define PCM_SW_INT2		BIT(2)
#define PCM_SW_INT3		BIT(3)
#define PCM_SW_INT4		BIT(4)
#define PCM_SW_INT5		BIT(5)
#define PCM_SW_INT6		BIT(6)
#define PCM_SW_INT7		BIT(7)
#define PCM_SW_INT8		BIT(8)
#define PCM_SW_INT9		BIT(9)

#define PCM_SW_INT_ALL		(PCM_SW_INT9 | PCM_SW_INT8 | PCM_SW_INT7 | \
				 PCM_SW_INT6 | PCM_SW_INT5 | PCM_SW_INT4 | \
				 PCM_SW_INT3 | PCM_SW_INT2 | PCM_SW_INT1 | \
				 PCM_SW_INT0)

/* SPM_AP_STANDBY_CON */
#define WFI_OP_AND		1
#define WFI_OP_OR		0

/* SPM_IRQ_MASK */
#define ISRM_TWAM		BIT(2)
#define ISRM_PCM_RETURN		BIT(3)
#define ISRM_RET_IRQ0		BIT(8)
#define ISRM_RET_IRQ1		BIT(9)
#define ISRM_RET_IRQ2		BIT(10)
#define ISRM_RET_IRQ3		BIT(11)
#define ISRM_RET_IRQ4		BIT(12)
#define ISRM_RET_IRQ5		BIT(13)
#define ISRM_RET_IRQ6		BIT(14)
#define ISRM_RET_IRQ7		BIT(15)
#define ISRM_RET_IRQ8		BIT(16)
#define ISRM_RET_IRQ9		BIT(17)
#define ISRM_RET_IRQ_AUX	((ISRM_RET_IRQ9) | (ISRM_RET_IRQ8) | \
				 (ISRM_RET_IRQ7) | (ISRM_RET_IRQ6) | \
				 (ISRM_RET_IRQ5) | (ISRM_RET_IRQ4) | \
				 (ISRM_RET_IRQ3) | (ISRM_RET_IRQ2) | \
				 (ISRM_RET_IRQ1))
#define ISRM_ALL_EXC_TWAM	(ISRM_RET_IRQ_AUX)
#define ISRM_ALL (ISRM_ALL_EXC_TWAM | ISRM_TWAM)

/* SPM_IRQ_STA */
#define ISRS_TWAM		BIT(2)
#define ISRS_PCM_RETURN		BIT(3)
#define ISRC_TWAM		ISRS_TWAM
#define ISRC_ALL_EXC_TWAM	ISRS_PCM_RETURN
#define ISRC_ALL		(ISRC_ALL_EXC_TWAM | ISRC_TWAM)

/* SPM_WAKEUP_MISC */
#define WAKE_MISC_GIC_WAKEUP			0x3FF /* bit0 ~ bit9 */
#define WAKE_MISC_DVFSRC_IRQ			DVFSRC_IRQ_LSB
#define WAKE_MISC_REG_CPU_WAKEUP		SPM_WAKEUP_MISC_REG_CPU_WAKEUP_LSB
#define WAKE_MISC_PCM_TIMER_EVENT		PCM_TIMER_EVENT_LSB
#define WAKE_MISC_PMIC_OUT_B			(BIT(19) | BIT(20))
#define WAKE_MISC_TWAM_IRQ_B			TWAM_IRQ_B_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_0		SPM_ACK_CHK_WAKEUP_0_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_1		SPM_ACK_CHK_WAKEUP_1_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_2		SPM_ACK_CHK_WAKEUP_2_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_3		SPM_ACK_CHK_WAKEUP_3_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL	SPM_ACK_CHK_WAKEUP_ALL_LSB
#define WAKE_MISC_PMIC_IRQ_ACK			PMIC_IRQ_ACK_LSB
#define WAKE_MISC_PMIC_SCP_IRQ			PMIC_SCP_IRQ_LSB

#define SPM_INTERNAL_STATUS_HW_S1		BIT(0)

/* Signal that monitor by HW CG  */
enum spm_hwcg_setting {
	HWCG_PWR,
	HWCG_PWR_MSB,
	HWCG_MODULE_BUSY,
	HWCG_SETTING_MAX
};

enum spm_hwcg_sta_type {
	HWCG_STA_DEFAULT_MASK,
	HWCG_STA_MASK
};

enum spm_peri_req_setting {
	PERI_REQ_EN  = 0,
	PERI_REQ_SETTING_MAX
};

enum spm_peri_req_sta_type {
	PERI_REQ_STA_DEFAULT_MASK,
	PERI_REQ_STA_MASK,
	PERI_REQ_STA_MAX
};

enum spm_peri_req_status {
	PERI_RES_REQ_EN,
	PERI_REQ_STATUS_MAX
};

enum spm_peri_req_status_raw {
	PERI_REQ_STATUS_RAW_NUM,
	PERI_REQ_STATUS_RAW_NAME,
	PERI_REQ_STATUS_RAW_STA,
	PERI_REQ_STATUS_RAW_MAX
};

#define MT_SPM_HW_CG_STA_INIT(_x)	({ if (_x) _x->sta = 0; })

struct spm_peri_req_sta {
	uint32_t sta;
};

struct spm_peri_req_info {
	uint32_t req_en;
	uint32_t req_sta;
};

struct spm_hwcg_sta {
	uint32_t sta;
};

void spm_hwreq_init(void);

/* Res:
 *	Please refer the mt_spm_resource_req.h.
 *	Section of SPM resource request internal bit_mask.
 */
void spm_hwcg_ctrl(uint32_t res, enum spm_hwcg_setting type,
		   uint32_t is_set, uint32_t val);

/* Idx:
 *	index of HWCG setting.
 */
void spm_hwcg_ctrl_by_index(uint32_t idx, enum spm_hwcg_setting type,
			    uint32_t is_set, uint32_t val);

/* Res:
 *	Please refer the mt_spm_resource_req.h.
 *	Section of SPM resource request internal bit_mask.
 */
int spm_hwcg_get_setting(uint32_t res, enum spm_hwcg_sta_type sta_type,
			 enum spm_hwcg_setting type,
			 struct spm_hwcg_sta *sta);

/* Idx:
 *	index of HWCG setting.
 */
int spm_hwcg_get_setting_by_index(uint32_t idx,
				  enum spm_hwcg_sta_type sta_type,
				  enum spm_hwcg_setting type,
				  struct spm_hwcg_sta *sta);

uint32_t spm_hwcg_get_status(uint32_t idx, enum spm_hwcg_setting type);

static inline uint32_t spm_hwcg_setting_num(void)
{
	return HWCG_SETTING_MAX;
}

uint32_t spm_peri_req_get_status(uint32_t idx, enum spm_peri_req_status type);
uint32_t spm_peri_req_get_status_raw(enum spm_peri_req_status_raw type,
				     uint32_t idx,
				     char *name, size_t sz);

static inline uint32_t spm_peri_req_setting_num(void)
{
	return PERI_REQ_SETTING_MAX;
}

int spm_peri_req_get_setting_by_index(uint32_t idx,
				      enum spm_peri_req_sta_type sta_type,
				      struct spm_peri_req_sta *sta);

void spm_peri_req_ctrl_by_index(uint32_t idx,
				uint32_t is_set, uint32_t val);

int spm_peri_req_name(uint32_t idex, char *name, size_t sz);

#ifdef __GNUC__
#define spm_likely(x)	__builtin_expect(!!(x), 1)
#define spm_unlikely(x)	__builtin_expect(!!(x), 0)
#else
#define spm_likely(x)	(x)
#define spm_unlikely(x)	(x)
#endif

/* AP_MDSRC_REQ MD 26M ON settle time (3ms) */
#define AP_MDSRC_REQ_MD_26M_SETTLE	3

/* Setting the SPM settle time*/
#define SPM_SYSCLK_SETTLE	0x60FE /* 1685us */

/* Setting the SPM req/ack time*/
#define SPM_ACK_TIMEOUT_US	1000

/* Settine the firmware status check for SPM PC */
#define SPM_PC_CHECKABLE

enum {
	SPM_ARGS_SPMFW_IDX_KICK = 0,
	SPM_ARGS_SPMFW_INIT,
	SPM_ARGS_SUSPEND,
	SPM_ARGS_SUSPEND_FINISH,
	SPM_ARGS_SODI,
	SPM_ARGS_SODI_FINISH,
	SPM_ARGS_DPIDLE,
	SPM_ARGS_DPIDLE_FINISH,
	SPM_ARGS_PCM_WDT,
	SPM_ARGS_SUSPEND_CALLBACK,
	SPM_ARGS_HARDWARE_CG_CHECK,
	SPM_ARGS_NUM,
};

typedef enum {
	WR_NONE = 0,
	WR_UART_BUSY,
	WR_ABORT,
	WR_PCM_TIMER,
	WR_WAKE_SRC,
	WR_DVFSRC,
	WR_TWAM,
	WR_PMSR,
	WR_SPM_ACK_CHK,
	WR_UNKNOWN,
} wake_reason_t;

struct pwr_ctrl;

void spm_set_irq_num(uint32_t num);
struct mt_lp_resource_user *get_spm_res_user(void);
int spm_boot_init(void);
void spm_dvfsfw_init(uint64_t boot_up_opp, uint64_t dram_issue);
/* Support by bl31_plat_setup.c */
uint32_t is_abnormal_boot(void);

#endif
