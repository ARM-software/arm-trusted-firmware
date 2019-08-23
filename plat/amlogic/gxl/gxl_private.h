/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GXBB_PRIVATE_H
#define GXBB_PRIVATE_H

#include <stdint.h>
#include <stddef.h>

/* Utility functions */
unsigned int plat_gxbb_calc_core_pos(u_register_t mpidr);
void gxbb_console_init(void);
void gxbb_setup_page_tables(void);

/* MHU functions */
void mhu_secure_message_start(void);
void mhu_secure_message_send(uint32_t msg);
uint32_t mhu_secure_message_wait(void);
void mhu_secure_message_end(void);
void mhu_secure_init(void);

/* SCPI functions */
void scpi_set_css_power_state(u_register_t mpidr, uint32_t cpu_state,
			      uint32_t cluster_state, uint32_t css_state);
uint32_t scpi_sys_power_state(uint64_t system_state);
void scpi_jtag_set_state(uint32_t state, uint8_t select);
uint32_t scpi_efuse_read(void *dst, uint32_t base, uint32_t size);
void scpi_unknown_thermal(uint32_t arg0, uint32_t arg1,
			  uint32_t arg2, uint32_t arg3);
void scpi_upload_scp_fw(uintptr_t addr, size_t size, int send);

/* Peripherals */
void gxbb_thermal_unknown(void);
uint64_t gxbb_efuse_read(void *dst, uint32_t offset, uint32_t size);
uint64_t gxbb_efuse_user_max(void);

#endif /* GXBB_PRIVATE_H */
