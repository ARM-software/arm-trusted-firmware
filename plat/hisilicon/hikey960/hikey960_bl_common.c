/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/pl061_gpio.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <hi3660.h>
#include "hikey960_private.h"

void hikey960_clk_init(void)
{
	/* change ldi0 sel to ppll2 */
	mmio_write_32(0xfff350b4, 0xf0002000);
	/* ldi0 20' */
	mmio_write_32(0xfff350bc, 0xfc004c00);
}

void hikey960_pmu_init(void)
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

void hikey960_regulator_enable(void)
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

void hikey960_tzc_init(void)
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

void hikey960_peri_init(void)
{
	/* unreset */
	mmio_setbits_32(CRG_PERRSTDIS4_REG, 1);
}

void hikey960_pinmux_init(void)
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

void hikey960_gpio_init(void)
{
	pl061_gpio_init();
	pl061_gpio_register(GPIO0_BASE, 0);
	pl061_gpio_register(GPIO1_BASE, 1);
	pl061_gpio_register(GPIO2_BASE, 2);
	pl061_gpio_register(GPIO3_BASE, 3);
	pl061_gpio_register(GPIO4_BASE, 4);
	pl061_gpio_register(GPIO5_BASE, 5);
	pl061_gpio_register(GPIO6_BASE, 6);
	pl061_gpio_register(GPIO7_BASE, 7);
	pl061_gpio_register(GPIO8_BASE, 8);
	pl061_gpio_register(GPIO9_BASE, 9);
	pl061_gpio_register(GPIO10_BASE, 10);
	pl061_gpio_register(GPIO11_BASE, 11);
	pl061_gpio_register(GPIO12_BASE, 12);
	pl061_gpio_register(GPIO13_BASE, 13);
	pl061_gpio_register(GPIO14_BASE, 14);
	pl061_gpio_register(GPIO15_BASE, 15);
	pl061_gpio_register(GPIO16_BASE, 16);
	pl061_gpio_register(GPIO17_BASE, 17);
	pl061_gpio_register(GPIO18_BASE, 18);
	pl061_gpio_register(GPIO19_BASE, 19);
	pl061_gpio_register(GPIO20_BASE, 20);
	pl061_gpio_register(GPIO21_BASE, 21);
	pl061_gpio_register(GPIO22_BASE, 22);
	pl061_gpio_register(GPIO23_BASE, 23);
	pl061_gpio_register(GPIO24_BASE, 24);
	pl061_gpio_register(GPIO25_BASE, 25);
	pl061_gpio_register(GPIO26_BASE, 26);
	pl061_gpio_register(GPIO27_BASE, 27);
	pl061_gpio_register(GPIO28_BASE, 28);

	/* PCIE_PERST_N output low */
	gpio_set_direction(89, GPIO_DIR_OUT);
	gpio_set_value(89, GPIO_LEVEL_LOW);
}
