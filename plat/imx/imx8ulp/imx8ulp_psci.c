/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <plat_imx8.h>
#include <upower_api.h>

extern void cgc1_save(void);
extern void cgc1_restore(void);
extern void imx_apd_ctx_save(unsigned int cpu);
extern void imx_apd_ctx_restore(unsigned int cpu);
extern void usb_wakeup_enable(bool enable);
extern void upower_wait_resp(void);
extern bool is_lpav_owned_by_apd(void);
extern void apd_io_pad_off(void);
extern int upower_pmic_i2c_read(uint32_t reg_addr, uint32_t *reg_val);
extern void imx8ulp_init_scmi_server(void);

static uintptr_t secure_entrypoint;

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define RVBARADDRx(c)		(IMX_SIM1_BASE + 0x5c + 0x4 * (c))
#define WKPUx(c)		(IMX_SIM1_BASE + 0x3c + 0x4 * (c))
#define AD_COREx_LPMODE(c)	(IMX_CMC1_BASE + 0x50 + 0x4 * (c))

#define PMIC_CFG(v, m, msk)		\
	{				\
		.volt = (v),		\
		.mode = (m),		\
		.mode_msk = (msk),	\
	}

#define PAD_CFG(c, r, t)		\
	{				\
		.pad_close = (c),	\
		.pad_reset = (r),	\
		.pad_tqsleep = (t)	\
	}

#define BIAS_CFG(m, n, p, mbias)	\
	{				\
		.dombias_cfg = {	\
			.mode = (m),	\
			.rbbn = (n),	\
			.rbbp = (p),	\
		},			\
		.membias_cfg = {mbias},	\
	}

#define SWT_BOARD(swt_on, msk)	\
	{			\
		.on = (swt_on),	\
		.mask = (msk),	\
	}

#define SWT_MEM(a, p, m)	\
	{			\
		.array = (a),	\
		.perif = (p),	\
		.mask = (m),	\
	}

static int imx_pwr_set_cpu_entry(unsigned int cpu, unsigned int entry)
{
	mmio_write_32(RVBARADDRx(cpu), entry);

	/* set update bit */
	mmio_write_32(IMX_SIM1_BASE + 0x8, mmio_read_32(IMX_SIM1_BASE + 0x8) | BIT_32(24 + cpu));
	/* wait for ack */
	while (!(mmio_read_32(IMX_SIM1_BASE + 0x8) & BIT_32(26 + cpu))) {
	}

	/* clear update bit */
	mmio_write_32(IMX_SIM1_BASE + 0x8, mmio_read_32(IMX_SIM1_BASE + 0x8) & ~BIT_32(24 + cpu));
	/* clear ack bit */
	mmio_write_32(IMX_SIM1_BASE + 0x8, mmio_read_32(IMX_SIM1_BASE + 0x8) | BIT_32(26 + cpu));

	return 0;
}

static volatile uint32_t cgc1_nicclk;
int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu = MPIDR_AFFLVL0_VAL(mpidr);

	imx_pwr_set_cpu_entry(cpu, secure_entrypoint);

	/* slow down the APD NIC bus clock */
	cgc1_nicclk = mmio_read_32(IMX_CGC1_BASE + 0x34);
	mmio_clrbits_32(IMX_CGC1_BASE + 0x34, GENMASK_32(29, 28));

	mmio_write_32(IMX_CMC1_BASE + 0x18, 0x3f);
	mmio_write_32(IMX_CMC1_BASE + 0x50 + 0x4 * cpu, 0);

	/* enable wku wakeup for idle */
	mmio_write_32(IMX_SIM1_BASE + 0x3c + 0x4 * cpu, 0xffffffff);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	imx_pwr_set_cpu_entry(0, IMX_ROM_ENTRY);
	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();

	/* set APD NIC back to orignally setting */
	mmio_write_32(IMX_CGC1_BASE + 0x34, cgc1_nicclk);
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned int cpu = MPIDR_AFFLVL0_VAL(read_mpidr_el1());

	plat_gic_cpuif_disable();

	/* disable wakeup */
	mmio_write_32(WKPUx(cpu), 0);

	/* set core power mode to PD */
	mmio_write_32(AD_COREx_LPMODE(cpu), 0x3);
}

/* APD power mode config */
ps_apd_pwr_mode_cfgs_t apd_pwr_mode_cfgs = {
	[DPD_PWR_MODE] = {
		.swt_board_offs = 0x180,
		.swt_mem_offs = 0x188,
		.pmic_cfg = PMIC_CFG(0x23, 0x0, 0x2),
		.pad_cfg = PAD_CFG(0x0, 0xc, 0x01e80a02),
		.bias_cfg = BIAS_CFG(0x0, 0x2, 0x2, 0x0),
	},

	/* PD */
	[PD_PWR_MODE] = {
		.swt_board_offs = 0x170,
		.swt_mem_offs = 0x178,
		.pmic_cfg = PMIC_CFG(0x23, 0x0, 0x2),
		.pad_cfg = PAD_CFG(0x0, 0xc, 0x01e80a00),
		.bias_cfg = BIAS_CFG(0x0, 0x2, 0x2, 0x0),
	},

	[ADMA_PWR_MODE] = {
		.swt_board_offs = 0x120,
		.swt_mem_offs = 0x128,
		.pmic_cfg = PMIC_CFG(0x23, 0x0, 0x2),
		.pad_cfg = PAD_CFG(0x0, 0x0, 0x0deb7a00),
		.bias_cfg = BIAS_CFG(0x2, 0x2, 0x2, 0x0),
	},

	[ACT_PWR_MODE] = {
		.swt_board_offs = 0x110,
		.swt_mem_offs = 0x118,
		.pmic_cfg = PMIC_CFG(0x23, 0x0, 0x2),
		.pad_cfg = PAD_CFG(0x0, 0x0, 0x0deb7a00),
		.bias_cfg = BIAS_CFG(0x2, 0x2, 0x2, 0x0),
	},
};

/* APD power switch config */
ps_apd_swt_cfgs_t apd_swt_cfgs = {
	[DPD_PWR_MODE] = {
		.swt_board[0] = SWT_BOARD(0x0, 0x1fffc),
		.swt_mem[0] = SWT_MEM(0x0, 0x0, 0x1ffff),
		.swt_mem[1] = SWT_MEM(0x003fffff, 0x003fffff, 0x0),
	},

	[PD_PWR_MODE] = {
		.swt_board[0] = SWT_BOARD(0x0, 0x00001fffc),
		.swt_mem[0] = SWT_MEM(0x00010c00, 0x0, 0x1ffff),
		.swt_mem[1] = SWT_MEM(0x003fffff, 0x003f0000, 0x0),
	},

	[ADMA_PWR_MODE] = {
		.swt_board[0] = SWT_BOARD(0x15f74, 0x15f74),
		.swt_mem[0] = SWT_MEM(0x0001fffd, 0x0001fffd, 0x1ffff),
		.swt_mem[1] = SWT_MEM(0x003fffff, 0x003fffff, 0x0),
	},

	[ACT_PWR_MODE] = {
		.swt_board[0] = SWT_BOARD(0x15f74, 0x15f74),
		.swt_mem[0] = SWT_MEM(0x0001fffd, 0x0001fffd, 0x1ffff),
		.swt_mem[1] = SWT_MEM(0x003fffff, 0x003fffff, 0x0),
	},
};

/* PMIC config for power down, LDO1 should be OFF */
ps_apd_pmic_reg_data_cfgs_t pd_pmic_reg_cfgs = {
	[0] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = PD_PWR_MODE,
		.i2c_addr = 0x30,
		.i2c_data = 0x9c,
	},
	[1] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = PD_PWR_MODE,
		.i2c_addr = 0x22,
		.i2c_data = 0xb,
	},
	[2] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = ACT_PWR_MODE,
		.i2c_addr = 0x30,
		.i2c_data = 0x9d,
	},
	[3] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = ACT_PWR_MODE,
		.i2c_addr = 0x22,
		.i2c_data = 0x28,
	},
};

/* PMIC config for deep power down, BUCK3 should be OFF */
ps_apd_pmic_reg_data_cfgs_t dpd_pmic_reg_cfgs = {
	[0] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = DPD_PWR_MODE,
		.i2c_addr = 0x21,
		.i2c_data = 0x78,
	},
	[1] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = DPD_PWR_MODE,
		.i2c_addr = 0x30,
		.i2c_data = 0x9c,
	},
	[2] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = ACT_PWR_MODE,
		.i2c_addr = 0x21,
		.i2c_data = 0x79,
	},
	[3] = {
		.tag = PMIC_REG_VALID_TAG,
		.power_mode = ACT_PWR_MODE,
		.i2c_addr = 0x30,
		.i2c_data = 0x9d,
	},
};

struct ps_pwr_mode_cfg_t *pwr_sys_cfg = (struct ps_pwr_mode_cfg_t *)UPWR_DRAM_SHARED_BASE_ADDR;

void imx_set_pwr_mode_cfg(abs_pwr_mode_t mode)
{
	uint32_t volt;

	if (mode >= NUM_PWR_MODES) {
		return;
	}

	/* apd power mode config */
	memcpy(&pwr_sys_cfg->ps_apd_pwr_mode_cfg[mode], &apd_pwr_mode_cfgs[mode],
		 sizeof(struct ps_apd_pwr_mode_cfg_t));

	/* apd power switch config */
	memcpy(&pwr_sys_cfg->ps_apd_swt_cfg[mode], &apd_swt_cfgs[mode], sizeof(swt_config_t));

	/*
	 * BUCK3 & LDO1 can only be shutdown when LPAV is owned by APD side
	 * otherwise RTD side is responsible to control them in low power mode.
	 */
	if (is_lpav_owned_by_apd()) {
		/* power off the BUCK3 in DPD mode */
		if (mode == DPD_PWR_MODE) {
			memcpy(&pwr_sys_cfg->ps_apd_pmic_reg_data_cfg, &dpd_pmic_reg_cfgs,
				 sizeof(ps_apd_pmic_reg_data_cfgs_t));
		/* LDO1 should be power off in PD mode */
		} else if (mode == PD_PWR_MODE) {
			/* overwrite the buck3 voltage setting in active mode */
			upower_pmic_i2c_read(0x22, &volt);
			pd_pmic_reg_cfgs[3].i2c_data = volt;
			memcpy(&pwr_sys_cfg->ps_apd_pmic_reg_data_cfg, &pd_pmic_reg_cfgs,
				 sizeof(ps_apd_pmic_reg_data_cfgs_t));
		}
	}
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int cpu = MPIDR_AFFLVL0_VAL(read_mpidr_el1());

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		imx_pwr_set_cpu_entry(cpu, secure_entrypoint);
		/* core put into power down */
		mmio_write_32(IMX_CMC1_BASE + 0x50 + 0x4 * cpu, 0x3);
		/* FIXME config wakeup interrupt in WKPU */
		mmio_write_32(IMX_SIM1_BASE + 0x3c + 0x4 * cpu, 0x7fffffe3);
	} else {
		/* for core standby/retention mode */
		mmio_write_32(IMX_CMC1_BASE + 0x50 + 0x4 * cpu, 0x1);
		mmio_write_32(IMX_SIM1_BASE + 0x3c + 0x4 * cpu, 0x7fffffe3);
		dsb();
		write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);
		isb();
	}

	if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
		/*
		 * just for sleep mode for now, need to update to
		 * support more modes, same for suspend finish call back.
		 */
		mmio_write_32(IMX_CMC1_BASE + 0x10, 0x1);
		mmio_write_32(IMX_CMC1_BASE + 0x20, 0x1);

	} else if (is_local_state_off(CLUSTER_PWR_STATE(target_state))) {
		/*
		 * for cluster off state, put cluster into power down mode,
		 * config the cluster clock to be off.
		 */
		mmio_write_32(IMX_CMC1_BASE + 0x10, 0x7);
		mmio_write_32(IMX_CMC1_BASE + 0x20, 0xf);
	}

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		/*
		 * low power mode config info used by upower
		 * to do low power mode transition.
		 */
		imx_set_pwr_mode_cfg(ADMA_PWR_MODE);
		imx_set_pwr_mode_cfg(ACT_PWR_MODE);
		imx_set_pwr_mode_cfg(PD_PWR_MODE);

		/* clear the upower wakeup */
		upwr_xcp_set_rtd_apd_llwu(APD_DOMAIN, 0, NULL);
		upower_wait_resp();

		/* enable the USB wakeup */
		usb_wakeup_enable(true);

		/* config the WUU to enabled the wakeup source */
		mmio_write_32(IMX_PCC3_BASE + 0x98, 0xc0800000);

		/* !!! clear all the pad wakeup pending event */
		mmio_write_32(IMX_WUU1_BASE + 0x20, 0xffffffff);

		/* enable upower usb phy wakeup by default */
		mmio_setbits_32(IMX_WUU1_BASE + 0x18, BIT(4) | BIT(1) | BIT(0));

		/* enabled all pad wakeup by default */
		mmio_write_32(IMX_WUU1_BASE + 0x8, 0xffffffff);

		/* save the AD domain context before entering PD mode */
		imx_apd_ctx_save(cpu);
	}
}

#define DRAM_LPM_STATUS		U(0x2802b004)
void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	unsigned int cpu = MPIDR_AFFLVL0_VAL(read_mpidr_el1());

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		/* restore the ap domain context */
		imx_apd_ctx_restore(cpu);

		/* clear the upower wakeup */
		upwr_xcp_set_rtd_apd_llwu(APD_DOMAIN, 0, NULL);
		upower_wait_resp();

		/* disable all pad wakeup */
		mmio_write_32(IMX_WUU1_BASE + 0x8, 0x0);

		/* clear all the pad wakeup pending event */
		mmio_write_32(IMX_WUU1_BASE + 0x20, 0xffffffff);

		/*
		 * disable the usb wakeup after resume to make sure the pending
		 * usb wakeup in WUU can be cleared successfully, otherwise,
		 * APD will resume failed in next PD mode.
		 */
		usb_wakeup_enable(false);

		/* re-init the SCMI channel */
		imx8ulp_init_scmi_server();
	}

	/*
	 * wait for DDR is ready when DDR is under the RTD
	 * side control for power saving
	 */
	while (mmio_read_32(DRAM_LPM_STATUS) != 0) {
		;
	}

	/*
	 * when resume from low power mode, need to delay for a while
	 * before access the CMC register.
	 */
	udelay(5);

	/* clear cluster's LPM setting. */
	mmio_write_32(IMX_CMC1_BASE + 0x20, 0x0);
	mmio_write_32(IMX_CMC1_BASE + 0x10, 0x0);

	/* clear core's LPM setting */
	mmio_write_32(IMX_CMC1_BASE + 0x50 + 0x4 * cpu, 0x0);
	mmio_write_32(IMX_SIM1_BASE + 0x3c + 0x4 * cpu, 0x0);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		imx_pwr_set_cpu_entry(0, IMX_ROM_ENTRY);
		plat_gic_cpuif_enable();
	} else {
		dsb();
		write_scr_el3(read_scr_el3() & (~SCR_FIQ_BIT));
		isb();
	}
}

void __dead2 imx8ulp_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1) {
		wfi();
	}
}

void __dead2 imx8ulp_system_reset(void)
{
	imx_pwr_set_cpu_entry(0, IMX_ROM_ENTRY);

	/* Write invalid command to WDOG CNT to trigger reset */
	mmio_write_32(IMX_WDOG3_BASE + 0x4, 0x12345678);

	while (true) {
		wfi();
	}
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pwr_type == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	/* No power down state support */
	if (pwr_type == PSTATE_TYPE_POWERDOWN) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_POWER_DOWN_OFF_STATE;
	}
}

void __dead2 imx_system_off(void)
{
	unsigned int i;

	/* config the all the core into OFF mode and IRQ masked. */
	for (i = 0U; i < PLATFORM_CORE_COUNT; i++) {
		/* disable wakeup from wkpu */
		mmio_write_32(WKPUx(i), 0x0);

		/* reset the core reset entry to 0x1000 */
		imx_pwr_set_cpu_entry(i, 0x1000);

		/* config the core power mode to off */
		mmio_write_32(AD_COREx_LPMODE(i), 0x3);
	}

	plat_gic_cpuif_disable();

	/* power off all the pad */
	apd_io_pad_off();

	/* Config the power mode info for entering DPD mode and ACT mode */
	imx_set_pwr_mode_cfg(ADMA_PWR_MODE);
	imx_set_pwr_mode_cfg(ACT_PWR_MODE);
	imx_set_pwr_mode_cfg(DPD_PWR_MODE);

	/* Set the APD domain into DPD mode */
	mmio_write_32(IMX_CMC1_BASE + 0x10, 0x7);
	mmio_write_32(IMX_CMC1_BASE + 0x20, 0x1f);

	/* make sure no pending upower wakeup */
	upwr_xcp_set_rtd_apd_llwu(APD_DOMAIN, 0, NULL);
	upower_wait_resp();

	/* enable the upower wakeup from wuu, act as APD boot up method  */
	mmio_write_32(IMX_PCC3_BASE + 0x98, 0xc0800000);
	mmio_setbits_32(IMX_WUU1_BASE + 0x18, BIT(4));

	/* make sure no pad wakeup event is pending */
	mmio_write_32(IMX_WUU1_BASE + 0x20, 0xffffffff);

	wfi();

	ERROR("power off failed.\n");
	panic();
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.system_off = imx_system_off,
	.system_reset = imx8ulp_system_reset,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.validate_power_state = imx_validate_power_state,
	.pwr_domain_pwr_down_wfi = imx8ulp_pwr_domain_pwr_down_wfi,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	secure_entrypoint = sec_entrypoint;
	imx_pwr_set_cpu_entry(0, sec_entrypoint);
	*psci_ops = &imx_plat_psci_ops;

	mmio_write_32(IMX_CMC1_BASE + 0x18, 0x3f);
	mmio_write_32(IMX_SIM1_BASE + 0x3c, 0xffffffff);

	return 0;
}
