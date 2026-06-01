/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <drivers/qti/cmd_db/cmd_db.h>
#include <drivers/qti/pdc/pdc_tcs.h>
#include <drivers/qti/pdc/tcs_resource.h>
#include "pdc_regs.h"

#include <platform_def.h>

extern struct pdc_tcs_config	g_pdc_tcs_config[TCS_NUM_TOTAL][NUM_COMMANDS_PER_TCS];
extern struct pdc_tcs_resource	g_pdc_resource_list[TCS_TOTAL_RESOURCE_NUM];

static void pdc_tcs_write_cmd(uint32_t base, uint16_t tcs_num, uint16_t cmd_num,
			      struct pdc_tcs_config *config)
{
	uint32_t msgid = 0U;
	uint32_t tmp;

	if ((config->data.options & TCS_CFG_OPT_READ) == 0U) {
		msgid |= PDC_TCS_MSGID_WRITE_BMSK;
	}

	if ((config->data.options & TCS_CFG_OPT_CMD_RESP_REQ) != 0U) {
		msgid |= PDC_TCS_MSGID_RES_REQ_BMSK;
	}

	PDC_TCS_MSGID_WRITE(base, tcs_num, cmd_num, msgid);

	if ((config->data.options & TCS_CFG_OPT_CMD_WAIT) != 0U) {
		tmp  = PDC_TCS_WAIT_CMPL_READ(base, tcs_num);
		tmp |= (1U << cmd_num);
		PDC_TCS_WAIT_CMPL_WRITE(base, tcs_num, tmp);
	}

	config->cmd.resource_p = &g_pdc_resource_list[config->cmd.index];

	PDC_TCS_ADDR_WRITE(base, tcs_num, cmd_num,
			   config->cmd.resource_p->base_addr +
			   config->data.addr_offset);

	PDC_TCS_DATA_WRITE(base, tcs_num, cmd_num, config->data.res_val);
}

void pdc_tcs_initialize(void)
{
	const uint32_t base      = PDC_BASE;
	const uint32_t total_tcs = 4U;
	uint32_t enable_mask[TCS_NUM_TOTAL] = { 0U };
	uint32_t n_tcs;
	uint32_t n_res;
	uint32_t n_cmd;
	uint32_t base_addr;
	struct pdc_tcs_config *cmd_data;

	assert(total_tcs <= TCS_NUM_TOTAL);

	for (n_res = 0U; n_res < TCS_TOTAL_RESOURCE_NUM; n_res++) {
		base_addr = cmd_db_query_addr(g_pdc_resource_list[n_res].name);
		if (base_addr == 0U) {
			return;
		}
		g_pdc_resource_list[n_res].base_addr += base_addr;
	}

	assert(PDC_PARAM_TCS_CMDS(base) == NUM_COMMANDS_PER_TCS);
	assert(total_tcs <= PDC_PARAM_TCS_COUNT(base));

	for (n_tcs = 0U; n_tcs < total_tcs; n_tcs++) {
		for (n_cmd = 0U; n_cmd < NUM_COMMANDS_PER_TCS; n_cmd++) {
			cmd_data = &g_pdc_tcs_config[n_tcs][n_cmd];

			if ((cmd_data->data.options & TCS_CFG_OPT_NOT_USED) != 0U) {
				continue;
			}

			if ((cmd_data->data.options & TCS_CFG_OPT_NO_ENABLE) == 0U) {
				enable_mask[n_tcs] |= (1U << n_cmd);
			}

			pdc_tcs_write_cmd(base, (uint16_t)n_tcs,
					  (uint16_t)n_cmd, cmd_data);
		}
	}

	for (n_tcs = 0U; n_tcs < total_tcs; n_tcs++) {
		PDC_TCS_CMD_ENABLE_WRITE(base, n_tcs, enable_mask[n_tcs]);
	}
}
