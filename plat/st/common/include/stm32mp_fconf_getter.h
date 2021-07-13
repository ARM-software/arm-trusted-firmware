/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_FCONF_GETTER
#define STM32MP_FCONF_GETTER

#include <assert.h>

#include <lib/fconf/fconf.h>

/* IO policies */
#define stm32mp__io_policies_getter(id) __extension__ ({	\
	assert((id) < MAX_NUMBER_IDS);				\
	&policies[id];						\
})

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

extern struct plat_io_policy policies[];
int fconf_populate_stm32mp_io_policies(uintptr_t config);

#endif /* STM32MP_FCONF_GETTER */
