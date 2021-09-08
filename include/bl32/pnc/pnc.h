/*
 * Copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PNC_H__
#define __PNC_H__

#define SMC_YIELD		0xbf000000
#define SMC_ACTION_FROM_S	0xbf000001
#define SMC_GET_SHAREDMEM	0xbf000002
#define SMC_CONFIG_SHAREDMEM	0xbf000003
#define SMC_ACTION_FROM_NS	0xbf000004

#ifndef __ASSEMBLER__

#include <stdint.h>

void *pncd_context_switch_to(unsigned long security_state);
int plat_pncd_setup(void);
uintptr_t plat_pncd_smc_handler(uint32_t smc_fid, u_register_t x1,
				u_register_t x2, u_register_t x3,
				u_register_t x4, void *cookie, void *handle,
				u_register_t flags);

#endif /* __ASSEMBLER__ */

#endif /* __PNC_H__ */
