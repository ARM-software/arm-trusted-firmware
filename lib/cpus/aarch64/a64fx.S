/*
 * Copyright (c) 2022, Fujitsu Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <a64fx.h>
#include <cpu_macros.S>
#include <plat_macros.S>

func a64fx_core_pwr_dwn
endfunc a64fx_core_pwr_dwn

func a64fx_cluster_pwr_dwn
endfunc a64fx_cluster_pwr_dwn

cpu_reset_func_start a64fx
cpu_reset_func_end a64fx


declare_cpu_ops a64fx, A64FX_MIDR, a64fx_reset_func \
                a64fx_core_pwr_dwn, \
                a64fx_cluster_pwr_dwn
