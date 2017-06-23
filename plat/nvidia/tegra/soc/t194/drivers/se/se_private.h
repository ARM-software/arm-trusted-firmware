/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SE_PRIVATE_H__
#define __SE_PRIVATE_H__

#include <lib/utils_def.h>

/* SE0_INT_ENABLE_0 */
#define SE0_INT_ENABLE				U(0x88)
#define  SE0_DISABLE_ALL_INT			U(0x0)

/* SE0_INT_STATUS_0 */
#define SE0_INT_STATUS				U(0x8C)
#define  SE0_CLEAR_ALL_INT_STATUS		U(0x3F)

/* SE0_SHA_INT_STATUS_0 */
#define SHA_INT_STATUS				U(0x184)
#define  SHA_SE_OP_DONE				(U(1) << 4)

/* SE0_SHA_ERR_STATUS_0 */
#define SHA_ERR_STATUS				U(0x18C)

/* SE0_AES0_INT_STATUS_0 */
#define AES0_INT_STATUS				U(0x2F0)
#define  AES0_SE_OP_DONE			(U(1) << 4)

/* SE0_AES0_ERR_STATUS_0 */
#define AES0_ERR_STATUS				U(0x2F8)

/* SE0_AES1_INT_STATUS_0 */
#define AES1_INT_STATUS				U(0x4F0)

/* SE0_AES1_ERR_STATUS_0 */
#define AES1_ERR_STATUS				U(0x4F8)

/* SE0_RSA_INT_STATUS_0 */
#define RSA_INT_STATUS				U(0x758)

/* SE0_RSA_ERR_STATUS_0 */
#define RSA_ERR_STATUS				U(0x760)

/* SE0_AES0_OPERATION_0 */
#define AES0_OPERATION				U(0x238)
#define  OP_MASK_BITS				U(0x7)
#define  SE_OP_CTX_SAVE				U(0x3)

/* SE0_AES0_CTX_SAVE_CONFIG_0 */
#define	CTX_SAVE_CONFIG				U(0x2D4)

/* SE0_AES0_CTX_SAVE_AUTO_STATUS_0 */
#define CTX_SAVE_AUTO_STATUS			U(0x300)
#define  CTX_SAVE_AUTO_SE_READY			U(0xFF)
#define	 CTX_SAVE_AUTO_SE_BUSY			(U(0x1) << 31)

/* SE0_AES0_CTX_SAVE_AUTO_CTRL_0 */
#define CTX_SAVE_AUTO_CTRL			U(0x304)
#define	 SE_CTX_SAVE_AUTO_EN			(U(0x1) << 0)
#define	 SE_CTX_SAVE_AUTO_LOCK_EN		(U(0x1) << 1)

/* SE0_AES0_CTX_SAVE_AUTO_START_ADDR_0 */
#define CTX_SAVE_AUTO_START_ADDR		U(0x308)

/* SE0_AES0_CTX_SAVE_AUTO_START_ADDR_HI_0 */
#define CTX_SAVE_AUTO_START_ADDR_HI		U(0x30C)

/*******************************************************************************
 * Inline functions definition
 ******************************************************************************/

static inline uint32_t tegra_se_read_32(uint32_t offset)
{
	return mmio_read_32(TEGRA_SE0_BASE + offset);
}

static inline void tegra_se_write_32(uint32_t offset, uint32_t val)
{
	mmio_write_32(TEGRA_SE0_BASE + offset, val);
}

#endif /* __SE_PRIVATE_H__ */
