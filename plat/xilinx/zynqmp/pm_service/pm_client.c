/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * APU specific definition of processors in the subsystem as well as functions
 * for getting information about and changing state of the APU.
 */

#include <assert.h>
#include <string.h>

#include <common/bl_common.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/utils.h>

#include <plat_ipi.h>
#include <zynqmp_def.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_ipi.h"

#define IRQ_MAX		84
#define NUM_GICD_ISENABLER	((IRQ_MAX >> 5) + 1)
#define UNDEFINED_CPUID		(~0)

#define PM_SUSPEND_MODE_STD		0
#define PM_SUSPEND_MODE_POWER_OFF	1

DEFINE_BAKERY_LOCK(pm_client_secure_lock);

extern const struct pm_ipi apu_ipi;

const struct pm_ipi apu_ipi = {
	.local_ipi_id = IPI_ID_APU,
	.remote_ipi_id = IPI_ID_PMU0,
	.buffer_base = IPI_BUFFER_APU_BASE,
};

static uint32_t suspend_mode = PM_SUSPEND_MODE_STD;

/* Order in pm_procs_all array must match cpu ids */
static const struct pm_proc pm_procs_all[] = {
	{
		.node_id = NODE_APU_0,
		.pwrdn_mask = APU_0_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
	{
		.node_id = NODE_APU_1,
		.pwrdn_mask = APU_1_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
	{
		.node_id = NODE_APU_2,
		.pwrdn_mask = APU_2_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
	{
		.node_id = NODE_APU_3,
		.pwrdn_mask = APU_3_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
};

/* Interrupt to PM node ID map */
static enum pm_node_id irq_node_map[IRQ_MAX + 1] = {
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,	/* 3 */
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,	/* 7 */
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,	/* 11 */
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_NAND,
	NODE_QSPI,	/* 15 */
	NODE_GPIO,
	NODE_I2C_0,
	NODE_I2C_1,
	NODE_SPI_0,	/* 19 */
	NODE_SPI_1,
	NODE_UART_0,
	NODE_UART_1,
	NODE_CAN_0,	/* 23 */
	NODE_CAN_1,
	NODE_UNKNOWN,
	NODE_RTC,
	NODE_RTC,	/* 27 */
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,	/* 31 */
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,	/* 35, NODE_IPI_APU */
	NODE_TTC_0,
	NODE_TTC_0,
	NODE_TTC_0,
	NODE_TTC_1,	/* 39 */
	NODE_TTC_1,
	NODE_TTC_1,
	NODE_TTC_2,
	NODE_TTC_2,	/* 43 */
	NODE_TTC_2,
	NODE_TTC_3,
	NODE_TTC_3,
	NODE_TTC_3,	/* 47 */
	NODE_SD_0,
	NODE_SD_1,
	NODE_SD_0,
	NODE_SD_1,	/* 51 */
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,
	NODE_UNKNOWN,	/* 55 */
	NODE_UNKNOWN,
	NODE_ETH_0,
	NODE_ETH_0,
	NODE_ETH_1,	/* 59 */
	NODE_ETH_1,
	NODE_ETH_2,
	NODE_ETH_2,
	NODE_ETH_3,	/* 63 */
	NODE_ETH_3,
	NODE_USB_0,
	NODE_USB_0,
	NODE_USB_0,	/* 67 */
	NODE_USB_0,
	NODE_USB_0,
	NODE_USB_1,
	NODE_USB_1,	/* 71 */
	NODE_USB_1,
	NODE_USB_1,
	NODE_USB_1,
	NODE_USB_0,	/* 75 */
	NODE_USB_0,
	NODE_ADMA,
	NODE_ADMA,
	NODE_ADMA,	/* 79 */
	NODE_ADMA,
	NODE_ADMA,
	NODE_ADMA,
	NODE_ADMA,	/* 83 */
	NODE_ADMA,
};

/**
 * irq_to_pm_node - Get PM node ID corresponding to the interrupt number
 * @irq:	Interrupt number
 *
 * Return:	PM node ID corresponding to the specified interrupt
 */
static enum pm_node_id irq_to_pm_node(unsigned int irq)
{
	assert(irq <= IRQ_MAX);
	return irq_node_map[irq];
}

/**
 * pm_client_set_wakeup_sources - Set all slaves with enabled interrupts as wake
 *				sources in the PMU firmware
 */
static void pm_client_set_wakeup_sources(void)
{
	uint32_t reg_num;
	uint8_t pm_wakeup_nodes_set[NODE_MAX];
	uintptr_t isenabler1 = BASE_GICD_BASE + GICD_ISENABLER + 4;

	/* In case of power-off suspend, only NODE_EXTERN must be set */
	if (suspend_mode == PM_SUSPEND_MODE_POWER_OFF) {
		enum pm_ret_status ret;

		ret = pm_set_wakeup_source(NODE_APU, NODE_EXTERN, 1);
		/**
		 * If NODE_EXTERN could not be set as wake source, proceed with
		 * standard suspend (no one will wake the system otherwise)
		 */
		if (ret == PM_RET_SUCCESS) {
			return;
		}
	}

	zeromem(&pm_wakeup_nodes_set, sizeof(pm_wakeup_nodes_set));

	for (reg_num = 0; reg_num < NUM_GICD_ISENABLER; reg_num++) {
		uint32_t base_irq = reg_num << ISENABLER_SHIFT;
		uint32_t reg = mmio_read_32(isenabler1 + (reg_num << 2));

		if (!reg) {
			continue;
		}

		while (reg) {
			enum pm_node_id node;
			uint32_t idx, ret, irq, lowest_set = reg & (-reg);

			idx = __builtin_ctz(lowest_set);
			irq = base_irq + idx;

			if (irq > IRQ_MAX) {
				break;
			}

			node = irq_to_pm_node(irq);
			reg &= ~lowest_set;

			if ((node != NODE_UNKNOWN) &&
			    (!pm_wakeup_nodes_set[node])) {
				ret = pm_set_wakeup_source(NODE_APU, node, 1);
				pm_wakeup_nodes_set[node] = !ret;
			}
		}
	}
}

/**
 * pm_get_proc() - returns pointer to the proc structure
 * @cpuid:	id of the cpu whose proc struct pointer should be returned
 *
 * Return: pointer to a proc structure if proc is found, otherwise NULL
 */
const struct pm_proc *pm_get_proc(unsigned int cpuid)
{
	if (cpuid < ARRAY_SIZE(pm_procs_all)) {
		return &pm_procs_all[cpuid];
	}

	return NULL;
}

/**
 * pm_get_proc_by_node() - returns pointer to the proc structure
 * @nid:	node id of the processor
 *
 * Return: pointer to a proc structure if proc is found, otherwise NULL
 */
const struct pm_proc *pm_get_proc_by_node(enum pm_node_id nid)
{
	for (size_t i = 0; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (nid == pm_procs_all[i].node_id) {
			return &pm_procs_all[i];
		}
	}
	return NULL;
}

/**
 * pm_get_cpuid() - get the local cpu ID for a global node ID
 * @nid:	node id of the processor
 *
 * Return: the cpu ID (starting from 0) for the subsystem
 */
static unsigned int pm_get_cpuid(enum pm_node_id nid)
{
	for (size_t i = 0; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (pm_procs_all[i].node_id == nid) {
			return i;
		}
	}
	return UNDEFINED_CPUID;
}

const struct pm_proc *primary_proc = &pm_procs_all[0];

/**
 * pm_client_suspend() - Client-specific suspend actions
 *
 * This function should contain any PU-specific actions
 * required prior to sending suspend request to PMU
 * Actions taken depend on the state system is suspending to.
 */
void pm_client_suspend(const struct pm_proc *proc, unsigned int state)
{
	bakery_lock_get(&pm_client_secure_lock);

	if (state == PM_STATE_SUSPEND_TO_RAM) {
		pm_client_set_wakeup_sources();
	}

	/* Set powerdown request */
	mmio_write_32(APU_PWRCTL, mmio_read_32(APU_PWRCTL) | proc->pwrdn_mask);

	bakery_lock_release(&pm_client_secure_lock);
}


/**
 * pm_client_abort_suspend() - Client-specific abort-suspend actions
 *
 * This function should contain any PU-specific actions
 * required for aborting a prior suspend request
 */
void pm_client_abort_suspend(void)
{
	/* Enable interrupts at processor level (for current cpu) */
	gicv2_cpuif_enable();

	bakery_lock_get(&pm_client_secure_lock);

	/* Clear powerdown request */
	mmio_write_32(APU_PWRCTL,
		 mmio_read_32(APU_PWRCTL) & ~primary_proc->pwrdn_mask);

	bakery_lock_release(&pm_client_secure_lock);
}

/**
 * pm_client_wakeup() - Client-specific wakeup actions
 *
 * This function should contain any PU-specific actions
 * required for waking up another APU core
 */
void pm_client_wakeup(const struct pm_proc *proc)
{
	unsigned int cpuid = pm_get_cpuid(proc->node_id);

	if (cpuid == UNDEFINED_CPUID) {
		return;
	}

	bakery_lock_get(&pm_client_secure_lock);

	/* clear powerdown bit for affected cpu */
	uint32_t val = mmio_read_32(APU_PWRCTL);
	val &= ~(proc->pwrdn_mask);
	mmio_write_32(APU_PWRCTL, val);

	bakery_lock_release(&pm_client_secure_lock);
}

enum pm_ret_status pm_set_suspend_mode(uint32_t mode)
{
	if ((mode != PM_SUSPEND_MODE_STD) &&
	    (mode != PM_SUSPEND_MODE_POWER_OFF)) {
		return PM_RET_ERROR_ARGS;
	}

	suspend_mode = mode;
	return PM_RET_SUCCESS;
}
