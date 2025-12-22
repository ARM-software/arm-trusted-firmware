/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_BOOKER_H
#define PLATFORM_BOOKER_H

#define BOOKER_MAP_REGION	0xA000000
#define BOOKER_MAP_SIZE		0x2000000

static const uintptr_t booker_mtsx_bases[] = {
	0x0a790000, /* EMI10 CHN0 */
	0x0af90000, /* EMI10 CHN1 */
	0x0b790000, /* EMI11 CHN0 */
	0x0bf90000, /* EMI11 CHN1 */
};

#endif /* PLATFORM_BOOKER_H*/
