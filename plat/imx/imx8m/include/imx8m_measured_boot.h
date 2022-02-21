/*
 * Copyright (c) 2022, Linaro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8M_MEASURED_BOOT_H
#define IMX8M_MEASURED_BOOT_H

#include <stdint.h>

#include <arch_helpers.h>

int imx8m_set_nt_fw_info(size_t log_size, uintptr_t *ns_log_addr);

#endif /* IMX8M_MEASURED_BOOT_H */
