/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AML_PRIVATE_H
#define AML_PRIVATE_H

#include <stddef.h>
#include <stdint.h>

/* Utility functions */
unsigned int plat_calc_core_pos(u_register_t mpidr);
void aml_console_init(void);
void aml_setup_page_tables(void);

/* MHU functions */
void aml_mhu_secure_message_start(void);
void aml_mhu_secure_message_send(uint32_t msg);
uint32_t aml_mhu_secure_message_wait(void);
void aml_mhu_secure_message_end(void);
void aml_mhu_secure_init(void);

/* SCPI functions */
void aml_scpi_set_css_power_state(u_register_t mpidr, uint32_t cpu_state,
				  uint32_t cluster_state, uint32_t css_state);
uint32_t aml_scpi_sys_power_state(uint64_t system_state);
void aml_scpi_jtag_set_state(uint32_t state, uint8_t select);
uint32_t aml_scpi_efuse_read(void *dst, uint32_t base, uint32_t size);
void aml_scpi_unknown_thermal(uint32_t arg0, uint32_t arg1,
			      uint32_t arg2, uint32_t arg3);
void aml_scpi_upload_scp_fw(uintptr_t addr, size_t size, int send);
uint32_t aml_scpi_get_chip_id(uint8_t *obuff, uint32_t osize);

/* Peripherals */
void aml_thermal_unknown(void);
uint64_t aml_efuse_read(void *dst, uint32_t offset, uint32_t size);
uint64_t aml_efuse_user_max(void);

#endif /* AML_PRIVATE_H */
