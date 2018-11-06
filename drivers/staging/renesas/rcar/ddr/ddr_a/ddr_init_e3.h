/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once
#include <stdint.h>

#ifndef	__DDR_INIT_E3_
#define	__DDR_INIT_E3_

#define RCAR_E3_DDR_VERSION    "rev.0.09"

#ifdef ddr_qos_init_setting
   #define REFRESH_RATE  3900               /*  Average periodic refresh interval[ns]. Support 3900,7800 */
#else
   #if RCAR_REF_INT == 0
      #define REFRESH_RATE  3900
   #elif RCAR_REF_INT == 1
      #define REFRESH_RATE  7800
   #else
      #define REFRESH_RATE  3900
   #endif
#endif

extern int32_t rcar_dram_init(void);
#define INITDRAM_OK (0)
#define INITDRAM_NG (0xffffffff)
#define INITDRAM_ERR_I (0xffffffff)
#define INITDRAM_ERR_O (0xfffffffe)
#define INITDRAM_ERR_T (0xfffffff0)

#endif /* __DDR_INIT_E3_ */
