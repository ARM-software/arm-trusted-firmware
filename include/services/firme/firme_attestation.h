/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_ATTESTATION_H
#define FIRME_ATTESTATION_H

#include <stddef.h>
#include <stdint.h>

#include <services/firme_svc.h>

#define FIRME_ATTESTATION_VERSION_MAJOR		U(1)
#define FIRME_ATTESTATION_VERSION_MINOR		U(0)

/* Attestation service feature register definitions. */
#define FIRME_ATTESTATION_FEATURE_REG_COUNT			U(2)
#define FIRME_ATTEST_FEAT_REG0_PAT_GET_BIT			BIT(0)
#define FIRME_ATTEST_FEAT_REG0_RAK_GET_BIT			BIT(1)
#define FIRME_ATTEST_FEAT_REG0_RAT_SIGN_BIT			BIT(2)
#define FIRME_ATTEST_FEAT_REG0_PAT_EXT_CLAIMS_STAGE_BIT		BIT(3)
#define FIRME_ATTEST_FEAT_REG0_PAT_EXT_CLAIMS_FINALISE_BIT	BIT(4)
#define FIRME_ATTEST_FEAT_REG0_PAT_EXT_CLAIMS_CLEAR_BIT		BIT(5)
#define FIRME_ATTEST_FEAT_REG1_MAX_PAT_PG_CNT_SHIFT		U(0)
#define FIRME_ATTEST_FEAT_REG1_MAX_PAT_PG_CNT_MASK		U(0xFF)

#ifndef FIRME_ATTEST_MAX_PAT_PG_CNT
#define FIRME_ATTEST_MAX_PAT_PG_CNT		U(1)
#endif

#define FIRME_ATTEST_FNUM_PAT_GET		U(0x8)
#define FIRME_ATTEST_FNUM_RAK_GET		U(0x9)
#define FIRME_ATTEST_FNUM_RAT_SIGN		U(0xA)
#define FIRME_ATTEST_FNUM_PAT_EXT_CLAIMS_STAGE	U(0xB)
#define FIRME_ATTEST_FNUM_PAT_EXT_CLAIMS_CLEAR	U(0xC)
#define FIRME_ATTEST_FNUM_PAT_EXT_CLAIMS_FINALISE U(0xD)

#define FIRME_ATTEST_PAT_GET_FID FIRME_FID(FIRME_ATTEST_FNUM_PAT_GET)
#define FIRME_ATTEST_RAK_GET_FID FIRME_FID(FIRME_ATTEST_FNUM_RAK_GET)
#define FIRME_ATTEST_RAT_SIGN_FID FIRME_FID(FIRME_ATTEST_FNUM_RAT_SIGN)
#define FIRME_ATTEST_PAT_EXT_CLAIMS_STAGE_FID	\
	FIRME_FID(FIRME_ATTEST_FNUM_PAT_EXT_CLAIMS_STAGE)
#define FIRME_ATTEST_PAT_EXT_CLAIMS_CLEAR_FID	\
	FIRME_FID(FIRME_ATTEST_FNUM_PAT_EXT_CLAIMS_CLEAR)
#define FIRME_ATTEST_PAT_EXT_CLAIMS_FINALISE_FID	\
	FIRME_FID(FIRME_ATTEST_FNUM_PAT_EXT_CLAIMS_FINALISE)

/**
 * @brief Return the platform shared buffer for a FIRME instance.
 *
 * @param instance           FIRME instance that owns the shared buffer.
 * @param plat_buf_page_cnt  Number of pages in the shared buffer.
 *
 * @return Base address of the shared buffer, or 0 if none is configured.
 */
uintptr_t firme_plat_shared_buf_addr(firme_instance_e instance,
				     size_t *plat_buf_page_cnt);

/**
 * @brief Platform hook to retrieve the platform attestation token.
 *
 * A non-zero challenge_size requests a fresh token; zero continues the previous
 * request. Up to shared_buf_sz bytes are written to shared_buf, with progress
 * reported through write_size and remaining_size.
 *
 * @param instance        FIRME instance that requested the token.
 * @param shared_buf      Address of the buffer where token data is written.
 * @param shared_buf_sz   Size in bytes of @p shared_buf.
 * @param challenge       Address of the platform challenge buffer.
 * @param challenge_size  Size in bytes of @p challenge, or 0 for continuation.
 * @param write_size      Output size in bytes written to @p shared_buf.
 * @param remaining_size  Output size in bytes pending retrieval.
 *
 * @return Status code:
 * @retval FIRME_SUCCESS             Token data was written successfully.
 * @retval FIRME_NOT_SUPPORTED       Platform token retrieval is not supported.
 * @retval FIRME_INVALID_PARAMETERS  Invalid input parameters or continuation
 *                                   state.
 * @retval FIRME_BUSY                Temporary failure; caller should retry later.
 * @retval FIRME_ABORTED             Token retrieval was aborted.
 * @retval FIRME_INCOMPLETE          Token retrieval has not completed.
 */
int32_t firme_attest_plat_get_token(firme_instance_e instance,
				    uintptr_t shared_buf, size_t shared_buf_sz,
				    uintptr_t challenge, size_t challenge_size,
				    size_t *write_size, size_t *remaining_size);

#endif /* FIRME_ATTESTATION_H */
