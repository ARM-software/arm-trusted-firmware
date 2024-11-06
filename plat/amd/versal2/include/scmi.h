/*
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCMI_H
#define SCMI_H

#include "versal2-scmi.h"

#define SIP_SCMI	(0xC200ffffU)
#define SMT_BUFFER_BASE	0x7fffe000

void init_scmi_server(void);

size_t plat_scmi_pd_count(unsigned int agent_id);
const char *plat_scmi_pd_get_name(unsigned int agent_id, unsigned int pd_id);
unsigned int plat_scmi_pd_statistics(unsigned int agent_id, unsigned long *pd_id);
unsigned int plat_scmi_pd_get_attributes(unsigned int agent_id, unsigned int pd_id);
unsigned int plat_scmi_pd_get_state(unsigned int agent_id, unsigned int pd_id);
int32_t plat_scmi_pd_set_state(unsigned int agent_id, unsigned int flags, unsigned int pd_id,
				unsigned int state);

#define SCMI_VENDOR	"AMD"
#define SCMI_PRODUCT	"Versal Gen 2"

#endif /* DEF_H */
