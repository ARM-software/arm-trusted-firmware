/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SIPSVC_H
#define SIPSVC_H

#include <stdint.h>

#define SMC_FUNC_MASK			0x0000ffff
#define SMC32_PARAM_MASK		0xffffffff

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT		0xff00
#define SIP_SVC_UID			0xff01
#define SIP_SVC_VERSION			0xff03
#define SIP_SVC_PRNG			0xff10
#define SIP_SVC_RNG			0xff11
#define SIP_SVC_MEM_BANK		0xff12
#define SIP_SVC_PREFETCH_DIS		0xff13
#define SIP_SVC_HUK			0xff14
#define SIP_SVC_ALLOW_L1L2_ERR		0xff15
#define SIP_SVC_ALLOW_L2_CLR		0xff16
#define SIP_SVC_2_AARCH32		0xff17
#define SIP_SVC_PORSR1			0xff18

/* Layerscape SiP Service Calls version numbers */
#define LS_SIP_SVC_VERSION_MAJOR	0x0
#define LS_SIP_SVC_VERSION_MINOR	0x1

/* Number of Layerscape SiP Calls implemented */
#define LS_COMMON_SIP_NUM_CALLS		10

/* Parameter Type Constants */
#define SIP_PARAM_TYPE_NONE		0x0
#define SIP_PARAM_TYPE_VALUE_INPUT	0x1
#define SIP_PARAM_TYPE_VALUE_OUTPUT	0x2
#define SIP_PARAM_TYPE_VALUE_INOUT	0x3
#define SIP_PARAM_TYPE_MEMREF_INPUT	0x5
#define SIP_PARAM_TYPE_MEMREF_OUTPUT	0x6
#define SIP_PARAM_TYPE_MEMREF_INOUT	0x7

#define SIP_PARAM_TYPE_MASK		0xF

/*
 * The macro SIP_PARAM_TYPES can be used to construct a value that you can
 * compare against an incoming paramTypes to check the type of all the
 * parameters in one comparison.
 */
#define SIP_PARAM_TYPES(t0, t1, t2, t3) \
		((t0) | ((t1) << 4) | ((t2) << 8) | ((t3) << 12))

/*
 * The macro SIP_PARAM_TYPE_GET can be used to extract the type of a given
 * parameter from paramTypes if you need more fine-grained type checking.
 */
#define SIP_PARAM_TYPE_GET(t, i)	((((uint32_t)(t)) >> ((i) * 4)) & 0xF)

/*
 * The macro SIP_PARAM_TYPE_SET can be used to load the type of a given
 * parameter from paramTypes without specifying all types (SIP_PARAM_TYPES)
 */
#define SIP_PARAM_TYPE_SET(t, i)	(((uint32_t)(t) & 0xF) << ((i) * 4))

#define SIP_SVC_RNG_PARAMS		(SIP_PARAM_TYPE_VALUE_INPUT, \
					 SIP_PARAM_TYPE_MEMREF_OUTPUT, \
					 SIP_PARAM_TYPE_NONE, \
					 SIP_PARAM_TYPE_NONE)

/* Layerscape SiP Calls error code */
enum {
	LS_SIP_SUCCESS = 0,
	LS_SIP_INVALID_PARAM = -1,
	LS_SIP_NOT_SUPPORTED = -2,
};

#endif /* SIPSVC_H */
