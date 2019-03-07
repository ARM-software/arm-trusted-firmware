/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>

#include <common/debug.h>
#include <errno.h>
#include <string.h>

#include <io.h>
#include <caam.h>
#include <jobdesc.h>
#include <sec_hw_specific.h>


#define BLOB_PROTO_INFO		 0x00000002


/* Callback function after Instantiation decsriptor is submitted to SEC */
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
#ifdef ALLOC
	uint8_t *key_data = NULL;
	uint8_t *in_data = NULL;
	uint8_t *out_data = NULL;
	static struct job_descriptor *jobdesc;
#else
	uint8_t key_data[KEY_IDNFR_SZ_BYTES];
	uint8_t in_data[16];
	uint8_t out_data[16 + KEY_BLOB_SIZE + MAC_SIZE];
	struct job_descriptor desc;
	struct job_descriptor *jobdesc = &desc;
#endif
	uint32_t in_sz = 16;

	/* output blob will have 32 bytes key blob in beginning and
	 * 16 byte HMAC identifier at end of data blob
	 */
	uint32_t out_sz = in_sz + KEY_BLOB_SIZE + MAC_SIZE;

	uint32_t operation = CMD_OPERATION | OP_TYPE_ENCAP_PROTOCOL |
	    OP_PCLID_BLOB | BLOB_PROTO_INFO;

#ifdef ALLOC
	key_data = alloc(KEY_IDNFR_SZ_BYTES, 64);

	if (key_data == NULL) {
		ERROR("%s: key data buffer allocation failed\n", __func__);
		ret = -1;
		goto clean_up;
	}
#endif
	memset(key_data, 0xff, KEY_IDNFR_SZ_BYTES);
#ifdef ALLOC
	in_data = alloc(in_sz, 64);

	if (in_data == NULL) {
		ERROR("%s: in data buffer allocation failed\n", __func__);
		ret = -1;
		goto clean_up;
	}
#endif
	memset(in_data, 0x00, in_sz);
#ifdef ALLOC
	out_data = alloc(out_sz, 64);

	if (out_data == NULL) {
		ERROR("%s: out data buffer allocation failed\n", __func__);
		ret = -1;
		goto clean_up;
	}
#endif
	memset(out_data, 0x00, in_sz);
#ifdef ALLOC
	jobdesc = alloc(sizeof(struct job_descriptor), 64);
	if (jobdesc == NULL) {
		debug("DEC allocation failed\n");
		ret = -1;
		goto clean_up;
	}
#endif

	jobdesc->arg = NULL;
	jobdesc->callback = blob_done;

	INFO("\nGenerating Master Key Verification Blob.\n");

	ret = cnstr_hw_encap_blob_jobdesc(jobdesc->desc, key_data, key_sz,
					  CLASS_2, in_data, in_sz, out_data,
					  out_sz, operation);

	/* Finally, generate the blob */
	ret = run_descriptor_jr(jobdesc);
	if (ret) {
		ERROR("Error in running hw unq key blob descriptor\n");
		return -1;
	}
	/* Copying alternate bytes of the Master Key Verification Blob. */
	for (i = 0; i < size; i++)
		hw_key[i] = out_data[2 * i];

#ifdef ALLOC
clean_up:
	if (key_data != NULL)
		free(key_data);
	if (in_data != NULL)
		free(in_data);
	if (out_data != NULL)
		free(out_data);
	if (jobdesc != NULL)
		free(jobdesc);
#endif
	return ret;
}
