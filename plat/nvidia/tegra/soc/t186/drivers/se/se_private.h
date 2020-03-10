/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_PRIVATE_H
#define SE_PRIVATE_H

#include <lib/utils_def.h>

/* SE0 security register */
#define SE0_SECURITY				U(0x18)
#define SE0_SECURITY_SE_SOFT_SETTING		(((uint32_t)1) << 16U)

/* SE0 config register */
#define SE0_SHA_CONFIG				U(0x104)
#define SE0_SHA_TASK_CONFIG			U(0x108)
#define SE0_SHA_CONFIG_HW_INIT_HASH		((1U) << 0U)
#define SE0_SHA_CONFIG_HW_INIT_HASH_DISABLE	U(0)

#define SE0_CONFIG_ENC_ALG_SHIFT		U(12)
#define SE0_CONFIG_ENC_ALG_SHA	\
		(((uint32_t)3) << SE0_CONFIG_ENC_ALG_SHIFT)
#define SE0_CONFIG_DEC_ALG_SHIFT		U(8)
#define SE0_CONFIG_DEC_ALG_NOP	\
		(((uint32_t)0) << SE0_CONFIG_DEC_ALG_SHIFT)
#define SE0_CONFIG_DST_SHIFT			U(2)
#define SE0_CONFIG_DST_HASHREG	\
		(((uint32_t)1) << SE0_CONFIG_DST_SHIFT)
#define SHA256_HASH_SIZE_BYTES			U(256)

#define SE0_CONFIG_ENC_MODE_SHIFT		U(24)
#define SE0_CONFIG_ENC_MODE_SHA256	\
			(((uint32_t)5) << SE0_CONFIG_ENC_MODE_SHIFT)

/* SHA input message length */
#define SE0_SHA_MSG_LENGTH_0			U(0x11c)
#define SE0_SHA_MSG_LENGTH_1			U(0x120)
#define SE0_SHA_MSG_LENGTH_2			U(0x124)
#define SE0_SHA_MSG_LENGTH_3			U(0x128)

/* SHA input message left  */
#define SE0_SHA_MSG_LEFT_0			U(0x12c)
#define SE0_SHA_MSG_LEFT_1			U(0x130)
#define SE0_SHA_MSG_LEFT_2			U(0x134)
#define SE0_SHA_MSG_LEFT_3			U(0x138)

/* SE Hash Result */
#define SE0_SHA_HASH_RESULT_0			U(0x13c)

/* SE OPERATION */
#define SE0_OPERATION_REG_OFFSET		U(0x17c)
#define SE0_UNIT_OPERATION_PKT_LASTBUF_SHIFT	U(16)
#define SE0_UNIT_OPERATION_PKT_LASTBUF_FIELD	\
		(((uint32_t)0x1) << SE0_UNIT_OPERATION_PKT_LASTBUF_SHIFT)
#define SE0_OPERATION_SHIFT			U(0)
#define SE0_OP_START	\
		(((uint32_t)0x1) << SE0_OPERATION_SHIFT)

/* SE Interrupt */
#define SE0_SHA_INT_ENABLE			U(0x180)

#define SE0_INT_STATUS_REG_OFFSET		U(0x184)
#define SE0_INT_OP_DONE_SHIFT			U(4)
#define SE0_INT_OP_DONE_CLEAR	\
		(((uint32_t)0) << SE0_INT_OP_DONE_SHIFT)
#define SE0_INT_OP_DONE(x)	\
		((x) & (((uint32_t)0x1) << SE0_INT_OP_DONE_SHIFT))

/* SE SHA status */
#define SE0_SHA_STATUS_0			U(0x188)
#define SE0_SHA_STATUS_IDLE			U(0)

/* SE error status */
#define SE0_ERR_STATUS_REG_OFFSET		U(0x18c)
#define SE0_ERR_STATUS_CLEAR			U(0)
#define SE0_IN_ADDR				U(0x10c)
#define SE0_IN_HI_ADDR_HI			U(0x110)
#define SE0_IN_HI_ADDR_HI_0_MSB_SHIFT		U(24)

/* SE error status */
#define SECURE_SCRATCH_TZDRAM_SHA256_HASH_START	SECURE_SCRATCH_RSV63_LO
#define SECURE_SCRATCH_TZDRAM_SHA256_HASH_END	SECURE_SCRATCH_RSV66_HI

/*******************************************************************************
 * Inline functions definition
 ******************************************************************************/

static inline uint32_t tegra_se_read_32(uint32_t offset)
{
	return mmio_read_32((uint32_t)(TEGRA_SE0_BASE + offset));
}

static inline void tegra_se_write_32(uint32_t offset, uint32_t val)
{
	mmio_write_32(((uint32_t)(TEGRA_SE0_BASE + offset)), val);
}

#endif /* SE_PRIVATE_H */
