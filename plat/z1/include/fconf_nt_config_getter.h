/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_NT_CONFIG_GETTER_H
#define FCONF_NT_CONFIG_GETTER_H

#include <lib/fconf/fconf.h>

/* NT Firmware Config related getter */
#define nt_config__event_log_config_getter(prop) event_log.prop

struct event_log_config_t {
#ifdef SPD_opteed
	void *tpm_event_log_sm_addr;
#endif
	void *tpm_event_log_addr;
	size_t tpm_event_log_size;
};

int fconf_populate_event_log_config(uintptr_t config);

extern struct event_log_config_t event_log_config;

#endif /* FCONF_NT_CONFIG_GETTER_H */
