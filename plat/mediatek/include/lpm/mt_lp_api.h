/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_API_H
#define MT_LP_API_H

#include <lpm/mt_lp_rm.h>

#if MTK_PUBEVENT_ENABLE
#include <vendor_pubsub_events.h>
#endif

/* Ufs clk enum for PLAT_RC_CLKBUF_STATUS */
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
};

#if MTK_PUBEVENT_ENABLE
#define MT_LP_PUBLISH_EVENT(x) ({\
	PUBLISH_EVENT_ARG(lpm_publish_event, (const void *)(x)); })
#define MT_LP_SUSPEND_PUBLISH_EVENT(x) ({\
	PUBLISH_EVENT_ARG(suspend_publish_event, (const void *)(x)); })

#define MT_LP_SUBSCRIBE_SUSPEND(func)	SUBSCRIBE_TO_EVENT(suspend_publish_event, func)
#define MT_LP_SUBSCRIBE_LPM(func)	SUBSCRIBE_TO_EVENT(lpm_publish_event, func)
#else
#define MT_LP_PUBLISH_EVENT(x) ({ (void)x; })
#define MT_LP_SUSPEND_PUBLISH_EVENT(x) ({ (void)x; })
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
};

int mt_audio_update(int type);
int mtk_usb_update(int type);

#endif /* MT_LP_API_H */
