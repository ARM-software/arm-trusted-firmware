/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_GICV3_COMMON_H
#define ARM_GICV3_COMMON_H

/*******************************************************************************
 * GIC500/GIC600 Re-distributor interface registers & constants
 ******************************************************************************/

/* GICR_WAKER implementation-defined bit definitions */
#define	WAKER_SL_SHIFT		0
#define	WAKER_QSC_SHIFT		31

#define WAKER_SL_BIT		(1U << WAKER_SL_SHIFT)
#define WAKER_QSC_BIT		(1U << WAKER_QSC_SHIFT)

#define IIDR_MODEL_ARM_GIC_600		U(0x0200043b)
#define IIDR_MODEL_ARM_GIC_600AE	U(0x0300043b)
#define IIDR_MODEL_ARM_GIC_700		U(0x0400043b)

#endif /* ARM_GICV3_COMMON_H */
