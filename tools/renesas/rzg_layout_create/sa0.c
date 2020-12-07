/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define RCAR_SA0_SIZE_SMALL	(0)	/* for RZ/G2E */
#define RCAR_SA0_SIZE_NORMAL	(1)	/* for RZ/G2[HMN] */

#define BL2_ADDRESS	(0xE6304000)	/* BL2 start address */

#if (RCAR_SA0_SIZE == RCAR_SA0_SIZE_SMALL)
#define BL2_SIZE	(80*1024/4)	/* BL2 size is 80KB(0x00005000) */
#else  /* (RCAR_SA0_SIZE == RCAR_SA0_SIZE_SMALL) */
#define BL2_SIZE	(170*1024/4)	/* BL2 size is 170KB(0x0000AA00) */
#endif /* (RCAR_SA0_SIZE == RCAR_SA0_SIZE_SMALL) */

/* SA0 */
/* 0x00000000 */
const unsigned int __attribute__ ((section(".sa0_bootrom"))) bootrom_paramA = 0x00000100;
/* 0x00000080 (Map Type 3 for eMMC Boot)*/
/* 0x000001D4 */
const unsigned int __attribute__ ((section(".sa0_bl2dst_addr3"))) bl2dst_addr3 = BL2_ADDRESS;
/* 0x000002E4 */
const unsigned int __attribute__ ((section(".sa0_bl2dst_size3"))) bl2dst_size3 = BL2_SIZE;
/* 0x00000C00 (Map Type 1 for HyperFlash/QSPI Flash Boot)*/
/* 0x00000D54 */
const unsigned int __attribute__ ((section(".sa0_bl2dst_addr1"))) bl2dst_addr1 = BL2_ADDRESS;
/* 0x00000E64 */
const unsigned int __attribute__ ((section(".sa0_bl2dst_size1"))) bl2dst_size1 = BL2_SIZE;
