/*
 * Copyright (c) 2015-2020, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <lib/mmio.h>

#include "emmc_config.h"
#include "emmc_def.h"
#include "emmc_hal.h"
#include "emmc_registers.h"
#include "emmc_std.h"
#include "rcar_def.h"

static EMMC_ERROR_CODE emmc_trans_sector(uint32_t *buff_address_virtual);

uint32_t emmc_interrupt(void)
{
	EMMC_ERROR_CODE result;
	uint32_t prr_data;
	uint32_t cut_ver;
	uint32_t end_bit;

	prr_data = mmio_read_32((uintptr_t) RCAR_PRR);
	cut_ver = prr_data & PRR_CUT_MASK;
	if ((prr_data & PRR_PRODUCT_MASK) == PRR_PRODUCT_H3) {
		if (cut_ver == PRR_PRODUCT_10) {
			end_bit = BIT17;
		} else if (cut_ver == PRR_PRODUCT_11) {
			end_bit = BIT17;
		} else {
			end_bit = BIT20;
		}
	} else if ((prr_data & PRR_PRODUCT_MASK) == PRR_PRODUCT_M3) {
		if (cut_ver == PRR_PRODUCT_10) {
			end_bit = BIT17;
		} else {
			end_bit = BIT20;
		}
	} else {
		end_bit = BIT20;
	}

	/* SD_INFO */
	mmc_drv_obj.error_info.info1 = GETR_32(SD_INFO1);
	mmc_drv_obj.error_info.info2 = GETR_32(SD_INFO2);

	/* SD_INFO EVENT */
	mmc_drv_obj.int_event1 =
	    mmc_drv_obj.error_info.info1 & GETR_32(SD_INFO1_MASK);
	mmc_drv_obj.int_event2 =
	    mmc_drv_obj.error_info.info2 & GETR_32(SD_INFO2_MASK);

	/* ERR_STS */
	mmc_drv_obj.error_info.status1 = GETR_32(SD_ERR_STS1);
	mmc_drv_obj.error_info.status2 = GETR_32(SD_ERR_STS2);

	/* DM_CM_INFO */
	mmc_drv_obj.error_info.dm_info1 = GETR_32(DM_CM_INFO1);
	mmc_drv_obj.error_info.dm_info2 = GETR_32(DM_CM_INFO2);

	/* DM_CM_INFO EVENT */
	mmc_drv_obj.dm_event1 =
	    mmc_drv_obj.error_info.dm_info1 & GETR_32(DM_CM_INFO1_MASK);
	mmc_drv_obj.dm_event2 =
	    mmc_drv_obj.error_info.dm_info2 & GETR_32(DM_CM_INFO2_MASK);

	/* ERR SD_INFO2 */
	if ((SD_INFO2_ALL_ERR & mmc_drv_obj.int_event2) != 0) {
		SETR_32(SD_INFO1_MASK, 0x00000000U);	/* interrupt disable */
		SETR_32(SD_INFO2_MASK, SD_INFO2_CLEAR);	/* interrupt disable */
		SETR_32(SD_INFO1, 0x00000000U);	/* interrupt clear */
		SETR_32(SD_INFO2, SD_INFO2_CLEAR);	/* interrupt clear */
		mmc_drv_obj.state_machine_blocking = FALSE;
	}

	/* PIO Transfer */
	/* BWE/BRE */
	else if (((SD_INFO2_BWE | SD_INFO2_BRE) & mmc_drv_obj.int_event2)) {
		/* BWE */
		if (SD_INFO2_BWE & mmc_drv_obj.int_event2) {
			SETR_32(SD_INFO2, (GETR_32(SD_INFO2) & ~SD_INFO2_BWE));
		}
		/* BRE */
		else {
			SETR_32(SD_INFO2, (GETR_32(SD_INFO2) & ~SD_INFO2_BRE));
		}

		result = emmc_trans_sector(mmc_drv_obj.buff_address_virtual);
		mmc_drv_obj.buff_address_virtual += EMMC_BLOCK_LENGTH;
		mmc_drv_obj.remain_size -= EMMC_BLOCK_LENGTH;

		if (result != EMMC_SUCCESS) {
			/* data transfer error */
			emmc_write_error_info(EMMC_FUNCNO_NONE, result);

			/* Panic */
			SETR_32(SD_INFO1_MASK, 0x00000000U);
			SETR_32(SD_INFO2_MASK, SD_INFO2_CLEAR);
			SETR_32(SD_INFO1, 0x00000000U);
			/* interrupt clear */
			SETR_32(SD_INFO2, SD_INFO2_CLEAR);
			mmc_drv_obj.force_terminate = TRUE;
		} else {
			mmc_drv_obj.during_transfer = FALSE;
		}
		mmc_drv_obj.state_machine_blocking = FALSE;
	}

	/* DMA_TRANSFER */
	/* DM_CM_INFO1: DMA-ch0 transfer complete or error occurred */
	else if ((BIT16 & mmc_drv_obj.dm_event1) != 0) {
		SETR_32(DM_CM_INFO1, 0x00000000U);
		SETR_32(DM_CM_INFO2, 0x00000000U);
		/* interrupt clear */
		SETR_32(SD_INFO2, (GETR_32(SD_INFO2) & ~SD_INFO2_BWE));
		/* DM_CM_INFO2:  DMA-ch0 error occurred */
		if ((BIT16 & mmc_drv_obj.dm_event2) != 0) {
			mmc_drv_obj.dma_error_flag = TRUE;
		} else {
			mmc_drv_obj.during_dma_transfer = FALSE;
			mmc_drv_obj.during_transfer = FALSE;
		}
		/* wait next interrupt */
		mmc_drv_obj.state_machine_blocking = FALSE;
	}
	/* DM_CM_INFO1: DMA-ch1 transfer complete or error occurred */
	else if ((end_bit & mmc_drv_obj.dm_event1) != 0U) {
		SETR_32(DM_CM_INFO1, 0x00000000U);
		SETR_32(DM_CM_INFO2, 0x00000000U);
		/* interrupt clear */
		SETR_32(SD_INFO2, (GETR_32(SD_INFO2) & ~SD_INFO2_BRE));
		/* DM_CM_INFO2: DMA-ch1 error occurred */
		if ((BIT17 & mmc_drv_obj.dm_event2) != 0) {
			mmc_drv_obj.dma_error_flag = TRUE;
		} else {
			mmc_drv_obj.during_dma_transfer = FALSE;
			mmc_drv_obj.during_transfer = FALSE;
		}
		/* wait next interrupt */
		mmc_drv_obj.state_machine_blocking = FALSE;
	}

	/* Response end  */
	else if ((SD_INFO1_INFO0 & mmc_drv_obj.int_event1) != 0) {
		/* interrupt clear */
		SETR_32(SD_INFO1, (GETR_32(SD_INFO1) & ~SD_INFO1_INFO0));
		mmc_drv_obj.state_machine_blocking = FALSE;
	}
	/* Access end  */
	else if ((SD_INFO1_INFO2 & mmc_drv_obj.int_event1) != 0) {
		/* interrupt clear */
		SETR_32(SD_INFO1, (GETR_32(SD_INFO1) & ~SD_INFO1_INFO2));
		mmc_drv_obj.state_machine_blocking = FALSE;
	} else {
		/* nothing to do. */
	}

	return (uint32_t) 0;
}

static EMMC_ERROR_CODE emmc_trans_sector(uint32_t *buff_address_virtual)
{
	uint32_t length, i;
	uint64_t *bufPtrLL;

	if (buff_address_virtual == NULL) {
		return EMMC_ERR_PARAM;
	}

	if ((mmc_drv_obj.during_transfer != TRUE)
	    || (mmc_drv_obj.remain_size == 0)) {
		return EMMC_ERR_STATE;
	}

	bufPtrLL = (uint64_t *) buff_address_virtual;
	length = mmc_drv_obj.remain_size;

	/* data transefer */
	for (i = 0; i < (length >> 3); i++) {
		/* Write */
		if (mmc_drv_obj.cmd_info.dir == HAL_MEMCARD_WRITE) {
			SETR_64(SD_BUF0, *bufPtrLL);	/* buffer --> FIFO */
		}
		/* Read */
		else {
			/* Checks when the read data reaches SD_SIZE. */
			/* The BRE bit is cleared at emmc_interrupt function. */
			if (((i %
			      (uint32_t) (EMMC_BLOCK_LENGTH >>
					  EMMC_BUF_SIZE_SHIFT)) == 0U)
			    && (i != 0U)) {
				/* BRE check */
				while (((GETR_32(SD_INFO2)) & SD_INFO2_BRE) ==
				       0U) {
					/* ERROR check */
					if (((GETR_32(SD_INFO2)) &
					     SD_INFO2_ALL_ERR) != 0U) {
						return EMMC_ERR_TRANSFER;
					}
				}
				/* BRE clear */
				SETR_32(SD_INFO2,
					(uint32_t) (GETR_32(SD_INFO2) &
						    ~SD_INFO2_BRE));
			}
			*bufPtrLL = GETR_64(SD_BUF0);	/* FIFO --> buffer */
		}
		bufPtrLL++;
	}

	return EMMC_SUCCESS;
}
