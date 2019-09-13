/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAUTH_H
#define PAUTH_H

/*******************************************************************************
 * ARMv8.3-PAuth support functions
 ******************************************************************************/

/* Disable ARMv8.3 pointer authentication in EL1/EL3 */
void pauth_disable_el1(void);
void pauth_disable_el3(void);

#endif /* PAUTH_H */
