/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <context.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/coreboot.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <platform.h>
#include <qti_plat.h>
#include <qtiseclib_cb_interface.h>

void *qtiseclib_cb_memcpy(void *dst, const void *src, size_t len)
{
	return memcpy(dst, src, len);
}

int qtiseclib_cb_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}

void *qtiseclib_cb_memset(void *s, int c, size_t n)
{
	return memset(s, c, n);
}

void *qtiseclib_cb_memmove(void *dest, const void *src, size_t n)
{
	return memmove(dest, src, n);
}

/* Printing logs below or equal LOG_LEVEL from QTISECLIB. */
void qtiseclib_cb_log(unsigned int loglvl, const char *fmt, ...)
{
	if (loglvl <= LOG_LEVEL) {
		va_list argp;
		static spinlock_t qti_log_lock;
		uint64_t uptime = read_cntpct_el0();

		va_start(argp, fmt);

		spin_lock(&qti_log_lock);
		printf("QTISECLIB [%x%08x]",
		       (uint32_t) ((uptime >> 32) & 0xFFFFFFFF),
		       (uint32_t) (uptime & 0xFFFFFFFF));
		vprintf(fmt, argp);
		putchar('\n');
		spin_unlock(&qti_log_lock);

		va_end(argp);
	}
}

void qtiseclib_cb_spin_lock(qtiseclib_cb_spinlock_t *lock)
{
	spin_lock((spinlock_t *) lock);
}

void qtiseclib_cb_spin_unlock(qtiseclib_cb_spinlock_t *lock)
{
	spin_unlock((spinlock_t *) lock);
}

unsigned int qtiseclib_cb_plat_my_core_pos(void)
{
	return plat_my_core_pos();
}

int qtiseclib_cb_plat_core_pos_by_mpidr(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

unsigned int qtiseclib_cb_plat_my_cluster_pos(void)
{
	return plat_qti_my_cluster_pos();
}

/* GIC platform functions */
void qtiseclib_cb_gic_pcpu_init(void)
{
	plat_qti_gic_pcpu_init();
}

void qtiseclib_cb_ic_raise_sgi(int sgi_num, u_register_t target)
{
	plat_ic_raise_el3_sgi(sgi_num, target);
}

void qtiseclib_cb_set_spi_routing(unsigned int id, unsigned int irm,
				  u_register_t target)
{
	assert(QTI_GICV3_IRM_PE == GICV3_IRM_PE);
	assert(QTI_GICV3_IRM_ANY == GICV3_IRM_ANY);
	gic_set_spi_routing(id, irm, target);
}

/* Crash reporting api's wrappers */
void qtiseclib_cb_switch_console_to_crash_state(void)
{
	console_switch_state(CONSOLE_FLAG_CRASH);
}

void qtiseclib_cb_udelay(uint32_t usec)
{
	udelay(usec);
}

void qtiseclib_cb_console_flush(void)
{
	return console_flush();
}

#if QTI_SDI_BUILD
void qtiseclib_cb_get_ns_ctx(qtiseclib_dbg_a64_ctxt_regs_type *qti_ns_ctx)
{
	void *ctx;

	ctx = cm_get_context(NON_SECURE);
	if (ctx) {
		/* nothing to be done w/o ns context */
		return;
	}

	qti_ns_ctx->spsr_el3 =
	    read_ctx_reg(get_el3state_ctx(ctx), CTX_SPSR_EL3);
	qti_ns_ctx->elr_el3 = read_ctx_reg(get_el3state_ctx(ctx), CTX_ELR_EL3);

	qti_ns_ctx->spsr_el1 =
	    read_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_SPSR_EL1);
	qti_ns_ctx->elr_el1 =
	    read_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_ELR_EL1);
	qti_ns_ctx->sp_el1 = read_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_SP_EL1);

	qti_ns_ctx->x0 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0);
	qti_ns_ctx->x1 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X1);
	qti_ns_ctx->x2 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X2);
	qti_ns_ctx->x3 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X3);
	qti_ns_ctx->x4 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X4);
	qti_ns_ctx->x5 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X5);
	qti_ns_ctx->x6 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X6);
	qti_ns_ctx->x7 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X7);
	qti_ns_ctx->x8 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X8);
	qti_ns_ctx->x9 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X9);
	qti_ns_ctx->x10 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X10);
	qti_ns_ctx->x11 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X11);
	qti_ns_ctx->x12 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X12);
	qti_ns_ctx->x13 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X13);
	qti_ns_ctx->x14 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X14);
	qti_ns_ctx->x15 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X15);
	qti_ns_ctx->x16 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X16);
	qti_ns_ctx->x17 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X17);
	qti_ns_ctx->x18 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X18);
	qti_ns_ctx->x19 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X19);
	qti_ns_ctx->x20 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X20);
	qti_ns_ctx->x21 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X21);
	qti_ns_ctx->x22 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X22);
	qti_ns_ctx->x23 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X23);
	qti_ns_ctx->x24 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X24);
	qti_ns_ctx->x25 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X25);
	qti_ns_ctx->x26 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X26);
	qti_ns_ctx->x27 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X27);
	qti_ns_ctx->x28 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X28);
	qti_ns_ctx->x29 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X29);
	qti_ns_ctx->x30 = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_LR);
	qti_ns_ctx->sp_el0 =
	    read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_SP_EL0);
}

void qtiseclib_cb_flush_dcache_all(void)
{
	dcsw_op_all(DCCISW);
}

int qtiseclib_cb_mmap_add_dynamic_region(unsigned long long base_pa,
					 size_t size,
					 qtiseclib_mmap_attr_t attr)
{
	unsigned int l_attr = 0;

	if (attr == QTISECLIB_MAP_NS_RO_XN_DATA) {
		l_attr = MT_NS | MT_RO | MT_EXECUTE_NEVER;
	} else if (attr == QTISECLIB_MAP_RW_XN_NC_DATA) {
		l_attr = MT_RW | MT_NON_CACHEABLE | MT_EXECUTE_NEVER;
	} else if (attr == QTISECLIB_MAP_RW_XN_DATA) {
		l_attr = MT_RW | MT_EXECUTE_NEVER;
	}
	return qti_mmap_add_dynamic_region(base_pa, size, l_attr);
}

int qtiseclib_cb_mmap_remove_dynamic_region(uintptr_t base_va, size_t size)
{
	return qti_mmap_remove_dynamic_region(base_va, size);
}
#endif

