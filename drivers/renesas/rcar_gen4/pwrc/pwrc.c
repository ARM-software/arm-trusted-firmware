/*
 * Copyright (c) 2015-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include "pwrc.h"
#include "timer.h"

#include "rcar_def.h"
#include "rcar_private.h"

#ifndef __ASSEMBLER__
IMPORT_SYM(uintptr_t, __system_ram_start__, SYSTEM_RAM_START);
IMPORT_SYM(uintptr_t, __system_ram_end__, SYSTEM_RAM_END);
IMPORT_SYM(uintptr_t, __SRAM_COPY_START__, SRAM_COPY_START);
#endif /*__ASSEMBLER__*/

#define	RCAR_CODE_COPY_NONE	0
#define	RCAR_CODE_COPY_DONE	1

static uint32_t dummy_sdram = 0xAAAAAAAA;
static uint32_t rcar_pwrc_code_copy_state;

/*
 * Someday there will be a generic power controller API. At the moment each
 * platform has its own PWRC so just exporting functions should be acceptable.
 */
static RCAR_INSTANTIATE_LOCK;

static u_register_t rcar_boot_mpidr;

/* APSREG boot configuration */
static uintptr_t apsreg_ap_cluster_aux0(uint32_t n)
{
	return APSREG_BASE + 0x10UL + ((n & 0x3) * 0x1000UL);
}

/* APMU */
static uintptr_t rcar_apmu_cluster_base(uint32_t n)
{
	return RCAR_APMU_BASE + 0x400UL + ((n & 0x3) * 0x40UL);
}

static uintptr_t rcar_apmu_cpu_base(uint32_t n)
{
	return RCAR_APMU_BASE + 0x800UL + ((n & 0x6) * 0x100UL) +
	       ((n & 0x1) * 0x40UL);
}

static uintptr_t rcar_apmu_pwrctrlcl(uint32_t n)
{
	return rcar_apmu_cluster_base(n);
}

static uintptr_t rcar_apmu_pwrctrlc(uint32_t n)
{
	return rcar_apmu_cpu_base(n);
}

static uintptr_t rcar_apmu_safectrlc(uint32_t n)
{
	return rcar_apmu_cpu_base(n) + 0x20UL;
}

static uintptr_t rcar_apmu_rvbarplc(uint32_t n)
{
	return rcar_apmu_cpu_base(n) + 0x38UL;
}

static uintptr_t rcar_apmu_rvbarphc(uint32_t n)
{
	return rcar_apmu_cpu_base(n) + 0x3cUL;
}

static uintptr_t rcar_apmu_fsmstsrc(uint32_t n)
{
	return rcar_apmu_cpu_base(n) + 0x18UL;
}

/* Product register */
static uint32_t prr_caxx_xx_en_cpu(uint32_t n)
{
	return BIT(n & 0x1);
}

static void write_cpupwrctlr(u_register_t v)
{
	__asm__ volatile ("msr S3_0_C15_C2_7, %0" : : "r" (v));
}

static uint32_t rcar_pwrc_core_pos(u_register_t mpidr)
{
	int cpu;

	cpu = plat_core_pos_by_mpidr(mpidr);
	if (cpu < 0) {
		ERROR("BL3-1 : The value of passed MPIDR is invalid.");
		panic();
	}

	return (uint32_t)cpu;
}

void rcar_pwrc_cpuon(u_register_t mpidr)
{
	uint32_t cluster, cpu;

	rcar_lock_get();

	cpu = rcar_pwrc_core_pos(mpidr);

	cluster = rcar_pwrc_get_mpidr_cluster(mpidr);

	/* clear Cluster OFF bit */
	mmio_clrbits_32(rcar_apmu_pwrctrlcl(cluster),
			RCAR_APMU_PWRCTRLCL_PCHPDNEN);

	/* clear Core OFF bit */
	mmio_clrbits_32(rcar_apmu_pwrctrlc(cpu), RCAR_APMU_PWRCTRLC_PCHPDNEN);
	while (mmio_read_32(rcar_apmu_pwrctrlc(cpu)) & RCAR_APMU_PWRCTRLC_PCHPDNEN)
		;

	mmio_setbits_32(rcar_apmu_pwrctrlc(cpu), RCAR_APMU_PWRCTRLC_WUP_REQ);

	/* Wait until CAXX wake up sequence finishes */
	while ((mmio_read_32(rcar_apmu_pwrctrlc(cpu)) & RCAR_APMU_PWRCTRLC_WUP_REQ) ==
	       RCAR_APMU_PWRCTRLC_WUP_REQ)
		;

	rcar_lock_release();

	/*
	 * mask should match the kernel's MPIDR_HWID_BITMASK so the core can be
	 * identified during cpuhotplug (check the kernel's psci migrate set of
	 * functions
	 */
	rcar_boot_mpidr = read_mpidr_el1() & RCAR_MPIDR_AFFMASK;
}

int32_t rcar_pwrc_cpu_migrate_info(u_register_t *resident_cpu)
{
	*resident_cpu = rcar_boot_mpidr;

	return PSCI_TOS_NOT_UP_MIG_CAP;
}

bool rcar_pwrc_mpidr_is_boot_cpu(u_register_t mpidr)
{
	return (mpidr & RCAR_MPIDR_AFFMASK) == rcar_boot_mpidr;
}

static void rcar_pwrc_cpuoff_sub(uint32_t cpu)
{
	/* Clear DBGGEN_PPDN bit for core down to 'OFF' mode */
	mmio_clrbits_32(rcar_apmu_safectrlc(cpu), RCAR_APMU_SAFECTRLC_DBGGEN);
	/* for Core OFF */
	mmio_setbits_32(rcar_apmu_pwrctrlc(cpu), RCAR_APMU_PWRCTRLC_PCHPDNEN);

	write_cpupwrctlr(CPUPWRCTLR_PWDN);
}

void rcar_pwrc_cpuoff(u_register_t mpidr)
{
	uint32_t cpu;

	rcar_lock_get();

	cpu = rcar_pwrc_core_pos(mpidr);

	rcar_pwrc_cpuoff_sub(cpu);

	rcar_lock_release();
}

void rcar_pwrc_enable_interrupt_wakeup(u_register_t mpidr)
{
	uint32_t cpu;

	rcar_lock_get();

	cpu = rcar_pwrc_core_pos(mpidr);

	mmio_setbits_32(rcar_apmu_pwrctrlc(cpu), RCAR_APMU_PWRCTRLC_IWUP_EN);

	rcar_lock_release();
}

void rcar_pwrc_disable_interrupt_wakeup(u_register_t mpidr)
{
	uint32_t cpu;

	rcar_lock_get();

	cpu = rcar_pwrc_core_pos(mpidr);

	mmio_clrbits_32(rcar_apmu_pwrctrlc(cpu), RCAR_APMU_PWRCTRLC_IWUP_EN);

	rcar_lock_release();
}

void rcar_pwrc_clusteroff(u_register_t mpidr)
{
	uint32_t cluster, cpu;

	rcar_lock_get();

	cpu = rcar_pwrc_core_pos(mpidr);

	cluster = rcar_pwrc_get_mpidr_cluster(mpidr);

	/* for Cluster OFF */
	mmio_setbits_32(rcar_apmu_pwrctrlcl(cluster),
			RCAR_APMU_PWRCTRLCL_PCHPDNEN);

	rcar_pwrc_cpuoff_sub(cpu);

	rcar_lock_release();
}

void rcar_pwrc_setup(void)
{
	uintptr_t rst_barh, rst_barl;
	uint32_t cpu, i, j, reg;
	uint64_t reset;

	reset = (uint64_t)(&plat_secondary_reset) & 0xFFFFFFFFU;
	reset &= RCAR_APMU_RVBARPLC_MASK;
	reset |= RCAR_APMU_RVBARPL_VLD;

	reg = mmio_read_32(RCAR_PRR) >> 17;
	for (i = 0; i < PLATFORM_CLUSTER_COUNT; i++) {
		reg >>= 3;

		if ((reg & PRR_CAXX_XX_EN_CLUSTER_MASK) != RCAR_CPU_HAVE_CAXX)
			continue;

		mmio_setbits_32(apsreg_ap_cluster_aux0(i),
			      APSREG_AP_CLUSTER_AUX0_INIT);

		for (j = 0; j < PLATFORM_MAX_CPUS_PER_CLUSTER; j++) {
			cpu = (i * PLATFORM_MAX_CPUS_PER_CLUSTER) + j;

			if ((reg & prr_caxx_xx_en_cpu(cpu)) != RCAR_CPU_HAVE_CAXX)
				continue;

			rst_barh = rcar_apmu_rvbarphc(cpu);
			rst_barl = rcar_apmu_rvbarplc(cpu);
			mmio_write_32(rst_barh, 0);
			mmio_write_32(rst_barl, (uint32_t)reset);
		}
	}

	mmio_setbits_32(APSREG_CCI500_AUX, APSREG_CCI500_AUX_INIT);
	mmio_setbits_32(APSREG_P_CCI500_AUX, APSREG_P_CCI500_AUX_INIT);

	rcar_lock_init();
}

uint32_t rcar_pwrc_get_mpidr_cluster(u_register_t mpidr)
{
	int32_t cluster = rcar_cluster_pos_by_mpidr(mpidr);

	if (cluster < 0) {
		ERROR("BL3-1 : The value of passed MPIDR is invalid.");
		panic();
	}

	return (uint32_t)cluster;
}

uint32_t rcar_pwrc_cpu_on_check(u_register_t mpidr)
{
	uint32_t core_pos, cpu, i, j, prr, state;
	uint32_t count = 0;

	core_pos = rcar_pwrc_core_pos(mpidr);
	prr = mmio_read_32(RCAR_PRR) >> 17;
	for (i = 0; i < PLATFORM_CLUSTER_COUNT; i++) {
		prr >>= 3;

		/* check the cluster has cores */
		if ((prr & PRR_CAXX_XX_EN_CLUSTER_MASK) != RCAR_CPU_HAVE_CAXX)
			continue;

		for (j = 0; j < PLATFORM_MAX_CPUS_PER_CLUSTER; j++) {
			cpu = (i * PLATFORM_MAX_CPUS_PER_CLUSTER) + j;

			/* check the core be implemented */
			if ((prr & prr_caxx_xx_en_cpu(cpu)) != RCAR_CPU_HAVE_CAXX)
				continue;

			if (core_pos != cpu) {
				state = mmio_read_32(rcar_apmu_fsmstsrc(cpu));
				if (state != RCAR_APMU_FSMSTSRC_STATE_OFF)
					count++;
			}
		}
	}

	return count;
}

static void __attribute__ ((section(".system_ram"), no_stack_protector))
	rcar_pwrc_set_self_refresh(void)
{
	uint64_t base_count, freq, get_count, wait_time;
	uint32_t reg;

	/* Enable DBSC4 register access */
	mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_ENABLE);

	/* DFI_PHYMSTR_ACK setting */
	mmio_clrbits_32(DBSC4_REG_DBDFIPMSTRCNF, DBSC4_BIT_DBDFIPMSTRCNF_PMSTREN);

	/* Set the Self-Refresh mode */

	/* Disable access to the SDRAM */
	mmio_write_32(DBSC4_REG_DBACEN, 0);

	/* Flush the access request in DBSC */
	mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);

	/* Wait succeed to flush */
	freq = read_cntfrq_el0();
	base_count = read_cntpct_el0();

	while ((mmio_read_32(DBSC4_REG_DBCAM0STAT0) & DBSC4_BIT_DBCAM0STAT0)
			!= DBSC4_BIT_DBCAM0STAT0) {

		get_count = read_cntpct_el0();
		wait_time = ((get_count - base_count) * RCAR_CONV_MICROSEC) / freq;

		/* Get base counter */
		if (wait_time >= RCAR_WAIT_DBCS4_FLUSH) {

			/* Stop flushing and enable access to SDRAM */
			mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 0);
			mmio_write_32(DBSC4_REG_DBACEN, 1);

			/* Dummy write to SDRAM */
			dummy_sdram = ~dummy_sdram;

			/* Re-Disable access and flush */
			mmio_write_32(DBSC4_REG_DBACEN, 0);
			mmio_write_32(DBSC4_REG_DBCAM0CTRL0, 1);

			/* refresh base counter */
			base_count = read_cntpct_el0();
		}
	}

	/* Clear the SDRAM calibration configuration register */
	mmio_write_32(DBSC4_REG_DBCALCNF, 0);

	/* Issue Precharge All (PREA) command */
	reg = DBSC4_SET_DBCMD_OPC_PRE | DBSC4_SET_DBCMD_CH_ALL |
	      DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ALL;
	mmio_write_32(DBSC4_REG_DBCMD, reg);

	/* Wait to complete PREA operation */
	while (mmio_read_32(DBSC4_REG_DBWAIT) != 0)
		;

	/* Issue Self-Refresh Entry (SRE) command   */
	reg = DBSC4_SET_DBCMD_OPC_SR | DBSC4_SET_DBCMD_CH_ALL |
	      DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ENTER;
	mmio_write_32(DBSC4_REG_DBCMD, reg);

	/* Wait to complete SRE operation */
	while (mmio_read_32(DBSC4_REG_DBWAIT) != 0)
		;

	/* Issue Mode Register 11 (MR11) write command. (ODT disabled)  */
	reg = DBSC4_SET_DBCMD_OPC_MRW | DBSC4_SET_DBCMD_CH_ALL |
	      DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_MRW_ODTC;
	mmio_write_32(DBSC4_REG_DBCMD, reg);

	/* Wait to complete MR11 operation */
	while (mmio_read_32(DBSC4_REG_DBWAIT) != 0)
		;

	/* Issue Power Down (PD) command */
	reg = DBSC4_SET_DBCMD_OPC_PD | DBSC4_SET_DBCMD_CH_ALL |
	      DBSC4_SET_DBCMD_RANK_ALL | DBSC4_SET_DBCMD_ARG_ENTER;
	mmio_write_32(DBSC4_REG_DBCMD, reg);

	/* Wait to complete PD operation */
	while (mmio_read_32(DBSC4_REG_DBWAIT) != 0)
		;

	/* Issue set the Auto-Refresh Enable register */
	/* to stop the auto-refresh function */
	mmio_write_32(DBSC4_REG_DBRFEN, 0);

	/* Dummy read DBWAIT register to wait tCKELPD time */
	(void)mmio_read_32(DBSC4_REG_DBWAIT);

	/* Disable DBSC4 register access */
	mmio_write_32(DBSC4_REG_DBSYSCNT0, DBSC4_SET_DBSYSCNT0_WRITE_DISABLE);
}

static void __attribute__ ((section(".system_ram"), no_stack_protector))
		rcar_pwrc_go_suspend_to_ram(void)
{
	rcar_pwrc_set_self_refresh();

	wfi();

	/* Do not return */
	while (true)
		;
}

void rcar_pwrc_suspend_to_ram(void)
{
	uintptr_t jump = (uintptr_t) rcar_pwrc_go_suspend_to_ram;
	uintptr_t stack = (uintptr_t) (DEVICE_SRAM_STACK_BASE +
				       DEVICE_SRAM_STACK_SIZE);

	rcar_pwrc_save_timer_state();

	/* Clear code copy state to execute copy on next boot time */
	rcar_pwrc_code_copy_state = RCAR_CODE_COPY_NONE;

	/* disable MMU */
	disable_mmu_el3();

	/* cache flush */
	dcsw_op_all(DCCISW);

	(void)rcar_pwrc_switch_stack(jump, stack, NULL);
}

void rcar_pwrc_code_copy_to_system_ram(void)
{
	int __maybe_unused ret;
	uint32_t attr;
	struct {
		uintptr_t	base;
		size_t		len;
	} sram = {
		.base	= (uintptr_t) DEVICE_SRAM_BASE,
		.len	= DEVICE_SRAM_SIZE
	}, code = {
		.base	= (uintptr_t) SRAM_COPY_START,
		.len	= (size_t)(SYSTEM_RAM_END - SYSTEM_RAM_START)
	};

	/*
	 * The copy of the code should only be executed for ColdBoot,
	 * and for WarmBoot from SuspendToRAM.
	 */
	if (rcar_pwrc_code_copy_state == RCAR_CODE_COPY_DONE) {
		/* No need to run because it has already been copied */
		return;
	}

	rcar_pwrc_code_copy_state = RCAR_CODE_COPY_DONE;
	flush_dcache_range((uintptr_t)(&rcar_pwrc_code_copy_state),
			   sizeof(rcar_pwrc_code_copy_state));

	attr = MT_MEMORY | MT_RW | MT_SECURE | MT_EXECUTE_NEVER;
	ret = xlat_change_mem_attributes(sram.base, sram.len, attr);
	assert(ret == 0);

	(void)memcpy((void *)sram.base, (void *)code.base, code.len);
	flush_dcache_range(sram.base, code.len);

	/* Invalidate instruction cache */
	plat_invalidate_icache();
	dsb();
	isb();

	attr = MT_MEMORY | MT_RO | MT_SECURE | MT_EXECUTE;
	ret = xlat_change_mem_attributes(sram.base, sram.len, attr);
	assert(ret == 0);

	/* clean up data and stack area in system ram */
	(void)memset((void *)DEVICE_SRAM_DATA_BASE, 0x0,
		     DEVICE_SRAM_DATA_SIZE + DEVICE_SRAM_STACK_SIZE);
	flush_dcache_range((uintptr_t)DEVICE_SRAM_DATA_BASE,
			   DEVICE_SRAM_DATA_SIZE + DEVICE_SRAM_STACK_SIZE);
}
