/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AVS_DRIVER_H
#define AVS_DRIVER_H

/* AVS Setting. 1:enable / 0:disable */
#ifndef AVS_SETTING_ENABLE
#define AVS_SETTING_ENABLE	1
#endif /* AVS_SETTING_ENABLE */

void rcar_avs_init(void);
void rcar_avs_setting(void);
void rcar_avs_end(void);

#endif /* AVS_DRIVER_H */
