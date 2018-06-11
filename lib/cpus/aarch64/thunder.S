/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cpu_macros.S>
#include <plat_macros.S>

func thunder_reset_func
	ret
endfunc thunder_reset_func

func thunder_core_pwr_dwn
	ret
endfunc thunder_core_pwr_dwn

func thunder_cluster_pwr_dwn
	ret
endfunc thunder_cluster_pwr_dwn

func thunder_cpu_reg_dump
	ret
endfunc thunder_cpu_reg_dump

declare_cpu_ops thunder, THUNDER81_MIDR, \
	thunder_reset_func, \
	thunder_core_pwr_dwn, \
	thunder_cluster_pwr_dwn

declare_cpu_ops thunder, THUNDER83_MIDR, \
	thunder_reset_func, \
	thunder_core_pwr_dwn, \
	thunder_cluster_pwr_dwn

declare_cpu_ops thunder, THUNDER88_MIDR, \
	thunder_reset_func, \
	thunder_core_pwr_dwn, \
	thunder_cluster_pwr_dwn
