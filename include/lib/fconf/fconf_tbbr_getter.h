/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_TBBR_GETTER_H
#define FCONF_TBBR_GETTER_H

#include <assert.h>

#include <lib/fconf/fconf.h>

/* TBBR related getter */
#define tbbr__cot_getter(id) __extension__ ({	\
	assert((id) < cot_desc_size);		\
	cot_desc_ptr[id];			\
})

#define tbbr__dyn_config_getter(id)	tbbr_dyn_config.id

struct tbbr_dyn_config_t {
	uint32_t disable_auth;
	void *mbedtls_heap_addr;
	size_t mbedtls_heap_size;
#if MEASURED_BOOT
	uint8_t bl2_hash_data[TCG_DIGEST_SIZE];
#endif
};

extern struct tbbr_dyn_config_t tbbr_dyn_config;

int fconf_populate_tbbr_dyn_config(uintptr_t config);

#endif /* FCONF_TBBR_GETTER_H */
