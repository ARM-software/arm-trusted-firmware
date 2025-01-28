/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_SHUTDOWN_CFG_H
#define PMIC_SHUTDOWN_CFG_H

#pragma weak spmi_shutdown
int spmi_shutdown(void);
int pmic_shutdown_cfg(void);

#endif /* PMIC_SHUTDOWN_CFG_H */
