/*
 * Copyright (c) 2006-2014, Intel Corporation. All rights reserved.
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include <arch.h>
#include <common/debug.h>
#include <lib/hob/hob.h>
#include <lib/hob/hob_guid.h>
#include <lib/hob/mmram.h>
#include <lib/utils_def.h>

#define ALIGN_UP(x, a)		((x + (a - 1)) & ~(a - 1))

static void *_create_hob(struct efi_hob_handoff_info_table *hob_table,
		uint16_t hob_type, uint16_t hob_length)
{
	size_t free_mem_size;
	struct efi_hob_generic_header *new_hob;
	struct efi_hob_generic_header *hob_end;

	if ((hob_table == NULL) || (hob_length == 0)) {
		return NULL;
	}

	hob_length = ALIGN_UP(hob_length, 8);
	free_mem_size = hob_table->efi_free_memory_top - hob_table->efi_free_memory_bottom;

	/**
	 * hob_length already including sizeof(efi_hob_generic_header).
	 * See the each export interface create_xxx_hob.
	 */
	if ((size_t) hob_length > free_mem_size) {
		return NULL;
	}

	new_hob = (struct efi_hob_generic_header *) hob_table->efi_end_of_hob_list;
	new_hob->hob_type = hob_type;
	new_hob->hob_length = hob_length;
	new_hob->reserved = 0x00;

	hob_end = (struct efi_hob_generic_header *) (hob_table->efi_end_of_hob_list + hob_length);
	hob_end->hob_type = EFI_HOB_TYPE_END_OF_HOB_LIST;
	hob_end->hob_length = sizeof(struct efi_hob_generic_header);
	hob_end->reserved = 0x00;

	hob_table->efi_end_of_hob_list = (efi_physical_address_t) hob_end;
	hob_table->efi_free_memory_bottom = (efi_physical_address_t) (hob_end + 1);

	return new_hob;
}

/*
 * Create PHIT HOB list.
 *
 * On success, return the address PHIT HOB list
 * On error, return NULL.
 *
 * efi_memory_begin
 *   Base address for partition.
 * efi_memory_length
 *   Size of memory for patition.
 * efi_free_memory_bottom
 *   Base address PHIT HOB list can be allocated
 * efi_free_memory_length.
 *   Maximum size of PHIT HOB list can have
 */
struct efi_hob_handoff_info_table *create_hob_list(
		efi_physical_address_t efi_memory_begin, size_t efi_memory_length,
		efi_physical_address_t efi_free_memory_bottom, size_t efi_free_memory_length)
{
	struct efi_hob_handoff_info_table *hob_table;
	struct efi_hob_generic_header *hob_end;

	if ((efi_memory_begin == 0) || (efi_free_memory_bottom == 0) ||
			(efi_memory_length == 0) || (efi_free_memory_length == 0)) {
		return NULL;
	}

	hob_table = (struct efi_hob_handoff_info_table *) efi_free_memory_bottom;
	hob_end = (struct efi_hob_generic_header *) (hob_table + 1);

	hob_table->header.hob_type = EFI_HOB_TYPE_HANDOFF;
	hob_table->header.hob_length = sizeof(struct efi_hob_handoff_info_table);
	hob_table->header.reserved = 0;

	hob_end->hob_type = EFI_HOB_TYPE_END_OF_HOB_LIST;
	hob_end->hob_length = sizeof(struct efi_hob_generic_header);
	hob_end->reserved = 0;

	hob_table->version = EFI_HOB_HANDOFF_TABLE_VERSION;
	hob_table->boot_mode = EFI_BOOT_WITH_FULL_CONFIGURATION;

	hob_table->efi_memory_top = efi_memory_begin + efi_memory_length;
	hob_table->efi_memory_bottom = efi_memory_begin;
	hob_table->efi_free_memory_top = efi_memory_begin + efi_free_memory_length;
	hob_table->efi_free_memory_bottom = (efi_physical_address_t) (hob_end + 1);
	hob_table->efi_end_of_hob_list = (efi_physical_address_t) hob_end;

	return hob_table;
}

/*
 * Create resource description HOB in PHIT HOB list.
 *
 * On success, return 0.
 * On error, return error code.
 *
 * hob_table
 *   Address of PHIT HOB list
 * resource_type
 *   Resource type see EFI_RESOURCE_* in the include/lib/hob/efi_types.h
 * resource_attribute
 *   Resource attribute see EFI_RESOURCE_ATTRIBUTES_*
 *   in the include/lib/hob/efi_types.h
 * phy_addr_start
 *   Physical base address of resource
 * resource_length
 *   Size of resource
 */
int create_resource_descriptor_hob(
		struct efi_hob_handoff_info_table *hob_table,
		efi_resource_type_t resource_type,
		efi_resource_attribute_type_t resource_attribute,
		efi_physical_address_t phy_addr_start,
		uint64_t resource_length)
{
	struct efi_hob_resource_descriptor *rd_hop;

	rd_hop = _create_hob(hob_table, EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,
			sizeof(struct efi_hob_resource_descriptor));

	if (rd_hop == NULL) {
		ERROR("No space for creating resource descriptor type hob...\n");
		return -ENOMEM;
	}

	rd_hop->resource_type = resource_type;
	rd_hop->resource_attribute = resource_attribute;
	rd_hop->physical_start = phy_addr_start;
	rd_hop->resource_length = resource_length;
	memset(&rd_hop->owner, 0, sizeof(struct efi_guid));

	return 0;
}

/*
 * Create GUID HOB in PHIT HOB list.
 *
 * On success, return 0.
 * On error, return error code.
 *
 * hob_table
 *   Address of PHIT HOB list
 * guid
 *   guid.
 * data length
 *   Size of data
 * data
 *   Data
 */
int create_guid_hob(struct efi_hob_handoff_info_table *hob_table,
		struct efi_guid *guid, uint16_t data_length, void **data)
{
	struct efi_hob_guid_type *guid_hob;
	uint16_t hob_length;

	hob_length = data_length + sizeof(struct efi_hob_guid_type);

	if ((guid == NULL) || (data == NULL) || (hob_length < data_length)) {
		return -EINVAL;
	}

	guid_hob = _create_hob(hob_table, EFI_HOB_TYPE_GUID_EXTENSION, hob_length);
	if (guid_hob == NULL) {
		ERROR("No space for creating guid type hob...\n");
		return -ENOMEM;
	}

	memcpy(&guid_hob->name, guid, sizeof(struct efi_guid));

	*data = (void *) (guid_hob + 1);

	return 0;
}

/*
 * Create Firmware Volume HOB in PHIT HOB list.
 *
 * On success, return 0.
 * On error, return error code.
 *
 * hob_table
 *   Address of PHIT HOB list
 * base_addr
 *   Base address of firmware volume
 * size
 *   Size of Firmware Volume
 */
int create_fv_hob(struct efi_hob_handoff_info_table *hob_table,
		efi_physical_address_t base_addr, uint64_t size)
{
	struct efi_hob_firmware_volume *fv_hob;

	fv_hob = _create_hob(hob_table, EFI_HOB_TYPE_FV,
			sizeof(struct efi_hob_firmware_volume));
	if (fv_hob == NULL) {
		ERROR("No space for creating fv type hob...\n");
		return -ENOMEM;
	}

	fv_hob->base_address = base_addr;
	fv_hob->length = size;

	return 0;
}
