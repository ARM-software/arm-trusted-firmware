// SPDX-License-Identifier: BSD-3-Clause
/*
 *  Copyright 2020-2021 NXP
 */

/**
 * @Flash info
 *
 */
#ifndef FLASH_INFO_H
#define FLASH_INFO_H

#define SZ_16M_BYTES			0x1000000U

/* Start of "if defined(CONFIG_MT25QU512A)" */
#if defined(CONFIG_MT25QU512A)
#define F_SECTOR_64K			0x10000U
#define F_PAGE_256			0x100U
#define F_SECTOR_4K			0x1000U
#define F_FLASH_SIZE_BYTES		0x4000000U
#define F_SECTOR_ERASE_SZ		F_SECTOR_64K
#ifdef CONFIG_FSPI_4K_ERASE
#define F_SECTOR_ERASE_SZ		F_SECTOR_4K
#endif

/* End of "if defined(CONFIG_MT25QU512A)" */

/* Start of "if defined(CONFIG_MX25U25645G)" */
#elif defined(CONFIG_MX25U25645G)
#define F_SECTOR_64K			0x10000U
#define F_PAGE_256			0x100U
#define F_SECTOR_4K			0x1000U
#define F_FLASH_SIZE_BYTES		0x2000000U
#define F_SECTOR_ERASE_SZ		F_SECTOR_64K
#ifdef CONFIG_FSPI_4K_ERASE
#define F_SECTOR_ERASE_SZ		F_SECTOR_4K
#endif

/* End of "if defined(CONFIG_MX25U25645G)" */

/* Start of "if defined(CONFIG_MX25U51245G)" */
#elif defined(CONFIG_MX25U51245G)
#define F_SECTOR_64K			0x10000U
#define F_PAGE_256			0x100U
#define F_SECTOR_4K			0x1000U
#define F_FLASH_SIZE_BYTES		0x4000000U
#define F_SECTOR_ERASE_SZ		F_SECTOR_64K
#ifdef CONFIG_FSPI_4K_ERASE
#define F_SECTOR_ERASE_SZ		F_SECTOR_4K
#endif

/* End of "if defined(CONFIG_MX25U51245G)" */

/* Start of "if defined(CONFIG_MT35XU512A)" */
#elif defined(CONFIG_MT35XU512A)
#define F_SECTOR_128K			0x20000U
#define F_SECTOR_32K			0x8000U
#define F_PAGE_256			0x100U
#define F_SECTOR_4K			0x1000U
#define F_FLASH_SIZE_BYTES		0x4000000U
#define F_SECTOR_ERASE_SZ		F_SECTOR_128K
#ifdef CONFIG_FSPI_4K_ERASE
#define F_SECTOR_ERASE_SZ		F_SECTOR_4K
#endif
/* If Warm boot is enabled for the platform,
 * count of arm instruction N-OP(s) to mark
 * the completion of write operation to flash;
 * varies from one flash to other.
 */
#ifdef NXP_WARM_BOOT
#define FLASH_WR_COMP_WAIT_BY_NOP_COUNT	0x20000
#endif

/* End of "if defined(CONFIG_MT35XU512A)" */

/* Start of #elif defined(CONFIG_MT35XU02G) */
#elif defined(CONFIG_MT35XU02G)
#define F_SECTOR_128K			0x20000U
#define F_PAGE_256			0x100U
#define F_SECTOR_4K			0x1000U
#define F_FLASH_SIZE_BYTES		0x10000000U
#define F_SECTOR_ERASE_SZ		F_SECTOR_128K
#ifdef CONFIG_FSPI_4K_ERASE
#define F_SECTOR_ERASE_SZ		F_SECTOR_4K
#endif

/* End of #elif defined(CONFIG_MT35XU02G) */

/* Start of #elif Micron MT25QU01GBBB, GigaDevice GD55LB01GF, Macronix MX66U1G45G, Winbond W25Q01NW */
#elif defined(CONFIG_MT25QU01GBBB) || \
      defined(CONFIG_GD55LB01GF)   || \
      defined(CONFIG_MX66U1G45G)   || \
      defined(CONFIG_W25Q01NW)

/* Common geometry for 1Gbit (128MiB) */
#define F_SECTOR_64K			0x10000U
#define F_PAGE_256			0x100U
#define F_SECTOR_4K			0x1000U
#define F_FLASH_SIZE_BYTES		0x8000000U   /* 128 MiB */
#define F_SECTOR_ERASE_SZ		F_SECTOR_64K
#ifdef CONFIG_FSPI_4K_ERASE
#undef  F_SECTOR_ERASE_SZ
#define F_SECTOR_ERASE_SZ		F_SECTOR_4K
#endif

/* End of #elif Micron MT25QU01GBBB, GigaDevice GD55LB01GF, Macronix MX66U1G45G, Winbond W25Q01NW */

/* Start of #elif GigaDevice GD55LB02GF (2Gbit/256 MiB) */
#elif defined(CONFIG_GD55LB02GF)

/*
 * GigaDevice GD55LB02GF: 2 Gbit uniform-sector xSPI NOR.
 *   JEDEC RDID : C8 60 1C  (mfg GigaDevice, memtype 0x60 = xSPI NOR,
 *                            capacity 0x1C = 2 Gbit/256 MB)
 *   page  : 256B  (PP opcode 0x02)
 *   erase : 4KB (0x20), 32KB (0x52), 64KB (0xD8)
 *   total : 256MiB
 */
#define F_SECTOR_64K			0x10000U
#define F_PAGE_256			0x100U
#define F_SECTOR_4K			0x1000U
#define F_FLASH_SIZE_BYTES		0x10000000U  /* 256 MiB */
#ifdef CONFIG_FSPI_4K_ERASE
#define F_SECTOR_ERASE_SZ		F_SECTOR_4K
#else
#define F_SECTOR_ERASE_SZ		F_SECTOR_64K
#endif

/* End of #elif GigaDevice GD55LB02GF */
#endif

#endif /* FLASH_INFO_H */
