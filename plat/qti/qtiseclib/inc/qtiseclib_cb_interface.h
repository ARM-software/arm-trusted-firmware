/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTISECLIB_CB_INTERFACE_H
#define QTISECLIB_CB_INTERFACE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <qtiseclib_defs.h>

/* Standard Library API's */
void *qtiseclib_cb_memcpy(void *dst, const void *src, size_t len);
int qtiseclib_cb_strcmp(const char *s1, const char *s2);
void *qtiseclib_cb_memset(void *s, int c, size_t n);
void *qtiseclib_cb_memmove(void *dest, const void *src, size_t n);

#define QTISECLIB_CB_ERROR(...)		qtiseclib_cb_log(QTISECLIB_LOG_LEVEL_ERROR, __VA_ARGS__)
#define QTISECLIB_CB_NOTICE(...)	qtiseclib_cb_log(QTISECLIB_LOG_LEVEL_NOTICE, __VA_ARGS__)
#define QTISECLIB_CB_WARN(...)		qtiseclib_cb_log(QTISECLIB_LOG_LEVEL_WARNING, __VA_ARGS__)
#define QTISECLIB_CB_INFO(...)		qtiseclib_cb_log(QTISECLIB_LOG_LEVEL_INFO, __VA_ARGS__)

void qtiseclib_cb_log(unsigned int loglvl, const char *fmt, ...);

void qtiseclib_cb_spin_lock(qtiseclib_cb_spinlock_t *lock);
void qtiseclib_cb_spin_unlock(qtiseclib_cb_spinlock_t *lock);

unsigned int qtiseclib_cb_plat_my_core_pos(void);
int qtiseclib_cb_plat_core_pos_by_mpidr(u_register_t mpidr);
unsigned int qtiseclib_cb_plat_my_cluster_pos(void);

/* GIC platform wrappers */
void qtiseclib_cb_gic_pcpu_init(void);
void qtiseclib_cb_ic_raise_sgi(int sgi_num, u_register_t target);
void qtiseclib_cb_set_spi_routing(unsigned int id, unsigned int irm,
				  u_register_t target);
/* Crash reporting api's wrappers */
void qtiseclib_cb_switch_console_to_crash_state(void);

void qtiseclib_cb_udelay(uint32_t usec);

void qtiseclib_cb_console_flush(void);

#if QTI_SDI_BUILD
int qtiseclib_cb_mmap_remove_dynamic_region(uintptr_t base_va, size_t size);
int qtiseclib_cb_mmap_add_dynamic_region(unsigned long long base_pa,
					 size_t size,
					 qtiseclib_mmap_attr_t attr);

void qtiseclib_cb_flush_dcache_all(void);
void qtiseclib_cb_get_ns_ctx(qtiseclib_dbg_a64_ctxt_regs_type *ns_ctx);
#endif

#endif /* QTISECLIB_CB_INTERFACE_H */
