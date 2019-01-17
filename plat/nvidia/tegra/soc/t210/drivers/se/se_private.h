/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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
#define PMC_SECURE_SCRATCH4_OFFSET      	0xC0U
#define PMC_SECURE_SCRATCH5_OFFSET      	0xC4U
#define PMC_SECURE_SCRATCH6_OFFSET      	0x224U
#define PMC_SECURE_SCRATCH7_OFFSET      	0x228U
#define PMC_SECURE_SCRATCH120_OFFSET    	0xB38U
#define PMC_SECURE_SCRATCH121_OFFSET    	0xB3CU
#define PMC_SECURE_SCRATCH122_OFFSET    	0xB40U
#define PMC_SECURE_SCRATCH123_OFFSET    	0xB44U

/*
 * AHB arbitration memory write queue
 */
#define ARAHB_MEM_WRQUE_MST_ID_OFFSET		0xFCU
#define ARAHB_MST_ID_SE2_MASK			(0x1U << 13)
#define ARAHB_MST_ID_SE_MASK			(0x1U << 14)

/* SE Status register */
#define SE_STATUS_OFFSET			0x800U
#define SE_STATUS_SHIFT				0
#define SE_STATUS_IDLE	\
		((0U) << SE_STATUS_SHIFT)
#define SE_STATUS_BUSY	\
		((1U) << SE_STATUS_SHIFT)
#define SE_STATUS(x)	\
		((x) & ((0x3U) << SE_STATUS_SHIFT))

/* SE config register */
#define SE_CONFIG_REG_OFFSET    		0x14U
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

#define SE_CONFIG_DST_SHIFT     		2
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

/* DRBG random number generator config */
#define SE_RNG_CONFIG_REG_OFFSET		0x340

#define DRBG_MODE_SHIFT				0
#define DRBG_MODE_NORMAL		\
		((0UL) << DRBG_MODE_SHIFT)
#define DRBG_MODE_FORCE_INSTANTION  \
		((1UL) << DRBG_MODE_SHIFT)
#define DRBG_MODE_FORCE_RESEED	  \
		((2UL) << DRBG_MODE_SHIFT)
#define SE_RNG_CONFIG_MODE(x)   \
		((x) & ((0x3UL) << DRBG_MODE_SHIFT))

#define DRBG_SRC_SHIFT				2
#define DRBG_SRC_NONE	   \
		((0UL) << DRBG_SRC_SHIFT)
#define DRBG_SRC_ENTROPY	\
		((1UL) << DRBG_SRC_SHIFT)
#define DRBG_SRC_LFSR	   \
		((2UL) << DRBG_SRC_SHIFT)
#define SE_RNG_SRC_CONFIG_MODE(x)   \
		((x) & ((0x3UL) << DRBG_SRC_SHIFT))

/* DRBG random number generator entropy config */
#define SE_RNG_SRC_CONFIG_REG_OFFSET		0x344U

#define DRBG_RO_ENT_SRC_SHIFT       		1
#define DRBG_RO_ENT_SRC_ENABLE	\
		((1U) << DRBG_RO_ENT_SRC_SHIFT)
#define DRBG_RO_ENT_SRC_DISABLE	\
		((0U) << DRBG_RO_ENT_SRC_SHIFT)
#define SE_RNG_SRC_CONFIG_RO_ENT_SRC(x)	\
		((x) & ((0x1U) << DRBG_RO_ENT_SRC_SHIFT))

#define DRBG_RO_ENT_SRC_LOCK_SHIFT  		0
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

/* SE OPERATION */
#define SE_OPERATION_REG_OFFSET 		0x8U
#define SE_OPERATION_SHIFT      		0
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

/* SE_CTX_SAVE_AUTO */
#define SE_CTX_SAVE_AUTO_REG_OFFSET 		0x74U

/* Enable */
#define SE_CTX_SAVE_AUTO_ENABLE_SHIFT  		0
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

/* Current context save number of blocks  */
#define SE_CTX_SAVE_AUTO_CURR_CNT_SHIFT		16
#define SE_CTX_SAVE_AUTO_CURR_CNT_MASK 		0x3FFU
#define SE_CTX_SAVE_GET_BLK_COUNT(x)	\
		(((x) >> SE_CTX_SAVE_AUTO_CURR_CNT_SHIFT) & \
		SE_CTX_SAVE_AUTO_CURR_CNT_MASK)

#define SE_CTX_SAVE_SIZE_BLOCKS_SE1      	133
#define SE_CTX_SAVE_SIZE_BLOCKS_SE2     	646

/* SE TZRAM OPERATION - only for SE1 */
#define SE_TZRAM_OPERATION      		0x540U

#define SE_TZRAM_OP_MODE_SHIFT  		1
#define SE_TZRAM_OP_MODE_SAVE		\
		((0U) << SE_TZRAM_OP_MODE_SHIFT)
#define SE_TZRAM_OP_MODE_RESTORE	\
		((1U) << SE_TZRAM_OP_MODE_SHIFT)
#define SE_TZRAM_OP_MODE(x)		\
		((x) & ((0x1U) << SE_TZRAM_OP_MODE_SHIFT))

#define SE_TZRAM_OP_BUSY_SHIFT  		2
#define SE_TZRAM_OP_BUSY_OFF	\
		((0U) << SE_TZRAM_OP_BUSY_SHIFT)
#define SE_TZRAM_OP_BUSY_ON	\
		((1U) << SE_TZRAM_OP_BUSY_SHIFT)
#define SE_TZRAM_OP_BUSY(x)	\
		((x) & ((0x1U) << SE_TZRAM_OP_BUSY_SHIFT))

#define SE_TZRAM_OP_REQ_SHIFT  			0
#define SE_TZRAM_OP_REQ_IDLE	\
		((0U) << SE_TZRAM_OP_REQ_SHIFT)
#define SE_TZRAM_OP_REQ_INIT	\
		((1U) << SE_TZRAM_OP_REQ_SHIFT)
#define SE_TZRAM_OP_REQ(x)	\
		((x) & ((0x1U) << SE_TZRAM_OP_REQ_SHIFT))

/* SE Interrupt */
#define SE_INT_STATUS_REG_OFFSET		0x10U
#define SE_INT_OP_DONE_SHIFT    		4
#define SE_INT_OP_DONE_CLEAR	\
		((0U) << SE_INT_OP_DONE_SHIFT)
#define SE_INT_OP_DONE_ACTIVE	\
		((1U) << SE_INT_OP_DONE_SHIFT)
#define SE_INT_OP_DONE(x)	\
		((x) & ((0x1U) << SE_INT_OP_DONE_SHIFT))

/* SE error status */
#define SE_ERR_STATUS_REG_OFFSET		0x804U

/* SE linked list (LL) register */
#define SE_IN_LL_ADDR_REG_OFFSET		0x18U
#define SE_OUT_LL_ADDR_REG_OFFSET  		0x24U
#define SE_BLOCK_COUNT_REG_OFFSET  		0x318U

/* AES data sizes */
#define TEGRA_SE_AES_BLOCK_SIZE 		16
#define TEGRA_SE_AES_MIN_KEY_SIZE  		16
#define TEGRA_SE_AES_MAX_KEY_SIZE  		32
#define TEGRA_SE_AES_IV_SIZE    		16

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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#endif /* SE_PRIVATE_H */
