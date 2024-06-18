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

#define SCMI_VENDOR	"AMD"
#define SCMI_PRODUCT	"Versal Gen 2"

#endif /* DEF_H */
