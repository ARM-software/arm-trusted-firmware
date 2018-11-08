/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MBR_H
#define MBR_H

#define MBR_OFFSET			0

#define MBR_PRIMARY_ENTRY_OFFSET	0x1be
#define MBR_PRIMARY_ENTRY_SIZE		0x10
#define MBR_PRIMARY_ENTRY_NUMBER	4
#define MBR_CHS_ADDRESS_LEN		3

#define MBR_SIGNATURE_FIRST		0x55
#define MBR_SIGNATURE_SECOND		0xAA

typedef struct mbr_entry {
	unsigned char		status;
	unsigned char		first_sector[MBR_CHS_ADDRESS_LEN];
	unsigned char		type;
	unsigned char		last_sector[MBR_CHS_ADDRESS_LEN];
	unsigned int		first_lba;
	unsigned int		sector_nums;
} mbr_entry_t;

#endif /* MBR_H */
