/*
 * Copyright (c) 2021-2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STM32MP_IO_STORAGE_H
#define STM32MP_IO_STORAGE_H

#include <stdint.h>

#include <drivers/io/io_storage.h>

/*******************************************************************************
 * STM32MP1 RAW partition offset for devices without GPT
 ******************************************************************************/
#define STM32MP_EMMC_BOOT_FIP_OFFSET	U(0x00040000)
#if PSA_FWU_SUPPORT
#define STM32MP_NOR_METADATA1_OFFSET	U(0x00080000)
#define STM32MP_NOR_METADATA2_OFFSET	U(0x000C0000)
#define STM32MP_NOR_FIP_A_OFFSET	U(0x00100000)
#define STM32MP_NOR_FIP_A_GUID		(const struct efi_guid)EFI_GUID(0x4fd84c93,  \
					0x54ef, 0x463f, 0xa7, 0xef, 0xae, 0x25, 0xff,\
					0x88, 0x70, 0x87)

#define STM32MP_NOR_FIP_B_OFFSET	U(0x00500000)
#define STM32MP_NOR_FIP_B_GUID		(const struct efi_guid)EFI_GUID(0x09c54952,  \
					0xd5bf, 0x45af, 0xac, 0xee, 0x33, 0x53, 0x03,\
					0x76, 0x6f, 0xb3)

#define STM32MP_NAND_METADATA1_OFFSET	U(0x00100000)
#define STM32MP_NAND_METADATA2_OFFSET	U(0x00180000)
#define STM32MP_NAND_FIP_A_OFFSET	U(0x00200000)
#define STM32MP_NAND_FIP_A_GUID		(const struct efi_guid)EFI_GUID(0x4fd84c93,  \
					0x54ef, 0x463f, 0xa7, 0xef, 0xae, 0x25, 0xff,\
					0x88, 0x70, 0x87)

#define STM32MP_NAND_FIP_B_OFFSET	U(0x00A00000)
#define STM32MP_NAND_FIP_B_GUID		(const struct efi_guid)EFI_GUID(0x09c54952,  \
					0xd5bf, 0x45af, 0xac, 0xee, 0x33, 0x53, 0x03,\
					0x76, 0x6f, 0xb3)

#define STM32MP_NAND_FIP_B_MAX_OFFSET	U(0x01200000)
#else /* PSA_FWU_SUPPORT */
#ifndef STM32MP_NOR_FIP_OFFSET
#define STM32MP_NOR_FIP_OFFSET		U(0x00080000)
#endif
#ifndef STM32MP_NAND_FIP_OFFSET
#define STM32MP_NAND_FIP_OFFSET		U(0x00200000)
#endif
#endif /* PSA_FWU_SUPPORT */

/*
 * Only used for MTD devices that need some backup blocks.
 * Must define a maximum size for a partition.
 */
#define PLATFORM_MTD_MAX_PART_SIZE	U(0x00400000)

#define FIP_IMAGE_NAME			"fip"
#define METADATA_PART_1			"metadata1"
#define METADATA_PART_2			"metadata2"

/* IO devices handle */
extern uintptr_t storage_dev_handle;
extern uintptr_t fip_dev_handle;
extern uintptr_t enc_dev_handle;

extern io_block_spec_t image_block_spec;

/* Function declarations */
int open_fip(const uintptr_t spec);
#ifndef DECRYPTION_SUPPORT_none
int open_enc_fip(const uintptr_t spec);
#endif
int open_storage(const uintptr_t spec);

#endif /* STM32MP_IO_STORAGE_H */
