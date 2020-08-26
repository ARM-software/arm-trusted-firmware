/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>

#include <qti_plat.h>
#include <spmi_arb.h>

/*
 * This driver implements PON support for PM8998-compatible PMICs. This can
 * include other part numbers like PM6150.
 */

#define PON_PS_HOLD_RESET_CTL		0x85a
#define RESET_TYPE_WARM_RESET		1
#define RESET_TYPE_SHUTDOWN		4

#define PON_PS_HOLD_RESET_CTL2		0x85b
#define S2_RESET_EN			BIT(7)

static void configure_ps_hold(uint32_t reset_type)
{
	/* QTI recommends disabling reset for 10 cycles before reconfiguring. */
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, 0);
	mdelay(1);

	spmi_arb_write8(PON_PS_HOLD_RESET_CTL, reset_type);
	spmi_arb_write8(PON_PS_HOLD_RESET_CTL2, S2_RESET_EN);
	mdelay(1);
}

void qti_pmic_prepare_reset(void)
{
	configure_ps_hold(RESET_TYPE_WARM_RESET);
}

void qti_pmic_prepare_shutdown(void)
{
	configure_ps_hold(RESET_TYPE_SHUTDOWN);
}
