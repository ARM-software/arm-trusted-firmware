/*
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NRD_SDEI_H
#define NRD_SDEI_H

#if SDEI_SUPPORT

/* ARM SDEI dynamic shared event numbers */
#define NRD_SDEI_DS_EVENT_0		U(804)
#define NRD_SDEI_DS_EVENT_1		U(805)

#define PLAT_ARM_PRIVATE_SDEI_EVENTS					      \
		SDEI_DEFINE_EVENT_0(ARM_SDEI_SGI),			      \
		SDEI_EXPLICIT_EVENT(NRD_SDEI_DS_EVENT_0, SDEI_MAPF_CRITICAL), \
		SDEI_EXPLICIT_EVENT(NRD_SDEI_DS_EVENT_1, SDEI_MAPF_CRITICAL),

#define PLAT_ARM_SHARED_SDEI_EVENTS

#endif /* SDEI_SUPPORT */

#endif /* NRD_SDEI_H */
