/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "caam.h"
#include <common/debug.h>
#include "jobdesc.h"
#include "sec_hw_specific.h"


/* Callback function after Instantiation decsriptor is submitted to SEC
 */
static void blob_done(uint32_t *desc, uint32_t status, void *arg,
		      void *job_ring)
{
	INFO("Blob Desc SUCCESS with status %x\n", status);
}

/* @brief Submit descriptor to create blob
 * @retval 0 on success
 * @retval -1 on error
 */
int get_hw_unq_key_blob_hw(uint8_t *hw_key, int size)
{
	int ret = 0;
	int i = 0;

	uint32_t key_sz = KEY_IDNFR_SZ_BYTES;
	uint8_t key_data[KEY_IDNFR_SZ_BYTES];
	uint8_t in_data[16];
	uint8_t out_data[16 + KEY_BLOB_SIZE + MAC_SIZE];
	struct job_descriptor desc __aligned(CACHE_WRITEBACK_GRANULE);
	struct job_descriptor *jobdesc = &desc;
	uint32_t in_sz = 16U;

	/* Output blob will have 32 bytes key blob in beginning and
	 * 16 byte HMAC identifier at end of data blob
	 */
	uint32_t out_sz = in_sz + KEY_BLOB_SIZE + MAC_SIZE;

	uint32_t operation = CMD_OPERATION | OP_TYPE_ENCAP_PROTOCOL |
	    OP_PCLID_BLOB | BLOB_PROTO_INFO;

	memset(key_data, 0xff, KEY_IDNFR_SZ_BYTES);
	memset(in_data, 0x00, in_sz);
	memset(out_data, 0x00, in_sz);

	jobdesc->arg = NULL;
	jobdesc->callback = blob_done;

	INFO("\nGenerating Master Key Verification Blob.\n");

	/* Create the hw_rng descriptor */
	ret = cnstr_hw_encap_blob_jobdesc(jobdesc->desc, key_data, key_sz,
					  CLASS_2, in_data, in_sz, out_data,
					  out_sz, operation);

	/* Finally, generate the blob. */
	ret = run_descriptor_jr(jobdesc);
	if (ret != 0) {
		ERROR("Error in running hw unq key blob descriptor\n");
		return -1;
	}
	/* Copying alternate bytes of the Master Key Verification Blob.
	 */
	for (i = 0; i < size; i++) {
		hw_key[i] = out_data[2 * i];
	}

	return ret;
}
