#include <arch_helpers.h>
#include <delay_timer.h>
#include <mt8173_def.h>

static uint32_t plat_timerus_get_value(void)
{
	/* Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. */
	return (uint32_t)(~read_cntpct_el0());
}

static const timer_ops_t plat_timer_ops = {
	.get_timer_value	= plat_timerus_get_value,
	.clk_mult		= 1,
	.clk_div		= SYS_COUNTER_FREQ_IN_MHZ,
};

void plat_delay_timer_init(void)
{
	timer_init(&plat_timer_ops);
}
