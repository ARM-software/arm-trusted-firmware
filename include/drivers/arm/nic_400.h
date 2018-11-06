/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NIC_400_H
#define NIC_400_H

/*
 * Address of slave 'n' security setting in the NIC-400 address region
 * control
 */
#define NIC400_ADDR_CTRL_SECURITY_REG(n)	(0x8 + (n) * 4)

#endif /* NIC_400_H */
