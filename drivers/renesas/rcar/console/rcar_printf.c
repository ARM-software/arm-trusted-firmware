/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <stdint.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/bakery_lock.h>

#include "rcar_def.h"
#include "rcar_private.h"
#include "rcar_printf.h"

#define INDEX_TIMER_COUNT	(4U)

extern RCAR_INSTANTIATE_LOCK typedef struct log_head {
	uint8_t head[4];
	uint32_t index;
	uint32_t size;
	uint8_t res[4];
} loghead_t;

typedef struct log_map {
	loghead_t header;
	uint8_t log_data[RCAR_BL31_LOG_MAX];
	uint8_t res_data[RCAR_LOG_RES_SIZE];
} logmap_t;

int32_t rcar_set_log_data(int32_t c)
{
	logmap_t *t_log;

	t_log = (logmap_t *) RCAR_BL31_LOG_BASE;

	rcar_lock_get();

	/*
	 * If index is broken, then index and size initialize
	 */
	if (t_log->header.index >= (uint32_t) RCAR_BL31_LOG_MAX) {
		t_log->header.index = 0U;
		t_log->header.size = 0U;
	}
	/*
	 * data store to log area then index and size renewal
	 */
	t_log->log_data[t_log->header.index] = (uint8_t) c;
	t_log->header.index++;
	if (t_log->header.size < t_log->header.index) {
		t_log->header.size = t_log->header.index;
	}
	if (t_log->header.index >= (uint32_t) RCAR_BL31_LOG_MAX) {
		t_log->header.index = 0U;
	}

	rcar_lock_release();

	return 1;
}

int32_t rcar_log_init(void)
{

	static const uint8_t const_header[] = "TLOG";
	logmap_t *t_log;
	int16_t init_flag = 0;

	t_log = (logmap_t *) RCAR_BL31_LOG_BASE;
	if (memcmp
	    ((const void *)t_log->header.head, (const void *)const_header,
	     sizeof(t_log->header.head)) != 0) {
		/*
		 * Log header is not "TLOG", then log area initialize
		 */
		init_flag = 1;
	}
	if (t_log->header.index >= (uint32_t) RCAR_BL31_LOG_MAX) {
		/*
		 * index is broken, then log area initialize
		 */
		init_flag = 1;
	}
	if (init_flag == 1) {
		(void)memset((void *)t_log->log_data, 0,
			     (size_t) RCAR_BL31_LOG_MAX);
		(void)memcpy((void *)t_log->header.head,
			     (const void *)const_header,
			     sizeof(t_log->header.head));
		t_log->header.index = 0U;
		t_log->header.size = 0U;
	}
	rcar_lock_init();

	return 1;
}
