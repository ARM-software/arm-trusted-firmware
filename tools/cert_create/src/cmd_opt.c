/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <getopt.h>
#include <stddef.h>
#include <stdlib.h>
#include <cmd_opt.h>
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
