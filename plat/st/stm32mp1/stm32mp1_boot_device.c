/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <drivers/nand.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#define SZ_512		0x200U

#if STM32MP_RAW_NAND
static int get_data_from_otp(struct nand_device *nand_dev)
{
	int result;
	uint32_t nand_param;

	/* Check if NAND parameters are stored in OTP */
	result = bsec_shadow_read_otp(&nand_param, NAND_OTP);
	if (result != BSEC_OK) {
		ERROR("BSEC: NAND_OTP Error %i\n", result);
		return -EACCES;
	}

	if (nand_param == 0U) {
		return 0;
	}

	if ((nand_param & NAND_PARAM_STORED_IN_OTP) == 0U) {
		goto ecc;
	}

	/* NAND parameter shall be read from OTP */
	if ((nand_param & NAND_WIDTH_MASK) != 0U) {
		nand_dev->buswidth = NAND_BUS_WIDTH_16;
	} else {
		nand_dev->buswidth = NAND_BUS_WIDTH_8;
	}

	switch ((nand_param & NAND_PAGE_SIZE_MASK) >> NAND_PAGE_SIZE_SHIFT) {
	case NAND_PAGE_SIZE_2K:
		nand_dev->page_size = 0x800U;
		break;

	case NAND_PAGE_SIZE_4K:
		nand_dev->page_size = 0x1000U;
		break;

	case NAND_PAGE_SIZE_8K:
		nand_dev->page_size = 0x2000U;
		break;

	default:
		ERROR("Cannot read NAND page size\n");
		return -EINVAL;
	}

	switch ((nand_param & NAND_BLOCK_SIZE_MASK) >> NAND_BLOCK_SIZE_SHIFT) {
	case NAND_BLOCK_SIZE_64_PAGES:
		nand_dev->block_size = 64U * nand_dev->page_size;
		break;

	case NAND_BLOCK_SIZE_128_PAGES:
		nand_dev->block_size = 128U * nand_dev->page_size;
		break;

	case NAND_BLOCK_SIZE_256_PAGES:
		nand_dev->block_size = 256U * nand_dev->page_size;
		break;

	default:
		ERROR("Cannot read NAND block size\n");
		return -EINVAL;
	}

	nand_dev->size = ((nand_param & NAND_BLOCK_NB_MASK) >>
			  NAND_BLOCK_NB_SHIFT) *
		NAND_BLOCK_NB_UNIT * nand_dev->block_size;

ecc:
	switch ((nand_param & NAND_ECC_BIT_NB_MASK) >>
		NAND_ECC_BIT_NB_SHIFT) {
	case NAND_ECC_BIT_NB_1_BITS:
		nand_dev->ecc.max_bit_corr = 1U;
		break;

	case NAND_ECC_BIT_NB_4_BITS:
		nand_dev->ecc.max_bit_corr = 4U;
		break;

	case NAND_ECC_BIT_NB_8_BITS:
		nand_dev->ecc.max_bit_corr = 8U;
		break;

	case NAND_ECC_ON_DIE:
		nand_dev->ecc.mode = NAND_ECC_ONDIE;
		break;

	default:
		if (nand_dev->ecc.max_bit_corr == 0U) {
			ERROR("No valid eccbit number\n");
			return -EINVAL;
		}
	}

	VERBOSE("OTP: Block %i Page %i Size %lli\n", nand_dev->block_size,
	     nand_dev->page_size, nand_dev->size);

	return 0;
}
#endif

#if STM32MP_RAW_NAND
int plat_get_raw_nand_data(struct rawnand_device *device)
{
	device->nand_dev->ecc.mode = NAND_ECC_HW;
	device->nand_dev->ecc.size = SZ_512;

	return get_data_from_otp(device->nand_dev);
}
#endif

