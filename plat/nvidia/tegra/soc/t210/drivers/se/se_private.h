/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_PRIVATE_H
#define SE_PRIVATE_H

#include <stdbool.h>
#include <security_engine.h>

/*
 * PMC registers
 */

/* Secure scratch registers */
#define PMC_SECURE_SCRATCH4_OFFSET		0xC0U
#define PMC_SECURE_SCRATCH5_OFFSET		0xC4U
#define PMC_SECURE_SCRATCH6_OFFSET		0x224U
#define PMC_SECURE_SCRATCH7_OFFSET		0x228U
#define PMC_SECURE_SCRATCH116_OFFSET		0xB28U
#define PMC_SECURE_SCRATCH117_OFFSET		0xB2CU
#define PMC_SECURE_SCRATCH120_OFFSET		0xB38U
#define PMC_SECURE_SCRATCH121_OFFSET		0xB3CU
#define PMC_SECURE_SCRATCH122_OFFSET		0xB40U
#define PMC_SECURE_SCRATCH123_OFFSET		0xB44U

/*
 * AHB arbitration memory write queue
 */
#define ARAHB_MEM_WRQUE_MST_ID_OFFSET		0xFCU
#define ARAHB_MST_ID_SE2_MASK			(0x1U << 13)
#define ARAHB_MST_ID_SE_MASK			(0x1U << 14)

/**
 * SE registers
 */
#define TEGRA_SE_AES_KEYSLOT_COUNT		16
#define SE_MAX_LAST_BLOCK_SIZE			0xFFFFF

/* SE Status register */
#define SE_STATUS_OFFSET			0x800U
#define SE_STATUS_SHIFT				0
#define SE_STATUS_IDLE	\
		((0U) << SE_STATUS_SHIFT)
#define SE_STATUS_BUSY	\
		((1U) << SE_STATUS_SHIFT)
#define SE_STATUS(x)	\
		((x) & ((0x3U) << SE_STATUS_SHIFT))

#define SE_MEM_INTERFACE_SHIFT			2
#define SE_MEM_INTERFACE_IDLE			0
#define SE_MEM_INTERFACE_BUSY			1
#define SE_MEM_INTERFACE(x)	((x) << SE_STATUS_SHIFT)

/* SE register definitions */
#define SE_SECURITY_REG_OFFSET			0x0
#define SE_SECURITY_TZ_LOCK_SOFT_SHIFT		5
#define SE_SECURE				0x0
#define SE_SECURITY_TZ_LOCK_SOFT(x)	((x) << SE_SECURITY_TZ_LOCK_SOFT_SHIFT)

#define SE_SEC_ENG_DIS_SHIFT			1
#define SE_DISABLE_FALSE			0
#define SE_DISABLE_TRUE				1
#define SE_SEC_ENG_DISABLE(x)((x) << SE_SEC_ENG_DIS_SHIFT)

/* SE config register */
#define SE_CONFIG_REG_OFFSET			0x14U
#define SE_CONFIG_ENC_ALG_SHIFT 		12
#define SE_CONFIG_ENC_ALG_AES_ENC	\
		((1U) << SE_CONFIG_ENC_ALG_SHIFT)
#define SE_CONFIG_ENC_ALG_RNG	\
		((2U) << SE_CONFIG_ENC_ALG_SHIFT)
#define SE_CONFIG_ENC_ALG_SHA	\
		((3U) << SE_CONFIG_ENC_ALG_SHIFT)
#define SE_CONFIG_ENC_ALG_RSA	\
		((4U) << SE_CONFIG_ENC_ALG_SHIFT)
#define SE_CONFIG_ENC_ALG_NOP	\
		((0U) << SE_CONFIG_ENC_ALG_SHIFT)
#define SE_CONFIG_ENC_ALG(x)	\
		((x) & ((0xFU) << SE_CONFIG_ENC_ALG_SHIFT))

#define SE_CONFIG_DEC_ALG_SHIFT 		8
#define SE_CONFIG_DEC_ALG_AES	\
		((1U) << SE_CONFIG_DEC_ALG_SHIFT)
#define SE_CONFIG_DEC_ALG_NOP	\
		((0U) << SE_CONFIG_DEC_ALG_SHIFT)
#define SE_CONFIG_DEC_ALG(x)	\
		((x) & ((0xFU) << SE_CONFIG_DEC_ALG_SHIFT))

#define SE_CONFIG_DST_SHIFT	 		2
#define SE_CONFIG_DST_MEMORY	\
		((0U) << SE_CONFIG_DST_SHIFT)
#define SE_CONFIG_DST_HASHREG	\
		((1U) << SE_CONFIG_DST_SHIFT)
#define SE_CONFIG_DST_KEYTAB	\
		((2U) << SE_CONFIG_DST_SHIFT)
#define SE_CONFIG_DST_SRK	\
		((3U) << SE_CONFIG_DST_SHIFT)
#define SE_CONFIG_DST_RSAREG	\
		((4U) << SE_CONFIG_DST_SHIFT)
#define SE_CONFIG_DST(x)	\
		((x) & ((0x7U) << SE_CONFIG_DST_SHIFT))

#define SE_CONFIG_ENC_MODE_SHIFT		24
#define SE_CONFIG_ENC_MODE_KEY128	\
			((0UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE_KEY192	\
			((1UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE_KEY256	\
			((2UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE_SHA1				\
			((0UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE_SHA224	\
			((4UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE_SHA256	\
			((5UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE_SHA384	\
			((6UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE_SHA512	\
			((7UL) << SE_CONFIG_ENC_MODE_SHIFT)
#define SE_CONFIG_ENC_MODE(x)\
			((x) & ((0xFFUL) << SE_CONFIG_ENC_MODE_SHIFT))

#define SE_CONFIG_DEC_MODE_SHIFT		16
#define SE_CONFIG_DEC_MODE_KEY128	\
			((0UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE_KEY192	\
			((1UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE_KEY256	\
			((2UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE_SHA1				\
			((0UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE_SHA224	\
			((4UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE_SHA256	\
			((5UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE_SHA384	\
			((6UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE_SHA512	\
			((7UL) << SE_CONFIG_DEC_MODE_SHIFT)
#define SE_CONFIG_DEC_MODE(x)\
			((x) & ((0xFFUL) << SE_CONFIG_DEC_MODE_SHIFT))


/* DRBG random number generator config */
#define SE_RNG_CONFIG_REG_OFFSET		0x340

#define DRBG_MODE_SHIFT				0
#define DRBG_MODE_NORMAL		\
		((0U) << DRBG_MODE_SHIFT)
#define DRBG_MODE_FORCE_INSTANTION  \
		((1U) << DRBG_MODE_SHIFT)
#define DRBG_MODE_FORCE_RESEED	  \
		((2U) << DRBG_MODE_SHIFT)
#define SE_RNG_CONFIG_MODE(x)   \
		((x) & ((0x3U) << DRBG_MODE_SHIFT))

#define DRBG_SRC_SHIFT				2
#define DRBG_SRC_NONE	   \
		((0U) << DRBG_SRC_SHIFT)
#define DRBG_SRC_ENTROPY	\
		((1U) << DRBG_SRC_SHIFT)
#define DRBG_SRC_LFSR	   \
		((2U) << DRBG_SRC_SHIFT)
#define SE_RNG_SRC_CONFIG_MODE(x)   \
		((x) & ((0x3U) << DRBG_SRC_SHIFT))

/* DRBG random number generator entropy config */

#define SE_RNG_SRC_CONFIG_REG_OFFSET		0x344U

#define DRBG_RO_ENT_SRC_SHIFT			1
#define DRBG_RO_ENT_SRC_ENABLE	\
		((1U) << DRBG_RO_ENT_SRC_SHIFT)
#define DRBG_RO_ENT_SRC_DISABLE	\
		((0U) << DRBG_RO_ENT_SRC_SHIFT)
#define SE_RNG_SRC_CONFIG_RO_ENT_SRC(x)	\
		((x) & ((0x1U) << DRBG_RO_ENT_SRC_SHIFT))

#define DRBG_RO_ENT_SRC_LOCK_SHIFT		0
#define DRBG_RO_ENT_SRC_LOCK_ENABLE	\
		((1U) << DRBG_RO_ENT_SRC_LOCK_SHIFT)
#define DRBG_RO_ENT_SRC_LOCK_DISABLE	\
		((0U) << DRBG_RO_ENT_SRC_LOCK_SHIFT)
#define SE_RNG_SRC_CONFIG_RO_ENT_SRC_LOCK(x)	\
		((x) & ((0x1U) << DRBG_RO_ENT_SRC_LOCK_SHIFT))

#define DRBG_RO_ENT_IGNORE_MEM_SHIFT		12
#define DRBG_RO_ENT_IGNORE_MEM_ENABLE	\
		((1U) << DRBG_RO_ENT_IGNORE_MEM_SHIFT)
#define DRBG_RO_ENT_IGNORE_MEM_DISABLE	\
		((0U) << DRBG_RO_ENT_IGNORE_MEM_SHIFT)
#define SE_RNG_SRC_CONFIG_RO_ENT_IGNORE_MEM(x)	\
		((x) & ((0x1U) << DRBG_RO_ENT_IGNORE_MEM_SHIFT))

#define SE_RNG_RESEED_INTERVAL_REG_OFFSET	0x348

/* SE CRYPTO */
#define SE_CRYPTO_REG_OFFSET			0x304
#define SE_CRYPTO_HASH_SHIFT			0
#define SE_CRYPTO_HASH_DISABLE	\
		((0U) << SE_CRYPTO_HASH_SHIFT)
#define SE_CRYPTO_HASH_ENABLE	\
		((1U) << SE_CRYPTO_HASH_SHIFT)

#define SE_CRYPTO_XOR_POS_SHIFT			1
#define SE_CRYPTO_XOR_BYPASS	\
		((0U) << SE_CRYPTO_XOR_POS_SHIFT)
#define SE_CRYPTO_XOR_TOP	\
		((2U) << SE_CRYPTO_XOR_POS_SHIFT)
#define SE_CRYPTO_XOR_BOTTOM	\
		((3U) << SE_CRYPTO_XOR_POS_SHIFT)

#define SE_CRYPTO_INPUT_SEL_SHIFT		3
#define SE_CRYPTO_INPUT_AHB 	\
		((0U) << SE_CRYPTO_INPUT_SEL_SHIFT)
#define SE_CRYPTO_INPUT_RANDOM	\
		((1U) << SE_CRYPTO_INPUT_SEL_SHIFT)
#define SE_CRYPTO_INPUT_AESOUT	\
		((2U) << SE_CRYPTO_INPUT_SEL_SHIFT)
#define SE_CRYPTO_INPUT_LNR_CTR \
		((3U) << SE_CRYPTO_INPUT_SEL_SHIFT)

#define SE_CRYPTO_VCTRAM_SEL_SHIFT		 5
#define SE_CRYPTO_VCTRAM_AHB	\
		((0U) << SE_CRYPTO_VCTRAM_SEL_SHIFT)
#define SE_CRYPTO_VCTRAM_AESOUT \
		((2U) << SE_CRYPTO_VCTRAM_SEL_SHIFT)
#define SE_CRYPTO_VCTRAM_PREVAHB	\
		((3U) << SE_CRYPTO_VCTRAM_SEL_SHIFT)

#define SE_CRYPTO_IV_SEL_SHIFT			 7
#define SE_CRYPTO_IV_ORIGINAL	\
		((0U) << SE_CRYPTO_IV_SEL_SHIFT)
#define SE_CRYPTO_IV_UPDATED	\
		((1U) << SE_CRYPTO_IV_SEL_SHIFT)

#define SE_CRYPTO_CORE_SEL_SHIFT		8
#define SE_CRYPTO_CORE_DECRYPT	\
		((0U) << SE_CRYPTO_CORE_SEL_SHIFT)
#define SE_CRYPTO_CORE_ENCRYPT	\
		((1U) << SE_CRYPTO_CORE_SEL_SHIFT)

#define SE_CRYPTO_KEY_INDEX_SHIFT		24
#define SE_CRYPTO_KEY_INDEX(x) (x << SE_CRYPTO_KEY_INDEX_SHIFT)

#define SE_CRYPTO_MEMIF_AHB	\
		((0U) << SE_CRYPTO_MEMIF_SHIFT)
#define SE_CRYPTO_MEMIF_MCCIF	\
		((1U) << SE_CRYPTO_MEMIF_SHIFT)
#define SE_CRYPTO_MEMIF_SHIFT			31

/* KEY TABLE */
#define SE_KEYTABLE_REG_OFFSET			0x31C

/* KEYIV PKT - key slot */
#define SE_KEYTABLE_SLOT_SHIFT			4
#define SE_KEYTABLE_SLOT(x)	(x << SE_KEYTABLE_SLOT_SHIFT)

/* KEYIV PKT - KEYIV select */
#define SE_KEYIV_PKT_KEYIV_SEL_SHIFT		3
#define SE_CRYPTO_KEYIV_KEY	\
		((0U) << SE_KEYIV_PKT_KEYIV_SEL_SHIFT)
#define SE_CRYPTO_KEYIV_IVS	\
		((1U) << SE_KEYIV_PKT_KEYIV_SEL_SHIFT)

/* KEYIV PKT - IV select */
#define SE_KEYIV_PKT_IV_SEL_SHIFT		2
#define SE_CRYPTO_KEYIV_IVS_OIV	\
		((0U) << SE_KEYIV_PKT_IV_SEL_SHIFT)
#define SE_CRYPTO_KEYIV_IVS_UIV \
		((1U) << SE_KEYIV_PKT_IV_SEL_SHIFT)

/* KEYIV PKT - key word */
#define SE_KEYIV_PKT_KEY_WORD_SHIFT		0
#define SE_KEYIV_PKT_KEY_WORD(x)	\
		((x) << SE_KEYIV_PKT_KEY_WORD_SHIFT)

/* KEYIV PKT - iv word */
#define SE_KEYIV_PKT_IV_WORD_SHIFT		0
#define SE_KEYIV_PKT_IV_WORD(x)		\
		((x) << SE_KEYIV_PKT_IV_WORD_SHIFT)

/* SE OPERATION */
#define SE_OPERATION_REG_OFFSET 		0x8U
#define SE_OPERATION_SHIFT			0
#define SE_OP_ABORT	\
		((0x0U) << SE_OPERATION_SHIFT)
#define SE_OP_START	\
		((0x1U) << SE_OPERATION_SHIFT)
#define SE_OP_RESTART	\
		((0x2U) << SE_OPERATION_SHIFT)
#define SE_OP_CTX_SAVE	\
		((0x3U) << SE_OPERATION_SHIFT)
#define SE_OP_RESTART_IN	\
		((0x4U) << SE_OPERATION_SHIFT)
#define SE_OPERATION(x)	\
		((x) & ((0x7U) << SE_OPERATION_SHIFT))

/* SE CONTEXT */
#define SE_CTX_SAVE_CONFIG_REG_OFFSET		0x70
#define SE_CTX_SAVE_WORD_QUAD_SHIFT		0
#define SE_CTX_SAVE_WORD_QUAD(x)	\
		(x << SE_CTX_SAVE_WORD_QUAD_SHIFT)
#define SE_CTX_SAVE_WORD_QUAD_KEYS_0_3	\
		((0U) << SE_CTX_SAVE_WORD_QUAD_SHIFT)
#define SE_CTX_SAVE_WORD_QUAD_KEYS_4_7	\
		((1U) << SE_CTX_SAVE_WORD_QUAD_SHIFT)
#define SE_CTX_SAVE_WORD_QUAD_ORIG_IV	\
		((2U) << SE_CTX_SAVE_WORD_QUAD_SHIFT)
#define SE_CTX_SAVE_WORD_QUAD_UPD_IV	\
		((3U) << SE_CTX_SAVE_WORD_QUAD_SHIFT)

#define SE_CTX_SAVE_KEY_INDEX_SHIFT		8
#define SE_CTX_SAVE_KEY_INDEX(x)	(x << SE_CTX_SAVE_KEY_INDEX_SHIFT)

#define SE_CTX_SAVE_STICKY_WORD_QUAD_SHIFT	24
#define SE_CTX_SAVE_STICKY_WORD_QUAD_STICKY_0_3	\
		((0U) << SE_CTX_SAVE_STICKY_WORD_QUAD_SHIFT)
#define SE_CTX_SAVE_STICKY_WORD_QUAD_STICKY_4_7	\
		((1U) << SE_CTX_SAVE_STICKY_WORD_QUAD_SHIFT)
#define SE_CTX_SAVE_STICKY_WORD_QUAD(x)	\
		(x << SE_CTX_SAVE_STICKY_WORD_QUAD_SHIFT)

#define SE_CTX_SAVE_SRC_SHIFT			29
#define SE_CTX_SAVE_SRC_STICKY_BITS	\
		((0U) << SE_CTX_SAVE_SRC_SHIFT)
#define SE_CTX_SAVE_SRC_RSA_KEYTABLE	\
		((1U) << SE_CTX_SAVE_SRC_SHIFT)
#define SE_CTX_SAVE_SRC_AES_KEYTABLE	\
		((2U) << SE_CTX_SAVE_SRC_SHIFT)
#define SE_CTX_SAVE_SRC_PKA1_STICKY_BITS	\
		((3U) << SE_CTX_SAVE_SRC_SHIFT)
#define SE_CTX_SAVE_SRC_MEM	\
		((4U) << SE_CTX_SAVE_SRC_SHIFT)
#define SE_CTX_SAVE_SRC_SRK	\
		((6U) << SE_CTX_SAVE_SRC_SHIFT)
#define SE_CTX_SAVE_SRC_PKA1_KEYTABLE	\
		((7U) << SE_CTX_SAVE_SRC_SHIFT)

#define SE_CTX_STICKY_WORD_QUAD_SHIFT		24
#define SE_CTX_STICKY_WORD_QUAD_WORDS_0_3 \
		((0U) << SE_CTX_STICKY_WORD_QUAD_SHIFT)
#define SE_CTX_STICKY_WORD_QUAD_WORDS_4_7 \
		((1U) << SE_CTX_STICKY_WORD_QUAD_SHIFT)
#define SE_CTX_STICKY_WORD_QUAD(x)	 (x << SE_CTX_STICKY_WORD_QUAD_SHIFT)

#define SE_CTX_SAVE_RSA_KEY_INDEX_SHIFT		16
#define SE_CTX_SAVE_RSA_KEY_INDEX(x)				\
			(x << SE_CTX_SAVE_RSA_KEY_INDEX_SHIFT)

#define SE_CTX_RSA_WORD_QUAD_SHIFT		12
#define SE_CTX_RSA_WORD_QUAD(x)	\
			(x << SE_CTX_RSA_WORD_QUAD_SHIFT)

#define SE_CTX_PKA1_WORD_QUAD_L_SHIFT		0
#define SE_CTX_PKA1_WORD_QUAD_L_SIZE		\
			((true ? 4:0) - \
			(false ? 4:0) + 1)
#define SE_CTX_PKA1_WORD_QUAD_L(x)\
			(((x) << SE_CTX_PKA1_WORD_QUAD_L_SHIFT) & 0x1f)

#define SE_CTX_PKA1_WORD_QUAD_H_SHIFT		12
#define SE_CTX_PKA1_WORD_QUAD_H(x)\
			((((x) >> SE_CTX_PKA1_WORD_QUAD_L_SIZE) & 0xf) \
			<< SE_CTX_PKA1_WORD_QUAD_H_SHIFT)

#define SE_RSA_KEY_INDEX_SLOT0_EXP		0
#define SE_RSA_KEY_INDEX_SLOT0_MOD		1
#define SE_RSA_KEY_INDEX_SLOT1_EXP		2
#define SE_RSA_KEY_INDEX_SLOT1_MOD		3


/* SE_CTX_SAVE_AUTO */
#define SE_CTX_SAVE_AUTO_REG_OFFSET 		0x74U

/* Enable */
#define SE_CTX_SAVE_AUTO_ENABLE_SHIFT		0
#define SE_CTX_SAVE_AUTO_DIS	\
		((0U) << SE_CTX_SAVE_AUTO_ENABLE_SHIFT)
#define SE_CTX_SAVE_AUTO_EN	\
		((1U) << SE_CTX_SAVE_AUTO_ENABLE_SHIFT)
#define SE_CTX_SAVE_AUTO_ENABLE(x)	\
		((x) & ((0x1U) << SE_CTX_SAVE_AUTO_ENABLE_SHIFT))

/* Lock */
#define SE_CTX_SAVE_AUTO_LOCK_SHIFT 		8
#define SE_CTX_SAVE_AUTO_LOCK_EN	\
		((1U) << SE_CTX_SAVE_AUTO_LOCK_SHIFT)
#define SE_CTX_SAVE_AUTO_LOCK_DIS	\
		((0U) << SE_CTX_SAVE_AUTO_LOCK_SHIFT)
#define SE_CTX_SAVE_AUTO_LOCK(x)	\
		((x) & ((0x1U) << SE_CTX_SAVE_AUTO_LOCK_SHIFT))

/* Current context save number of blocks*/
#define SE_CTX_SAVE_AUTO_CURR_CNT_SHIFT		16
#define SE_CTX_SAVE_AUTO_CURR_CNT_MASK 		0x3FFU
#define SE_CTX_SAVE_GET_BLK_COUNT(x)	\
		(((x) >> SE_CTX_SAVE_AUTO_CURR_CNT_SHIFT) & \
		SE_CTX_SAVE_AUTO_CURR_CNT_MASK)

#define SE_CTX_SAVE_SIZE_BLOCKS_SE1		133
#define SE_CTX_SAVE_SIZE_BLOCKS_SE2	 	646

/* SE TZRAM OPERATION - only for SE1 */
#define SE_TZRAM_OPERATION			0x540U

#define SE_TZRAM_OP_MODE_SHIFT			1
#define SE_TZRAM_OP_COMMAND_INIT		1
#define SE_TZRAM_OP_COMMAND_SHIFT		0
#define SE_TZRAM_OP_MODE_SAVE		\
		((0U) << SE_TZRAM_OP_MODE_SHIFT)
#define SE_TZRAM_OP_MODE_RESTORE	\
		((1U) << SE_TZRAM_OP_MODE_SHIFT)
#define SE_TZRAM_OP_MODE(x)		\
		((x) & ((0x1U) << SE_TZRAM_OP_MODE_SHIFT))

#define SE_TZRAM_OP_BUSY_SHIFT			2
#define SE_TZRAM_OP_BUSY_OFF	\
		((0U) << SE_TZRAM_OP_BUSY_SHIFT)
#define SE_TZRAM_OP_BUSY_ON	\
		((1U) << SE_TZRAM_OP_BUSY_SHIFT)
#define SE_TZRAM_OP_BUSY(x)	\
		((x) & ((0x1U) << SE_TZRAM_OP_BUSY_SHIFT))

#define SE_TZRAM_OP_REQ_SHIFT			0
#define SE_TZRAM_OP_REQ_IDLE	\
		((0U) << SE_TZRAM_OP_REQ_SHIFT)
#define SE_TZRAM_OP_REQ_INIT	\
		((1U) << SE_TZRAM_OP_REQ_SHIFT)
#define SE_TZRAM_OP_REQ(x)	\
		((x) & ((0x1U) << SE_TZRAM_OP_REQ_SHIFT))

/* SE Interrupt */
#define SE_INT_STATUS_REG_OFFSET		0x10U
#define SE_INT_OP_DONE_SHIFT			4
#define SE_INT_OP_DONE_CLEAR	\
		((0U) << SE_INT_OP_DONE_SHIFT)
#define SE_INT_OP_DONE_ACTIVE	\
		((1U) << SE_INT_OP_DONE_SHIFT)
#define SE_INT_OP_DONE(x)	\
		((x) & ((0x1U) << SE_INT_OP_DONE_SHIFT))

/* SE TZRAM SECURITY */
#define SE_TZRAM_SEC_REG_OFFSET			0x4

#define SE_TZRAM_SEC_SETTING_SHIFT		 0
#define SE_TZRAM_SECURE		\
		((0UL) << SE_TZRAM_SEC_SETTING_SHIFT)
#define SE_TZRAM_NONSECURE	 \
		((1UL) << SE_TZRAM_SEC_SETTING_SHIFT)
#define SE_TZRAM_SEC_SETTING(x)		\
		((x) & ((0x1UL) << SE_TZRAM_SEC_SETTING_SHIFT))

/* PKA1 KEY SLOTS */
#define TEGRA_SE_PKA1_KEYSLOT_COUNT		4


/* SE error status */
#define SE_ERR_STATUS_REG_OFFSET		0x804U
#define SE_CRYPTO_KEYTABLE_DST_REG_OFFSET	0x330
#define SE_CRYPTO_KEYTABLE_DST_WORD_QUAD_SHIFT	0
#define SE_CRYPTO_KEYTABLE_DST_WORD_QUAD(x)	\
			(x << SE_CRYPTO_KEYTABLE_DST_WORD_QUAD_SHIFT)

#define SE_KEY_INDEX_SHIFT			8
#define SE_CRYPTO_KEYTABLE_DST_KEY_INDEX(x)	(x << SE_KEY_INDEX_SHIFT)


/* SE linked list (LL) register */
#define SE_IN_LL_ADDR_REG_OFFSET		0x18U
#define SE_OUT_LL_ADDR_REG_OFFSET		0x24U
#define SE_BLOCK_COUNT_REG_OFFSET		0x318U

/* AES data sizes */
#define TEGRA_SE_KEY_256_SIZE			32
#define TEGRA_SE_KEY_192_SIZE			24
#define TEGRA_SE_KEY_128_SIZE			16
#define TEGRA_SE_AES_BLOCK_SIZE 		16
#define TEGRA_SE_AES_MIN_KEY_SIZE		16
#define TEGRA_SE_AES_MAX_KEY_SIZE		32
#define TEGRA_SE_AES_IV_SIZE			16

#define TEGRA_SE_RNG_IV_SIZE			16
#define TEGRA_SE_RNG_DT_SIZE			16
#define TEGRA_SE_RNG_KEY_SIZE			16
#define TEGRA_SE_RNG_SEED_SIZE	(TEGRA_SE_RNG_IV_SIZE + \
									TEGRA_SE_RNG_KEY_SIZE + \
									TEGRA_SE_RNG_DT_SIZE)
#define TEGRA_SE_RSA512_DIGEST_SIZE		64
#define TEGRA_SE_RSA1024_DIGEST_SIZE		128
#define TEGRA_SE_RSA1536_DIGEST_SIZE		192
#define TEGRA_SE_RSA2048_DIGEST_SIZE		256

#define SE_KEY_TABLE_ACCESS_REG_OFFSET		0x284
#define SE_KEY_READ_DISABLE_SHIFT		0

#define SE_CTX_BUFER_SIZE			1072
#define SE_CTX_DRBG_BUFER_SIZE			2112

/* SE blobs size in bytes */
#define SE_CTX_SAVE_RSA_KEY_LENGTH		1024
#define SE_CTX_SAVE_RANDOM_DATA_SIZE		16
#define SE_CTX_SAVE_STICKY_BITS_SIZE		16
#define SE2_CONTEXT_SAVE_PKA1_STICKY_BITS_LENGTH 16
#define SE2_CONTEXT_SAVE_PKA1_KEYS_LENGTH	8192
#define SE_CTX_KNOWN_PATTERN_SIZE		16
#define SE_CTX_KNOWN_PATTERN_SIZE_WORDS		(SE_CTX_KNOWN_PATTERN_SIZE/4)

/* SE RSA */
#define TEGRA_SE_RSA_KEYSLOT_COUNT		2
#define SE_RSA_KEY_SIZE_REG_OFFSET		0x404
#define SE_RSA_EXP_SIZE_REG_OFFSET		0x408
#define SE_RSA_MAX_EXP_BIT_SIZE			2048
#define SE_RSA_MAX_EXP_SIZE32	\
		(SE_RSA_MAX_EXP_BIT_SIZE >> 5)
#define SE_RSA_MAX_MOD_BIT_SIZE			2048
#define SE_RSA_MAX_MOD_SIZE32	\
		(SE_RSA_MAX_MOD_BIT_SIZE >> 5)

/* SE_RSA_KEYTABLE_ADDR */
#define SE_RSA_KEYTABLE_ADDR			0x420
#define RSA_KEY_PKT_WORD_ADDR_SHIFT		0
#define RSA_KEY_PKT_EXPMOD_SEL_SHIFT	\
		((6U) << RSA_KEY_PKT_WORD_ADDR_SHIFT)
#define RSA_KEY_MOD	\
		((1U) << RSA_KEY_PKT_EXPMOD_SEL_SHIFT)
#define RSA_KEY_EXP	\
		((0U) << RSA_KEY_PKT_EXPMOD_SEL_SHIFT)
#define RSA_KEY_PKT_SLOT_SHIFT			7
#define RSA_KEY_SLOT_1	\
		((0U) << RSA_KEY_PKT_SLOT_SHIFT)
#define RSA_KEY_SLOT_2	\
		((1U) << RSA_KEY_PKT_SLOT_SHIFT)
#define RSA_KEY_PKT_INPUT_MODE_SHIFT		8
#define RSA_KEY_REG_INPUT	\
		((0U) << RSA_KEY_PKT_INPUT_MODE_SHIFT)
#define RSA_KEY_DMA_INPUT	\
		((1U) << RSA_KEY_PKT_INPUT_MODE_SHIFT)

/* SE_RSA_KEYTABLE_DATA */
#define SE_RSA_KEYTABLE_DATA			0x424

/* SE_RSA_CONFIG register */
#define SE_RSA_CONFIG				0x400
#define RSA_KEY_SLOT_SHIFT			24
#define RSA_KEY_SLOT(x) \
		((x) << RSA_KEY_SLOT_SHIFT)

/*******************************************************************************
 * Structure definition
 ******************************************************************************/

/* SE context blob */
#pragma pack(push, 1)
typedef struct tegra_aes_key_slot {
	/* 0 - 7 AES key */
	uint32_t key[8];
	/* 8 - 11 Original IV */
	uint32_t oiv[4];
	/* 12 - 15 Updated IV */
	uint32_t uiv[4];
} tegra_se_aes_key_slot_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tegra_se_context {
	/* random number */
	unsigned char rand_data[SE_CTX_SAVE_RANDOM_DATA_SIZE];
	/* Sticky bits */
	unsigned char sticky_bits[SE_CTX_SAVE_STICKY_BITS_SIZE * 2];
	/* AES key slots */
	tegra_se_aes_key_slot_t key_slots[TEGRA_SE_AES_KEYSLOT_COUNT];
	/* RSA key slots */
	unsigned char rsa_keys[SE_CTX_SAVE_RSA_KEY_LENGTH];
} tegra_se_context_t;
#pragma pack(pop)

/* PKA context blob */
#pragma pack(push, 1)
typedef struct tegra_pka_context {
	unsigned char sticky_bits[SE2_CONTEXT_SAVE_PKA1_STICKY_BITS_LENGTH];
	unsigned char pka_keys[SE2_CONTEXT_SAVE_PKA1_KEYS_LENGTH];
} tegra_pka_context_t;
#pragma pack(pop)

/* SE context blob */
#pragma pack(push, 1)
typedef struct tegra_se_context_blob {
	/* SE context */
	tegra_se_context_t se_ctx;
	/* Known Pattern */
	unsigned char known_pattern[SE_CTX_KNOWN_PATTERN_SIZE];
} tegra_se_context_blob_t;
#pragma pack(pop)

/* SE2 and PKA1 context blob */
#pragma pack(push, 1)
typedef struct tegra_se2_context_blob {
	/* SE2 context */
	tegra_se_context_t se_ctx;
	/* PKA1 context */
	tegra_pka_context_t pka_ctx;
	/* Known Pattern */
	unsigned char known_pattern[SE_CTX_KNOWN_PATTERN_SIZE];
} tegra_se2_context_blob_t;
#pragma pack(pop)

/* SE AES key type 128bit, 192bit, 256bit */
typedef enum {
	SE_AES_KEY128,
	SE_AES_KEY192,
	SE_AES_KEY256,
} tegra_se_aes_key_type_t;

/* SE RSA key slot */
typedef struct tegra_se_rsa_key_slot {
	/* 0 - 63 exponent key */
	uint32_t exponent[SE_RSA_MAX_EXP_SIZE32];
	/* 64 - 127 modulus key */
	uint32_t modulus[SE_RSA_MAX_MOD_SIZE32];
} tegra_se_rsa_key_slot_t;


/*******************************************************************************
 * Inline functions definition
 ******************************************************************************/

static inline uint32_t tegra_se_read_32(const tegra_se_dev_t *dev, uint32_t offset)
{
	return mmio_read_32(dev->se_base + offset);
}

static inline void tegra_se_write_32(const tegra_se_dev_t *dev, uint32_t offset, uint32_t val)
{
	mmio_write_32(dev->se_base + offset, val);
}

static inline uint32_t tegra_pka_read_32(tegra_pka_dev_t *dev, uint32_t offset)
{
	return mmio_read_32(dev->pka_base + offset);
}

static inline void tegra_pka_write_32(tegra_pka_dev_t *dev, uint32_t offset,
uint32_t val)
{
	mmio_write_32(dev->pka_base + offset, val);
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int tegra_se_start_normal_operation(const tegra_se_dev_t *, uint32_t);
int tegra_se_start_ctx_save_operation(const tegra_se_dev_t *, uint32_t);

#endif /* SE_PRIVATE_H */
