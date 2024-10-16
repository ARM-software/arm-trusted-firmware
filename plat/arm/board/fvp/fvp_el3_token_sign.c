/*
 * Copyright (c) 2024, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <plat/common/platform.h>
#include <services/rmm_el3_token_sign.h>

static struct el3_token_sign_request el3_req = { 0 };
static bool el3_req_valid;

/*
 * According to https://www.secg.org/sec1-v2.pdf 2.3.3
 * the size of the ECDSA P384 public key is 97 bytes,
 * with the first byte being 0x04.
 */
static uint8_t sample_attest_pub_key[] = {
	0x04, 0x76, 0xf9, 0x88, 0x09, 0x1b, 0xe5, 0x85, 0xed, 0x41,
	0x80, 0x1a, 0xec, 0xfa, 0xb8, 0x58, 0x54, 0x8c, 0x63, 0x05,
	0x7e, 0x16, 0xb0, 0xe6, 0x76, 0x12, 0x0b, 0xbd, 0x0d, 0x2f,
	0x9c, 0x29, 0xe0, 0x56, 0xc5, 0xd4, 0x1a, 0x01, 0x30, 0xeb,
	0x9c, 0x21, 0x51, 0x78, 0x99, 0xdc, 0x23, 0x14, 0x6b, 0x28,
	0xe1, 0xb0, 0x62, 0xbd, 0x3e, 0xa4, 0xb3, 0x15, 0xfd, 0x21,
	0x9f, 0x1c, 0xbb, 0x52, 0x8c, 0xb6, 0xe7, 0x4c, 0xa4, 0x9b,
	0xe1, 0x67, 0x73, 0x73, 0x4f, 0x61, 0xa1, 0xca, 0x61, 0x03,
	0x1b, 0x2b, 0xbf, 0x3d, 0x91, 0x8f, 0x2f, 0x94, 0xff, 0xc4,
	0x22, 0x8e, 0x50, 0x91, 0x95, 0x44, 0xae
};

/*
 * FVP does not support HES, so provide 0's as keys.
 */
int plat_rmmd_el3_token_sign_get_rak_pub(uintptr_t buf, size_t *len,
					 unsigned int type)
{
	(void)type;
	if (*len < sizeof(sample_attest_pub_key)) {
		return E_RMM_INVAL;
	}

	if (type != ATTEST_KEY_CURVE_ECC_SECP384R1) {
		ERROR("Invalid ECC curve specified\n");
		return E_RMM_INVAL;
	}

	*len = sizeof(sample_attest_pub_key);

	(void)memcpy((void *)buf, sample_attest_pub_key,
		     sizeof(sample_attest_pub_key));

	return 0;
}

int plat_rmmd_el3_token_sign_push_req(const struct el3_token_sign_request *req)
{
	/*
	 * TODO: Today this function is called with a lock held on the
	 * RMM<->EL3 shared buffer. In the future, we may move to a
	 * different design that may require handling multi-threaded
	 * calls to this function, for example, if we have a per CPU
	 * buffer between RMM and EL3.
	 */
	if (el3_req_valid) {
		return E_RMM_AGAIN;
	}

	el3_req = *req;

	if ((el3_req.hash_alg_id != EL3_TOKEN_SIGN_HASH_ALG_SHA384) ||
	    (el3_req.sig_alg_id != ATTEST_KEY_CURVE_ECC_SECP384R1)) {
		return E_RMM_INVAL;
	}

	el3_req_valid = true;

	return 0;
}

int plat_rmmd_el3_token_sign_pull_resp(struct el3_token_sign_response *resp)
{
	if (!el3_req_valid) {
		return E_RMM_AGAIN;
	}

	resp->rec_granule = el3_req.rec_granule;
	resp->req_ticket = el3_req.req_ticket;
	resp->sig_len = (uint16_t)sizeof(resp->signature_buf);
	/* TODO: Provide real signature */
	memset(resp->signature_buf, 0, sizeof(resp->signature_buf));

	el3_req_valid = false;

	return 0;
}
