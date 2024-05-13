/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HOB_H
#define HOB_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/hob/efi_types.h>
#include <lib/utils_def.h>
#include <tools_share/uuid.h>

/*****************************************************************************
 *                            Hob Generic Header                             *
 *****************************************************************************/

/**
 * HobType values of EFI_HOB_GENERIC_HEADER.
 */
#define EFI_HOB_TYPE_HANDOFF              U(0x0001)
#define EFI_HOB_TYPE_MEMORY_ALLOCATION    U(0x0002)
#define EFI_HOB_TYPE_RESOURCE_DESCRIPTOR  U(0x0003)
#define EFI_HOB_TYPE_GUID_EXTENSION       U(0x0004)
#define EFI_HOB_TYPE_FV                   U(0x0005)
#define EFI_HOB_TYPE_CPU                  U(0x0006)
#define EFI_HOB_TYPE_MEMORY_POOL          U(0x0007)
#define EFI_HOB_TYPE_FV2                  U(0x0009)
#define EFI_HOB_TYPE_LOAD_PEIM_UNUSED     U(0x000A)
#define EFI_HOB_TYPE_UEFI_CAPSULE         U(0x000B)
#define EFI_HOB_TYPE_FV3                  U(0x000C)
#define EFI_HOB_TYPE_UNUSED               U(0xFFFE)
#define EFI_HOB_TYPE_END_OF_HOB_LIST      U(0xFFFF)

struct efi_hob_generic_header {
	uint16_t hob_type;
	uint16_t hob_length;
	uint32_t reserved;
};

/*****************************************************************************
 *                               PHIT Hob.                                   *
 *****************************************************************************/

#define EFI_HOB_HANDOFF_TABLE_VERSION     U(0x000a)

struct efi_hob_handoff_info_table {
	struct efi_hob_generic_header header;
	uint32_t version;
	efi_boot_mode_t  boot_mode;
	efi_physical_address_t efi_memory_top;
	efi_physical_address_t efi_memory_bottom;
	efi_physical_address_t efi_free_memory_top;
	efi_physical_address_t efi_free_memory_bottom;
	efi_physical_address_t efi_end_of_hob_list;
};

/*****************************************************************************
 *                       Resource Descriptor Hob.                            *
 *****************************************************************************/

struct efi_hob_resource_descriptor {
	struct efi_hob_generic_header header;
	struct efi_guid owner;
	efi_resource_type_t resource_type;
	efi_resource_attribute_type_t resource_attribute;
	efi_physical_address_t physical_start;
	uint64_t resource_length;
};

/*****************************************************************************
 *                           Guid Extension Hob.                             *
 *****************************************************************************/
struct efi_hob_guid_type {
	struct efi_hob_generic_header header;
	struct efi_guid name;
	/**
	 * Guid specific data goes here.
	 */
};

/*****************************************************************************
 *                           Firmware Volume Hob.                            *
 *****************************************************************************/
struct efi_hob_firmware_volume {
	struct efi_hob_generic_header header;
	efi_physical_address_t base_address;
	uint64_t length;
	/**
	 * Guid specific data goes here.
	 */
};

/*****************************************************************************
 *                              Interfaces.                                  *
 *****************************************************************************/

struct efi_hob_handoff_info_table *
create_hob_list(
		efi_physical_address_t efi_memory_begin, size_t efi_memory_length,
		efi_physical_address_t efi_free_memory_bottom, size_t efi_free_memory_length);

int create_resource_descriptor_hob(
		struct efi_hob_handoff_info_table *hob_table,
		efi_resource_type_t resource_type,
		efi_resource_attribute_type_t resource_attribute,
		efi_physical_address_t phy_addr_start,
		uint64_t resource_length);

int create_guid_hob(struct efi_hob_handoff_info_table *hob_table,
		struct efi_guid *guid, uint16_t data_length, void **data);

int create_fv_hob(struct efi_hob_handoff_info_table *hob_table,
		efi_physical_address_t base_addr, uint64_t size);

#endif /* HOB_H */
