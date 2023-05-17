/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <tools_share/firmware_image_package.h>

#include "socfpga_mailbox.h"
#include "socfpga_vab.h"

static size_t get_img_size(uint8_t *img_buf, size_t img_buf_sz)
{
	uint8_t *img_buf_end = img_buf + img_buf_sz;
	uint32_t cert_sz = get_unaligned_le32(img_buf_end - sizeof(uint32_t));
	uint8_t *p = img_buf_end - cert_sz - sizeof(uint32_t);

	/* Ensure p is pointing within the img_buf */
	if (p < img_buf || p > (img_buf_end - VAB_CERT_HEADER_SIZE))
		return 0;

	if (get_unaligned_le32(p) == SDM_CERT_MAGIC_NUM)
		return (size_t)(p - img_buf);

	return 0;
}



int socfpga_vendor_authentication(void **p_image, size_t *p_size)
{
	int retry_count = 20;
	uint8_t hash384[FCS_SHA384_WORD_SIZE];
	uint64_t img_addr, mbox_data_addr;
	uint32_t img_sz, mbox_data_sz;
	uint8_t *cert_hash_ptr, *mbox_relocate_data_addr;
	uint32_t resp = 0, resp_len = 1;
	int ret = 0;

	img_addr = (uintptr_t)*p_image;
	img_sz = get_img_size((uint8_t *)img_addr, *p_size);

	if (!img_sz) {
		NOTICE("VAB certificate not found in image!\n");
		return -ENOVABIMG;
	}

	if (!IS_BYTE_ALIGNED(img_sz, sizeof(uint32_t))) {
		NOTICE("Image size (%d bytes) not aliged to 4 bytes!\n", img_sz);
		return -EIMGERR;
	}

	/* Generate HASH384 from the image */
	/* TODO: This part need to cross check !!!!!! */
	sha384_csum_wd((uint8_t *)img_addr, img_sz, hash384, CHUNKSZ_PER_WD_RESET);
	cert_hash_ptr = (uint8_t *)(img_addr + img_sz +
	VAB_CERT_MAGIC_OFFSET + VAB_CERT_FIT_SHA384_OFFSET);

	/*
	 * Compare the SHA384 found in certificate against the SHA384
	 * calculated from image
	 */
	if (memcmp(hash384, cert_hash_ptr, FCS_SHA384_WORD_SIZE)) {
		NOTICE("SHA384 does not match!\n");
		return -EKEYREJECTED;
	}


	mbox_data_addr = img_addr + img_sz - sizeof(uint32_t);
	/* Size in word (32bits) */
	mbox_data_sz = (BYTE_ALIGN(*p_size - img_sz, sizeof(uint32_t))) >> 2;

	NOTICE("mbox_data_addr = %lx    mbox_data_sz = %d\n", mbox_data_addr, mbox_data_sz);

	/* TODO: This part need to cross check !!!!!! */
	// mbox_relocate_data_addr = (uint8_t *)malloc(mbox_data_sz * sizeof(uint32_t));
	// if (!mbox_relocate_data_addr) {
		// NOTICE("Cannot allocate memory for VAB certificate relocation!\n");
		// return -ENOMEM;
	// }

	memcpy(mbox_relocate_data_addr, (uint8_t *)mbox_data_addr, mbox_data_sz * sizeof(uint32_t));
	*(uint32_t *)mbox_relocate_data_addr = 0;

	do {
		/* Invoke SMC call to ATF to send the VAB certificate to SDM */
		ret  = mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_VAB_SRC_CERT,
(uint32_t *)mbox_relocate_data_addr, mbox_data_sz, 0, &resp, &resp_len);

		/* If SDM is not available, just delay 50ms and retry again */
		/* 0x1FF = The device is busy */
		if (ret == MBOX_RESP_ERR(0x1FF)) {
			mdelay(50);
		} else {
			break;
		}
	} while (--retry_count);

	/* Free the relocate certificate memory space */
	zeromem((void *)&mbox_relocate_data_addr, sizeof(uint32_t));


	/* Exclude the size of the VAB certificate from image size */
	*p_size = img_sz;

	if (ret) {
		/*
		 * Unsupported mailbox command or device not in the
		 * owned/secure state
		 */
		 /* 0x85 = Not allowed under current security setting */
		if (ret == MBOX_RESP_ERR(0x85)) {
			/* SDM bypass authentication */
			NOTICE("Image Authentication bypassed at address\n");
			return 0;
		}
		NOTICE("VAB certificate authentication failed in SDM\n");
		/* 0x1FF = The device is busy */
		if (ret == MBOX_RESP_ERR(0x1FF)) {
			NOTICE("Operation timed out\n");
			return -ETIMEOUT;
		} else if (ret == MBOX_WRONG_ID) {
			NOTICE("No such process\n");
			return -EPROCESS;
		}
	} else {
		/* If Certificate Process Status has error */
		if (resp) {
			NOTICE("VAB certificate execution format error\n");
			return -EIMGERR;
		}
	}

	NOTICE("Image Authentication bypassed at address\n");
	return ret;

}

static uint32_t get_unaligned_le32(const void *p)
{
	/* TODO: Temp for testing */
	//return le32_to_cpup((__le32 *)p);
	return 0;
}

void sha384_csum_wd(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz)
{
	/* TODO: Update sha384 start, update and finish */
}
