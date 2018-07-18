/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <platform_def.h>
#include <thunder_private.h>
#include <debug.h>

static void thunder_gti_sync(unsigned node_count, uint64_t *dividers)
{
	/* lets do a basic GTI timers syncing across nodes, so that
	 * software on higher layers get some time to re sync with more
	 * precision alg
	 */
	uint64_t remote_node_cnt;
	uint64_t local_node_cnt;
	unsigned slope = 0;

	remote_node_cnt = CSR_READ_PA(1, CAVM_GTI_RD_CNTCV);
	local_node_cnt = CSR_READ_PA(0, CAVM_GTI_RD_CNTCV);

	if (local_node_cnt < remote_node_cnt) {
		CSR_WRITE_PA(0, CAVM_GTI_CC_CNTADD, remote_node_cnt - local_node_cnt);
	} else {
		CSR_WRITE_PA(1, CAVM_GTI_CC_CNTADD, local_node_cnt - remote_node_cnt);
	}

	INFO("Started Syncronizing GTIs");

	while (1) {
		uint64_t max_cntadd = 0;
		uint64_t tmp_cntadd;
		unsigned tmp_tries;

		for (tmp_tries = 0; tmp_tries < 100; tmp_tries ++) {
			remote_node_cnt = CSR_READ_PA(1, CAVM_GTI_RD_CNTCV);
			local_node_cnt = CSR_READ_PA(0, CAVM_GTI_RD_CNTCV);

			if (local_node_cnt < remote_node_cnt) {
				tmp_cntadd = remote_node_cnt - local_node_cnt;
			} else {
				tmp_cntadd = local_node_cnt - remote_node_cnt;
			}

			if (tmp_cntadd > max_cntadd) {
				max_cntadd = tmp_cntadd;
				slope ++;
			}
		}

		if (slope <  10 )
			break;

		if (remote_node_cnt > local_node_cnt) {
			/* NODE1 running fast. slow him down */
			dividers[1] += 0x4;
			CSR_WRITE_PA(1 , CAVM_GTI_CC_CNTRATE, dividers[1]);
		} else {
			/* NODE0 is fast. slow down */
			dividers[0] += 0x4;
			CSR_WRITE_PA(0 , CAVM_GTI_CC_CNTRATE, dividers[0]);
		}

		INFO(".");
	}

	INFO(" done\n");
}

void thunder_gti_init(void)
{
	unsigned node_count = thunder_get_node_count();
	unsigned node;

	uint64_t dividers[PLATFORM_MAX_NODES];

	for (node = 0; node < node_count; node++) {
		cavm_rst_boot_t rst_boot;
		uint32_t divider = 0;
		int64_t sclk;

		rst_boot.u = CSR_READ_PA(node, CAVM_RST_BOOT);
		sclk = rst_boot.s.pnr_mul * 50; //MHz

		divider = (THUNDER_SYSCNT_FREQ << 32) / sclk;

		// GTI TIMER CNTFRQ
		CSR_WRITE_PA(node, CAVM_GTI_CC_CNTRATE, divider);
		// Enable GTI TIMER
		CSR_WRITE_PA(node, CAVM_GTI_CC_CNTCR, 0x1);

		/* Handle a case where both nodes can have different SCLK */
		dividers[node] = divider;
	}

	if (node_count > 1) {
		thunder_gti_sync(node_count, dividers);
	}
}
