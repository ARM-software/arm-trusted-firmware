/*
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/console.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <libfdt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>
#include <plat/common/platform.h>

#include "avs_driver.h"
#include "board.h"
#include "boot_init_dram.h"
#include "cpg_registers.h"
#include "emmc_def.h"
#include "emmc_hal.h"
#include "emmc_std.h"
#include "io_common.h"
#include "io_rcar.h"
#include "qos_init.h"
#include "rcar_def.h"
#include "rcar_private.h"
#include "rcar_version.h"
#include "rom_api.h"

#define MAX_DRAM_CHANNELS 4
/*
 * DDR ch0 has a shadow area mapped in 32bit address space.
 * Physical address 0x4_0000_0000 - 0x4_7fff_ffff in 64bit space
 * is mapped to 0x4000_0000 - 0xbfff_ffff in 32bit space.
 */
#define MAX_DRAM_SIZE_CH0_32BIT_ADDR_SPACE 0x80000000ULL

#if RCAR_BL2_DCACHE == 1
/*
 * Following symbols are only used during plat_arch_setup() only
 * when RCAR_BL2_DCACHE is enabled.
 */
static const uint64_t BL2_RO_BASE		= BL_CODE_BASE;
static const uint64_t BL2_RO_LIMIT		= BL_CODE_END;

#if USE_COHERENT_MEM
static const uint64_t BL2_COHERENT_RAM_BASE	= BL_COHERENT_RAM_BASE;
static const uint64_t BL2_COHERENT_RAM_LIMIT	= BL_COHERENT_RAM_END;
#endif /* USE_COHERENT_MEM */

#endif /* RCAR_BL2_DCACHE */

extern void plat_rcar_gic_driver_init(void);
extern void plat_rcar_gic_init(void);
extern void bl2_enter_bl31(const struct entry_point_info *bl_ep_info);
extern void bl2_system_cpg_init(void);
extern void bl2_secure_setting(void);
extern void bl2_cpg_init(void);
extern void rcar_io_emmc_setup(void);
extern void rcar_io_setup(void);
extern void rcar_swdt_release(void);
extern void rcar_swdt_init(void);
extern void rcar_rpc_init(void);
extern void rcar_dma_init(void);
extern void rzg_pfc_init(void);

static void bl2_init_generic_timer(void);

/* RZ/G2 product check */
#if RCAR_LSI == RZ_G2M
#define TARGET_PRODUCT			PRR_PRODUCT_M3
#define TARGET_NAME			"RZ/G2M"
#elif RCAR_LSI == RZ_G2H
#define TARGET_PRODUCT			PRR_PRODUCT_H3
#define TARGET_NAME			"RZ/G2H"
#elif RCAR_LSI == RZ_G2N
#define TARGET_PRODUCT			PRR_PRODUCT_M3N
#define TARGET_NAME			"RZ/G2N"
#elif RCAR_LSI == RZ_G2E
#define TARGET_PRODUCT			PRR_PRODUCT_E3
#define TARGET_NAME			"RZ/G2E"
#elif RCAR_LSI == RCAR_AUTO
#define TARGET_NAME			"RZ/G2M"
#endif /* RCAR_LSI == RZ_G2M */

#if (RCAR_LSI == RZ_G2E)
#define GPIO_INDT			(GPIO_INDT6)
#define GPIO_BKUP_TRG_SHIFT		((uint32_t)1U << 13U)
#else
#define GPIO_INDT			(GPIO_INDT1)
#define GPIO_BKUP_TRG_SHIFT		(1U << 8U)
#endif /* RCAR_LSI == RZ_G2E */

CASSERT((PARAMS_BASE + sizeof(bl2_to_bl31_params_mem_t) + 0x100)
	 < (RCAR_SHARED_MEM_BASE + RCAR_SHARED_MEM_SIZE),
	assert_bl31_params_do_not_fit_in_shared_memory);

static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/* FDT with DRAM configuration */
uint64_t fdt_blob[PAGE_SIZE_4KB / sizeof(uint64_t)];
static void *fdt = (void *)fdt_blob;

static void unsigned_num_print(uint64_t unum, unsigned int radix, char *string)
{
	/* Just need enough space to store 64 bit decimal integer */
	char num_buf[20];
	int i = 0;
	unsigned int rem;

	do {
		rem = unum % radix;
		if (rem < 0xaU) {
			num_buf[i] = '0' + rem;
		} else {
			num_buf[i] = 'a' + (rem - 0xaU);
		}
		i++;
		unum /= radix;
	} while (unum > 0U);

	while (--i >= 0) {
		*string++ = num_buf[i];
	}
	*string = 0;
}

#if RCAR_LOSSY_ENABLE == 1
typedef struct bl2_lossy_info {
	uint32_t magic;
	uint32_t a0;
	uint32_t b0;
} bl2_lossy_info_t;

static void bl2_lossy_gen_fdt(uint32_t no, uint64_t start_addr,
			      uint64_t end_addr, uint32_t format,
			      uint32_t enable, int fcnlnode)
{
	const uint64_t fcnlsize = cpu_to_fdt64(end_addr - start_addr);
	char nodename[40] = { 0 };
	int ret, node;

	/* Ignore undefined addresses */
	if (start_addr == 0UL && end_addr == 0UL) {
		return;
	}

	snprintf(nodename, sizeof(nodename), "lossy-decompression@");
	unsigned_num_print(start_addr, 16, nodename + strlen(nodename));

	node = ret = fdt_add_subnode(fdt, fcnlnode, nodename);
	if (ret < 0) {
		NOTICE("BL2: Cannot create FCNL node (ret=%i)\n", ret);
		panic();
	}

	ret = fdt_setprop_string(fdt, node, "compatible",
				 "renesas,lossy-decompression");
	if (ret < 0) {
		NOTICE("BL2: Cannot add FCNL compat string %s (ret=%i)\n",
		       "renesas,lossy-decompression", ret);
		panic();
	}

	ret = fdt_appendprop_string(fdt, node, "compatible",
				    "shared-dma-pool");
	if (ret < 0) {
		NOTICE("BL2: Cannot append FCNL compat string %s (ret=%i)\n",
		       "shared-dma-pool", ret);
		panic();
	}

	ret = fdt_setprop_u64(fdt, node, "reg", start_addr);
	if (ret < 0) {
		NOTICE("BL2: Cannot add FCNL reg prop (ret=%i)\n", ret);
		panic();
	}

	ret = fdt_appendprop(fdt, node, "reg", &fcnlsize, sizeof(fcnlsize));
	if (ret < 0) {
		NOTICE("BL2: Cannot append FCNL reg size prop (ret=%i)\n", ret);
		panic();
	}

	ret = fdt_setprop(fdt, node, "no-map", NULL, 0);
	if (ret < 0) {
		NOTICE("BL2: Cannot add FCNL no-map prop (ret=%i)\n", ret);
		panic();
	}

	ret = fdt_setprop_u32(fdt, node, "renesas,formats", format);
	if (ret < 0) {
		NOTICE("BL2: Cannot add FCNL formats prop (ret=%i)\n", ret);
		panic();
	}
}

static void bl2_lossy_setting(uint32_t no, uint64_t start_addr,
			      uint64_t end_addr, uint32_t format,
			      uint32_t enable, int fcnlnode)
{
	bl2_lossy_info_t info;
	uint32_t reg;

	bl2_lossy_gen_fdt(no, start_addr, end_addr, format, enable, fcnlnode);

	reg = format | (start_addr >> 20);
	mmio_write_32(AXI_DCMPAREACRA0 + 0x8U * no, reg);
	mmio_write_32(AXI_DCMPAREACRB0 + 0x8U * no, end_addr >> 20);
	mmio_write_32(AXI_DCMPAREACRA0 + 0x8U * no, reg | enable);

	info.magic = 0x12345678U;
	info.a0 = mmio_read_32(AXI_DCMPAREACRA0 + 0x8U * no);
	info.b0 = mmio_read_32(AXI_DCMPAREACRB0 + 0x8U * no);

	mmio_write_32(LOSSY_PARAMS_BASE + sizeof(info) * no, info.magic);
	mmio_write_32(LOSSY_PARAMS_BASE + sizeof(info) * no + 0x4U, info.a0);
	mmio_write_32(LOSSY_PARAMS_BASE + sizeof(info) * no + 0x8U, info.b0);

	NOTICE("     Entry %d: DCMPAREACRAx:0x%x DCMPAREACRBx:0x%x\n", no,
	       mmio_read_32(AXI_DCMPAREACRA0 + 0x8U * no),
	       mmio_read_32(AXI_DCMPAREACRB0 + 0x8U * no));
}
#endif /* RCAR_LOSSY_ENABLE == 1 */

void bl2_plat_flush_bl31_params(void)
{
	uint32_t product_cut, product, cut;
	uint32_t boot_dev, boot_cpu;
	uint32_t reg;

	reg = mmio_read_32(RCAR_MODEMR);
	boot_dev = reg & MODEMR_BOOT_DEV_MASK;

	if (boot_dev == MODEMR_BOOT_DEV_EMMC_25X1 ||
	    boot_dev == MODEMR_BOOT_DEV_EMMC_50X8) {
		emmc_terminate();
	}

	if ((reg & MODEMR_BOOT_CPU_MASK) != MODEMR_BOOT_CPU_CR7) {
		bl2_secure_setting();
	}

	reg = mmio_read_32(RCAR_PRR);
	product_cut = reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK);
	product = reg & PRR_PRODUCT_MASK;
	cut = reg & PRR_CUT_MASK;

	if (!((product == PRR_PRODUCT_M3 && cut < PRR_PRODUCT_30) ||
	      (product == PRR_PRODUCT_H3 && cut < PRR_PRODUCT_20))) {
		/* Disable MFIS write protection */
		mmio_write_32(MFISWPCNTR, MFISWPCNTR_PASSWORD | 1U);
	}

	reg = mmio_read_32(RCAR_MODEMR);
	boot_cpu = reg & MODEMR_BOOT_CPU_MASK;
	if (boot_cpu == MODEMR_BOOT_CPU_CA57 ||
	    boot_cpu == MODEMR_BOOT_CPU_CA53) {
		if (product_cut == PRR_PRODUCT_H3_CUT20) {
			mmio_write_32(IPMMUVI0_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUVI1_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUPV0_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUPV1_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUPV2_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUPV3_IMSCTLR, IMSCTLR_DISCACHE);
		} else if (product_cut == (PRR_PRODUCT_M3N | PRR_PRODUCT_10) ||
			   product_cut == (PRR_PRODUCT_M3N | PRR_PRODUCT_11)) {
			mmio_write_32(IPMMUVI0_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUPV0_IMSCTLR, IMSCTLR_DISCACHE);
		} else if ((product_cut == (PRR_PRODUCT_E3 | PRR_PRODUCT_10)) ||
			   (product_cut == (PRR_PRODUCT_E3 | PRR_PRODUCT_11))) {
			mmio_write_32(IPMMUVI0_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUVP0_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUPV0_IMSCTLR, IMSCTLR_DISCACHE);
		}

		if (product_cut == (PRR_PRODUCT_H3_CUT20) ||
		    product_cut == (PRR_PRODUCT_M3N | PRR_PRODUCT_10) ||
		    product_cut == (PRR_PRODUCT_M3N | PRR_PRODUCT_11) ||
		    product_cut == (PRR_PRODUCT_E3 | PRR_PRODUCT_10)) {
			mmio_write_32(IPMMUHC_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMURT_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUMP_IMSCTLR, IMSCTLR_DISCACHE);

			mmio_write_32(IPMMUDS0_IMSCTLR, IMSCTLR_DISCACHE);
			mmio_write_32(IPMMUDS1_IMSCTLR, IMSCTLR_DISCACHE);
		}
	}

	mmio_write_32(IPMMUMM_IMSCTLR, IPMMUMM_IMSCTLR_ENABLE);
	mmio_write_32(IPMMUMM_IMAUXCTLR, IPMMUMM_IMAUXCTLR_NMERGE40_BIT);

	rcar_swdt_release();
	bl2_system_cpg_init();

#if RCAR_BL2_DCACHE == 1
	/* Disable data cache (clean and invalidate) */
	disable_mmu_el3();
#endif /* RCAR_BL2_DCACHE == 1 */
}

static uint32_t is_ddr_backup_mode(void)
{
#if RCAR_SYSTEM_SUSPEND
	static uint32_t reason = RCAR_COLD_BOOT;
	static uint32_t once;

	if (once != 0U) {
		return reason;
	}

	once = 1;
	if ((mmio_read_32(GPIO_INDT) & GPIO_BKUP_TRG_SHIFT) == 0U) {
		return reason;
	}

	reason = RCAR_WARM_BOOT;
	return reason;
#else /* RCAR_SYSTEM_SUSPEND */
	return RCAR_COLD_BOOT;
#endif /* RCAR_SYSTEM_SUSPEND */
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	u_register_t *boot_kind = (void *)BOOT_KIND_BASE;
	bl_mem_params_node_t *bl_mem_params;

	if (image_id != BL31_IMAGE_ID) {
		return 0;
	}

	bl_mem_params = get_bl_mem_params_node(image_id);

	if (is_ddr_backup_mode() != RCAR_COLD_BOOT) {
		*boot_kind  = RCAR_WARM_BOOT;
		flush_dcache_range(BOOT_KIND_BASE, sizeof(*boot_kind));

		console_flush();
		bl2_plat_flush_bl31_params();

		/* will not return */
		bl2_enter_bl31(&bl_mem_params->ep_info);
	}

	*boot_kind  = RCAR_COLD_BOOT;
	flush_dcache_range(BOOT_KIND_BASE, sizeof(*boot_kind));

	return 0;
}

static uint64_t rzg_get_dest_addr_from_cert(uint32_t certid, uintptr_t *dest)
{
	uint32_t cert, len;
	int err;

	err = rcar_get_certificate(certid, &cert);
	if (err != 0) {
		ERROR("%s : cert file load error", __func__);
		return 1U;
	}

	rcar_read_certificate((uint64_t)cert, &len, dest);

	return 0U;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	static bl2_to_bl31_params_mem_t *params;
	bl_mem_params_node_t *bl_mem_params;
	uintptr_t dest;
	uint64_t ret;

	if (params == NULL) {
		params = (bl2_to_bl31_params_mem_t *)PARAMS_BASE;
		memset((void *)PARAMS_BASE, 0, sizeof(*params));
	}

	bl_mem_params = get_bl_mem_params_node(image_id);

	switch (image_id) {
	case BL31_IMAGE_ID:
		ret = rzg_get_dest_addr_from_cert(SOC_FW_CONTENT_CERT_ID,
						  &dest);
		if (ret == 0U) {
			bl_mem_params->image_info.image_base = dest;
		}
		break;
	case BL32_IMAGE_ID:
		ret = rzg_get_dest_addr_from_cert(TRUSTED_OS_FW_CONTENT_CERT_ID,
						  &dest);
		if (ret == 0U) {
			bl_mem_params->image_info.image_base = dest;
		}

		memcpy(&params->bl32_ep_info, &bl_mem_params->ep_info,
		       sizeof(entry_point_info_t));
		break;
	case BL33_IMAGE_ID:
		memcpy(&params->bl33_ep_info, &bl_mem_params->ep_info,
		       sizeof(entry_point_info_t));
		break;
	default:
		break;
	}

	return 0;
}

struct meminfo *bl2_plat_sec_mem_layout(void)
{
	return &bl2_tzram_layout;
}

static void bl2_populate_compatible_string(void *dt)
{
	uint32_t board_type;
	uint32_t board_rev;
	uint32_t reg;
	int ret;

	fdt_setprop_u32(dt, 0, "#address-cells", 2);
	fdt_setprop_u32(dt, 0, "#size-cells", 2);

	/* Populate compatible string */
	rzg_get_board_type(&board_type, &board_rev);
	switch (board_type) {
	case BOARD_HIHOPE_RZ_G2M:
		ret = fdt_setprop_string(dt, 0, "compatible",
					 "hoperun,hihope-rzg2m");
		break;
	case BOARD_HIHOPE_RZ_G2H:
		ret = fdt_setprop_string(dt, 0, "compatible",
					 "hoperun,hihope-rzg2h");
		break;
	case BOARD_HIHOPE_RZ_G2N:
		ret = fdt_setprop_string(dt, 0, "compatible",
					 "hoperun,hihope-rzg2n");
		break;
	case BOARD_EK874_RZ_G2E:
		ret = fdt_setprop_string(dt, 0, "compatible",
					 "si-linux,cat874");
		break;
	default:
		NOTICE("BL2: Cannot set compatible string, board unsupported\n");
		panic();
		break;
	}

	if (ret < 0) {
		NOTICE("BL2: Cannot set compatible string (ret=%i)\n", ret);
		panic();
	}

	reg = mmio_read_32(RCAR_PRR);
	switch (reg & PRR_PRODUCT_MASK) {
	case PRR_PRODUCT_M3:
		ret = fdt_appendprop_string(dt, 0, "compatible",
					    "renesas,r8a774a1");
		break;
	case PRR_PRODUCT_H3:
		ret = fdt_appendprop_string(dt, 0, "compatible",
					    "renesas,r8a774e1");
		break;
	case PRR_PRODUCT_M3N:
		ret = fdt_appendprop_string(dt, 0, "compatible",
					    "renesas,r8a774b1");
		break;
	case PRR_PRODUCT_E3:
		ret = fdt_appendprop_string(dt, 0, "compatible",
					    "renesas,r8a774c0");
		break;
	default:
		NOTICE("BL2: Cannot set compatible string, SoC unsupported\n");
		panic();
		break;
	}

	if (ret < 0) {
		NOTICE("BL2: Cannot set compatible string (ret=%i)\n", ret);
		panic();
	}
}

static int bl2_add_memory_node(uint64_t start, uint64_t size)
{
	char nodename[32] = { 0 };
	uint64_t fdtsize;
	int ret, node;

	fdtsize = cpu_to_fdt64(size);

	snprintf(nodename, sizeof(nodename), "memory@");
	unsigned_num_print(start, 16, nodename + strlen(nodename));
	node = ret = fdt_add_subnode(fdt, 0, nodename);
	if (ret < 0) {
		return ret;
	}

	ret = fdt_setprop_string(fdt, node, "device_type", "memory");
	if (ret < 0) {
		return ret;
	}

	ret = fdt_setprop_u64(fdt, node, "reg", start);
	if (ret < 0) {
		return ret;
	}

	return fdt_appendprop(fdt, node, "reg", &fdtsize, sizeof(fdtsize));
}

static void bl2_advertise_dram_entries(uint64_t dram_config[8])
{
	uint64_t start, size;
	int ret, chan;

	for (chan = 0; chan < MAX_DRAM_CHANNELS; chan++) {
		start = dram_config[2 * chan];
		size = dram_config[2 * chan + 1];
		if (size == 0U) {
			continue;
		}

		NOTICE("BL2: CH%d: %llx - %llx, %lld %siB\n",
		       chan, start, start + size - 1U,
		       (size >> 30) ? : size >> 20,
		       (size >> 30) ? "G" : "M");
	}

	/*
	 * We add the DT nodes in reverse order here. The fdt_add_subnode()
	 * adds the DT node before the first existing DT node, so we have
	 * to add them in reverse order to get nodes sorted by address in
	 * the resulting DT.
	 */
	for (chan = MAX_DRAM_CHANNELS - 1; chan >= 0; chan--) {
		start = dram_config[2 * chan];
		size = dram_config[2 * chan + 1];
		if (size == 0U) {
			continue;
		}

		/*
		 * Channel 0 is mapped in 32bit space and the first
		 * 128 MiB are reserved
		 */
		if (chan == 0) {
			/*
			 * Maximum DDR size in Channel 0 for 32 bit space is 2GB, Add DT node
			 * for remaining region in 64 bit address space
			 */
			if (size > MAX_DRAM_SIZE_CH0_32BIT_ADDR_SPACE) {
				start = dram_config[chan] + MAX_DRAM_SIZE_CH0_32BIT_ADDR_SPACE;
				size -= MAX_DRAM_SIZE_CH0_32BIT_ADDR_SPACE;
				ret = bl2_add_memory_node(start, size);
				if (ret < 0) {
					goto err;
				}
			}
			start = 0x48000000U;
			size -= 0x8000000U;
		}

		ret = bl2_add_memory_node(start, size);
		if (ret < 0) {
			goto err;
		}
	}

	return;
err:
	NOTICE("BL2: Cannot add memory node to FDT (ret=%i)\n", ret);
	panic();
}

static void bl2_advertise_dram_size(uint32_t product)
{
	uint64_t dram_config[8] = {
		[0] = 0x400000000ULL,
		[2] = 0x500000000ULL,
		[4] = 0x600000000ULL,
		[6] = 0x700000000ULL,
	};

	switch (product) {
	case PRR_PRODUCT_M3:
		/* 4GB(2GBx2 2ch split) */
		dram_config[1] = 0x80000000ULL;
		dram_config[5] = 0x80000000ULL;
		break;
	case PRR_PRODUCT_H3:
#if (RCAR_DRAM_LPDDR4_MEMCONF == 0)
		/* 4GB(1GBx4) */
		dram_config[1] = 0x40000000ULL;
		dram_config[3] = 0x40000000ULL;
		dram_config[5] = 0x40000000ULL;
		dram_config[7] = 0x40000000ULL;
#elif (RCAR_DRAM_LPDDR4_MEMCONF == 1) && (RCAR_DRAM_CHANNEL == 5) && \
	(RCAR_DRAM_SPLIT == 2)
		/* 4GB(2GBx2 2ch split) */
		dram_config[1] = 0x80000000ULL;
		dram_config[3] = 0x80000000ULL;
#elif (RCAR_DRAM_LPDDR4_MEMCONF == 1) && (RCAR_DRAM_CHANNEL == 15)
		/* 8GB(2GBx4: default) */
		dram_config[1] = 0x80000000ULL;
		dram_config[3] = 0x80000000ULL;
		dram_config[5] = 0x80000000ULL;
		dram_config[7] = 0x80000000ULL;
#endif /* RCAR_DRAM_LPDDR4_MEMCONF == 0 */
		break;
	case PRR_PRODUCT_M3N:
		/* 4GB(4GBx1) */
		dram_config[1] = 0x100000000ULL;
		break;
	case PRR_PRODUCT_E3:
#if (RCAR_DRAM_DDR3L_MEMCONF == 0)
		/* 1GB(512MBx2) */
		dram_config[1] = 0x40000000ULL;
#elif (RCAR_DRAM_DDR3L_MEMCONF == 1)
		/* 2GB(512MBx4) */
		dram_config[1] = 0x80000000ULL;
#elif (RCAR_DRAM_DDR3L_MEMCONF == 2)
		/* 4GB(1GBx4) */
		dram_config[1] = 0x100000000ULL;
#endif /* RCAR_DRAM_DDR3L_MEMCONF == 0 */
		break;
	default:
		NOTICE("BL2: Detected invalid DRAM entries\n");
		break;
	}

	bl2_advertise_dram_entries(dram_config);
}

void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
				  u_register_t arg3, u_register_t arg4)
{
	uint32_t reg, midr, boot_dev, boot_cpu, type, rev;
	uint32_t product, product_cut, major, minor;
	int32_t ret;
	const char *str;
	const char *unknown = "unknown";
	const char *cpu_ca57 = "CA57";
	const char *cpu_ca53 = "CA53";
	const char *product_g2e = "G2E";
	const char *product_g2h = "G2H";
	const char *product_g2m = "G2M";
	const char *product_g2n = "G2N";
	const char *boot_hyper80 = "HyperFlash(80MHz)";
	const char *boot_qspi40 = "QSPI Flash(40MHz)";
	const char *boot_qspi80 = "QSPI Flash(80MHz)";
	const char *boot_emmc25x1 = "eMMC(25MHz x1)";
	const char *boot_emmc50x8 = "eMMC(50MHz x8)";
#if (RCAR_LSI == RZ_G2E)
	uint32_t sscg;
	const char *sscg_on = "PLL1 SSCG Clock select";
	const char *sscg_off = "PLL1 nonSSCG Clock select";
	const char *boot_hyper160 = "HyperFlash(150MHz)";
#else
	const char *boot_hyper160 = "HyperFlash(160MHz)";
#endif /* RCAR_LSI == RZ_G2E */
#if RZG_LCS_STATE_DETECTION_ENABLE
	uint32_t lcs;
	const char *lcs_secure = "SE";
	const char *lcs_cm = "CM";
	const char *lcs_dm = "DM";
	const char *lcs_sd = "SD";
	const char *lcs_fa = "FA";
#endif /* RZG_LCS_STATE_DETECTION_ENABLE */

#if (RCAR_LOSSY_ENABLE == 1)
	int fcnlnode;
#endif /* (RCAR_LOSSY_ENABLE == 1) */

	bl2_init_generic_timer();

	reg = mmio_read_32(RCAR_MODEMR);
	boot_dev = reg & MODEMR_BOOT_DEV_MASK;
	boot_cpu = reg & MODEMR_BOOT_CPU_MASK;

	bl2_cpg_init();

	if (boot_cpu == MODEMR_BOOT_CPU_CA57 ||
	    boot_cpu == MODEMR_BOOT_CPU_CA53) {
		rzg_pfc_init();
		rcar_console_boot_init();
	}

	plat_rcar_gic_driver_init();
	plat_rcar_gic_init();
	rcar_swdt_init();

	/* FIQ interrupts are taken to EL3 */
	write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);

	write_daifclr(DAIF_FIQ_BIT);

	reg = read_midr();
	midr = reg & (MIDR_PN_MASK << MIDR_PN_SHIFT);
	switch (midr) {
	case MIDR_CA57:
		str = cpu_ca57;
		break;
	case MIDR_CA53:
		str = cpu_ca53;
		break;
	default:
		str = unknown;
		break;
	}

	NOTICE("BL2: RZ/G2 Initial Program Loader(%s) Rev.%s\n", str,
	       version_of_renesas);

	reg = mmio_read_32(RCAR_PRR);
	product_cut = reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK);
	product = reg & PRR_PRODUCT_MASK;

	switch (product) {
	case PRR_PRODUCT_M3:
		str = product_g2m;
		break;
	case PRR_PRODUCT_H3:
		str = product_g2h;
		break;
	case PRR_PRODUCT_M3N:
		str = product_g2n;
		break;
	case PRR_PRODUCT_E3:
		str = product_g2e;
		break;
	default:
		str = unknown;
		break;
	}

	if ((product == PRR_PRODUCT_M3) &&
	    ((reg & RCAR_MAJOR_MASK) == PRR_PRODUCT_20)) {
		if ((reg & PRR_CUT_MASK) == RCAR_M3_CUT_VER11) {
			/* M3 Ver.1.1 or Ver.1.2 */
			NOTICE("BL2: PRR is RZ/%s Ver.1.1 / Ver.1.2\n", str);
		} else {
			NOTICE("BL2: PRR is RZ/%s Ver.1.%d\n", str,
				(reg & RCAR_MINOR_MASK) + RCAR_M3_MINOR_OFFSET);
		}
	} else {
		major = (reg & RCAR_MAJOR_MASK) >> RCAR_MAJOR_SHIFT;
		major = major + RCAR_MAJOR_OFFSET;
		minor = reg & RCAR_MINOR_MASK;
		NOTICE("BL2: PRR is RZ/%s Ver.%d.%d\n", str, major, minor);
	}

#if (RCAR_LSI == RZ_G2E)
	if (product == PRR_PRODUCT_E3) {
		reg = mmio_read_32(RCAR_MODEMR);
		sscg = reg & RCAR_SSCG_MASK;
		str = sscg == RCAR_SSCG_ENABLE ? sscg_on : sscg_off;
		NOTICE("BL2: %s\n", str);
	}
#endif /* RCAR_LSI == RZ_G2E */

	rzg_get_board_type(&type, &rev);

	switch (type) {
	case BOARD_HIHOPE_RZ_G2M:
	case BOARD_HIHOPE_RZ_G2H:
	case BOARD_HIHOPE_RZ_G2N:
	case BOARD_EK874_RZ_G2E:
		break;
	default:
		type = BOARD_UNKNOWN;
		break;
	}

	if (type == BOARD_UNKNOWN || rev == BOARD_REV_UNKNOWN) {
		NOTICE("BL2: Board is %s Rev.---\n", GET_BOARD_NAME(type));
	} else {
		NOTICE("BL2: Board is %s Rev.%d.%d\n",
		       GET_BOARD_NAME(type),
		       GET_BOARD_MAJOR(rev), GET_BOARD_MINOR(rev));
	}

#if RCAR_LSI != RCAR_AUTO
	if (product != TARGET_PRODUCT) {
		ERROR("BL2: IPL was been built for the %s.\n", TARGET_NAME);
		ERROR("BL2: Please write the correct IPL to flash memory.\n");
		panic();
	}
#endif /* RCAR_LSI != RCAR_AUTO */
	rcar_avs_init();
	rcar_avs_setting();

	switch (boot_dev) {
	case MODEMR_BOOT_DEV_HYPERFLASH160:
		str = boot_hyper160;
		break;
	case MODEMR_BOOT_DEV_HYPERFLASH80:
		str = boot_hyper80;
		break;
	case MODEMR_BOOT_DEV_QSPI_FLASH40:
		str = boot_qspi40;
		break;
	case MODEMR_BOOT_DEV_QSPI_FLASH80:
		str = boot_qspi80;
		break;
	case MODEMR_BOOT_DEV_EMMC_25X1:
		str = boot_emmc25x1;
		break;
	case MODEMR_BOOT_DEV_EMMC_50X8:
		str = boot_emmc50x8;
		break;
	default:
		str = unknown;
		break;
	}
	NOTICE("BL2: Boot device is %s\n", str);

	rcar_avs_setting();

#if RZG_LCS_STATE_DETECTION_ENABLE
	reg = rcar_rom_get_lcs(&lcs);
	if (reg != 0U) {
		str = unknown;
		goto lcm_state;
	}

	switch (lcs) {
	case LCS_CM:
		str = lcs_cm;
		break;
	case LCS_DM:
		str = lcs_dm;
		break;
	case LCS_SD:
		str = lcs_sd;
		break;
	case LCS_SE:
		str = lcs_secure;
		break;
	case LCS_FA:
		str = lcs_fa;
		break;
	default:
		str = unknown;
		break;
	}

lcm_state:
	NOTICE("BL2: LCM state is %s\n", str);
#endif /* RZG_LCS_STATE_DETECTION_ENABLE */

	rcar_avs_end();
	is_ddr_backup_mode();

	bl2_tzram_layout.total_base = BL31_BASE;
	bl2_tzram_layout.total_size = BL31_LIMIT - BL31_BASE;

	if (boot_cpu == MODEMR_BOOT_CPU_CA57 ||
	    boot_cpu == MODEMR_BOOT_CPU_CA53) {
		ret = rcar_dram_init();
		if (ret != 0) {
			NOTICE("BL2: Failed to DRAM initialize (%d).\n", ret);
			panic();
		}
		rzg_qos_init();
	}

	/* Set up FDT */
	ret = fdt_create_empty_tree(fdt, sizeof(fdt_blob));
	if (ret != 0) {
		NOTICE("BL2: Cannot allocate FDT for U-Boot (ret=%i)\n", ret);
		panic();
	}

	/* Add platform compatible string */
	bl2_populate_compatible_string(fdt);

	/* Print DRAM layout */
	bl2_advertise_dram_size(product);

	if (boot_dev == MODEMR_BOOT_DEV_EMMC_25X1 ||
	    boot_dev == MODEMR_BOOT_DEV_EMMC_50X8) {
		if (rcar_emmc_init() != EMMC_SUCCESS) {
			NOTICE("BL2: Failed to eMMC driver initialize.\n");
			panic();
		}
		rcar_emmc_memcard_power(EMMC_POWER_ON);
		if (rcar_emmc_mount() != EMMC_SUCCESS) {
			NOTICE("BL2: Failed to eMMC mount operation.\n");
			panic();
		}
	} else {
		rcar_rpc_init();
		rcar_dma_init();
	}

	reg = mmio_read_32(RST_WDTRSTCR);
	reg &= ~WDTRSTCR_RWDT_RSTMSK;
	reg |= WDTRSTCR_PASSWORD;
	mmio_write_32(RST_WDTRSTCR, reg);

	mmio_write_32(CPG_CPGWPR, CPGWPR_PASSWORD);
	mmio_write_32(CPG_CPGWPCR, CPGWPCR_PASSWORD);

	reg = mmio_read_32(RCAR_PRR);
	if ((reg & RCAR_CPU_MASK_CA57) == RCAR_CPU_HAVE_CA57) {
		mmio_write_32(CPG_CA57DBGRCR,
			      DBGCPUPREN | mmio_read_32(CPG_CA57DBGRCR));
	}

	if ((reg & RCAR_CPU_MASK_CA53) == RCAR_CPU_HAVE_CA53) {
		mmio_write_32(CPG_CA53DBGRCR,
			      DBGCPUPREN | mmio_read_32(CPG_CA53DBGRCR));
	}

	if (product_cut == PRR_PRODUCT_H3_CUT10) {
		reg = mmio_read_32(CPG_PLL2CR);
		reg &= ~((uint32_t)1 << 5);
		mmio_write_32(CPG_PLL2CR, reg);

		reg = mmio_read_32(CPG_PLL4CR);
		reg &= ~((uint32_t)1 << 5);
		mmio_write_32(CPG_PLL4CR, reg);

		reg = mmio_read_32(CPG_PLL0CR);
		reg &= ~((uint32_t)1 << 12);
		mmio_write_32(CPG_PLL0CR, reg);
	}
#if (RCAR_LOSSY_ENABLE == 1)
	NOTICE("BL2: Lossy Decomp areas\n");

	fcnlnode = fdt_add_subnode(fdt, 0, "reserved-memory");
	if (fcnlnode < 0) {
		NOTICE("BL2: Cannot create reserved mem node (ret=%i)\n",
		       fcnlnode);
		panic();
	}

	bl2_lossy_setting(0, LOSSY_ST_ADDR0, LOSSY_END_ADDR0,
			  LOSSY_FMT0, LOSSY_ENA_DIS0, fcnlnode);
	bl2_lossy_setting(1, LOSSY_ST_ADDR1, LOSSY_END_ADDR1,
			  LOSSY_FMT1, LOSSY_ENA_DIS1, fcnlnode);
	bl2_lossy_setting(2, LOSSY_ST_ADDR2, LOSSY_END_ADDR2,
			  LOSSY_FMT2, LOSSY_ENA_DIS2, fcnlnode);
#endif /* RCAR_LOSSY_ENABLE */

	fdt_pack(fdt);
	NOTICE("BL2: FDT at %p\n", fdt);

	if (boot_dev == MODEMR_BOOT_DEV_EMMC_25X1 ||
	    boot_dev == MODEMR_BOOT_DEV_EMMC_50X8) {
		rcar_io_emmc_setup();
	} else {
		rcar_io_setup();
	}
}

void bl2_el3_plat_arch_setup(void)
{
#if RCAR_BL2_DCACHE == 1
	NOTICE("BL2: D-Cache enable\n");
	rcar_configure_mmu_el3(BL2_BASE,
			       BL2_END - BL2_BASE,
			       BL2_RO_BASE, BL2_RO_LIMIT
#if USE_COHERENT_MEM
			       , BL2_COHERENT_RAM_BASE, BL2_COHERENT_RAM_LIMIT
#endif /* USE_COHERENT_MEM */
	    );
#endif /* RCAR_BL2_DCACHE == 1 */
}

void bl2_platform_setup(void)
{
	/*
	 * Place holder for performing any platform initialization specific
	 * to BL2.
	 */
}

static void bl2_init_generic_timer(void)
{
#if RCAR_LSI == RZ_G2E
	uint32_t reg_cntfid = EXTAL_EBISU;
#else
	uint32_t reg_cntfid;
	uint32_t modemr;
	uint32_t modemr_pll;
	uint32_t pll_table[] = {
		EXTAL_MD14_MD13_TYPE_0,	/* MD14/MD13 : 0b00 */
		EXTAL_MD14_MD13_TYPE_1,	/* MD14/MD13 : 0b01 */
		EXTAL_MD14_MD13_TYPE_2,	/* MD14/MD13 : 0b10 */
		EXTAL_MD14_MD13_TYPE_3	/* MD14/MD13 : 0b11 */
	};

	modemr = mmio_read_32(RCAR_MODEMR);
	modemr_pll = (modemr & MODEMR_BOOT_PLL_MASK);

	/* Set frequency data in CNTFID0 */
	reg_cntfid = pll_table[modemr_pll >> MODEMR_BOOT_PLL_SHIFT];
#endif /* RCAR_LSI == RZ_G2E */

	/* Update memory mapped and register based frequency */
	write_cntfrq_el0((u_register_t)reg_cntfid);
	mmio_write_32(ARM_SYS_CNTCTL_BASE + (uintptr_t)CNTFID_OFF, reg_cntfid);
	/* Enable counter */
	mmio_setbits_32(RCAR_CNTC_BASE + (uintptr_t)CNTCR_OFF,
			(uint32_t)CNTCR_EN);
}
