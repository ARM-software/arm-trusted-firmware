/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <plat/common/platform.h>

int plat_rmmd_el3_ide_key_program(uint64_t ecam_address, uint64_t root_port_id,
				  uint64_t ide_stream_info,
				  rp_ide_key_info_t *ide_key_info_ptr, uint64_t request_id,
				  uint64_t cookie)
{
	/* placeholder to add further implementation */

	return 0;

}

int plat_rmmd_el3_ide_key_set_go(uint64_t ecam_address, uint64_t root_port_id,
				 uint64_t ide_stream_info, uint64_t request_id,
				 uint64_t cookie)
{
	/* placeholder to add further implementation */

	return 0;

}

int plat_rmmd_el3_ide_key_set_stop(uint64_t ecam_address, uint64_t root_port_id,
				   uint64_t ide_stream_info, uint64_t request_id,
				   uint64_t cookie)
{
	/* placeholder to add further implementation */

	return 0;
}

int plat_rmmd_el3_ide_km_pull_response(uint64_t ecam_address, uint64_t root_port_id,
				   uint64_t *req_resp, uint64_t *request,
				   uint64_t *cookie)
{
	/* placeholder to add further implementation */
	return E_RMM_UNK;
}
