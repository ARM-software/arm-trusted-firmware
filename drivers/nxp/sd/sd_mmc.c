/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 *
 */

#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/io/io_block.h>
#include "nxp_timer.h"
#include "sd_mmc.h"
#include <utils.h>
#include <utils_def.h>


/* Private structure for MMC driver data */
static struct mmc mmc_drv_data;

#ifndef NXP_POLICY_OTA
/*
 * For NXP_POLICY_OTA, SD needs to do R/W on OCRAM. OCRAM is secure memory at
 * default. SD can only do non-secure DMA. Configuring SD to work in PIO mode
 * instead of DMA mode will make SD R/W on OCRAM available.
 */
/* To debug without dma comment this MACRO */
#define NXP_SD_DMA_CAPABILITY
#endif
#define SD_TIMEOUT        1000 /* ms */
#define SD_TIMEOUT_HIGH   20000 /* ms */
#define SD_BLOCK_TIMEOUT  8 /* ms */

#define ERROR_ESDHC_CARD_DETECT_FAIL	-1
#define ERROR_ESDHC_UNUSABLE_CARD	-2
#define ERROR_ESDHC_COMMUNICATION_ERROR	-3
#define ERROR_ESDHC_BLOCK_LENGTH	-4
#define ERROR_ESDHC_DMA_ERROR		-5
#define ERROR_ESDHC_BUSY		-6

/***************************************************************
 * Function    :    set_speed
 * Arguments   :    mmc - Pointer to mmc struct
 *                  clock - Clock Value to be set
 * Return      :    void
 * Description :    Calculates the value of SDCLKFS and DVS to be set
 *                  for getting the required clock assuming the base_clk
 *                  as a fixed value (MAX_PLATFORM_CLOCK)
 *****************************************************************/
static void set_speed(struct mmc *mmc, uint32_t clock)
{
	/* sdhc_clk = (base clock) / [(SDCLKFS × 2) × (DVS +1)] */

	uint32_t dvs = 1U;
	uint32_t sdclkfs = 2U;
	/* TBD - Change this to actual platform clock by reading via RCW */
	uint32_t base_clk = MAX_PLATFORM_CLOCK;

	if (base_clk / 16 > clock) {
		for (sdclkfs = 2U; sdclkfs < 256U; sdclkfs *= 2U) {
			if ((base_clk / sdclkfs) <= (clock * 16)) {
				break;
			}
		}
	}

	for (dvs = 1U; dvs <= 16U; dvs++) {
		if ((base_clk / (dvs * sdclkfs)) <= clock) {
			break;
		}
	}

	sdclkfs >>= 1U;
	dvs -= 1U;

	esdhc_out32(&mmc->esdhc_regs->sysctl,
			(ESDHC_SYSCTL_DTOCV(TIMEOUT_COUNTER_SDCLK_2_27) |
			 ESDHC_SYSCTL_SDCLKFS(sdclkfs) | ESDHC_SYSCTL_DVS(dvs) |
			 ESDHC_SYSCTL_SDCLKEN));
}

/***************************************************************************
 * Function    :    esdhc_init
 * Arguments   :    mmc - Pointer to mmc struct
 *                  card_detect - flag to indicate if card insert needs
 *                  to be detected or not. For SDHC2 controller, Card detect
 *                  is not present, so this field will be false
 * Return      :    SUCCESS or Error Code
 * Description :    1. Set Initial Clock Speed
 *                  2. Card Detect if not eMMC
 *                  3. Enable Controller Clock
 *                  4. Send 80 ticks for card to power up
 *                  5. Set LE mode and Bus Width as 1 bit.
 ***************************************************************************/
static int esdhc_init(struct mmc *mmc, bool card_detect)
{
	uint32_t val;
	uint64_t start_time;

	/* Reset the entire host controller */
	val = esdhc_in32(&mmc->esdhc_regs->sysctl) | ESDHC_SYSCTL_RSTA;
	esdhc_out32(&mmc->esdhc_regs->sysctl, val);

	/* Wait until the controller is available */
	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
		val = esdhc_in32(&mmc->esdhc_regs->sysctl) & ESDHC_SYSCTL_RSTA;
		if (val == 0U) {
			break;
		}
	}

	val = esdhc_in32(&mmc->esdhc_regs->sysctl) &
		(ESDHC_SYSCTL_RSTA);
	if (val != 0U) {
		ERROR("SD Reset failed\n");
		return ERROR_ESDHC_BUSY;
	}

	/* Set initial clock speed */
	set_speed(mmc, CARD_IDENTIFICATION_FREQ);

	if (card_detect) {
		/* Check CINS in prsstat register */
		val = esdhc_in32(&mmc->esdhc_regs->prsstat) &
			ESDHC_PRSSTAT_CINS;
		if (val == 0) {
			ERROR("CINS not set in prsstat\n");
			return ERROR_ESDHC_CARD_DETECT_FAIL;
		}
	}

	/* Enable controller clock */
	val = esdhc_in32(&mmc->esdhc_regs->sysctl) | ESDHC_SYSCTL_SDCLKEN;
	esdhc_out32(&mmc->esdhc_regs->sysctl, val);

	/* Send 80 clock ticks for the card to power up */
	val = esdhc_in32(&mmc->esdhc_regs->sysctl) | ESDHC_SYSCTL_INITA;
	esdhc_out32(&mmc->esdhc_regs->sysctl, val);

	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT) {
		val = esdhc_in32(&mmc->esdhc_regs->sysctl) & ESDHC_SYSCTL_INITA;
		if (val != 0U) {
			break;
		}
	}

	val = esdhc_in32(&mmc->esdhc_regs->sysctl) & ESDHC_SYSCTL_INITA;
	if (val == 0U) {
		ERROR("Failed to power up the card\n");
		return ERROR_ESDHC_CARD_DETECT_FAIL;
	}

	INFO("Card detected successfully\n");

	val = esdhc_in32(&mmc->esdhc_regs->proctl);
	val = val | (ESDHC_PROCTL_EMODE_LE | ESDHC_PROCTL_DTW_1BIT);

	/* Set little endian mode, set bus width as 1-bit */
	esdhc_out32(&mmc->esdhc_regs->proctl, val);

	/* Enable cache snooping for DMA transactions */
	val = esdhc_in32(&mmc->esdhc_regs->ctl) | ESDHC_DCR_SNOOP;
	esdhc_out32(&mmc->esdhc_regs->ctl, val);

	return 0;
}

/***************************************************************************
 * Function    :    esdhc_send_cmd
 * Arguments   :    mmc - Pointer to mmc struct
 *                  cmd - Command Number
 *                  args - Command Args
 * Return      :    SUCCESS is 0, or Error Code ( < 0)
 * Description :    Updates the eSDHC registers cmdargs and xfertype
 ***************************************************************************/
static int esdhc_send_cmd(struct mmc *mmc, uint32_t cmd, uint32_t args)
{
	uint32_t val;
	uint64_t start_time;
	uint32_t xfertyp = 0;

	esdhc_out32(&mmc->esdhc_regs->irqstat, ESDHC_IRQSTAT_CLEAR_ALL);

	/* Wait for the command line & data line to be free */
	/* (poll the CIHB,CDIHB bit of the present state register) */
	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
		val = esdhc_in32(&mmc->esdhc_regs->prsstat) &
			(ESDHC_PRSSTAT_CIHB | ESDHC_PRSSTAT_CDIHB);
		if (val == 0U) {
			break;
		}
	}

	val = esdhc_in32(&mmc->esdhc_regs->prsstat) &
		(ESDHC_PRSSTAT_CIHB | ESDHC_PRSSTAT_CDIHB);
	if (val != 0U) {
		ERROR("SD send cmd: Command Line or Data Line Busy cmd = %x\n",
				cmd);
		return ERROR_ESDHC_BUSY;
	}

	if (cmd == CMD2 || cmd == CMD9) {
		xfertyp |= ESDHC_XFERTYP_RSPTYP_136;
	} else  if (cmd == CMD7 || (cmd == CMD6 && mmc->card.type == MMC_CARD)) {
		xfertyp |= ESDHC_XFERTYP_RSPTYP_48_BUSY;
	} else if (cmd != CMD0) {
		xfertyp |= ESDHC_XFERTYP_RSPTYP_48;
	}

	if (cmd == CMD2 || cmd == CMD9) {
		xfertyp |= ESDHC_XFERTYP_CCCEN; /* Command index check enable */
	} else if ((cmd != CMD0) && (cmd != ACMD41) && (cmd != CMD1)) {
		xfertyp = xfertyp | ESDHC_XFERTYP_CCCEN | ESDHC_XFERTYP_CICEN;
	}

	if ((cmd == CMD8 || cmd == CMD14 || cmd == CMD19) &&
			mmc->card.type == MMC_CARD) {
		xfertyp |=  ESDHC_XFERTYP_DPSEL;
		if (cmd != CMD19) {
			xfertyp |= ESDHC_XFERTYP_DTDSEL;
		}
	}

	if (cmd == CMD6 || cmd == CMD17 || cmd == CMD18 || cmd == CMD24 ||
	    cmd == ACMD51) {
		if (!(mmc->card.type == MMC_CARD && cmd == CMD6)) {
			if (cmd == CMD24) {
				xfertyp |= ESDHC_XFERTYP_DPSEL;
			} else {
				xfertyp |= (ESDHC_XFERTYP_DPSEL |
					    ESDHC_XFERTYP_DTDSEL);
			}
		}

		if (cmd == CMD18) {
			xfertyp |= ESDHC_XFERTYP_BCEN;
			if (mmc->dma_support != 0) {
				/* Set BCEN of XFERTYP */
				xfertyp |= ESDHC_XFERTYP_DMAEN;
			}
		}

		if ((cmd == CMD17 || cmd == CMD24) && (mmc->dma_support != 0)) {
			xfertyp |= ESDHC_XFERTYP_DMAEN;
		}
	}

	xfertyp |= ((cmd & 0x3F) << 24);
	esdhc_out32(&mmc->esdhc_regs->cmdarg, args);
	esdhc_out32(&mmc->esdhc_regs->xfertyp, xfertyp);

#ifdef NXP_SD_DEBUG
	INFO("cmd = %d\n", cmd);
	INFO("args = %x\n", args);
	INFO("xfertyp: = %x\n", xfertyp);
#endif
	return 0;
}

/***************************************************************************
 * Function    :    esdhc_wait_response
 * Arguments   :    mmc - Pointer to mmc struct
 *                  response - Value updated
 * Return      :    SUCCESS - Response Received
 *                  COMMUNICATION_ERROR - Command not Complete
 *                  COMMAND_ERROR - CIE, CCE or CEBE  error
 *                  RESP_TIMEOUT - CTOE error
 * Description :    Checks for successful command completion.
 *                  Clears the CC bit at the end.
 ***************************************************************************/
static int esdhc_wait_response(struct mmc *mmc, uint32_t *response)
{
	uint32_t val;
	uint64_t start_time;
	uint32_t status = 0U;

	/* Wait for the command to complete */
	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
		val = esdhc_in32(&mmc->esdhc_regs->irqstat) & ESDHC_IRQSTAT_CC;
		if (val != 0U) {
			break;
		}
	}

	val = esdhc_in32(&mmc->esdhc_regs->irqstat) & ESDHC_IRQSTAT_CC;
	if (val == 0U) {
		ERROR("%s:IRQSTAT Cmd not complete(CC not set)\n", __func__);
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	status = esdhc_in32(&mmc->esdhc_regs->irqstat);

	/* Check whether the interrupt is a CRC, CTOE or CIE error */
	if ((status & (ESDHC_IRQSTAT_CIE | ESDHC_IRQSTAT_CEBE |
				ESDHC_IRQSTAT_CCE)) != 0) {
		ERROR("%s: IRQSTAT CRC, CEBE or CIE error = %x\n",
							__func__, status);
		return COMMAND_ERROR;
	}

	if ((status & ESDHC_IRQSTAT_CTOE) != 0) {
		INFO("%s: IRQSTAT CTOE set = %x\n", __func__, status);
		return RESP_TIMEOUT;
	}

	if ((status & ESDHC_IRQSTAT_DMAE) != 0) {
		ERROR("%s: IRQSTAT DMAE set = %x\n", __func__, status);
		return ERROR_ESDHC_DMA_ERROR;
	}

	if (response != NULL) {
		/* Get response values from eSDHC CMDRSPx registers. */
		response[0] = esdhc_in32(&mmc->esdhc_regs->cmdrsp[0]);
		response[1] = esdhc_in32(&mmc->esdhc_regs->cmdrsp[1]);
		response[2] = esdhc_in32(&mmc->esdhc_regs->cmdrsp[2]);
		response[3] = esdhc_in32(&mmc->esdhc_regs->cmdrsp[3]);
#ifdef NXP_SD_DEBUG
		INFO("Resp R1 R2 R3 R4\n");
		INFO("Resp R1 = %x\n", response[0]);
		INFO("R2 = %x\n", response[1]);
		INFO("R3 = %x\n", response[2]);
		INFO("R4 = %x\n", response[3]);
		INFO("\n");
#endif
	}

	/* Clear the CC bit - w1c */
	val = esdhc_in32(&mmc->esdhc_regs->irqstat) | ESDHC_IRQSTAT_CC;
	esdhc_out32(&mmc->esdhc_regs->irqstat, val);

	return 0;
}

/***************************************************************************
 * Function    :    mmc_switch_to_high_frquency
 * Arguments   :    mmc - Pointer to mmc struct
 * Return      :    SUCCESS or Error Code
 * Description :    mmc card bellow ver 4.0 does not support high speed
 *                  freq = 20 MHz
 *                  Send CMD6 (CMD_SWITCH_FUNC) With args 0x03B90100
 *                  Send CMD13 (CMD_SEND_STATUS)
 *                  if SWITCH Error, freq = 26 MHz
 *                  if no error, freq = 52 MHz
 ***************************************************************************/
static int mmc_switch_to_high_frquency(struct mmc *mmc)
{
	int error;
	uint32_t response[4];
	uint64_t start_time;

	mmc->card.bus_freq = MMC_SS_20MHZ;
	/* mmc card bellow ver 4.0 does not support high speed */
	if (mmc->card.version < MMC_CARD_VERSION_4_X) {
		return 0;
	}

	/* send switch cmd to change the card to High speed */
	error = esdhc_send_cmd(mmc, CMD_SWITCH_FUNC, SET_EXT_CSD_HS_TIMING);
	if (error != 0) {
		return error;
	}
	error = esdhc_wait_response(mmc, response);
	if (error != 0) {
		return error;
	}

	start_time = get_timer_val(0);
	do {
		/* check the status for which error */
		error = esdhc_send_cmd(mmc,
				CMD_SEND_STATUS, mmc->card.rca << 16);
		if (error != 0) {
			return error;
		}

		error = esdhc_wait_response(mmc, response);
		if (error != 0) {
			return error;
		}
	} while (((response[0] & SWITCH_ERROR) != 0) &&
			(get_timer_val(start_time) < SD_TIMEOUT));

	/* Check for the present state of card */
	if ((response[0] & SWITCH_ERROR) != 0) {
		mmc->card.bus_freq = MMC_HS_26MHZ;
	} else {
		mmc->card.bus_freq = MMC_HS_52MHZ;
	}

	return 0;
}

/***************************************************************************
 * Function    :    esdhc_set_data_attributes
 * Arguments   :    mmc - Pointer to mmc struct
 *                  blkcnt
 *                  blklen
 * Return      :    SUCCESS or Error Code
 * Description :    Set block attributes and watermark level register
 ***************************************************************************/
static int esdhc_set_data_attributes(struct mmc *mmc, uint32_t *dest_ptr,
		uint32_t blkcnt, uint32_t blklen)
{
	uint32_t val;
	uint64_t start_time;
	uint32_t wml;
	uint32_t wl;
	uint32_t dst = (uint32_t)((uint64_t)(dest_ptr));

	/* set blkattr when no transactions are executing */
	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
		val = esdhc_in32(&mmc->esdhc_regs->prsstat) & ESDHC_PRSSTAT_DLA;
		if (val == 0U) {
			break;
		}
	}

	val = esdhc_in32(&mmc->esdhc_regs->prsstat) & ESDHC_PRSSTAT_DLA;
	if (val != 0U) {
		ERROR("%s: Data line active.Can't set attribute\n", __func__);
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	wml = esdhc_in32(&mmc->esdhc_regs->wml);
	wml &= ~(ESDHC_WML_WR_BRST_MASK | ESDHC_WML_RD_BRST_MASK |
			ESDHC_WML_RD_WML_MASK | ESDHC_WML_WR_WML_MASK);

	if ((mmc->dma_support != 0) && (dest_ptr != NULL)) {
		/* Set burst length to 128 bytes */
		esdhc_out32(&mmc->esdhc_regs->wml,
				wml | ESDHC_WML_WR_BRST(BURST_128_BYTES));
		esdhc_out32(&mmc->esdhc_regs->wml,
				wml | ESDHC_WML_RD_BRST(BURST_128_BYTES));

		/* Set DMA System Destination Address */
		esdhc_out32(&mmc->esdhc_regs->dsaddr, dst);
	} else {
		wl = (blklen >= BLOCK_LEN_512) ?
			WML_512_BYTES : ((blklen + 3) / 4);
		/* Set 'Read Water Mark Level' register */
		esdhc_out32(&mmc->esdhc_regs->wml, wml | ESDHC_WML_RD_WML(wl));
	}

	/* Configure block Attributes register */
	esdhc_out32(&mmc->esdhc_regs->blkattr,
		ESDHC_BLKATTR_BLKCNT(blkcnt) | ESDHC_BLKATTR_BLKSZE(blklen));

	mmc->block_len = blklen;

	return 0;
}

/***************************************************************************
 * Function    :    esdhc_read_data_nodma
 * Arguments   :    mmc - Pointer to mmc struct
 *                  dest_ptr - Bufffer where read data is to be copied
 *                  len - Length of Data to be read
 * Return      :    SUCCESS or Error Code
 * Description :    Read data from the sdhc buffer without using DMA
 *                  and using polling mode
 ***************************************************************************/
static int esdhc_read_data_nodma(struct mmc *mmc, void *dest_ptr, uint32_t len)
{
	uint32_t i = 0U;
	uint32_t status;
	uint32_t num_blocks;
	uint32_t *dst = (uint32_t *)dest_ptr;
	uint32_t val;
	uint64_t start_time;

	num_blocks = len / mmc->block_len;

	while ((num_blocks--) != 0U) {

		start_time = get_timer_val(0);
		while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
			val = esdhc_in32(&mmc->esdhc_regs->prsstat) &
				ESDHC_PRSSTAT_BREN;
			if (val != 0U) {
				break;
			}
		}

		val = esdhc_in32(&mmc->esdhc_regs->prsstat)
			& ESDHC_PRSSTAT_BREN;
		if (val == 0U) {
			return ERROR_ESDHC_COMMUNICATION_ERROR;
		}

		for (i = 0U, status = esdhc_in32(&mmc->esdhc_regs->irqstat);
				i < mmc->block_len / 4;    i++, dst++) {
			/* get data from data port */
			val = mmio_read_32(
					(uintptr_t)&mmc->esdhc_regs->datport);
			esdhc_out32(dst, val);
			/* Increment destination pointer */
			status = esdhc_in32(&mmc->esdhc_regs->irqstat);
		}
		/* Check whether the interrupt is an DTOE/DCE/DEBE */
		if ((status & (ESDHC_IRQSTAT_DTOE | ESDHC_IRQSTAT_DCE |
					ESDHC_IRQSTAT_DEBE)) != 0) {
			ERROR("SD read error - DTOE, DCE, DEBE bit set = %x\n",
									status);
			return ERROR_ESDHC_COMMUNICATION_ERROR;
		}
	}

	/* Wait for TC */

	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
		val = esdhc_in32(&mmc->esdhc_regs->irqstat) & ESDHC_IRQSTAT_TC;
		if (val != 0U) {
			break;
		}
	}

	val = esdhc_in32(&mmc->esdhc_regs->irqstat) & ESDHC_IRQSTAT_TC;
	if (val == 0U) {
		ERROR("SD read timeout: Transfer bit not set in IRQSTAT\n");
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	return 0;
}

/***************************************************************************
 * Function    :    esdhc_write_data_nodma
 * Arguments   :    mmc - Pointer to mmc struct
 *                  src_ptr - Buffer where data is copied from
 *                  len - Length of Data to be written
 * Return      :    SUCCESS or Error Code
 * Description :    Write data to the sdhc buffer without using DMA
 *                  and using polling mode
 ***************************************************************************/
static int esdhc_write_data_nodma(struct mmc *mmc, void *src_ptr, uint32_t len)
{
	uint32_t i = 0U;
	uint32_t status;
	uint32_t num_blocks;
	uint32_t *src = (uint32_t *)src_ptr;
	uint32_t val;
	uint64_t start_time;

	num_blocks = len / mmc->block_len;

	while ((num_blocks--) != 0U) {
		start_time = get_timer_val(0);
		while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
			val = esdhc_in32(&mmc->esdhc_regs->prsstat) &
					 ESDHC_PRSSTAT_BWEN;
			if (val != 0U) {
				break;
			}
		}

		val = esdhc_in32(&mmc->esdhc_regs->prsstat) &
				 ESDHC_PRSSTAT_BWEN;
		if (val == 0U) {
			return ERROR_ESDHC_COMMUNICATION_ERROR;
		}

		for (i = 0U, status = esdhc_in32(&mmc->esdhc_regs->irqstat);
		     i < mmc->block_len / 4; i++, src++) {
			val = esdhc_in32(src);
			/* put data to data port */
			mmio_write_32((uintptr_t)&mmc->esdhc_regs->datport,
				      val);
			/* Increment source pointer */
			status = esdhc_in32(&mmc->esdhc_regs->irqstat);
		}
		/* Check whether the interrupt is an DTOE/DCE/DEBE */
		if ((status & (ESDHC_IRQSTAT_DTOE | ESDHC_IRQSTAT_DCE |
					ESDHC_IRQSTAT_DEBE)) != 0) {
			ERROR("SD write error - DTOE, DCE, DEBE bit set = %x\n",
			      status);
			return ERROR_ESDHC_COMMUNICATION_ERROR;
		}
	}

	/* Wait for TC */
	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
		val = esdhc_in32(&mmc->esdhc_regs->irqstat) & ESDHC_IRQSTAT_TC;
		if (val != 0U) {
			break;
		}
	}

	val = esdhc_in32(&mmc->esdhc_regs->irqstat) & ESDHC_IRQSTAT_TC;
	if (val == 0U) {
		ERROR("SD write timeout: Transfer bit not set in IRQSTAT\n");
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	return 0;
}

/***************************************************************************
 * Function    :    esdhc_read_data_dma
 * Arguments   :    mmc - Pointer to mmc struct
 *                  len - Length of Data to be read
 * Return      :    SUCCESS or Error Code
 * Description :    Read data from the sd card using DMA.
 ***************************************************************************/
static int esdhc_read_data_dma(struct mmc *mmc, uint32_t len)
{
	uint32_t status;
	uint32_t tblk;
	uint64_t start_time;

	tblk = SD_BLOCK_TIMEOUT * (len / mmc->block_len);

	start_time = get_timer_val(0);

	/* poll till TC is set */
	do {
		status = esdhc_in32(&mmc->esdhc_regs->irqstat);

		if ((status & (ESDHC_IRQSTAT_DEBE | ESDHC_IRQSTAT_DCE
					| ESDHC_IRQSTAT_DTOE)) != 0) {
			ERROR("SD read error - DTOE, DCE, DEBE bit set = %x\n",
								 status);
			return ERROR_ESDHC_COMMUNICATION_ERROR;
		}

		if ((status & ESDHC_IRQSTAT_DMAE) != 0) {
			ERROR("SD read error - DMA error = %x\n", status);
			return ERROR_ESDHC_DMA_ERROR;
		}

	} while (((status & ESDHC_IRQSTAT_TC) == 0) &&
		((esdhc_in32(&mmc->esdhc_regs->prsstat) & ESDHC_PRSSTAT_DLA) != 0) &&
		(get_timer_val(start_time) < SD_TIMEOUT_HIGH + tblk));

	if (get_timer_val(start_time) > SD_TIMEOUT_HIGH + tblk) {
		ERROR("SD read DMA timeout\n");
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	return 0;
}

/***************************************************************************
 * Function    :    esdhc_write_data_dma
 * Arguments   :    mmc - Pointer to mmc struct
 *                  len - Length of Data to be written
 * Return      :    SUCCESS or Error Code
 * Description :    Write data to the sd card using DMA.
 ***************************************************************************/
static int esdhc_write_data_dma(struct mmc *mmc, uint32_t len)
{
	uint32_t status;
	uint32_t tblk;
	uint64_t start_time;

	tblk = SD_BLOCK_TIMEOUT * (len / mmc->block_len);

	start_time = get_timer_val(0);

	/* poll till TC is set */
	do {
		status = esdhc_in32(&mmc->esdhc_regs->irqstat);

		if ((status & (ESDHC_IRQSTAT_DEBE | ESDHC_IRQSTAT_DCE
					| ESDHC_IRQSTAT_DTOE)) != 0) {
			ERROR("SD write error - DTOE, DCE, DEBE bit set = %x\n",
			      status);
			return ERROR_ESDHC_COMMUNICATION_ERROR;
		}

		if ((status & ESDHC_IRQSTAT_DMAE) != 0) {
			ERROR("SD write error - DMA error = %x\n", status);
			return ERROR_ESDHC_DMA_ERROR;
		}
	} while (((status & ESDHC_IRQSTAT_TC) == 0) &&
		((esdhc_in32(&mmc->esdhc_regs->prsstat) & ESDHC_PRSSTAT_DLA) != 0) &&
		(get_timer_val(start_time) < SD_TIMEOUT_HIGH + tblk));

	if (get_timer_val(start_time) > SD_TIMEOUT_HIGH + tblk) {
		ERROR("SD write DMA timeout\n");
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	return 0;
}

/***************************************************************************
 * Function    :    esdhc_read_data
 * Arguments   :    mmc - Pointer to mmc struct
 *                  dest_ptr - Bufffer where read data is to be copied
 *                  len - Length of Data to be read
 * Return      :    SUCCESS or Error Code
 * Description :    Calls esdhc_read_data_nodma and clear interrupt status
 ***************************************************************************/
int esdhc_read_data(struct mmc *mmc, void *dest_ptr, uint32_t len)
{
	int ret;

	if (mmc->dma_support && len > 64) {
		ret = esdhc_read_data_dma(mmc, len);
	} else {
		ret = esdhc_read_data_nodma(mmc, dest_ptr, len);
	}

	/* clear interrupt status */
	esdhc_out32(&mmc->esdhc_regs->irqstat, ESDHC_IRQSTAT_CLEAR_ALL);

	return ret;
}

/***************************************************************************
 * Function    :    esdhc_write_data
 * Arguments   :    mmc - Pointer to mmc struct
 *                  src_ptr - Buffer where data is copied from
 *                  len - Length of Data to be written
 * Return      :    SUCCESS or Error Code
 * Description :    Calls esdhc_write_data_nodma and clear interrupt status
 ***************************************************************************/
int esdhc_write_data(struct mmc *mmc, void *src_ptr, uint32_t len)
{
	int ret;

	if (mmc->dma_support && len > 64) {
		ret = esdhc_write_data_dma(mmc, len);
	} else {
		ret = esdhc_write_data_nodma(mmc, src_ptr, len);
	}

	/* clear interrupt status */
	esdhc_out32(&mmc->esdhc_regs->irqstat, ESDHC_IRQSTAT_CLEAR_ALL);

	return ret;
}

/***************************************************************************
 * Function    :    sd_switch_to_high_freq
 * Arguments   :    mmc - Pointer to mmc struct
 * Return      :    SUCCESS or Error Code
 * Description :    1. Send ACMD51 (CMD_SEND_SCR)
 *                  2. Read the SCR to check if card supports higher freq
 *                  3. check version from SCR
 *                  4. If SD 1.0, return (no Switch) freq = 25 MHz.
 *                  5. Send CMD6 (CMD_SWITCH_FUNC) with args 0x00FFFFF1 to
 *                     check the status of switch func
 *                  6. Send CMD6 (CMD_SWITCH_FUNC) With args 0x80FFFFF1 to
 *                     switch to high frequency = 50 Mhz
 ***************************************************************************/
static int sd_switch_to_high_freq(struct mmc *mmc)
{
	int err;
	uint8_t scr[8];
	uint8_t status[64];
	uint32_t response[4];
	uint32_t version;
	uint32_t count;
	uint32_t sd_versions[] = {SD_CARD_VERSION_1_0, SD_CARD_VERSION_1_10,
		SD_CARD_VERSION_2_0};

	mmc->card.bus_freq = SD_SS_25MHZ;
	/* Send Application command */
	err = esdhc_send_cmd(mmc, CMD_APP_CMD, mmc->card.rca << 16);
	if (err != 0) {
		return err;
	}

	err = esdhc_wait_response(mmc, response);
	if (err != 0) {
		return err;
	}

	esdhc_set_data_attributes(mmc, NULL, 1, 8);
	/* Read the SCR to find out if this card supports higher speeds */
	err = esdhc_send_cmd(mmc, CMD_SEND_SCR,  mmc->card.rca << 16);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, response);
	if (err != 0) {
		return err;
	}

	/* read 8 bytes of scr data */
	err = esdhc_read_data(mmc, scr, 8U);
	if (err != 0) {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	/* check version from SCR */
	version = scr[0] & U(0xF);
	if (version <= 2U) {
		mmc->card.version = sd_versions[version];
	} else {
		mmc->card.version = SD_CARD_VERSION_2_0;
	}

	/* does not support switch func */
	if (mmc->card.version == SD_CARD_VERSION_1_0) {
		return 0;
	}

	/* read 64 bytes of status */
	esdhc_set_data_attributes(mmc, NULL, 1U, 64U);

	/* check the status of switch func */
	for (count = 0U; count < 4U; count++) {
		err = esdhc_send_cmd(mmc, CMD_SWITCH_FUNC,
				SD_SWITCH_FUNC_CHECK_MODE);
		if (err != 0) {
			return err;
		}
		err = esdhc_wait_response(mmc, response);
		if (err != 0) {
			return err;
		}
		/* read 64 bytes of scr data */
		err = esdhc_read_data(mmc, status, 64U);
		if (err != 0) {
			return ERROR_ESDHC_COMMUNICATION_ERROR;
		}

		if ((status[29] & SD_SWITCH_FUNC_HIGH_SPEED) == 0) {
			break;
		}
	}

	if ((status[13] & SD_SWITCH_FUNC_HIGH_SPEED) == 0) {
		return 0;
	}

	/* SWITCH */
	esdhc_set_data_attributes(mmc, NULL, 1, 64);
	err = esdhc_send_cmd(mmc, CMD_SWITCH_FUNC, SD_SWITCH_FUNC_SWITCH_MODE);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, response);
	if (err != 0) {
		return err;
	}

	err = esdhc_read_data(mmc, status, 64U);
	if (err != 0) {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	if ((status[16]) == U(0x01)) {
		mmc->card.bus_freq = SD_HS_50MHZ;
	}

	return 0;
}

/***************************************************************************
 * Function    :    change_state_to_transfer_state
 * Arguments   :    mmc - Pointer to mmc struct
 * Return      :    SUCCESS or Error Code
 * Description :    1. Send CMD7 (CMD_SELECT_CARD) to toggles the card
 *                     between stand-by and transfer state
 *                  2. Send CMD13 (CMD_SEND_STATUS) to check state as
 *                     Transfer State
 ***************************************************************************/
static int change_state_to_transfer_state(struct mmc *mmc)
{
	int error = 0;
	uint32_t response[4];
	uint64_t start_time;

	/* Command CMD_SELECT_CARD/CMD7 toggles the card between stand-by
	 * and transfer states
	 */
	error = esdhc_send_cmd(mmc, CMD_SELECT_CARD, mmc->card.rca << 16);
	if (error != 0) {
		return error;
	}
	error = esdhc_wait_response(mmc, response);
	if (error != 0) {
		return error;
	}

	start_time = get_timer_val(0);
	while (get_timer_val(start_time) < SD_TIMEOUT_HIGH) {
		/* send CMD13 to check card status */
		error = esdhc_send_cmd(mmc,
					CMD_SEND_STATUS, mmc->card.rca << 16);
		if (error != 0) {
			return error;
		}
		error = esdhc_wait_response(mmc, response);
		if ((error != 0) || ((response[0] & R1_ERROR) != 0)) {
			return error;
		}

		/* Check for the present state of card */
		if (((response[0] >> 9U) & U(0xF)) == STATE_TRAN) {
			break;
		}
	}
	if (((response[0] >> 9U) & U(0xF)) == STATE_TRAN) {
		return 0;
	} else {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}
}

/***************************************************************************
 * Function    :    get_cid_rca_csd
 * Arguments   :    mmc - Pointer to mmc struct
 * Return      :    SUCCESS or Error Code
 * Description :    1. Send CMD2 (CMD_ALL_SEND_CID)
 *                  2. get RCA for SD cards, set rca for mmc cards
 *                     Send CMD3 (CMD_SEND_RELATIVE_ADDR)
 *                  3. Send CMD9 (CMD_SEND_CSD)
 *                  4. Get MMC Version from CSD
 ***************************************************************************/
static int get_cid_rca_csd(struct mmc *mmc)
{
	int err;
	uint32_t version;
	uint32_t response[4];
	uint32_t mmc_version[] = {MMC_CARD_VERSION_1_2, MMC_CARD_VERSION_1_4,
		MMC_CARD_VERSION_2_X, MMC_CARD_VERSION_3_X,
		MMC_CARD_VERSION_4_X};

	err = esdhc_send_cmd(mmc, CMD_ALL_SEND_CID, 0);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, response);
	if (err != 0) {
		return err;
	}

	/* get RCA for SD cards, set rca for mmc cards */
	mmc->card.rca = SD_MMC_CARD_RCA;

	/* send RCA cmd */
	err = esdhc_send_cmd(mmc, CMD_SEND_RELATIVE_ADDR, mmc->card.rca << 16);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, response);
	if (err != 0) {
		return err;
	}

	/* for SD, get the the RCA */
	if (mmc->card.type == SD_CARD) {
		mmc->card.rca = (response[0] >> 16) & 0xFFFF;
	}

	/* Get the CSD (card specific data) from card. */
	err = esdhc_send_cmd(mmc, CMD_SEND_CSD, mmc->card.rca << 16);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, response);
	if (err != 0) {
		return err;
	}

	version = (response[3] >> 18U) & U(0xF);
	if (mmc->card.type == MMC_CARD) {
		if (version <= MMC_CARD_VERSION_4_X) {
			mmc->card.version = mmc_version[version];
		} else {
			mmc->card.version = MMC_CARD_VERSION_4_X;
		}
	}

	mmc->card.block_len = 1 << ((response[2] >> 8) & 0xF);

	if (mmc->card.block_len > BLOCK_LEN_512) {
		mmc->card.block_len = BLOCK_LEN_512;
	}

	return 0;
}

/***************************************************************************
 * Function    :    identify_mmc_card
 * Arguments   :    mmc - Pointer to mmc struct
 * Return      :    SUCCESS or Error Code
 * Description :    1. Send Reset Command
 *                  2. Send CMD1 with args to set voltage range and Sector
 *                     Mode. (Voltage Args = 0xFF8)
 *                  3. Check the OCR Response
 ***************************************************************************/
static int identify_mmc_card(struct mmc *mmc)
{
	uint64_t start_time;
	uint32_t resp[4];
	int ret;
	uint32_t args;

	/* card reset */
	ret = esdhc_send_cmd(mmc, CMD_GO_IDLE_STATE, 0U);
	if (ret != 0) {
		return ret;
	}
	ret = esdhc_wait_response(mmc, resp);
	if (ret != 0) {
		return ret;
	}

	/* Send CMD1 to get the ocr value repeatedly till the card */
	/* busy is clear. timeout = 20sec */

	start_time = get_timer_val(0);
	do {
		/* set the bits for the voltage ranges supported by host */
		args = mmc->voltages_caps | MMC_OCR_SECTOR_MODE;
		ret = esdhc_send_cmd(mmc, CMD_MMC_SEND_OP_COND, args);
		if (ret != 0) {
			return ret;
		}
		ret = esdhc_wait_response(mmc, resp);
		if (ret != 0) {
			return ERROR_ESDHC_UNUSABLE_CARD;
		}
	} while (((resp[0] & MMC_OCR_BUSY) == 0U) &&
			(get_timer_val(start_time) < SD_TIMEOUT_HIGH));

	if (get_timer_val(start_time) > SD_TIMEOUT_HIGH) {
		return ERROR_ESDHC_UNUSABLE_CARD;
	}

	if ((resp[0] & MMC_OCR_CCS) == MMC_OCR_CCS) {
		mmc->card.is_high_capacity = 1;
	}

	return MMC_CARD;
}

/***************************************************************************
 * Function    :    check_for_sd_card
 * Arguments   :    mmc - Pointer to mmc struct
 * Return      :    SUCCESS or Error Code
 * Description :    1. Send Reset Command
 *                  2. Send CMD8 with pattern 0xAA (to check for SD 2.0)
 *                  3. Send ACMD41 with args to set voltage range and HCS
 *                     HCS is set only for SD Card > 2.0
 *                     Voltage Caps = 0xFF8
 *                  4. Check the OCR Response
 ***************************************************************************/
static int check_for_sd_card(struct mmc *mmc)
{
	uint64_t start_time;
	uint32_t args;
	int  ret;
	uint32_t resp[4];

	/* Send reset command */
	ret = esdhc_send_cmd(mmc, CMD_GO_IDLE_STATE, 0U);
	if (ret != 0) {
		return ret;
	}
	ret = esdhc_wait_response(mmc, resp);
	if (ret != 0) {
		return ret;
	}

	/* send CMD8 with  pattern 0xAA */
	args = MMC_VDD_HIGH_VOLTAGE | 0xAA;
	ret = esdhc_send_cmd(mmc, CMD_SEND_IF_COND, args);
	if (ret != 0) {
		return ret;
	}
	ret = esdhc_wait_response(mmc, resp);
	if (ret == RESP_TIMEOUT) { /* sd ver 1.x or not sd */
		mmc->card.is_high_capacity = 0;
	} else if ((resp[0] & U(0xFF)) == U(0xAA)) { /* ver 2.0 or later */
		mmc->card.version = SD_CARD_VERSION_2_0;
	} else {
		return  NOT_SD_CARD;
	}
	/* Send Application command-55 to get the ocr value repeatedly till
	 * the card busy is clear. timeout = 20sec
	 */

	start_time = get_timer_val(0);
	do {
		ret = esdhc_send_cmd(mmc, CMD_APP_CMD, 0U);
		if (ret != 0) {
			return ret;
		}
		ret = esdhc_wait_response(mmc, resp);
		if (ret == COMMAND_ERROR) {
			return ERROR_ESDHC_UNUSABLE_CARD;
		}

		/* set the bits for the voltage ranges supported by host */
		args = mmc->voltages_caps;
		if (mmc->card.version == SD_CARD_VERSION_2_0) {
			args |= SD_OCR_HCS;
		}

		/* Send ACMD41 to set voltage range */
		ret = esdhc_send_cmd(mmc, CMD_SD_SEND_OP_COND, args);
		if (ret != 0) {
			return ret;
		}
		ret = esdhc_wait_response(mmc, resp);
		if (ret == COMMAND_ERROR) {
			return ERROR_ESDHC_UNUSABLE_CARD;
		} else if (ret == RESP_TIMEOUT) {
			return NOT_SD_CARD;
		}
	} while (((resp[0] & MMC_OCR_BUSY) == 0U) &&
			(get_timer_val(start_time) < SD_TIMEOUT_HIGH));

	if (get_timer_val(start_time) > SD_TIMEOUT_HIGH) {
		INFO("SD_TIMEOUT_HIGH\n");
		return ERROR_ESDHC_UNUSABLE_CARD;
	}

	/* bit set in card capacity status */
	if ((resp[0] & MMC_OCR_CCS) == MMC_OCR_CCS) {
		mmc->card.is_high_capacity = 1;
	}

	return SD_CARD;
}

/***************************************************************************
 * Function    :    esdhc_emmc_init
 * Arguments   :    mmc - Pointer to mmc struct
 *                  src_emmc - Flag to Indicate SRC as emmc
 * Return      :    SUCCESS or Error Code (< 0)
 * Description :    Base Function called from sd_mmc_init or emmc_init
 ***************************************************************************/
int esdhc_emmc_init(struct mmc *mmc, bool card_detect)
{
	int error = 0;
	int ret = 0;

	error = esdhc_init(mmc, card_detect);
	if (error != 0) {
		return error;
	}

	mmc->card.bus_freq = CARD_IDENTIFICATION_FREQ;
	mmc->card.rca = 0;
	mmc->card.is_high_capacity = 0;
	mmc->card.type = ERROR_ESDHC_UNUSABLE_CARD;

	/* Set Voltage caps as FF8 i.e all supported */
	/* high voltage bits 2.7 - 3.6 */
	mmc->voltages_caps = MMC_OCR_VDD_FF8;

#ifdef NXP_SD_DMA_CAPABILITY
	/* Getting host DMA capabilities. */
	mmc->dma_support = esdhc_in32(&mmc->esdhc_regs->hostcapblt) &
					ESDHC_HOSTCAPBLT_DMAS;
#else
	mmc->dma_support = 0;
#endif

	ret = NOT_SD_CARD;
	/* If SRC is not EMMC, check for SD or MMC */
	ret = check_for_sd_card(mmc);
	switch (ret) {
	case SD_CARD:
		mmc->card.type = SD_CARD;
		break;

	case NOT_SD_CARD:
		/* try for MMC card */
		if (identify_mmc_card(mmc) == MMC_CARD) {
			mmc->card.type = MMC_CARD;
		} else {
			return ERROR_ESDHC_UNUSABLE_CARD;
		}
		break;

	default:
		return ERROR_ESDHC_UNUSABLE_CARD;
	}

	/* get CID, RCA and CSD. For MMC, set the rca */
	error = get_cid_rca_csd(mmc);
	if (error != 0) {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	/* change state to Transfer mode */
	error = change_state_to_transfer_state(mmc);
	if (error != 0) {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	/* change to high frequency if supported */
	if (mmc->card.type == SD_CARD) {
		error = sd_switch_to_high_freq(mmc);
	} else {
		error = mmc_switch_to_high_frquency(mmc);
	}
	if (error != 0) {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	/* mmc: 20000000, 26000000, 52000000 */
	/* sd: 25000000, 50000000 */
	set_speed(mmc, mmc->card.bus_freq);

	INFO("init done:\n");
	return 0;
}

/***************************************************************************
 * Function    :    sd_mmc_init
 * Arguments   :    mmc - Pointer to mmc struct
 * Return      :    SUCCESS or Error Code
 * Description :    Base Function called via hal_init for SD/MMC
 *                  initialization
 ***************************************************************************/
int sd_mmc_init(uintptr_t nxp_esdhc_addr, bool card_detect)
{
	struct mmc *mmc = NULL;
	int ret;

	mmc = &mmc_drv_data;
	memset(mmc, 0, sizeof(struct mmc));
	mmc->esdhc_regs = (struct esdhc_regs *)nxp_esdhc_addr;

	INFO("esdhc_emmc_init\n");
	ret = esdhc_emmc_init(mmc, card_detect);
	return ret;
}

/***************************************************************************
 * Function    :    esdhc_read_block
 * Arguments   :    mmc - Pointer to mmc struct
 *                  dst - Destination Pointer
 *                  block - Block Number
 * Return      :    SUCCESS or Error Code
 * Description :    Read a Single block to Destination Pointer
 *                  1. Send CMD16 (CMD_SET_BLOCKLEN) with args as blocklen
 *                  2. Send CMD17 (CMD_READ_SINGLE_BLOCK) with args offset
 ***************************************************************************/
static int esdhc_read_block(struct mmc *mmc, void *dst, uint32_t block)
{
	uint32_t offset;
	int err;

	/* send cmd16 to set the block size. */
	err = esdhc_send_cmd(mmc, CMD_SET_BLOCKLEN, mmc->card.block_len);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, NULL);
	if (err != 0) {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	if (mmc->card.is_high_capacity != 0) {
		offset = block;
	} else {
		offset = block * mmc->card.block_len;
	}

	esdhc_set_data_attributes(mmc, dst, 1, mmc->card.block_len);
	err = esdhc_send_cmd(mmc, CMD_READ_SINGLE_BLOCK, offset);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, NULL);
	if (err != 0) {
		return err;
	}

	err = esdhc_read_data(mmc, dst, mmc->card.block_len);

	return err;
}

/***************************************************************************
 * Function    :    esdhc_write_block
 * Arguments   :    mmc - Pointer to mmc struct
 *                  src - Source Pointer
 *                  block - Block Number
 * Return      :    SUCCESS or Error Code
 * Description :    Write a Single block from Source Pointer
 *                  1. Send CMD16 (CMD_SET_BLOCKLEN) with args as blocklen
 *                  2. Send CMD24 (CMD_WRITE_SINGLE_BLOCK) with args offset
 ***************************************************************************/
static int esdhc_write_block(struct mmc *mmc, void *src, uint32_t block)
{
	uint32_t offset;
	int err;

	/* send cmd16 to set the block size. */
	err = esdhc_send_cmd(mmc, CMD_SET_BLOCKLEN, mmc->card.block_len);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, NULL);
	if (err != 0) {
		return ERROR_ESDHC_COMMUNICATION_ERROR;
	}

	if (mmc->card.is_high_capacity != 0) {
		offset = block;
	} else {
		offset = block * mmc->card.block_len;
	}

	esdhc_set_data_attributes(mmc, src, 1, mmc->card.block_len);
	err = esdhc_send_cmd(mmc, CMD_WRITE_SINGLE_BLOCK, offset);
	if (err != 0) {
		return err;
	}
	err = esdhc_wait_response(mmc, NULL);
	if (err != 0) {
		return err;
	}

	err = esdhc_write_data(mmc, src, mmc->card.block_len);

	return err;
}

/***************************************************************************
 * Function    :    esdhc_read
 * Arguments   :    src_offset - offset on sd/mmc to read from. Should be block
 *		    size aligned
 *                  dst - Destination Pointer
 *                  size - Length of Data ( Multiple of block size)
 * Return      :    SUCCESS or Error Code
 * Description :    Calls esdhc_read_block repeatedly for reading the
 *                  data.
 ***************************************************************************/
int esdhc_read(struct mmc *mmc, uint32_t src_offset, uintptr_t dst, size_t size)
{
	int error = 0;
	uint32_t blk, num_blocks;
	uint8_t *buff = (uint8_t *)dst;

#ifdef NXP_SD_DEBUG
	INFO("sd mmc read\n");
	INFO("src = %x, dst = %lxsize = %lu\n", src_offset, dst, size);
#endif

	/* check for size */
	if (size == 0) {
		return 0;
	}

	if ((size % mmc->card.block_len) != 0) {
		ERROR("Size is not block aligned\n");
		return -1;
	}

	if ((src_offset % mmc->card.block_len) != 0) {
		ERROR("Size is not block aligned\n");
		return -1;
	}

	/* start block */
	blk = src_offset / mmc->card.block_len;
#ifdef NXP_SD_DEBUG
	INFO("blk = %x\n", blk);
#endif

	/* Number of blocks to be read */
	num_blocks = size / mmc->card.block_len;

	while (num_blocks) {
		error = esdhc_read_block(mmc, buff, blk);
		if (error != 0) {
			ERROR("Read error = %x\n", error);
			return error;
		}

		buff = buff + mmc->card.block_len;
		blk++;
		num_blocks--;
	}

	INFO("sd-mmc read done.\n");
	return error;
}

/***************************************************************************
 * Function    :    esdhc_write
 * Arguments   :    src - Source Pointer
 *                  dst_offset - offset on sd/mmc to write to. Should be block
 *		    size aligned
 *                  size - Length of Data (Multiple of block size)
 * Return      :    SUCCESS or Error Code
 * Description :    Calls esdhc_write_block repeatedly for writing the
 *                  data.
 ***************************************************************************/
int esdhc_write(struct mmc *mmc, uintptr_t src, uint32_t dst_offset,
		size_t size)
{
	int error = 0;
	uint32_t blk, num_blocks;
	uint8_t *buff = (uint8_t *)src;

#ifdef NXP_SD_DEBUG
	INFO("sd mmc write\n");
	INFO("src = %x, dst = %lxsize = %lu\n", src, dst_offset, size);
#endif

	/* check for size */
	if (size == 0) {
		return 0;
	}

	if ((size % mmc->card.block_len) != 0) {
		ERROR("Size is not block aligned\n");
		return -1;
	}

	if ((dst_offset % mmc->card.block_len) != 0) {
		ERROR("Size is not block aligned\n");
		return -1;
	}

	/* start block */
	blk = dst_offset / mmc->card.block_len;
#ifdef NXP_SD_DEBUG
	INFO("blk = %x\n", blk);
#endif

	/* Number of blocks to be written */
	num_blocks = size / mmc->card.block_len;

	while (num_blocks != 0U) {
		error = esdhc_write_block(mmc, buff, blk);
		if (error != 0U) {
			ERROR("Write error = %x\n", error);
			return error;
		}

		buff = buff + mmc->card.block_len;
		blk++;
		num_blocks--;
	}

	INFO("sd-mmc write done.\n");
	return error;
}

static size_t ls_sd_emmc_read(int lba, uintptr_t buf, size_t size)
{
	struct mmc *mmc = NULL;
	int ret;

	mmc = &mmc_drv_data;
	lba *= BLOCK_LEN_512;
	ret = esdhc_read(mmc, lba, buf, size);
	return ret ? 0 : size;
}

static struct io_block_dev_spec ls_emmc_dev_spec = {
	.buffer = {
		.offset = 0,
		.length = 0,
	},
	.ops = {
		.read = ls_sd_emmc_read,
	},
	.block_size = BLOCK_LEN_512,
};

int sd_emmc_init(uintptr_t *block_dev_spec,
			uintptr_t nxp_esdhc_addr,
			size_t nxp_sd_block_offset,
			size_t nxp_sd_block_size,
			bool card_detect)
{
	int ret;

	ret = sd_mmc_init(nxp_esdhc_addr, card_detect);
	if (ret != 0) {
		return ret;
	}

	ls_emmc_dev_spec.buffer.offset = nxp_sd_block_offset;
	ls_emmc_dev_spec.buffer.length = nxp_sd_block_size;
	*block_dev_spec = (uintptr_t)&ls_emmc_dev_spec;

	return 0;
}
