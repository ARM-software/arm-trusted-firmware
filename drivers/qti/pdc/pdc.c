/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <drivers/qti/pdc/pdc.h>
#include <drivers/qti/pdc/pdc_internal.h>
#include <drivers/qti/pdc/pdc_seq.h>
#include <drivers/qti/pdc/pdc_tcs.h>
#include <drivers/qti/pdc/pdc_types.h>

#include "pdc_regs.h"

extern struct pdc_interrupt_mapping	g_pdc_interrupt_mapping[];
extern const uint32_t			g_pdc_interrupt_table_size;

extern struct pdc_gpio_inputs		g_pdc_gpio_inputs[];
extern const uint32_t			g_pdc_gpio_input_size;

extern struct pdc_gpio_mapping		g_pdc_gpio_mapping[];
extern const uint32_t			g_pdc_gpio_mapping_size;

static void pdc_set_owner(uint32_t pdc_bit_num, uint32_t num_int,
			  enum pdc_pin_type pin_type, enum pdc_drv_type owner)
{
	uint32_t bit_num = pdc_bit_num;

	if (pin_type == PDC_GPIO) {
		bit_num += num_int;
	}

	PDC_IRQ_OWNER_WRITE(bit_num, (uint32_t)owner);
}

static void pdc_target_init(void)
{
	uint32_t i;

	if (g_pdc_gpio_mapping_size == g_pdc_gpio_input_size) {
		for (i = 0U; i < g_pdc_gpio_mapping_size; i++) {
			g_pdc_gpio_mapping[i].gpio_tbl_ptr = &g_pdc_gpio_inputs[i];
			g_pdc_gpio_inputs[i].mux_idx_num   = (uint16_t)i;
		}
	}

	for (i = 0U; i < g_pdc_interrupt_table_size; i++) {
		pdc_set_owner(i, g_pdc_interrupt_table_size, PDC_IRQ,
			      g_pdc_interrupt_mapping[i].trig_config.drv_num);
	}

	for (i = 0U; i < g_pdc_gpio_mapping_size; i++) {
		pdc_set_owner(i, g_pdc_interrupt_table_size, PDC_GPIO,
			      g_pdc_gpio_mapping[i].trig_config.drv_num);
	}
}

void qti_pdc_init(void)
{
	pdc_seq_sys_init();
	pdc_tcs_initialize();
	pdc_target_init();
}
