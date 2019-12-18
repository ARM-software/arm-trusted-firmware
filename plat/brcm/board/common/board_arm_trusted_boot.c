/*
 * Copyright 2015 - 2020 Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <tools_share/tbbr_oid.h>

#include <sbl_util.h>
#include <sotp.h>

/* Weak definition may be overridden in specific platform */
#pragma weak plat_match_rotpk
#pragma weak plat_get_nv_ctr
#pragma weak plat_set_nv_ctr

/* SHA256 algorithm */
#define SHA256_BYTES			32

/* ROTPK locations */
#define ARM_ROTPK_REGS_ID		1
#define ARM_ROTPK_DEVEL_RSA_ID		2
#define BRCM_ROTPK_SOTP_RSA_ID		3

#if !ARM_ROTPK_LOCATION_ID
  #error "ARM_ROTPK_LOCATION_ID not defined"
#endif

static const unsigned char rotpk_hash_hdr[] =
		"\x30\x31\x30\x0D\x06\x09\x60\x86\x48"
		"\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20";
static const unsigned int rotpk_hash_hdr_len = sizeof(rotpk_hash_hdr) - 1;
static unsigned char rotpk_hash_der[sizeof(rotpk_hash_hdr) - 1 + SHA256_BYTES];

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID)
static const unsigned char arm_devel_rotpk_hash[] =
		"\xB0\xF3\x82\x09\x12\x97\xD8\x3A"
		"\x37\x7A\x72\x47\x1B\xEC\x32\x73"
		"\xE9\x92\x32\xE2\x49\x59\xF6\x5E"
		"\x8B\x4A\x4A\x46\xD8\x22\x9A\xDA";
#endif

#pragma weak plat_rotpk_hash
const unsigned char plat_rotpk_hash[] =
		"\xdb\x06\x67\x95\x4f\x88\x2b\x88"
		"\x49\xbf\x70\x3f\xde\x50\x4a\x96"
		"\xd8\x17\x69\xd4\xa0\x6c\xba\xee"
		"\x66\x3e\x71\x82\x2d\x95\x69\xe4";

#pragma weak rom_slice
const unsigned char rom_slice[] =
		"\x77\x06\xbc\x98\x40\xbe\xfd\xab"
		"\x60\x4b\x74\x3c\x9a\xb3\x80\x75"
		"\x39\xb6\xda\x27\x07\x2e\x5b\xbf"
		"\x5c\x47\x91\xc9\x95\x26\x26\x0c";

#if (ARM_ROTPK_LOCATION_ID == BRCM_ROTPK_SOTP_RSA_ID)
static int plat_is_trusted_boot(void)
{
	uint64_t section3_row0_data;

	section3_row0_data = sotp_mem_read(SOTP_DEVICE_SECURE_CFG0_ROW, 0);

	if ((section3_row0_data & SOTP_DEVICE_SECURE_CFG0_AB_MASK) == 0) {
		INFO("NOT AB\n");
		return 0;
	}

	INFO("AB\n");
	return TRUSTED_BOARD_BOOT;
}

/*
 * FAST AUTH is enabled if all following conditions are met:
 * - AB part
 * - SOTP.DEV != 0
 * - SOTP.CID != 0
 * - SOTP.ENC_DEV_TYPE = ENC_AB_DEV
 * - Manuf_debug strap set high
 */
static int plat_fast_auth_enabled(void)
{
	uint32_t chip_state;
	uint64_t section3_row0_data;
	uint64_t section3_row1_data;

	section3_row0_data =
		sotp_mem_read(SOTP_DEVICE_SECURE_CFG0_ROW, 0);
	section3_row1_data =
		sotp_mem_read(SOTP_DEVICE_SECURE_CFG1_ROW, 0);

	chip_state = mmio_read_32(SOTP_REGS_SOTP_CHIP_STATES);

	if (plat_is_trusted_boot() &&
	    (section3_row0_data & SOTP_DEVICE_SECURE_CFG0_DEV_MASK) &&
	    (section3_row0_data & SOTP_DEVICE_SECURE_CFG0_CID_MASK) &&
	    ((section3_row1_data & SOTP_ENC_DEV_TYPE_MASK) ==
	     SOTP_ENC_DEV_TYPE_AB_DEV) &&
	    (chip_state & SOTP_CHIP_STATES_MANU_DEBUG_MASK))
		return 1;

	return 0;
}
#endif

/*
 * Return the ROTPK hash in the following ASN.1 structure in DER format:
 *
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm         OBJECT IDENTIFIER,
 *     parameters        ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm   AlgorithmIdentifier,
 *     digest            OCTET STRING
 * }
 */
int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	uint8_t *dst;

	assert(key_ptr != NULL);
	assert(key_len != NULL);
	assert(flags != NULL);

	*flags = 0;

	/* Copy the DER header */
	memcpy(rotpk_hash_der, rotpk_hash_hdr, rotpk_hash_hdr_len);
	dst = (uint8_t *)&rotpk_hash_der[rotpk_hash_hdr_len];

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID)
	memcpy(dst, arm_devel_rotpk_hash, SHA256_BYTES);
#elif (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_REGS_ID)
	uint32_t *src, tmp;
	unsigned int words, i;

	/*
	 * Append the hash from Trusted Root-Key Storage registers. The hash has
	 * not been written linearly into the registers, so we have to do a bit
	 * of byte swapping:
	 *
	 *     0x00    0x04    0x08    0x0C    0x10    0x14    0x18    0x1C
	 * +---------------------------------------------------------------+
	 * | Reg0  | Reg1  | Reg2  | Reg3  | Reg4  | Reg5  | Reg6  | Reg7  |
	 * +---------------------------------------------------------------+
	 *  | ...                    ... |   | ...                   ...  |
	 *  |       +--------------------+   |                    +-------+
	 *  |       |                        |                    |
	 *  +----------------------------+   +----------------------------+
	 *          |                    |                        |       |
	 *  +-------+                    |   +--------------------+       |
	 *  |                            |   |                            |
	 *  v                            v   v                            v
	 * +---------------------------------------------------------------+
	 * |                               |                               |
	 * +---------------------------------------------------------------+
	 *  0                           15  16                           31
	 *
	 * Additionally, we have to access the registers in 32-bit words
	 */
	words = SHA256_BYTES >> 3;

	/* Swap bytes 0-15 (first four registers) */
	src = (uint32_t *)TZ_PUB_KEY_HASH_BASE;
	for (i = 0 ; i < words ; i++) {
		tmp = src[words - 1 - i];
		/* Words are read in little endian */
		*dst++ = (uint8_t)((tmp >> 24) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 16) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 8) & 0xFF);
		*dst++ = (uint8_t)(tmp & 0xFF);
	}

	/* Swap bytes 16-31 (last four registers) */
	src = (uint32_t *)(TZ_PUB_KEY_HASH_BASE + SHA256_BYTES / 2);
	for (i = 0 ; i < words ; i++) {
		tmp = src[words - 1 - i];
		*dst++ = (uint8_t)((tmp >> 24) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 16) & 0xFF);
		*dst++ = (uint8_t)((tmp >> 8) & 0xFF);
		*dst++ = (uint8_t)(tmp & 0xFF);
	}
#elif (ARM_ROTPK_LOCATION_ID == BRCM_ROTPK_SOTP_RSA_ID)
{
	int i;
	int ret = -1;

	/*
	 * In non-AB mode, we do not read the key.
	 * In AB mode:
	 * - The Dauth is in BL11 if SBL is enabled
	 * - The Dauth is in SOTP if SBL is disabled.
	 */
	if (plat_is_trusted_boot() == 0) {

		INFO("NON-AB: Do not read DAUTH!\n");
		*flags = ROTPK_NOT_DEPLOYED;
		ret = 0;

	} else if ((sbl_status() == SBL_ENABLED) &&
		(mmio_read_32(BL11_DAUTH_BASE) == BL11_DAUTH_ID)) {

		/* Read hash from BL11 */
		INFO("readKeys (DAUTH) from BL11\n");

		memcpy(dst,
			(void *)(BL11_DAUTH_BASE + sizeof(uint32_t)),
			SHA256_BYTES);

		for (i = 0; i < SHA256_BYTES; i++)
			if (dst[i] != 0)
				break;

		if (i >= SHA256_BYTES)
			ERROR("Hash not valid from BL11\n");
		else
			ret = 0;

	} else if (sotp_key_erased()) {

		memcpy(dst, plat_rotpk_hash, SHA256_BYTES);

		INFO("SOTP erased, Use internal key hash.\n");
		ret = 0;

	} else if (plat_fast_auth_enabled()) {

		INFO("AB DEV: FAST AUTH!\n");
		*flags = ROTPK_NOT_DEPLOYED;
		ret = 0;

	} else if (!(mmio_read_32(SOTP_STATUS_1) & SOTP_DAUTH_ECC_ERROR_MASK)) {

		/* Read hash from SOTP */
		ret = sotp_read_key(dst,
				    SHA256_BYTES,
				    SOTP_DAUTH_ROW,
				    SOTP_K_HMAC_ROW-1);

		INFO("sotp_read_key (DAUTH): %i\n", ret);

	} else {

		uint64_t row_data;
		uint32_t k;

		for (k = 0; k < (SOTP_K_HMAC_ROW - SOTP_DAUTH_ROW); k++) {
			row_data = sotp_mem_read(SOTP_DAUTH_ROW + k,
					SOTP_ROW_NO_ECC);

			if (row_data != 0)
				break;
		}

		if (k == (SOTP_K_HMAC_ROW - SOTP_DAUTH_ROW)) {
			INFO("SOTP NOT PROGRAMMED: Do not use DAUTH!\n");

			if (sotp_mem_read(SOTP_ATF2_CFG_ROW_ID,
					SOTP_ROW_NO_ECC) & SOTP_ROMKEY_MASK) {
				memcpy(dst, plat_rotpk_hash, SHA256_BYTES);

				INFO("Use internal key hash.\n");
				ret = 0;
			} else {
				*flags = ROTPK_NOT_DEPLOYED;
				ret = 0;
			}
		} else {
			INFO("No hash found in SOTP\n");
		}
	}
	if (ret)
		return ret;
}
#endif

	*key_ptr = (void *)rotpk_hash_der;
	*key_len = (unsigned int)sizeof(rotpk_hash_der);
	*flags |= ROTPK_IS_HASH;

	return 0;
}

#define SOTP_NUM_BITS_PER_ROW 41
#define SOTP_NVCTR_ROW_ALL_ONES 0x1ffffffffff
#define SOTP_NVCTR_TRUSTED_IN_USE \
		((uint64_t)0x3 << (SOTP_NUM_BITS_PER_ROW-2))
#define SOTP_NVCTR_NON_TRUSTED_IN_USE ((uint64_t)0x3)
#define SOTP_NVCTR_TRUSTED_NEAR_END SOTP_NVCTR_NON_TRUSTED_IN_USE
#define SOTP_NVCTR_NON_TRUSTED_NEAR_END SOTP_NVCTR_TRUSTED_IN_USE

#define SOTP_NVCTR_ROW_START 64
#define SOTP_NVCTR_ROW_END   75

/*
 * SOTP NVCTR are stored in section 10 of SOTP (rows 64-75).
 * Each row of SOTP is 41 bits.
 * NVCTR's are stored in a bitstream format.
 * We are tolerant to consecutive bit errors.
 * Trusted NVCTR starts at the top of row 64 in bitstream format.
 * Non Trusted NVCTR starts at the bottom of row 75 in reverse bitstream.
 * Each row can only be used by 1 of the 2 counters.  This is determined
 * by 2 zeros remaining at the beginning or end of the last available row.
 * If one counter has already starting using a row, the other will be
 * prevent from writing to that row.
 *
 * Example counter values for SOTP programmed below:
 * Trusted Counter (rows64-69) = 5 * 41 + 40 = 245
 * NonTrusted Counter (row75-71) = 3 * 41 + 4 = 127
 *        40 39 38 37 36 ..... 5 4 3 2 1 0
 * row 64  1  1  1  1  1       1 1 1 1 1 1
 * row 65  1  1  1  1  1       1 1 1 1 1 1
 * row 66  1  1  1  1  1       1 1 1 1 1 1
 * row 67  1  1  1  1  1       1 1 1 1 1 1
 * row 68  1  1  1  1  1       1 1 1 1 1 1
 * row 69  1  1  1  1  1       1 1 1 1 1 0
 * row 71  0  0  0  0  0       0 0 0 0 0 0
 * row 71  0  0  0  0  0       0 0 0 0 0 0
 * row 71  0  0  0  0  0       0 0 1 1 1 1
 * row 73  1  1  1  1  1       1 1 1 1 1 1
 * row 74  1  1  1  1  1       1 1 1 1 1 1
 * row 75  1  1  1  1  1       1 1 1 1 1 1
 *
 */

#if (DEBUG == 1)
/*
 * Dump sotp rows
 */
void sotp_dump_rows(uint32_t start_row, uint32_t end_row)
{
	int32_t rownum;
	uint64_t rowdata;

	for (rownum = start_row; rownum <= end_row; rownum++) {
		rowdata = sotp_mem_read(rownum, SOTP_ROW_NO_ECC);
		INFO("%d 0x%llx\n", rownum, rowdata);
	}
}
#endif

/*
 * Get SOTP Trusted nvctr
 */
unsigned int sotp_get_trusted_nvctr(void)
{
	uint64_t rowdata;
	uint64_t nextrowdata;
	uint32_t rownum;
	unsigned int nvctr;

	rownum = SOTP_NVCTR_ROW_START;
	nvctr = SOTP_NUM_BITS_PER_ROW;

	/*
	 * Determine what row has last valid data for trusted ctr
	 */
	rowdata = sotp_mem_read(rownum, SOTP_ROW_NO_ECC);
	while ((rowdata & SOTP_NVCTR_TRUSTED_IN_USE) &&
	       (rowdata & SOTP_NVCTR_TRUSTED_NEAR_END) &&
	       (rownum < SOTP_NVCTR_ROW_END)) {
		/*
		 * Current row in use and has data in last 2 bits as well.
		 * Check if next row also has data for this counter
		 */
		nextrowdata = sotp_mem_read(rownum+1, SOTP_ROW_NO_ECC);
		if (nextrowdata & SOTP_NVCTR_TRUSTED_IN_USE) {
			/* Next row also has data so increment rownum */
			rownum++;
			nvctr += SOTP_NUM_BITS_PER_ROW;
			rowdata = nextrowdata;
		} else {
			/* Next row does not have data */
			break;
		}
	}

	if (rowdata & SOTP_NVCTR_TRUSTED_IN_USE) {
		while ((rowdata & 0x1) == 0) {
			nvctr--;
			rowdata >>= 1;
		}
	} else
		nvctr -= SOTP_NUM_BITS_PER_ROW;

	INFO("CTR %i\n", nvctr);
	return nvctr;
}

/*
 * Get SOTP NonTrusted nvctr
 */
unsigned int sotp_get_nontrusted_nvctr(void)
{
	uint64_t rowdata;
	uint64_t nextrowdata;
	uint32_t rownum;
	unsigned int nvctr;

	nvctr = SOTP_NUM_BITS_PER_ROW;
	rownum = SOTP_NVCTR_ROW_END;

	/*
	 * Determine what row has last valid data for nontrusted ctr
	 */
	rowdata = sotp_mem_read(rownum, SOTP_ROW_NO_ECC);
	while ((rowdata & SOTP_NVCTR_NON_TRUSTED_NEAR_END) &&
	       (rowdata & SOTP_NVCTR_NON_TRUSTED_IN_USE) &&
	       (rownum > SOTP_NVCTR_ROW_START)) {
		/*
		 * Current row in use and has data in last 2 bits as well.
		 * Check if next row also has data for this counter
		 */
		nextrowdata = sotp_mem_read(rownum-1, SOTP_ROW_NO_ECC);
		if (nextrowdata & SOTP_NVCTR_NON_TRUSTED_IN_USE) {
			/* Next row also has data so decrement rownum */
			rownum--;
			nvctr += SOTP_NUM_BITS_PER_ROW;
			rowdata = nextrowdata;
		} else {
			/* Next row does not have data */
			break;
		}
	}

	if (rowdata & SOTP_NVCTR_NON_TRUSTED_IN_USE) {
		while ((rowdata & ((uint64_t)0x1 << (SOTP_NUM_BITS_PER_ROW-1)))
			==
			0) {
			nvctr--;
			rowdata <<= 1;
		}
	} else
		nvctr -= SOTP_NUM_BITS_PER_ROW;

	INFO("NCTR %i\n", nvctr);
	return nvctr;
}

/*
 * Set SOTP Trusted nvctr
 */
int sotp_set_trusted_nvctr(unsigned int nvctr)
{
	int numrows_available;
	uint32_t nontrusted_rownum;
	uint32_t trusted_rownum;
	uint64_t rowdata;
	unsigned int maxnvctr;

	/*
	 * Read SOTP to find out how many rows are used by the
	 * NON Trusted nvctr
	 */
	nontrusted_rownum = SOTP_NVCTR_ROW_END;
	do {
		rowdata = sotp_mem_read(nontrusted_rownum, SOTP_ROW_NO_ECC);
		if (rowdata & SOTP_NVCTR_NON_TRUSTED_IN_USE)
			nontrusted_rownum--;
		else
			break;
	} while (nontrusted_rownum >= SOTP_NVCTR_ROW_START);

	/*
	 * Calculate maximum value we can have for nvctr based on
	 * number of available rows.
	 */
	numrows_available = nontrusted_rownum - SOTP_NVCTR_ROW_START + 1;
	maxnvctr = numrows_available * SOTP_NUM_BITS_PER_ROW;
	if (maxnvctr) {
		/*
		 * Last 2 bits of counter can't be written or it will
		 * overflow with nontrusted counter
		 */
		maxnvctr -= 2;
	}

	if (nvctr > maxnvctr) {
		/* Error - not enough room */
		WARN("tctr not set\n");
		return 1;
	}

	/*
	 * It is safe to write the nvctr, fill all 1's up to the
	 * last row and then fill the last row with partial bitstream
	 */
	trusted_rownum = SOTP_NVCTR_ROW_START;
	rowdata = SOTP_NVCTR_ROW_ALL_ONES;

	while (nvctr >= SOTP_NUM_BITS_PER_ROW) {
		sotp_mem_write(trusted_rownum, SOTP_ROW_NO_ECC, rowdata);
		nvctr -= SOTP_NUM_BITS_PER_ROW;
		trusted_rownum++;
	}
	rowdata <<= (SOTP_NUM_BITS_PER_ROW - nvctr);
	sotp_mem_write(trusted_rownum, SOTP_ROW_NO_ECC, rowdata);
	return 0;
}

/*
 * Set SOTP NonTrusted nvctr
 */
int sotp_set_nontrusted_nvctr(unsigned int nvctr)
{
	int numrows_available;
	uint32_t nontrusted_rownum;
	uint32_t trusted_rownum;
	uint64_t rowdata;
	unsigned int maxnvctr;

	/*
	 * Read SOTP to find out how many rows are used by the
	 * Trusted nvctr
	 */
	trusted_rownum = SOTP_NVCTR_ROW_START;
	do {
		rowdata = sotp_mem_read(trusted_rownum, SOTP_ROW_NO_ECC);
		if (rowdata & SOTP_NVCTR_TRUSTED_IN_USE)
			trusted_rownum++;
		else
			break;
	} while (trusted_rownum <= SOTP_NVCTR_ROW_END);

	/*
	 * Calculate maximum value we can have for nvctr based on
	 * number of available rows.
	 */
	numrows_available = SOTP_NVCTR_ROW_END - trusted_rownum + 1;
	maxnvctr = numrows_available * SOTP_NUM_BITS_PER_ROW;
	if (maxnvctr) {
		/*
		 * Last 2 bits of counter can't be written or it will
		 * overflow with nontrusted counter
		 */
		maxnvctr -= 2;
	}

	if (nvctr > maxnvctr) {
		/* Error - not enough room */
		WARN("nctr not set\n");
		return 1;
	}

	/*
	 * It is safe to write the nvctr, fill all 1's up to the
	 * last row and then fill the last row with partial bitstream
	 */
	nontrusted_rownum = SOTP_NVCTR_ROW_END;
	rowdata = SOTP_NVCTR_ROW_ALL_ONES;

	while (nvctr >= SOTP_NUM_BITS_PER_ROW) {
		sotp_mem_write(nontrusted_rownum, SOTP_ROW_NO_ECC, rowdata);
		nvctr -= SOTP_NUM_BITS_PER_ROW;
		nontrusted_rownum--;
	}
	rowdata >>= (SOTP_NUM_BITS_PER_ROW - nvctr);
	sotp_mem_write(nontrusted_rownum, SOTP_ROW_NO_ECC, rowdata);
	return 0;
}

/*
 * Return the non-volatile counter value stored in the platform. The cookie
 * will contain the OID of the counter in the certificate.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	const char *oid;

	assert(cookie != NULL);
	assert(nv_ctr != NULL);

	*nv_ctr = 0;
	if ((sotp_mem_read(SOTP_ATF_CFG_ROW_ID, SOTP_ROW_NO_ECC) &
			SOTP_ATF_NVCOUNTER_ENABLE_MASK)) {
		oid = (const char *)cookie;
		if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0)
			*nv_ctr = sotp_get_trusted_nvctr();
		else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0)
			*nv_ctr = sotp_get_nontrusted_nvctr();
		else
			return 1;
	}
	return 0;
}

/*
 * Store a new non-volatile counter value.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	const char *oid;

	if (sotp_mem_read(SOTP_ATF_CFG_ROW_ID, SOTP_ROW_NO_ECC) &
			SOTP_ATF_NVCOUNTER_ENABLE_MASK) {
		INFO("set CTR %i\n", nv_ctr);
		oid = (const char *)cookie;
		if (strcmp(oid, TRUSTED_FW_NVCOUNTER_OID) == 0)
			return sotp_set_trusted_nvctr(nv_ctr);
		else if (strcmp(oid, NON_TRUSTED_FW_NVCOUNTER_OID) == 0)
			return sotp_set_nontrusted_nvctr(nv_ctr);
		return 1;
	}
	return 0;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
