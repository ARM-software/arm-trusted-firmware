/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <../hikey960_def.h>
#include <arch_helpers.h>
#include <assert.h>
#include <hisi_ipc.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>


#include "hisi_pwrc.h"


/* resource lock api */
#define RES0_LOCK_BASE		(SOC_PCTRL_RESOURCE0_LOCK_ADDR(PCTRL_BASE))
#define RES1_LOCK_BASE		(SOC_PCTRL_RESOURCE1_LOCK_ADDR(PCTRL_BASE))
#define RES2_LOCK_BASE		(SOC_PCTRL_RESOURCE2_LOCK_ADDR(PCTRL_BASE))

#define LOCK_BIT			(0x1 << 28)
#define LOCK_ID_MASK			(0x7 << 29)
#define CPUIDLE_LOCK_ID(core)		(0x6 - (core))
#define LOCK_UNLOCK_OFFSET		0x4
#define LOCK_STAT_OFFSET		0x8

#define CLUSTER0_CPUS_ONLINE_MASK	(0xF << 16)
#define	CLUSTER1_CPUS_ONLINE_MASK	(0xF << 20)

/* cpu hotplug flag api */
#define SCTRL_BASE			(SOC_ACPU_SCTRL_BASE_ADDR)
#define REG_SCBAKDATA3_OFFSET		(SOC_SCTRL_SCBAKDATA3_ADDR(SCTRL_BASE))
#define REG_SCBAKDATA8_OFFSET		(SOC_SCTRL_SCBAKDATA8_ADDR(SCTRL_BASE))
#define REG_SCBAKDATA9_OFFSET		(SOC_SCTRL_SCBAKDATA9_ADDR(SCTRL_BASE))

#define CPUIDLE_FLAG_REG(cluster) \
			((cluster == 0) ? REG_SCBAKDATA8_OFFSET : \
			 REG_SCBAKDATA9_OFFSET)
#define CLUSTER_IDLE_BIT				BIT(8)
#define CLUSTER_IDLE_MASK		(CLUSTER_IDLE_BIT | 0x0F)

#define AP_SUSPEND_FLAG			(1 << 16)

#define CLUSTER_PWDN_IDLE		(0<<28)
#define CLUSTER_PWDN_HOTPLUG		(1<<28)
#define CLUSTER_PWDN_SR			(2<<28)

#define CLUSTER0_PDC_OFFSET			0x260
#define CLUSTER1_PDC_OFFSET			0x300

#define PDC_EN_OFFSET				0x0
#define PDC_COREPWRINTEN_OFFSET		0x4
#define PDC_COREPWRINTSTAT_OFFSET	0x8
#define PDC_COREGICMASK_OFFSET		0xc
#define PDC_COREPOWERUP_OFFSET		0x10
#define PDC_COREPOWERDN_OFFSET		0x14
#define PDC_COREPOWERSTAT_OFFSET	0x18

#define PDC_COREPWRSTAT_MASK   (0XFFFF)

enum pdc_gic_mask {
	PDC_MASK_GIC_WAKE_IRQ,
	PDC_UNMASK_GIC_WAKE_IRQ
};

enum pdc_finish_int_mask {
	PDC_DISABLE_FINISH_INT,
	PDC_ENABLE_FINISH_INT
};

static void hisi_resource_lock(unsigned int lockid, unsigned int offset)
{
	unsigned int lock_id = (lockid << 29);
	unsigned int lock_val =  lock_id | LOCK_BIT;
	unsigned int lock_state;

	do {
		mmio_write_32(offset, lock_val);
		lock_state = mmio_read_32(LOCK_STAT_OFFSET + (uintptr_t)offset);
	} while ((lock_state & LOCK_ID_MASK) != lock_id);
}

static void hisi_resource_unlock(unsigned int lockid, unsigned int offset)
{
	unsigned int lock_val = (lockid << 29) | LOCK_BIT;

	mmio_write_32((LOCK_UNLOCK_OFFSET + (uintptr_t)offset), lock_val);
}


static void hisi_cpuhotplug_lock(unsigned int cluster, unsigned int core)
{
	unsigned int lock_id;

	lock_id = (cluster << 2) + core;

	hisi_resource_lock(lock_id, RES2_LOCK_BASE);
}

static void hisi_cpuhotplug_unlock(unsigned int cluster, unsigned int core)
{
	unsigned int lock_id;

	lock_id = (cluster << 2) + core;

	hisi_resource_unlock(lock_id, RES2_LOCK_BASE);
}

/* get the resource lock */
void hisi_cpuidle_lock(unsigned int cluster, unsigned int core)
{
	unsigned int offset = (cluster == 0 ? RES0_LOCK_BASE : RES1_LOCK_BASE);

	hisi_resource_lock(CPUIDLE_LOCK_ID(core), offset);
}

/* release the resource lock */
void hisi_cpuidle_unlock(unsigned int cluster, unsigned int core)
{
	unsigned int offset = (cluster == 0 ? RES0_LOCK_BASE : RES1_LOCK_BASE);

	hisi_resource_unlock(CPUIDLE_LOCK_ID(core), offset);
}

unsigned int hisi_get_cpuidle_flag(unsigned int cluster)
{
	unsigned int val;

	val = mmio_read_32(CPUIDLE_FLAG_REG(cluster));
	val &= 0xF;

	return val;
}

void hisi_set_cpuidle_flag(unsigned int cluster, unsigned int core)
{
	mmio_setbits_32(CPUIDLE_FLAG_REG(cluster), BIT(core));
}

void hisi_clear_cpuidle_flag(unsigned int cluster, unsigned int core)
{
	mmio_clrbits_32(CPUIDLE_FLAG_REG(cluster), BIT(core));

}

int hisi_test_ap_suspend_flag(unsigned int cluster)
{
	unsigned int val;

	val = mmio_read_32(CPUIDLE_FLAG_REG(cluster));
	val &= AP_SUSPEND_FLAG;
	return !!val;
}

void hisi_set_cluster_pwdn_flag(unsigned int cluster,
				unsigned int core, unsigned int value)
{
	unsigned int val;

	hisi_cpuhotplug_lock(cluster, core);

	val = mmio_read_32(REG_SCBAKDATA3_OFFSET);
	val = (value << (cluster << 1)) | (val & 0xFFFFFFF);
	mmio_write_32(REG_SCBAKDATA3_OFFSET, val);

	hisi_cpuhotplug_unlock(cluster, core);
}

unsigned int hisi_get_cpu_boot_flag(unsigned int cluster, unsigned int core)
{
	unsigned int val;

	hisi_cpuhotplug_lock(cluster, core);
	val = mmio_read_32(REG_SCBAKDATA3_OFFSET);
	val = val >> (16 + (cluster << 2));
	val &= 0xF;
	hisi_cpuhotplug_unlock(cluster, core);

	return val;
}

unsigned int hisi_test_cpu_down(unsigned int cluster, unsigned int core)
{
	unsigned int val;

	hisi_cpuhotplug_lock(cluster, core);
	val = mmio_read_32(REG_SCBAKDATA3_OFFSET);
	val = val >> (16 + (cluster << 2));
	val &= 0xF;
	hisi_cpuhotplug_unlock(cluster, core);

	if (val)
		return 0;
	else
		return 1;
}

void hisi_set_cpu_boot_flag(unsigned int cluster, unsigned int core)
{
	unsigned int flag = BIT((cluster<<2) + core + 16);

	hisi_cpuhotplug_lock(cluster, core);

	mmio_setbits_32(REG_SCBAKDATA3_OFFSET, flag);

	hisi_cpuhotplug_unlock(cluster, core);
}

void hisi_clear_cpu_boot_flag(unsigned int cluster, unsigned int core)
{
	unsigned int flag = BIT((cluster<<2) + core + 16);

	hisi_cpuhotplug_lock(cluster, core);

	mmio_clrbits_32(REG_SCBAKDATA3_OFFSET, flag);

	hisi_cpuhotplug_unlock(cluster, core);
}

int cluster_is_powered_on(unsigned int cluster)
{
	unsigned int val = mmio_read_32(REG_SCBAKDATA3_OFFSET);
	int ret;

	if (cluster == 0)
		ret = val & CLUSTER0_CPUS_ONLINE_MASK;
	else
		ret = val & CLUSTER1_CPUS_ONLINE_MASK;

	return !!ret;
}

static void *hisi_get_pdc_addr(unsigned int cluster)
{
	void *pdc_base_addr;
	uintptr_t addr;

	if (cluster == 0)
		addr = SOC_CRGPERIPH_A53_PDCEN_ADDR(CRG_BASE);
	else
		addr = SOC_CRGPERIPH_MAIA_PDCEN_ADDR(CRG_BASE);
	pdc_base_addr = (void *)addr;

	return pdc_base_addr;
}

static unsigned int hisi_get_pdc_stat(unsigned int cluster)
{
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);
	unsigned int val;

	val = mmio_read_32((uintptr_t)pdc_base_addr + PDC_COREPOWERSTAT_OFFSET);

	return val;
}

int hisi_test_pwrdn_allcores(unsigned int cluster, unsigned int core)
{
	unsigned int mask = 0xf << (core * 4);
	unsigned int pdc_stat = hisi_get_pdc_stat(cluster);
	unsigned int boot_flag = hisi_get_cpu_boot_flag(cluster, core);
	unsigned int cpuidle_flag = hisi_get_cpuidle_flag(cluster);

	mask = (PDC_COREPWRSTAT_MASK & (~mask));
	pdc_stat &= mask;

	if ((boot_flag ^ cpuidle_flag) || pdc_stat)
		return 0;
	else
		return 1;
}

void hisi_disable_pdc(unsigned int cluster)
{
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);

	mmio_write_32((uintptr_t)pdc_base_addr, 0x0);
}

void hisi_enable_pdc(unsigned int cluster)
{
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);

	mmio_write_32((uintptr_t)pdc_base_addr, 0x1);
}

static inline void hisi_pdc_set_intmask(void *pdc_base_addr,
					unsigned int core,
					enum pdc_finish_int_mask intmask)
{
	unsigned int val;

	val = mmio_read_32((uintptr_t)pdc_base_addr + PDC_COREPWRINTEN_OFFSET);
	if (intmask == PDC_ENABLE_FINISH_INT)
		val |= BIT(core);
	else
		val &= ~BIT(core);

	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREPWRINTEN_OFFSET, val);
}

static inline void hisi_pdc_set_gicmask(void *pdc_base_addr,
					unsigned int core,
					enum pdc_gic_mask gicmask)
{
	unsigned int val;

	val = mmio_read_32((uintptr_t)pdc_base_addr + PDC_COREGICMASK_OFFSET);
	if (gicmask == PDC_MASK_GIC_WAKE_IRQ)
		val |= BIT(core);
	else
		val &= ~BIT(core);

	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREGICMASK_OFFSET, val);
}

void hisi_pdc_mask_cluster_wakeirq(unsigned int cluster)
{
	int i;
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);

	for (i = 0; i < 4; i++)
		hisi_pdc_set_gicmask(pdc_base_addr, i, PDC_MASK_GIC_WAKE_IRQ);
}

static void hisi_pdc_powerup_core(unsigned int cluster, unsigned int core,
				  enum pdc_gic_mask gicmask,
				  enum pdc_finish_int_mask intmask)
{
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);

	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREPOWERUP_OFFSET,
		      BIT(core));
}

static void hisi_pdc_powerdn_core(unsigned int cluster, unsigned int core,
				  enum pdc_gic_mask gicmask,
				  enum pdc_finish_int_mask intmask)
{
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);

	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREPOWERDN_OFFSET,
		      BIT(core));
}

void hisi_powerup_core(unsigned int cluster, unsigned int core)
{
	hisi_pdc_powerup_core(cluster, core, PDC_MASK_GIC_WAKE_IRQ,
			      PDC_DISABLE_FINISH_INT);
}

void hisi_powerdn_core(unsigned int cluster, unsigned int core)
{
	hisi_pdc_powerdn_core(cluster, core, PDC_MASK_GIC_WAKE_IRQ,
			      PDC_DISABLE_FINISH_INT);
}

void hisi_powerup_cluster(unsigned int cluster, unsigned int core)
{
	hisi_ipc_pm_on_off(core, cluster, PM_ON);
}

void hisi_powerdn_cluster(unsigned int cluster, unsigned int core)
{
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);

	hisi_set_cluster_pwdn_flag(cluster, core, CLUSTER_PWDN_HOTPLUG);
	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREPWRINTEN_OFFSET,
		      (0x10001 << core));
	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREPOWERDN_OFFSET,
		      BIT(core));
}

void hisi_enter_core_idle(unsigned int cluster, unsigned int core)
{
	hisi_pdc_powerdn_core(cluster, core, PDC_UNMASK_GIC_WAKE_IRQ,
			      PDC_DISABLE_FINISH_INT);
}

void hisi_enter_cluster_idle(unsigned int cluster, unsigned int core)
{
	void *pdc_base_addr = hisi_get_pdc_addr(cluster);

	hisi_set_cluster_pwdn_flag(cluster, core, CLUSTER_PWDN_IDLE);
	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREPWRINTEN_OFFSET,
		      (0x10001 << core));
	mmio_write_32((uintptr_t)pdc_base_addr + PDC_COREPOWERDN_OFFSET,
		      BIT(core));
}

void hisi_enter_ap_suspend(unsigned int cluster, unsigned int core)
{
	hisi_ipc_pm_suspend(core, cluster, 0x3);
}
