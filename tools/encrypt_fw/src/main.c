/*
 * Copyright (c) 2019, Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/conf.h>

#include "cmd_opt.h"
#include "debug.h"
#include "encrypt.h"
#include "firmware_encrypted.h"

#define NUM_ELEM(x)			((sizeof(x)) / (sizeof(x[0])))
#define HELP_OPT_MAX_LEN		128

/* Global options */

/* Info messages created in the Makefile */
extern const char build_msg[];

static char *key_algs_str[] = {
	[KEY_ALG_GCM] = "gcm",
};

static void print_help(const char *cmd, const struct option *long_opt)
{
	int rem, i = 0;
	const struct option *opt;
	char line[HELP_OPT_MAX_LEN];
	char *p;

	assert(cmd != NULL);
	assert(long_opt != NULL);

	printf("\n\n");
	printf("The firmware encryption tool loads the binary image and\n"
	       "outputs encrypted binary image using an encryption key\n"
	       "provided as an input hex string.\n");
	printf("\n");
	printf("Usage:\n");
	printf("\t%s [OPTIONS]\n\n", cmd);

	printf("Available options:\n");
	opt = long_opt;
	while (opt->name) {
		p = line;
		rem = HELP_OPT_MAX_LEN;
		if (isalpha(opt->val)) {
			/* Short format */
			sprintf(p, "-%c,", (char)opt->val);
			p += 3;
			rem -= 3;
		}
		snprintf(p, rem, "--%s %s", opt->name,
			 (opt->has_arg == required_argument) ? "<arg>" : "");
		printf("\t%-32s %s\n", line, cmd_opt_get_help_msg(i));
		opt++;
		i++;
	}
	printf("\n");
}

static int get_key_alg(const char *key_alg_str)
{
	int i;

	for (i = 0 ; i < NUM_ELEM(key_algs_str) ; i++) {
		if (strcmp(key_alg_str, key_algs_str[i]) == 0) {
			return i;
		}
	}

	return -1;
}

static void parse_fw_enc_status_flag(const char *arg,
				     unsigned short *fw_enc_status)
{
	unsigned long flag;
	char *endptr;

	flag = strtoul(arg, &endptr, 16);
	if (*endptr != '\0' || flag > FW_ENC_WITH_BSSK) {
		ERROR("Invalid fw_enc_status flag '%s'\n", arg);
		exit(1);
	}

	*fw_enc_status = flag & FW_ENC_STATUS_FLAG_MASK;
}

/* Common command line options */
static const cmd_opt_t common_cmd_opt[] = {
	{
		{ "help", no_argument, NULL, 'h' },
		"Print this message and exit"
	},
	{
		{ "fw-enc-status", required_argument, NULL, 'f' },
		"Firmware encryption status flag (with SSK=0 or BSSK=1)."
	},
	{
		{ "key-alg", required_argument, NULL, 'a' },
		"Encryption key algorithm: 'gcm' (default)"
	},
	{
		{ "key", required_argument, NULL, 'k' },
		"Encryption key (for supported algorithm)."
	},
	{
		{ "nonce", required_argument, NULL, 'n' },
		"Nonce or Initialization Vector (for supported algorithm)."
	},
	{
		{ "in", required_argument, NULL, 'i' },
		"Input filename to be encrypted."
	},
	{
		{ "out", required_argument, NULL, 'o' },
		"Encrypted output filename."
	},
};

int main(int argc, char *argv[])
{
	int i, key_alg, ret;
	int c, opt_idx = 0;
	const struct option *cmd_opt;
	char *key = NULL;
	char *nonce = NULL;
	char *in_fn = NULL;
	char *out_fn = NULL;
	unsigned short fw_enc_status = 0;

	NOTICE("Firmware Encryption Tool: %s\n", build_msg);

	/* Set default options */
	key_alg = KEY_ALG_GCM;

	/* Add common command line options */
	for (i = 0; i < NUM_ELEM(common_cmd_opt); i++) {
		cmd_opt_add(&common_cmd_opt[i]);
	}

	/* Get the command line options populated during the initialization */
	cmd_opt = cmd_opt_get_array();

	while (1) {
		/* getopt_long stores the option index here. */
		c = getopt_long(argc, argv, "a:f:hi:k:n:o:", cmd_opt, &opt_idx);

		/* Detect the end of the options. */
		if (c == -1) {
			break;
		}

		switch (c) {
		case 'a':
			key_alg = get_key_alg(optarg);
			if (key_alg < 0) {
				ERROR("Invalid key algorithm '%s'\n", optarg);
				exit(1);
			}
			break;
		case 'f':
			parse_fw_enc_status_flag(optarg, &fw_enc_status);
			break;
		case 'k':
			key = optarg;
			break;
		case 'i':
			in_fn = optarg;
			break;
		case 'o':
			out_fn = optarg;
			break;
		case 'n':
			nonce = optarg;
			break;
		case 'h':
			print_help(argv[0], cmd_opt);
			exit(0);
		case '?':
		default:
			print_help(argv[0], cmd_opt);
			exit(1);
		}
	}

	if (!key) {
		ERROR("Key must not be NULL\n");
		exit(1);
	}

	if (!nonce) {
		ERROR("Nonce must not be NULL\n");
		exit(1);
	}

	if (!in_fn) {
		ERROR("Input filename must not be NULL\n");
		exit(1);
	}

	if (!out_fn) {
		ERROR("Output filename must not be NULL\n");
		exit(1);
	}

	ret = encrypt_file(fw_enc_status, key_alg, key, nonce, in_fn, out_fn);

	CRYPTO_cleanup_all_ex_data();

	return ret;
}
