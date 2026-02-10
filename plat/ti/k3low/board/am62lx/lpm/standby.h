/*
 * Copyright (c) 2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
* AM62L Low Power Management - CPU Standby Mode
*
* This module implements CPU standby mode for the AM62L SoC,
* which allows the CPU to enter a power-saving state while maintaining
* fast resume capabilities.
*/

#ifndef AM62L_LPM_STANDBY_H
#define AM62L_LPM_STANDBY_H

#include <plat/common/platform.h>

/* Power domain indices for AM62L MPU cluster */
#define PD_MPU_CLST			4U
#define PD_MPU_CLST_CORE_0		5U
#define PD_MPU_CLST_CORE_1		6U

/* LPSC (Local Power Sleep Controller) module indices */
#define LPSC_MAIN_MPU_CLST		38U
#define LPSC_MAIN_MPU_CLST_PBIST	39U
#define LPSC_MAIN_MPU_CLST_CORE_0	40U
#define LPSC_MAIN_MPU_CLST_CORE_1	41U

/* PSC module states (Power and Sleep Controller) */
#define PSC_SYNCRESETDISABLE		0x0U
#define PSC_SYNCRESET			0x1U
#define PSC_DISABLE			0x2U
#define PSC_ENABLE			0x3U

/* PSC power domain states */
#define PSC_PD_OFF			0x0U
#define PSC_PD_ON			0x1U

/* CPU idle state definitions for PSCI */
#define CORE_IDLE_STATE			0x1U
#define LOW_LATENCY_IDLE_STATE		0x2U
#define HIGH_LATENCY_IDLE_STATE		0x3U

/*
 * Does the state coordination between the cores for entry into the standby states.
 * Depending on the result of the coordination and the cluster power state,
 * it calls the function to enter the required standby mode.
 */
void am62l_enter_standby(uint32_t core, uint32_t cluster_pwr_state);

/*
 * Does the required state coordination for the cores and calls the function to
 * exit the standby mode and restore the system state.
 */
void am62l_exit_standby(uint32_t core, uint32_t cluster_pwr_state);

/*
 * Sets the requested state of required module and power domain.
 * This function:
 * 1. Checks if the requested states are already set
 * 2. Waits for any ongoing power state transitions to complete
 * 3. Programs the PDCTL and MDCTL registers with the new states
 * 4. Initiates the power state transition
 * 5. Waits for the transition to complete if powering on
 * 6. Logs the before and after states for debugging
 *
 * @pd_id: Power domain ID (e.g., PD_MPU_CLST, PD_MPU_CLST_CORE_0)
 * @md_id: Module ID (e.g., LPSC_MAIN_MPU_CLST, LPSC_MAIN_MPU_CLST_CORE_0)
 * @pd_state: Target power domain state (PSC_PD_ON or PSC_PD_OFF)
 * @md_state: Target module state (PSC_ENABLE, PSC_DISABLE, PSC_SYNCRESETDISABLE, etc.)
 */
void set_main_psc_state(uint32_t pd_id, uint32_t md_id,
			uint32_t pd_state, uint32_t md_state);

#endif /* AM62L_LPM_STANDBY_H */
