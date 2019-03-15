/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <bl31/interrupt_mgmt.h>
#include <../drivers/arm/gic/v3/gicv3_private.h>
#include <mt_gic_v3.h>
#include <mtk_plat_common.h>
#include "plat_private.h"
#include <plat/common/platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_IRQ_NR			335
#ifndef MAX_GIC_NR
#define MAX_GIC_NR			1
#endif

#define MAX_RDIST_NR		64
#define NR_INT_POL_CTL		20
#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))
#define SZ_64K				(0x00010000)

/* helpers for later ICC encode macros
 * Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

/* #define ICC_GRPEN1_EL1  S3_0_c12_c12_7 */
/* DEFINE_RENAME_SYSREG_WRITE_FUNC(icc_igrpen1_el1, ICC_GRPEN1_EL1) */

#define MPIDR_LEVEL_BITS_SHIFT	3
#define MPIDR_LEVEL_BITS		(1 << MPIDR_LEVEL_BITS_SHIFT)
#define MPIDR_LEVEL_MASK		((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_LEVEL_SHIFT(level) \
	(((1 << (level)) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
	(((mpidr) >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

#define CPU_LOGIC_MAP_RESET_VAL	(-1L)
static uint64_t cpu_logical_map[PLATFORM_CORE_COUNT];

struct gic_chip_data {
	unsigned int saved_enable[11];
	unsigned int saved_conf[21];
	unsigned int saved_priority[84];
	uint64_t saved_target[MAX_IRQ_NR];
	unsigned int saved_group[11];
	unsigned int saved_grpmod[11];
	unsigned int rdist_base[MAX_RDIST_NR];
	unsigned int saved_active_sel;
	unsigned int saved_sgi[PLATFORM_CORE_COUNT];
};

static struct gic_chip_data gic_data[MAX_GIC_NR];

void gic_sgi_save_all(void)
{
	int cpuid = 0;
	unsigned int base;
	unsigned int cpu_offset = 0;

	base = MT_GIC_RDIST_BASE;
	base += SZ_64K;
	for (cpuid = 0; cpuid < PLATFORM_CORE_COUNT; cpuid++) {
		cpu_offset = cpuid * SZ_64K * 2;
		gic_data[0].saved_sgi[cpuid] = mmio_read_32(base + GICD_V3_ISPENDR + cpu_offset) & SGI_MASK;
	}
}

void gic_sgi_restore_all(void)
{
	int cpuid = 0;
	unsigned int base;
	unsigned int cpu_offset = 0;

	base = MT_GIC_RDIST_BASE;
	base += SZ_64K;
	for (cpuid = 0; cpuid < PLATFORM_CORE_COUNT; cpuid++) {
		cpu_offset = cpuid*SZ_64K*2;
		mmio_write_32(base + GICD_V3_ICPENDR + cpu_offset, SGI_MASK);
		mmio_write_32(base + GICD_V3_ISPENDR + cpu_offset, gic_data[0].saved_sgi[cpuid] & SGI_MASK);
	}
}

static void gicd_v3_do_wait_for_rwp(unsigned int gicd_base)
{
	while (gicd_v3_read_ctlr(gicd_base) & GICD_V3_CTLR_RWP) {
	}
}

static int gic_populate_rdist(unsigned int *rdist_base)
{
	int cpuid = 0;
	unsigned int reg = 0;
	unsigned int base = 0;

	cpuid = plat_core_pos_by_mpidr(read_mpidr());
	reg = gicd_v3_read_pidr2(MT_GIC_BASE) & GIC_V3_PIDR2_ARCH_MASK;

	if ((reg != GIC_V3_PIDR2_ARCH_GICv3) && (reg != GIC_V3_PIDR2_ARCH_GICv4)) {
		ERROR("We are in trouble, not GIC-v3 or v4\n");
		return -1;
	}

	base = MT_GIC_RDIST_BASE + cpuid*SZ_64K*2;

	*rdist_base = base;
	/* save a copy for later save/restore use */
	gic_data[0].rdist_base[cpuid] = base;

	return 0;
}

void gic_dist_save(void)
{
	unsigned int gic_irqs;
	unsigned int dist_base;
	int i;

	dist_base = MT_GIC_BASE;
	gic_irqs = MAX_IRQ_NR;

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		gic_data[0].saved_conf[i] =
			mmio_read_32(dist_base + GICD_ICFGR + i * 4);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		gic_data[0].saved_priority[i] =
			mmio_read_32(dist_base + GICD_IPRIORITYR + i * 4);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 1); i++)
		gic_data[0].saved_target[i] =
			mmio_read_64(dist_base + GICD_V3_IROUTER + i*8);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_enable[i] =
			mmio_read_32(dist_base + GICD_ISENABLER + i * 4);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_group[i] =
			mmio_read_32(dist_base + GICD_IGROUPR + i * 4);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		gic_data[0].saved_grpmod[i] =
			mmio_read_32(dist_base + GICE_V3_IGRPMOD0 + i * 4);
}

static uint32_t rdist_has_saved[PLATFORM_CORE_COUNT];

void gic_rdist_save(void)
{
	unsigned int rdist_base;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&rdist_base) == -1) {
		return;
	}

	/* the SGI/PPI base is at 64K offset of current rdist base */
	rdist_base += SZ_64K;

	/* handle SGI & PPI */
	gic_data[0].saved_conf[0] = mmio_read_32(rdist_base + GICD_ICFGR);
	gic_data[0].saved_priority[0] = mmio_read_32(rdist_base + GICD_IPRIORITYR);
	gic_data[0].saved_enable[0] = mmio_read_32(rdist_base + GICD_ISENABLER);
	gic_data[0].saved_group[0] = mmio_read_32(rdist_base + GICD_IGROUPR);
	gic_data[0].saved_grpmod[0] = mmio_read_32(rdist_base + GICE_V3_IGRPMOD0);

	rdist_has_saved[plat_core_pos_by_mpidr(read_mpidr())] = 1;
	int_schedule_mode_save();
}

void gic_dist_restore(void)
{
	unsigned int gic_irqs;
	unsigned int dist_base;
	unsigned int ctlr;
	unsigned int rdist_sgi_base;
	unsigned int i = 0;

	dist_base = MT_GIC_BASE;
	gic_irqs = MAX_IRQ_NR;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&rdist_sgi_base) == -1) {
		return;
	}

	/* the SGI/PPI base is at 64K offset of current rdist base */
	rdist_sgi_base += SZ_64K;

	/* disable first before going on */
	ctlr = gicd_v3_read_ctlr(dist_base);
	ctlr &= ~(GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S);
	gicd_v3_write_ctlr(dist_base, ctlr);
	gicd_v3_do_wait_for_rwp(dist_base);

	/* handle SGI & PPI */
	mmio_write_32(rdist_sgi_base + GICD_ICFGR, gic_data[0].saved_conf[0]);
	mmio_write_32(rdist_sgi_base + GICD_IPRIORITYR, gic_data[0].saved_priority[0]);
	mmio_write_32(rdist_sgi_base + GICD_ISENABLER, gic_data[0].saved_enable[0]);
	mmio_write_32(rdist_sgi_base + GICD_IGROUPR, gic_data[0].saved_group[0]);
	mmio_write_32(rdist_sgi_base + GICE_V3_IGRPMOD0, gic_data[0].saved_grpmod[0]);

	/* restore SPI setting */
	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 16); i++)
		mmio_write_32(dist_base + GICD_ICFGR + i * 4, gic_data[0].saved_conf[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 4); i++)
		mmio_write_32(dist_base + GICD_IPRIORITYR + i * 4, gic_data[0].saved_priority[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 1); i++)
		mmio_write_64(dist_base + GICD_V3_IROUTER + i*8, gic_data[0].saved_target[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_ISENABLER + i * 4, gic_data[0].saved_enable[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICD_IGROUPR + i * 4, gic_data[0].saved_group[i]);

	for (i = 1; i < DIV_ROUND_UP(gic_irqs, 32); i++)
		mmio_write_32(dist_base + GICE_V3_IGRPMOD0 + i * 4, gic_data[0].saved_grpmod[i]);

	/* enable all groups & ARE */
	ctlr = GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S |
		GICD_V3_CTLR_ARE_S | GICD_V3_CTLR_ARE_NS;

	gicd_v3_write_ctlr(dist_base, ctlr);
	gicd_v3_do_wait_for_rwp(dist_base);
	int_schedule_mode_restore();
}

void gic_rdist_restore_all(void)
{
	int cpuid = 0;
	unsigned int base;
	unsigned int cpu_offset = 0;

	base = MT_GIC_RDIST_BASE;
	base += SZ_64K;

	for (cpuid = 0; cpuid < PLATFORM_CORE_COUNT; cpuid++) {
		cpu_offset = cpuid*SZ_64K*2;
		mmio_write_32(base + GICD_ICFGR + cpu_offset, gic_data[0].saved_conf[0]);
		mmio_write_32(base + GICD_IPRIORITYR + cpu_offset, gic_data[0].saved_priority[0]);
		mmio_write_32(base + GICD_ISENABLER + cpu_offset, gic_data[0].saved_enable[0]);
		mmio_write_32(base + GICD_IGROUPR + cpu_offset, gic_data[0].saved_group[0]);
		mmio_write_32(base + GICE_V3_IGRPMOD0 + cpu_offset, gic_data[0].saved_grpmod[0]);
	}
}

void gic_rdist_restore(void)
{
	unsigned int rdist_sgi_base;

	if (rdist_has_saved[plat_core_pos_by_mpidr(read_mpidr())] == 0)
		return;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&rdist_sgi_base) == -1) {
		return;
	}

	/* the SGI/PPI base is at 64K offset of current rdist base */
	rdist_sgi_base += SZ_64K;

	/* handle SGI & PPI */
	mmio_write_32(rdist_sgi_base + GICD_ICFGR, gic_data[0].saved_conf[0]);
	mmio_write_32(rdist_sgi_base + GICD_IPRIORITYR, gic_data[0].saved_priority[0]);
	mmio_write_32(rdist_sgi_base + GICD_ISENABLER, gic_data[0].saved_enable[0]);
	mmio_write_32(rdist_sgi_base + GICD_IGROUPR, gic_data[0].saved_group[0]);
	mmio_write_32(rdist_sgi_base + GICE_V3_IGRPMOD0, gic_data[0].saved_grpmod[0]);
}

static uint16_t compute_target_list(uint8_t *cpu, unsigned int map, uint64_t cluster_id)
{
	uint16_t tlist = 0;
	uint8_t target_cpu = 0;

	for (target_cpu = *cpu; target_cpu < PLATFORM_CORE_COUNT; ++target_cpu) {
		uint64_t mpidr = cpu_logical_map[target_cpu];
		uint64_t target_cluster_id = mpidr & ~0xffUL;

		/* only check those cores in map */
		if ((((1 << target_cpu) & map) == 0) ||
			cpu_logical_map[target_cpu] == CPU_LOGIC_MAP_RESET_VAL)
			continue;
		/* gic-500 only support 16 cores in a cluster at max */
		if ((mpidr & 0xff) > 16) {
			ERROR("%s:%d: wrong cpu[%u] mpidr(%lx)\n",
				__func__, __LINE__, (unsigned int) target_cpu, (long unsigned int) mpidr);
			break;
		}

		/* we assume cluster_id only changes _WHEN_
		 * all cores in the same cluster is counted */
		if (target_cluster_id != cluster_id) {
			break;
		} else {
			/* the core in the same cluster, add it. */
			tlist |= 1 << (mpidr & 0xff);
		}
	}

	/* update cpu for the next cluster */
	*cpu = target_cpu - 1;
	return tlist;
}

static void gic_send_sgi(uint64_t cluster_id, uint16_t tlist, unsigned int irq)
{
	uint64_t val = 0;

	val = irq << 24	| (((cluster_id >> 8) & 0xff) << 16) | tlist;
	write_icc_sgi0r_el1(val);
}

void irq_raise_softirq(unsigned int map, unsigned int irq)
{
	uint8_t cpu = 0;
	uint16_t tlist = 0;
	uint64_t cluster_id = 0;
	uint64_t mpidr = 0;

	/*
	 * Ensure that stores to Normal memory are visible to the
	 * other CPUs before issuing the IPI.
	 */
	dmbsy();

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT ; ++cpu) {
		/* only check those cores in map */
		if ((((1 << cpu) & map) == 0) ||
				(cpu_logical_map[cpu] == CPU_LOGIC_MAP_RESET_VAL)) {
			continue;
		}

		/* gicv3 can only send sgi per cluster, */
		/* gather all cores in map in the same cluster */
		mpidr = cpu_logical_map[cpu];
		cluster_id = mpidr & ~0xffUL;
		tlist = compute_target_list(&cpu, map, cluster_id);
		gic_send_sgi(cluster_id, tlist, irq);
	}

	/* Force the above writes to ICC_SGI0R_EL1 to be executed */
	isb();
}

unsigned int get_ack_info(void)
{
	return (read_icc_iar0_el1()&0x3ff);
}

void ack_sgi(unsigned int irq)
{
	write_icc_eoir0_el1(irq);
}

void mask_wdt_fiq(void)
{
	gicd_set_icenabler(MT_GIC_BASE, WDT_IRQ_BIT_ID);
}


/* =========================================================================== */
/* Set WDT interrupt pending. It is for simulation of WDT when processing SMC  */
/* "AEEDump".                                                                  */
/* =========================================================================== */
void mt_atf_trigger_WDT_FIQ(void)
{
	gicd_set_ispendr(MT_GIC_BASE, WDT_IRQ_BIT_ID);
}

/*******************************************************************************
 * Place the cpu interface in a state where it can never make a cpu exit wfi as
 * as result of an asserted interrupt. This is critical for powering down a cpu
 ******************************************************************************/
void gic_cpuif_deactivate(unsigned int gicc_base)
{
	unsigned int val;
	unsigned int rdist_base;

	/* disable group0 */
	write_icc_igrpen0_el1(0x0);

	/*
	 * When taking CPUs down we need to set GICR_WAKER.ProcessorSleep and
	 * wait for GICR_WAKER.ChildrenAsleep to get set.
	 * (GICv3 Architecture specification 5.4.23).
	 * GICR_WAKER is NOT banked per CPU, compute the correct base address
	 * per CPU.
	 */
	if (gic_populate_rdist(&rdist_base) == -1) {
		ERROR("%s:%d: can not get redistributor's base\n", __func__, __LINE__);
		panic();
	}

	val = mmio_read_32(rdist_base+GICR_V3_WAKER);
	val |= GICR_V3_WAKER_ProcessorSleep;
	mmio_write_32(rdist_base+GICR_V3_WAKER, val);
	dsb();

	/* wait until our children sleep */
	while ((mmio_read_32(rdist_base + GICR_V3_WAKER) & GICR_V3_WAKER_ChildrenAsleep) == 0) {
	}
}


void mt_irq_set_polarity(unsigned int irq, unsigned int polarity)
{
	unsigned int offset, reg_index, value;

	offset = (irq - GIC_PRIVATE_SIGNALS) & 0x1F;
	reg_index = (irq - GIC_PRIVATE_SIGNALS) >> 5;

	if (polarity == 0) {
		/* active low */
		value = mmio_read_32(INT_POL_CTL0 + (reg_index * 4));
		value |= (1 << offset);
		mmio_write_32((INT_POL_CTL0 + (reg_index * 4)), value);
	} else {
		/* active high */
		value = mmio_read_32(INT_POL_CTL0 + (reg_index * 4));
		value &= ~(0x1 << offset);
		mmio_write_32(INT_POL_CTL0 + (reg_index * 4), value);
	}
}

void mt_irq_set_sens(unsigned int base, unsigned int irq, unsigned int sens)
{
	unsigned int config;

	if (sens == MT_EDGE_SENSITIVE) {
		config = mmio_read_32(base + GICD_ICFGR + (irq / 16) * 4);
		config |= (0x2 << (irq % 16) * 2);
		mmio_write_32(base + GICD_ICFGR + (irq / 16) * 4, config);
	} else {
		config = mmio_read_32(base + GICD_ICFGR + (irq / 16) * 4);
		config &= ~(0x2 << (irq % 16) * 2);
		mmio_write_32(base + GICD_ICFGR + (irq / 16) * 4, config);
	}
}

static void enable_sgi_fiq(uint32_t fiq_num)
{
	unsigned int rdist_sgi_base = 0;

	assert(gic_populate_rdist((unsigned int *)&rdist_sgi_base) != -1);
	gicr_write_igroupr0(rdist_sgi_base, ~0x0);
}

int gic_cpuif_init(void)
{
	unsigned int rdist_base = 0;
	unsigned int val = 0;
	unsigned int scr_val = 0;
	uint64_t mpidr = 0;
	uint32_t cpu = 0;
	unsigned int index;

	if (gic_populate_rdist(&rdist_base) == -1)
		return -1;

	/* Wake up this CPU redistributor */
	val = mmio_read_32(rdist_base+GICR_V3_WAKER);
	val &= ~GICR_V3_WAKER_ProcessorSleep;
	mmio_write_32(rdist_base+GICR_V3_WAKER, val);

	/* wait until our children wakeup */
	while (mmio_read_32(rdist_base + GICR_V3_WAKER) &
			GICR_V3_WAKER_ChildrenAsleep) {
	}

	/* set all SGI/PPI as non-secure GROUP1 by default. */
	/* rdist_base + 64K == SGI_base */
	mmio_write_32(rdist_base+SZ_64K+GICE_V3_IGROUP0, 0xffffffff);
	mmio_write_32(rdist_base+SZ_64K+GICE_V3_IGRPMOD0, 0x0);

	/*
	 * Setup the default PPI/SGI priorities.
	 * 0xD0 is the priority from kernel
	 */
	for (index = 0; index < MIN_SPI_ID; index++)
		gicd_set_ipriorityr(rdist_base+SZ_64K, index, 0xD0);
	/*
	 * We need to set SCR_EL3.NS in order to see GICv3 non-secure state.
	 * Restore SCR_EL3.NS again before exit.
	 */
	scr_val = read_scr();
	write_scr(scr_val | SCR_NS_BIT);
	isb();	/* ensure NS=1 takes effect before accessing ICC_SRE_EL2 */

	/* enable SRE bit in ICC_SRE_ELx in order */
	val = read_icc_sre_el3();
	write_icc_sre_el3(val | ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT);
	isb(); /* before enable lower SRE, be sure SRE in el3 takes effect */

	val = read_icc_sre_el2();
	write_icc_sre_el2(val | ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT);
	isb(); /* before enable lower SRE, be sure SRE in el2 takes effect */

	write_icc_pmr_el1(GIC_PRI_MASK);
	isb();	/* commite ICC_* changes before setting NS=0 */

	/* Restore SCR_EL3 */
	write_scr(scr_val);
	isb();	/* ensure NS=0 takes effect immediately */

	/* MUST set secure copy of icc_sre_el1 as SRE_SRE to enable FIQ,
	see GICv3 spec 4.6.4 FIQ Enable */
	val = read_icc_sre_el1();
	write_icc_sre_el1(val | ICC_SRE_SRE_BIT);
	isb(); /* before we can touch other ICC_* system registers, make sure this have effect */

	/* here we go, can handle FIQ after this */
	write_icc_igrpen0_el1(0x1);
	/* allow S-EL1 can get group1 irq */
	write_icc_igrpen1_el1(0x1);

	/* everytime we setup the cpu IF, add a SGI as FIQ for smp call debug */
	enable_sgi_fiq(FIQ_SMP_CALL_SGI);

	/* init mpidr table for this cpu for later sgi usage */
	mpidr = read_mpidr();
	cpu = plat_core_pos_by_mpidr(mpidr);
	if (cpu < PLATFORM_CORE_COUNT)
		cpu_logical_map[cpu] = mpidr;

	return 0;
}

static void gic_distif_init(unsigned int gicd_base)
{
	unsigned int ctlr = 0;
	unsigned int irq_set = 0;
	unsigned int i = 0;

	/* disable first before going on */
	ctlr = gicd_v3_read_ctlr(gicd_base);
	ctlr &= ~(GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S);
	gicd_v3_write_ctlr(gicd_base, ctlr);
	gicd_v3_do_wait_for_rwp(gicd_base);

	/* query how many irq sets in our implementation,
	 * the formula is found in section 4.3.2 GIC400 TRM */
	irq_set = (mmio_read_32(gicd_base+GICD_V3_TYPER)&0x1f)+1;

	/* set all SPI as non-secure group1 by default,
	 * index from 1, because GICE_V3_IGROUP0, GICE_V3_IGRPMOD0 are RES0,
	 * equivalent function is provided by GICR_IGROUPR0, GICE_V3_IGRPMOD0,
	 * which are both initialized in gic_cpuif_init() */
	for (i = 1; i < irq_set; i++) {
		mmio_write_32(gicd_base+GICE_V3_IGROUP0+(i<<2), 0xffffffff);
		mmio_write_32(gicd_base+GICE_V3_IGRPMOD0+(i<<2), 0x0);
	}

	/* enable all groups & ARE */
	ctlr = GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S |
		GICD_V3_CTLR_ARE_S | GICD_V3_CTLR_ARE_NS;

	gicd_v3_write_ctlr(gicd_base, ctlr);
	gicd_v3_do_wait_for_rwp(gicd_base);
}

void mt_bl31_spi_enable(uint32_t fiq_num, uint32_t int_group, uint32_t trigger_type)
{

	uintptr_t gicd_base = MT_GIC_BASE;

	assert(fiq_num >= GIC_PRIVATE_SIGNALS);
	/* Configure this interrupt as a secure interrupt */
	gicd_clr_igroupr(gicd_base, fiq_num);

	/* Configure this interrupt as G0 or a G1S interrupt */
	if (int_group == INTR_GROUP1S)
		gicd_set_igrpmodr(gicd_base, fiq_num);
	else
		gicd_clr_igrpmodr(gicd_base, fiq_num);

	/* give FIQ the highest priority */
	gicd_set_ipriorityr(gicd_base, fiq_num, GIC_HIGHEST_SEC_PRIORITY);
	/* use 1-N model, means as long as one of N core can handle, this will be handled */
	gicd_write_irouter(gicd_base, fiq_num, GICD_V3_IROUTER_SPI_MODE_ANY);

	/* mt_irq_set_sens(gicd_base, fiq_num, GET_INT_EDGE_TYPE(trigger_type)); */
	/* mt_irq_set_polarity(fiq_num, GET_INT_POLARITY_TYPE(trigger_type)); */

	gicd_set_isenabler(gicd_base, fiq_num);
}

void setup_int_schedule_mode(enum irq_schedule_mode mode, unsigned int active_cpu)
{
	assert(mode <= HW_MODE);
	assert(active_cpu <= 0xFF);

	if (mode == HW_MODE) {
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_SEL_MASK))
									| (0x1 << GIC500_ACTIVE_SEL_SHIFT));
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_CPU_MASK))
									| (active_cpu << GIC500_ACTIVE_CPU_SHIFT));
	} else if (mode == SW_MODE) {
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_SEL_MASK)));
		mmio_write_32(GIC_INT_MASK, (mmio_read_32(GIC_INT_MASK) & ~(GIC500_ACTIVE_CPU_MASK))
									| (active_cpu << GIC500_ACTIVE_CPU_SHIFT));
	}
	return;
}

void int_schedule_mode_save(void)
{
	gic_data[0].saved_active_sel = mmio_read_32(GIC_INT_MASK);
}

void int_schedule_mode_restore(void)
{
	mmio_write_32(GIC_INT_MASK, gic_data[0].saved_active_sel);
}

void clear_sec_pol_ctl_en(void)
{
	unsigned int i;

	/* total 19 polarity ctrl registers */
	for (i = 0; i <= NR_INT_POL_CTL-1; i++) {
		mmio_write_32((SEC_POL_CTL_EN0 + (i * 4)), 0);
	}
	dsb();
}

void gic_setup(void)
{
	unsigned int gicd_base = 0;
	int	i;

	/* Init cpu logic map to avoid unnecessary SGI interrupt
		to cpu0 if WDT happens before cpu1~cpu7 on */
	for (i = 0 ; i < sizeof(cpu_logical_map)/sizeof(uint64_t); i++)
		cpu_logical_map[i] = CPU_LOGIC_MAP_RESET_VAL;
	gicd_base = MT_GIC_BASE;
	gic_distif_init(gicd_base);
	gic_cpuif_init();

	setup_int_schedule_mode(SW_MODE, 0xF);
	clear_sec_pol_ctl_en();
}

/*******************************************************************************
 * An ARM processor signals interrupt exceptions through the IRQ and FIQ pins.
 * The interrupt controller knows which pin/line it uses to signal a type of
 * interrupt. The platform knows which interrupt controller type is being used
 * in a particular security state e.g. with an ARM GIC, normal world could use
 * the GICv2 features while the secure world could use GICv3 features and vice
 * versa.
 * This function is exported by the platform to let the interrupt management
 * framework determine for a type of interrupt and security state, which line
 * should be used in the SCR_EL3 to control its routing to EL3. The interrupt
 * line is represented as the bit position of the IRQ or FIQ bit in the SCR_EL3.
 ******************************************************************************/
uint32_t plat_interrupt_type_to_line(uint32_t type, uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(security_state == NON_SECURE || security_state == SECURE);

	/*
	 * We ignore the security state parameter under the assumption that
	 * both normal and secure worlds are using ARM GICv2. This parameter
	 * will be used when the secure world starts using GICv3.
	 */
	/* Non-secure interrupts are signalled on the IRQ line always */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/* since we have enabled FIQ and trap FIQ in EL3 only,
	 * when type is not INTR_TYPE_NS, should
	 */
	return __builtin_ctz(SCR_FIQ_BIT);
}

/*******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_TYPE_INVAL is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_type(void)
{
	uint64_t irq = 0;

	irq = read_icc_hppir0_el1()&0x3FF;
	switch (irq) {
	case PENDING_G1S_INTID:
		return INTR_TYPE_S_EL1;
	case PENDING_G1NS_INTID:
		return INTR_TYPE_NS;
	case GIC_SPURIOUS_INTERRUPT:
		return INTR_TYPE_INVAL;
	default:
		return INTR_TYPE_EL3;
	}
}

/*******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC cpu interface. INTR_ID_UNAVAILABLE is returned when there is no
 * interrupt pending.
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_id(void)
{
	uint32_t irq = 0;

	irq = read_icc_hppir0_el1()&0x3FF;
	if (irq < PENDING_G1S_INTID) {
		return irq;
	}

	if (irq == GIC_SPURIOUS_INTERRUPT) {
		return INTR_ID_UNAVAILABLE;
	}

	/*
	 * Find out which non-secure interrupt it is under the assumption that
	 * the ICC_CTLR_EL3.RM is 0
	 */
	return (read_icc_hppir1_el1()&0x3ff);
}

/*******************************************************************************
 * This functions reads the GIC cpu interface Interrupt Acknowledge register
 * to start handling the pending interrupt. It returns the contents of the IAR.
 ******************************************************************************/
uint32_t plat_ic_acknowledge_interrupt(void)
{
	return read_icc_iar0_el1();
}

/*******************************************************************************
 * This functions writes the GIC cpu interface End Of Interrupt register with
 * the passed value to finish handling the active interrupt
 ******************************************************************************/
void plat_ic_end_of_interrupt(uint32_t id)
{
	write_icc_eoir0_el1(id);
}

/*******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1.
 ******************************************************************************/
uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	group = gicd_get_igroupr(MT_GIC_BASE, id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	/* group == GRP0 */
	if (group == 0)
		return INTR_TYPE_S_EL1;

	return INTR_TYPE_NS;
}

uint64_t mt_irq_dump_status(uint32_t irq)
{
	uint32_t dist_base;
	unsigned int bit;
	uint32_t result;
	uint64_t rc = 0;

	if (irq < 32) {
		return 0;
	}

	if (irq > 1020) {
		return 0;
	}

	dist_base = MT_GIC_BASE;

	/* get mask */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISENABLER + irq / 32 * 4) & bit)?1:0);
	rc |= result;

	/*get group*/
	bit = 1 << (irq % 32);
	/* 0x1:irq,0x0:fiq */
	result = ((mmio_read_32(dist_base + GICD_IGROUPR + irq / 32 * 4) & bit)?1:0);
	rc |=  result << 1;

	/* get priority */
	bit = 0xff << ((irq % 4)*8);
	result = ((mmio_read_32(dist_base + GICD_IPRIORITYR + irq / 4 * 4) & bit) >> ((irq % 4)*8));
	rc |= result << 2;

	/* get sensitivity */
	bit = 0x3 << ((irq % 16)*2);
	/* edge:0x2, level:0x1 */
	result = ((mmio_read_32(dist_base + GICD_ICFGR + irq / 16 * 4) & bit) >> ((irq % 16)*2));
	rc |= (result>>1) << 10;

	/* get pending status */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISPENDR + irq / 32 * 4) & bit)?1:0);
	rc |= result << 11;

	/* get active status */
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(dist_base + GICD_ISACTIVER + irq / 32 * 4) & bit)?1:0);
	rc |= result << 12;

	/* get polarity */
	{
		uint32_t reg = ((irq-32)/32);

		if (reg >= 8) {
			reg -= 8;
			reg += 0x70/4;
		}

		bit = 1 << (irq % 32);
		/* 0x0: high, 0x1:low */
		result = ((mmio_read_32(INT_POL_CTL0 + reg*4) & bit)?1:0);
		rc |= result << 13;
	}

	/* get router, occupy 16bit in rc [14:29] */
	{
		uint64_t route = mmio_read_64(dist_base + GICD_V3_IROUTER + (irq*8));
		uint16_t cpu = 4*((route&0xff00)>>8) + (route&0xff);

		if (route & GICD_V3_IROUTER_SPI_MODE_ANY) {
			rc |= 0xffff << 14;
		} else {
			rc |= (1<<cpu) << 14;
		}
	}

	return rc;
}

uint32_t mt_irq_get_pending(uint32_t irq)
{
	uint32_t bit = 1 << (irq % 32);

	return (mmio_read_32(BASE_GICD_BASE + GICD_ISPENDR + irq / 32 * 4) & bit) ? 1 : 0;
}


void mt_irq_set_pending(uint32_t irq)
{
	uint32_t bit = 1 << (irq % 32);

	mmio_write_32(BASE_GICD_BASE + GICD_ISPENDR + irq / 32 * 4, bit);
}

/*
 * mt_irq_mask_all: disable all interrupts
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 * (This is ONLY used for the idle current measurement by the factory mode.)
 */
int32_t mt_irq_mask_all(mtk_irq_mask_t *mask)
{
	unsigned int rdist_sgi_base = 0;

	assert(gic_populate_rdist((unsigned int *)&rdist_sgi_base) != -1);
	if (!mask)
		return -1;

	rdist_sgi_base += SZ_64K;
	mask->mask0 = mmio_read_32((rdist_sgi_base + GICR_ISENABLER));
	mask->mask1 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x4));
	mask->mask2 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x8));
	mask->mask3 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0xC));
	mask->mask4 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x10));
	mask->mask5 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x14));
	mask->mask6 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x18));
	mask->mask7 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x1C));
	mask->mask8 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x20));
	mask->mask9 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x24));
	mask->mask10 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x28));

	mmio_write_32((BASE_GICR_BASE + GICR_ICENABLER), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x4), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x8), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0xc), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x10), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x14), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x18), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x1c), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x20), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x24), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x28), 0xFFFFFFFF);

	mask->header = IRQ_MASK_HEADER;
	mask->footer = IRQ_MASK_FOOTER;

	return 0;
}

/*
 * mt_irq_mask_restore: restore all interrupts
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 * (This is ONLY used for the idle current measurement by the factory mode.)
 */
int32_t mt_irq_mask_restore(struct mtk_irq_mask *mask)
{
	unsigned int rdist_sgi_base = 0;

	assert(gic_populate_rdist((unsigned int *)&rdist_sgi_base) != -1);
	if (!mask)
		return -1;
	if (mask->header != IRQ_MASK_HEADER)
		return -1;
	if (mask->footer != IRQ_MASK_FOOTER)
		return -1;

	gic_distif_init(MT_GIC_BASE);
	rdist_sgi_base += SZ_64K;
	mmio_write_32((rdist_sgi_base + GICR_ISENABLER), mask->mask0);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x4), mask->mask1);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x8), mask->mask2);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0xC), mask->mask3);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x10), mask->mask4);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x14), mask->mask5);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x18), mask->mask6);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x1C), mask->mask7);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x20), mask->mask8);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x24), mask->mask9);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x28), mask->mask10);

	return 0;
}

/*
 * mt_irq_mask_for_sleep: disable an interrupt for the sleep manager's use
 * @irq: interrupt id
 * (THIS IS ONLY FOR SLEEP FUNCTION USE. DO NOT USE IT YOURSELF!)
 */
void mt_irq_mask_for_sleep(uint32_t irq)
{
	uint32_t mask;

	mask = 1 << (irq % 32);
	if (irq < 16) {
		ERROR("Fail to enable interrupt %d\n", irq);
		return;
	}

	mmio_write_32(BASE_GICD_BASE + GICD_ICENABLER + irq / 32 * 4, mask);
}

/*
 * mt_irq_unmask_for_sleep: enable an interrupt for the sleep manager's use
 * @irq: interrupt id
 * (THIS IS ONLY FOR SLEEP FUNCTION USE. DO NOT USE IT YOURSELF!)
 */
void mt_irq_unmask_for_sleep(int32_t irq)
{

	uint32_t mask;

	mask = 1 << (irq % 32);
	if (irq < 16) {
		ERROR("Fail to enable interrupt %d\n", irq);
		return;
	}

	mmio_write_32(BASE_GICD_BASE + GICD_ISENABLER + irq / 32 * 4, mask);
}

void gic_sync_dcm_enable(void)
{
	unsigned int val = mmio_read_32(GIC_SYNC_DCM);

	val &= ~GIC_SYNC_DCM_MASK;
	mmio_write_32(GIC_SYNC_DCM, val | GIC_SYNC_DCM_ON);
}

void gic_sync_dcm_disable(void)
{
	unsigned int val = mmio_read_32(GIC_SYNC_DCM);

	val &= ~GIC_SYNC_DCM_MASK;
	mmio_write_32(GIC_SYNC_DCM, val | GIC_SYNC_DCM_OFF);
}
