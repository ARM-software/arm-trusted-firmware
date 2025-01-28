/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_API_H
#define MT_LP_API_H

#include <lpm_v2/mt_lp_rm.h>

#if MTK_PUBEVENT_ENABLE
#include <vendor_pubsub_events.h>
#endif

/* UFS clk enum for PLAT_RC_CLKBUF_STATUS */
enum rc_update_ex_ufs_ref_clk {
	UFS_REF_CLK_OFF = 0,
	UFS_REF_CLK_ON,
};

/* Enum for flight mode  */
enum rc_update_ex_flight_mode {
	FLIGHT_MODE_OFF = 0,
	FLIGHT_MODE_ON,
};

struct mt_lpm_pubevent_data {
	unsigned int u32;
};

enum mt_lpm_pubevents_id {
	MT_LPM_PUBEVENTS_BBLPM_ENTER,
	MT_LPM_PUBEVENTS_BBLPM_LEAVE,
	MT_LPM_PUBEVENTS_TARGET_CORE,
	MT_LPM_PUBEVENTS_SYS_POWER_OFF,
	MT_LPM_PUBEVENTS_SYS_POWER_ON,
};

struct mt_lp_publish_event {
	unsigned int id;
	struct mt_lpm_pubevent_data val;
	unsigned int level;
};

#if MTK_PUBEVENT_ENABLE
#define MT_LP_PUBLISH_EVENT(x) ({ \
	PUBLISH_EVENT_ARG(lpm_publish_event, (const void *)(x)); })
#define MT_LP_SUSPEND_PUBLISH_EVENT(x) ({ \
	PUBLISH_EVENT_ARG(suspend_publish_event, (const void *)(x)); })

#define MT_LP_SUBSCRIBE_SUSPEND(func) \
	SUBSCRIBE_TO_EVENT(suspend_publish_event, func)
#define MT_LP_SUBSCRIBE_LPM(func) \
	SUBSCRIBE_TO_EVENT(lpm_publish_event, func)
#else
#define MT_LP_PUBLISH_EVENT(x)	({ (void)x; })
#define MT_LP_SUSPEND_PUBLISH_EVENT(x)	({ (void)x; })
#define MT_LP_SUBSCRIBE_SUSPEND(func)
#define MT_LP_SUBSCRIBE_LPM(func)
#endif

/* MTK low power API types for audio */
enum mt_lp_api_audio_type {
	AUDIO_AFE_ENTER,
	AUDIO_AFE_LEAVE,
	AUDIO_DSP_ENTER,
	AUDIO_DSP_LEAVE,
};

/* MTK low power API types for usb */
enum mt_lp_api_usb_type {
	LPM_USB_ENTER,
	LPM_USB_LEAVE,
	USB_HEADSET_ENTER,
	USB_HEADSET_LEAVE,
};

int mt_audio_update(int type);
int mt_usb_update(int type);

/* MTK Low Power Scenario Types for logging */
enum mtk_lp_scenario_status {
	AUDIO_AFE,
	AUDIO_DSP,
	USB_HEADSET,
	MTK_LP_SCENE_NUM,
};

/* MTK Low Power API Types for CCCI */
enum mt_lp_api_ccci_type {
	CCCI_AP_MDSRC_REQUEST,
	CCCI_AP_MDSRC_RELEASE,
	CCCI_AP_MDSRC_ACK,
	CCCI_AP_MDSRC_GET_SETTLE,
	CCCI_AP_IS_MD_SLEEP,
};

/* System power level */
#define MT_LP_SYSPOWER_LEVEL_APMCU	BIT(0)
#define MT_LP_SYSPOWER_LEVEL_DRAM	BIT(1)
#define MT_LP_SYSPOWER_LEVEL_SYSPLL	BIT(2)
#define MT_LP_SYSPOWER_LEVEL_PMIC_LP	BIT(3)
#define MT_LP_SYSPOWER_LEVEL_BUS26M	BIT(4)
#define MT_LP_SYSPOWER_LEVEL_VCORE0V	BIT(5)
#define MT_LP_SYSPOWER_LEVEL_SUSPEND	BIT(6)


enum mt_lpm_pubevent_wake_src {
	MT_LPM_WAKE_MD_WAKEUP_CCIF0 = 1,
	MT_LPM_WAKE_MD_WAKEUP_CCIF1,
	MT_LPM_WAKE_MD_WAKEUP_CLDMA,
	MT_LPM_WAKE_MD_WAKEUP_DPMAIF,
	MT_LPM_WAKE_MD_WDT,
};

/* MTK Low Power API Types for GPUEB */
enum mt_lp_api_gpueb_type {
	GPUEB_PLL_EN,
	GPUEB_PLL_DIS,
	GPUEB_GET_PWR_STATUS,
	GPUEB_GET_MFG0_PWR_CON,
};

int mt_ccci_hwctrl(int type, void *priv);
int mt_gpueb_hwctrl(int type, void *priv);
uint64_t mt_get_lp_scenario_status(void);

#endif /* MT_LP_API_H */
