/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
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
		return -ENOVABCERT;
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

static void sha512_transform(uint64_t *state, const uint8_t *input)
{
	uint64_t a, b, c, d, e, f, g, h, t1, t2;

	int i;
	uint64_t W[16];

	/* load the state into our registers */
	a = state[0];   b = state[1];   c = state[2];   d = state[3];
	e = state[4];   f = state[5];   g = state[6];   h = state[7];

	/* now iterate */
	for (i = 0 ; i < 80; i += 8) {
		if (!(i & 8)) {
			int j;

			if (i < 16) {
				/* load the input */
				for (j = 0; j < 16; j++)
					LOAD_OP(i + j, W, input);
			} else {
				for (j = 0; j < 16; j++) {
					BLEND_OP(i + j, W);
				}
			}
		}

		t1 = h + e1(e) + Ch(e, f, g) + sha512_K[i] + W[(i & 15)];
		t2 = e0(a) + Maj(a, b, c);    d += t1;    h = t1 + t2;
		t1 = g + e1(d) + Ch(d, e, f) + sha512_K[i+1] + W[(i & 15) + 1];
		t2 = e0(h) + Maj(h, a, b);    c += t1;    g = t1 + t2;
		t1 = f + e1(c) + Ch(c, d, e) + sha512_K[i+2] + W[(i & 15) + 2];
		t2 = e0(g) + Maj(g, h, a);    b += t1;    f = t1 + t2;
		t1 = e + e1(b) + Ch(b, c, d) + sha512_K[i+3] + W[(i & 15) + 3];
		t2 = e0(f) + Maj(f, g, h);    a += t1;    e = t1 + t2;
		t1 = d + e1(a) + Ch(a, b, c) + sha512_K[i+4] + W[(i & 15) + 4];
		t2 = e0(e) + Maj(e, f, g);    h += t1;    d = t1 + t2;
		t1 = c + e1(h) + Ch(h, a, b) + sha512_K[i+5] + W[(i & 15) + 5];
		t2 = e0(d) + Maj(d, e, f);    g += t1;    c = t1 + t2;
		t1 = b + e1(g) + Ch(g, h, a) + sha512_K[i+6] + W[(i & 15) + 6];
		t2 = e0(c) + Maj(c, d, e);    f += t1;    b = t1 + t2;
		t1 = a + e1(f) + Ch(f, g, h) + sha512_K[i+7] + W[(i & 15) + 7];
		t2 = e0(b) + Maj(b, c, d);    e += t1;    a = t1 + t2;
	}

	state[0] += a; state[1] += b; state[2] += c; state[3] += d;
	state[4] += e; state[5] += f; state[6] += g; state[7] += h;

	/* erase our data */
	a = b = c = d = e = f = g = h = t1 = t2 = 0;
}

static void sha512_block_fn(sha512_context *sst, const uint8_t *src,
				    int blocks)
{
	while (blocks--) {
		sha512_transform(sst->state, src);
		src += SHA512_BLOCK_SIZE;
	}
}


static void sha512_base_do_finalize(sha512_context *sctx)
{
	const int bit_offset = SHA512_BLOCK_SIZE - sizeof(uint64_t[2]);
	uint64_t *bits = (uint64_t *)(sctx->buf + bit_offset);
	unsigned int partial = sctx->count[0] % SHA512_BLOCK_SIZE;

	sctx->buf[partial++] = 0x80;
	if (partial > bit_offset) {
		memset(sctx->buf + partial, 0x0, SHA512_BLOCK_SIZE - partial);
		partial = 0;

		sha512_block_fn(sctx, sctx->buf, 1);
	}

	memset(sctx->buf + partial, 0x0, bit_offset - partial);
	//fixme bits[0] = cpu_to_be64(sctx->count[1] << 3 | sctx->count[0] >> 61);
	//fixme bits[1] = cpu_to_be64(sctx->count[0] << 3);
	bits[0] = (sctx->count[1] << 3 | sctx->count[0] >> 61);
	bits[1] = (sctx->count[0] << 3);
	sha512_block_fn(sctx, sctx->buf, 1);
}

static void sha512_base_do_update(sha512_context *sctx,
					const uint8_t *data,
					unsigned int len)
{
	unsigned int partial = sctx->count[0] % SHA512_BLOCK_SIZE;

	sctx->count[0] += len;
	if (sctx->count[0] < len)
		sctx->count[1]++;

	if (((partial + len) >= SHA512_BLOCK_SIZE)) {
		int blocks;

		if (partial) {
			int p = SHA512_BLOCK_SIZE - partial;

			memcpy(sctx->buf + partial, data, p);
			data += p;
			len -= p;

			sha512_block_fn(sctx, sctx->buf, 1);
		}

		blocks = len / SHA512_BLOCK_SIZE;
		len %= SHA512_BLOCK_SIZE;

		if (blocks) {
			sha512_block_fn(sctx, data, blocks);
			data += blocks * SHA512_BLOCK_SIZE;
		}
		partial = 0;
	}
	if (len)
		memcpy(sctx->buf + partial, data, len);
}

void sha384_starts(sha512_context *ctx)
{
	ctx->state[0] = SHA384_H0;
	ctx->state[1] = SHA384_H1;
	ctx->state[2] = SHA384_H2;
	ctx->state[3] = SHA384_H3;
	ctx->state[4] = SHA384_H4;
	ctx->state[5] = SHA384_H5;
	ctx->state[6] = SHA384_H6;
	ctx->state[7] = SHA384_H7;
	ctx->count[0] = ctx->count[1] = 0;
}

void sha384_update(sha512_context *ctx, const uint8_t *input, uint32_t length)
{
	sha512_base_do_update(ctx, input, length);
}

void sha384_finish(sha512_context *ctx, uint8_t digest[SHA384_SUM_LEN])
{
	int i;

	sha512_base_do_finalize(ctx);
	for (i = 0; i < SHA384_SUM_LEN / sizeof(uint64_t); i++)
		PUT_UINT64_BE(ctx->state[i], digest, i * 8);
}

void sha384_csum_wd(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz)
{
	sha512_context ctx;
// #if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	// const unsigned char *end;
	// unsigned char *curr;
	// int chunk;
// #endif

	sha384_starts(&ctx);

// #if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	// curr = (unsigned char *)input;
	// end = input + ilen;
	// while (curr < end) {
		// chunk = end - curr;
		// if (chunk > chunk_sz)
			// chunk = chunk_sz;
		// sha384_update(&ctx, curr, chunk);
		// curr += chunk;
		// schedule();
	// }
// #else
	sha384_update(&ctx, input, ilen);
// #endif

	sha384_finish(&ctx, output);
}
