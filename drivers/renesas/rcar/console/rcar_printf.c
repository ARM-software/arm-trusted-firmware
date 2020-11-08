/*
 * Copyright (c) 2015-2020, Renesas Electronics Corporation. All rights reserved.
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

#define RCAR_LOG_HEAD	(('T' << 0) | ('L' << 8) | ('O' << 16) | ('G' << 24))

/*
 * The log is initialized and used before BL31 xlat tables are initialized,
 * therefore the log memory is a device memory at that point. Make sure the
 * memory is correclty aligned and accessed only with up-to 32bit, aligned,
 * writes.
 */
CASSERT((RCAR_BL31_LOG_BASE & 0x7) == 0, assert_bl31_log_base_unaligned);
CASSERT((RCAR_BL31_LOG_MAX & 0x7) == 0, assert_bl31_log_max_unaligned);

extern RCAR_INSTANTIATE_LOCK typedef struct log_head {
	uint32_t head;
	uint32_t index;
	uint32_t size;
	uint32_t res;
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
	logmap_t *t_log = (logmap_t *)RCAR_BL31_LOG_BASE;
	uint32_t *log_data = (uint32_t *)t_log->log_data;
	int16_t init_flag = 0;
	int i;

	if (t_log->header.head != RCAR_LOG_HEAD) {
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
		for (i = 0; i < RCAR_BL31_LOG_MAX; i += 4)
			*log_data++ = 0;

		t_log->header.head = RCAR_LOG_HEAD;
		t_log->header.index = 0U;
		t_log->header.size = 0U;
	}
	rcar_lock_init();

	return 1;
}
