/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDC_INTERNAL_H
#define PDC_INTERNAL_H

#include <stdint.h>

#include <drivers/qti/pdc/pdc_types.h>

/* Sentinel: GPIO not yet assigned to a MUX slot */
#define PDC_GPIO_INVALID	((uint16_t)-1)

/* PDC pin type */
enum pdc_pin_type {
	PDC_IRQ  = 0,
	PDC_GPIO,
};

/* GPIO input table entry */
struct pdc_gpio_inputs {
	uint16_t	gpio_num;
	uint16_t	mux_idx_num;
};

/* GPIO MUX mapping table entry */
struct pdc_gpio_mapping {
	struct pdc_trigger_config	trig_config;
	struct pdc_gpio_inputs		*gpio_tbl_ptr;
	const uint32_t			subsystem_interrupt;
};

/* Static interrupt mapping table entry */
struct pdc_interrupt_mapping {
	struct pdc_trigger_config	trig_config;
	uint32_t			subsystem_interrupt;
};

#endif /* PDC_INTERNAL_H */
