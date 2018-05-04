/*
 * Texas Instruments System Control Interface API
 *   Based on Linux and U-Boot implementation
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TI_SCI_H
#define __TI_SCI_H

/**
 * ti_sci_init() - Basic initialization
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
int ti_sci_init(void);

#endif	/* __TI_SCI_H */
