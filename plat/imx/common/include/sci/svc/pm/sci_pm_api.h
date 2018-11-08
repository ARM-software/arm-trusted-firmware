/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file containing the public API for the System Controller (SC)
 * Power Management (PM) function. This includes functions for power state
 * control, clock control, reset control, and wake-up event control.
 *
 * @addtogroup PM_SVC (SVC) Power Management Service
 *
 * Module for the Power Management (PM) service.
 *
 * @{
 */

#ifndef SCI_PM_API_H
#define SCI_PM_API_H

/* Includes */

#include <sci/sci_types.h>
#include <sci/svc/rm/sci_rm_api.h>

/* Defines */

/*!
 * @name Defines for type widths
 */
/*@{*/
#define SC_PM_POWER_MODE_W      2	/* Width of sc_pm_power_mode_t */
#define SC_PM_CLOCK_MODE_W      3	/* Width of sc_pm_clock_mode_t */
#define SC_PM_RESET_TYPE_W      2	/* Width of sc_pm_reset_type_t */
#define SC_PM_RESET_REASON_W    3	/* Width of sc_pm_reset_reason_t */
/*@}*/

/*!
 * @name Defines for clock indexes (sc_pm_clk_t)
 */
/*@{*/
/*@}*/

/*!
 * @name Defines for ALL parameters
 */
/*@{*/
#define SC_PM_CLK_ALL   UINT8_MAX	/* All clocks */
/*@}*/

/*!
 * @name Defines for sc_pm_power_mode_t
 */
/*@{*/
#define SC_PM_PW_MODE_OFF       0U	/* Power off */
#define SC_PM_PW_MODE_STBY      1U	/* Power in standby */
#define SC_PM_PW_MODE_LP        2U	/* Power in low-power */
#define SC_PM_PW_MODE_ON        3U	/* Power on */
/*@}*/

/*!
 * @name Defines for sc_pm_clk_t
 */
/*@{*/
#define SC_PM_CLK_SLV_BUS       0U	/* Slave bus clock */
#define SC_PM_CLK_MST_BUS       1U	/* Master bus clock */
#define SC_PM_CLK_PER           2U	/* Peripheral clock */
#define SC_PM_CLK_PHY           3U	/* Phy clock */
#define SC_PM_CLK_MISC          4U	/* Misc clock */
#define SC_PM_CLK_MISC0         0U	/* Misc 0 clock */
#define SC_PM_CLK_MISC1         1U	/* Misc 1 clock */
#define SC_PM_CLK_MISC2         2U	/* Misc 2 clock */
#define SC_PM_CLK_MISC3         3U	/* Misc 3 clock */
#define SC_PM_CLK_MISC4         4U	/* Misc 4 clock */
#define SC_PM_CLK_CPU           2U	/* CPU clock */
#define SC_PM_CLK_PLL           4U	/* PLL */
#define SC_PM_CLK_BYPASS        4U	/* Bypass clock */
/*@}*/

/*!
 * @name Defines for sc_pm_clk_mode_t
 */
/*@{*/
#define SC_PM_CLK_MODE_ROM_INIT        0U	/* Clock is initialized by ROM. */
#define SC_PM_CLK_MODE_OFF             1U	/* Clock is disabled */
#define SC_PM_CLK_MODE_ON              2U	/* Clock is enabled. */
#define SC_PM_CLK_MODE_AUTOGATE_SW     3U	/* Clock is in SW autogate mode */
#define SC_PM_CLK_MODE_AUTOGATE_HW     4U	/* Clock is in HW autogate mode */
#define SC_PM_CLK_MODE_AUTOGATE_SW_HW  5U	/* Clock is in SW-HW autogate mode */
/*@}*/

/*!
 * @name Defines for sc_pm_clk_parent_t
 */
/*@{*/
#define SC_PM_PARENT_XTAL              0U	/* Parent is XTAL. */
#define SC_PM_PARENT_PLL0              1U	/* Parent is PLL0 */
#define SC_PM_PARENT_PLL1              2U	/* Parent is PLL1 or PLL0/2 */
#define SC_PM_PARENT_PLL2              3U	/* Parent in PLL2 or PLL0/4 */
#define SC_PM_PARENT_BYPS              4U	/* Parent is a bypass clock. */
/*@}*/

/*!
 * @name Defines for sc_pm_reset_type_t
 */
/*@{*/
#define SC_PM_RESET_TYPE_COLD          0U	/* Cold reset */
#define SC_PM_RESET_TYPE_WARM          1U	/* Warm reset */
#define SC_PM_RESET_TYPE_BOARD         2U	/* Board reset */
/*@}*/

/*!
 * @name Defines for sc_pm_reset_cause_t
 */
/*@{*/
#define SC_PM_RESET_CAUSE_TEMP         0U	/* Reset due to temp panic alarm */
#define SC_PM_RESET_CAUSE_FAULT        1U	/* Reset due to fault exception */
#define SC_PM_RESET_CAUSE_IRQ          2U	/* Reset due to SCU reset IRQ */
#define SC_PM_RESET_CAUSE_WDOG         3U	/* Reset due to SW WDOG */
#define SC_PM_RESET_CAUSE_API          4U	/* Reset due to pm_reset() or monitor */
/*@}*/

/*!
 * @name Defines for sc_pm_reset_reason_t
 */
/*@{*/
#define SC_PM_RESET_REASON_POR         0U	/* Power on reset */
#define SC_PM_RESET_REASON_WARM        1U	/* Warm reset */
#define SC_PM_RESET_REASON_SW          2U	/* Software reset */
#define SC_PM_RESET_REASON_WDOG        3U	/* Watchdog reset */
#define SC_PM_RESET_REASON_LOCKUP      4U	/* Lockup reset */
#define SC_PM_RESET_REASON_TAMPER      5U	/* Tamper reset */
#define SC_PM_RESET_REASON_TEMP        6U	/* Temp reset */
#define SC_PM_RESET_REASON_LOW_VOLT    7U	/* Low voltage reset */
/*@}*/

/*!
 * @name Defines for sc_pm_sys_if_t
 */
/*@{*/
#define SC_PM_SYS_IF_INTERCONNECT       0U	/* System interconnect */
#define SC_PM_SYS_IF_MU                 1U	/* AP -> SCU message units */
#define SC_PM_SYS_IF_OCMEM              2U	/* On-chip memory (ROM/OCRAM) */
#define SC_PM_SYS_IF_DDR                3U	/* DDR memory */
/*@}*/

/*!
 * @name Defines for sc_pm_wake_src_t
 */
/*@{*/
#define SC_PM_WAKE_SRC_NONE             0U	/* No wake source, used for self-kill */
#define SC_PM_WAKE_SRC_SCU              1U	/* Wakeup from SCU to resume CPU (IRQSTEER & GIC powered down) */
#define SC_PM_WAKE_SRC_IRQSTEER         2U	/* Wakeup from IRQSTEER to resume CPU (GIC powered down) */
#define SC_PM_WAKE_SRC_IRQSTEER_GIC     3U	/* Wakeup from IRQSTEER+GIC to wake CPU  (GIC clock gated) */
#define SC_PM_WAKE_SRC_GIC              4U	/* Wakeup from GIC to wake CPU */
/*@}*/

/* Types */

/*!
 * This type is used to declare a power mode. Note resources only use
 * SC_PM_PW_MODE_OFF and SC_PM_PW_MODE_ON. The other modes are used only
 * as system power modes.
 */
typedef uint8_t sc_pm_power_mode_t;

/*!
 * This type is used to declare a clock.
 */
typedef uint8_t sc_pm_clk_t;

/*!
 * This type is used to declare a clock mode.
 */
typedef uint8_t sc_pm_clk_mode_t;

/*!
 * This type is used to declare the clock parent.
 */
typedef uint8_t sc_pm_clk_parent_t;

/*!
 * This type is used to declare clock rates.
 */
typedef uint32_t sc_pm_clock_rate_t;

/*!
 * This type is used to declare a desired reset type.
 */
typedef uint8_t sc_pm_reset_type_t;

/*!
 * This type is used to declare a desired reset type.
 */
typedef uint8_t sc_pm_reset_cause;

/*!
 * This type is used to declare a reason for a reset.
 */
typedef uint8_t sc_pm_reset_reason_t;

/*!
 * This type is used to specify a system-level interface to be power managed.
 */
typedef uint8_t sc_pm_sys_if_t;

/*!
 * This type is used to specify a wake source for CPU resources.
 */
typedef uint8_t sc_pm_wake_src_t;

/* Functions */

/*!
 * @name Power Functions
 * @{
 */

/*!
 * This function sets the system power mode. Only the owner of the
 * SC_R_SYSTEM resource can do this.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     mode        power mode to apply
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid mode,
 * - SC_ERR_NOACCESS if caller not the owner of SC_R_SYSTEM
 *
 * @see sc_pm_set_sys_power_mode().
 */
sc_err_t sc_pm_set_sys_power_mode(sc_ipc_t ipc, sc_pm_power_mode_t mode);

/*!
 * This function sets the power mode of a partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition
 * @param[in]     mode        power mode to apply
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid partition or mode,
 * - SC_ERR_NOACCESS if caller's partition is not the owner or
 *   parent of \a pt
 *
 * The power mode of the partitions is a max power any resource will
 * be set to. Calling this will result in all resources owned
 * by \a pt to have their power changed to the lower of \a mode or the
 * individual resource mode set using sc_pm_set_resource_power_mode().
 */
sc_err_t sc_pm_set_partition_power_mode(sc_ipc_t ipc, sc_rm_pt_t pt,
					sc_pm_power_mode_t mode);

/*!
 * This function gets the power mode of a partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition
 * @param[out]    mode        pointer to return power mode
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid partition
 */
sc_err_t sc_pm_get_sys_power_mode(sc_ipc_t ipc, sc_rm_pt_t pt,
				  sc_pm_power_mode_t *mode);

/*!
 * This function sets the power mode of a resource.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     mode        power mode to apply
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or mode,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner
 *   or parent of the owner
 *
 * This function will record the individual resource power mode
 * and change it if the requested mode is lower than or equal to the
 * partition power mode set with sc_pm_set_partition_power_mode().
 * In other words, the power mode of the resource will be the minimum
 * of the resource power mode and the partition power mode.
 *
 * Note some resources are still not accessible even when powered up if bus
 * transactions go through a fabric not powered up. Examples of this are
 * resources in display and capture subsystems which require the display
 * controller or the imaging subsytem to be powered up first.
 *
 * Not that resources are grouped into power domains by the underlying
 * hardware. If any resource in the domain is on, the entire power domain
 * will be on. Other power domains required to access the resource will
 * also be turned on. Clocks required to access the peripheral will be
 * turned on. Refer to the SoC RM for more info on power domains and access
 * infrastructure (bus fabrics, clock domains, etc.).
 */
sc_err_t sc_pm_set_resource_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				       sc_pm_power_mode_t mode);

/*!
 * This function gets the power mode of a resource.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[out]    mode        pointer to return power mode
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Note only SC_PM_PW_MODE_OFF and SC_PM_PW_MODE_ON are valid. The value
 * returned does not reflect the power mode of the partition..
 */
sc_err_t sc_pm_get_resource_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				       sc_pm_power_mode_t *mode);

/*!
 * This function requests the low power mode some of the resources
 * can enter based on their state. This API is only valid for the
 * following resources : SC_R_A53, SC_R_A53_0, SC_R_A53_1, SC_A53_2,
 * SC_A53_3, SC_R_A72, SC_R_A72_0, SC_R_A72_1, SC_R_CC1, SC_R_A35,
 * SC_R_A35_0, SC_R_A35_1, SC_R_A35_2, SC_R_A35_3.
 * For all other resources it will return SC_ERR_PARAM.
 * This function will set the low power mode the cores, cluster
 * and cluster associated resources will enter when all the cores
 * in a given cluster execute WFI
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     mode        power mode to apply
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 */
sc_err_t sc_pm_req_low_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				  sc_pm_power_mode_t mode);

/*!
 * This function requests low-power mode entry for CPU/cluster
 * resources. This API is only valid for the following resources:
 * SC_R_A53, SC_R_A53_x, SC_R_A72, SC_R_A72_x, SC_R_A35, SC_R_A35_x,
 * SC_R_CCI. For all other resources it will return SC_ERR_PARAM.
 * For individual core resources, the specified power mode
 * and wake source will be applied after the core has entered
 * WFI.  For cluster resources, the specified power mode is
 * applied after all cores in the cluster have entered low-power mode.
 * For multicluster resources, the specified power mode is applied
 * after all clusters have reached low-power mode.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     mode        power mode to apply
 * @param[in]     wake_src    wake source for low-power exit
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 */
sc_err_t sc_pm_req_cpu_low_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				      sc_pm_power_mode_t mode,
				      sc_pm_wake_src_t wake_src);

/*!
 * This function is used to set the resume address of a CPU.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the CPU resource
 * @param[in]     address     64-bit resume address
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or address,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of the
 *   resource (CPU) owner
 */
sc_err_t sc_pm_set_cpu_resume_addr(sc_ipc_t ipc, sc_rsrc_t resource,
				   sc_faddr_t address);

/*!
 * This function is used to set parameters for CPU resume from
 * low-power mode.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the CPU resource
 * @param[in]     isPrimary   set SC_TRUE if primary wake CPU
 * @param[in]     address     64-bit resume address
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or address,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of the
 *   resource (CPU) owner
 */
sc_err_t sc_pm_set_cpu_resume(sc_ipc_t ipc, sc_rsrc_t resource,
			      sc_bool_t isPrimary, sc_faddr_t address);

/*!
 * This function requests the power mode configuration for system-level
 * interfaces including messaging units, interconnect, and memories.  This API
 * is only valid for the following resources : SC_R_A53, SC_R_A72, and
 * SC_R_M4_x_PID_y.  For all other resources, it will return SC_ERR_PARAM.
 * The requested power mode will be captured and applied to system-level
 * resources as system conditions allow.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     sys_if      system-level interface to be configured
 * @param[in]     hpm         high-power mode for the system interface
 * @param[in]     lpm         low-power mode for the system interface
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 */
sc_err_t sc_pm_req_sys_if_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				     sc_pm_sys_if_t sys_if,
				     sc_pm_power_mode_t hpm,
				     sc_pm_power_mode_t lpm);

/* @} */

/*!
 * @name Clock/PLL Functions
 * @{
 */

/*!
 * This function sets the rate of a resource's clock/PLL.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     clk         clock/PLL to affect
 * @param[in,out] rate        pointer to rate to set,
 *                            return actual rate
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or clock/PLL,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner
 *   or parent of the owner,
 * - SC_ERR_UNAVAILABLE if clock/PLL not applicable to this resource,
 * - SC_ERR_LOCKED if rate locked (usually because shared clock/PLL)
 *
 * Refer to the [Clock List](@ref CLOCKS) for valid clock/PLL values.
 */
sc_err_t sc_pm_set_clock_rate(sc_ipc_t ipc, sc_rsrc_t resource,
			      sc_pm_clk_t clk, sc_pm_clock_rate_t *rate);

/*!
 * This function gets the rate of a resource's clock/PLL.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     clk         clock/PLL to affect
 * @param[out]    rate        pointer to return rate
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or clock/PLL,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner
 *   or parent of the owner,
 * - SC_ERR_UNAVAILABLE if clock/PLL not applicable to this resource
 *
 * Refer to the [Clock List](@ref CLOCKS) for valid clock/PLL values.
 */
sc_err_t sc_pm_get_clock_rate(sc_ipc_t ipc, sc_rsrc_t resource,
			      sc_pm_clk_t clk, sc_pm_clock_rate_t *rate);

/*!
 * This function enables/disables a resource's clock.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     clk         clock to affect
 * @param[in]     enable      enable if SC_TRUE; otherwise disabled
 * @param[in]     autog       HW auto clock gating
 *
 * If \a resource is SC_R_ALL then all resources owned will be affected.
 * No error will be returned.
 *
 * If \a clk is SC_PM_CLK_ALL, then an error will be returned if any
 * of the available clocks returns an error.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or clock,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner
 *   or parent of the owner,
 * - SC_ERR_UNAVAILABLE if clock not applicable to this resource
 *
 * Refer to the [Clock List](@ref CLOCKS) for valid clock values.
 */
sc_err_t sc_pm_clock_enable(sc_ipc_t ipc, sc_rsrc_t resource,
			    sc_pm_clk_t clk, sc_bool_t enable, sc_bool_t autog);

/*!
 * This function sets the parent of a resource's clock.
 * This function should only be called when the clock is disabled.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     clk         clock to affect
 * @param[in]     parent      New parent of the clock.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or clock,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner
 *   or parent of the owner,
 * - SC_ERR_UNAVAILABLE if clock not applicable to this resource
 * - SC_ERR_BUSY if clock is currently enabled.
 * - SC_ERR_NOPOWER if resource not powered
 *
 * Refer to the [Clock List](@ref CLOCKS) for valid clock values.
 */
sc_err_t sc_pm_set_clock_parent(sc_ipc_t ipc, sc_rsrc_t resource,
				sc_pm_clk_t clk, sc_pm_clk_parent_t parent);

/*!
 * This function gets the parent of a resource's clock.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the resource
 * @param[in]     clk         clock to affect
 * @param[out]     parent     pointer to return parent of clock.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or clock,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner
 *   or parent of the owner,
 * - SC_ERR_UNAVAILABLE if clock not applicable to this resource
 *
 * Refer to the [Clock List](@ref CLOCKS) for valid clock values.
 */
sc_err_t sc_pm_get_clock_parent(sc_ipc_t ipc, sc_rsrc_t resource,
				sc_pm_clk_t clk, sc_pm_clk_parent_t *parent);

/* @} */

/*!
 * @name Reset Functions
 * @{
 */

/*!
 * This function is used to reset the system. Only the owner of the
 * SC_R_SYSTEM resource can do this.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     type        reset type
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid type,
 * - SC_ERR_NOACCESS if caller not the owner of SC_R_SYSTEM
 *
 * If this function returns, then the reset did not occur due to an
 * invalid parameter.
 */
sc_err_t sc_pm_reset(sc_ipc_t ipc, sc_pm_reset_type_t type);

/*!
 * This function gets a caller's reset reason.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    reason      pointer to return reset reason
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_pm_reset_reason(sc_ipc_t ipc, sc_pm_reset_reason_t *reason);

/*!
 * This function is used to boot a partition.
 *
 * @param[in]     ipc          IPC handle
 * @param[in]     pt           handle of partition to boot
 * @param[in]     resource_cpu ID of the CPU resource to start
 * @param[in]     boot_addr    64-bit boot address
 * @param[in]     resource_mu  ID of the MU that must be powered
 * @param[in]     resource_dev ID of the boot device that must be powered
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid partition, resource, or addr,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of the
 *   partition to boot
 */
sc_err_t sc_pm_boot(sc_ipc_t ipc, sc_rm_pt_t pt,
		    sc_rsrc_t resource_cpu, sc_faddr_t boot_addr,
		    sc_rsrc_t resource_mu, sc_rsrc_t resource_dev);

/*!
 * This function is used to reboot the caller's partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     type        reset type
 *
 * If \a type is SC_PM_RESET_TYPE_COLD, then most peripherals owned by
 * the calling partition will be reset if possible. SC state (partitions,
 * power, clocks, etc.) is reset. The boot SW of the booting CPU must be
 * able to handle peripherals that that are not reset.
 *
 * If \a type is SC_PM_RESET_TYPE_WARM, then only the boot CPU is reset.
 * SC state (partitions, power, clocks, etc.) are NOT reset. The boot SW
 * of the booting CPU must be able to handle peripherals and SC state that
 * that are not reset.
 *
 * If \a type is SC_PM_RESET_TYPE_BOARD, then return with no action.
 *
 * If this function returns, then the reset did not occur due to an
 * invalid parameter.
 */
void sc_pm_reboot(sc_ipc_t ipc, sc_pm_reset_type_t type);

/*!
 * This function is used to reboot a partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to reboot
 * @param[in]     type        reset type
 *
 * If \a type is SC_PM_RESET_TYPE_COLD, then most peripherals owned by
 * the calling partition will be reset if possible. SC state (partitions,
 * power, clocks, etc.) is reset. The boot SW of the booting CPU must be
 * able to handle peripherals that that are not reset.
 *
 * If \a type is SC_PM_RESET_TYPE_WARM, then only the boot CPU is reset.
 * SC state (partitions, power, clocks, etc.) are NOT reset. The boot SW
 * of the booting CPU must be able to handle peripherals and SC state that
 * that are not reset.
 *
 * If \a type is SC_PM_RESET_TYPE_BOARD, then return with no action.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid partition or type
 * - SC_ERR_NOACCESS if caller's partition is not the parent of \a pt,
 *
 * Most peripherals owned by the partition will be reset if
 * possible. SC state (partitions, power, clocks, etc.) is reset. The
 * boot SW of the booting CPU must be able to handle peripherals that
 * that are not reset.
 */
sc_err_t sc_pm_reboot_partition(sc_ipc_t ipc, sc_rm_pt_t pt,
				sc_pm_reset_type_t type);

/*!
 * This function is used to start/stop a CPU.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    ID of the CPU resource
 * @param[in]     enable      start if SC_TRUE; otherwise stop
 * @param[in]     address     64-bit boot address
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid resource or address,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of the
 *   resource (CPU) owner
 */
sc_err_t sc_pm_cpu_start(sc_ipc_t ipc, sc_rsrc_t resource, sc_bool_t enable,
			 sc_faddr_t address);

/* @} */

#endif /* SCI_PM_API_H */

/**@}*/
