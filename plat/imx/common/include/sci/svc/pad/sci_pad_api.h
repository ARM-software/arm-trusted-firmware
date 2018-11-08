/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file containing the public API for the System Controller (SC)
 * Pad Control (PAD) function.
 *
 * @addtogroup PAD_SVC (SVC) Pad Service
 *
 * Module for the Pad Control (PAD) service.
 *
 * @details
 *
 * Pad configuration is managed by SC firmware. The pad configuration
 * features supported by the SC firmware include:
 *
 * - Configuring the mux, input/output connection, and low-power isolation
     mode.
 * - Configuring the technology-specific pad setting such as drive strength,
 *   pullup/pulldown, etc.
 * - Configuring compensation for pad groups with dual voltage capability.
 *
 * Pad functions fall into one of three categories. Generic functions are
 * common to all SoCs and all process technologies. SoC functions are raw
 * low-level functions. Technology-specific functions are specific to the
 * process technology.
 *
 * The list of pads is SoC specific.  Refer to the SoC [Pad List](@ref PADS)
 * for valid pad values. Note that all pads exist on a die but may or
 * may not be brought out by the specific package.  Mapping of pads to
 * package pins/balls is documented in the associated Data Sheet. Some pads
 * may not be brought out because the part (die+package) is defeatured and
 * some pads may connect to the substrate in the package.
 *
 * Some pads (SC_P_COMP_*) that can be specified are not individual pads
 * but are in fact pad groups. These groups have additional configuration
 * that can be done using the sc_pad_set_gp_28fdsoi_comp() function. More
 * info on these can be found in the associated Reference Manual.
 *
 * Pads are managed as a resource by the Resource Manager (RM).  They have
 * assigned owners and only the owners can configure the pads. Some of the
 * pads are reserved for use by the SCFW itself and this can be overriden
 * with the implementation of board_config_sc(). Additionally, pads may
 * be assigned to various other partitions via the implementation of
 * board_system_config().
 *
 * Note muxing two input pads to the same IP functional signal will
 * result in undefined behavior.
 * @{
 */

#ifndef SCI_PAD_API_H
#define SCI_PAD_API_H

/* Includes */

#include <sci/sci_types.h>
#include <sci/svc/rm/sci_rm_api.h>

/* Defines */

/*!
 * @name Defines for type widths
 */
/*@{*/
#define SC_PAD_MUX_W            3	/* Width of mux parameter */
/*@}*/

/*!
 * @name Defines for sc_pad_config_t
 */
/*@{*/
#define SC_PAD_CONFIG_NORMAL    0U	/* Normal */
#define SC_PAD_CONFIG_OD        1U	/* Open Drain */
#define SC_PAD_CONFIG_OD_IN     2U	/* Open Drain and input */
#define SC_PAD_CONFIG_OUT_IN    3U	/* Output and input */
/*@}*/

/*!
 * @name Defines for sc_pad_iso_t
 */
/*@{*/
#define SC_PAD_ISO_OFF          0U	/* ISO latch is transparent */
#define SC_PAD_ISO_EARLY        1U	/* Follow EARLY_ISO */
#define SC_PAD_ISO_LATE         2U	/* Follow LATE_ISO */
#define SC_PAD_ISO_ON           3U	/* ISO latched data is held */
/*@}*/

/*!
 * @name Defines for sc_pad_28fdsoi_dse_t
 */
/*@{*/
#define SC_PAD_28FDSOI_DSE_18V_1MA   0U	/* Drive strength of 1mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_2MA   1U	/* Drive strength of 2mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_4MA   2U	/* Drive strength of 4mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_6MA   3U	/* Drive strength of 6mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_8MA   4U	/* Drive strength of 8mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_10MA  5U	/* Drive strength of 10mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_12MA  6U	/* Drive strength of 12mA for 1.8v */
#define SC_PAD_28FDSOI_DSE_18V_HS    7U	/* High-speed drive strength for 1.8v */
#define SC_PAD_28FDSOI_DSE_33V_2MA   0U	/* Drive strength of 2mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_33V_4MA   1U	/* Drive strength of 4mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_33V_8MA   2U	/* Drive strength of 8mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_33V_12MA  3U	/* Drive strength of 12mA for 3.3v */
#define SC_PAD_28FDSOI_DSE_DV_HIGH   0U	/* High drive strength for dual volt */
#define SC_PAD_28FDSOI_DSE_DV_LOW    1U	/* Low drive strength for dual volt */
/*@}*/

/*!
 * @name Defines for sc_pad_28fdsoi_ps_t
 */
/*@{*/
#define SC_PAD_28FDSOI_PS_KEEPER 0U	/* Bus-keeper (only valid for 1.8v) */
#define SC_PAD_28FDSOI_PS_PU     1U	/* Pull-up */
#define SC_PAD_28FDSOI_PS_PD     2U	/* Pull-down */
#define SC_PAD_28FDSOI_PS_NONE   3U	/* No pull (disabled) */
/*@}*/

/*!
 * @name Defines for sc_pad_28fdsoi_pus_t
 */
/*@{*/
#define SC_PAD_28FDSOI_PUS_30K_PD  0U	/* 30K pull-down */
#define SC_PAD_28FDSOI_PUS_100K_PU 1U	/* 100K pull-up */
#define SC_PAD_28FDSOI_PUS_3K_PU   2U	/* 3K pull-up */
#define SC_PAD_28FDSOI_PUS_30K_PU  3U	/* 30K pull-up */
/*@}*/

/*!
 * @name Defines for sc_pad_wakeup_t
 */
/*@{*/
#define SC_PAD_WAKEUP_OFF       0U	/* Off */
#define SC_PAD_WAKEUP_CLEAR     1U	/* Clears pending flag */
#define SC_PAD_WAKEUP_LOW_LVL   4U	/* Low level */
#define SC_PAD_WAKEUP_FALL_EDGE 5U	/* Falling edge */
#define SC_PAD_WAKEUP_RISE_EDGE 6U	/* Rising edge */
#define SC_PAD_WAKEUP_HIGH_LVL  7U	/* High-level */
/*@}*/

/* Types */

/*!
 * This type is used to declare a pad config. It determines how the
 * output data is driven, pull-up is controlled, and input signal is
 * connected. Normal and OD are typical and only connect the input
 * when the output is not driven.  The IN options are less common and
 * force an input connection even when driving the output.
 */
typedef uint8_t sc_pad_config_t;

/*!
 * This type is used to declare a pad low-power isolation config.
 * ISO_LATE is the most common setting. ISO_EARLY is only used when
 * an output pad is directly determined by another input pad. The
 * other two are only used when SW wants to directly contol isolation.
 */
typedef uint8_t sc_pad_iso_t;

/*!
 * This type is used to declare a drive strength. Note it is specific
 * to 28FDSOI. Also note that valid values depend on the pad type.
 */
typedef uint8_t sc_pad_28fdsoi_dse_t;

/*!
 * This type is used to declare a pull select. Note it is specific
 * to 28FDSOI.
 */
typedef uint8_t sc_pad_28fdsoi_ps_t;

/*!
 * This type is used to declare a pull-up select. Note it is specific
 * to 28FDSOI HSIC pads.
 */
typedef uint8_t sc_pad_28fdsoi_pus_t;

/*!
 * This type is used to declare a wakeup mode of a pad.
 */
typedef uint8_t sc_pad_wakeup_t;

/* Functions */

/*!
 * @name Generic Functions
 * @{
 */

/*!
 * This function configures the mux settings for a pad. This includes
 * the signal mux, pad config, and low-power isolation mode.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     mux         mux setting
 * @param[in]     config      pad config
 * @param[in]     iso         low-power isolation mode
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Note muxing two input pads to the same IP functional signal will
 * result in undefined behavior.
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_set_mux(sc_ipc_t ipc, sc_pad_t pad,
			uint8_t mux, sc_pad_config_t config, sc_pad_iso_t iso);

/*!
 * This function gets the mux settings for a pad. This includes
 * the signal mux, pad config, and low-power isolation mode.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    mux         pointer to return mux setting
 * @param[out]    config      pointer to return pad config
 * @param[out]    iso         pointer to return low-power isolation mode
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get_mux(sc_ipc_t ipc, sc_pad_t pad,
			uint8_t *mux, sc_pad_config_t *config,
			sc_pad_iso_t *iso);

/*!
 * This function configures the general purpose pad control. This
 * is technology dependent and includes things like drive strength,
 * slew rate, pull up/down, etc. Refer to the SoC Reference Manual
 * for bit field details.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     ctrl        control value to set
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_set_gp(sc_ipc_t ipc, sc_pad_t pad, uint32_t ctrl);

/*!
 * This function gets the general purpose pad control. This
 * is technology dependent and includes things like drive strength,
 * slew rate, pull up/down, etc. Refer to the SoC Reference Manual
 * for bit field details.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    ctrl        pointer to return control value
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get_gp(sc_ipc_t ipc, sc_pad_t pad, uint32_t *ctrl);

/*!
 * This function configures the wakeup mode of the pad.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     wakeup      wakeup to set
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_set_wakeup(sc_ipc_t ipc, sc_pad_t pad, sc_pad_wakeup_t wakeup);

/*!
 * This function gets the wakeup mode of a pad.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    wakeup      pointer to return wakeup
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get_wakeup(sc_ipc_t ipc, sc_pad_t pad, sc_pad_wakeup_t *wakeup);

/*!
 * This function configures a pad.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     mux         mux setting
 * @param[in]     config      pad config
 * @param[in]     iso         low-power isolation mode
 * @param[in]     ctrl        control value
 * @param[in]     wakeup      wakeup to set
 *
 * @see sc_pad_set_mux().
 * @see sc_pad_set_gp().
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Note muxing two input pads to the same IP functional signal will
 * result in undefined behavior.
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_set_all(sc_ipc_t ipc, sc_pad_t pad, uint8_t mux,
			sc_pad_config_t config, sc_pad_iso_t iso, uint32_t ctrl,
			sc_pad_wakeup_t wakeup);

/*!
 * This function gets a pad's config.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    mux         pointer to return mux setting
 * @param[out]    config      pointer to return pad config
 * @param[out]    iso         pointer to return low-power isolation mode
 * @param[out]    ctrl        pointer to return control value
 * @param[out]    wakeup      pointer to return wakeup to set
 *
 * @see sc_pad_set_mux().
 * @see sc_pad_set_gp().
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get_all(sc_ipc_t ipc, sc_pad_t pad, uint8_t *mux,
			sc_pad_config_t *config, sc_pad_iso_t *iso,
			uint32_t *ctrl, sc_pad_wakeup_t *wakeup);

/* @} */

/*!
 * @name SoC Specific Functions
 * @{
 */

/*!
 * This function configures the settings for a pad. This setting is SoC
 * specific.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     val         value to set
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_set(sc_ipc_t ipc, sc_pad_t pad, uint32_t val);

/*!
 * This function gets the settings for a pad. This setting is SoC
 * specific.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    val         pointer to return setting
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get(sc_ipc_t ipc, sc_pad_t pad, uint32_t *val);

/* @} */

/*!
 * @name Technology Specific Functions
 * @{
 */

/*!
 * This function configures the pad control specific to 28FDSOI.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     dse         drive strength
 * @param[in]     ps          pull select
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner,
 * - SC_ERR_UNAVAILABLE if process not applicable
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_set_gp_28fdsoi(sc_ipc_t ipc, sc_pad_t pad,
			       sc_pad_28fdsoi_dse_t dse,
			       sc_pad_28fdsoi_ps_t ps);

/*!
 * This function gets the pad control specific to 28FDSOI.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    dse         pointer to return drive strength
 * @param[out]    ps          pointer to return pull select
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner,
 * - SC_ERR_UNAVAILABLE if process not applicable
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get_gp_28fdsoi(sc_ipc_t ipc, sc_pad_t pad,
			       sc_pad_28fdsoi_dse_t *dse,
			       sc_pad_28fdsoi_ps_t *ps);

/*!
 * This function configures the pad control specific to 28FDSOI.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     dse         drive strength
 * @param[in]     hys         hysteresis
 * @param[in]     pus         pull-up select
 * @param[in]     pke         pull keeper enable
 * @param[in]     pue         pull-up enable
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner,
 * - SC_ERR_UNAVAILABLE if process not applicable
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_set_gp_28fdsoi_hsic(sc_ipc_t ipc, sc_pad_t pad,
				    sc_pad_28fdsoi_dse_t dse, sc_bool_t hys,
				    sc_pad_28fdsoi_pus_t pus, sc_bool_t pke,
				    sc_bool_t pue);

/*!
 * This function gets the pad control specific to 28FDSOI.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    dse         pointer to return drive strength
 * @param[out]    hys         pointer to return hysteresis
 * @param[out]    pus         pointer to return pull-up select
 * @param[out]    pke         pointer to return pull keeper enable
 * @param[out]    pue         pointer to return pull-up enable
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner,
 * - SC_ERR_UNAVAILABLE if process not applicable
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get_gp_28fdsoi_hsic(sc_ipc_t ipc, sc_pad_t pad,
				    sc_pad_28fdsoi_dse_t *dse, sc_bool_t *hys,
				    sc_pad_28fdsoi_pus_t *pus, sc_bool_t *pke,
				    sc_bool_t *pue);

/*!
 * This function configures the compensation control specific to 28FDSOI.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to configure
 * @param[in]     compen      compensation/freeze mode
 * @param[in]     fastfrz     fast freeze
 * @param[in]     rasrcp      compensation code for PMOS
 * @param[in]     rasrcn      compensation code for NMOS
 * @param[in]     nasrc_sel   NASRC read select
 * @param[in]     psw_ovr     2.5v override
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner,
 * - SC_ERR_UNAVAILABLE if process not applicable
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 *
 * Note \a psw_ovr is only applicable to pads supporting 2.5 volt
 * operation (e.g. some Ethernet pads).
 */
sc_err_t sc_pad_set_gp_28fdsoi_comp(sc_ipc_t ipc, sc_pad_t pad,
				    uint8_t compen, sc_bool_t fastfrz,
				    uint8_t rasrcp, uint8_t rasrcn,
				    sc_bool_t nasrc_sel, sc_bool_t psw_ovr);

/*!
 * This function gets the compensation control specific to 28FDSOI.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pad         pad to query
 * @param[out]    compen      pointer to return compensation/freeze mode
 * @param[out]    fastfrz     pointer to return fast freeze
 * @param[out]    rasrcp      pointer to return compensation code for PMOS
 * @param[out]    rasrcn      pointer to return compensation code for NMOS
 * @param[out]    nasrc_sel   pointer to return NASRC read select
 * @param[out]    compok      pointer to return compensation status
 * @param[out]    nasrc       pointer to return NASRCP/NASRCN
 * @param[out]    psw_ovr     pointer to return the 2.5v override
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pad owner,
 * - SC_ERR_UNAVAILABLE if process not applicable
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
sc_err_t sc_pad_get_gp_28fdsoi_comp(sc_ipc_t ipc, sc_pad_t pad,
				    uint8_t *compen, sc_bool_t *fastfrz,
				    uint8_t *rasrcp, uint8_t *rasrcn,
				    sc_bool_t *nasrc_sel, sc_bool_t *compok,
				    uint8_t *nasrc, sc_bool_t *psw_ovr);

/* @} */

#endif /* SCI_PAD_API_H */

/**@}*/
