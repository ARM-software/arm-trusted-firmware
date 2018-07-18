/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#ifndef __SATA_AHCI_H__
#define __SATA_AHCI_H__

#define SATA_SIG_PMP	0x96690101
#define SATA_SIG_ATA	0x00000101
#define SATA_SIG_ATAPI	0xEB140101
#define SATA_SIG_SEMB	0xC33C0101

//
// Command List structure includes total 32 entries.
// The entry data structure is listed at the following.
//
struct ahci_command_list {
	uint32_t cmd_cfl:5;      //Command FIS Length
	uint32_t cmd_a:1;        //ATAPI
	uint32_t cmd_w:1;        //Write
	uint32_t cmd_p:1;        //Prefetchable
	uint32_t cmd_r:1;        //Reset
	uint32_t cmd_b:1;        //BIST
	uint32_t cmd_c:1;        //Clear Busy upon R_OK
	uint32_t cmd_rsvd:1;
	uint32_t cmd_pmp:4;      //Port Multiplier Port
	uint32_t cmd_prdtl:16;   //Physical Region Descriptor Table Length
	uint32_t cmd_prdbc;      //Physical Region Descriptor Byte Count
	uint32_t cmd_ctba;       //Command Table Descriptor Base Address
	uint32_t cmd_ctbau;      //Command Table Descriptor Base Address Upper 32-BITs
	uint32_t cmd_rsvd1[4];
} __attribute__((packed));

//
// Received FIS structure
//
struct ahci_received_fis {
	uint8_t  dma_setup[0x1C];         // Dma Setup Fis: offset 0x00
	uint8_t  dma_setup_rsvd[0x04];
	uint8_t  pio_setup[0x14];         // Pio Setup Fis: offset 0x20
	uint8_t  pio_setup_rsvd[0x0C];
	uint8_t  d2h_register[0x14];      // D2H Register Fis: offset 0x40
	uint8_t  d2h_register_rsvd[0x04];
	uint64_t set_device_bits;         // Set Device Bits Fix: offset 0x58
	uint8_t  unknown[0x40];           // Unkonwn Fis: offset 0x60
	uint8_t  unknown_rsvd[0x60];
} __attribute__((packed));

//
// This is a software constructed FIS.
// For data transfer operations, this is the H2D Register FIS format as
// specified in the Serial ATA Revision 2.6 specification.
//
struct ahci_command_fis {
	uint8_t  type;
	uint8_t  pmnum:4;
	uint8_t  rsvd:1;
	uint8_t  rsvd1:1;
	uint8_t  rsvd2:1;
	uint8_t  cmdind:1;
	uint8_t  cmd;
	uint8_t  feature;
	uint8_t  secnum;
	uint8_t  clylow;
	uint8_t  clyhigh;
	uint8_t  devhead;
	uint8_t  secnumexp;
	uint8_t  clylowexp;
	uint8_t  clyhighexp;
	uint8_t  featureexp;
	uint8_t  seccount;
	uint8_t  seccountexp;
	uint8_t  rsvd3;
	uint8_t  control;
	uint8_t  rsvd4[4];
	uint8_t  rsvd5[44];
} __attribute__((packed));

#define AHCI_FIS_REGISTER_H2D              0x27      //Register FIS - Host to Device
#define   AHCI_FIS_REGISTER_H2D_LENGTH     5

#define ATA_CTL_SRST (1 << 2)

#define ATA_TFD_BSY (1 << 7)
#define ATA_TFD_DRQ (1 << 3)
#define ATA_TFD_ERR (1 << 0)

#endif
