/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <inttypes.h>
#include <lib/libc/errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <platform_def.h>
#include <scmi.h>

#include <upower_api.h>

#define POWER_STATE_ON	(0 << 30)
#define POWER_STATE_OFF	(1 << 30)

extern bool is_lpav_owned_by_apd(void);

enum {
	PS0 = 0,
	PS1 = 1,
	PS2 = 2,
	PS3 = 3,
	PS4 = 4,
	PS5 = 5,
	PS6 = 6,
	PS7 = 7,
	PS8 = 8,
	PS9 = 9,
	PS10 = 10,
	PS11 = 11,
	PS12 = 12,
	PS13 = 13,
	PS14 = 14,
	PS15 = 15,
	PS16 = 16,
	PS17 = 17,
	PS18 = 18,
	PS19 = 19,
};

#define SRAM_DMA1		BIT(6)
#define SRAM_FLEXSPI2		BIT(7)
#define SRAM_USB0		BIT(10)
#define SRAM_USDHC0		BIT(11)
#define SRAM_USDHC1		BIT(12)
#define SRAM_USDHC2_USB1	BIT(13)
#define SRAM_DCNANO		GENMASK_32(18, 17)
#define SRAM_EPDC		GENMASK_32(20, 19)
#define SRAM_DMA2		BIT(21)
#define SRAM_GPU2D		GENMASK_32(23, 22)
#define SRAM_GPU3D		GENMASK_32(25, 24)
#define SRAM_HIFI4		BIT(26)
#define SRAM_ISI_BUFFER		BIT(27)
#define SRAM_MIPI_CSI_FIFO	BIT(28)
#define SRAM_MIPI_DSI_FIFO	BIT(29)
#define SRAM_PXP		BIT(30)

#define SRAM_DMA0		BIT_64(33)
#define SRAM_FLEXCAN		BIT_64(34)
#define SRAM_FLEXSPI0		BIT_64(35)
#define SRAM_FLEXSPI1		BIT_64(36)

struct psw {
	char *name;
	uint32_t reg;
	int power_state;
	uint32_t count;
	int flags;
};

#define ALWAYS_ON BIT(0)

static struct psw imx8ulp_psw[] = {
	[PS6] = { .name = "PS6", .reg = PS6, .flags = ALWAYS_ON, .power_state = POWER_STATE_ON },
	[PS7] = { .name = "PS7", .reg = PS7, .power_state = POWER_STATE_OFF },
	[PS8] = { .name = "PS8", .reg = PS8, .power_state = POWER_STATE_OFF },
	[PS13] = { .name = "PS13", .reg = PS13, .power_state = POWER_STATE_OFF },
	[PS14] = { .name = "PS14", .reg = PS14, .flags = ALWAYS_ON, .power_state = POWER_STATE_OFF },
	[PS15] = { .name = "PS15", .reg = PS15, .power_state = POWER_STATE_OFF },
	[PS16] = { .name = "PS16", .reg = PS16, .flags = ALWAYS_ON, .power_state = POWER_STATE_ON },
};

struct power_domain {
	char *name;
	uint32_t reg;
	uint32_t psw_parent;
	uint32_t sram_parent;
	uint64_t bits;
	uint32_t power_state;
	bool lpav; /* belong to lpav domain */
	uint32_t sw_rst_reg; /* pcc sw reset reg offset */
};

/* The Rich OS need flow the macro */
#define IMX8ULP_PD_DMA1		0
#define IMX8ULP_PD_FLEXSPI2	1
#define IMX8ULP_PD_USB0		2
#define IMX8ULP_PD_USDHC0	3
#define IMX8ULP_PD_USDHC1	4
#define IMX8ULP_PD_USDHC2_USB1	5
#define IMX8ULP_PD_DCNANO	6
#define IMX8ULP_PD_EPDC		7
#define IMX8ULP_PD_DMA2		8
#define IMX8ULP_PD_GPU2D	9
#define IMX8ULP_PD_GPU3D	10
#define IMX8ULP_PD_HIFI4	11
#define IMX8ULP_PD_ISI		12
#define IMX8ULP_PD_MIPI_CSI	13
#define IMX8ULP_PD_MIPI_DSI	14
#define IMX8ULP_PD_PXP		15

#define IMX8ULP_PD_PS6		16
#define IMX8ULP_PD_PS7		17
#define IMX8ULP_PD_PS8		18
#define IMX8ULP_PD_PS13		19
#define IMX8ULP_PD_PS14		20
#define IMX8ULP_PD_PS15		21
#define IMX8ULP_PD_PS16		22
#define IMX8ULP_PD_MAX		23

/* LPAV peripheral PCC */
#define PCC_GPU2D	(IMX_PCC5_BASE + 0xf0)
#define PCC_GPU3D	(IMX_PCC5_BASE + 0xf4)
#define PCC_EPDC	(IMX_PCC5_BASE + 0xcc)
#define PCC_CSI		(IMX_PCC5_BASE + 0xbc)
#define PCC_PXP		(IMX_PCC5_BASE + 0xd0)

#define PCC_SW_RST	BIT(28)

#define PWR_DOMAIN(_name, _reg, _psw_parent, _sram_parent, \
		   _bits, _state, _lpav, _rst_reg) \
	{ \
		.name = _name, \
		.reg = _reg, \
		.psw_parent = _psw_parent, \
		.sram_parent = _sram_parent, \
		.bits = _bits, \
		.power_state = _state, \
		.lpav = _lpav, \
		.sw_rst_reg = _rst_reg, \
	}

static struct power_domain scmi_power_domains[] = {
	PWR_DOMAIN("DMA1", IMX8ULP_PD_DMA1, PS6, PS6, SRAM_DMA1, POWER_STATE_OFF, false, 0U),
	PWR_DOMAIN("FLEXSPI2", IMX8ULP_PD_FLEXSPI2, PS6, PS6, SRAM_FLEXSPI2, POWER_STATE_OFF, false, 0U),
	PWR_DOMAIN("USB0", IMX8ULP_PD_USB0, PS6, PS6, SRAM_USB0, POWER_STATE_OFF, false, 0U),
	PWR_DOMAIN("USDHC0", IMX8ULP_PD_USDHC0, PS6, PS6, SRAM_USDHC0, POWER_STATE_OFF, false, 0U),
	PWR_DOMAIN("USDHC1", IMX8ULP_PD_USDHC1, PS6, PS6, SRAM_USDHC1, POWER_STATE_OFF, false, 0U),
	PWR_DOMAIN("USDHC2_USB1", IMX8ULP_PD_USDHC2_USB1, PS6, PS6, SRAM_USDHC2_USB1, POWER_STATE_OFF, false, 0U),
	PWR_DOMAIN("DCNano", IMX8ULP_PD_DCNANO, PS16, PS16, SRAM_DCNANO, POWER_STATE_OFF, true, 0U),
	PWR_DOMAIN("EPDC", IMX8ULP_PD_EPDC, PS13, PS13, SRAM_EPDC, POWER_STATE_OFF, true, PCC_EPDC),
	PWR_DOMAIN("DMA2", IMX8ULP_PD_DMA2, PS16, PS16, SRAM_DMA2, POWER_STATE_OFF, true, 0U),
	PWR_DOMAIN("GPU2D", IMX8ULP_PD_GPU2D, PS16, PS16, SRAM_GPU2D, POWER_STATE_OFF, true, PCC_GPU2D),
	PWR_DOMAIN("GPU3D", IMX8ULP_PD_GPU3D, PS7, PS7, SRAM_GPU3D, POWER_STATE_OFF, true, PCC_GPU3D),
	PWR_DOMAIN("HIFI4", IMX8ULP_PD_HIFI4, PS8, PS8, SRAM_HIFI4, POWER_STATE_OFF, true, 0U),
	PWR_DOMAIN("ISI", IMX8ULP_PD_ISI, PS16, PS16, SRAM_ISI_BUFFER, POWER_STATE_OFF, true, 0U),
	PWR_DOMAIN("MIPI_CSI", IMX8ULP_PD_MIPI_CSI, PS15, PS16, SRAM_MIPI_CSI_FIFO, POWER_STATE_OFF, true, PCC_CSI),
	PWR_DOMAIN("MIPI_DSI", IMX8ULP_PD_MIPI_DSI, PS14, PS16, SRAM_MIPI_DSI_FIFO, POWER_STATE_OFF, true, 0U),
	PWR_DOMAIN("PXP", IMX8ULP_PD_PXP, PS13, PS13, SRAM_PXP | SRAM_EPDC, POWER_STATE_OFF, true, PCC_PXP)
};

size_t plat_scmi_pd_count(unsigned int agent_id __unused)
{
	return ARRAY_SIZE(scmi_power_domains);
}

const char *plat_scmi_pd_get_name(unsigned int agent_id __unused,
				  unsigned int pd_id)
{
	if (pd_id >= IMX8ULP_PD_PS6) {
		return imx8ulp_psw[pd_id - IMX8ULP_PD_PS6].name;
	}

	return scmi_power_domains[pd_id].name;
}

unsigned int plat_scmi_pd_get_state(unsigned int agent_id __unused,
				    unsigned int pd_id __unused)
{
	if (pd_id >= IMX8ULP_PD_PS6) {
		return imx8ulp_psw[pd_id - IMX8ULP_PD_PS6].power_state;
	}

	return scmi_power_domains[pd_id].power_state;
}

extern void upower_wait_resp(void);
int upwr_pwm_power(const uint32_t swton[], const uint32_t memon[], bool on)
{
	int ret_val;
	int ret;

	if (on == true) {
		ret = upwr_pwm_power_on(swton, memon, NULL);
	} else {
		ret = upwr_pwm_power_off(swton, memon, NULL);
	}

	if (ret != 0U) {
		WARN("%s failed: ret: %d, state: %x\n", __func__, ret, on);
		return ret;
	}

	upower_wait_resp();

	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		WARN("Failure %d, %s\n", ret, __func__);
		if (ret == UPWR_REQ_BUSY) {
			return -EBUSY;
		} else {
			return -EINVAL;
		}
	}

	return 0;
}

int32_t plat_scmi_pd_psw(unsigned int index, unsigned int state)
{
	uint32_t psw_parent = scmi_power_domains[index].psw_parent;
	uint32_t sram_parent = scmi_power_domains[index].sram_parent;
	uint64_t swt;
	bool on;
	int ret = 0;

	if ((imx8ulp_psw[psw_parent].flags & ALWAYS_ON) != 0U &&
	    (imx8ulp_psw[sram_parent].flags & ALWAYS_ON) != 0U) {
		return 0;
	}

	on = (state == POWER_STATE_ON) ? true : false;

	if ((imx8ulp_psw[psw_parent].flags & ALWAYS_ON) == 0U) {
		swt = 1 << imx8ulp_psw[psw_parent].reg;
		if (imx8ulp_psw[psw_parent].count == 0U) {
			if (on == false) {
				WARN("off PSW[%d] that already in off state\n", psw_parent);
				ret = -EACCES;
			} else {
				ret = upwr_pwm_power((const uint32_t *)&swt, NULL, on);
				imx8ulp_psw[psw_parent].count++;
			}
		} else {
			if (on == true) {
				imx8ulp_psw[psw_parent].count++;
			} else {
				imx8ulp_psw[psw_parent].count--;
			}

			if (imx8ulp_psw[psw_parent].count == 0U) {
				ret = upwr_pwm_power((const uint32_t *)&swt, NULL, on);
			}
		}
	}

	if (!(imx8ulp_psw[sram_parent].flags & ALWAYS_ON) && (psw_parent != sram_parent)) {
		swt = 1 << imx8ulp_psw[sram_parent].reg;
		if (imx8ulp_psw[sram_parent].count == 0U) {
			if (on == false) {
				WARN("off PSW[%d] that already in off state\n", sram_parent);
				ret = -EACCES;
			} else {
				ret = upwr_pwm_power((const uint32_t *)&swt, NULL, on);
				imx8ulp_psw[sram_parent].count++;
			}
		} else {
			if (on == true) {
				imx8ulp_psw[sram_parent].count++;
			} else {
				imx8ulp_psw[sram_parent].count--;
			}

			if (imx8ulp_psw[sram_parent].count == 0U) {
				ret = upwr_pwm_power((const uint32_t *)&swt, NULL, on);
			}
		}
	}

	return ret;
}

bool pd_allow_power_off(unsigned int pd_id)
{
	if (scmi_power_domains[pd_id].lpav) {
		if (!is_lpav_owned_by_apd()) {
			return false;
		}
	}

	return true;
}

void assert_pcc_reset(unsigned int pcc)
{
	/* if sw_rst_reg is valid, assert the pcc reset */
	if (pcc != 0U) {
		mmio_clrbits_32(pcc, PCC_SW_RST);
	}
}

int32_t plat_scmi_pd_set_state(unsigned int agent_id __unused,
			       unsigned int flags,
			       unsigned int pd_id,
			       unsigned int state)
{
	unsigned int ps_idx;
	uint64_t mem;
	bool on;
	int ret;

	if (flags != 0U || pd_id >= IMX8ULP_PD_PS6) {
		return SCMI_NOT_SUPPORTED;
	}

	ps_idx = 0;
	while (ps_idx < IMX8ULP_PD_PS6 && scmi_power_domains[ps_idx].reg != pd_id) {
		ps_idx++;
	}

	if (ps_idx == IMX8ULP_PD_PS6) {
		return SCMI_NOT_FOUND;
	}

	if (state == scmi_power_domains[ps_idx].power_state) {
		return SCMI_SUCCESS;
	}

	mem = scmi_power_domains[ps_idx].bits;
	on = (state == POWER_STATE_ON ? true : false);
	if (on == true) {
		/* Assert pcc sw reset if necessary */
		assert_pcc_reset(scmi_power_domains[ps_idx].sw_rst_reg);

		ret = plat_scmi_pd_psw(ps_idx, state);
		if (ret != 0U) {
			return SCMI_DENIED;
		}

		ret = upwr_pwm_power(NULL, (const uint32_t *)&mem, on);
		if (ret != 0U) {
			return SCMI_DENIED;
		}
	} else {
		if (!pd_allow_power_off(ps_idx)) {
			return SCMI_DENIED;
		}

		ret = upwr_pwm_power(NULL, (const uint32_t *)&mem, on);
		if (ret != 0U) {
			return SCMI_DENIED;
		}

		ret = plat_scmi_pd_psw(ps_idx, state);
		if (ret != 0U) {
			return SCMI_DENIED;
		}
	}

	scmi_power_domains[pd_id].power_state = state;

	return SCMI_SUCCESS;
}
