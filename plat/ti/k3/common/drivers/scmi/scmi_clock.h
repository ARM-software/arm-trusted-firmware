#include <stdint.h>
#include <common.h>

typedef struct ti_scmi_clock {
	int8_t is_security;
	uint8_t enable;
	uint32_t clock_id;
	uint32_t dev_id;
	uint32_t enable_count;
	unsigned long *rates;
	unsigned long *rate_table;
	uint64_t cur_rate;
	const struct ti_clk_ops *clk_ops;
	char name[64];
} ti_scmi_clock_t;
