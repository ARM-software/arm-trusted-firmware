/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_IPI_H
#define PM_IPI_H

#include <plat_ipi.h>
#include "pm_common.h"

#define IPI_BLOCKING		1
#define IPI_NON_BLOCKING	0

int pm_ipi_init(const struct pm_proc *proc);

enum pm_ret_status pm_ipi_send(const struct pm_proc *proc,
			       uint32_t payload[PAYLOAD_ARG_CNT]);
enum pm_ret_status pm_ipi_send_non_blocking(const struct pm_proc *proc,
					    uint32_t payload[PAYLOAD_ARG_CNT]);
enum pm_ret_status pm_ipi_send_sync(const struct pm_proc *proc,
				    uint32_t payload[PAYLOAD_ARG_CNT],
				    unsigned int *value, size_t count);
void pm_ipi_buff_read_callb(unsigned int *value, size_t count);
void pm_ipi_irq_enable(const struct pm_proc *proc);
void pm_ipi_irq_clear(const struct pm_proc *proc);
uint32_t pm_ipi_irq_status(const struct pm_proc *proc);

#endif /* PM_IPI_H */
