/*
 * Copyright (C) 2017-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef _DT_BINDINGS_STM32_ETZPC_H
#define _DT_BINDINGS_STM32_ETZPC_H

/* DECPROT modes */
#define DECPROT_S_RW		0x0
#define DECPROT_NS_R_S_W	0x1
#define DECPROT_MCU_ISOLATION	0x2
#define DECPROT_NS_RW		0x3

/* DECPROT lock */
#define DECPROT_UNLOCK		0x0
#define DECPROT_LOCK		0x1

#endif /* _DT_BINDINGS_STM32_ETZPC_H */
