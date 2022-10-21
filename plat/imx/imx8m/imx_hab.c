/*
 * Copyright 2017-2020 NXP
 * Copyright 2022 Leica Geosystems AG
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/runtime_svc.h>
#include <imx_sip_svc.h>

#define HAB_CID_ATF	U(2)	/* TF-A Caller ID */

/* HAB Status definitions */
enum hab_status {
	HAB_STS_ANY = 0x00,	/* Match any status in report_event() */
	HAB_FAILURE = 0x33,	/* Operation failed */
	HAB_WARNING = 0x69,	/* Operation completed with warning */
	HAB_SUCCESS = 0xf0	/* Operation completed successfully */
};

/* HAB Configuration definitions */
enum hab_config {
	HAB_CFG_RETURN = 0x33,	/* Field Return IC */
	HAB_CFG_OPEN   = 0xf0,	/* Non-secure IC */
	HAB_CFG_CLOSED = 0xcc	/* Secure IC */
};

/* HAB State definitions */
enum hab_state {
	HAB_STATE_INITIAL   = 0x33,	/* Initializing state (transitory) */
	HAB_STATE_CHECK     = 0x55,	/* Check state (non-secure) */
	HAB_STATE_NONSECURE = 0x66,	/* Non-secure state */
	HAB_STATE_TRUSTED   = 0x99,	/* Trusted state */
	HAB_STATE_SECURE    = 0xaa,	/* Secure state */
	HAB_STATE_FAIL_SOFT = 0xcc,	/* Soft fail state */
	HAB_STATE_FAIL_HARD = 0xff,	/* Hard fail state (terminal) */
	HAB_STATE_NONE      = 0xf0	/* No security state machine */
};

/* HAB Verification Target definitions */
enum hab_target {
	HAB_TGT_MEMORY     = 0x0f,	/* Check memory allowed list */
	HAB_TGT_PERIPHERAL = 0xf0,	/* Check peripheral allowed list */
	HAB_TGT_ANY        = 0x55	/* Check memory & peripheral allowed list */
};

/* Authenticate Image Loader Callback prototype */
typedef enum hab_status hab_loader_callback_f_t(void **, size_t *, const void *);

/*
 * HAB Rom VectorTable (RVT) structure.
 * This table provides function pointers into the HAB library in ROM for
 * use by post-ROM boot sequence components.
 * Functions are ordered in the structure below based on the offsets in ROM
 * image, and shall not be changed!
 * Details on API allocation offsets and function description could be
 * found in following documents from NXP:
 * - High Assurance Boot Version 4 Application Programming Interface
 *   Reference Manual (available in CST package)
 * - HABv4 RVT Guidelines and Recommendations (AN12263)
 */
struct hab_rvt_api {
	uint64_t	hdr;
	enum hab_status (*entry)(void);
	enum hab_status (*exit)(void);
	enum hab_status (*check_target)(enum hab_target type, const void *start, size_t bytes);
	void* (*authenticate_image)(uint8_t cid, long ivt_offset, void **start,
		size_t *bytes, hab_loader_callback_f_t loader);
	enum hab_status (*run_dcd)(const uint8_t *dcd);
	enum hab_status (*run_csf)(const uint8_t *csf, uint8_t cid, uint32_t srkmask);
	enum hab_status (*assert)(long type, const void *data, uint32_t count);
	enum hab_status (*report_event)(enum hab_status status, uint32_t index,
		uint8_t *event, size_t *bytes);
	enum hab_status (*report_status)(enum hab_config *config, enum hab_state *state);
	void (*failsafe)(void);
	void* (*authenticate_image_no_dcd)(uint8_t cid, long ivt_offset, void **start,
		size_t *bytes, hab_loader_callback_f_t loader);
	uint32_t (*get_version)(void);
	enum hab_status (*authenticate_container)(uint8_t cid, long ivt_offset, void **start,
		size_t *bytes, hab_loader_callback_f_t loader, uint32_t srkmask, int skip_dcd);
};

struct hab_rvt_api *g_hab_rvt_api = (struct hab_rvt_api *)HAB_RVT_BASE;

/*******************************************************************************
 * Handler for servicing HAB SMC calls
 ******************************************************************************/
int imx_hab_handler(uint32_t smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4)
{
	switch (x1) {
	case IMX_SIP_HAB_ENTRY:
		return g_hab_rvt_api->entry();
	case IMX_SIP_HAB_EXIT:
		return g_hab_rvt_api->exit();
	case IMX_SIP_HAB_CHECK_TARGET:
		return g_hab_rvt_api->check_target((enum hab_target)x2,
			(const void *)x3, (size_t)x4);
	case IMX_SIP_HAB_AUTH_IMG:
		return (unsigned long)g_hab_rvt_api->authenticate_image(HAB_CID_ATF,
			x2, (void **)x3, (size_t *)x4, NULL);
	case IMX_SIP_HAB_REPORT_EVENT:
		return g_hab_rvt_api->report_event(HAB_FAILURE,
			(uint32_t)x2, (uint8_t *)x3, (size_t *)x4);
	case IMX_SIP_HAB_REPORT_STATUS:
		return g_hab_rvt_api->report_status((enum hab_config *)x2,
			(enum hab_state *)x3);
	case IMX_SIP_HAB_FAILSAFE:
		g_hab_rvt_api->failsafe();
		break;
	case IMX_SIP_HAB_AUTH_IMG_NO_DCD:
		return (unsigned long)g_hab_rvt_api->authenticate_image_no_dcd(
			HAB_CID_ATF, x2, (void **)x3, (size_t *)x4, NULL);
	case IMX_SIP_HAB_GET_VERSION:
		return g_hab_rvt_api->get_version();
	default:
		return SMC_UNK;
	};

	return SMC_OK;
}
