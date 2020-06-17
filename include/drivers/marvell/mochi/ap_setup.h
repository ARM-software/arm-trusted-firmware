/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* AP8xx Marvell SoC driver */

#ifndef AP_SETUP_H
#define AP_SETUP_H

void ap_init(void);
void ap_ble_init(void);
int ap_get_count(void);
void update_cp110_default_win(int cp_id);

#endif /* AP_SETUP_H */
