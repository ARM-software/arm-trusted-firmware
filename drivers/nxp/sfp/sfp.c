/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <caam.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <sfp.h>
#include <sfp_error_codes.h>

static uintptr_t g_nxp_sfp_addr;
static uint32_t srk_hash[SRK_HASH_SIZE/sizeof(uint32_t)]
					__aligned(CACHE_WRITEBACK_GRANULE);

void sfp_init(uintptr_t nxp_sfp_addr)
{
	g_nxp_sfp_addr = nxp_sfp_addr;
}

uintptr_t get_sfp_addr(void)
{
	return g_nxp_sfp_addr;
}

uint32_t *get_sfp_srk_hash(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs =
			(void *) (g_nxp_sfp_addr + SFP_FUSE_REGS_OFFSET);
	int i = 0;

	/* Add comparison of hash with SFP hash here */
	for (i = 0; i < SRK_HASH_SIZE/sizeof(uint32_t); i++)
		srk_hash[i] =
			mmio_read_32((uintptr_t)&sfp_ccsr_regs->srk_hash[i]);

	return srk_hash;
}

void set_sfp_wr_disable(void)
{
	/*
	 * Mark SFP Write Disable and Write Disable Lock
	 * Bit to prevent write to SFP fuses like
	 * OUID's, Key Revocation fuse etc
	 */
	void *sfpcr = (void *)(g_nxp_sfp_addr + SFP_SFPCR_OFFSET);
	uint32_t sfpcr_val;

	sfpcr_val = sfp_read32(sfpcr);
	sfpcr_val |= (SFP_SFPCR_WD | SFP_SFPCR_WDL);
	sfp_write32(sfpcr, sfpcr_val);
}

int sfp_program_fuses(void)
{
	uint32_t ingr;
	uint32_t sfp_cmd_status = 0U;
	int ret = 0;

	/* Program SFP fuses from mirror registers */
	sfp_write32((void *)(g_nxp_sfp_addr + SFP_INGR_OFFSET),
		    SFP_INGR_PROGFB_CMD);

	/* Wait until fuse programming is successful */
	do {
		ingr = sfp_read32(g_nxp_sfp_addr + SFP_INGR_OFFSET);
	} while (ingr & SFP_INGR_PROGFB_CMD);

	/* Check for SFP fuse programming error */
	sfp_cmd_status = sfp_read32(g_nxp_sfp_addr + SFP_INGR_OFFSET)
			 & SFP_INGR_ERROR_MASK;

	if (sfp_cmd_status != 0U) {
		return ERROR_PROGFB_CMD;
	}

	return ret;
}

uint32_t sfp_read_oem_uid(uint8_t oem_uid)
{
	uint32_t val = 0U;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(g_nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	if (oem_uid > MAX_OEM_UID) {
		ERROR("Invalid OEM UID received.\n");
		return ERROR_OEMUID_WRITE;
	}

	val = sfp_read32(&sfp_ccsr_regs->oem_uid[oem_uid]);

	return val;
}

/*
 * return val:  0 - No update required.
 *              1 - successful update done.
 *              ERROR_OEMUID_WRITE - Invalid OEM UID
 */
uint32_t sfp_write_oem_uid(uint8_t oem_uid, uint32_t sfp_val)
{
	uint32_t val = 0U;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(g_nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	val = sfp_read_oem_uid(oem_uid);

	if (val == ERROR_OEMUID_WRITE) {
		return ERROR_OEMUID_WRITE;
	}

	/* Counter already set. No need to do anything */
	if ((val & sfp_val) != 0U) {
		return 0U;
	}

	val |= sfp_val;

	INFO("SFP Value is %x for setting sfp_val = %d\n", val, sfp_val);

	sfp_write32(&sfp_ccsr_regs->oem_uid[oem_uid], val);

	return 1U;
}

int sfp_check_its(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(g_nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	if ((sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_ITS_MASK) != 0) {
		return 1;
	} else {
		return 0;
	}
}

int sfp_check_oem_wp(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(g_nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	if ((sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_WP_MASK) != 0) {
		return 1;
	} else {
		return 0;
	}
}

/* This function returns ospr's key_revoc values.*/
uint32_t get_key_revoc(void)
{
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(g_nxp_sfp_addr
							+ SFP_FUSE_REGS_OFFSET);

	return (sfp_read32(&sfp_ccsr_regs->ospr) & OSPR_KEY_REVOC_MASK) >>
						OSPR_KEY_REVOC_SHIFT;
}
