/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stddef.h>

#include "bcm_emmc.h"
#include "emmc_chal_types.h"
#include "emmc_chal_sd.h"
#include "emmc_csl_sdprot.h"
#include "emmc_csl_sdcmd.h"
#include "emmc_csl_sd.h"
#include "emmc_chal_sd.h"
#include "emmc_pboot_hal_memory_drv.h"

int sd_cmd0(struct sd_handle *handle)
{
	int res;
	uint32_t argument = 0x0; /* Go to IDLE state. */

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_GO_IDLE_STATE, argument, 0, NULL);

	if (res == SD_OK) {
		/* Clear all other interrupts */
		chal_sd_clear_irq((void *)handle->device, 0xffffffff);
	}

	return res;
}

int sd_cmd1(struct sd_handle *handle, uint32_t ocr, uint32_t *ocr_output)
{
	int res;
	uint32_t options;
	struct sd_resp resp;

	options = SD_CMDR_RSP_TYPE_R3_4 << SD_CMDR_RSP_TYPE_S;

	if (ocr_output == NULL) {
		EMMC_TRACE("Invalid args\n");
		return SD_FAIL;
	}

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_SEND_OPCOND, ocr, options, &resp);

	if (res == SD_OK)
		*ocr_output = resp.data.r3.ocr;

	return res;
}

int sd_cmd2(struct sd_handle *handle)
{
	uint32_t options;
	struct sd_resp resp;

	/* send cmd and parse result */
	options = SD_CMDR_RSP_TYPE_R2 << SD_CMDR_RSP_TYPE_S;

	return send_cmd(handle, SD_CMD_ALL_SEND_CID, 0, options, &resp);
}

int sd_cmd3(struct sd_handle *handle)
{
	int res;
	uint32_t options = 0;
	uint32_t argument;
	struct sd_resp resp;

	/* use non zero and non 0x1 value for rca */
	handle->device->ctrl.rca = 0x5;
	argument = handle->device->ctrl.rca << SD_CMD7_ARG_RCA_SHIFT;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_MMC_SET_RCA, argument, options, &resp);

	if (res != SD_OK)
		handle->device->ctrl.rca = 0;

	return res;
}

int sd_cmd7(struct sd_handle *handle, uint32_t rca)
{
	int res;
	uint32_t argument, options;
	struct sd_resp resp;

	argument = (rca << SD_CMD7_ARG_RCA_SHIFT);

	/*
	 * Response to CMD7 is:
	 * R1 while selectiing from Stand-By State to Transfer State
	 * R1b while selecting from Disconnected State to Programming State.
	 *
	 * In this driver, we only issue a CMD7 once, to go to transfer mode
	 * during init_mmc_card().
	 */
	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_SELECT_DESELECT_CARD, argument, options,
		       &resp);

	if (res == SD_OK)
		/* Clear all other interrupts */
		chal_sd_clear_irq((void *)handle->device, 0xffffffff);

	return res;
}


/*
 * CMD8 Get CSD_EXT
 */
int mmc_cmd8(struct sd_handle *handle, uint8_t *extCsdReg)
{
	uint32_t res, options;
	struct sd_resp resp;

	data_xfer_setup(handle, extCsdReg, CEATA_EXT_CSDBLOCK_SIZE,
			    SD_XFER_CARD_TO_HOST);

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_DPS_MASK | SD4_EMMC_TOP_CMD_DTDS_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK | SD4_EMMC_TOP_CMD_CRC_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_READ_EXT_CSD, 0, options, &resp);

	if (res == SD_OK)
		res = process_data_xfer(handle, extCsdReg, 0,
					CEATA_EXT_CSDBLOCK_SIZE,
					SD_XFER_CARD_TO_HOST);

	return res;
}

int sd_cmd9(struct sd_handle *handle, struct sd_card_data *card)
{
	int res;
	uint32_t argument, options, iBlkNum, multiFactor = 1;
	uint32_t maxReadBlockLen = 1, maxWriteBlockLen = 1;
	struct sd_resp resp;

	argument = handle->device->ctrl.rca << SD_CMD7_ARG_RCA_SHIFT;

	options = SD_CMDR_RSP_TYPE_R2 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_SEND_CSD, argument, options, &resp);

	if (res != SD_OK)
		return res;

	if (handle->card->type == SD_CARD_MMC) {
		card->csd.mmc.structure = (resp.data.r2.rsp4 >> 22) & 0x3;
		card->csd.mmc.csdSpecVer = (resp.data.r2.rsp4 >> 18) & 0x0f;
		card->csd.mmc.taac = (resp.data.r2.rsp4 >> 8) & 0xff;
		card->csd.mmc.nsac = resp.data.r2.rsp4 & 0xff;
		card->csd.mmc.speed = resp.data.r2.rsp3 >> 24;
		card->csd.mmc.classes = (resp.data.r2.rsp3 >> 12) & 0xfff;
		card->csd.mmc.rdBlkLen = (resp.data.r2.rsp3 >> 8) & 0xf;
		card->csd.mmc.rdBlkPartial = (resp.data.r2.rsp3 >> 7) & 0x01;
		card->csd.mmc.wrBlkMisalign = (resp.data.r2.rsp3 >> 6) & 0x1;
		card->csd.mmc.rdBlkMisalign = (resp.data.r2.rsp3 >> 5) & 0x1;
		card->csd.mmc.dsr = (resp.data.r2.rsp2 >> 4) & 0x01;
		card->csd.mmc.size =
		    ((resp.data.r2.rsp3 & 0x3) << 10) +
		    ((resp.data.r2.rsp2 >> 22) & 0x3ff);
		card->csd.mmc.vddRdCurrMin = (resp.data.r2.rsp2 >> 19) & 0x7;
		card->csd.mmc.vddRdCurrMax = (resp.data.r2.rsp2 >> 16) & 0x7;
		card->csd.mmc.vddWrCurrMin = (resp.data.r2.rsp2 >> 13) & 0x7;
		card->csd.mmc.vddWrCurrMax = (resp.data.r2.rsp2 >> 10) & 0x7;
		card->csd.mmc.devSizeMulti = (resp.data.r2.rsp2 >> 7) & 0x7;
		card->csd.mmc.eraseGrpSize = (resp.data.r2.rsp2 >> 2) & 0x1f;
		card->csd.mmc.eraseGrpSizeMulti =
		    ((resp.data.r2.rsp2 & 0x3) << 3) +
		    ((resp.data.r2.rsp1 >> 29) & 0x7);
		card->csd.mmc.wrProtGroupSize =
		    ((resp.data.r2.rsp1 >> 24) & 0x1f);
		card->csd.mmc.wrProtGroupEnable =
		    (resp.data.r2.rsp1 >> 23) & 0x1;
		card->csd.mmc.manuDefEcc = (resp.data.r2.rsp1 >> 21) & 0x3;
		card->csd.mmc.wrSpeedFactor = (resp.data.r2.rsp1 >> 18) & 0x7;
		card->csd.mmc.wrBlkLen = (resp.data.r2.rsp1 >> 14) & 0xf;
		card->csd.mmc.wrBlkPartial = (resp.data.r2.rsp1 >> 13) & 0x1;
		card->csd.mmc.protAppl = (resp.data.r2.rsp1 >> 8) & 0x1;
		card->csd.mmc.copyFlag = (resp.data.r2.rsp1 >> 7) & 0x1;
		card->csd.mmc.permWrProt = (resp.data.r2.rsp1 >> 6) & 0x1;
		card->csd.mmc.tmpWrProt = (resp.data.r2.rsp1 >> 5) & 0x1;
		card->csd.mmc.fileFormat = (resp.data.r2.rsp1 >> 4) & 0x03;
		card->csd.mmc.eccCode = resp.data.r2.rsp1 & 0x03;
		maxReadBlockLen <<= card->csd.mmc.rdBlkLen;
		maxWriteBlockLen <<= card->csd.mmc.wrBlkLen;

		iBlkNum = card->csd.mmc.size + 1;
		multiFactor = (1 << (card->csd.mmc.devSizeMulti + 2));

		handle->card->size =
		    iBlkNum * multiFactor * (1 << card->csd.mmc.rdBlkLen);
	}

	handle->card->maxRdBlkLen = maxReadBlockLen;
	handle->card->maxWtBlkLen = maxWriteBlockLen;

	if (handle->card->size < 0xA00000) {
		/*
		 * 10MB Too small size mean, cmd9 response is wrong,
		 * Use default value 1G
		 */
		handle->card->size = 0x40000000;
		handle->card->maxRdBlkLen = 512;
		handle->card->maxWtBlkLen = 512;
	}

	if ((handle->card->maxRdBlkLen > 512) ||
	    (handle->card->maxWtBlkLen > 512)) {
		handle->card->maxRdBlkLen = 512;
		handle->card->maxWtBlkLen = 512;
	} else if ((handle->card->maxRdBlkLen == 0) ||
		   (handle->card->maxWtBlkLen == 0)) {
		handle->card->maxRdBlkLen = 512;
		handle->card->maxWtBlkLen = 512;
	}

	handle->device->cfg.blockSize = handle->card->maxRdBlkLen;

	return res;
}

int sd_cmd13(struct sd_handle *handle, uint32_t *status)
{
	int res;
	uint32_t argument, options;
	struct sd_resp resp;

	argument = handle->device->ctrl.rca << SD_CMD7_ARG_RCA_SHIFT;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_SEND_STATUS, argument, options, &resp);

	if (res == SD_OK) {
		*status = resp.cardStatus;
	}

	return res;
}

int sd_cmd16(struct sd_handle *handle, uint32_t length)
{
	int res;
	uint32_t argument, options, ntry;
	struct sd_resp resp;

	argument = length;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK;

	ntry = 0;
	do {
		res = sd_cmd13(handle, &resp.cardStatus);
		if (res != SD_OK) {
			EMMC_TRACE(
				"cmd13 failed before cmd16: rca 0x%0x, return %d, response 0x%0x\n",
				handle->device->ctrl.rca, res, resp.cardStatus);
			return res;
		}

		if (resp.cardStatus & 0x100)
			break;

		EMMC_TRACE("cmd13 rsp:0x%08x before cmd16\n", resp.cardStatus);

		if (ntry > handle->device->cfg.retryLimit) {
			EMMC_TRACE("cmd13 retry reach limit %d\n",
				   handle->device->cfg.retryLimit);
			return SD_CMD_TIMEOUT;
		}

		ntry++;
		EMMC_TRACE("cmd13 retry %d\n", ntry);

		SD_US_DELAY(1000);

	} while (1);

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_SET_BLOCKLEN, argument, options, &resp);

	return res;
}

int sd_cmd17(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer)
{
	int res;
	uint32_t argument, options, ntry;
	struct sd_resp resp;

	ntry = 0;
	do {
		res = sd_cmd13(handle, &resp.cardStatus);
		if (res != SD_OK) {
			EMMC_TRACE(
				"cmd 13 failed before cmd17: rca 0x%0x, return %d, response 0x%0x\n",
				handle->device->ctrl.rca, res, resp.cardStatus);
			return res;
		}

		if (resp.cardStatus & 0x100)
			break;

		EMMC_TRACE("cmd13 rsp:0x%08x before cmd17\n", resp.cardStatus);

		if (ntry > handle->device->cfg.retryLimit) {
			EMMC_TRACE("cmd13 retry reach limit %d\n",
				   handle->device->cfg.retryLimit);
			return SD_CMD_TIMEOUT;
		}

		ntry++;
		EMMC_TRACE("cmd13 retry %d\n", ntry);

		SD_US_DELAY(1000);

	} while (1);

	data_xfer_setup(handle, buffer, len, SD_XFER_CARD_TO_HOST);

	/* send cmd and parse result */
	argument = addr;
	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_DPS_MASK | SD4_EMMC_TOP_CMD_DTDS_MASK |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK | SD4_EMMC_TOP_CMD_CCHK_EN_MASK;

	res = send_cmd(handle, SD_CMD_READ_SINGLE_BLOCK, argument, options,
		       &resp);

	if (res != SD_OK)
		return res;

	res = process_data_xfer(handle, buffer, addr, len, SD_XFER_CARD_TO_HOST);

	return res;
}

int sd_cmd18(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer)
{
	int res;
	uint32_t argument, options, ntry;
	struct sd_resp resp;

	ntry = 0;
	do {
		res = sd_cmd13(handle, &resp.cardStatus);
		if (res != SD_OK) {
			EMMC_TRACE(
				"cmd 13 failed before cmd18: rca 0x%0x, return %d, response 0x%0x\n",
				handle->device->ctrl.rca, res, resp.cardStatus);
			return res;
		}

		if (resp.cardStatus & 0x100)
			break;

		EMMC_TRACE("cmd13 rsp:0x%08x before cmd18\n", resp.cardStatus);

		if (ntry > handle->device->cfg.retryLimit) {
			EMMC_TRACE("cmd13 retry reach limit %d\n",
				   handle->device->cfg.retryLimit);
			return SD_CMD_TIMEOUT;
		}

		ntry++;
		EMMC_TRACE("cmd13 retry %d\n", ntry);

		SD_US_DELAY(1000);
	} while (1);

	data_xfer_setup(handle, buffer, len, SD_XFER_CARD_TO_HOST);

	argument = addr;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_DPS_MASK | SD4_EMMC_TOP_CMD_DTDS_MASK |
		  SD4_EMMC_TOP_CMD_MSBS_MASK | SD4_EMMC_TOP_CMD_CCHK_EN_MASK |
		  SD4_EMMC_TOP_CMD_BCEN_MASK | SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  BIT(SD4_EMMC_TOP_CMD_ACMDEN_SHIFT);

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_READ_MULTIPLE_BLOCK, argument, options,
		       &resp);

	if (res != SD_OK)
		return res;

	res = process_data_xfer(handle, buffer, addr, len, SD_XFER_CARD_TO_HOST);

	return res;
}

#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
static int card_sts_resp(struct sd_handle *handle, uint32_t *status)
{
	int res;
	uint32_t ntry = 0;

	do {
		res = sd_cmd13(handle, status);
		if (res != SD_OK) {
			EMMC_TRACE(
				"cmd 13 failed before cmd35: rca 0x%0x, return %d\n",
				handle->device->ctrl.rca, res);
			return res;
		}

		if (*status & 0x100)
			break;

		EMMC_TRACE("cmd13 rsp:0x%08x before cmd35\n", resp.cardStatus);

		if (ntry > handle->device->cfg.retryLimit) {
			EMMC_TRACE("cmd13 retry reach limit %d\n",
				   handle->device->cfg.retryLimit);
			return SD_CMD_TIMEOUT;
		}

		ntry++;
		EMMC_TRACE("cmd13 retry %d\n", ntry);

		SD_US_DELAY(1000);
	} while (1);

	return SD_OK;
}

int sd_cmd35(struct sd_handle *handle, uint32_t start)
{
	int res;
	uint32_t argument, options;
	struct sd_resp resp;

	res = card_sts_resp(handle, &resp.cardStatus);
	if (res != SD_OK)
		return res;

	argument = start;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_ERASE_GROUP_START,
		       argument, options, &resp);

	if (res != SD_OK)
		return res;

	return res;
}

int sd_cmd36(struct sd_handle *handle, uint32_t end)
{
	int res;
	uint32_t argument, options;
	struct sd_resp resp;

	res = card_sts_resp(handle, &resp.cardStatus);
	if (res != SD_OK)
		return res;

	argument = end;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_ERASE_GROUP_END,
		       argument, options, &resp);

	if (res != SD_OK)
		return res;

	return res;
}

int sd_cmd38(struct sd_handle *handle)
{
	int res;
	uint32_t argument, options;
	struct sd_resp resp;

	res = card_sts_resp(handle, &resp.cardStatus);
	if (res != SD_OK)
		return res;

	argument = 0;

	options = (SD_CMDR_RSP_TYPE_R1b_5b << SD_CMDR_RSP_TYPE_S) |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_ERASE, argument, options, &resp);

	if (res != SD_OK)
		return res;

	return res;
}
#endif

#ifdef INCLUDE_EMMC_DRIVER_WRITE_CODE

int sd_cmd24(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer)
{
	int res;
	uint32_t argument, options, ntry;
	struct sd_resp resp;

	ntry = 0;
	do {
		res = sd_cmd13(handle, &resp.cardStatus);
		if (res != SD_OK) {
			EMMC_TRACE(
				"cmd 13 failed before cmd24: rca 0x%0x, return %d, response 0x%0x\n",
				handle->device->ctrl.rca, res, &resp.cardStatus);
			return res;
		}

		if (resp.cardStatus & 0x100)
			break;

		EMMC_TRACE("cmd13 rsp:0x%08x before cmd24\n", resp.cardStatus);

		if (ntry > handle->device->cfg.retryLimit) {
			EMMC_TRACE("cmd13 retry reach limit %d\n",
				   handle->device->cfg.retryLimit);
			return SD_CMD_TIMEOUT;
		}

		ntry++;
		EMMC_TRACE("cmd13 retry %d\n", ntry);

		SD_US_DELAY(1000);

	} while (1);

	data_xfer_setup(handle, buffer, len, SD_XFER_HOST_TO_CARD);

	argument = addr;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_DPS_MASK | SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK;

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_WRITE_BLOCK, argument, options, &resp);

	if (res != SD_OK)
		return res;

	res = process_data_xfer(handle, buffer, addr, len, SD_XFER_HOST_TO_CARD);

	return res;
}

int sd_cmd25(struct sd_handle *handle,
	     uint32_t addr, uint32_t len, uint8_t *buffer)
{
	int res = SD_OK;
	uint32_t argument, options, ntry;
	struct sd_resp resp;

	ntry = 0;
	do {
		res = sd_cmd13(handle, &resp.cardStatus);
		if (res != SD_OK) {
			EMMC_TRACE(
				"cmd 13 failed before cmd25: rca 0x%0x, return %d, response 0x%0x\n",
				handle->device->ctrl.rca, res, &resp.cardStatus);
			return res;
		}

		if (resp.cardStatus & 0x100)
			break;

		EMMC_TRACE("cmd13 rsp:0x%08x before cmd25\n", resp.cardStatus);

		if (ntry > handle->device->cfg.retryLimit) {
			EMMC_TRACE("cmd13 retry reach limit %d\n",
				   handle->device->cfg.retryLimit);
			return SD_CMD_TIMEOUT;
		}

		ntry++;
		EMMC_TRACE("cmd13 retry %d\n", ntry);

		SD_US_DELAY(1000);
	} while (1);

	data_xfer_setup(handle, buffer, len, SD_XFER_HOST_TO_CARD);

	argument = addr;

	options = SD_CMDR_RSP_TYPE_R1_5_6 << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_DPS_MASK | SD4_EMMC_TOP_CMD_MSBS_MASK |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK | SD4_EMMC_TOP_CMD_BCEN_MASK |
		  SD4_EMMC_TOP_CMD_CRC_EN_MASK |
		  BIT(SD4_EMMC_TOP_CMD_ACMDEN_SHIFT);

	/* send cmd and parse result */
	res = send_cmd(handle, SD_CMD_WRITE_MULTIPLE_BLOCK,
		       argument, options, &resp);

	if (res != SD_OK)
		return res;

	res = process_data_xfer(handle, buffer, addr, len, SD_XFER_HOST_TO_CARD);

	return res;
}
#endif /* INCLUDE_EMMC_DRIVER_WRITE_CODE */

int mmc_cmd6(struct sd_handle *handle, uint32_t argument)
{
	int res;
	uint32_t options;
	struct sd_resp resp;

	options = SD_CMDR_RSP_TYPE_R1b_5b << SD_CMDR_RSP_TYPE_S |
		  SD4_EMMC_TOP_CMD_CCHK_EN_MASK | SD4_EMMC_TOP_CMD_CRC_EN_MASK;

	EMMC_TRACE("Sending CMD6 with argument 0x%X\n", argument);

	/* send cmd and parse result */
	res = send_cmd(handle, SD_ACMD_SET_BUS_WIDTH, argument, options, &resp);

	/*
	 * For R1b type response:
	 * controller issues a COMMAND COMPLETE interrupt when the R1
	 * response is received,
	 * then controller monitors DAT0 for busy status,
	 * controller issues a TRANSFER COMPLETE interrupt when busy signal
	 * clears.
	 */
	wait_for_event(handle,
		       SD4_EMMC_TOP_INTR_TXDONE_MASK | SD_ERR_INTERRUPTS,
		       handle->device->cfg.wfe_retry);

	if (res == SD_OK) {
		/* Check result of Cmd6 using Cmd13 to check card status */

		/* Check status using Cmd13 */
		res = sd_cmd13(handle, &resp.cardStatus);

		if (res == SD_OK) {
			/* Check bit 7 (SWITCH_ERROR) in card status */
			if ((resp.cardStatus & 0x80) != 0) {
				EMMC_TRACE("cmd6 failed: SWITCH_ERROR\n");
				res = SD_FAIL;
			}
		} else {
			EMMC_TRACE("cmd13 failed after cmd6: ");
			EMMC_TRACE("rca 0x%0x, return %d, response 0x%0x\n",
				handle->device->ctrl.rca, res, resp.cardStatus);
		}
	}

	return res;
}


#define SD_BUSY_CHECK		0x00203000
#define DAT0_LEVEL_MASK		0x100000	/* bit20 in PSTATE */
#define DEV_BUSY_TIMEOUT	600000		/* 60 Sec : 600000 * 100us */

int send_cmd(struct sd_handle *handle, uint32_t cmdIndex, uint32_t argument,
	     uint32_t options, struct sd_resp *resp)
{
	int status = SD_OK;
	uint32_t event = 0, present, timeout = 0, retry = 0, mask = 3;
	uint32_t temp_resp[4];

	if (handle == NULL) {
		EMMC_TRACE("Invalid handle for cmd%d\n", cmdIndex);
		return SD_INVALID_HANDLE;
	}

	mask = (SD_BUSY_CHECK & options) ? 3 : 1;

RETRY_WRITE_CMD:
	do {
		/* Make sure it is ok to send command */
		present =
		    chal_sd_get_present_status((CHAL_HANDLE *) handle->device);
		timeout++;

		if (present & mask)
			SD_US_DELAY(1000);
		else
			break;

	} while (timeout < EMMC_BUSY_CMD_TIMEOUT_MS);

	if (timeout >= EMMC_BUSY_CMD_TIMEOUT_MS) {
		status = SD_CMD_MISSING;
		EMMC_TRACE("cmd%d timedout %dms\n", cmdIndex, timeout);
	}

	/* Reset both DAT and CMD line if only of them are stuck */
	if (present & mask)
		check_error(handle, SD4_EMMC_TOP_INTR_CMDERROR_MASK);

	handle->device->ctrl.argReg = argument;
	chal_sd_send_cmd((CHAL_HANDLE *) handle->device, cmdIndex,
			 handle->device->ctrl.argReg, options);

	handle->device->ctrl.cmdIndex = cmdIndex;

	event = wait_for_event(handle,
			       (SD4_EMMC_TOP_INTR_CMDDONE_MASK |
				SD_ERR_INTERRUPTS),
			       handle->device->cfg.wfe_retry);

	if (handle->device->ctrl.cmdStatus == SD_CMD_MISSING) {
		retry++;

		if (retry >= handle->device->cfg.retryLimit) {
			status = SD_CMD_MISSING;
			EMMC_TRACE("cmd%d retry reaches the limit %d\n",
				   cmdIndex, retry);
		} else {
			/* reset both DAT & CMD line if one of them is stuck */
			present = chal_sd_get_present_status((CHAL_HANDLE *)
							     handle->device);

			if (present & mask)
				check_error(handle,
					    SD4_EMMC_TOP_INTR_CMDERROR_MASK);

			EMMC_TRACE("cmd%d retry %d PSTATE[0x%08x]\n",
				   cmdIndex, retry,
				   chal_sd_get_present_status((CHAL_HANDLE *)
							      handle->device));
			goto RETRY_WRITE_CMD;
		}
	}

	if (handle->device->ctrl.cmdStatus == SD_OK) {
		if (resp != NULL) {
			status =
			    chal_sd_get_response((CHAL_HANDLE *) handle->device,
						 temp_resp);
			process_cmd_response(handle,
					     handle->device->ctrl.cmdIndex,
					     temp_resp[0], temp_resp[1],
					     temp_resp[2], temp_resp[3], resp);
		}

		/* Check Device busy after CMD */
		if ((cmdIndex == 5) || (cmdIndex == 6) || (cmdIndex == 7) ||
		    (cmdIndex == 28) || (cmdIndex == 29) || (cmdIndex == 38)) {

			timeout = 0;
			do {
				present =
				    chal_sd_get_present_status((CHAL_HANDLE *)
							       handle->device);

				timeout++;

				/* Dat[0]:bit20 low means device busy */
				if ((present & DAT0_LEVEL_MASK) == 0) {
					EMMC_TRACE("Device busy: ");
					EMMC_TRACE(
					  "cmd%d arg:0x%08x: PSTATE[0x%08x]\n",
					  cmdIndex, argument, present);
					SD_US_DELAY(100);
				} else {
					break;
				}
			} while (timeout < DEV_BUSY_TIMEOUT);
		}
	} else if (handle->device->ctrl.cmdStatus &&
		   handle->device->ctrl.cmdStatus != SD_CMD_MISSING) {
		retry++;
		status = check_error(handle, handle->device->ctrl.cmdStatus);

		EMMC_TRACE(
			"cmd%d error: cmdStatus:0x%08x error_status:0x%08x\n",
			cmdIndex, handle->device->ctrl.cmdStatus, status);

		if ((handle->device->ctrl.cmdIndex == 1) ||
		    (handle->device->ctrl.cmdIndex == 5)) {
			status = event;
		} else if ((handle->device->ctrl.cmdIndex == 7) ||
			   (handle->device->ctrl.cmdIndex == 41)) {
			status = event;
		} else if ((status == SD_ERROR_RECOVERABLE) &&
			   (retry < handle->device->cfg.retryLimit)) {
			EMMC_TRACE("cmd%d recoverable error ", cmdIndex);
			EMMC_TRACE("retry %d PSTATE[0x%08x].\n", retry,
				   chal_sd_get_present_status((CHAL_HANDLE *)
							      handle->device));
			goto RETRY_WRITE_CMD;
		} else {
			EMMC_TRACE("cmd%d retry reaches the limit %d\n",
				   cmdIndex, retry);
			status = event;
		}
	}

	handle->device->ctrl.blkReg = 0;
	/* clear error status for next command */
	handle->device->ctrl.cmdStatus = 0;

	return status;
}
