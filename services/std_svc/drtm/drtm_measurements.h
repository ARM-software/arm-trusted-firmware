/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_MEASUREMENTS_H
#define DRTM_MEASUREMENTS_H

#include <stdint.h>

#include "drtm_main.h"
#include <platform_def.h>

#define DRTM_EVENT_ARM_BASE		0x9000U
#define DRTM_EVENT_TYPE(n)		(DRTM_EVENT_ARM_BASE + (unsigned int)(n))

#define DRTM_EVENT_ARM_PCR_SCHEMA	DRTM_EVENT_TYPE(1)
#define DRTM_EVENT_ARM_DCE		DRTM_EVENT_TYPE(2)
#define DRTM_EVENT_ARM_DCE_PUBKEY	DRTM_EVENT_TYPE(3)
#define DRTM_EVENT_ARM_DLME		DRTM_EVENT_TYPE(4)
#define DRTM_EVENT_ARM_DLME_EP		DRTM_EVENT_TYPE(5)
#define DRTM_EVENT_ARM_DEBUG_CONFIG	DRTM_EVENT_TYPE(6)
#define DRTM_EVENT_ARM_NONSECURE_CONFIG	DRTM_EVENT_TYPE(7)
#define DRTM_EVENT_ARM_DCE_SECONDARY	DRTM_EVENT_TYPE(8)
#define DRTM_EVENT_ARM_TZFW		DRTM_EVENT_TYPE(9)
#define DRTM_EVENT_ARM_SEPARATOR	DRTM_EVENT_TYPE(10)

#define CHECK_RC(rc, func_call) { \
	if (rc != 0) { \
		ERROR("%s(): " #func_call "failed unexpectedly rc=%d\n",  \
		      __func__, rc);  \
		panic();  \
	}  \
}

enum drtm_retc drtm_take_measurements(const struct_drtm_dl_args *a);
void drtm_serialise_event_log(uint8_t *dst, size_t *event_log_size_out);

#endif /* DRTM_MEASUREMENTS_H */
