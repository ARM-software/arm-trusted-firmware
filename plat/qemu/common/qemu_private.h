/*
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QEMU_PRIVATE_H
#define QEMU_PRIVATE_H

#include <stdint.h>

#include <lib/xlat_tables/xlat_tables_v2.h>

void plat_qemu_io_setup(void);
int qemu_io_register_sp_pkg(const char *name, const char *uuid,
			    uintptr_t load_addr);
unsigned int plat_qemu_calc_core_pos(u_register_t mpidr);
const mmap_region_t *plat_qemu_get_mmap(void);

void qemu_console_init(void);
#ifdef PLAT_qemu_sbsa
void sip_svc_init(void);
#endif

void plat_qemu_gic_init(void);
void qemu_pwr_gic_on_finish(void);
void qemu_pwr_gic_off(void);

int qemu_set_tos_fw_info(uintptr_t config_base, uintptr_t log_addr,
			size_t log_size);

int qemu_set_nt_fw_info(
/*
 * Currently OP-TEE does not support reading DTBs from Secure memory
 * and this option should be removed when feature is supported.
 */
#ifdef SPD_opteed
			uintptr_t log_addr,
#endif
			size_t log_size,
			uintptr_t *ns_log_addr);

void qemu_bl2_sync_transfer_list(void);

#endif /* QEMU_PRIVATE_H */
