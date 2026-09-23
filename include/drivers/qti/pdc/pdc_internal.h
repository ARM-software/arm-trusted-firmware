/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_INTERNAL_H
#define PDC_INTERNAL_H

#include <stdint.h>

#include <drivers/qti/pdc/pdc_types.h>

/* PDC pin type */
enum pdc_pin_type {
	PDC_IRQ  = 0,
	PDC_GPIO,
};

/* GPIO mapping table entry */
struct pdc_gpio_mapping {
	struct pdc_trigger_config	trig_config;
	const uint32_t			subsystem_interrupt;
};

/* Static interrupt mapping table entry */
struct pdc_interrupt_mapping {
	struct pdc_trigger_config	trig_config;
	uint32_t			subsystem_interrupt;
};

#endif /* PDC_INTERNAL_H */
