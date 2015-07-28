/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TEGRA_DEF_H__
#define __TEGRA_DEF_H__

#include <platform_def.h>

/*******************************************************************************
 * Power down state IDs
 ******************************************************************************/
#define PSTATE_ID_CORE_POWERDN		7
#define PSTATE_ID_CLUSTER_IDLE		16
#define PSTATE_ID_CLUSTER_POWERDN	17
#define PSTATE_ID_SOC_POWERDN		27

/*******************************************************************************
 * This value is used by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call as the `state-id` field in the 'power state' parameter.
 ******************************************************************************/
#define PLAT_SYS_SUSPEND_STATE_ID	PSTATE_ID_SOC_POWERDN

/*******************************************************************************
 * Implementation defined ACTLR_EL3 bit definitions
 ******************************************************************************/
#define ACTLR_EL3_L2ACTLR_BIT		(1 << 6)
#define ACTLR_EL3_L2ECTLR_BIT		(1 << 5)
#define ACTLR_EL3_L2CTLR_BIT		(1 << 4)
#define ACTLR_EL3_CPUECTLR_BIT		(1 << 1)
#define ACTLR_EL3_CPUACTLR_BIT		(1 << 0)
#define ACTLR_EL3_ENABLE_ALL_ACCESS	(ACTLR_EL3_L2ACTLR_BIT | \
					 ACTLR_EL3_L2ECTLR_BIT | \
					 ACTLR_EL3_L2CTLR_BIT | \
					 ACTLR_EL3_CPUECTLR_BIT | \
					 ACTLR_EL3_CPUACTLR_BIT)

/*******************************************************************************
 * GIC memory map
 ******************************************************************************/
#define TEGRA_GICD_BASE			0x50041000
#define TEGRA_GICC_BASE			0x50042000

/*******************************************************************************
 * Tegra Memory Select Switch Controller constants
 ******************************************************************************/
#define TEGRA_MSELECT_BASE		0x50060000

#define MSELECT_CONFIG			0x0
#define ENABLE_WRAP_INCR_MASTER2_BIT	(1 << 29)
#define ENABLE_WRAP_INCR_MASTER1_BIT	(1 << 28)
#define ENABLE_WRAP_INCR_MASTER0_BIT	(1 << 27)
#define UNSUPPORTED_TX_ERR_MASTER2_BIT	(1 << 25)
#define UNSUPPORTED_TX_ERR_MASTER1_BIT	(1 << 24)
#define ENABLE_UNSUP_TX_ERRORS		(UNSUPPORTED_TX_ERR_MASTER2_BIT | \
					 UNSUPPORTED_TX_ERR_MASTER1_BIT)
#define ENABLE_WRAP_TO_INCR_BURSTS	(ENABLE_WRAP_INCR_MASTER2_BIT | \
					 ENABLE_WRAP_INCR_MASTER1_BIT | \
					 ENABLE_WRAP_INCR_MASTER0_BIT)

/*******************************************************************************
 * Tegra micro-seconds timer constants
 ******************************************************************************/
#define TEGRA_TMRUS_BASE		0x60005010

/*******************************************************************************
 * Tegra Clock and Reset Controller constants
 ******************************************************************************/
#define TEGRA_CAR_RESET_BASE		0x60006000

/*******************************************************************************
 * Tegra Flow Controller constants
 ******************************************************************************/
#define TEGRA_FLOWCTRL_BASE		0x60007000

/*******************************************************************************
 * Tegra Secure Boot Controller constants
 ******************************************************************************/
#define TEGRA_SB_BASE			0x6000C200

/*******************************************************************************
 * Tegra Exception Vectors constants
 ******************************************************************************/
#define TEGRA_EVP_BASE			0x6000F000

/*******************************************************************************
 * Tegra Power Mgmt Controller constants
 ******************************************************************************/
#define TEGRA_PMC_BASE			0x7000E400

/*******************************************************************************
 * Tegra Memory Controller constants
 ******************************************************************************/
#define TEGRA_MC_BASE			0x70019000

#endif /* __TEGRA_DEF_H__ */
