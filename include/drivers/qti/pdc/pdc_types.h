/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_TYPES_H
#define PDC_TYPES_H

/*
 * Trigger type encoding (polarity:1, edge:2):
 *   0,00 = Level Sensitive Active Low
 *   0,01 = Falling Edge Sensitive
 *   0,10 = Rising Edge Sensitive
 *   0,11 = Dual Edge Sensitive
 *   1,00 = Level Sensitive Active High
 */
enum pdc_trigger_type {
	TRIGGER_LEVEL_LOW    = 0x00,
	TRIGGER_FALLING_EDGE = 0x01,
	TRIGGER_RISING_EDGE  = 0x02,
	TRIGGER_DUAL_EDGE    = 0x03,
	TRIGGER_LEVEL_HIGH   = 0x04,
};

/* DRV owner values */
enum pdc_drv_type {
	PDC_DRV_INVALID = -1,
	PDC_DRV0        =  0,
	PDC_DRV1        =  1,
	PDC_DRV2        =  2,
	PDC_DRV_DEFAULT = PDC_DRV0,
};

/* Interrupt/GPIO PDC pin configuration */
struct pdc_trigger_config {
	enum pdc_trigger_type	trig_type;
	enum pdc_drv_type	drv_num;
};

#endif /* PDC_TYPES_H */
