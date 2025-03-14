/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "rmmd_private.h"
#include <common/debug.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <services/rmmd_svc.h>
#include <smccc_helpers.h>

#define DIR_BIT_SHIFT		0xB
#define KEYSET_BIT_SHIFT	0xC
#define STREAM_ID_MASK		0xFF
#define SUBSTREAM_MASK		0x7
#define SUBSTREAM_SHIFT		0x8
#define MAX_STREAM_ID		32U
#define MAX_SUBSTREAM		3U

bool extract_ide_stream_info(uint64_t ide_stream_info)
{
	uint8_t keyset, dir;
	uint8_t stream_id, substream;

	/* Extract keyset, dir, substream and stream ID */
	keyset = (ide_stream_info >> KEYSET_BIT_SHIFT) & 0x1;
	dir = (ide_stream_info >> DIR_BIT_SHIFT) & 0x1;
	stream_id = ide_stream_info & STREAM_ID_MASK;
	substream = (ide_stream_info >> SUBSTREAM_SHIFT) & SUBSTREAM_MASK;

	if ((stream_id >= MAX_STREAM_ID) || (substream >= MAX_SUBSTREAM)) {
		ERROR("invalid input: stream_id = %x, substream = %x\n", stream_id, substream);
		return false;
	}

	VERBOSE("keyset = %d, dir = %d, stream_id = %d and substream = %d\n", keyset, dir,
		 stream_id, substream);

	return true;
}

int rmmd_el3_ide_key_program(uint64_t ecam_address, uint64_t rp_id,
			     uint64_t ide_stream_info, rp_ide_key_info_t *ide_key_info_ptr,
			     uint64_t request_id, uint64_t cookie)
{
	int err;

	/* TODO: Do validation of params */

	VERBOSE("IDE_KEY_PROG: ecam address = 0x%lx and rp_id = 0x%lx\n", ecam_address, rp_id);

	if (!extract_ide_stream_info(ide_stream_info)) {
		err = E_RMM_INVAL;
		goto exit_fn;
	}

	err = plat_rmmd_el3_ide_key_program(ecam_address, rp_id, ide_stream_info,
					    ide_key_info_ptr, request_id, cookie);

	assert(err == E_RMM_OK || err == E_RMM_AGAIN || err == E_RMM_INVAL ||
		err == E_RMM_IN_PROGRESS || err == E_RMM_UNK || err == E_RMM_FAULT);

exit_fn:
	return err;
}

int rmmd_el3_ide_key_set_go(uint64_t ecam_address, uint64_t rp_id,
			    uint64_t ide_stream_info, uint64_t request_id,
			    uint64_t cookie)
{
	int err;

	/* TODO: Do validation of params */

	VERBOSE("IDE_KEY_SET_GO: ecam address = 0x%lx and rp_id = 0x%lx\n", ecam_address, rp_id);

	if (!extract_ide_stream_info(ide_stream_info)) {
		err = E_RMM_INVAL;
		goto exit_fn;
	}

	err = plat_rmmd_el3_ide_key_set_go(ecam_address, rp_id, ide_stream_info,
					   request_id, cookie);

	assert(err == E_RMM_OK || err == E_RMM_AGAIN || err == E_RMM_INVAL ||
		err == E_RMM_IN_PROGRESS || err == E_RMM_UNK || err == E_RMM_FAULT);

exit_fn:
	return err;
}

int rmmd_el3_ide_key_set_stop(uint64_t ecam_address, uint64_t rp_id,
			      uint64_t ide_stream_info, uint64_t request_id,
			      uint64_t cookie)
{
	int err;

	/* TODO: Do validation of params */

	VERBOSE("IDE_KEY_SET_STOP: ecam address = 0x%lx and rp_id = 0x%lx\n", ecam_address, rp_id);

	if (!extract_ide_stream_info(ide_stream_info)) {
		err = E_RMM_INVAL;
		goto exit_fn;
	}

	err = plat_rmmd_el3_ide_key_set_stop(ecam_address, rp_id, ide_stream_info,
					     request_id, cookie);

	assert(err == E_RMM_OK || err == E_RMM_AGAIN || err == E_RMM_INVAL ||
		err == E_RMM_IN_PROGRESS || err == E_RMM_UNK || err == E_RMM_FAULT);

exit_fn:
	return err;
}

int rmmd_el3_ide_km_pull_response(uint64_t ecam_address, uint64_t rp_id,
				  uint64_t *req_resp, uint64_t *request_id,
				  uint64_t *cookie)
{
	int err;

	/* TODO: Do validation of params */

	VERBOSE("IDE_KM_PULL: ecam address = 0x%lx, rp_id = 0x%lx\n", ecam_address, rp_id);

	err = plat_rmmd_el3_ide_km_pull_response(ecam_address, rp_id, req_resp, request_id, cookie);

	assert(err == E_RMM_OK || err == E_RMM_AGAIN || err == E_RMM_INVAL ||
		err == E_RMM_IN_PROGRESS || err == E_RMM_UNK || err == E_RMM_FAULT);

	return err;
}
