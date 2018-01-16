/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <delay_timer.h>
#include <dw_ufs.h>
#include <errno.h>
#include <generic_delay_timer.h>
#include <gicv2.h>
#include <hi3660.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <tbbr/tbbr_img_desc.h>
#include <ufs.h>

#include "../../bl1/bl1_private.h"
#include "hikey960_def.h"
#include "hikey960_private.h"

enum {
	BOOT_MODE_RECOVERY = 0,
	BOOT_MODE_NORMAL,
	BOOT_MODE_MASK = 1,
};

/*
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted RAM
 */
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL1_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
const unsigned int g0_interrupt_array[] = {
	IRQ_SEC_PHY_TIMER,
	IRQ_SEC_SGI_0
};

const gicv2_driver_data_t hikey960_gic_data = {
	.gicd_base = GICD_REG_BASE,
	.gicc_base = GICC_REG_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
};

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

#if LOAD_IMAGE_V2
/*******************************************************************************
 * Function that takes a memory layout into which BL2 has been loaded and
 * populates a new memory layout for BL2 that ensures that BL1's data sections
 * resident in secure RAM are not visible to BL2.
 ******************************************************************************/
void bl1_init_bl2_mem_layout(const meminfo_t *bl1_mem_layout,
			     meminfo_t *bl2_mem_layout)
{

	assert(bl1_mem_layout != NULL);
	assert(bl2_mem_layout != NULL);

	/*
	 * Cannot remove BL1 RW data from the scope of memory visible to BL2
	 * like arm platforms because they overlap in hikey960
	 */
	bl2_mem_layout->total_base = BL2_BASE;
	bl2_mem_layout->total_size = NS_BL1U_LIMIT - BL2_BASE;

	flush_dcache_range((unsigned long)bl2_mem_layout, sizeof(meminfo_t));
}
#endif /* LOAD_IMAGE_V2 */

/*
 * Perform any BL1 specific platform actions.
 */
void bl1_early_platform_setup(void)
{
	unsigned int id, uart_base;

	generic_delay_timer_init();
	hikey960_read_boardid(&id);
	if (id == 5300)
		uart_base = PL011_UART5_BASE;
	else
		uart_base = PL011_UART6_BASE;
	/* Initialize the console to provide early debug support */
	console_init(uart_base, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL1_RW_BASE;
	bl1_tzram_layout.total_size = BL1_RW_SIZE;

#if !LOAD_IMAGE_V2
	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_tzram_layout.free_base = BL1_RW_BASE;
	bl1_tzram_layout.free_size = BL1_RW_SIZE;
	reserve_mem(&bl1_tzram_layout.free_base,
		    &bl1_tzram_layout.free_size,
		    BL1_RAM_BASE,
		    BL1_RAM_LIMIT - BL1_RAM_BASE); /* bl1_size */
#endif /* LOAD_IMAGE_V2 */

	INFO("BL1: 0x%lx - 0x%lx [size = %lu]\n", BL1_RAM_BASE, BL1_RAM_LIMIT,
	     BL1_RAM_LIMIT - BL1_RAM_BASE); /* bl1_size */
}

/*
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 */
void bl1_plat_arch_setup(void)
{
	hikey960_init_mmu_el3(bl1_tzram_layout.total_base,
			      bl1_tzram_layout.total_size,
			      BL1_RO_BASE,
			      BL1_RO_LIMIT,
			      BL1_COHERENT_RAM_BASE,
			      BL1_COHERENT_RAM_LIMIT);
}

static void hikey960_clk_init(void)
{
	/* change ldi0 sel to ppll2 */
	mmio_write_32(0xfff350b4, 0xf0002000);
	/* ldi0 20' */
	mmio_write_32(0xfff350bc, 0xfc004c00);
}

static void hikey960_pmu_init(void)
{
	/* clear np_xo_abb_dig_START bit in PMIC_CLK_TOP_CTRL7 register */
	mmio_clrbits_32(PMU_SSI0_CLK_TOP_CTRL7_REG, NP_XO_ABB_DIG);
}

static void hikey960_enable_ppll3(void)
{
	/* enable ppll3 */
	mmio_write_32(PMC_PPLL3_CTRL0_REG, 0x4904305);
	mmio_write_32(PMC_PPLL3_CTRL1_REG, 0x2300000);
	mmio_write_32(PMC_PPLL3_CTRL1_REG, 0x6300000);
}

static void bus_idle_clear(unsigned int value)
{
	unsigned int pmc_value, value1, value2;
	int timeout = 100;

	pmc_value = value << 16;
	pmc_value &= ~value;
	mmio_write_32(PMC_NOC_POWER_IDLEREQ_REG, pmc_value);

	for (;;) {
		value1 = (unsigned int)mmio_read_32(PMC_NOC_POWER_IDLEACK_REG);
		value2 = (unsigned int)mmio_read_32(PMC_NOC_POWER_IDLE_REG);
		if (((value1 & value) == 0) && ((value2 & value) == 0))
			break;
		udelay(1);
		timeout--;
		if (timeout <= 0) {
			WARN("%s timeout\n", __func__);
			break;
		}
	}
}

static void set_vivobus_power_up(void)
{
	/* clk enable */
	mmio_write_32(CRG_CLKDIV20_REG, 0x00020002);
	mmio_write_32(CRG_PEREN0_REG, 0x00001000);
}

static void set_dss_power_up(void)
{
	/* set edc0 133MHz = 1600MHz / 12 */
	mmio_write_32(CRG_CLKDIV5_REG, 0x003f000b);
	/* set ldi0 ppl0 */
	mmio_write_32(CRG_CLKDIV3_REG, 0xf0001000);
	/* set ldi0 133MHz, 1600MHz / 12 */
	mmio_write_32(CRG_CLKDIV5_REG, 0xfc002c00);
	/* mtcmos on */
	mmio_write_32(CRG_PERPWREN_REG, 0x00000020);
	udelay(100);
	/* DISP CRG */
	mmio_write_32(CRG_PERRSTDIS4_REG, 0x00000010);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV18_REG, 0x01400140);
	mmio_write_32(CRG_PEREN0_REG, 0x00002000);
	mmio_write_32(CRG_PEREN3_REG, 0x0003b000);
	udelay(1);
	/* clk disable */
	mmio_write_32(CRG_PERDIS3_REG, 0x0003b000);
	mmio_write_32(CRG_PERDIS0_REG, 0x00002000);
	mmio_write_32(CRG_CLKDIV18_REG, 0x01400000);
	udelay(1);
	/* iso disable */
	mmio_write_32(CRG_ISODIS_REG, 0x00000040);
	/* unreset */
	mmio_write_32(CRG_PERRSTDIS4_REG, 0x00000006);
	mmio_write_32(CRG_PERRSTDIS3_REG, 0x00000c00);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV18_REG, 0x01400140);
	mmio_write_32(CRG_PEREN0_REG, 0x00002000);
	mmio_write_32(CRG_PEREN3_REG, 0x0003b000);
	/* bus idle clear */
	bus_idle_clear(PMC_NOC_POWER_IDLEREQ_DSS);
	/* set edc0 400MHz for 2K 1600MHz / 4 */
	mmio_write_32(CRG_CLKDIV5_REG, 0x003f0003);
	/* set ldi 266MHz, 1600MHz / 6 */
	mmio_write_32(CRG_CLKDIV5_REG, 0xfc001400);
}

static void set_vcodec_power_up(void)
{
	/* clk enable */
	mmio_write_32(CRG_CLKDIV20_REG, 0x00040004);
	mmio_write_32(CRG_PEREN0_REG, 0x00000060);
	mmio_write_32(CRG_PEREN2_REG, 0x10000000);
	/* unreset */
	mmio_write_32(CRG_PERRSTDIS0_REG, 0x00000018);
	/* bus idle clear */
	bus_idle_clear(PMC_NOC_POWER_IDLEREQ_VCODEC);
}

static void set_vdec_power_up(void)
{
	/* mtcmos on */
	mmio_write_32(CRG_PERPWREN_REG, 0x00000004);
	udelay(100);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV18_REG, 0x80008000);
	mmio_write_32(CRG_PEREN2_REG, 0x20080000);
	mmio_write_32(CRG_PEREN3_REG, 0x00000800);
	udelay(1);
	/* clk disable */
	mmio_write_32(CRG_PERDIS3_REG, 0x00000800);
	mmio_write_32(CRG_PERDIS2_REG, 0x20080000);
	mmio_write_32(CRG_CLKDIV18_REG, 0x80000000);
	udelay(1);
	/* iso disable */
	mmio_write_32(CRG_ISODIS_REG, 0x00000004);
	/* unreset */
	mmio_write_32(CRG_PERRSTDIS3_REG, 0x00000200);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV18_REG, 0x80008000);
	mmio_write_32(CRG_PEREN2_REG, 0x20080000);
	mmio_write_32(CRG_PEREN3_REG, 0x00000800);
	/* bus idle clear */
	bus_idle_clear(PMC_NOC_POWER_IDLEREQ_VDEC);
}

static void set_venc_power_up(void)
{
	/* set venc ppll3 */
	mmio_write_32(CRG_CLKDIV8_REG, 0x18001000);
	/* set venc 258MHz, 1290MHz / 5 */
	mmio_write_32(CRG_CLKDIV8_REG, 0x07c00100);
	/* mtcmos on */
	mmio_write_32(CRG_PERPWREN_REG, 0x00000002);
	udelay(100);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV19_REG, 0x00010001);
	mmio_write_32(CRG_PEREN2_REG, 0x40000100);
	mmio_write_32(CRG_PEREN3_REG, 0x00000400);
	udelay(1);
	/* clk disable */
	mmio_write_32(CRG_PERDIS3_REG, 0x00000400);
	mmio_write_32(CRG_PERDIS2_REG, 0x40000100);
	mmio_write_32(CRG_CLKDIV19_REG, 0x00010000);
	udelay(1);
	/* iso disable */
	mmio_write_32(CRG_ISODIS_REG, 0x00000002);
	/* unreset */
	mmio_write_32(CRG_PERRSTDIS3_REG, 0x00000100);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV19_REG, 0x00010001);
	mmio_write_32(CRG_PEREN2_REG, 0x40000100);
	mmio_write_32(CRG_PEREN3_REG, 0x00000400);
	/* bus idle clear */
	bus_idle_clear(PMC_NOC_POWER_IDLEREQ_VENC);
	/* set venc 645MHz, 1290MHz / 2 */
	mmio_write_32(CRG_CLKDIV8_REG, 0x07c00040);
}

static void set_isp_power_up(void)
{
	/* mtcmos on */
	mmio_write_32(CRG_PERPWREN_REG, 0x00000001);
	udelay(100);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV18_REG, 0x70007000);
	mmio_write_32(CRG_CLKDIV20_REG, 0x00100010);
	mmio_write_32(CRG_PEREN5_REG, 0x01000010);
	mmio_write_32(CRG_PEREN3_REG, 0x0bf00000);
	udelay(1);
	/* clk disable */
	mmio_write_32(CRG_PERDIS5_REG, 0x01000010);
	mmio_write_32(CRG_PERDIS3_REG, 0x0bf00000);
	mmio_write_32(CRG_CLKDIV18_REG, 0x70000000);
	mmio_write_32(CRG_CLKDIV20_REG, 0x00100000);
	udelay(1);
	/* iso disable */
	mmio_write_32(CRG_ISODIS_REG, 0x00000001);
	/* unreset */
	mmio_write_32(CRG_ISP_SEC_RSTDIS_REG, 0x0000002f);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV18_REG, 0x70007000);
	mmio_write_32(CRG_CLKDIV20_REG, 0x00100010);
	mmio_write_32(CRG_PEREN5_REG, 0x01000010);
	mmio_write_32(CRG_PEREN3_REG, 0x0bf00000);
	/* bus idle clear */
	bus_idle_clear(PMC_NOC_POWER_IDLEREQ_ISP);
	/* csi clk enable */
	mmio_write_32(CRG_PEREN3_REG, 0x00700000);
}

static void set_ivp_power_up(void)
{
	/* set ivp ppll0 */
	mmio_write_32(CRG_CLKDIV0_REG, 0xc0000000);
	/* set ivp 267MHz, 1600MHz / 6 */
	mmio_write_32(CRG_CLKDIV0_REG, 0x3c001400);
	/* mtcmos on */
	mmio_write_32(CRG_PERPWREN_REG, 0x00200000);
	udelay(100);
	/* IVP CRG unreset */
	mmio_write_32(CRG_IVP_SEC_RSTDIS_REG, 0x00000001);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV20_REG, 0x02000200);
	mmio_write_32(CRG_PEREN4_REG, 0x000000a8);
	udelay(1);
	/* clk disable */
	mmio_write_32(CRG_PERDIS4_REG, 0x000000a8);
	mmio_write_32(CRG_CLKDIV20_REG, 0x02000000);
	udelay(1);
	/* iso disable */
	mmio_write_32(CRG_ISODIS_REG, 0x01000000);
	/* unreset */
	mmio_write_32(CRG_IVP_SEC_RSTDIS_REG, 0x00000002);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV20_REG, 0x02000200);
	mmio_write_32(CRG_PEREN4_REG, 0x000000a8);
	/* bus idle clear */
	bus_idle_clear(PMC_NOC_POWER_IDLEREQ_IVP);
	/* set ivp 533MHz, 1600MHz / 3 */
	mmio_write_32(CRG_CLKDIV0_REG, 0x3c000800);
}

static void set_audio_power_up(void)
{
	unsigned int ret;
	int timeout = 100;
	/* mtcmos on */
	mmio_write_32(SCTRL_SCPWREN_REG, 0x00000001);
	udelay(100);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV19_REG, 0x80108010);
	mmio_write_32(SCTRL_SCCLKDIV2_REG, 0x00010001);
	mmio_write_32(SCTRL_SCPEREN0_REG, 0x0c000000);
	mmio_write_32(CRG_PEREN0_REG, 0x04000000);
	mmio_write_32(CRG_PEREN5_REG, 0x00000080);
	mmio_write_32(SCTRL_SCPEREN1_REG, 0x0000000f);
	udelay(1);
	/* clk disable */
	mmio_write_32(SCTRL_SCPERDIS1_REG, 0x0000000f);
	mmio_write_32(SCTRL_SCPERDIS0_REG, 0x0c000000);
	mmio_write_32(CRG_PERDIS5_REG, 0x00000080);
	mmio_write_32(CRG_PERDIS0_REG, 0x04000000);
	mmio_write_32(SCTRL_SCCLKDIV2_REG, 0x00010000);
	mmio_write_32(CRG_CLKDIV19_REG, 0x80100000);
	udelay(1);
	/* iso disable */
	mmio_write_32(SCTRL_SCISODIS_REG, 0x00000001);
	udelay(1);
	/* unreset */
	mmio_write_32(SCTRL_PERRSTDIS1_SEC_REG, 0x00000001);
	mmio_write_32(SCTRL_SCPERRSTDIS0_REG, 0x00000780);
	/* clk enable */
	mmio_write_32(CRG_CLKDIV19_REG, 0x80108010);
	mmio_write_32(SCTRL_SCCLKDIV2_REG, 0x00010001);
	mmio_write_32(SCTRL_SCPEREN0_REG, 0x0c000000);
	mmio_write_32(CRG_PEREN0_REG, 0x04000000);
	mmio_write_32(CRG_PEREN5_REG, 0x00000080);
	mmio_write_32(SCTRL_SCPEREN1_REG, 0x0000000f);
	/* bus idle clear */
	mmio_write_32(SCTRL_SCPERCTRL7_REG, 0x00040000);
	for (;;) {
		ret = mmio_read_32(SCTRL_SCPERSTAT6_REG);
		if (((ret & (1 << 5)) == 0) && ((ret & (1 << 8)) == 0))
			break;
		udelay(1);
		timeout--;
		if (timeout <= 0) {
			WARN("%s timeout\n", __func__);
			break;
		}
	}
	mmio_write_32(ASP_CFG_MMBUF_CTRL_REG, 0x00ff0000);
}

static void set_pcie_power_up(void)
{
	/* mtcmos on */
	mmio_write_32(SCTRL_SCPWREN_REG, 0x00000010);
	udelay(100);
	/* clk enable */
	mmio_write_32(SCTRL_SCCLKDIV6_REG, 0x08000800);
	mmio_write_32(SCTRL_SCPEREN2_REG, 0x00104000);
	mmio_write_32(CRG_PEREN7_REG, 0x000003a0);
	udelay(1);
	/* clk disable */
	mmio_write_32(SCTRL_SCPERDIS2_REG, 0x00104000);
	mmio_write_32(CRG_PERDIS7_REG, 0x000003a0);
	mmio_write_32(SCTRL_SCCLKDIV6_REG, 0x08000000);
	udelay(1);
	/* iso disable */
	mmio_write_32(SCTRL_SCISODIS_REG, 0x00000030);
	/* unreset */
	mmio_write_32(CRG_PERRSTDIS3_REG, 0x8c000000);
	/* clk enable */
	mmio_write_32(SCTRL_SCCLKDIV6_REG, 0x08000800);
	mmio_write_32(SCTRL_SCPEREN2_REG, 0x00104000);
	mmio_write_32(CRG_PEREN7_REG, 0x000003a0);
}

static void ispfunc_enable(void)
{
	/* enable ispfunc. Otherwise powerup isp_srt causes exception. */
	mmio_write_32(0xfff35000, 0x00000008);
	mmio_write_32(0xfff35460, 0xc004ffff);
	mmio_write_32(0xfff35030, 0x02000000);
	mdelay(10);
}

static void isps_control_clock(int flag)
{
	unsigned int ret;

	/* flag: 0 -- disable clock, 1 -- enable clock */
	if (flag) {
		ret = mmio_read_32(0xe8420364);
		ret |= 1;
		mmio_write_32(0xe8420364, ret);
	} else {
		ret = mmio_read_32(0xe8420364);
		ret &= ~1;
		mmio_write_32(0xe8420364, ret);
	}
}

static void set_isp_srt_power_up(void)
{
	unsigned int ret;

	ispfunc_enable();
	/* reset */
	mmio_write_32(0xe8420374, 0x00000001);
	mmio_write_32(0xe8420350, 0x00000000);
	mmio_write_32(0xe8420358, 0x00000000);
	/* mtcmos on */
	mmio_write_32(0xfff35150, 0x00400000);
	udelay(100);
	/* clk enable */
	isps_control_clock(1);
	udelay(1);
	isps_control_clock(0);
	udelay(1);
	/* iso disable */
	mmio_write_32(0xfff35148, 0x08000000);
	/* unreset */
	ret = mmio_read_32(0xe8420374);
	ret &= ~0x1;
	mmio_write_32(0xe8420374, ret);
	/* clk enable */
	isps_control_clock(1);
	/* enable clock gating for accessing csi registers */
	mmio_write_32(0xe8420010, ~0);
}

static void hikey960_regulator_enable(void)
{
	set_vivobus_power_up();
	hikey960_enable_ppll3();
	set_dss_power_up();
	set_vcodec_power_up();
	set_vdec_power_up();
	set_venc_power_up();
	set_isp_power_up();
	set_ivp_power_up();
	set_audio_power_up();
	set_pcie_power_up();
	set_isp_srt_power_up();

	/* set ISP_CORE_CTRL_S to unsecure mode */
	mmio_write_32(0xe8583800, 0x7);
	/* set ISP_SUB_CTRL_S to unsecure mode */
	mmio_write_32(0xe8583804, 0xf);
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

static void hikey960_ufs_init(void)
{
	dw_ufs_params_t ufs_params;

	memset(&ufs_params, 0, sizeof(ufs_params));
	ufs_params.reg_base = UFS_REG_BASE;
	ufs_params.desc_base = HIKEY960_UFS_DESC_BASE;
	ufs_params.desc_size = HIKEY960_UFS_DESC_SIZE;

	if ((ufs_params.flags & UFS_FLAGS_SKIPINIT) == 0)
		hikey960_ufs_reset();
	dw_ufs_init(&ufs_params);
}

static void hikey960_tzc_init(void)
{
	mmio_write_32(TZC_EN0_REG, 0x7fbff066);
	mmio_write_32(TZC_EN1_REG, 0xfffff5fc);
	mmio_write_32(TZC_EN2_REG, 0x0007005c);
	mmio_write_32(TZC_EN3_REG, 0x37030700);
	mmio_write_32(TZC_EN4_REG, 0xf63fefae);
	mmio_write_32(TZC_EN5_REG, 0x000410fd);
	mmio_write_32(TZC_EN6_REG, 0x0063ff68);
	mmio_write_32(TZC_EN7_REG, 0x030000f3);
	mmio_write_32(TZC_EN8_REG, 0x00000007);
}

static void hikey960_peri_init(void)
{
	/* unreset */
	mmio_setbits_32(CRG_PERRSTDIS4_REG, 1);
}

static void hikey960_pinmux_init(void)
{
	unsigned int id;

	hikey960_read_boardid(&id);
	if (id == 5301) {
		/* hikey960 hardware v2 */
		/* GPIO150: LED */
		mmio_write_32(IOMG_FIX_006_REG, 0);
		/* GPIO151: LED */
		mmio_write_32(IOMG_FIX_007_REG, 0);
		/* GPIO189: LED */
		mmio_write_32(IOMG_AO_011_REG, 0);
		/* GPIO190: LED */
		mmio_write_32(IOMG_AO_012_REG, 0);
		/* GPIO46 */
		mmio_write_32(IOMG_044_REG, 0);
		/* GPIO202 */
		mmio_write_32(IOMG_AO_023_REG, 0);
		/* GPIO206 */
		mmio_write_32(IOMG_AO_026_REG, 0);
		/* GPIO219 - PD pullup */
		mmio_write_32(IOMG_AO_039_REG, 0);
		mmio_write_32(IOCG_AO_043_REG, 1 << 0);
	}
	/* GPIO005 - PMU SSI, 10mA */
	mmio_write_32(IOCG_006_REG, 2 << 4);
	/* GPIO213 - PCIE_CLKREQ_N */
	mmio_write_32(IOMG_AO_033_REG, 1);
}

/*
 * Function which will perform any remaining platform-specific setup that can
 * occur after the MMU and data cache have been enabled.
 */
void bl1_platform_setup(void)
{
	hikey960_clk_init();
	hikey960_pmu_init();
	hikey960_regulator_enable();
	hikey960_tzc_init();
	hikey960_peri_init();
	hikey960_ufs_init();
	hikey960_pinmux_init();
	hikey960_io_setup();
}

/*
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or not.
 */
unsigned int bl1_plat_get_next_image_id(void)
{
	unsigned int mode, ret;

	mode = mmio_read_32(SCTRL_BAK_DATA0_REG);
	switch (mode & BOOT_MODE_MASK) {
	case BOOT_MODE_RECOVERY:
		ret = NS_BL1U_IMAGE_ID;
		break;
	case BOOT_MODE_NORMAL:
		ret = BL2_IMAGE_ID;
		break;
	default:
		WARN("Invalid boot mode is found:%d\n", mode);
		panic();
	}
	return ret;
}

image_desc_t *bl1_plat_get_image_desc(unsigned int image_id)
{
	unsigned int index = 0;

	while (bl1_tbbr_image_descs[index].image_id != INVALID_IMAGE_ID) {
		if (bl1_tbbr_image_descs[index].image_id == image_id)
			return &bl1_tbbr_image_descs[index];
		index++;
	}

	return NULL;
}

void bl1_plat_set_ep_info(unsigned int image_id,
		entry_point_info_t *ep_info)
{
	unsigned int data = 0;
	uintptr_t tmp = HIKEY960_NS_TMP_OFFSET;

	if (image_id == BL2_IMAGE_ID)
		return;
	/* Copy NS BL1U from 0x1AC1_8000 to 0x1AC9_8000 */
	memcpy((void *)tmp, (void *)HIKEY960_NS_IMAGE_OFFSET,
		NS_BL1U_SIZE);
	memcpy((void *)NS_BL1U_BASE, (void *)tmp, NS_BL1U_SIZE);
	inv_dcache_range(NS_BL1U_BASE, NS_BL1U_SIZE);
	/* Initialize the GIC driver, cpu and distributor interfaces */
	gicv2_driver_init(&hikey960_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
	/* CNTFRQ is read-only in EL1 */
	write_cntfrq_el0(plat_get_syscnt_freq2());
	data = read_cpacr_el1();
	do {
		data |= 3 << 20;
		write_cpacr_el1(data);
		data = read_cpacr_el1();
	} while ((data & (3 << 20)) != (3 << 20));
	INFO("cpacr_el1:0x%x\n", data);

	ep_info->args.arg0 = 0xffff & read_mpidr();
	ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);
}
