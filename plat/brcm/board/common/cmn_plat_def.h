/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CMN_PLAT_DEF_H
#define CMN_PLAT_DEF_H

#include <bcm_elog.h>
#include <platform_def.h>

#ifndef GET_LOG_LEVEL
#define GET_LOG_LEVEL() LOG_LEVEL
#endif

#ifndef SET_LOG_LEVEL
#define SET_LOG_LEVEL(x) ((void)(x))
#endif

#define PLAT_LOG_NOTICE(...)						 \
		do {							 \
			if (GET_LOG_LEVEL() >= LOG_LEVEL_NOTICE) {	 \
				bcm_elog(LOG_MARKER_NOTICE __VA_ARGS__); \
				tf_log(LOG_MARKER_NOTICE __VA_ARGS__);	 \
			}						 \
		} while (0)

#define PLAT_LOG_ERROR(...)						 \
		do {							 \
			if (GET_LOG_LEVEL() >= LOG_LEVEL_ERROR) {	 \
				bcm_elog(LOG_MARKER_ERROR, __VA_ARGS__); \
				tf_log(LOG_MARKER_ERROR __VA_ARGS__);	 \
			}						 \
		} while (0)

#define PLAT_LOG_WARN(...)						 \
		do {							 \
			if (GET_LOG_LEVEL() >= LOG_LEVEL_WARNING) {	 \
				bcm_elog(LOG_MARKER_WARNING, __VA_ARGS__);\
				tf_log(LOG_MARKER_WARNING __VA_ARGS__);	 \
			}						 \
		} while (0)

#define PLAT_LOG_INFO(...)						 \
		do {							 \
			if (GET_LOG_LEVEL() >= LOG_LEVEL_INFO) {	 \
				bcm_elog(LOG_MARKER_INFO __VA_ARGS__);	 \
				tf_log(LOG_MARKER_INFO  __VA_ARGS__);    \
			}						 \
		} while (0)

#define PLAT_LOG_VERBOSE(...)						 \
		do {							 \
			if (GET_LOG_LEVEL()  >= LOG_LEVEL_VERBOSE) {	 \
				bcm_elog(LOG_MARKER_VERBOSE __VA_ARGS__);\
				tf_log(LOG_MARKER_VERBOSE __VA_ARGS__);	 \
			}						 \
		} while (0)

/*
 * The number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#if USE_COHERENT_MEM
#define CMN_BL_REGIONS	3
#else
#define CMN_BL_REGIONS	2
#endif

/*
 * FIP definitions
 */
#define PLAT_FIP_ATTEMPT_OFFSET		0x20000
#define PLAT_FIP_NUM_ATTEMPTS		128

#define PLAT_BRCM_FIP_QSPI_BASE		QSPI_BASE_ADDR
#define PLAT_BRCM_FIP_NAND_BASE		NAND_BASE_ADDR
#define PLAT_BRCM_FIP_MAX_SIZE		0x01000000

#define PLAT_BRCM_FIP_BASE	PLAT_BRCM_FIP_QSPI_BASE
#endif
