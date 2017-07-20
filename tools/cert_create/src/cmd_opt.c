/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cmd_opt.h>
#include <getopt.h>
#include <stddef.h>
#include <stdlib.h>
#include "debug.h"

/* Command line options */
static struct option long_opt[CMD_OPT_MAX_NUM+1];
static const char *help_msg[CMD_OPT_MAX_NUM+1];
static int num_reg_opt;

void cmd_opt_add(const cmd_opt_t *cmd_opt)
{
	assert(cmd_opt != NULL);

	if (num_reg_opt >= CMD_OPT_MAX_NUM) {
		ERROR("Out of memory. Please increase CMD_OPT_MAX_NUM\n");
		exit(1);
	}

	long_opt[num_reg_opt].name = cmd_opt->long_opt.name;
	long_opt[num_reg_opt].has_arg = cmd_opt->long_opt.has_arg;
	long_opt[num_reg_opt].flag = 0;
	long_opt[num_reg_opt].val = cmd_opt->long_opt.val;

	help_msg[num_reg_opt] = cmd_opt->help_msg;

	num_reg_opt++;
}

const struct option *cmd_opt_get_array(void)
{
	return long_opt;
}

const char *cmd_opt_get_name(int idx)
{
	if (idx >= num_reg_opt) {
		return NULL;
	}

	return long_opt[idx].name;
}

const char *cmd_opt_get_help_msg(int idx)
{
	if (idx >= num_reg_opt) {
		return NULL;
	}

	return help_msg[idx];
}
