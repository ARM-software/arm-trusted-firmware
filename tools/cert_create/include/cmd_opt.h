/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CMD_OPT_H_
#define CMD_OPT_H_

#include <getopt.h>

#define CMD_OPT_MAX_NUM			64

/* Supported long command line option types */
enum {
	CMD_OPT_CERT,
	CMD_OPT_KEY,
	CMD_OPT_EXT
};

/* Structure to define a command line option */
typedef struct cmd_opt_s {
	struct option long_opt;
	const char *help_msg;
} cmd_opt_t;

/* Exported API*/
void cmd_opt_add(const cmd_opt_t *cmd_opt);
const struct option *cmd_opt_get_array(void);
const char *cmd_opt_get_name(int idx);
const char *cmd_opt_get_help_msg(int idx);

#endif /* CMD_OPT_H_ */
