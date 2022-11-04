/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/pl011.h>
#include <drivers/delay_timer.h>
#include <drivers/dw_ufs.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/partition/partition.h>
#include <drivers/ufs.h>
#include <lib/mmio.h>
#ifdef SPD_opteed
#include <lib/optee_utils.h>
#endif

#include <hi3660.h>
#include "hikey960_def.h"
#include "hikey960_private.h"

#define BL2_RW_BASE		(BL_CODE_END)

/* BL2 platform parameters passed to BL31 */
static plat_params_from_bl2_t plat_params_from_bl2;

static meminfo_t bl2_el3_tzram_layout;
static console_t console;
extern int load_lpm3(void);

enum {
	BOOT_MODE_RECOVERY = 0,
	BOOT_MODE_NORMAL,
	BOOT_MODE_MASK = 1,
};

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
int plat_hikey960_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	int i;
	int *buf;

	assert(scp_bl2_image_info->image_size < SCP_BL2_SIZE);

	INFO("BL2: Initiating SCP_BL2 transfer to SCP\n");

	INFO("BL2: SCP_BL2: 0x%lx@0x%x\n",
	     scp_bl2_image_info->image_base,
	     scp_bl2_image_info->image_size);

	buf = (int *)scp_bl2_image_info->image_base;

	INFO("BL2: SCP_BL2 HEAD:\n");
	for (i = 0; i < 64; i += 4)
		INFO("BL2: SCP_BL2 0x%x 0x%x 0x%x 0x%x\n",
			buf[i], buf[i+1], buf[i+2], buf[i+3]);

	buf = (int *)(scp_bl2_image_info->image_base +
		      scp_bl2_image_info->image_size - 256);

	INFO("BL2: SCP_BL2 TAIL:\n");
	for (i = 0; i < 64; i += 4)
		INFO("BL2: SCP_BL2 0x%x 0x%x 0x%x 0x%x\n",
			buf[i], buf[i+1], buf[i+2], buf[i+3]);

	INFO("BL2: SCP_BL2 transferred to SCP\n");

	load_lpm3();
	(void)buf;

	return 0;
}

static void hikey960_ufs_reset(void)
{
	unsigned int data, mask;

	mmio_write_32(CRG_PERDIS7_REG, 1 << 14);
	mmio_clrbits_32(UFS_SYS_PHY_CLK_CTRL_REG, BIT_SYSCTRL_REF_CLOCK_EN);
	do {
		data = mmio_read_32(UFS_SYS_PHY_CLK_CTRL_REG);
	} while (data & BIT_SYSCTRL_REF_CLOCK_EN);
	/* use abb clk */
	mmio_clrbits_32(UFS_SYS_UFS_SYSCTRL_REG, BIT_UFS_REFCLK_SRC_SE1);
	mmio_clrbits_32(UFS_SYS_PHY_ISO_EN_REG, BIT_UFS_REFCLK_ISO_EN);
	mmio_write_32(PCTRL_PERI_CTRL3_REG, (1 << 0) | (1 << 16));
	mdelay(1);
	mmio_write_32(CRG_PEREN7_REG, 1 << 14);
	mmio_setbits_32(UFS_SYS_PHY_CLK_CTRL_REG, BIT_SYSCTRL_REF_CLOCK_EN);

	mmio_write_32(CRG_PERRSTEN3_REG, PERI_UFS_BIT);
	do {
		data = mmio_read_32(CRG_PERRSTSTAT3_REG);
	} while ((data & PERI_UFS_BIT) == 0);
	mmio_setbits_32(UFS_SYS_PSW_POWER_CTRL_REG, BIT_UFS_PSW_MTCMOS_EN);
	mdelay(1);
	mmio_setbits_32(UFS_SYS_HC_LP_CTRL_REG, BIT_SYSCTRL_PWR_READY);
	mmio_write_32(UFS_SYS_UFS_DEVICE_RESET_CTRL_REG,
		      MASK_UFS_DEVICE_RESET);
	/* clear SC_DIV_UFS_PERIBUS */
	mask = SC_DIV_UFS_PERIBUS << 16;
	mmio_write_32(CRG_CLKDIV17_REG, mask);
	/* set SC_DIV_UFSPHY_CFG(3) */
	mask = SC_DIV_UFSPHY_CFG_MASK << 16;
	data = SC_DIV_UFSPHY_CFG(3);
	mmio_write_32(CRG_CLKDIV16_REG, mask | data);
	data = mmio_read_32(UFS_SYS_PHY_CLK_CTRL_REG);
	data &= ~MASK_SYSCTRL_CFG_CLOCK_FREQ;
	data |= 0x39;
	mmio_write_32(UFS_SYS_PHY_CLK_CTRL_REG, data);
	mmio_clrbits_32(UFS_SYS_PHY_CLK_CTRL_REG, MASK_SYSCTRL_REF_CLOCK_SEL);
	mmio_setbits_32(UFS_SYS_CLOCK_GATE_BYPASS_REG,
			MASK_UFS_CLK_GATE_BYPASS);
	mmio_setbits_32(UFS_SYS_UFS_SYSCTRL_REG, MASK_UFS_SYSCTRL_BYPASS);

	mmio_setbits_32(UFS_SYS_PSW_CLK_CTRL_REG, BIT_SYSCTRL_PSW_CLK_EN);
	mmio_clrbits_32(UFS_SYS_PSW_POWER_CTRL_REG, BIT_UFS_PSW_ISO_CTRL);
	mmio_clrbits_32(UFS_SYS_PHY_ISO_EN_REG, BIT_UFS_PHY_ISO_CTRL);
	mmio_clrbits_32(UFS_SYS_HC_LP_CTRL_REG, BIT_SYSCTRL_LP_ISOL_EN);
	mmio_write_32(CRG_PERRSTDIS3_REG, PERI_ARST_UFS_BIT);
	mmio_setbits_32(UFS_SYS_RESET_CTRL_EN_REG, BIT_SYSCTRL_LP_RESET_N);
	mdelay(1);
	mmio_write_32(UFS_SYS_UFS_DEVICE_RESET_CTRL_REG,
		      MASK_UFS_DEVICE_RESET | BIT_UFS_DEVICE_RESET);
	mdelay(20);
	mmio_write_32(UFS_SYS_UFS_DEVICE_RESET_CTRL_REG,
		      0x03300330);

	mmio_write_32(CRG_PERRSTDIS3_REG, PERI_UFS_BIT);
	do {
		data = mmio_read_32(CRG_PERRSTSTAT3_REG);
	} while (data & PERI_UFS_BIT);
}

static void hikey960_init_ufs(void)
{
	dw_ufs_params_t ufs_params;

	memset(&ufs_params, 0, sizeof(ufs_params_t));
	ufs_params.reg_base = UFS_REG_BASE;
	ufs_params.desc_base = HIKEY960_UFS_DESC_BASE;
	ufs_params.desc_size = HIKEY960_UFS_DESC_SIZE;
	hikey960_ufs_reset();
	dw_ufs_init(&ufs_params);
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t hikey960_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL3-2 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
#ifdef __aarch64__
uint32_t hikey960_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#else
uint32_t hikey960_get_spsr_for_bl33_entry(void)
{
	unsigned int hyp_status, mode, spsr;

	hyp_status = GET_VIRT_EXT(read_id_pfr1());

	mode = (hyp_status) ? MODE32_hyp : MODE32_svc;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_MODE32(mode, plat_get_ns_image_entrypoint() & 0x1,
			SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#endif /* __aarch64__ */

int hikey960_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#ifdef SPD_opteed
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif
	assert(bl_mem_params);

	switch (image_id) {
	case BL31_IMAGE_ID:
		/* Pass BL2 platform parameter to BL31 */
		bl_mem_params->ep_info.args.arg1 = (uint64_t) &plat_params_from_bl2;
		break;

#ifdef __aarch64__
	case BL32_IMAGE_ID:
#ifdef SPD_opteed
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
				&pager_mem_params->image_info,
				&paged_mem_params->image_info);
		if (err != 0) {
			WARN("OPTEE header parse error.\n");
		}
#endif
		bl_mem_params->ep_info.spsr = hikey960_get_spsr_for_bl32_entry();
		break;
#endif

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = hikey960_get_spsr_for_bl33_entry();
		break;

#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = plat_hikey960_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err) {
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
		break;
#endif
	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	return hikey960_set_fip_addr(image_id, "fip");
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return hikey960_bl2_handle_post_image_load(image_id);
}

void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
				  u_register_t arg3, u_register_t arg4)
{
	unsigned int id, uart_base;

	generic_delay_timer_init();
	hikey960_read_boardid(&id);
	if (id == 5300)
		uart_base = PL011_UART5_BASE;
	else
		uart_base = PL011_UART6_BASE;
	/* Initialize the console to provide early debug support */
	console_pl011_register(uart_base, PL011_UART_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);
	/*
	 * Allow BL2 to see the whole Trusted RAM.
	 */
	bl2_el3_tzram_layout.total_base = BL2_RW_BASE;
	bl2_el3_tzram_layout.total_size = BL31_LIMIT - BL2_RW_BASE;
}

void bl2_el3_plat_arch_setup(void)
{
	hikey960_init_mmu_el3(bl2_el3_tzram_layout.total_base,
			      bl2_el3_tzram_layout.total_size,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END);
}

void bl2_platform_setup(void)
{
	int ret;

	/* disable WDT0 */
	if (mmio_read_32(WDT0_REG_BASE + WDT_LOCK_OFFSET) == WDT_LOCKED) {
		mmio_write_32(WDT0_REG_BASE + WDT_LOCK_OFFSET, WDT_UNLOCK);
		mmio_write_32(WDT0_REG_BASE + WDT_CONTROL_OFFSET, 0);
		mmio_write_32(WDT0_REG_BASE + WDT_LOCK_OFFSET, 0);
	}
	hikey960_clk_init();
	hikey960_pmu_init();
	hikey960_regulator_enable();
	hikey960_tzc_init();
	hikey960_peri_init();
	hikey960_pinmux_init();
	hikey960_gpio_init();
	hikey960_init_ufs();
	hikey960_io_setup();

	/* Read serial number from storage */
	plat_params_from_bl2.fastboot_serno = 0;
	ret = hikey960_load_serialno(&plat_params_from_bl2.fastboot_serno);
	if (ret != 0) {
		ERROR("BL2: could not read serial number\n");
	}
	INFO("BL2: fastboot_serno %lx\n", plat_params_from_bl2.fastboot_serno);
	flush_dcache_range((uintptr_t)&plat_params_from_bl2, sizeof(plat_params_from_bl2_t));
}
