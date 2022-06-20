/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/utils_def.h>
#include <lib/mtk_init/mtk_init.h>

INIT_CALL_TABLE(EXPAND_AS_EXTERN);
extern struct initcall __MTK_PLAT_INITCALL_END__[];

struct initcall *initcall_list[] = {
	INIT_CALL_TABLE(EXPAND_AS_SYMBOL_ARR)
	__MTK_PLAT_INITCALL_END__
};

void mtk_init_one_level(uint32_t level)
{
	const struct initcall *entry;
	int error;

	if (level >= MTK_INIT_LVL_MAX) {
		ERROR("invalid level:%u\n", level);
		panic();
	}

	INFO("init calling level:%u\n", level);
	for (entry = initcall_list[level];
	     (entry != NULL) && (entry < initcall_list[level + 1]);
	     entry++) {
		INFO("calling %s\n", entry->name);
		error = entry->fn();
		if (error != 0) {
			ERROR("init %s fail, errno:%d\n", entry->name, error);
		}
	}
}
