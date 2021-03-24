/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_WARM_RST_H
#define PLAT_WARM_RST_H

#ifndef NXP_COINED_BB
#define ERLY_WRM_RST_FLG_FLSH_UPDT	0
#endif

#ifndef __ASSEMBLER__

#if defined(IMAGE_BL2)
uint32_t is_warm_boot(void);
#endif

#if defined(IMAGE_BL31)
int prep_n_execute_warm_reset(void);
int _soc_sys_warm_reset(void);
#endif

#endif	/* __ASSEMBLER__ */

#endif	/* PLAT_WARM_RST_H */
