/*
 * Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>

#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#if HOB_LIST
#include <lib/hob/hob.h>
#include <lib/hob/hob_guid.h>
#include <lib/hob/mmram.h>
#include <lib/hob/mpinfo.h>
#endif
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <services/ffa_svc.h>
#include "spm_common.h"
#include "spm_shim_private.h"
#include "spmc.h"
#if TRANSFER_LIST
#include <transfer_list.h>
#include <tpm_event_log.h>
#endif
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>

#define FFA_BOOT_INFO_SIZE	(PAGE_SIZE * 2)

/*
 * Statically allocate a page of memory for passing boot information to an SP.
 */
static uint8_t ffa_boot_info_mem[FFA_BOOT_INFO_SIZE] __aligned(PAGE_SIZE);

#if HOB_LIST
static void *tpm_evtlog_addr;
static size_t tpm_evtlog_size;
#endif

/*
 * We need to choose one execution context from all those available for a S-EL0
 * SP. This execution context will be used subsequently irrespective of which
 * physical CPU the SP runs on.
 */
#define SEL0_SP_EC_INDEX 0
#define SP_MEM_READ 0x1
#define SP_MEM_WRITE 0x2
#define SP_MEM_EXECUTE 0x4
#define SP_MEM_NON_SECURE 0x8
#define SP_MEM_READ_ONLY SP_MEM_READ
#define SP_MEM_READ_WRITE (SP_MEM_READ | SP_MEM_WRITE)

/* Type of the memory region in SP's manifest. */
enum sp_memory_region_type {
	SP_MEM_REGION_DEVICE,
	SP_MEM_REGION_MEMORY,
	SP_MEM_REGION_NOT_SPECIFIED
};

#if HOB_LIST
#if TRANSFER_LIST
static void get_tpm_event_log(void *secure_tl)
{
	struct transfer_list_entry *te;
	void *evlog;

	if (secure_tl == NULL) {
		tpm_evtlog_addr = NULL;
		tpm_evtlog_size = 0;
		return;
	}

	te = transfer_list_find((struct transfer_list_header *)secure_tl,
				TL_TAG_TPM_EVLOG);
	if (te == NULL) {
		tpm_evtlog_addr = NULL;
		tpm_evtlog_size = 0;
		return;
	}

	evlog = transfer_list_entry_data(te);
	assert(evlog != NULL);

	tpm_evtlog_addr = evlog + EVENT_LOG_RESERVED_BYTES;
	tpm_evtlog_size = te->data_size - EVENT_LOG_RESERVED_BYTES;
}
#else
static void get_tpm_event_log(void *secure_tl)
{
	tpm_evtlog_addr = NULL;
	tpm_evtlog_size = 0;
}
#endif

static int get_memory_region_info(void *sp_manifest, int mem_region_node,
		const char *name, uint32_t granularity,
		uint64_t *base_address, uint32_t *size)
{
	char *property;
	int node, ret;

	if (name != NULL) {
		node = fdt_subnode_offset_namelen(sp_manifest, mem_region_node,
				name, strlen(name));
		if (node < 0) {
			ERROR("Not found '%s' region in memory regions configuration for SP.\n",
					name);
			return -ENOENT;
		}
	} else {
		node = mem_region_node;
	}

	property = "base-address";
	ret = fdt_read_uint64(sp_manifest, node, property, base_address);
	if (ret < 0) {
		ERROR("Not found property(%s) in memory region(%s).\n",
				property, name);
		return -ENOENT;
	}

	property = "pages-count";
	ret = fdt_read_uint32(sp_manifest, node, property, size);
	if (ret < 0) {
		ERROR("Not found property(%s) in memory region(%s).\n",
				property, name);
		return -ENOENT;
	}

	*size = ((*size) << (PAGE_SIZE_SHIFT + (granularity << 1)));

	return 0;
}

static struct efi_hob_handoff_info_table *build_sp_boot_hob_list(
		void *secure_tl, void *sp_manifest,
		uintptr_t hob_table_start, size_t *hob_table_size)
{
	struct efi_hob_handoff_info_table *hob_table;
	uintptr_t base_address;
	int mem_region_node;
	int32_t node, ret;
	const char *name;
	uint32_t granularity, size;
	uint32_t mem_region_num;
	struct efi_guid ns_buf_guid = MM_NS_BUFFER_GUID;
	struct efi_guid mmram_resv_guid = MM_PEI_MMRAM_MEMORY_RESERVE_GUID;
	struct efi_guid tpm_evtlog_guid = MM_TPM_EVENT_LOG_GUID;
	struct efi_mmram_descriptor *mmram_desc_data;
	struct efi_mmram_hob_descriptor_block *mmram_hob_desc_data;

	if (sp_manifest == NULL || hob_table_size == NULL || *hob_table_size == 0) {
		return NULL;
	}

	node = fdt_path_offset(sp_manifest, "/");
	if (node < 0) {
		ERROR("Failed to get root in sp_manifest.\n");
		return NULL;
	}

	ret = fdt_read_uint32(sp_manifest, node, "xlat-granule", &granularity);
	if (ret < 0) {
		ERROR("Not found property(xlat-granule) in sp_manifest.\n");
		return NULL;
	}

	if (granularity > 0x02) {
		ERROR("Invalid granularity value: 0x%x\n", granularity);
		return NULL;
	}

	mem_region_node = fdt_subnode_offset_namelen(sp_manifest, 0, "memory-regions",
			sizeof("memory-regions") - 1);
	if (node < 0) {
		ERROR("Not found memory-region configuration for SP.\n");
		return NULL;
	}

	INFO("Generating PHIT_HOB...\n");

	hob_table = create_hob_list(BL32_BASE, BL32_LIMIT,
			hob_table_start, *hob_table_size);
	if (hob_table == NULL) {
		ERROR("Failed to create Hob Table.\n");
		return NULL;
	}

	/*
	 * Create fv hob.
	 */
	ret = get_memory_region_info(sp_manifest, mem_region_node,
			"stmm_region", granularity, &base_address, &size);
	if (ret < 0) {
		return NULL;
	}

	if (base_address != BL32_BASE &&
			base_address + size > BL32_LIMIT) {
		ERROR("Image is ouf of bound(0x%lx/0x%x), should be in (0x%llx/0x%llx)\n",
				base_address, size, BL32_BASE, BL32_LIMIT - BL32_BASE);
		return NULL;
	}

	ret = create_fv_hob(hob_table, base_address, size);
	if (ret < 0) {
		ERROR("Failed to create fv hob... ret:%d\n", ret);
		return NULL;
	}

	INFO("Success to create FV hob(0x%lx/0x%x).\n", base_address, size);

	/*
	 * Create Ns Buffer hob.
	 */
	ret = get_memory_region_info(sp_manifest, mem_region_node,
			"ns_comm_buffer", granularity, &base_address, &size);
	if (ret < 0) {
		return NULL;
	}

	ret = create_guid_hob(hob_table, &ns_buf_guid,
			sizeof(struct efi_mmram_descriptor), (void **) &mmram_desc_data);
	if (ret < 0) {
		ERROR("Failed to create ns buffer hob\n");
		return NULL;
	}

	mmram_desc_data->physical_start = base_address;
	mmram_desc_data->physical_size = size;
	mmram_desc_data->cpu_start = base_address;
	mmram_desc_data->region_state = EFI_CACHEABLE | EFI_ALLOCATED;

	/*
	 * Create mmram_resv hob.
	 */
	for (node = fdt_first_subnode(sp_manifest, mem_region_node), mem_region_num = 0;
			node >= 0;
			node = fdt_next_subnode(sp_manifest, node), mem_region_num++) {
		ret = get_memory_region_info(sp_manifest, node, NULL, granularity,
				&base_address, &size);
		if (ret < 0) {
			name = fdt_get_name(sp_manifest, node, NULL);
			ERROR("Invalid memory region(%s) found!\n", name);
			return NULL;
		}
	}

	ret = create_guid_hob(hob_table, &mmram_resv_guid,
			(sizeof(struct efi_mmram_hob_descriptor_block) +
			 (sizeof(struct efi_mmram_descriptor) * (mem_region_num))),
			(void **) &mmram_hob_desc_data);
	if (ret < 0) {
		ERROR("Failed to create mmram_resv hob. ret: %d\n", ret);
		return NULL;
	}

	mmram_hob_desc_data->number_of_mm_reserved_regions = mem_region_num;

	for (node = fdt_first_subnode(sp_manifest, mem_region_node), mem_region_num = 0;
			node >= 0;
			node = fdt_next_subnode(sp_manifest, node), mem_region_num++) {
		get_memory_region_info(sp_manifest, node, NULL, granularity,
				&base_address, &size);
		name = fdt_get_name(sp_manifest, node, NULL);

		mmram_desc_data = &mmram_hob_desc_data->descriptor[mem_region_num];
		mmram_desc_data->physical_start = base_address;
		mmram_desc_data->physical_size = size;
		mmram_desc_data->cpu_start = base_address;

		if (!strcmp(name, "heap")) {
			mmram_desc_data->region_state = EFI_CACHEABLE;
		} else {
			mmram_desc_data->region_state = EFI_CACHEABLE | EFI_ALLOCATED;
		}
	}

	/*
	 * Add tpm mmram descriptor.
	 */
	get_tpm_event_log(secure_tl);

	if (tpm_evtlog_addr != NULL && tpm_evtlog_size != 0) {
		ret = create_guid_hob(hob_table, &tpm_evtlog_guid,
				sizeof(struct efi_mmram_descriptor), (void **) &mmram_desc_data);
		if (ret < 0) {
			ERROR("Failed to create tpm_event_log hob\n");
			return NULL;
		}

		mmram_desc_data->physical_start = (uintptr_t)tpm_evtlog_addr;
		mmram_desc_data->physical_size = tpm_evtlog_size;
		mmram_desc_data->cpu_start = (uintptr_t)tpm_evtlog_addr;
		mmram_desc_data->region_state = EFI_CACHEABLE | EFI_ALLOCATED;
	}

	*hob_table_size = hob_table->efi_free_memory_bottom -
		(efi_physical_address_t) hob_table;

  return hob_table;
}
#endif

/*
 * This function creates a initialization descriptor in the memory reserved
 * for passing boot information to an SP. It then copies the partition manifest
 * into this region and ensures that its reference in the initialization
 * descriptor is updated.
 */
static void spmc_create_boot_info(entry_point_info_t *ep_info,
				  struct secure_partition_desc *sp)
{
	struct ffa_boot_info_header *boot_header;
	struct ffa_boot_info_desc *boot_descriptor;
	uintptr_t content_addr;
	void *sp_manifest;
	void *tl __maybe_unused;
#if TRANSFER_LIST && !RESET_TO_BL31
	struct transfer_list_entry *te;

	tl = (void *)((uintptr_t)ep_info->args.arg3);
	te = transfer_list_find((struct transfer_list_header *)tl,
				TL_TAG_DT_FFA_MANIFEST);
	assert(te != NULL);

	sp_manifest = (void *)transfer_list_entry_data(te);
#else
	tl = NULL;
	sp_manifest = (void *)ep_info->args.arg0;
#endif

	/*
	 * Calculate the maximum size of the manifest that can be accommodated
	 * in the boot information memory region.
	 */
	size_t max_sz = sizeof(ffa_boot_info_mem) -
			  (sizeof(struct ffa_boot_info_header) +
			   sizeof(struct ffa_boot_info_desc));

	/*
	 * The current implementation only supports the FF-A v1.1
	 * implementation of the boot protocol, therefore check
	 * that a v1.0 SP has not requested use of the protocol.
	 */
	if (sp->ffa_version == MAKE_FFA_VERSION(1, 0)) {
		ERROR("FF-A boot protocol not supported for v1.0 clients\n");
		return;
	}

	/* Zero the memory region before populating. */
	memset(ffa_boot_info_mem, 0, FFA_BOOT_INFO_SIZE);

	/*
	 * Populate the ffa_boot_info_header at the start of the boot info
	 * region.
	 */
	boot_header = (struct ffa_boot_info_header *) ffa_boot_info_mem;

	/* Position the ffa_boot_info_desc after the ffa_boot_info_header. */
	boot_header->offset_boot_info_desc =
					sizeof(struct ffa_boot_info_header);
	boot_descriptor = (struct ffa_boot_info_desc *)
			  (ffa_boot_info_mem +
			   boot_header->offset_boot_info_desc);

	/*
	 * We must use the FF-A version corresponding to the version implemented
	 * by the SP. Currently this can only be v1.1.
	 */
	boot_header->version = sp->ffa_version;

	/* Populate the boot information header. */
	boot_header->size_boot_info_desc = sizeof(struct ffa_boot_info_desc);

	/* Set the signature "0xFFA". */
	boot_header->signature = FFA_INIT_DESC_SIGNATURE;

	/* Set the count. Currently 1 since only the manifest is specified. */
	boot_header->count_boot_info_desc = 1;

	boot_descriptor->flags =
		FFA_BOOT_INFO_FLAG_NAME(FFA_BOOT_INFO_FLAG_NAME_UUID) |
		FFA_BOOT_INFO_FLAG_CONTENT(FFA_BOOT_INFO_FLAG_CONTENT_ADR);

	content_addr = (uintptr_t) (ffa_boot_info_mem +
				     boot_header->offset_boot_info_desc +
				     boot_header->size_boot_info_desc);

#if HOB_LIST
	/* Populate the boot information descriptor for the hob_list. */
	boot_descriptor->type =
		FFA_BOOT_INFO_TYPE(FFA_BOOT_INFO_TYPE_STD) |
		FFA_BOOT_INFO_TYPE_ID(FFA_BOOT_INFO_TYPE_ID_HOB);

	content_addr = (uintptr_t) build_sp_boot_hob_list(
			tl, sp_manifest, content_addr, &max_sz);
	if (content_addr == (uintptr_t) NULL) {
		WARN("Unable to create phit hob properly.");
		return;
	}

	boot_descriptor->size_boot_info = max_sz;
	boot_descriptor->content = content_addr;
#else
	/*
	 * Check if the manifest will fit into the boot info memory region else
	 * bail.
	 */
	if (ep_info->args.arg1 > max_sz) {
		WARN("Unable to copy manifest into boot information. ");
		WARN("Max sz = %lu bytes. Manifest sz = %lu bytes\n",
		     max_sz, ep_info->args.arg1);
		return;
	}

	/* Populate the boot information descriptor for the manifest. */
	boot_descriptor->type =
		FFA_BOOT_INFO_TYPE(FFA_BOOT_INFO_TYPE_STD) |
		FFA_BOOT_INFO_TYPE_ID(FFA_BOOT_INFO_TYPE_ID_FDT);

	/*
	 * Copy the manifest into boot info region after the boot information
	 * descriptor.
	 */
	boot_descriptor->size_boot_info = (uint32_t) ep_info->args.arg1;

	memcpy((void *) content_addr, sp_manifest, boot_descriptor->size_boot_info);

	boot_descriptor->content = content_addr;
#endif

	/* Calculate the size of the total boot info blob. */
	boot_header->size_boot_info_blob = boot_header->offset_boot_info_desc +
					   boot_descriptor->size_boot_info +
					   (boot_header->count_boot_info_desc *
					    boot_header->size_boot_info_desc);

	INFO("SP boot info @ 0x%lx, size: %u bytes.\n",
	     (uintptr_t) ffa_boot_info_mem,
	     boot_header->size_boot_info_blob);
	INFO("SP content @ 0x%lx, size: %u bytes.\n",
	     boot_descriptor->content,
	     boot_descriptor->size_boot_info);
}

/*
 * S-EL1 partitions can be assigned with multiple execution contexts, each
 * pinned to the physical CPU. Each execution context index corresponds to the
 * respective liner core position.
 * S-EL0 partitions execute in a single execution context (index 0).
 */
unsigned int get_ec_index(struct secure_partition_desc *sp)
{
	return (sp->runtime_el == S_EL0) ?
		SEL0_SP_EC_INDEX : plat_my_core_pos();
}

#if SPMC_AT_EL3_SEL0_SP
/* Setup spsr in entry point info for common context management code to use. */
void spmc_el0_sp_spsr_setup(entry_point_info_t *ep_info)
{
	/* Setup Secure Partition SPSR for S-EL0 SP. */
	ep_info->spsr = SPSR_64(MODE_EL0, MODE_SP_EL0, DISABLE_ALL_EXCEPTIONS);
}

static void read_optional_string(void *manifest, int32_t offset,
				 char *property, char *out, size_t len)
{
	const fdt32_t *prop;
	int lenp;

	prop = fdt_getprop(manifest, offset, property, &lenp);
	if (prop == NULL) {
		out[0] = '\0';
	} else {
		memcpy(out, prop, MIN(lenp, (int)len));
		out[MIN(lenp, (int)len) - 1] = '\0';
	}
}

/*******************************************************************************
 * This function will parse the Secure Partition Manifest for fetching secure
 * partition specific memory/device region details. It will find base address,
 * size, memory attributes for each region and then add the respective region
 * into secure parition's translation context.
 ******************************************************************************/
static void populate_sp_regions(struct secure_partition_desc *sp,
				void *sp_manifest, int node,
				enum sp_memory_region_type type)
{
	uintptr_t base_address;
	uint32_t mem_attr, mem_region, size;
	struct mmap_region sp_mem_regions = {0};
	int32_t offset, ret;
	char *compatibility[SP_MEM_REGION_NOT_SPECIFIED] = {
		"arm,ffa-manifest-device-regions",
		"arm,ffa-manifest-memory-regions"
	};
	char description[10];
	char *property;
	char *region[SP_MEM_REGION_NOT_SPECIFIED] = {
		"device regions",
		"memory regions"
	};

	if (type >= SP_MEM_REGION_NOT_SPECIFIED) {
		WARN("Invalid region type\n");
		return;
	}

	INFO("Mapping SP's %s\n", region[type]);

	if (fdt_node_check_compatible(sp_manifest, node,
				      compatibility[type]) != 0) {
		WARN("Incompatible region node in manifest\n");
		return;
	}

	for (offset = fdt_first_subnode(sp_manifest, node), mem_region = 0;
	     offset >= 0;
	     offset = fdt_next_subnode(sp_manifest, offset), mem_region++) {
		read_optional_string(sp_manifest, offset, "description",
				     description, sizeof(description));

		INFO("Mapping: region: %d, %s\n", mem_region, description);

		property = "base-address";
		ret = fdt_read_uint64(sp_manifest, offset, property,
					&base_address);
		if (ret < 0) {
			WARN("Missing:%s for %s.\n", property, description);
			continue;
		}

		property = "pages-count";
		ret = fdt_read_uint32(sp_manifest, offset, property, &size);
		if (ret < 0) {
			WARN("Missing: %s for %s.\n", property, description);
			continue;
		}
		size *= PAGE_SIZE;

		property = "attributes";
		ret = fdt_read_uint32(sp_manifest, offset, property, &mem_attr);
		if (ret < 0) {
			WARN("Missing: %s for %s.\n", property, description);
			continue;
		}

		sp_mem_regions.attr = MT_USER;
		if (type == SP_MEM_REGION_DEVICE) {
			sp_mem_regions.attr |= MT_EXECUTE_NEVER;
		} else {
			sp_mem_regions.attr |= MT_MEMORY;
			if ((mem_attr & SP_MEM_EXECUTE) == SP_MEM_EXECUTE) {
				sp_mem_regions.attr &= ~MT_EXECUTE_NEVER;
			} else {
				sp_mem_regions.attr |= MT_EXECUTE_NEVER;
			}
		}

		if ((mem_attr & SP_MEM_READ_WRITE) == SP_MEM_READ_WRITE) {
			sp_mem_regions.attr |= MT_RW;
		}

		if ((mem_attr & SP_MEM_NON_SECURE) == SP_MEM_NON_SECURE) {
			sp_mem_regions.attr |= MT_NS;
		} else {
			sp_mem_regions.attr |= MT_SECURE;
		}

		sp_mem_regions.base_pa = base_address;
		sp_mem_regions.base_va = base_address;
		sp_mem_regions.size = size;

		INFO("Adding PA: 0x%llx VA: 0x%lx Size: 0x%lx mem_attr: 0x%x, attr:0x%x\n",
		     sp_mem_regions.base_pa,
		     sp_mem_regions.base_va,
		     sp_mem_regions.size,
		     mem_attr,
		     sp_mem_regions.attr);

		if (type == SP_MEM_REGION_DEVICE) {
			sp_mem_regions.granularity = XLAT_BLOCK_SIZE(1);
		} else {
			sp_mem_regions.granularity = XLAT_BLOCK_SIZE(3);
		}
		mmap_add_region_ctx(sp->xlat_ctx_handle, &sp_mem_regions);
	}
}

static void spmc_el0_sp_setup_mmu(struct secure_partition_desc *sp,
				  cpu_context_t *ctx)
{
	xlat_ctx_t *xlat_ctx;
	uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

	xlat_ctx = sp->xlat_ctx_handle;
	init_xlat_tables_ctx(sp->xlat_ctx_handle);
	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, 0, xlat_ctx->base_table,
		      xlat_ctx->pa_max_address, xlat_ctx->va_max_address,
		      EL1_EL0_REGIME);

	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), mair_el1,
		      mmu_cfg_params[MMU_CFG_MAIR]);

	write_ctx_tcr_el1_reg_errata(ctx, mmu_cfg_params[MMU_CFG_TCR]);

	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), ttbr0_el1,
		      mmu_cfg_params[MMU_CFG_TTBR0]);
}

static void spmc_el0_sp_setup_sctlr_el1(cpu_context_t *ctx)
{
	u_register_t sctlr_el1_val;

	/* Setup SCTLR_EL1 */
	sctlr_el1_val = read_ctx_sctlr_el1_reg_errata(ctx);

	sctlr_el1_val |=
		/*SCTLR_EL1_RES1 |*/
		/* Don't trap DC CVAU, DC CIVAC, DC CVAC, DC CVAP, or IC IVAU */
		SCTLR_UCI_BIT |
		/* RW regions at xlat regime EL1&0 are forced to be XN. */
		SCTLR_WXN_BIT |
		/* Don't trap to EL1 execution of WFI or WFE at EL0. */
		SCTLR_NTWI_BIT | SCTLR_NTWE_BIT |
		/* Don't trap to EL1 accesses to CTR_EL0 from EL0. */
		SCTLR_UCT_BIT |
		/* Don't trap to EL1 execution of DZ ZVA at EL0. */
		SCTLR_DZE_BIT |
		/* Enable SP Alignment check for EL0 */
		SCTLR_SA0_BIT |
		/* Don't change PSTATE.PAN on taking an exception to EL1 */
		SCTLR_SPAN_BIT |
		/* Allow cacheable data and instr. accesses to normal memory. */
		SCTLR_C_BIT | SCTLR_I_BIT |
		/* Enable MMU. */
		SCTLR_M_BIT;

	sctlr_el1_val &= ~(
		/* Explicit data accesses at EL0 are little-endian. */
		SCTLR_E0E_BIT |
		/*
		 * Alignment fault checking disabled when at EL1 and EL0 as
		 * the UEFI spec permits unaligned accesses.
		 */
		SCTLR_A_BIT |
		/* Accesses to DAIF from EL0 are trapped to EL1. */
		SCTLR_UMA_BIT
	);

	/* Store the initialised SCTLR_EL1 value in the cpu_context */
	write_ctx_sctlr_el1_reg_errata(ctx, sctlr_el1_val);
}

static void spmc_el0_sp_setup_system_registers(struct secure_partition_desc *sp,
					       cpu_context_t *ctx)
{

	spmc_el0_sp_setup_mmu(sp, ctx);

	spmc_el0_sp_setup_sctlr_el1(ctx);

	/* Setup other system registers. */

	/* Shim Exception Vector Base Address */
	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), vbar_el1,
			SPM_SHIM_EXCEPTIONS_PTR);
	write_el1_ctx_arch_timer(get_el1_sysregs_ctx(ctx), cntkctl_el1,
		      EL0PTEN_BIT | EL0VTEN_BIT | EL0PCTEN_BIT | EL0VCTEN_BIT);

	/*
	 * FPEN: Allow the Secure Partition to access FP/SIMD registers.
	 * Note that SPM will not do any saving/restoring of these registers on
	 * behalf of the SP. This falls under the SP's responsibility.
	 * TTA: Enable access to trace registers.
	 * ZEN (v8.2): Trap SVE instructions and access to SVE registers.
	 */
	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), cpacr_el1,
			CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_NONE));
}

/* Setup context of an EL0 Secure Partition.  */
void spmc_el0_sp_setup(struct secure_partition_desc *sp,
		       int32_t boot_info_reg,
		       void *sp_manifest)
{
	mmap_region_t sel1_exception_vectors =
		MAP_REGION_FLAT(SPM_SHIM_EXCEPTIONS_START,
				SPM_SHIM_EXCEPTIONS_SIZE,
				MT_CODE | MT_SECURE | MT_PRIVILEGED);
	cpu_context_t *ctx;
	int node;
	int offset = 0;
	struct mmap_region sp_mem_regions __unused = {0};

	ctx = &sp->ec[SEL0_SP_EC_INDEX].cpu_ctx;

	sp->xlat_ctx_handle->xlat_regime = EL1_EL0_REGIME;

	/* This region contains the exception vectors used at S-EL1. */
	mmap_add_region_ctx(sp->xlat_ctx_handle,
			    &sel1_exception_vectors);

	/*
	 * If the SP manifest specified the register to pass the address of the
	 * boot information, then map the memory region to pass boot
	 * information.
	 */
	if (boot_info_reg >= 0) {
		mmap_region_t ffa_boot_info_region = MAP_REGION_FLAT(
			(uintptr_t) ffa_boot_info_mem,
			FFA_BOOT_INFO_SIZE,
			MT_RO_DATA | MT_SECURE | MT_USER);
		mmap_add_region_ctx(sp->xlat_ctx_handle, &ffa_boot_info_region);
	}

	/*
	 * Parse the manifest for any device regions that the SP wants to be
	 * mapped in its translation regime.
	 */
	node = fdt_subnode_offset_namelen(sp_manifest, offset,
					  "device-regions",
					  sizeof("device-regions") - 1);
	if (node < 0) {
		WARN("Not found device-region configuration for SP.\n");
	} else {
		populate_sp_regions(sp, sp_manifest, node,
				    SP_MEM_REGION_DEVICE);
	}

	/*
	 * Parse the manifest for any memory regions that the SP wants to be
	 * mapped in its translation regime.
	 */
	node = fdt_subnode_offset_namelen(sp_manifest, offset,
					  "memory-regions",
					  sizeof("memory-regions") - 1);
	if (node < 0) {
		WARN("Not found memory-region configuration for SP.\n");
	} else {
		populate_sp_regions(sp, sp_manifest, node,
				    SP_MEM_REGION_MEMORY);
	}

#if HOB_LIST
	/*
	 * Add tpm event log region with RO permission.
	 */
	if (tpm_evtlog_addr != NULL && tpm_evtlog_size != 0) {
		INFO("Mapping SP's TPM event log\n");
		INFO("TPM event log addr(0x%lx), size(0x%lx)\n",
				(uintptr_t)tpm_evtlog_addr, tpm_evtlog_size);
		sp_mem_regions.base_pa = (uintptr_t)
			((unsigned long)tpm_evtlog_addr & ~(PAGE_SIZE_MASK));
		sp_mem_regions.base_va = sp_mem_regions.base_pa;
		sp_mem_regions.size = (tpm_evtlog_size & ~(PAGE_SIZE_MASK)) + PAGE_SIZE;
		sp_mem_regions.attr = MT_USER | MT_SECURE | MT_RO_DATA;
		sp_mem_regions.granularity = XLAT_BLOCK_SIZE(3);

		INFO("Adding PA: 0x%llx VA: 0x%lx Size: 0x%lx attr:0x%x\n",
		     sp_mem_regions.base_pa,
		     sp_mem_regions.base_va,
		     sp_mem_regions.size,
		     sp_mem_regions.attr);

		mmap_add_region_ctx(sp->xlat_ctx_handle, &sp_mem_regions);
	}
#endif

	spmc_el0_sp_setup_system_registers(sp, ctx);
}
#endif /* SPMC_AT_EL3_SEL0_SP */

/* S-EL1 partition specific initialisation. */
void spmc_el1_sp_setup(struct secure_partition_desc *sp,
		       entry_point_info_t *ep_info)
{
	/* Sanity check input arguments. */
	assert(sp != NULL);
	assert(ep_info != NULL);

	/* Initialise the SPSR for S-EL1 SPs. */
	ep_info->spsr =	SPSR_64(MODE_EL1, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);

	/*
	 * TF-A Implementation defined behaviour to provide the linear
	 * core ID in the x4 register.
	 */
	ep_info->args.arg4 = (uintptr_t) plat_my_core_pos();

	/*
	 * Check whether setup is being performed for the primary or a secondary
	 * execution context. In the latter case, indicate to the SP that this
	 * is a warm boot.
	 * TODO: This check would need to be reworked if the same entry point is
	 * used for both primary and secondary initialisation.
	 */
	if (sp->secondary_ep != 0U) {
		/*
		 * Sanity check that the secondary entry point is still what was
		 * originally set.
		 */
		assert(sp->secondary_ep == ep_info->pc);
		ep_info->args.arg0 = FFA_WB_TYPE_S2RAM;
	}
}

/* Common initialisation for all SPs. */
void spmc_sp_common_setup(struct secure_partition_desc *sp,
			  entry_point_info_t *ep_info,
			  int32_t boot_info_reg)
{
	uint16_t sp_id;

	/* Assign FF-A Partition ID if not already assigned. */
	if (sp->sp_id == INV_SP_ID) {
		sp_id = FFA_SP_ID_BASE + ACTIVE_SP_DESC_INDEX;
		/*
		 * Ensure we don't clash with previously assigned partition
		 * IDs.
		 */
		while (!is_ffa_secure_id_valid(sp_id)) {
			sp_id++;

			if (sp_id == FFA_SWD_ID_LIMIT) {
				ERROR("Unable to determine valid SP ID.\n");
				panic();
			}
		}
		sp->sp_id = sp_id;
	}

	/* Check if the SP wants to use the FF-A boot protocol. */
	if (boot_info_reg >= 0) {
		/*
		 * Create a boot information descriptor and copy the partition
		 * manifest into the reserved memory region for consumption by
		 * the SP.
		 */
		spmc_create_boot_info(ep_info, sp);

		/*
		 * We have consumed what we need from ep args so we can now
		 * zero them before we start populating with new information
		 * specifically for the SP.
		 */
		zeromem(&ep_info->args, sizeof(ep_info->args));

		/*
		 * Pass the address of the boot information in the
		 * boot_info_reg.
		 */
		switch (boot_info_reg) {
		case 0:
			ep_info->args.arg0 = (uintptr_t) ffa_boot_info_mem;
			break;
		case 1:
			ep_info->args.arg1 = (uintptr_t) ffa_boot_info_mem;
			break;
		case 2:
			ep_info->args.arg2 = (uintptr_t) ffa_boot_info_mem;
			break;
		case 3:
			ep_info->args.arg3 = (uintptr_t) ffa_boot_info_mem;
			break;
		default:
			ERROR("Invalid value for \"gp-register-num\" %d.\n",
			      boot_info_reg);
		}
	} else {
		/*
		 * We don't need any of the information that was populated
		 * in ep_args so we can clear them.
		 */
		zeromem(&ep_info->args, sizeof(ep_info->args));
	}
}

/*
 * Initialise the SP context now we have populated the common and EL specific
 * entrypoint information.
 */
void spmc_sp_common_ep_commit(struct secure_partition_desc *sp,
			      entry_point_info_t *ep_info)
{
	cpu_context_t *cpu_ctx;

	cpu_ctx = &(spmc_get_sp_ec(sp)->cpu_ctx);
	print_entry_point_info(ep_info);
	cm_setup_context(cpu_ctx, ep_info);
}
