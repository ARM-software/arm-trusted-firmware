/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AGATTI_DEF_H
#define AGATTI_DEF_H

#include <bruin_def.h>

/* GIC-500 */
#define QTI_GICD_BASE				0x0F200000
#define QTI_GICR_BASE				0x0F300000
#define QTI_GICC_BASE				0x0

/* APSS QTimer control base (CNTCTLBase) */
#define QTI_QTIMER_BASE				0x0F120000

/* APSS per-core power controllers, as on MSM8916 */
#define AGATTI_APCS_ALIAS_ACS(core)		(0x0F188000 + (core) * 0x10000)

/* Debug UART: QUPv3 wrap 0, serial engine 4 */
#define PLAT_QTI_UART_BASE			0x04A90000

/* PM4125 PON registers behind the SPMI arbiter */
#define PON_PS_HOLD_RESET_CTL			0x85A
#define PON_PS_HOLD_RESET_CTL2			0x85B

/* Memory map */
#define QTI_DEVICE_SIZE				(0x10000000 - QTI_DEVICE_BASE)

#define QTI_PIMEM_LIMIT				0x10500000

#define QTI_SMEM_BASE				0x46000000

#endif /* AGATTI_DEF_H */
