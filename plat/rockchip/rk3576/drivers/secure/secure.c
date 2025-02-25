// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>
#include <lib/mmio.h>

#include <platform_def.h>

#include <secure.h>
#include <soc.h>

static void secure_timer_init(void)
{
	/* gpu's cntvalue comes from stimer1 channel_5 */
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_CONTROL_REG,
		      TIMER_DIS);

	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_LOAD_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_LOAD_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_CHN_BASE(5) + TIMER_CONTROL_REG,
		      TIMER_EN | TIMER_FMODE);
}

void secure_init(void)
{
	secure_timer_init();
	fw_init();

	/* crypto secure controlled by crypto */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(0), BITS_WITH_WMASK(0, 0x1, 4));
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(0), BITS_WITH_WMASK(0, 0x1, 5));

	/* disable DP encryption mode */
	mmio_write_32(SYS_SGRF_BASE + SYSSGRF_SOC_CON(1), BITS_WITH_WMASK(1, 0x1, 14));
}
