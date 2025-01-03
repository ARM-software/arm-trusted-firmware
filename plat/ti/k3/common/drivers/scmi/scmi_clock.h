#include <stdint.h>
#include <common.h>

typedef struct ti_scmi_clock {
	char name[64];
	uint32_t dev_id;
	uint8_t enable;
	int8_t is_security;
	uint32_t clock_id;
	unsigned long *rates;
	uint64_t cur_rate;
	uint32_t enable_count;
	const struct ti_clk_ops *clk_ops;
	unsigned long *rate_table;
} ti_scmi_clock_t;
