/* SPDX-License-Identifier: BSD-3-Clause */
/**
 * Copyright 2019-2024 NXP
 *
 * KEYWORDS: micro-power uPower driver API
 * -----------------------------------------------------------------------------
 * PURPOSE: SoC-dependent uPower driver API #defines and typedefs shared
 *          with the firmware
 * -----------------------------------------------------------------------------
 * PARAMETERS:
 * PARAM NAME RANGE:DESCRIPTION:       DEFAULTS:                           UNITS
 * -----------------------------------------------------------------------------
 * REUSE ISSUES: no reuse issues
 */

#ifndef UPWR_SOC_DEFS_H
#define UPWR_SOC_DEFS_H

#include <stdbool.h>
#include <stdint.h>

#include "upower_defs.h"

#define UPWR_MU_MSG_SIZE            (2U) /* words */

#ifdef NUM_PMC_SWT_WORDS
#define UPWR_PMC_SWT_WORDS          NUM_PMC_SWT_WORDS
#endif

#ifdef NUM_PMC_RAM_WORDS
#define UPWR_PMC_MEM_WORDS          NUM_PMC_RAM_WORDS
#endif

#ifndef UPWR_DRAM_SHARED_BASE_ADDR
#define UPWR_DRAM_SHARED_BASE_ADDR      (0x28330000U)
#endif

#ifndef UPWR_DRAM_SHARED_SIZE
#define UPWR_DRAM_SHARED_SIZE           (2048U)
#endif

#define UPWR_DRAM_SHARED_ENDPLUS        (UPWR_DRAM_SHARED_BASE_ADDR+\
					 UPWR_DRAM_SHARED_SIZE)

#ifndef UPWR_API_BUFFER_BASE
#define UPWR_API_BUFFER_BASE            (0x28330600U)
#endif

#ifndef UPWR_API_BUFFER_ENDPLUS
#define UPWR_API_BUFFER_ENDPLUS         (UPWR_DRAM_SHARED_ENDPLUS - 64U)
#endif

#ifndef UPWR_PMC_SWT_WORDS
#define UPWR_PMC_SWT_WORDS              (1U)
#endif

#ifndef UPWR_PMC_MEM_WORDS
#define UPWR_PMC_MEM_WORDS              (2U)
#endif

#define UPWR_OSC_HI_FREQ               (64U) // MHz
#define UPWR_OSC_LO_FREQ               (16U) // MHz

#ifndef UPWR_I2C_FREQ
#define UPWR_I2C_FREQ                  (UPWR_OSC_HI_FREQ * 1000000U)
#endif

/*
 * i.MX8ULP-dependent uPower API Definition
 *
 * This chapter documents the API definitions that are specific to the
 * i.MX8ULP SoC.
 *
 */

/**---------------------------------------------------------------
 * INITIALIZATION, CONFIGURATION
 *
 * i.MX8ULP provides only one Message Unit (MU) for each core domain:
 * Real Time Domain (RTD) and Application Domain (APD), which has two A35 cores.
 * Both A35 cores in APD must share the same API instance, meaning upwr_init
 * must be called only once for each domain. The API does not provide any
 * mutually exclusion or locking mechanism for concurrent accesses from both
 * APD cores, so any API arbitration, if needed, must be implemented by the
 * API user code.
 *
 * A domain must not go to Power Down (PD) or Deep Power Down (DPD) power modes
 * with any service still pending (response not received).
 *
 * Next sections describe the i.MX8ULP particularities of service calls.
 *
 */

/**+
 * upwr_start()
 *
 * i.MX8ULP ROM firmware provides only the launch option 0, which has no
 * power mode transition support and provides the following services:
 * - upwr_xcp_config
 * - upwr_xcp_sw_alarm
 * - upwr_pwm_param
 * - upwr_pwm_power_on
 * - upwr_pwm_power-off
 * - upwr_pwm_mem_retain
 * - upwr_pwm_chng_dom_bias
 * - upwr_pwm_chng_mem_bias
 *
 * i.MX8ULP RAM firmware provides 2 launch options:
 *
 * 1. starts all tasks, services and power mode ones;
 *    this is the full-featured firmware option.
 * 2. starts only the power mode tasks; services are not available with
 *    this option, and futher calls to upwr_start (from either domain)
 *    have no response; this option is mostly used to accelerate power mode
 *    mixed-signal simulations, and not intended to be used with silicon.
 *
 * Note: option 0 is also available if the RAM firmware is loaded.
 */

/* service upwr_pwm_set_domain_pmic_rail message argument fields*/
typedef struct {
	uint32_t domain : 16U;
	uint32_t rail : 16U;
} upwr_pwm_dom_pmic_rail_args;

#define UPWR_FILL_DOMBIAS_ARGS(dom, bias, args)           \
do {                                                      \
	(args).B.domapply = (args).B.avdapply = 0U;            \
	switch ((bias)->apply) {                            \
	case BIAS_APPLY_RTD_AVD:                  \
		(args).B.avdapply = 1U;              \
	/* fall through */                        \
	case BIAS_APPLY_RTD:                      \
		(dom) = (uint32_t)RTD_DOMAIN;       \
		(args).B.domapply = 1U;              \
		break;                            \
	case BIAS_APPLY_APD_AVD:                  \
		(args).B.avdapply = 1U;              \
	/* fall through */                      \
	case BIAS_APPLY_APD:                      \
		(dom) = (uint32_t)APD_DOMAIN;       \
		(args).B.domapply = 1U;              \
		break;                            \
	case BIAS_APPLY_AVD:                      \
		(args).B.avdapply = 1U;              \
		break;                            \
	default:                              \
		break;                            \
	}                                                 \
	(args).B.dommode = (uint32_t)((bias)->dommode);         \
	(args).B.avdmode = (uint32_t)((bias)->avdmode);         \
	uint32_t sat = UPWR_BIAS2MILIV((1UL << UPWR_DOMBIAS_RBB_BITS) - 1UL);\
	(args).B.domrbbn = ((bias)->dombias.rbbn > sat) ? sat : \
			   UPWR_BIAS_MILIV((bias)->dombias.rbbn); \
	(args).B.domrbbp = ((bias)->dombias.rbbp > sat) ? sat : \
			   UPWR_BIAS_MILIV((bias)->dombias.rbbp); \
	(args).B.avdrbbn = ((bias)->avdbias.rbbn > sat) ? sat : \
			   UPWR_BIAS_MILIV((bias)->avdbias.rbbn); \
	(args).B.avdrbbp = ((bias)->avdbias.rbbp > sat) ? sat : \
			   UPWR_BIAS_MILIV((bias)->avdbias.rbbp); \
} while (false)

#define UPWR_FILL_MEMBIAS_ARGS(bias, args)		\
do {							\
	(args).B.en = (bias)->en;			\
} while (false)


#define UPWR_APD_CORES      (2U)
#define UPWR_RTD_CORES      (1U)

#define RTD_DOMAIN (0U)
#define APD_DOMAIN (1U)
#define UPWR_MAIN_DOMAINS (2U)
#define AVD_DOMAIN (2U)
#define UPWR_DOMAIN_COUNT (3U)
#define PSD_DOMAIN (3U)
#define UPWR_ALL_DOMAINS (4U)

typedef uint32_t soc_domain_t;

/*=========================================================================
 * UNIT CONVERSION MACROS
 *   These macros convert physical units to the values passed as arguments
 *   in API functions.
 *=========================================================================
 */

#define UPWR_VOLT_MILIV(v) (v)        /* voltage in mV    to argument value */
#define UPWR_VOLT_MICROV(v)((v) / 1000U) /* voltage in uV    to argument value */
#define UPWR_BIAS_MILIV(v) (((v) + 49UL) / 50UL)   /* bias voltage(mV) to argument value */
#define UPWR_BIAS2MILIV(v) ((v) * 50UL)   /* inverse of UPWR_BIAS_MILIV         */
#define UPWR_FREQ_KHZ(f)   (f)        /* frequency (kHz)  to argument value */

#define UPWR_DOMBIAS_MAX_MV      (UPWR_BIAS2MILIV((1U << UPWR_DOMBIAS_RBB_BITS) - 1U))

/**---------------------------------------------------------------
 * EXCEPTION SERVICE GROUP
 */

/**+
 * upwr_xcp_config()
 *
 * The i.MX8ULP uPower configuration struct contains the following bitfields:
 *
 *  - ALARM_INT (1 bit): tells which RTD MU interrupt should be used for alarms;
 *    1= MU GPI1; 0= MU GPI0; APD alarms always use GPI0.
 *  - CFG_IOMUX (1 bit): determintes if uPower configures i.MX8ULP IOMUX for
 *    I2C and mode pins used to control an external PMIC;
 *    1= uPower firmware or PMIC driver configures i.MX8ULP IOMUX and mode pins;
 *    0= i.MX8ULP IOMUX and mode pins not configured by uPower;
 *  - DGNBUFBITS (4 bits): determines the diagnostic buffer size according to
 *    the formula: size = 2^(DGNBUFBITS+3) bytes;
 *
 *  Defaults are all zeroes; all other bits are reserved, and must be written 0.
 */

typedef union {
	uint32_t R;
	struct {
		uint32_t ALARM_INT : 1U;
		uint32_t CFG_IOMUX : 1U;
		uint32_t DGNBUFBITS : 4U;
		uint32_t RSV : 26U;
	} B;
} upwr_xcp_config_t;

/**+
 * upwr_xcp_sw_alarm()
 *
 * Argument code is defined by the enum upwr_alarm_t, with the values:
 *  - UPWR_ALARM_INTERNAL: internal software error
 *  - UPWR_ALARM_EXCEPTION: uPower core exception, either illegal instruction or
 *    bus error
 *  - UPWR_ALARM_SLACK: delay path too slow, meaning a timing violation occurred
 *    or is iminent.
 *  - UPWR_ALARM_VOLTAGE: one of the measured voltages is below safety margins.
 *
 * Note that this service emulates an alarm that would normally be issued by
 * uPower when it detects one of the causes above. A request to alarm the APD
 * domain when it is powered off returns success, but is ineffective.
 *
 */

#define	UPWR_ALARM_INTERNAL   (0U) /* internal error */
#define	UPWR_ALARM_EXCEPTION  (1U) /* core exception */
#define	UPWR_ALARM_SLACK      (2U) /* delay path too slow */
#define	UPWR_ALARM_VOLTAGE    (3U) /* voltage drop */
#define	UPWR_ALARM_LAST       UPWR_ALARM_VOLTAGE

typedef uint32_t upwr_alarm_t;

/**---------------------------------------------------------------
 * POWER MANAGEMENT SERVICE GROUP
 */

/* values in mV: */
#define UPWR_RTD_RBBN_MAX     (1300U) /* max. RTD Reverse Back Bias N-Well */
#define UPWR_RTD_RBBN_MIN      (100U) /* min. RTD Reverse Back Bias N-Well */

#define UPWR_RTD_RBBP_MAX     (1300U) /* max. RTD Reverse Back Bias P-Well */
#define UPWR_RTD_RBBP_MIN      (100U) /* min. RTD Reverse Back Bias P-Well */

/* APD bias can only two values (mV): */
#define UPWR_APD_RBBN_LO      (1000U) /* low  APD Reverse Back Bias N-Well */
#define UPWR_APD_RBBN_HI      (1300U) /* high APD Reverse Back Bias N-Well */

#define UPWR_APD_RBBP_LO      (1000U) /* low  APD Reverse Back Bias P-Well */
#define UPWR_APD_RBBP_HI      (1300U) /* high APD Reverse Back Bias P-Well */

/* AVD bias can only two values (mV): */
#define UPWR_AVD_RBBN_LO      (1000U) /* low  AVD Reverse Back Bias N-Well */
#define UPWR_AVD_RBBN_HI      (1300U) /* high AVD Reverse Back Bias N-Well */

#define UPWR_AVD_RBBP_LO      (1000U) /* low  AVD Reverse Back Bias P-Well */
#define UPWR_AVD_RBBP_HI      (1300U) /* high AVD Reverse Back Bias P-Well */

/**+
 * upwr_pwm_param()
 *
 * Argument param is defined by the struct/union upwr_pwm_param_t with the
 * following i.MX8ULP-specific bitfields:
 * - DPD_ALLOW (1 bit): 1= allows uPower power mode to go Deep Power Down (DPD);
 *   uPower DPD also depends on other conditions, but if this bit is 0 uPower
 *   won't go DPD even if those conditions are met; it can go either Sleep or
 *   Deep Sleep (DSL) depending on the other configurations.
 * - DSL_DIS (1 bit): if this bit is 1, uPower power mode won't go Deep Sleep
 *   (DSL) even if the other conditions for that are met;
 *   it may go Sleep instead.
 * - SLP_ALLOW (1 bit): if this bit is 1, uPower power mode will go Sleep if
 *   the conditions for Partial Active are met; it may also go Deep Sleep if bit
 *   DSL_DIS=1.
 * - DSL_BGAP_OFF (1 bit): 1= turns bandgap off when uPower goes Deep Sleep;
 *   0= leaves bandgap on when uPower goes Deep Sleep (DSL).
 * - DPD_BGAP_ON (1 bit): 1= leaves bandgap on when uPower goes Deep Power Down
 *   (DPD); 0= powers off bandgap when uPower goes Deep Power Down (DPD).
 *
 *  Defaults are all zeroes; all other bits are reserved, and must be written 0.
 */

typedef union {
	uint32_t R;
	struct {
		uint32_t DPD_ALLOW : 1U;
		uint32_t DSL_DIS : 1U;
		uint32_t SLP_ALLOW : 1U;
		uint32_t DSL_BGAP_OFF : 1U;
		uint32_t DPD_BGAP_ON : 1U;
		uint32_t RSV : 27U;
	} B;
} upwr_pwm_param_t;

/**+
 * upwr_pwm_chng_reg_voltage()
 *
 * Argument reg is defined by the enum upwr_pmc_reg_t, with regulator ids:
 *  - RTD_PMC_REG: RTD regulator
 *  - APD_PMC_REG: APD regulator
 *  - RTD_BIAS_PMC_REG: RTD bias regulator
 *  - APD_BIAS_PMC_REG: APD bias regulator
 *  - RTD_LVD_PMC_MON: RTD LVD regulator
 *  - APD_LVD_PMC_MON: APD LVD regulator
 *  - AVD_LVD_PMC_MON: AVD LVD regulator
 *
 * Argument volt is defined by the formula:
 *
 * argument = 92.30797633*V - 55.000138, rounded to the nearest integer,
 * where V is the value in Volts, with a minimum of 0.595833 V (argument = 0).
 *
 */

/* Regulator ids */
typedef enum {
	RTD_PMC_REG,
	APD_PMC_REG,
	RTD_BIAS_PMC_REG,
	APD_BIAS_PMC_REG,
	RTD_LVD_PMC_MON,
	APD_LVD_PMC_MON,
	AVD_LVD_PMC_MON
} upwr_pmc_reg_t;

/**+
 * upwr_pwm_freq_setup()
 *
 * Argument domain is either RTD_DOMAIN or APD_DOMAIN.
 * Arguments nextfq and currfq are to be defined (TBD).
 */

/**+
 * upwr_pwm_dom_power_on()
 *
 * The arguments must comply with the restrictions below, otherwise the service
 * is not executed and returns error UPWR_RESP_BAD_REQ:
 * - argument domain can only be APD_DOMAIN, because in i.MX8ULP it is not
 *   possible APD powered on (calling the service) with RTD completely
 *   powered off.
 * - the call can only be made from the RTD domain, for the same reason.
 * - argument boot can only be 1, because in i.MX8ULP it is not possible to
 *   power on the APD domain without starting the core boot.
 *
 * If APD is already powered on and booting/booted when the service is called,
 * it returns success without doing anything.
 */

/**+
 * upwr_pwm_boot_start()
 *
 * The arguments must comply with the restrictions below, otherwise the service
 * is not executed and returns error UPWR_RESP_BAD_REQ:
 * - argument domain can only be APD_DOMAIN, because in i.MX8ULP it is not
 *   possible APD powered on (calling the service) with RTD completely
 *   powered off.
 * - the call can only be made from the RTD domain, for the same reason.
 *
 * If APD is already booted when the service is called, it returns success
 * without doing anything. Otherwise, it returns the error UPWR_RESP_BAD_STATE,
 * because in i.MX8ULP APD cannot be booted separately from power on.
 */

/**+
 * upwr_pwm_power_on(),
 * upwr_pwm_power_off(),
 * upwr_pwm_mem_retain()
 *
 * These three service functions use the same arguments:
 *
 * argument swt is an array of one 32-bit word: uint32_t swt[1];
 * naturally the pointer to a single uint32_t variable may be passed.
 * Each bit of the word corresponds to a switch, according to the i.MX8ULP
 * Reference Manual Rev B draft 2 table 64 Power switch reset state,
 * and the following formula:
 *
 * if switch number < 10 bit number = switch number;
 * if switch number >  9 bit number = switch number + 3;
 *
 * bits 9, 10, 11 and 12 must have the same value (corresponding to switch 9)
 *
 * Note: this argument is not used in upwr_pwm_mem_retain.
 *
 * argument mem is an array of two 32-bit words: uint32_t mem[2];
 * naturally the pointer to a single uint64_t variable may be passed, since
 * both ARM and RISC-V are little endian architectures.
 * Each bit of the words corresponds to a memory, according to the i.MX8ULP
 * Reference Manual table "Memory Partitions".
 *
 * Turning a memory completely on (array and peripheral) will automatically
 * turn on its power switch, even if not explicitly commanded.
 * Turning a memory's power switch off will automatically turn off its array
 * and peripheral beforehand, even if not explicitly commanded.
 *
 * Argument restrictions:
 *
 * The swt and mem arguments must comply with the restrictions below, otherwise
 * the service is not executed (no switch/memory is changed) and returns error
 * UPWR_RESP_BAD_REQ:
 *  1. one must not put a memory in retention coming from an off state.
 *  2. switches 9, 10, 11 and 12 must be turned on/off simultaneously.
 *  3. an AVD switch can only be turned off if all AVD switches belong to the
 *     domain requesting the service (as defined by registers SYSCTRL0,
 *     LPAV_MASTER_ALLOC_CTRL and LPAV_SLAVE_ALLOC_CTRL);
 *     there is no such restriction to turn the switch on.
 *  4. an AVD memory can only be turned off or put in retention if all
 *     AVD memories belong to the domain requesting the service
 *     (as defined by registers SYSCTRL0, LPAV_MASTER_ALLOC_CTRL and
 *      LPAV_SLAVE_ALLOC_CTRL); there is no such restriction to turn on the
 *     memories.
 *  5. EdgeLock RAMs must not be turned off, unless RTD domain is in
 *     Deep Power Down (DPD).
 *  6. Power Switch 19 must be on to turn on switches 17 (MIPI/DSI),
 *     18 (MIPI/CSI), and all AVD power switches.
 *
 * Service Errors:
 *
 * Besides the error UPWR_RESP_BAD_REQ caused by violations of the restrictions
 * above, the services may fail with error UPWR_RESP_RESOURCE if a power mode
 * transition or a similar service is executing at the same time.
 * This error should be interpreted as a "try later" response, as the service
 * will succeed once those concurrent executions are done, and no other is
 * started.
 */

/**+
 * upwr_pwm_chng_switch_mem()
 *
 * The bit numbers in the argument struct mask and on/off state fields
 * are the same as for services upwr_pwm_power_on, upwr_pwm_power_off and
 * upwr_pwm_mem_retain.
 *
 * Turning a memory completely on (array and peripheral) will automatically
 * turn on its power switch, even if not explicitly commanded.
 *
 * Argument restrictions:
 *
 * Same argument restrictions as services upwr_pwm_power_on, upwr_pwm_power_off
 * and upwr_pwm_mem_retain, plus the following:
 *
 *  1. one must not turn a memory peripheral on and a memory array off.
 *  2. one must not put a memory in retention and switch its power switch off.
 *
 * Service Errors:
 *
 * Besides the error UPWR_RESP_BAD_REQ caused by violations of the restrictions
 * above, the service may fail with error UPWR_RESP_RESOURCE if a power mode
 * transition or a similar service is executing at the same time.
 * This error should be interpreted as a "try later" response, as the service
 * will succeed once those concurrent executions are done, and no other is
 * started.
 */

/**+
 * upwr_pwm_pmode_config()
 *
 * The same power switch and memory restrictions of service
 * upwr_pwm_chng_switch_mem apply between power modes, however they are not
 * enforced by this service, that is, it does not return service error.
 *
 * The default power mode configurations for RTD and APD are documented in the
 * i.MX8ULP Reference Manual sections "Power mode details (real-time domain)"
 * and "Power mode details (application domain)", respectively.
 * If those configurations are satisfactory, this service does not have
 * to be called.
 *
 * Power Mode Configuration Structure:
 *
 * Follows a description of the power mode configuration structure elements.
 * - dom_swts: the same switch configuration structures used in service
 *             upwr_pwm_chng_switch_mem argument swt.
 * - mem_swts: the same memory configuration structures used in service
 *             upwr_pwm_chng_switch_mem argument mem.
 * - regs: an array of structs base_reg_cfg_t (see upower_soc_defs.h),
 *         one element for each regulator; base_reg_cfg_t has fields
 *         mode (regulator-dependent), lvl (voltage level in uV),
 *         comp (regulator-dependent complamentary info).
 * - pads: pad configuration in low power; see pad_cfg_t definition below.
 * - mons: domain monitors (LVD and HVD) configuration;
 *         see mon_cfg_t definition below.
 * - avd_mons: same as mons for the AVD domain; see mon_cfg_t definition below.
 * - dom_bbias: back-bias configuration for the domain;
 *              see base_bbias_cfg_t definition below.
 * - avd_bbias: back-bias configuration for the AVD domain;
 *              see base_bbias_cfg_t definition below.
 * - mem_bbias: back-bias configuration for the memory;
 *              see base_bbias_cfg_t definition below.
 * - mem_fbias: forward-bias configuration for the memory;
 *              see base_fbias_cfg_t definition below.
 * - pmic: PMIC-specific configuration
 *
 * Structure pad_cfg_t:
 *
 * Pad control for low power modes (power off, etc), 1 bit per pad segment.
 * - rst  : put pad segment in reset.
 * - iso  : put pad segment in isolation.
 * - compl: specific pad segment information.
 * - msk  : select which pads will be updated.
 *
 * Structure mon_cfg_t:
 *
 * Configures a voltage monitor and its actions.
 * There are monitors for RTD, APD and AVD, monitoring LVD and HVD.
 * - lvl  : Voltage level (in uV).
 * - mode : Mode of monitor (ON, OFF, LP, etc).
 * - compl: Extra info for the monitor.
 *
 * Structure base_bbias_cfg_t:
 *
 * Configures back-bias (for domain or memory).
 * - mode : Back bias mode (OFF, RBB, ARBB, etc).
 * - p_lvl: Voltage level of p-well (in mV).
 * - n_lvl: Voltage level of n-well (in mV).
 * - compl: Complementary bias-specific (enable reset, interrupt, clamp, etc).
 *
 * Structure base_fbias_cfg_t:
 *
 * Configure memory forward bias for a memory segment.
 *
 * - mode : Forward bias mode (OFF, ON).
 * - msk  : Selects which memory will be updated
 *
 */

/*=========================================================================
 * Domain bias
 *=========================================================================
 */

/**+
 * upwr_pwm_chng_dom_bias()
 *
 * Argument bias is a pointer to a struct with fields:
 *  - apply: tells to which domains the bias must be applied;
 *    options are RTD only (BIAS_APPLY_RTD), RTD and AVD (BIAS_APPLY_RTD_AVD),
 *    APD only (BIAS_APPLY_APD), APD and AVD (BIAS_APPLY_APD_AVD),
 *    AVD only (BIAS_APPLY_AVD)
 *  - dommode: bias mode of the main domain (RTD or APD, determined by apply);
 *    options are disabled (NBB_BIAS_MODE), reverse back bias (RBB_BIAS_MODE),
 *    asymmetrical forward bias (AFBB_BIAS_MODE), asymmetrical reverse bias
 *    (ARBB_BIAS_MODE).
 *  - avdmode: bias mode of Audio-Video Domain (AVD);
 *    options are the same as dommode.
 *  - dombias: bias voltage level(s) for the main domain (RTD or APD,
 *    determined by apply); it is a structure with 2 fields, rbbn and rbbp,
 *    for the N-well and P-well voltages, respectively; values are in mV.
 *  - avdbias: bias voltage level(s) for the Audio-Video Domain (AVD);
 *    same fields as dombias;
 *
 * Argument restrictions:
 *
 * Voltage levels must comply with the #define-determined limits/options:
 * between UPWR_RTD_RBBN_MIN and UPWR_RTD_RBBN_MAX (inclusive) for RTD N-well;
 * between UPWR_RTD_RBBP_MIN and UPWR_RTD_RBBP_MAX (inclusive) for RTD P-well;
 * either UPWR_APD_RBBN_LO or UPWR_APD_RBBN_HI for APD N-well;
 * either UPWR_APD_RBBP_LO or UPWR_APD_RBBP_HI for APD P-well;
 * either UPWR_AVD_RBBN_LO or UPWR_AVD_RBBN_HI for AVD N-well;
 * either UPWR_AVD_RBBP_LO or UPWR_AVD_RBBP_HI for AVD P-well;
 *
 * But note that the limits/options above do not apply to all bias modes:
 * rbbn is used and checked only in mode RBB_BIAS_MODE;
 * rbbp is used and checked only in modes RBB_BIAS_MODE and ARBB_BIAS_MODE;
 * modes AFBB_BIAS_MODE and NBB_BIAS_MODE use or check neither rbbn nor rbbp;
 *
 * Service error UPWR_RESP_BAD_REQ is returned if the voltage limits/options
 * above are violated.
 */

/* argument struct for service upwr_pwm_chng_dom_bias:
 */

typedef enum {               /* bias modes (both domain and memory): */
	NBB_BIAS_MODE  = 0,  /* bias disabled */
	RBB_BIAS_MODE  = 1,  /* reverse back bias enabled */
	AFBB_BIAS_MODE = 2,  /* asymmetrical forward bias */
	ARBB_BIAS_MODE = 3   /* asymmetrical reverse bias */
} upwr_bias_mode_t;

/* Domain Bias config (one per domain) */

typedef enum {
	BIAS_APPLY_RTD,      /* apply to RTD only    */
	BIAS_APPLY_RTD_AVD,  /* apply to RTD and AVD */
	BIAS_APPLY_APD,      /* apply to APD only    */
	BIAS_APPLY_APD_AVD,  /* apply to APD and AVD */
	BIAS_APPLY_AVD,      /* apply to AVD only    */
	BIAS_APPLY_COUNT     /* number of apply options */
} upwr_bias_apply_t;

typedef struct {
	uint16_t rbbn;    /* reverse back bias N well (mV) */
	uint16_t rbbp;    /* reverse back bias P well (mV) */
} upwr_rbb_t;

struct upwr_dom_bias_cfg_t {
	upwr_bias_apply_t apply;   /* bias application option  */
	upwr_bias_mode_t  dommode; /* RTD/APD bias mode config */
	upwr_bias_mode_t  avdmode; /* AVD     bias mode config */
	upwr_rbb_t        dombias; /* RTD/APD reverse back bias */
	upwr_rbb_t        avdbias; /* AVD     reverse back bias */
};

/* bias struct used in power mode config definitions */

/**
 * When write power mode transition program, please read below comments carefully.
 * The structure and logic is complex, There is a lot of extension and reuse.
 *
 * First, for mode, extend "uint32_t mode" to a union struct, add support for AVD:
 * typedef union {
 *    uint32_t R;
 *    struct {
 *        uint32_t mode : 8;
 *        uint32_t rsrv_1 : 8;
 *        uint32_t avd_mode : 8;
 *        uint32_t rsrv_2 : 8;
 *    } B;
 * } dom_bias_mode_cfg_t;

  Second, if mode is AFBB mode, no need to configure rbbn and rbbp, uPower firmware
  will configure all SRAM_AFBB_0 or SRAM_AFBB_1 for corresponding domain.

  Third, if mode is RBB mode, extend "uint32_t rbbn" and "uint32_t rbbp" to a union
  struct, add support for AVD:
  typedef union {
  uint32_t                  R;
  struct {
    uint32_t                  lvl       : 8;
    uint32_t                  rsrv_1    : 8;
    uint32_t                  avd_lvl   : 8;
    uint32_t                  rsrv_2    : 8;
  }                         B;
} dom_bias_lvl_cfg_t;

 *
 */
typedef struct {
	uint32_t mode; /* Domain bias mode config, extend to dom_bias_mode_cfg_t to support RTD, APD, AVD */
	uint32_t rbbn; /* reverse back bias N well */
	uint32_t rbbp; /* reverse back bias P well */
} UPWR_DOM_BIAS_CFG_T;

/*=========================================================================
 * Memory bias
 *=========================================================================
 */
/**+
 * upwr_pwm_chng_mem_bias()
 *
 * Argument struct contains only the field en, which can be either 1 (bias
 * enabled) or 0 (bias disabled).
 *
 * Argument domain must be either RTD_DOMAIN (Real Time Domain) or APD_DOMAIN
 * (Application Domain).
 */

/* Memory Bias config */
struct upwr_mem_bias_cfg_t {
	uint32_t en; /* Memory bias enable config */
};

/* bias struct used in power mode config definitions */
typedef struct {
	uint32_t en; /* Memory bias enable config */
} UPWR_MEM_BIAS_CFG_T;

/* Split different Bias */
struct upwr_pmc_bias_cfg_t {
	UPWR_DOM_BIAS_CFG_T dombias_cfg; /* Domain Bias config */
	UPWR_MEM_BIAS_CFG_T membias_cfg; /* Memory Bias config */
};

/*=========================================================================
 * Power modes
 *=========================================================================
 */

/* from msb->lsb: Azure bit, dual boot bit, low power boot bit */
typedef enum {
	SOC_BOOT_SINGLE   = 0,
	SOC_BOOT_LOW_PWR  = 1,
	SOC_BOOT_DUAL     = 2,
	SOC_BOOT_AZURE    = 4
} SOC_BOOT_TYPE_T;

#ifdef UPWR_COMP_RAM
/* Power modes for RTD domain  */
typedef enum {
	DPD_RTD_PWR_MODE, /* Real Time Deep Power Down mode */
	PD_RTD_PWR_MODE,  /* Real Time Power Down mode */
	DSL_RTD_PWR_MODE, /* Real Time Domain Deep Sleep Mode */
	HLD_RTD_PWR_MODE, /* Real Time Domain Hold Mode */
	SLP_RTD_PWR_MODE, /* Sleep Mode */
	ADMA_RTD_PWR_MODE,/* Active DMA Mode */
	ACT_RTD_PWR_MODE, /* Active Domain Mode */
	NUM_RTD_PWR_MODES
} upwr_ps_rtd_pwr_mode_t;

/* Abstract power modes */
typedef enum {
	DPD_PWR_MODE,
	PD_PWR_MODE,
	PACT_PWR_MODE,
	DSL_PWR_MODE,
	HLD_PWR_MODE,
	SLP_PWR_MODE,
	ADMA_PWR_MODE,
	ACT_PWR_MODE,
	NUM_PWR_MODES,
	NUM_APD_PWR_MODES = NUM_PWR_MODES,
	TRANS_PWR_MODE    = NUM_PWR_MODES,
	INVALID_PWR_MODE  = TRANS_PWR_MODE + 1
} abs_pwr_mode_t;
#else /* UPWR_COMP_RAM */
/* Power modes for RTD domain  */
#define	DPD_RTD_PWR_MODE   (0U)  /* Real Time Deep Power Down mode */
#define	PD_RTD_PWR_MODE    (1U)  /* Real Time Power Down mode */
#define	DSL_RTD_PWR_MODE   (2U)  /* Real Time Domain Deep Sleep Mode */
#define	HLD_RTD_PWR_MODE   (3U)  /* Real Time Domain Hold Mode */
#define	SLP_RTD_PWR_MODE   (4U)  /* Sleep Mode */
#define	ADMA_RTD_PWR_MODE  (5U)  /* Active DMA Mode */
#define	ACT_RTD_PWR_MODE   (6U)  /* Active Domain Mode */
#define	NUM_RTD_PWR_MODES  (7U)

typedef uint32_t upwr_ps_rtd_pwr_mode_t;

/* Abstract power modes */
#define	DPD_PWR_MODE       (0U)
#define	PD_PWR_MODE        (1U)
#define	PACT_PWR_MODE      (2U)
#define	DSL_PWR_MODE       (3U)
#define	HLD_PWR_MODE       (4U)
#define	SLP_PWR_MODE       (5U)
#define	ADMA_PWR_MODE      (6U)
#define	ACT_PWR_MODE       (7U)
#define	NUM_PWR_MODES      (8U)
#define	NUM_APD_PWR_MODES NUM_PWR_MODES
#define	TRANS_PWR_MODE    NUM_PWR_MODES
#define	INVALID_PWR_MODE  (TRANS_PWR_MODE + 1U)

typedef uint32_t abs_pwr_mode_t;
#endif /* UPWR_COMP_RAM */

typedef struct {
	abs_pwr_mode_t mode;
	bool ok;
} pch_trans_t;

typedef pch_trans_t rtd_trans_t;

typedef struct {
	abs_pwr_mode_t mode;
	pch_trans_t core[UPWR_APD_CORES];
} apd_trans_t;

/* Codes for APD pwr mode as programmed in LPMODE reg */
typedef enum {
	ACT_APD_LPM,
	SLP_APD_LPM = 1,
	DSL_APD_LPM = 3,
	PACT_APD_LPM = 7,
	PD_APD_LPM = 15,
	DPD_APD_LPM = 31,
	HLD_APD_LPM = 63
} upwr_apd_lpm_t;

/* PowerSys low power config */
struct upwr_powersys_cfg_t {
	uint32_t lpm_mode; /* Powersys low power mode */
};

/*=*************************************************************************
 * RTD
 *=*************************************************************************/
/* Config pmc PADs */
struct upwr_pmc_pad_cfg_t {
	uint32_t pad_close;   /* PMC PAD close config */
	uint32_t pad_reset;   /* PMC PAD reset config */
	uint32_t pad_tqsleep; /* PMC PAD TQ Sleep config */
};

/* Config regulator (internal and external) */
struct upwr_reg_cfg_t {
	uint32_t volt;  /* Regulator voltage config */
	uint32_t mode;  /* Regulator mode config */
};

/* Config pmc monitors */
struct  upwr_pmc_mon_cfg_t {
	uint32_t mon_hvd_en; /* PMC mon HVD */
	uint32_t mon_lvd_en; /* PMC mon LVD */
	uint32_t mon_lvdlvl; /* PMC mon LVDLVL */
};

/* Same monitor config for RTD (for compatibility) */
#define upwr_pmc_mon_rtd_cfg_t upwr_pmc_mon_cfg_t

typedef swt_config_t ps_rtd_swt_cfgs_t[NUM_RTD_PWR_MODES];
typedef swt_config_t ps_apd_swt_cfgs_t[NUM_APD_PWR_MODES];

/*=*************************************************************************
 * APD
 *=*************************************************************************/

/* PowerSys PMIC config */
struct upwr_pmic_cfg_t {
	uint32_t volt;
	uint32_t mode;
	uint32_t mode_msk;
};

typedef uint32_t offs_t;

struct ps_apd_pwr_mode_cfg_t {
	#ifdef UPWR_SIMULATOR_ONLY
	struct upwr_switch_board_t *swt_board_offs;
	struct upwr_mem_switches_t *swt_mem_offs;
	#else
	offs_t swt_board_offs;
	offs_t swt_mem_offs;
	#endif
	struct upwr_pmic_cfg_t pmic_cfg;
	struct upwr_pmc_pad_cfg_t pad_cfg;
	struct upwr_pmc_bias_cfg_t bias_cfg;
};

/* Get the pointer to swt config */
static inline struct upwr_switch_board_t*
get_apd_swt_cfg(volatile struct ps_apd_pwr_mode_cfg_t *cfg)
{
	char *ptr;

	ptr = (char *)cfg;
	ptr += (uint64_t)cfg->swt_board_offs;
	return (struct upwr_switch_board_t *)ptr;
}

/* Get the pointer to mem config */
static inline struct upwr_mem_switches_t*
get_apd_mem_cfg(volatile struct ps_apd_pwr_mode_cfg_t *cfg)
{
	char *ptr;

	ptr = (char *)cfg;
	ptr += (uint64_t)cfg->swt_mem_offs;
	return (struct upwr_mem_switches_t *)ptr;
}

/* Power Mode configuration */

#define ps_rtd_pwr_mode_cfg_t upwr_power_mode_cfg_t

/* these typedefs are just for RISC-V sizeof purpose */
typedef uint32_t swt_board_ptr_t;
typedef uint32_t swt_mem_ptr_t;

struct upwr_power_mode_cfg_t {
	#ifdef UPWR_SIMULATOR_ONLY
	struct upwr_switch_board_t *swt_board; /* Swt board for mem. */
	struct upwr_mem_switches_t *swt_mem;   /* Swt to mem. arrays, perif */
	#else
	#ifdef __LP64__
	uint32_t swt_board;
	uint32_t swt_mem;
	#else
	struct upwr_switch_board_t *swt_board; /* Swt board for mem. */
	struct upwr_mem_switches_t *swt_mem;   /* Swt to mem. arrays, perif */
	#endif
	#endif
	struct upwr_reg_cfg_t in_reg_cfg; /* internal regulator config*/
	struct upwr_reg_cfg_t pmic_cfg;  /* external regulator - pmic*/
	struct upwr_pmc_pad_cfg_t pad_cfg;  /* Pad conf for power trans*/
	struct upwr_pmc_mon_rtd_cfg_t mon_cfg; /*monitor configuration */
	struct upwr_pmc_bias_cfg_t bias_cfg; /* Memory/Domain Bias conf */
	struct upwr_powersys_cfg_t pwrsys_lpm_cfg; /* pwrsys low power config*/
};

static inline unsigned int upwr_sizeof_pmode_cfg(uint32_t domain)
{
	switch (domain) {
	case RTD_DOMAIN:
		return sizeof(struct upwr_power_mode_cfg_t) +
					(sizeof(struct upwr_switch_board_t)*
					 UPWR_PMC_SWT_WORDS) +
					(sizeof(struct upwr_mem_switches_t)*
					 UPWR_PMC_MEM_WORDS) -
					2U * (sizeof(void *) - sizeof(swt_board_ptr_t));

		/* fall through */
	case APD_DOMAIN:
		return sizeof(struct ps_apd_pwr_mode_cfg_t) +
					(sizeof(struct upwr_switch_board_t)*
					 UPWR_PMC_SWT_WORDS) +
					(sizeof(struct upwr_mem_switches_t)*
					 UPWR_PMC_MEM_WORDS);

		/* fall through */
	default:
		break;
	}

	return 0;
}

/*=*************************************************************************
 * All configs
 *=*************************************************************************/

/* LVD/HVD monitor config for a single domain */

/* Domain + AVD monitor config
 * For RTD, mapped in mon_cfg.mon_hvd_en
 * For APD, mapped temporarily in pad_cfg.pad_tqsleep
 */
typedef union upwr_mon_cfg_union_t {
	volatile uint32_t R;
	struct {
		/* Original config, not change */
		volatile uint32_t rsrv_1          : 8;
		/* DOM */
		volatile uint32_t dom_lvd_irq_ena : 1;
		volatile uint32_t dom_lvd_rst_ena : 1;
		volatile uint32_t dom_hvd_irq_ena : 1;
		volatile uint32_t dom_hvd_rst_ena : 1;
		volatile uint32_t dom_lvd_lvl     : 4;
		volatile uint32_t dom_lvd_ena     : 1;
		volatile uint32_t dom_hvd_ena     : 1;
		/* AVD */
		volatile uint32_t avd_lvd_irq_ena : 1;
		volatile uint32_t avd_lvd_rst_ena : 1;
		volatile uint32_t avd_hvd_irq_ena : 1;
		volatile uint32_t avd_hvd_rst_ena : 1;
		volatile uint32_t avd_lvd_lvl     : 4;
		volatile uint32_t avd_lvd_ena     : 1;
		volatile uint32_t avd_hvd_ena     : 1;
	}                         B;
} upwr_mon_cfg_t;

/* Get the monitor config word from RAM (domaind and AVD) */
static inline uint32_t get_mon_cfg(uint8_t dom, void *mode_cfg)
{
	if (dom == RTD_DOMAIN) {
		return ((struct ps_rtd_pwr_mode_cfg_t *)mode_cfg)->mon_cfg.mon_hvd_en;
	} else {
		return ((struct ps_apd_pwr_mode_cfg_t *)mode_cfg)->pad_cfg.pad_tqsleep;
	}
}

/* Set the monitor config word in RAM (domaind and AVD) */
static inline void set_mon_cfg(uint8_t dom, void *mode_cfg,
				upwr_mon_cfg_t mon_cfg)
{
	uint32_t *cfg;

	if (dom == RTD_DOMAIN) {
		cfg = (uint32_t *)&((struct ps_rtd_pwr_mode_cfg_t *)mode_cfg)->mon_cfg.mon_hvd_en;
	} else {
		cfg = (uint32_t *)&((struct ps_apd_pwr_mode_cfg_t *)mode_cfg)->pad_cfg.pad_tqsleep;
	}

	*cfg = mon_cfg.R;
}

#define PMIC_REG_VALID_TAG 0xAAU

/**
 * limit the max pmic register->value count to 8
 * each data cost 4 Bytes, totally 32 Bytes
 */
#define MAX_PMIC_REG_COUNT 0x8U

/**
 * the configuration structure for PMIC register setting
 *
 * @ tag: The TAG number to judge if the data is valid or not, valid tag is PMIC_REG_VALID_TAG
 * @ power_mode : corresponding to each domain's power mode
 * RTD refer to upwr_ps_rtd_pwr_mode_t
 * APD refer to abs_pwr_mode_t
 * @ i2c_addr : i2c address
 * @ i2c_data : i2c data value
 */
struct ps_pmic_reg_data_cfg_t {
	uint32_t tag : 8;
	uint32_t power_mode : 8;
	uint32_t i2c_addr : 8;
	uint32_t i2c_data : 8;
};

/* Uniformize access to PMIC cfg for RTD and APD */

typedef union {
	struct upwr_reg_cfg_t RTD;
	struct upwr_pmic_cfg_t APD;
} pmic_cfg_t;

/* Access to PMIC mode mask and AVD mode */

typedef union {
	uint32_t R;
	struct {
		uint8_t mode;     /* Domain PMIC mode */
		uint8_t msk;      /* Domain PMIC mode mask */
		uint8_t avd_mode; /* AVD PMIC mode */
		uint8_t avd_msk;  /* AVD PMIC mode mask */
	} B;
} pmic_mode_cfg_t;

/* Access RTD, APD and AVD modes and masks */
static inline pmic_mode_cfg_t *get_pmic_mode_cfg(uint8_t dom, pmic_cfg_t *cfg)
{
	uint32_t *mode_cfg;

	if (dom == RTD_DOMAIN) {
		mode_cfg = &cfg->RTD.mode;
	} else {
		mode_cfg = &cfg->APD.mode;
	}

	return (pmic_mode_cfg_t *)mode_cfg;
}

static inline uint8_t get_pmic_mode(uint8_t dom, pmic_cfg_t *cfg)
{
	return get_pmic_mode_cfg(dom, cfg)->B.mode;
}

static inline void set_pmic_mode(uint8_t dom, pmic_cfg_t *cfg, uint8_t mode)
{
	get_pmic_mode_cfg(dom, cfg)->B.mode = mode;
}

static inline uint32_t get_pmic_mode_msk(uint8_t dom, pmic_cfg_t *cfg)
{
	pmic_mode_cfg_t   *mode_cfg;

	if (dom == RTD_DOMAIN) {
		mode_cfg = (pmic_mode_cfg_t *)&cfg->RTD.mode;
		return mode_cfg->B.msk;
	} else {
		return cfg->APD.mode_msk;
	}
}

/* Getters and setters for AVD mode and mask */
static inline uint8_t get_avd_pmic_mode(uint8_t dom, pmic_cfg_t *cfg)
{
	return get_pmic_mode_cfg(dom, cfg)->B.avd_mode;
}

static inline void set_avd_pmic_mode(uint8_t dom, pmic_cfg_t *cfg, uint8_t mode)
{
	get_pmic_mode_cfg(dom, cfg)->B.avd_mode = mode;
}

static inline uint8_t get_avd_pmic_mode_msk(uint8_t dom, pmic_cfg_t *cfg)
{
	return get_pmic_mode_cfg(dom, cfg)->B.avd_msk;
}

static inline void set_avd_pmic_mode_msk(uint8_t dom,
					 pmic_cfg_t *cfg,
					 uint8_t msk)
{
	get_pmic_mode_cfg(dom, cfg)->B.avd_msk = msk;
}

struct ps_delay_cfg_t {
	uint32_t tag : 8U;
	uint32_t rsv : 8U;
	uint32_t exitdelay : 16U;   // exit delay in us
};

#define PS_DELAY_TAG 0xA5U

/* max exit delay = 0xffff = 65535 us = 65.5 ms (it is enough...) */
/* with 8 bits, 256 -> not enough */

typedef struct ps_delay_cfg_t ps_rtd_delay_cfgs_t[NUM_RTD_PWR_MODES];
typedef struct ps_delay_cfg_t ps_apd_delay_cfgs_t[NUM_APD_PWR_MODES];

typedef struct ps_rtd_pwr_mode_cfg_t ps_rtd_pwr_mode_cfgs_t[NUM_RTD_PWR_MODES];
typedef struct ps_apd_pwr_mode_cfg_t ps_apd_pwr_mode_cfgs_t[NUM_APD_PWR_MODES];
typedef struct ps_pmic_reg_data_cfg_t ps_rtd_pmic_reg_data_cfgs_t[MAX_PMIC_REG_COUNT];
typedef struct ps_pmic_reg_data_cfg_t ps_apd_pmic_reg_data_cfgs_t[MAX_PMIC_REG_COUNT];

struct ps_pwr_mode_cfg_t {
	ps_rtd_pwr_mode_cfgs_t  ps_rtd_pwr_mode_cfg;
	ps_rtd_swt_cfgs_t       ps_rtd_swt_cfg;
	ps_apd_pwr_mode_cfgs_t  ps_apd_pwr_mode_cfg;
	ps_apd_swt_cfgs_t       ps_apd_swt_cfg;
	ps_rtd_pmic_reg_data_cfgs_t ps_rtd_pmic_reg_data_cfg;
	ps_apd_pmic_reg_data_cfgs_t ps_apd_pmic_reg_data_cfg;
	ps_rtd_delay_cfgs_t    ps_rtd_delay_cfg;
	ps_apd_delay_cfgs_t    ps_apd_delay_cfg;

};

#define UPWR_XCP_MIN_ADDR   (0x28350000U)
#define UPWR_XCP_MAX_ADDR   (0x2836FFFCU)

struct upwr_reg_access_t {
	uint32_t addr;
	uint32_t data;
	uint32_t mask; /* mask=0 commands read */
};

typedef upwr_pointer_msg upwr_xcp_access_msg;

/* unions for the shared memory buffer */

typedef union {
	struct upwr_reg_access_t reg_access;
} upwr_xcp_union_t;

typedef union {
	struct {
		struct ps_rtd_pwr_mode_cfg_t rtd_struct;
		struct upwr_switch_board_t   rtd_switch;
		struct upwr_mem_switches_t   rtd_memory;
	} rtd_pwr_mode;
	struct {
		struct ps_apd_pwr_mode_cfg_t apd_struct;
		struct upwr_switch_board_t   apd_switch;
		struct upwr_mem_switches_t   apd_memory;
	} apd_pwr_mode;
} upwr_pwm_union_t;

#define MAX_SG_EXCEPT_MEM_SIZE sizeof(upwr_xcp_union_t)
#define MAX_SG_PWRMGMT_MEM_SIZE sizeof(upwr_pwm_union_t)

/**
 * VOLTM group need shared memory for PMIC IC configuration
 * 256 Bytes is enough for PMIC register array
 */
#define MAX_SG_VOLTM_MEM_SIZE 256U

#endif /* UPWR_SOC_DEFS_H */
