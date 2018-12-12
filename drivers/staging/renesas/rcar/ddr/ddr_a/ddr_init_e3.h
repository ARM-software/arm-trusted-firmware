/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_INIT_E3_H
#define DDR_INIT_E3_H

#include <stdint.h>

#define RCAR_E3_DDR_VERSION    "rev.0.11"

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

#endif /* DDR_INIT_E3_H */
