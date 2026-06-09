/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include "smem/smem.h"
#include "smem/smem_internal.h"
#include "smem/smem_partition.h"
#include "smem/smem_toc.h"
#include "smem/smem_version.h"

/* Mask for smallest possible page mapping size */
#define SMEM_PAGE_ALIGN_MASK		0x00000FFF

#define SMEM_PART_INVALID_HOST_INDX	0xFFFFFFFF

const struct smem_funcs smem_part_funcs = {
	smem_part_get_addr,
};

static struct smem_partition_info *smem_info_prt;
static uint32_t smem_info_num_prt;

static int32_t smem_part_process_item(
	struct smem_partition_info *info, struct smem_alloc_params *params,
	struct smem_partition_allocation_header **item_hdr, uint16_t padding_header,
	uint32_t *size_remaining, bool cached)
{
	uint32_t size_item;
	uint16_t smem_type;
	uint16_t canary;
	uint32_t size_total;
	uint32_t padding_data;
	bool found = false;

	canary = mmio_read_16((uintptr_t)&((*item_hdr)->canary));
	smem_type = mmio_read_16((uintptr_t)&((*item_hdr)->smem_type));
	size_item = mmio_read_32((uintptr_t)&((*item_hdr)->size));

	size_total = size_item + padding_header +
		     sizeof(struct smem_partition_allocation_header);

	if (canary != SMEM_ALLOC_HDR_CANARY) {
		ERROR("SMEM: invalid canary 0x%X at location 0x%X\n",
		      canary, (uint32_t)(uintptr_t)(*item_hdr));
		return SMEM_STATUS_FAILURE;
	}

	if (size_item > info->size) {
		ERROR("SMEM: invalid item size:%d, partition size:%d smem_type:0x%04X\n",
		      size_item, info->size, smem_type);
		return SMEM_STATUS_FAILURE;
	}

	if (size_total > *size_remaining) {
		ERROR("SMEM: invalid item size. Total size: %d, remaining: %d\n",
		      size_total, *size_remaining);
		return SMEM_STATUS_FAILURE;
	}

	*size_remaining -= size_total;

	do {
		if (smem_type != params->smem_type) {
			break;
		}

		if (cached) {
			params->buffer =
				(void *)((size_t)(*item_hdr) - size_item);
			params->flags |= SMEM_ALLOC_FLAG_CACHED;
		} else {
			params->buffer =
				(void *)((size_t)(*item_hdr) +
					 sizeof(struct smem_partition_allocation_header));
			params->flags &= ~SMEM_ALLOC_FLAG_CACHED;
		}

		padding_data = mmio_read_16((uintptr_t)&(*item_hdr)->padding_data);
		if (padding_data > size_item) {
			ERROR("SMEM: invalid padding data size:%d, size_item:%d smem_type:0x%04X\n",
			      padding_data, size_item, smem_type);
			return SMEM_STATUS_FAILURE;
		}

		params->size = size_item - padding_data;

		params->size = ROUND_UP(params->size, 8);

		found = true;
	} while (0);

	if (cached) {
		*item_hdr = (struct smem_partition_allocation_header
				     *)((size_t)(*item_hdr) - size_total);
	} else {
		*item_hdr = (struct smem_partition_allocation_header
				     *)((size_t)(*item_hdr) + size_total);
	}

	return found ? SMEM_STATUS_SUCCESS : SMEM_STATUS_NOT_FOUND;
}

static bool smem_part_is_host_present(struct smem_toc_entry *toc_entry)
{
	uint16_t host0 = mmio_read_16((uintptr_t)&toc_entry->host0);
	uint16_t host1 = mmio_read_16((uintptr_t)&toc_entry->host1);
	uint32_t size = mmio_read_32((uintptr_t)&toc_entry->size);

	if (!size) {
		return false;
	}

	/* Check for common partition, host ids should be 0xFFFE */
	if (host0 == SMEM_COMMON_HOST && host1 == SMEM_COMMON_HOST) {
		return true;
	}

	return false;
}

static struct smem_partition_info *smem_part_info_alloc(uint32_t *part_info_cnt)
{
	static struct smem_partition_info smem_info_prt_static = { 0 };

	*part_info_cnt = 1; /* Only common partition allocation supported */

	return &smem_info_prt_static;
}

static struct smem_partition_info *smem_part_info_get(uint16_t remote_host)
{
	uint32_t indx;

	if ((remote_host == SMEM_INVALID_HOST) ||
		(remote_host == smem_info.this_host)) {
		remote_host = SMEM_COMMON_HOST;
	}

	for (indx = 0; indx < smem_info_num_prt; indx++) {
		struct smem_partition_info *info = &smem_info_prt[indx];

		if (info->host0 == remote_host || info->host1 == remote_host) {
			return info;
		}
	}

	return NULL;
}


int32_t smem_part_get_addr_ex(struct smem_alloc_params *params)
{
	struct smem_partition_info *info;
	struct smem_partition_allocation_header *item_hdr;
	uint32_t size_remaining;
	uint8_t *limit_addr;
	uint16_t padding_header;
	int32_t ret;

	info = smem_part_info_get(params->remote_host);
	if (!info) {
		params->buffer = NULL;
		return SMEM_STATUS_NOT_FOUND;
	}

	info->offset_free_cached =
		mmio_read_32((uintptr_t)&info->header->offset_free_cached);
	info->offset_free_uncached =
		mmio_read_32((uintptr_t)&info->header->offset_free_uncached);

	if ((info->offset_free_uncached > info->offset_free_cached) ||
	    (info->offset_free_uncached < sizeof(struct smem_partition_header)) ||
	    (info->offset_free_uncached > info->size) ||
	    (info->offset_free_cached > info->size)) {
		ERROR("SMEM: invalid heap pointers. Uncached: 0x%X, cached: 0x%X\n",
		      info->offset_free_uncached, info->offset_free_cached);
		return SMEM_STATUS_OUT_OF_RESOURCES;
	}

	size_remaining = info->size - sizeof(struct smem_partition_header);

	padding_header = 0;

	item_hdr = (struct smem_partition_allocation_header
			    *)((uint8_t *)info->header +
			       sizeof(struct smem_partition_header));

	limit_addr = (uint8_t *)info->header + info->offset_free_uncached;

	while ((uint8_t *)item_hdr +
		       sizeof(struct smem_partition_allocation_header) <=
	       limit_addr) {
		ret = smem_part_process_item(
			info,
			params,
			&item_hdr,
			padding_header,
			&size_remaining,
			false);

		if (ret == SMEM_STATUS_SUCCESS || ret == SMEM_STATUS_FAILURE) {
			return ret;
		}
	}

	padding_header = SMEM_PARTITION_ITEM_PADDING(info->size_cacheline);

	item_hdr = (struct smem_partition_allocation_header
			    *)((uint8_t *)info->header + info->size -
			       padding_header -
			       sizeof(struct smem_partition_allocation_header));

	limit_addr = (uint8_t *)info->header + info->offset_free_cached;

	while (((uint8_t *)item_hdr >= limit_addr) &&
	       (((uint8_t *)item_hdr +
		 sizeof(struct smem_partition_allocation_header)) <=
		((uint8_t *)info->header + info->size))) {
		ret = smem_part_process_item(
			info,
			params,
			&item_hdr,
			padding_header,
			&size_remaining,
			true);

		if (ret == SMEM_STATUS_SUCCESS || ret == SMEM_STATUS_FAILURE) {
			return ret;
		}
	}

	return SMEM_STATUS_NOT_FOUND;
}


void *smem_part_get_addr(enum smem_mem_type smem_type, uint32_t *buf_size)
{
	struct smem_alloc_params params;
	int32_t ret;

	params.remote_host = SMEM_INVALID_HOST;
	params.smem_type = smem_type;
	params.size = 0;
	params.buffer = NULL;
	params.flags = SMEM_ALLOC_FLAG_NONE;

	ret = smem_part_get_addr_ex(&params);
	*buf_size = params.size;
	return (ret == SMEM_STATUS_SUCCESS) ? params.buffer : NULL;
}

void smem_part_init(void)
{
	struct smem_toc *toc;
	uint8_t *smem_base;
	uint32_t smem_size;
	uint32_t num_entries;
	uint16_t this_host;

	uint32_t i, part_indx;
	uint32_t size;
	int32_t status;

	smem_base = smem_info.smem_base_addr;
	smem_size = smem_info.smem_size;
	this_host = smem_info.this_host;

	toc = (struct smem_toc *)(smem_base + smem_size - SMEM_TOC_SIZE);

	if (mmio_read_32((uintptr_t)&toc->identifier) != SMEM_TOC_IDENTIFIER) {
		return;
	}

	if (mmio_read_32((uintptr_t)&toc->version) != 1) {
		ERROR("SMEM: assert failed in %s() at line %d\n",
		      __func__, __LINE__);
		plat_error_handler(EFAULT);
	}
	num_entries = mmio_read_32((uintptr_t)&toc->num_entries);
	if (num_entries > SMEM_TOC_MAX_SMEM_PARTITIONS) {
		ERROR("SMEM: assert failed in %s() at line %d\n",
		      __func__, __LINE__);
		plat_error_handler(EFAULT);
	}

	smem_info_prt = smem_part_info_alloc(&smem_info_num_prt);
	if (!smem_info_prt) {
		ERROR("SMEM: assert failed in %s() at line %d\n",
		      __func__, __LINE__);
		plat_error_handler(EFAULT);
		return;
	}

	for (i = 0, part_indx = 0;
	     i < num_entries && part_indx < smem_info_num_prt; i++) {
		struct smem_toc_entry *entry = &toc->entry[i];
		struct smem_partition_info *prt;
		uint32_t partition_offset;
		uint32_t size_cacheline;
		struct smem_partition_header *hdr;
		uint32_t part_size;
		struct smem_alloc_params params = { 0 };

		if (!smem_part_is_host_present(entry)) {
			continue;
		}

		size = mmio_read_32((uintptr_t)&entry->size);
		partition_offset = mmio_read_32((uintptr_t)&entry->offset);
		size_cacheline = mmio_read_32((uintptr_t)&entry->size_cacheline);

		if (partition_offset == 0) {
			ERROR("SMEM: assert failed in %s() at line %d\n",
			      __func__, __LINE__);
			plat_error_handler(EFAULT);
		}
		if (partition_offset >= smem_size - SMEM_TOC_SIZE) {
			ERROR("SMEM: assert failed in %s() at line %d\n",
			      __func__, __LINE__);
			plat_error_handler(EFAULT);
		}

		hdr = (struct smem_partition_header *)(smem_base +
						     partition_offset);

		if (((unsigned long)hdr & SMEM_PAGE_ALIGN_MASK) != 0) {
			ERROR("SMEM: assert failed in %s() at line %d\n",
			      __func__, __LINE__);
			plat_error_handler(EFAULT);
		}
		if (hdr->identifier != SMEM_PARTITION_HEADER_ID) {
			ERROR("SMEM: assert failed in %s() at line %d\n",
			      __func__, __LINE__);
			plat_error_handler(EFAULT);
		}

		part_size = mmio_read_32((uintptr_t)&hdr->size);
		if (part_size != size) {
			ERROR("SMEM: invalid partition size(%d). Actual "
			      "partition size:%d, part indx: %d\n",
			      part_size, size, i);
			continue;
		}

		prt = &smem_info_prt[part_indx++];
		prt->header = hdr;

		prt->host0 = mmio_read_16((uintptr_t)&hdr->host0);
		prt->host1 = mmio_read_16((uintptr_t)&hdr->host1);

		prt->size = size;
		prt->size_cacheline = size_cacheline;
		prt->offset_free_uncached = sizeof(struct smem_partition_header);
		prt->offset_free_cached = size;

		params.remote_host = this_host;
		params.smem_type = SMEM_MEM_FIRST;

		status = smem_part_get_addr_ex(&params);

		if (status != SMEM_STATUS_SUCCESS &&
		    status != SMEM_STATUS_NOT_FOUND) {
			ERROR("SMEM: partition indx:%d init failed with status 0x%x\n",
			      i, status);
			plat_error_handler(EFAULT);
		}
	}
}
