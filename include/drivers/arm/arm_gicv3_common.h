/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ARM_GICV3_COMMON_H__
#define __ARM_GICV3_COMMON_H__

/*******************************************************************************
 * GIC500/GIC600 Re-distributor interface registers & constants
 ******************************************************************************/

/* GICR_WAKER implementation-defined bit definitions */
#define	WAKER_SL_SHIFT		0
#define	WAKER_QSC_SHIFT		31

#define WAKER_SL_BIT		(1U << WAKER_SL_SHIFT)
#define WAKER_QSC_BIT		(1U << WAKER_QSC_SHIFT)

#endif /* __ARM_GICV3_COMMON_H__ */
