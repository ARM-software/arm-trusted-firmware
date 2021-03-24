/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <caam.h>
#include <common/debug.h>
#include <dcfg.h>
#include <drivers/delay_timer.h>
#include <fuse_prov.h>
#include <sfp.h>
#include <sfp_error_codes.h>


static int write_a_fuse(uint32_t *fuse_addr, uint32_t *fuse_hdr_val,
			uint32_t mask)
{
	uint32_t last_stored_val = sfp_read32(fuse_addr);

	 /* Check if fuse already blown or not */
	if ((last_stored_val & mask) == mask) {
		return ERROR_ALREADY_BLOWN;
	}

	 /* Write fuse in mirror registers */
	sfp_write32(fuse_addr, last_stored_val | (*fuse_hdr_val & mask));

	 /* Read back to check if write success */
	if (sfp_read32(fuse_addr) != (last_stored_val | (*fuse_hdr_val & mask))) {
		return ERROR_WRITE;
	}

	return 0;
}

static int write_fuses(uint32_t *fuse_addr, uint32_t *fuse_hdr_val, uint8_t len)
{
	int i;

	 /* Check if fuse already blown or not */
	for (i = 0; i < len; i++) {
		if (sfp_read32(&fuse_addr[i]) != 0) {
			return ERROR_ALREADY_BLOWN;
		}
	}

	 /* Write fuse in mirror registers */
	for (i = 0; i < len; i++) {
		sfp_write32(&fuse_addr[i], fuse_hdr_val[i]);
	}

	 /* Read back to check if write success */
	for (i = 0; i < len; i++) {
		if (sfp_read32(&fuse_addr[i]) != fuse_hdr_val[i]) {
			return ERROR_WRITE;
		}
	}

	return 0;
}

/*
 * This function program Super Root Key Hash (SRKH) in fuse
 * registers.
 */
static int prog_srkh(struct fuse_hdr_t *fuse_hdr,
		     struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int ret = 0;

	ret = write_fuses(sfp_ccsr_regs->srk_hash, fuse_hdr->srkh, 8);

	if (ret != 0) {
		ret = (ret == ERROR_ALREADY_BLOWN) ?
			ERROR_SRKH_ALREADY_BLOWN : ERROR_SRKH_WRITE;
	}

	return ret;
}

/* This function program OEMUID[0-4] in fuse registers. */
static int prog_oemuid(struct fuse_hdr_t *fuse_hdr,
		       struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int i, ret = 0;

	for (i = 0; i < 5; i++) {
		 /* Check OEMUIDx to be blown or not */
		if (((fuse_hdr->flags >> (FLAG_OUID0_SHIFT + i)) & 0x1) != 0) {
			 /* Check if OEMUID[i] already blown or not */
			ret = write_fuses(&sfp_ccsr_regs->oem_uid[i],
					 &fuse_hdr->oem_uid[i], 1);

			if (ret != 0) {
				ret = (ret == ERROR_ALREADY_BLOWN) ?
					ERROR_OEMUID_ALREADY_BLOWN
					: ERROR_OEMUID_WRITE;
			}
		}
	}
	return ret;
}

/* This function program DCV[0-1], DRV[0-1] in fuse registers. */
static int prog_debug(struct fuse_hdr_t *fuse_hdr,
		      struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int ret;

	 /* Check DCV to be blown or not */
	if (((fuse_hdr->flags >> (FLAG_DCV0_SHIFT)) & 0x3) != 0) {
		 /* Check if DCV[i] already blown or not */
		ret = write_fuses(sfp_ccsr_regs->dcv, fuse_hdr->dcv, 2);

		if (ret != 0) {
			ret = (ret == ERROR_ALREADY_BLOWN) ?
				ERROR_DCV_ALREADY_BLOWN
				: ERROR_DCV_WRITE;
		}
	}

	 /* Check DRV to be blown or not */
	if ((((fuse_hdr->flags >> (FLAG_DRV0_SHIFT)) & 0x3)) != 0) {
		 /* Check if DRV[i] already blown or not */
		ret = write_fuses(sfp_ccsr_regs->drv, fuse_hdr->drv, 2);

		if (ret != 0) {
			ret = (ret == ERROR_ALREADY_BLOWN) ?
				ERROR_DRV_ALREADY_BLOWN
				: ERROR_DRV_WRITE;
		} else {
			 /* Check for DRV hamming error */
			if (sfp_read32((void *)(get_sfp_addr()
							+ SFP_SVHESR_OFFSET))
				& SFP_SVHESR_DRV_MASK) {
				return ERROR_DRV_HAMMING_ERROR;
			}
		}
	}

	return 0;
}

 /*
  * Turn a 256-bit random value (32 bytes) into an OTPMK code word
  * modifying the input data array in place
  */
static void otpmk_make_code_word_256(uint8_t *otpmk, bool minimal_flag)
{
	int i;
	uint8_t parity_bit;
	uint8_t code_bit;

	if (minimal_flag == true) {
		 /*
		  * Force bits 252, 253, 254 and 255 to 1
		  * This is because these fuses may have already been blown
		  * and the OTPMK cannot force them back to 0
		  */
		otpmk[252/8] |= (1 << (252%8));
		otpmk[253/8] |= (1 << (253%8));
		otpmk[254/8] |= (1 << (254%8));
		otpmk[255/8] |= (1 << (255%8));
	}

	 /* Generate the hamming code for the code word */
	parity_bit = 0;
	code_bit = 0;
	for (i = 0; i < 256; i += 1) {
		if ((otpmk[i/8] & (1 << (i%8))) != 0) {
			parity_bit ^= 1;
			code_bit   ^= i;
		}
	}

	 /* Inverting otpmk[code_bit] will cause the otpmk
	  * to become a valid code word (except for overall parity)
	  */
	if (code_bit < 252) {
		otpmk[code_bit/8] ^= (1 << (code_bit % 8));
		parity_bit  ^= 1;  // account for flipping a bit changing parity
	} else {
		 /* Invert two bits:  (code_bit - 4) and 4
		  * Because we invert two bits, no need to touch the parity bit
		  */
		otpmk[(code_bit - 4)/8] ^= (1 << ((code_bit - 4) % 8));
		otpmk[4/8] ^= (1 << (4 % 8));
	}

	 /* Finally, adjust the overall parity of the otpmk
	  * otpmk bit 0
	  */
	otpmk[0] ^= parity_bit;
}

/* This function program One Time Programmable Master Key (OTPMK)
 *  in fuse registers.
 */
static int prog_otpmk(struct fuse_hdr_t *fuse_hdr,
		      struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int ret = 0;
	uint32_t otpmk_flags;
	uint32_t otpmk_random[8] __aligned(CACHE_WRITEBACK_GRANULE);

	otpmk_flags = (fuse_hdr->flags >> (FLAG_OTPMK_SHIFT)) & FLAG_OTPMK_MASK;

	switch (otpmk_flags) {
	case PROG_OTPMK_MIN:
		memset(fuse_hdr->otpmk, 0, sizeof(fuse_hdr->otpmk));

		 /* Minimal OTPMK value (252-255 bits set to 1) */
		fuse_hdr->otpmk[0] |= OTPMK_MIM_BITS_MASK;
		break;

	case PROG_OTPMK_RANDOM:
		if (is_sec_enabled() == false) {
			ret = ERROR_OTPMK_SEC_DISABLED;
			goto out;
		}

		 /* Generate Random number using CAAM for OTPMK */
		memset(otpmk_random, 0, sizeof(otpmk_random));
		if (get_rand_bytes_hw((uint8_t *)otpmk_random,
				      sizeof(otpmk_random)) != 0) {
			ret = ERROR_OTPMK_SEC_ERROR;
			goto out;
		}

		 /* Run hamming over random no. to make OTPMK */
		otpmk_make_code_word_256((uint8_t *)otpmk_random, false);

		 /* Swap OTPMK */
		fuse_hdr->otpmk[0] = otpmk_random[7];
		fuse_hdr->otpmk[1] = otpmk_random[6];
		fuse_hdr->otpmk[2] = otpmk_random[5];
		fuse_hdr->otpmk[3] = otpmk_random[4];
		fuse_hdr->otpmk[4] = otpmk_random[3];
		fuse_hdr->otpmk[5] = otpmk_random[2];
		fuse_hdr->otpmk[6] = otpmk_random[1];
		fuse_hdr->otpmk[7] = otpmk_random[0];
		break;

	case PROG_OTPMK_USER:
		break;

	case PROG_OTPMK_RANDOM_MIN:
		 /* Here assumption is that user is aware of minimal OTPMK
		  * already blown.
		  */

		 /* Generate Random number using CAAM for OTPMK */
		if (is_sec_enabled() == false) {
			ret = ERROR_OTPMK_SEC_DISABLED;
			goto out;
		}

		memset(otpmk_random, 0, sizeof(otpmk_random));
		if (get_rand_bytes_hw((uint8_t *)otpmk_random,
				      sizeof(otpmk_random)) != 0) {
			ret = ERROR_OTPMK_SEC_ERROR;
			goto out;
		}

		 /* Run hamming over random no. to make OTPMK */
		otpmk_make_code_word_256((uint8_t *)otpmk_random, true);

		 /* Swap OTPMK */
		fuse_hdr->otpmk[0] = otpmk_random[7];
		fuse_hdr->otpmk[1] = otpmk_random[6];
		fuse_hdr->otpmk[2] = otpmk_random[5];
		fuse_hdr->otpmk[3] = otpmk_random[4];
		fuse_hdr->otpmk[4] = otpmk_random[3];
		fuse_hdr->otpmk[5] = otpmk_random[2];
		fuse_hdr->otpmk[6] = otpmk_random[1];
		fuse_hdr->otpmk[7] = otpmk_random[0];
		break;

	case PROG_OTPMK_USER_MIN:
		 /*
		  * Here assumption is that user is aware of minimal OTPMK
		  * already blown. Check if minimal bits are set in user
		  * supplied OTPMK.
		  */
		if ((fuse_hdr->otpmk[0] & OTPMK_MIM_BITS_MASK) !=
							OTPMK_MIM_BITS_MASK) {
			ret = ERROR_OTPMK_USER_MIN;
			goto out;
		}
		break;

	default:
		ret = 0;
		goto out;
	}

	ret = write_fuses(sfp_ccsr_regs->otpmk, fuse_hdr->otpmk, 8);

	if (ret != 0) {
		ret = (ret == ERROR_ALREADY_BLOWN) ?
			ERROR_OTPMK_ALREADY_BLOWN
			: ERROR_OTPMK_WRITE;
	} else {
		 /* Check for DRV hamming error */
		if ((sfp_read32((void *)(get_sfp_addr() + SFP_SVHESR_OFFSET))
			& SFP_SVHESR_OTPMK_MASK) != 0) {
			ret = ERROR_OTPMK_HAMMING_ERROR;
		}
	}

out:
	return ret;
}

/* This function program OSPR1 in fuse registers.
 */
static int prog_ospr1(struct fuse_hdr_t *fuse_hdr,
		      struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int ret;
	uint32_t mask;

#ifdef NXP_SFP_VER_3_4
	if (((fuse_hdr->flags >> FLAG_MC_SHIFT) & 0x1) != 0) {
		mask = OSPR1_MC_MASK;
	}
#endif
	if (((fuse_hdr->flags >> FLAG_DBG_LVL_SHIFT) & 0x1) != 0) {
		mask = mask | OSPR1_DBG_LVL_MASK;
	}

	ret = write_a_fuse(&sfp_ccsr_regs->ospr1, &fuse_hdr->ospr1, mask);

	if (ret != 0) {
		ret = (ret == ERROR_ALREADY_BLOWN) ?
				ERROR_OSPR1_ALREADY_BLOWN
				: ERROR_OSPR1_WRITE;
	}

	return ret;
}

/* This function program SYSCFG in fuse registers.
 */
static int prog_syscfg(struct fuse_hdr_t *fuse_hdr,
		       struct sfp_ccsr_regs_t *sfp_ccsr_regs)
{
	int ret;

	 /* Check if SYSCFG already blown or not */
	ret = write_a_fuse(&sfp_ccsr_regs->ospr, &fuse_hdr->sc, OSPR0_SC_MASK);

	if (ret != 0) {
		ret = (ret == ERROR_ALREADY_BLOWN) ?
				ERROR_SC_ALREADY_BLOWN
				: ERROR_SC_WRITE;
	}

	return ret;
}

/* This function does fuse provisioning.
 */
int provision_fuses(unsigned long long fuse_scr_addr,
		    bool en_povdd_status)
{
	struct fuse_hdr_t *fuse_hdr = NULL;
	struct sfp_ccsr_regs_t *sfp_ccsr_regs = (void *)(get_sfp_addr()
							+ SFP_FUSE_REGS_OFFSET);
	int ret = 0;

	fuse_hdr = (struct fuse_hdr_t *)fuse_scr_addr;

	/*
	 * Check for Write Protect (WP) fuse. If blown then do
	 *  no fuse provisioning.
	 */
	if ((sfp_read32(&sfp_ccsr_regs->ospr) & 0x1) != 0) {
		goto out;
	}

	 /* Check if SRKH to be blown or not */
	if (((fuse_hdr->flags >> FLAG_SRKH_SHIFT) & 0x1) != 0) {
		INFO("Fuse: Program SRKH\n");
		ret = prog_srkh(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0) {
			error_handler(ret);
			goto out;
		}
	}

	 /* Check if OEMUID to be blown or not */
	if (((fuse_hdr->flags >> FLAG_OUID0_SHIFT) & FLAG_OUID_MASK) != 0) {
		INFO("Fuse: Program OEMUIDs\n");
		ret = prog_oemuid(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0) {
			error_handler(ret);
			goto out;
		}
	}

	 /* Check if Debug values to be blown or not */
	if (((fuse_hdr->flags >> FLAG_DCV0_SHIFT) & FLAG_DEBUG_MASK) != 0) {
		INFO("Fuse: Program Debug values\n");
		ret = prog_debug(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0) {
			error_handler(ret);
			goto out;
		}
	}

	 /* Check if OTPMK values to be blown or not */
	if (((fuse_hdr->flags >> FLAG_OTPMK_SHIFT) & PROG_NO_OTPMK) !=
		PROG_NO_OTPMK) {
		INFO("Fuse: Program OTPMK\n");
		ret = prog_otpmk(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0) {
			error_handler(ret);
			goto out;
		}
	}


	 /* Check if MC or DBG LVL to be blown or not */
	if ((((fuse_hdr->flags >> FLAG_MC_SHIFT) & 0x1) != 0) ||
		(((fuse_hdr->flags >> FLAG_DBG_LVL_SHIFT) & 0x1) != 0)) {
		INFO("Fuse: Program OSPR1\n");
		ret = prog_ospr1(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0) {
			error_handler(ret);
			goto out;
		}
	}

	 /* Check if SYSCFG to be blown or not */
	if (((fuse_hdr->flags >> FLAG_SYSCFG_SHIFT) & 0x1) != 0) {
		INFO("Fuse: Program SYSCFG\n");
		ret = prog_syscfg(fuse_hdr, sfp_ccsr_regs);
		if (ret != 0) {
			error_handler(ret);
			goto out;
		}
	}

	if (en_povdd_status) {
		ret = sfp_program_fuses();
		if (ret != 0) {
			error_handler(ret);
			goto out;
		}
	}
out:
	return ret;
}
