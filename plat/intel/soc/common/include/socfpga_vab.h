/*
 * Copyright (c) 2020-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_VAB_H
#define SOCFPGA_VAB_H


#include <stdlib.h>
#include "socfpga_fcs.h"

struct fcs_hps_vab_certificate_data {
	uint32_t vab_cert_magic_num;			/* offset 0x10 */
	uint32_t flags;
	uint8_t rsvd0_1[8];
	uint8_t fcs_sha384[FCS_SHA384_WORD_SIZE];	/* offset 0x20 */
};

struct fcs_hps_vab_certificate_header {
	uint32_t cert_magic_num;			/* offset 0 */
	uint32_t cert_data_sz;
	uint32_t cert_ver;
	uint32_t cert_type;
	struct fcs_hps_vab_certificate_data d;		/* offset 0x10 */
	/* keychain starts at offset 0x50 */
};

/* Macros */
#define IS_BYTE_ALIGNED(x, a)		(((x) & ((typeof(x))(a) - 1)) == 0)
#define BYTE_ALIGN(x, a)		__ALIGN_MASK((x), (typeof(x))(a)-1)
#define __ALIGN_MASK(x, mask)		(((x)+(mask))&~(mask))
#define VAB_CERT_HEADER_SIZE		sizeof(struct fcs_hps_vab_certificate_header)
#define VAB_CERT_MAGIC_OFFSET		offsetof(struct fcs_hps_vab_certificate_header, d)
#define VAB_CERT_FIT_SHA384_OFFSET	offsetof(struct fcs_hps_vab_certificate_data, fcs_sha384[0])
#define SDM_CERT_MAGIC_NUM		0x25D04E7F
#define CHUNKSZ_PER_WD_RESET		(256 * 1024)

/* SHA related return Macro */
#define ENOVABIMG		1 /* VAB certificate not available */
#define EIMGERR		2 /* Image format/size not valid */
#define ETIMEOUT		3 /* Execution timeout */
#define EPROCESS		4 /* Process error */
#define EKEYREJECTED		5/* Key was rejected by service */

/* Function Definitions */
static size_t get_img_size(uint8_t *img_buf, size_t img_buf_sz);
int socfpga_vendor_authentication(void **p_image, size_t *p_size);
static uint32_t get_unaligned_le32(const void *p);
void sha384_csum_wd(const unsigned char *input, unsigned int ilen,
unsigned char *output, unsigned int chunk_sz);

#endif
