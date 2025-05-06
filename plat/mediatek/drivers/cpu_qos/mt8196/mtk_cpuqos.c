/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/pm/mtk_pm.h>
#include <mtk_bl31_interface.h>

static void *cpu_qos_handle_cluster_on_event_cb(const void *arg)
{
	return cpu_qos_handle_cluster_on_event(arg);
}

MT_CPUPM_SUBCRIBE_CLUSTER_PWR_ON(cpu_qos_handle_cluster_on_event_cb);
