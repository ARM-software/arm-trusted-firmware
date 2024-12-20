/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_HWREQ_H
#define MT_SPM_HWREQ_H

#include <drivers/spm/mt_spm_resource_req.h>

/* Resource requirement which HW CG support */
enum {
	HWCG_DDREN = 0,
	HWCG_VRF18,
	HWCG_INFRA,
	HWCG_PMIC,
	HWCG_F26M,
	HWCG_VCORE,
	HWCG_MAX
};

/* Signal that monitor by HW CG  */
enum spm_hwcg_setting {
	HWCG_PWR,
	HWCG_PWR_MSB,
	HWCG_MODULE_BUSY,
	HWCG_SETTING_MAX
};

enum spm_pwr_status {
	HWCG_PWR_MD1 = 0,
	HWCG_PWR_CONN,
	HWCG_PWR_APIFR_IO,
	HWCG_PWR_APIFR_MEM,
	HWCG_PWR_PERI,
	HWCG_PWR_PERI_ETHER,
	HWCG_PWR_SSUSB_PD_PHY_P0,
	HWCG_PWR_SSUSB_P0,
	HWCG_PWR_SSUSB_P1,
	HWCG_PWR_SSUSB_P23,
	HWCG_PWR_SSUSB_PHY_P2,
	HWCG_PWR_UFS0,
	HWCG_PWR_UFS0_PHY,
	HWCG_PWR_PEXTP_MAC0,
	HWCG_PWR_PEXTP_MAC1,
	HWCG_PWR_PEXTP_MAC2,
	HWCG_PWR_PEXTP_PHY0,
	HWCG_PWR_PEXTP_PHY1,
	HWCG_PWR_PEXTP_PHY3,
	HWCG_PWR_AUDIO,
	HWCG_PWR_ADSP_CORE1,
	HWCG_PWR_ADSP_TOP,
	HWCG_PWR_ADSP_INFRA,
	HWCG_PWR_ADSP_AO,
	HWCG_PWR_MM_PROC,
	HWCG_PWR_SCP,
	HWCG_PWR_SCP2,
	HWCG_PWR_DPYD0,
	HWCG_PWR_DPYD1,
	HWCG_PWR_DPYD2,
	HWCG_PWR_DPYD3,
	HWCG_PWR_DPYA0
};

CASSERT(HWCG_PWR_SSUSB_P1 == 8, spm_pwr_status_err);
CASSERT(HWCG_PWR_PEXTP_PHY0 == 16, spm_pwr_status_err);
CASSERT(HWCG_PWR_MM_PROC == 24, spm_pwr_status_err);

enum spm_hwcg_module_busy {
	HWCG_MODULE_ADSP = 0,
	HWCG_MODULE_MMPLL,
	HWCG_MODULE_TVDPLL,
	HWCG_MODULE_MSDCPLL,
	HWCG_MODULE_UNIVPLL
};

enum spm_hwcg_sta_type {
	HWCG_STA_DEFAULT_MASK,
	HWCG_STA_MASK
};

/* Signal that monitor by HW CG  */
enum spm_peri_req_setting {
	PERI_REQ_EN  = 0,
	PERI_REQ_SETTING_MAX
};

/* Resource requirement which PERI REQ support */
enum spm_peri_req {
	PERI_REQ_F26M = 0,
	PERI_REQ_INFRA,
	PERI_REQ_SYSPLL,
	PERI_REQ_APSRC,
	PERI_REQ_EMI,
	PERI_REQ_DDREN,
	PERI_REQ_MAX
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

enum spm_peri_req_en {
	PERI_REQ_EN_FLASHIF = 0,
	PERI_REQ_EN_AP_DMA,
	PERI_REQ_EN_UART0,
	PERI_REQ_EN_UART1,
	PERI_REQ_EN_UART2,
	PERI_REQ_EN_UART3,
	PERI_REQ_EN_UART4,
	PERI_REQ_EN_UART5,
	PERI_REQ_EN_PWM,
	PERI_REQ_EN_SPI0,
	PERI_REQ_EN_SPI0_INCR16,
	PERI_REQ_EN_SPI1,
	PERI_REQ_EN_SPI2,
	PERI_REQ_EN_SPI3,
	PERI_REQ_EN_SPI4,
	PERI_REQ_EN_SPI5,
	PERI_REQ_EN_SPI6,
	PERI_REQ_EN_SPI7,
	PERI_REQ_EN_IMP_IIC,
	PERI_REQ_EN_MSDC1,
	PERI_REQ_EN_MSDC2,
	PERI_REQ_EN_USB,
	PERI_REQ_EN_UFS0,
	PERI_REQ_EN_PEXTP1,
	PERI_REQ_EN_PEXTP0,
	PERI_REQ_EN_RSV_DUMMY0,
	PERI_REQ_EN_PERI_BUS_TRAFFIC,
	PERI_REQ_EN_RSV_DUMMY1,
	PERI_REQ_EN_RSV_FOR_MSDC,
	PERI_REQ_EN_MAX
};

CASSERT(PERI_REQ_EN_PWM == 8, spm_peri_req_en_err);
CASSERT(PERI_REQ_EN_SPI6 == 16, spm_peri_req_en_err);
CASSERT(PERI_REQ_EN_PEXTP0 == 24, spm_peri_req_en_err);

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

#define MT_SPM_HW_CG_STA_INIT(_x)	({ if (_x) _x->sta = 0; })

#define INFRA_AO_OFFSET(offset)		(INFRACFG_AO_BASE + offset)
#define INFRA_SW_CG_MASK		INFRA_AO_OFFSET(0x060)

#define REG_PERI_REQ_EN(N)	(PERICFG_AO_BASE + 0x070 + 0x4 * (N))
#define REG_PERI_REQ_STA(N)	(PERICFG_AO_BASE + 0x0A0 + 0x4 * (N))

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

int spm_hwcg_name(uint32_t idex, char *name, size_t sz);

static inline uint32_t spm_hwcg_num(void)
{
	return HWCG_MAX;
}

static inline uint32_t spm_hwcg_setting_num(void)
{
	return HWCG_SETTING_MAX;
}

uint32_t spm_peri_req_get_status(uint32_t idx, enum spm_peri_req_status type);
uint32_t spm_peri_req_get_status_raw(enum spm_peri_req_status_raw type,
				     uint32_t idx,
				     char *name, size_t sz);

static inline uint32_t spm_peri_req_num(void)
{
	return PERI_REQ_MAX;
}

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

#endif /* MT_SPM_HWREQ_H */
