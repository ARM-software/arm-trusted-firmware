/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STM32MP_RAM_H
#define STM32MP_RAM_H

#include <stdbool.h>

#include <drivers/st/stm32mp_ddr.h>

#define PARAM(x, y)							\
	{								\
		.name = x,						\
		.offset = offsetof(struct stm32mp_ddr_config, y),	\
		.size = sizeof(config.y) / sizeof(uint32_t),		\
	}

#define CTL_PARAM(x) PARAM("st,ctl-"#x, c_##x)
#define PHY_PARAM(x) PARAM("st,phy-"#x, p_##x)

struct stm32mp_ddr_param {
	const char *name;      /* Name in DT */
	const uint32_t offset; /* Offset in config struct */
	const uint32_t size;   /* Size of parameters */
};

int stm32mp_ddr_dt_get_info(void *fdt, int node, struct stm32mp_ddr_info *info);
int stm32mp_ddr_dt_get_param(void *fdt, int node, const struct stm32mp_ddr_param *param,
			     uint32_t param_size, uintptr_t config);

#endif /* STM32MP_RAM_H */
