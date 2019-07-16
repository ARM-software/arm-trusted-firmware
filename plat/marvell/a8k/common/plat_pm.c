/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <drivers/marvell/cache_llc.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <armada_common.h>
#include <marvell_pm.h>
#include <mss_pm_ipc.h>
#include <plat_marvell.h>
#include <plat_pm_trace.h>

#define MVEBU_PRIVATE_UID_REG		0x30
#define MVEBU_RFU_GLOBL_SW_RST		0x84
#define MVEBU_CCU_RVBAR(cpu)		(MVEBU_REGS_BASE + 0x640 + (cpu * 4))
#define MVEBU_CCU_CPU_UN_RESET(cpu)	(MVEBU_REGS_BASE + 0x650 + (cpu * 4))

#define MPIDR_CPU_GET(mpidr)		((mpidr) & MPIDR_CPU_MASK)
#define MPIDR_CLUSTER_GET(mpidr)	MPIDR_AFFLVL1_VAL((mpidr))

#define MVEBU_GPIO_MASK(index)		(1 << (index % 32))
#define MVEBU_MPP_MASK(index)		(0xF << (4 * (index % 8)))
#define MVEBU_GPIO_VALUE(index, value)	(value << (index % 32))

#define MVEBU_USER_CMD_0_REG		(MVEBU_DRAM_MAC_BASE + 0x20)
#define MVEBU_USER_CMD_CH0_OFFSET	28
#define MVEBU_USER_CMD_CH0_MASK		(1 << MVEBU_USER_CMD_CH0_OFFSET)
#define MVEBU_USER_CMD_CH0_EN		(1 << MVEBU_USER_CMD_CH0_OFFSET)
#define MVEBU_USER_CMD_CS_OFFSET	24
#define MVEBU_USER_CMD_CS_MASK		(0xF << MVEBU_USER_CMD_CS_OFFSET)
#define MVEBU_USER_CMD_CS_ALL		(0xF << MVEBU_USER_CMD_CS_OFFSET)
#define MVEBU_USER_CMD_SR_OFFSET	6
#define MVEBU_USER_CMD_SR_MASK		(0x3 << MVEBU_USER_CMD_SR_OFFSET)
#define MVEBU_USER_CMD_SR_ENTER		(0x1 << MVEBU_USER_CMD_SR_OFFSET)
#define MVEBU_MC_PWR_CTRL_REG		(MVEBU_DRAM_MAC_BASE + 0x54)
#define MVEBU_MC_AC_ON_DLY_OFFSET	8
#define MVEBU_MC_AC_ON_DLY_MASK		(0xF << MVEBU_MC_AC_ON_DLY_OFFSET)
#define MVEBU_MC_AC_ON_DLY_DEF_VAR	(8 << MVEBU_MC_AC_ON_DLY_OFFSET)
#define MVEBU_MC_AC_OFF_DLY_OFFSET	4
#define MVEBU_MC_AC_OFF_DLY_MASK	(0xF << MVEBU_MC_AC_OFF_DLY_OFFSET)
#define MVEBU_MC_AC_OFF_DLY_DEF_VAR	(0xC << MVEBU_MC_AC_OFF_DLY_OFFSET)
#define MVEBU_MC_PHY_AUTO_OFF_OFFSET	0
#define MVEBU_MC_PHY_AUTO_OFF_MASK	(1 << MVEBU_MC_PHY_AUTO_OFF_OFFSET)
#define MVEBU_MC_PHY_AUTO_OFF_EN	(1 << MVEBU_MC_PHY_AUTO_OFF_OFFSET)

/* this lock synchronize AP multiple cores execution with MSS */
DEFINE_BAKERY_LOCK(pm_sys_lock);

/* Weak definitions may be overridden in specific board */
#pragma weak plat_marvell_get_pm_cfg

/* AP806 CPU power down /power up definitions */
enum CPU_ID {
	CPU0,
	CPU1,
	CPU2,
	CPU3
};

#define REG_WR_VALIDATE_TIMEOUT		(2000)

#define FEATURE_DISABLE_STATUS_REG			\
			(MVEBU_REGS_BASE + 0x6F8230)
#define FEATURE_DISABLE_STATUS_CPU_CLUSTER_OFFSET	4
#define FEATURE_DISABLE_STATUS_CPU_CLUSTER_MASK		\
			(0x1 << FEATURE_DISABLE_STATUS_CPU_CLUSTER_OFFSET)

#ifdef MVEBU_SOC_AP807
	#define PWRC_CPUN_CR_PWR_DN_RQ_OFFSET		1
	#define PWRC_CPUN_CR_LDO_BYPASS_RDY_OFFSET	0
#else
	#define PWRC_CPUN_CR_PWR_DN_RQ_OFFSET		0
	#define PWRC_CPUN_CR_LDO_BYPASS_RDY_OFFSET	31
#endif

#define PWRC_CPUN_CR_REG(cpu_id)		\
			(MVEBU_REGS_BASE + 0x680000 + (cpu_id * 0x10))
#define PWRC_CPUN_CR_PWR_DN_RQ_MASK		\
			(0x1 << PWRC_CPUN_CR_PWR_DN_RQ_OFFSET)
#define PWRC_CPUN_CR_ISO_ENABLE_OFFSET		16
#define PWRC_CPUN_CR_ISO_ENABLE_MASK		\
			(0x1 << PWRC_CPUN_CR_ISO_ENABLE_OFFSET)
#define PWRC_CPUN_CR_LDO_BYPASS_RDY_MASK	\
			(0x1U << PWRC_CPUN_CR_LDO_BYPASS_RDY_OFFSET)

#define CCU_B_PRCRN_REG(cpu_id)			\
			(MVEBU_REGS_BASE + 0x1A50 + \
			((cpu_id / 2) * (0x400)) + ((cpu_id % 2) * 4))
#define CCU_B_PRCRN_CPUPORESET_STATIC_OFFSET	0
#define CCU_B_PRCRN_CPUPORESET_STATIC_MASK	\
			(0x1 << CCU_B_PRCRN_CPUPORESET_STATIC_OFFSET)

/* power switch fingers */
#define AP807_PWRC_LDO_CR0_REG			\
			(MVEBU_REGS_BASE + 0x680000 + 0x100)
#define AP807_PWRC_LDO_CR0_OFFSET		16
#define AP807_PWRC_LDO_CR0_MASK			\
			(0xff << AP807_PWRC_LDO_CR0_OFFSET)
#define AP807_PWRC_LDO_CR0_VAL			0xfc

/*
 * Power down CPU:
 * Used to reduce power consumption, and avoid SoC unnecessary temperature rise.
 */
static int plat_marvell_cpu_powerdown(int cpu_id)
{
	uint32_t	reg_val;
	int		exit_loop = REG_WR_VALIDATE_TIMEOUT;

	INFO("Powering down CPU%d\n", cpu_id);

	/* 1. Isolation enable */
	reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
	reg_val |= 0x1 << PWRC_CPUN_CR_ISO_ENABLE_OFFSET;
	mmio_write_32(PWRC_CPUN_CR_REG(cpu_id), reg_val);

	/* 2. Read and check Isolation enabled - verify bit set to 1 */
	do {
		reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
		exit_loop--;
	} while (!(reg_val & (0x1 << PWRC_CPUN_CR_ISO_ENABLE_OFFSET)) &&
		 exit_loop > 0);

	/* 3. Switch off CPU power */
	reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
	reg_val &= ~PWRC_CPUN_CR_PWR_DN_RQ_MASK;
	mmio_write_32(PWRC_CPUN_CR_REG(cpu_id), reg_val);

	/* 4. Read and check Switch Off - verify bit set to 0 */
	exit_loop = REG_WR_VALIDATE_TIMEOUT;
	do {
		reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
		exit_loop--;
	} while (reg_val & PWRC_CPUN_CR_PWR_DN_RQ_MASK && exit_loop > 0);

	if (exit_loop <= 0)
		goto cpu_poweroff_error;

	/* 5. De-Assert power ready */
	reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
	reg_val &= ~PWRC_CPUN_CR_LDO_BYPASS_RDY_MASK;
	mmio_write_32(PWRC_CPUN_CR_REG(cpu_id), reg_val);

	/* 6. Assert CPU POR reset */
	reg_val = mmio_read_32(CCU_B_PRCRN_REG(cpu_id));
	reg_val &= ~CCU_B_PRCRN_CPUPORESET_STATIC_MASK;
	mmio_write_32(CCU_B_PRCRN_REG(cpu_id), reg_val);

	/* 7. Read and poll on Validate the CPU is out of reset */
	exit_loop = REG_WR_VALIDATE_TIMEOUT;
	do {
		reg_val = mmio_read_32(CCU_B_PRCRN_REG(cpu_id));
		exit_loop--;
	} while (reg_val & CCU_B_PRCRN_CPUPORESET_STATIC_MASK && exit_loop > 0);

	if (exit_loop <= 0)
		goto cpu_poweroff_error;

	INFO("Successfully powered down CPU%d\n", cpu_id);

	return 0;

cpu_poweroff_error:
	ERROR("ERROR: Can't power down CPU%d\n", cpu_id);
	return -1;
}

/*
 * Power down CPUs 1-3 at early boot stage,
 * to reduce power consumption and SoC temperature.
 * This is triggered by BLE prior to DDR initialization.
 *
 * Note:
 * All CPUs will be powered up by plat_marvell_cpu_powerup on Linux boot stage,
 * which is triggered by PSCI ops (pwr_domain_on).
 */
int plat_marvell_early_cpu_powerdown(void)
{
	uint32_t cpu_cluster_status =
		mmio_read_32(FEATURE_DISABLE_STATUS_REG) &
			     FEATURE_DISABLE_STATUS_CPU_CLUSTER_MASK;
	/* if cpu_cluster_status bit is set,
	 * that means we have only single cluster
	 */
	int cluster_count = cpu_cluster_status ? 1 : 2;

	INFO("Powering off unused CPUs\n");

	/* CPU1 is in AP806 cluster-0, which always exists, so power it down */
	if (plat_marvell_cpu_powerdown(CPU1) == -1)
		return -1;

	/*
	 * CPU2-3 are in AP806 2nd cluster (cluster-1),
	 * which doesn't exists in dual-core systems.
	 * so need to check if we have dual-core (single cluster)
	 * or quad-code (2 clusters)
	 */
	if (cluster_count == 2) {
		/* CPU2-3 are part of 2nd cluster */
		if (plat_marvell_cpu_powerdown(CPU2) == -1)
			return -1;
		if (plat_marvell_cpu_powerdown(CPU3) == -1)
			return -1;
	}

	return 0;
}

/*
 * Power up CPU - part of Linux boot stage
 */
static int plat_marvell_cpu_powerup(u_register_t mpidr)
{
	uint32_t	reg_val;
	int	cpu_id = MPIDR_CPU_GET(mpidr),
		cluster = MPIDR_CLUSTER_GET(mpidr);
	int	exit_loop = REG_WR_VALIDATE_TIMEOUT;

	/* calculate absolute CPU ID */
	cpu_id = cluster * PLAT_MARVELL_CLUSTER_CORE_COUNT + cpu_id;

	INFO("Powering on CPU%d\n", cpu_id);

#ifdef MVEBU_SOC_AP807
	/* Activate 2 power switch fingers */
	reg_val = mmio_read_32(AP807_PWRC_LDO_CR0_REG);
	reg_val &= ~(AP807_PWRC_LDO_CR0_MASK);
	reg_val |= (AP807_PWRC_LDO_CR0_VAL << AP807_PWRC_LDO_CR0_OFFSET);
	mmio_write_32(AP807_PWRC_LDO_CR0_REG, reg_val);
	udelay(100);
#endif

	/* 1. Switch CPU power ON */
	reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
	reg_val |= 0x1 << PWRC_CPUN_CR_PWR_DN_RQ_OFFSET;
	mmio_write_32(PWRC_CPUN_CR_REG(cpu_id), reg_val);

	/* 2. Wait for CPU on, up to 100 uSec: */
	udelay(100);

	/* 3. Assert power ready */
	reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
	reg_val |= 0x1U << PWRC_CPUN_CR_LDO_BYPASS_RDY_OFFSET;
	mmio_write_32(PWRC_CPUN_CR_REG(cpu_id), reg_val);

	/* 4. Read & Validate power ready
	 * used in order to generate 16 Host CPU cycles
	 */
	do {
		reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
		exit_loop--;
	} while (!(reg_val & (0x1U << PWRC_CPUN_CR_LDO_BYPASS_RDY_OFFSET)) &&
		 exit_loop > 0);

	if (exit_loop <= 0)
		goto cpu_poweron_error;

	/* 5. Isolation disable */
	reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
	reg_val &= ~PWRC_CPUN_CR_ISO_ENABLE_MASK;
	mmio_write_32(PWRC_CPUN_CR_REG(cpu_id), reg_val);

	/* 6. Read and check Isolation enabled - verify bit set to 1 */
	exit_loop = REG_WR_VALIDATE_TIMEOUT;
	do {
		reg_val = mmio_read_32(PWRC_CPUN_CR_REG(cpu_id));
		exit_loop--;
	} while ((reg_val & (0x1 << PWRC_CPUN_CR_ISO_ENABLE_OFFSET)) &&
		 exit_loop > 0);

	/* 7. De Assert CPU POR reset & Core reset */
	reg_val = mmio_read_32(CCU_B_PRCRN_REG(cpu_id));
	reg_val |= 0x1 << CCU_B_PRCRN_CPUPORESET_STATIC_OFFSET;
	mmio_write_32(CCU_B_PRCRN_REG(cpu_id), reg_val);

	/* 8. Read & Validate CPU POR reset */
	exit_loop = REG_WR_VALIDATE_TIMEOUT;
	do {
		reg_val = mmio_read_32(CCU_B_PRCRN_REG(cpu_id));
		exit_loop--;
	} while (!(reg_val & (0x1 << CCU_B_PRCRN_CPUPORESET_STATIC_OFFSET)) &&
		 exit_loop > 0);

	if (exit_loop <= 0)
		goto cpu_poweron_error;

	INFO("Successfully powered on CPU%d\n", cpu_id);

	return 0;

cpu_poweron_error:
	ERROR("ERROR: Can't power up CPU%d\n", cpu_id);
	return -1;
}

static int plat_marvell_cpu_on(u_register_t mpidr)
{
	int cpu_id;
	int cluster;

	/* Set barierr */
	dsbsy();

	/* Get cpu number - use CPU ID */
	cpu_id =  MPIDR_CPU_GET(mpidr);

	/* Get cluster number - use affinity level 1 */
	cluster = MPIDR_CLUSTER_GET(mpidr);

	/* Set CPU private UID */
	mmio_write_32(MVEBU_REGS_BASE + MVEBU_PRIVATE_UID_REG, cluster + 0x4);

	/* Set the cpu start address to BL1 entry point (align to 0x10000) */
	mmio_write_32(MVEBU_CCU_RVBAR(cpu_id),
		      PLAT_MARVELL_CPU_ENTRY_ADDR >> 16);

	/* Get the cpu out of reset */
	mmio_write_32(MVEBU_CCU_CPU_UN_RESET(cpu_id), 0x10001);

	return 0;
}

/*****************************************************************************
 * A8K handler called to check the validity of the power state
 * parameter.
 *****************************************************************************
 */
static int a8k_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != MARVELL_PWR_LVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MARVELL_PWR_LVL0] =
					MARVELL_LOCAL_STATE_RET;
	} else {
		for (i = MARVELL_PWR_LVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					MARVELL_LOCAL_STATE_OFF;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*****************************************************************************
 * A8K handler called when a CPU is about to enter standby.
 *****************************************************************************
 */
static void a8k_cpu_standby(plat_local_state_t cpu_state)
{
	if (!is_pm_fw_running()) {
		ERROR("%s: needs to be implemented\n", __func__);
		panic();
	}
}

/*****************************************************************************
 * A8K handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 *****************************************************************************
 */
static int a8k_pwr_domain_on(u_register_t mpidr)
{
	/* Power up CPU (CPUs 1-3 are powered off at start of BLE) */
	plat_marvell_cpu_powerup(mpidr);

	if (is_pm_fw_running()) {
		unsigned int target =
				((mpidr & 0xFF) + (((mpidr >> 8) & 0xFF) * 2));

		/*
		 * pm system synchronization - used to synchronize
		 * multiple core access to MSS
		 */
		bakery_lock_get(&pm_sys_lock);

		/* send CPU ON IPC Message to MSS */
		mss_pm_ipc_msg_send(target, PM_IPC_MSG_CPU_ON, 0);

		/* trigger IPC message to MSS */
		mss_pm_ipc_msg_trigger();

		/* pm system synchronization */
		bakery_lock_release(&pm_sys_lock);

		/* trace message */
		PM_TRACE(TRACE_PWR_DOMAIN_ON | target);
	} else {
		/* proprietary CPU ON exection flow */
		plat_marvell_cpu_on(mpidr);
	}

	return 0;
}

/*****************************************************************************
 * A8K handler called to validate the entry point.
 *****************************************************************************
 */
static int a8k_validate_ns_entrypoint(uintptr_t entrypoint)
{
	return PSCI_E_SUCCESS;
}

/*****************************************************************************
 * A8K handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 *****************************************************************************
 */
static void a8k_pwr_domain_off(const psci_power_state_t *target_state)
{
	if (is_pm_fw_running()) {
		unsigned int idx = plat_my_core_pos();

		/* Prevent interrupts from spuriously waking up this cpu */
		gicv2_cpuif_disable();

		/* pm system synchronization - used to synchronize multiple
		 * core access to MSS
		 */
		bakery_lock_get(&pm_sys_lock);

		/* send CPU OFF IPC Message to MSS */
		mss_pm_ipc_msg_send(idx, PM_IPC_MSG_CPU_OFF, target_state);

		/* trigger IPC message to MSS */
		mss_pm_ipc_msg_trigger();

		/* pm system synchronization */
		bakery_lock_release(&pm_sys_lock);

		/* trace message */
		PM_TRACE(TRACE_PWR_DOMAIN_OFF);
	} else {
		INFO("%s: is not supported without SCP\n", __func__);
	}
}

/* Get PM config to power off the SoC */
void *plat_marvell_get_pm_cfg(void)
{
	return NULL;
}

/*
 * This function should be called on restore from
 * "suspend to RAM" state when the execution flow
 * has to bypass BootROM image to RAM copy and speed up
 * the system recovery
 *
 */
static void plat_marvell_exit_bootrom(void)
{
	marvell_exit_bootrom(PLAT_MARVELL_TRUSTED_ROM_BASE);
}

/*
 * Prepare for the power off of the system via GPIO
 */
static void plat_marvell_power_off_gpio(struct power_off_method *pm_cfg,
					register_t *gpio_addr,
					register_t *gpio_data)
{
	unsigned int gpio;
	unsigned int idx;
	unsigned int shift;
	unsigned int reg;
	unsigned int addr;
	gpio_info_t *info;
	unsigned int tog_bits;

	assert((pm_cfg->cfg.gpio.pin_count < PMIC_GPIO_MAX_NUMBER) &&
	       (pm_cfg->cfg.gpio.step_count < PMIC_GPIO_MAX_TOGGLE_STEP));

	/* Prepare GPIOs for PMIC */
	for (gpio = 0; gpio < pm_cfg->cfg.gpio.pin_count; gpio++) {
		info = &pm_cfg->cfg.gpio.info[gpio];
		/* Set PMIC GPIO to output mode */
		reg = mmio_read_32(MVEBU_CP_GPIO_DATA_OUT_EN(
				   info->cp_index, info->gpio_index));
		mmio_write_32(MVEBU_CP_GPIO_DATA_OUT_EN(
			      info->cp_index, info->gpio_index),
			      reg & ~MVEBU_GPIO_MASK(info->gpio_index));

		/* Set the appropriate MPP to GPIO mode */
		reg = mmio_read_32(MVEBU_PM_MPP_REGS(info->cp_index,
						     info->gpio_index));
		mmio_write_32(MVEBU_PM_MPP_REGS(info->cp_index,
						info->gpio_index),
			reg & ~MVEBU_MPP_MASK(info->gpio_index));
	}

	/* Wait for MPP & GPIO pre-configurations done */
	mdelay(pm_cfg->cfg.gpio.delay_ms);

	/* Toggle the GPIO values, and leave final step to be triggered
	 * after  DDR self-refresh is enabled
	 */
	for (idx = 0; idx < pm_cfg->cfg.gpio.step_count; idx++) {
		tog_bits = pm_cfg->cfg.gpio.seq[idx];

		/* The GPIOs must be within same GPIO register,
		 * thus could get the original value by first GPIO
		 */
		info = &pm_cfg->cfg.gpio.info[0];
		reg = mmio_read_32(MVEBU_CP_GPIO_DATA_OUT(
				   info->cp_index, info->gpio_index));
		addr = MVEBU_CP_GPIO_DATA_OUT(info->cp_index, info->gpio_index);

		for (gpio = 0; gpio < pm_cfg->cfg.gpio.pin_count; gpio++) {
			shift = pm_cfg->cfg.gpio.info[gpio].gpio_index % 32;
			if (GPIO_LOW == (tog_bits & (1 << gpio)))
				reg &= ~(1 << shift);
			else
				reg |= (1 << shift);
		}

		/* Set the GPIO register, for last step just store
		 * register address and values to system registers
		 */
		if (idx < pm_cfg->cfg.gpio.step_count - 1) {
			mmio_write_32(MVEBU_CP_GPIO_DATA_OUT(
				      info->cp_index, info->gpio_index), reg);
			mdelay(pm_cfg->cfg.gpio.delay_ms);
		} else {
			/* Save GPIO register and address values for
			 * finishing the power down operation later
			 */
			*gpio_addr = addr;
			*gpio_data = reg;
		}
	}
}

/*
 * Prepare for the power off of the system
 */
static void plat_marvell_power_off_prepare(struct power_off_method *pm_cfg,
					   register_t *addr, register_t *data)
{
	switch (pm_cfg->type) {
	case PMIC_GPIO:
		plat_marvell_power_off_gpio(pm_cfg, addr, data);
		break;
	default:
		break;
	}
}

/*****************************************************************************
 * A8K handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 *****************************************************************************
 */
static void a8k_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	if (is_pm_fw_running()) {
		unsigned int idx;

		/* Prevent interrupts from spuriously waking up this cpu */
		gicv2_cpuif_disable();

		idx = plat_my_core_pos();

		/* pm system synchronization - used to synchronize multiple
		 * core access to MSS
		 */
		bakery_lock_get(&pm_sys_lock);

		/* send CPU Suspend IPC Message to MSS */
		mss_pm_ipc_msg_send(idx, PM_IPC_MSG_CPU_SUSPEND, target_state);

		/* trigger IPC message to MSS */
		mss_pm_ipc_msg_trigger();

		/* pm system synchronization */
		bakery_lock_release(&pm_sys_lock);

		/* trace message */
		PM_TRACE(TRACE_PWR_DOMAIN_SUSPEND);
	} else {
		uintptr_t *mailbox = (void *)PLAT_MARVELL_MAILBOX_BASE;

		INFO("Suspending to RAM\n");

		marvell_console_runtime_end();

		/* Prevent interrupts from spuriously waking up this cpu */
		gicv2_cpuif_disable();

		mailbox[MBOX_IDX_SUSPEND_MAGIC] = MVEBU_MAILBOX_SUSPEND_STATE;
		mailbox[MBOX_IDX_ROM_EXIT_ADDR] = (uintptr_t)&plat_marvell_exit_bootrom;

#if PLAT_MARVELL_SHARED_RAM_CACHED
		flush_dcache_range(PLAT_MARVELL_MAILBOX_BASE +
		MBOX_IDX_SUSPEND_MAGIC * sizeof(uintptr_t),
		2 * sizeof(uintptr_t));
#endif
		/* Flush and disable LLC before going off-power */
		llc_disable(0);

		isb();
		/*
		 * Do not halt here!
		 * The function must return for allowing the caller function
		 * psci_power_up_finish() to do the proper context saving and
		 * to release the CPU lock.
		 */
	}
}

/*****************************************************************************
 * A8K handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 *****************************************************************************
 */
static void a8k_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* arch specific configuration */
	marvell_psci_arch_init(0);

	/* Interrupt initialization */
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	if (is_pm_fw_running()) {
		/* trace message */
		PM_TRACE(TRACE_PWR_DOMAIN_ON_FINISH);
	}
}

/*****************************************************************************
 * A8K handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 * TODO: At the moment we reuse the on finisher and reinitialize the secure
 * context. Need to implement a separate suspend finisher.
 *****************************************************************************
 */
static void a8k_pwr_domain_suspend_finish(
					const psci_power_state_t *target_state)
{
	if (is_pm_fw_running()) {
		/* arch specific configuration */
		marvell_psci_arch_init(0);

		/* Interrupt initialization */
		gicv2_cpuif_enable();

		/* trace message */
		PM_TRACE(TRACE_PWR_DOMAIN_SUSPEND_FINISH);
	} else {
		uintptr_t *mailbox = (void *)PLAT_MARVELL_MAILBOX_BASE;

		/* Only primary CPU requres platform init */
		if (!plat_my_core_pos()) {
			/* Initialize the console to provide
			 * early debug support
			 */
			marvell_console_runtime_init();

			bl31_plat_arch_setup();
			marvell_bl31_platform_setup();
			/*
			 * Remove suspend to RAM marker from the mailbox
			 * for treating a regular reset as a cold boot
			 */
			mailbox[MBOX_IDX_SUSPEND_MAGIC] = 0;
			mailbox[MBOX_IDX_ROM_EXIT_ADDR] = 0;
#if PLAT_MARVELL_SHARED_RAM_CACHED
			flush_dcache_range(PLAT_MARVELL_MAILBOX_BASE +
			MBOX_IDX_SUSPEND_MAGIC * sizeof(uintptr_t),
			2 * sizeof(uintptr_t));
#endif
		}
	}
}

/*****************************************************************************
 * This handler is called by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call to get the `power_state` parameter. This allows the platform to encode
 * the appropriate State-ID field within the `power_state` parameter which can
 * be utilized in `pwr_domain_suspend()` to suspend to system affinity level.
 *****************************************************************************
 */
static void a8k_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	/* lower affinities use PLAT_MAX_OFF_STATE */
	for (int i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

static void
__dead2 a8k_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	struct power_off_method *pm_cfg;
	unsigned int srcmd;
	unsigned int sdram_reg;
	register_t gpio_data = 0, gpio_addr = 0;

	if (is_pm_fw_running()) {
		psci_power_down_wfi();
		panic();
	}

	pm_cfg = (struct power_off_method *)plat_marvell_get_pm_cfg();

	/* Prepare for power off */
	plat_marvell_power_off_prepare(pm_cfg, &gpio_addr, &gpio_data);

	/* First step to enable DDR self-refresh
	 * to keep the data during suspend
	 */
	mmio_write_32(MVEBU_MC_PWR_CTRL_REG, 0x8C1);

	/* Save DDR self-refresh second step register
	 * and value to be issued later
	 */
	sdram_reg = MVEBU_USER_CMD_0_REG;
	srcmd = mmio_read_32(sdram_reg);
	srcmd &= ~(MVEBU_USER_CMD_CH0_MASK | MVEBU_USER_CMD_CS_MASK |
		 MVEBU_USER_CMD_SR_MASK);
	srcmd |= (MVEBU_USER_CMD_CH0_EN | MVEBU_USER_CMD_CS_ALL |
		 MVEBU_USER_CMD_SR_ENTER);

	/*
	 * Wait for DRAM is done using registers access only.
	 * At this stage any access to DRAM (procedure call) will
	 * release it from the self-refresh mode
	 */
	__asm__ volatile (
		/* Align to a cache line */
		"	.balign 64\n\t"

		/* Enter self refresh */
		"	str %[srcmd], [%[sdram_reg]]\n\t"

		/*
		 * Wait 100 cycles for DDR to enter self refresh, by
		 * doing 50 times two instructions.
		 */
		"	mov x1, #50\n\t"
		"1:	subs x1, x1, #1\n\t"
		"	bne 1b\n\t"

		/* Issue the command to trigger the SoC power off */
		"	str	%[gpio_data], [%[gpio_addr]]\n\t"

		/* Trap the processor */
		"	b .\n\t"
		: : [srcmd] "r" (srcmd), [sdram_reg] "r" (sdram_reg),
		    [gpio_addr] "r" (gpio_addr),  [gpio_data] "r" (gpio_data)
		: "x1");

	panic();
}

/*****************************************************************************
 * A8K handlers to shutdown/reboot the system
 *****************************************************************************
 */
static void __dead2 a8k_system_off(void)
{
	ERROR("%s:  needs to be implemented\n", __func__);
	panic();
}

void plat_marvell_system_reset(void)
{
	mmio_write_32(MVEBU_RFU_BASE + MVEBU_RFU_GLOBL_SW_RST, 0x0);
}

static void __dead2 a8k_system_reset(void)
{
	plat_marvell_system_reset();

	/* we shouldn't get to this point */
	panic();
}

/*****************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 *****************************************************************************
 */
const plat_psci_ops_t plat_arm_psci_pm_ops = {
	.cpu_standby = a8k_cpu_standby,
	.pwr_domain_on = a8k_pwr_domain_on,
	.pwr_domain_off = a8k_pwr_domain_off,
	.pwr_domain_suspend = a8k_pwr_domain_suspend,
	.pwr_domain_on_finish = a8k_pwr_domain_on_finish,
	.get_sys_suspend_power_state = a8k_get_sys_suspend_power_state,
	.pwr_domain_suspend_finish = a8k_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = a8k_pwr_domain_pwr_down_wfi,
	.system_off = a8k_system_off,
	.system_reset = a8k_system_reset,
	.validate_power_state = a8k_validate_power_state,
	.validate_ns_entrypoint = a8k_validate_ns_entrypoint
};
