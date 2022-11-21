/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <plat_imx8.h>
#include <xrdc.h>

#define PCC_PR	BIT(31)
#define PFD_VALID_MASK	U(0x40404040)

#define S400_MU_BASE	U(0x27020000)
#define S400_MU_RSR	(S400_MU_BASE + 0x12c)
#define S400_MU_TRx(i)	(S400_MU_BASE + 0x200 + (i) * 4)
#define S400_MU_RRx(i)	(S400_MU_BASE + 0x280 + (i) * 4)

/*
 * need to re-init the PLL, CGC1, PCC, CMC, XRDC, SIM, GPIO etc.
 * init the PLL &PFD first, then switch the CA35 clock to PLL for
 * performance consideration, restore other bus fabric clock.
 */

extern void imx8ulp_caam_init(void);
extern void upower_wait_resp(void);
extern void dram_enter_retention(void);
extern void dram_exit_retention(void);

struct plat_gic_ctx imx_gicv3_ctx;
static uint32_t cmc1_pmprot;
static uint32_t cmc1_srie;

/* TPM5: global timer */
static uint32_t tpm5[3];

static uint32_t wdog3[2];

/* CGC1 PLL2 */
uint32_t pll2[][2] = {
	{0x292c0510, 0x0}, {0x292c0518, 0x0}, {0x292c051c, 0x0},
	{0x292c0520, 0x0}, {0x292c0500, 0x0},
};

/* CGC1 PLL3 */
uint32_t pll3[][2] = {
	{0x292c0604, 0x0}, {0x292c0608, 0x0}, {0x292c060c, 0x0},
	{0x292c0610, 0x0}, {0x292c0618, 0x0}, {0x292c061c, 0x0},
	{0x292c0620, 0x0}, {0x292c0624, 0x0}, {0x292c0600, 0x0},
	{0x292c0614, 0x0},
};

/* CGC1 others */
uint32_t cgc1[][2] = {
	{0x292c0014, 0x0}, {0x292c0034, 0x0}, {0x292c0038, 0x0},
	{0x292c0108, 0x0}, {0x292c0208, 0x0}, {0x292c0700, 0x0},
	{0x292c0810, 0x0}, {0x292c0900, 0x0}, {0x292c0904, 0x0},
	{0x292c0908, 0x0}, {0x292c090c, 0x0}, {0x292c0a00, 0x0},
};

static uint32_t pcc3[61];
static uint32_t pcc4[32];

static uint32_t pcc5_0[33];
static uint32_t pcc5_1[][2] = {
	{0x2da70084, 0x0}, {0x2da70088, 0x0}, {0x2da7008c, 0x0},
	{0x2da700a0, 0x0}, {0x2da700a4, 0x0}, {0x2da700a8, 0x0},
	{0x2da700ac, 0x0}, {0x2da700b0, 0x0}, {0x2da700b4, 0x0},
	{0x2da700bc, 0x0}, {0x2da700c0, 0x0}, {0x2da700c8, 0x0},
	{0x2da700cc, 0x0}, {0x2da700d0, 0x0}, {0x2da700f0, 0x0},
	{0x2da700f4, 0x0}, {0x2da700f8, 0x0}, {0x2da70108, 0x0},
	{0x2da7010c, 0x0}, {0x2da70110, 0x0}, {0x2da70114, 0x0},
};

static uint32_t cgc2[][2] = {
	{0x2da60014, 0x0}, {0x2da60020, 0x0}, {0x2da6003c, 0x0},
	{0x2da60040, 0x0}, {0x2da60108, 0x0}, {0x2da60208, 0x0},
	{0x2da60900, 0x0}, {0x2da60904, 0x0}, {0x2da60908, 0x0},
	{0x2da60910, 0x0}, {0x2da60a00, 0x0},
};

static uint32_t pll4[][2] = {
	{0x2da60604, 0x0}, {0x2da60608, 0x0}, {0x2da6060c, 0x0},
	{0x2da60610, 0x0}, {0x2da60618, 0x0}, {0x2da6061c, 0x0},
	{0x2da60620, 0x0}, {0x2da60624, 0x0}, {0x2da60600, 0x0},
	{0x2da60614, 0x0},
};

static uint32_t lpav_sim[][2] = {
	{0x2da50000, 0x0}, {0x2da50004, 0x0}, {0x2da50008, 0x0},
	{0x2da5001c, 0x0}, {0x2da50020, 0x0}, {0x2da50024, 0x0},
	{0x2da50034, 0x0},
};

#define APD_GPIO_CTRL_NUM		2
#define LPAV_GPIO_CTRL_NUM		1
#define GPIO_CTRL_REG_NUM		8
#define GPIO_PIN_MAX_NUM	32
#define GPIO_CTX(addr, num)	\
	{.base = (addr), .pin_num = (num), }

struct gpio_ctx {
	/* gpio base */
	uintptr_t base;
	/* port control */
	uint32_t port_ctrl[GPIO_CTRL_REG_NUM];
	/* GPIO ICR, Max 32 */
	uint32_t pin_num;
	uint32_t gpio_icr[GPIO_PIN_MAX_NUM];
};

static uint32_t gpio_ctrl_offset[GPIO_CTRL_REG_NUM] = {
	 0xc, 0x10, 0x14, 0x18, 0x1c, 0x40, 0x54, 0x58
};
static struct gpio_ctx apd_gpio_ctx[APD_GPIO_CTRL_NUM] = {
	GPIO_CTX(IMX_GPIOE_BASE, 24),
	GPIO_CTX(IMX_GPIOF_BASE, 32),
};

static struct gpio_ctx lpav_gpio_ctx = GPIO_CTX(IMX_GPIOD_BASE, 24);
/* iomuxc setting */
#define IOMUXC_SECTION_NUM	8
struct iomuxc_section {
	uint32_t offset;
	uint32_t reg_num;
};

struct iomuxc_section iomuxc_sections[IOMUXC_SECTION_NUM] = {
	{.offset = IOMUXC_PTD_PCR_BASE, .reg_num = 24},
	{.offset = IOMUXC_PTE_PCR_BASE, .reg_num = 24},
	{.offset = IOMUXC_PTF_PCR_BASE, .reg_num = 32},
	{.offset = IOMUXC_PSMI_BASE0, .reg_num = 10},
	{.offset = IOMUXC_PSMI_BASE1, .reg_num = 61},
	{.offset = IOMUXC_PSMI_BASE2, .reg_num = 12},
	{.offset = IOMUXC_PSMI_BASE3, .reg_num = 20},
	{.offset = IOMUXC_PSMI_BASE4, .reg_num = 75},
};
static uint32_t iomuxc_ctx[258];

#define PORTS_NUM		3U
void apd_io_pad_off(void)
{
	unsigned int i, j;

	/* off the PTD/E/F, need to be customized based on actual user case */
	for (i = 0; i < PORTS_NUM; i++) {
		for (j = 0; j < iomuxc_sections[i].reg_num; j++) {
			mmio_write_32(iomuxc_sections[i].offset + j * 4, 0);
		}
	}

	/* disable the PTD compensation */
	mmio_write_32(IMX_SIM1_BASE + 0x48, 0x800);
}

void iomuxc_save(void)
{
	unsigned int i, j;
	unsigned int index = 0U;

	for (i = 0U; i < IOMUXC_SECTION_NUM; i++) {
		for (j = 0U; j < iomuxc_sections[i].reg_num; j++) {
			iomuxc_ctx[index++] = mmio_read_32(iomuxc_sections[i].offset + j * 4);
		}
	}

	apd_io_pad_off();
}

void iomuxc_restore(void)
{
	unsigned int i, j;
	unsigned int index = 0U;

	for (i = 0U; i < IOMUXC_SECTION_NUM; i++) {
		for (j = 0U; j < iomuxc_sections[i].reg_num; j++) {
			mmio_write_32(iomuxc_sections[i].offset + j * 4, iomuxc_ctx[index++]);
		}
	}
}

void gpio_save(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	for (i = 0U; i < port_num; i++) {
		/* save the port control setting */
		for (j = 0U; j < GPIO_CTRL_REG_NUM; j++) {
			if (j < 4U) {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
				/*
				 * clear the permission setting to read the GPIO
				 * non-secure world setting.
				 */
				mmio_write_32(ctx->base + gpio_ctrl_offset[j], 0x0);
			} else {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
			}
		}
		/* save the gpio icr setting */
		for (j = 0U; j < ctx->pin_num; j++) {
			ctx->gpio_icr[j] = mmio_read_32(ctx->base + 0x80 + j * 4);
		}

		ctx++;
	}
}

void gpio_restore(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	for (i = 0U; i < port_num; i++) {
		for (j = 0U; j < ctx->pin_num; j++)
			mmio_write_32(ctx->base + 0x80 + j * 4, ctx->gpio_icr[j]);

		for (j = 4U; j < GPIO_CTRL_REG_NUM; j++) {
			mmio_write_32(ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		/* permission config retore last */
		for (j = 0U; j < 4; j++) {
			mmio_write_32(ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		ctx++;
	}
}

void cgc1_save(void)
{
	unsigned int i;

	/* PLL2 */
	for (i = 0U; i < ARRAY_SIZE(pll2); i++) {
		pll2[i][1] = mmio_read_32(pll2[i][0]);
	}

	/* PLL3 */
	for (i = 0U; i < ARRAY_SIZE(pll3); i++) {
		pll3[i][1] = mmio_read_32(pll3[i][0]);
	}

	/* CGC1 others */
	for (i = 0U; i < ARRAY_SIZE(cgc1); i++) {
		cgc1[i][1] = mmio_read_32(cgc1[i][0]);
	}
}

void cgc1_restore(void)
{
	unsigned int i;

	/* PLL2 */
	for (i = 0U; i < ARRAY_SIZE(pll2); i++) {
		mmio_write_32(pll2[i][0], pll2[i][1]);
	}
	/* wait for PLL2 lock */
	while (!(mmio_read_32(pll2[4][0]) & BIT(24))) {
		;
	}

	/* PLL3 */
	for (i = 0U; i < 9U; i++) {
		mmio_write_32(pll3[i][0], pll3[i][1]);
	}

	/* wait for PLL3 lock */
	while (!(mmio_read_32(pll3[4][0]) & BIT(24))) {
		;
	}

	/* restore the PFDs */
	mmio_write_32(pll3[9][0], pll3[9][1] & ~(BIT(31) | BIT(23) | BIT(15) | BIT(7)));
	mmio_write_32(pll3[9][0], pll3[9][1]);

	/* wait for the PFD is stable, only need to check the enabled PFDs */
	while (!(mmio_read_32(pll3[9][0]) & PFD_VALID_MASK)) {
		;
	}

	/* CGC1 others */
	for (i = 0U; i < ARRAY_SIZE(cgc1); i++) {
		mmio_write_32(cgc1[i][0], cgc1[i][1]);
	}
}

void tpm5_save(void)
{
	tpm5[0] = mmio_read_32(IMX_TPM5_BASE + 0x10);
	tpm5[1] = mmio_read_32(IMX_TPM5_BASE + 0x18);
	tpm5[2] = mmio_read_32(IMX_TPM5_BASE + 0x20);
}

void tpm5_restore(void)
{
	mmio_write_32(IMX_TPM5_BASE + 0x10, tpm5[0]);
	mmio_write_32(IMX_TPM5_BASE + 0x18, tpm5[1]);
	mmio_write_32(IMX_TPM5_BASE + 0x20, tpm5[2]);
}

void wdog3_save(void)
{
	/* enable wdog3 clock */
	mmio_write_32(IMX_PCC3_BASE + 0xa8, 0xd2800000);

	/* save the CS & TOVAL regiter */
	wdog3[0] = mmio_read_32(IMX_WDOG3_BASE);
	wdog3[1] = mmio_read_32(IMX_WDOG3_BASE + 0x8);
}

void wdog3_restore(void)
{
	/* enable wdog3 clock */
	mmio_write_32(IMX_PCC3_BASE + 0xa8, 0xd2800000);

	/* reconfig the CS */
	mmio_write_32(IMX_WDOG3_BASE, wdog3[0]);
	/* set the tiemout value */
	mmio_write_32(IMX_WDOG3_BASE + 0x8, wdog3[1]);

	/* wait for the lock status */
	while ((mmio_read_32(IMX_WDOG3_BASE) & BIT(11))) {
		;
	}

	/* wait for the config done */
	while (!(mmio_read_32(IMX_WDOG3_BASE) & BIT(10))) {
		;
	}
}

static uint32_t lpuart_regs[4];
#define LPUART_BAUD     0x10
#define LPUART_CTRL     0x18
#define LPUART_FIFO     0x28
#define LPUART_WATER    0x2c

void lpuart_save(void)
{
	lpuart_regs[0] = mmio_read_32(IMX_LPUART5_BASE + LPUART_BAUD);
	lpuart_regs[1] = mmio_read_32(IMX_LPUART5_BASE + LPUART_FIFO);
	lpuart_regs[2] = mmio_read_32(IMX_LPUART5_BASE + LPUART_WATER);
	lpuart_regs[3] = mmio_read_32(IMX_LPUART5_BASE + LPUART_CTRL);
}

void lpuart_restore(void)
{
	mmio_write_32(IMX_LPUART5_BASE + LPUART_BAUD, lpuart_regs[0]);
	mmio_write_32(IMX_LPUART5_BASE + LPUART_FIFO, lpuart_regs[1]);
	mmio_write_32(IMX_LPUART5_BASE + LPUART_WATER, lpuart_regs[2]);
	mmio_write_32(IMX_LPUART5_BASE + LPUART_CTRL, lpuart_regs[3]);
}

bool is_lpav_owned_by_apd(void)
{
	return (mmio_read_32(0x2802b044) & BIT(7)) ? true : false;
}

void lpav_ctx_save(void)
{
	unsigned int i;
	uint32_t val;

	/* CGC2 save */
	for (i = 0U; i < ARRAY_SIZE(cgc2); i++) {
		cgc2[i][1] = mmio_read_32(cgc2[i][0]);
	}

	/* PLL4 */
	for (i = 0U; i < ARRAY_SIZE(pll4); i++) {
		pll4[i][1] = mmio_read_32(pll4[i][0]);
	}

	/* PCC5 save */
	for (i = 0U; i < ARRAY_SIZE(pcc5_0); i++) {
		val = mmio_read_32(IMX_PCC5_BASE + i * 4);
		if (val & PCC_PR) {
			pcc5_0[i] = val;
		}
	}

	for (i = 0U; i < ARRAY_SIZE(pcc5_1); i++) {
		val = mmio_read_32(pcc5_1[i][0]);
		if (val & PCC_PR) {
			pcc5_1[i][1] = val;
		}
	}

	/* LPAV SIM save */
	for (i = 0U; i < ARRAY_SIZE(lpav_sim); i++) {
		lpav_sim[i][1] = mmio_read_32(lpav_sim[i][0]);
	}

	/* Save GPIO port D */
	gpio_save(&lpav_gpio_ctx, LPAV_GPIO_CTRL_NUM);

	/* put DDR into retention */
	dram_enter_retention();
}

void lpav_ctx_restore(void)
{
	unsigned int i;

	/* PLL4 */
	for (i = 0U; i < 9U; i++) {
		mmio_write_32(pll4[i][0], pll4[i][1]);
	}

	/* wait for PLL4 lock */
	while (!(mmio_read_32(pll4[8][0]) & BIT(24))) {
		;
	}

	/* restore the PLL4 PFDs */
	mmio_write_32(pll4[9][0], pll4[9][1] & ~(BIT(31) | BIT(23) | BIT(15) | BIT(7)));
	mmio_write_32(pll4[9][0], pll4[9][1]);

	/* wait for the PFD is stable */
	while (!(mmio_read_32(pll4[9][0]) & PFD_VALID_MASK)) {
		;
	}

	/* CGC2 restore */
	for (i = 0U; i < ARRAY_SIZE(cgc2); i++) {
		mmio_write_32(cgc2[i][0], cgc2[i][1]);
	}

	/* PCC5 restore */
	for (i = 0U; i < ARRAY_SIZE(pcc5_0); i++) {
		if (pcc5_0[i] & PCC_PR) {
			mmio_write_32(IMX_PCC5_BASE + i * 4, pcc5_0[i]);
		}
	}

	for (i = 0U; i < ARRAY_SIZE(pcc5_1); i++) {
		if (pcc5_1[i][1] & PCC_PR) {
			mmio_write_32(pcc5_1[i][0], pcc5_1[i][1]);
		}
	}

	/* LPAV_SIM */
	for (i = 0U; i < ARRAY_SIZE(lpav_sim); i++) {
		mmio_write_32(lpav_sim[i][0], lpav_sim[i][1]);
	}

	gpio_restore(&lpav_gpio_ctx, LPAV_GPIO_CTRL_NUM);
	/* DDR retention exit */
	dram_exit_retention();
}

void imx_apd_ctx_save(unsigned int proc_num)
{
	unsigned int i;
	uint32_t val;

	/* enable LPUART5's clock by default */
	mmio_setbits_32(IMX_PCC3_BASE + 0xe8, BIT(30));

	/* save the gic config */
	plat_gic_save(proc_num, &imx_gicv3_ctx);

	cmc1_pmprot = mmio_read_32(IMX_CMC1_BASE + 0x18);
	cmc1_srie = mmio_read_32(IMX_CMC1_BASE + 0x8c);

	/* save the PCC3 */
	for (i = 0U; i < ARRAY_SIZE(pcc3); i++) {
		/* save the pcc if it is exist */
		val = mmio_read_32(IMX_PCC3_BASE + i * 4);
		if (val & PCC_PR) {
			pcc3[i] = val;
		}
	}

	/* save the PCC4 */
	for (i = 0U; i < ARRAY_SIZE(pcc4); i++) {
		/* save the pcc if it is exist */
		val = mmio_read_32(IMX_PCC4_BASE + i * 4);
		if (val & PCC_PR) {
			pcc4[i] = val;
		}
	}

	/* save the CGC1 */
	cgc1_save();

	wdog3_save();

	gpio_save(apd_gpio_ctx, APD_GPIO_CTRL_NUM);

	iomuxc_save();

	tpm5_save();

	lpuart_save();

	/*
	 * save the lpav ctx & put the ddr into retention
	 * if lpav master is assigned to APD domain.
	 */
	if (is_lpav_owned_by_apd()) {
		lpav_ctx_save();
	}
}

void xrdc_reinit(void)
{
	xrdc_apply_apd_config();
	xrdc_apply_lpav_config();

	xrdc_enable();
}

void s400_release_caam(void)
{
	uint32_t msg, resp;

	mmio_write_32(S400_MU_TRx(0), 0x17d70206);
	mmio_write_32(S400_MU_TRx(1), 0x7);

	do {
		resp = mmio_read_32(S400_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(S400_MU_RRx(0));
	resp = mmio_read_32(S400_MU_RRx(1));

	VERBOSE("resp %x; %x", msg, resp);
}

void imx_apd_ctx_restore(unsigned int proc_num)
{
	unsigned int i;

	/* restore the CCG1 */
	cgc1_restore();

	for (i = 0U; i < ARRAY_SIZE(pcc3); i++) {
		/* save the pcc if it is exist */
		if (pcc3[i] & PCC_PR) {
			mmio_write_32(IMX_PCC3_BASE + i * 4, pcc3[i]);
		}
	}

	for (i = 0U; i < ARRAY_SIZE(pcc4); i++) {
		if (pcc4[i] & PCC_PR) {
			mmio_write_32(IMX_PCC4_BASE + i * 4, pcc4[i]);
		}
	}

	wdog3_restore();

	iomuxc_restore();

	tpm5_restore();

	xrdc_reinit();

	/* Restore GPIO after xrdc_reinit, otherwise MSCs are invalid */
	gpio_restore(apd_gpio_ctx, APD_GPIO_CTRL_NUM);

	/* restore the gic config */
	plat_gic_restore(proc_num, &imx_gicv3_ctx);

	mmio_write_32(IMX_CMC1_BASE + 0x18, cmc1_pmprot);
	mmio_write_32(IMX_CMC1_BASE + 0x8c, cmc1_srie);

	/* enable LPUART5's clock by default */
	mmio_setbits_32(IMX_PCC3_BASE + 0xe8, BIT(30));

	/* restore the console lpuart */
	lpuart_restore();

	/* FIXME: make uart work for ATF */
	mmio_write_32(IMX_LPUART_BASE + 0x18, 0xc0000);

	/* Allow M core to reset A core */
	mmio_clrbits_32(IMX_MU0B_BASE + 0x10, BIT(2));
	/*
	 * Ask S400 to release caam to APD as it is owned by s400
	 */
	s400_release_caam();

	/* re-init the caam */
	imx8ulp_caam_init();

	/*
	 * ack the upower, seems a necessary steps, otherwise the upower can
	 * not response to the new API service call. put this just before the
	 * ddr retention exit because that the dram retention exit flow need to
	 * communicate with upower.
	 */
	upower_wait_resp();

	/*
	 * restore the lpav ctx & make ddr out of retention
	 * if lpav master is assigned to APD domain.
	 */
	if (is_lpav_owned_by_apd()) {
		lpav_ctx_restore();
	}
}

#define DGO_CTRL1	U(0xc)
#define USB_WAKEUP	U(0x44)
#define USB1_PHY_DPD_WAKEUP_EN	BIT_32(5)
#define USB0_PHY_DPD_WAKEUP_EN	BIT_32(4)
#define USB1_PHY_WAKEUP_ISO_DISABLE	BIT_32(1)
#define USB0_PHY_WAKEUP_ISO_DISABLE	BIT_32(0)

void usb_wakeup_enable(bool enable)
{
	if (enable) {
		mmio_setbits_32(IMX_SIM1_BASE + USB_WAKEUP,
				USB1_PHY_WAKEUP_ISO_DISABLE | USB0_PHY_WAKEUP_ISO_DISABLE);
		mmio_setbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		while (!(mmio_read_32(IMX_SIM1_BASE + DGO_CTRL1) & BIT(1))) {
			;
		}

		mmio_clrbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		mmio_write_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(1));

		/* Need to delay for a while to make sure the wakeup logic can work */
		udelay(500);

		mmio_setbits_32(IMX_SIM1_BASE + USB_WAKEUP,
				USB1_PHY_DPD_WAKEUP_EN | USB0_PHY_DPD_WAKEUP_EN);
		mmio_setbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		while (!(mmio_read_32(IMX_SIM1_BASE + DGO_CTRL1) & BIT(1))) {
			;
		}

		mmio_clrbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		mmio_write_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(1));
	} else {
		/*
		 * USBx_PHY_DPD_WAKEUP_EN should be cleared before USB0_PHY_WAKEUP_ISO_DISABLE
		 * to provide the correct the wake-up functionality.
		 */
		mmio_write_32(IMX_SIM1_BASE + USB_WAKEUP, USB1_PHY_WAKEUP_ISO_DISABLE |
			USB0_PHY_WAKEUP_ISO_DISABLE);
		mmio_write_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		while (!(mmio_read_32(IMX_SIM1_BASE + DGO_CTRL1) & BIT(1))) {
			;
		}

		mmio_clrbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		mmio_write_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(1));
	}
}
