/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOKER_H
#define BOOKER_H

#define MTU_AUX_CTL		0x0A08
#define TC_NO_FILL_MODE_BIT	0x1

#define MTU_TC_FLUSH_PR		0x0A30
#define TC_FLUSH_ENABLE_BIT	0x0

#define MTU_TC_FLUSH_SR		0x0A38
#define TC_FLUSH_COMPLETE_BIT	0x0

void booker_flush(void);

#endif /* BOOKER_H*/
