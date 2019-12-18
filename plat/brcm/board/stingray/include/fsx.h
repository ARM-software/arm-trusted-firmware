/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSX_H
#define FSX_H

#include <stdbool.h>

typedef enum FSX_TYPE {
	eFS4_RAID,
	eFS4_CRYPTO,
	eFS6_PKI,
} eFSX_TYPE;

void fsx_init(eFSX_TYPE fsx_type,
	      unsigned int ring_count,
	      unsigned int dme_count,
	      unsigned int ae_count,
	      unsigned int start_stream_id,
	      unsigned int msi_dev_id,
	      uintptr_t idm_io_control_direct,
	      uintptr_t idm_reset_control,
	      uintptr_t base,
	      uintptr_t dme_base);

void fsx_meminit(const char *name,
		 uintptr_t idm_io_control_direct,
		 uintptr_t idm_io_status);

void fs4_disable_clocks(bool disable_sram,
			bool disable_crypto,
			bool disable_raid);

#endif /* FSX_H */
