/* SPDX-License-Identifier: BSD-3-Clause */
/**
 * Copyright 2019-2024 NXP
 *
 * KEYWORDS: micro-power uPower driver API
 * -----------------------------------------------------------------------------
 * PURPOSE: uPower driver API #defines and typedefs shared with the firmware
 * -----------------------------------------------------------------------------
 * PARAMETERS:
 * PARAM NAME RANGE:DESCRIPTION:       DEFAULTS:                           UNITS
 * -----------------------------------------------------------------------------
 * REUSE ISSUES: no reuse issues
 */

#ifndef UPWR_DEFS_H
#define UPWR_DEFS_H

#include <stdint.h>

#ifndef UPWR_PMC_SWT_WORDS
#define UPWR_PMC_SWT_WORDS              (1U)
#endif

#ifndef UPWR_PMC_MEM_WORDS
#define UPWR_PMC_MEM_WORDS              (2U)
#endif

/* ****************************************************************************
 * DOWNSTREAM MESSAGES - COMMANDS/FUNCTIONS
 * ****************************************************************************
 */
#define UPWR_SRVGROUP_BITS  (4U)
#define UPWR_FUNCTION_BITS  (4U)
#define UPWR_PWDOMAIN_BITS  (4U)
#define UPWR_HEADER_BITS   \
		(UPWR_SRVGROUP_BITS + UPWR_FUNCTION_BITS + UPWR_PWDOMAIN_BITS)
#define UPWR_ARG_BITS      (32U - UPWR_HEADER_BITS)
#if   ((UPWR_ARG_BITS & 1U) > 0U)
#error "UPWR_ARG_BITS must be an even number"
#endif
#define UPWR_ARG64_BITS          (64U - UPWR_HEADER_BITS)
#define UPWR_HALF_ARG_BITS       (UPWR_ARG_BITS >> 1U)
#define UPWR_DUAL_OFFSET_BITS    ((UPWR_ARG_BITS + 32U) >> 1U)

/*
 * message header: header fields common to all downstream messages.
 */
struct upwr_msg_hdr {
	uint32_t domain   : UPWR_PWDOMAIN_BITS; /* power domain */
	uint32_t srvgrp   : UPWR_SRVGROUP_BITS; /* service group */
	uint32_t function : UPWR_FUNCTION_BITS; /* function */
	uint32_t arg      : UPWR_ARG_BITS; /* function-specific argument */
};

/* generic 1-word downstream message format */
typedef union {
	struct upwr_msg_hdr  hdr;
	uint32_t             word;  /* message first word */
} upwr_down_1w_msg;

/* generic 2-word downstream message format */
typedef struct {
	struct upwr_msg_hdr  hdr;
	uint32_t             word2;  /* message second word */
} upwr_down_2w_msg;

/* message format for functions that receive a pointer/offset */
typedef struct {
	struct upwr_msg_hdr  hdr;
	uint32_t             ptr; /* config struct offset */
} upwr_pointer_msg;

/* message format for functions that receive 2 pointers/offsets */
typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint64_t rsv : UPWR_HEADER_BITS;
		uint64_t ptr0 : UPWR_DUAL_OFFSET_BITS;
		uint64_t ptr1 : UPWR_DUAL_OFFSET_BITS;
	} ptrs;
} upwr_2pointer_msg;

#define UPWR_SG_EXCEPT   (0U) /* 0 = exception           */
#define UPWR_SG_PWRMGMT  (1U) /* 1 = power management    */
#define UPWR_SG_DELAYM   (2U) /* 2 = delay   measurement */
#define	UPWR_SG_VOLTM    (3U) /* 3 = voltage measurement */
#define UPWR_SG_CURRM    (4U) /* 4 = current measurement */
#define	UPWR_SG_TEMPM    (5U) /* 5 = temperature measurement */
#define	UPWR_SG_DIAG     (6U) /* 6 = diagnostic  */
#define	UPWR_SG_COUNT    (7U)

typedef uint32_t upwr_sg_t;

/* *************************************************************************
 * Initialization - downstream
 ***************************************************************************/
typedef upwr_down_1w_msg upwr_start_msg; /* start command message */
typedef upwr_down_1w_msg upwr_power_on_msg;   /* power on   command message */
typedef upwr_down_1w_msg upwr_boot_start_msg; /* boot start command message */
typedef union {
	struct upwr_msg_hdr hdr;
	upwr_power_on_msg   power_on;
	upwr_boot_start_msg boot_start;
	upwr_start_msg      start;
} upwr_startup_down_msg;

/* *************************************************************************
 * Service Group EXCEPTION - downstream
 ***************************************************************************/

#define	UPWR_XCP_INIT			(0U) /* 0 = init msg (not a service request itself) */
#define	UPWR_XCP_PING			(0U) /* 0 = also ping request, since its response isan init msg */
#define	UPWR_XCP_START			(1U) /* 1 = service start: upwr_start *(not a service request itself) */
#define	UPWR_XCP_SHUTDOWN		(2U) /* 2 = service shutdown: upwr_xcp_shutdown */
#define	UPWR_XCP_CONFIG			(3U) /* 3 = uPower configuration: upwr_xcp_config */
#define	UPWR_XCP_SW_ALARM		(4U) /* 4 = uPower software alarm: upwr_xcp_sw_alarm */
#define	UPWR_XCP_I2C			(5U) /* 5 = I2C access: upwr_xcp_i2c_access */
#define	UPWR_XCP_SPARE_6		(6U) /* 6 = spare */
#define	UPWR_XCP_SET_DDR_RETN		(7U) /* 7 = set/clear ddr retention */
#define UPWR_XCP_SET_RTD_APD_LLWU	(8U) /* 8 = set/clear rtd/apd llwu */
#define	UPWR_XCP_SPARE_8		(8U) /* 8 = spare */
#define UPWR_XCP_SET_RTD_USE_DDR	(9U) /* 9 = M33 core set it is using DDR or not */
#define	UPWR_XCP_SPARE_9		(9U)  /*  9 = spare */
#define	UPWR_XCP_SPARE_10		(10U) /* 10 = spare */
#define	UPWR_XCP_SET_MIPI_DSI_ENA	(10U) /* 10 = set/clear mipi dsi ena */
#define	UPWR_XCP_SPARE_11		(11U) /* 11 = spare */
#define	UPWR_XCP_GET_MIPI_DSI_ENA	(11U) /* 11 = get mipi dsi ena status */
#define	UPWR_XCP_SPARE_12		(12U) /* 12 = spare */
#define	UPWR_XCP_SET_OSC_MODE		(12U) /* 12 = set uPower OSC mode, high or low */
#define	UPWR_XCP_SPARE_13		(13U) /* 13 = spare */
#define	UPWR_XCP_SPARE_14		(14U) /* 14 = spare */
#define	UPWR_XCP_SPARE_15		(15U) /* 15 = spare */
#define	UPWR_XCP_F_COUNT		(16U)

typedef uint32_t upwr_xcp_f_t;
typedef upwr_down_1w_msg    upwr_xcp_ping_msg;
typedef upwr_down_1w_msg    upwr_xcp_shutdown_msg;
typedef upwr_power_on_msg   upwr_xcp_power_on_msg;
typedef upwr_boot_start_msg upwr_xcp_boot_start_msg;
typedef upwr_start_msg      upwr_xcp_start_msg;
typedef upwr_down_2w_msg    upwr_xcp_config_msg;
typedef upwr_down_1w_msg    upwr_xcp_swalarm_msg;
typedef upwr_down_1w_msg    upwr_xcp_ddr_retn_msg;
typedef upwr_down_1w_msg    upwr_xcp_set_mipi_dsi_ena_msg;
typedef upwr_down_1w_msg    upwr_xcp_get_mipi_dsi_ena_msg;
typedef upwr_down_1w_msg    upwr_xcp_rtd_use_ddr_msg;
typedef upwr_down_1w_msg    upwr_xcp_rtd_apd_llwu_msg;
typedef upwr_down_1w_msg    upwr_xcp_set_osc_mode_msg;
typedef upwr_pointer_msg    upwr_xcp_i2c_msg;

 /* structure pointed by message upwr_xcp_i2c_msg */
typedef struct {
	uint16_t addr;
	int8_t data_size;
	uint8_t subaddr_size;
	uint32_t subaddr;
	uint32_t data;
} upwr_i2c_access;

/* Exception all messages */
typedef union {
	struct upwr_msg_hdr       hdr;       /* message header */
	upwr_xcp_ping_msg         ping;      /* ping */
	upwr_xcp_start_msg        start;     /* service start */
	upwr_xcp_shutdown_msg     shutdown;  /* shutdown */
	upwr_xcp_boot_start_msg   bootstart; /* boot start */
	upwr_xcp_config_msg       config;    /* uPower configuration */
	upwr_xcp_swalarm_msg      swalarm;   /* software alarm */
	upwr_xcp_i2c_msg          i2c;       /* I2C access */
	upwr_xcp_ddr_retn_msg     set_ddr_retn;       /* set ddr retention msg */
	upwr_xcp_set_mipi_dsi_ena_msg     set_mipi_dsi_ena; /* set mipi dsi ena msg */
	upwr_xcp_get_mipi_dsi_ena_msg     get_mipi_dsi_ena; /* get mipi dsi ena msg */
	upwr_xcp_rtd_use_ddr_msg     set_rtd_use_ddr; /* set rtd is using ddr msg */
	upwr_xcp_rtd_apd_llwu_msg     set_llwu; /* set rtd/apd llwu msg */
	upwr_xcp_set_osc_mode_msg     set_osc_mode; /* set osc_mode msg */
} upwr_xcp_msg;

/* structure pointed by message upwr_volt_dva_req_id_msg */
typedef struct {
	uint32_t id_word0;
	uint32_t id_word1;
	uint32_t mode;
} upwr_dva_id_struct;

/**
 * PMIC voltage accuracy is 12.5 mV, 12500 uV
 */
#define PMIC_VOLTAGE_MIN_STEP 12500U

/* *************************************************************************
 * Service Group POWER MANAGEMENT - downstream
 ***************************************************************************/

#define	UPWR_PWM_REGCFG    (0U)     /* 0 = regulator config: upwr_pwm_reg_config */
#define UPWR_PWM_DEVMODE   (0U)     /* deprecated, for old compile */
#define	UPWR_PWM_VOLT      (1U)     /* 1 = voltage change: upwr_pwm_chng_reg_voltage */
#define	UPWR_PWM_SWITCH    (2U)     /* 2 = switch control: upwr_pwm_chng_switch_mem */
#define UPWR_PWM_PWR_ON    (3U)     /* 3 = switch/RAM/ROM power on: upwr_pwm_power_on  */
#define	UPWR_PWM_PWR_OFF   (4U)     /* 4 = switch/RAM/ROM power off: upwr_pwm_power_off */
#define	UPWR_PWM_RETAIN    (5U)     /* 5 = retain memory array: upwr_pwm_mem_retain */
#define UPWR_PWM_DOM_BIAS  (6U)     /* 6 = Domain bias control: upwr_pwm_chng_dom_bias */
#define	UPWR_PWM_MEM_BIAS  (7U)     /* 7 = Memory bias control: upwr_pwm_chng_mem_bias */
#define	UPWR_PWM_PMICCFG   (8U)     /* 8 = PMIC configuration:  upwr_pwm_pmic_config */
#define	UPWR_PWM_PMICMOD   (8U)     /* deprecated, for old compile */
#define	UPWR_PWM_PES       (9U)     /* 9 so far, no use */
#define	UPWR_PWM_CONFIG    (10U)    /* 10= apply power mode defined configuration */
#define	UPWR_PWM_CFGPTR    (11U)    /* 11= configuration pointer */
#define	UPWR_PWM_DOM_PWRON (12U)    /* 12 = domain power on: upwr_pwm_dom_power_on */
#define	UPWR_PWM_BOOT      (13U)    /* 13 = boot start: upwr_pwm_boot_start */
#define UPWR_PWM_FREQ      (14U)    /* 14 = domain frequency setup */
#define	UPWR_PWM_PARAM     (15U)    /* 15 = power management parameters */
#define	UPWR_PWM_F_COUNT (16U)

typedef uint32_t upwr_pwm_f_t;

#define MAX_PMETER_SSEL 7U

#define	UPWR_VTM_CHNG_PMIC_RAIL_VOLT    (0U)      /* 0 = change pmic rail voltage */
#define	UPWR_VTM_GET_PMIC_RAIL_VOLT     (1U)      /* 1 = get pmic rail voltage */
#define UPWR_VTM_PMIC_CONFIG            (2U)      /* 2 = configure PMIC IC */
#define UPWR_VTM_DVA_DUMP_INFO          (3U)      /* 3 = dump dva information */
#define UPWR_VTM_DVA_REQ_ID             (4U)      /* 4 = dva request ID array */
#define UPWR_VTM_DVA_REQ_DOMAIN         (5U)      /* 5 = dva request domain */
#define UPWR_VTM_DVA_REQ_SOC            (6U)      /* 6 = dva request the whole SOC */
#define UPWR_VTM_PMETER_MEAS            (7U)      /* 7 = pmeter measure */
#define UPWR_VTM_VMETER_MEAS            (8U)      /* 8 = vmeter measure */
#define UPWR_VTM_PMIC_COLD_RESET        (9U)      /* 9 = pmic cold reset */
#define UPWR_VTM_SET_DVFS_PMIC_RAIL     (10U)     /* 10 = set which domain use which pmic rail, for DVFS use */
#define UPWR_VTM_SET_PMIC_MODE          (11U)     /* 11 = set pmic mode */
#define UPWR_VTM_F_COUNT                (16U)

typedef uint32_t upwr_volt_f_t;

#define VMETER_SEL_RTD 0U
#define VMETER_SEL_LDO 1U
#define VMETER_SEL_APD 2U
#define VMETER_SEL_AVD 3U
#define VMETER_SEL_MAX 3U

/**
 * The total TSEL count is 256
 */
#define MAX_TEMP_TSEL 256U

/**
 * Support 3 temperature sensor, sensor 0, 1, 2
 */
#define MAX_TEMP_SENSOR 2U

#define UPWR_TEMP_GET_CUR_TEMP (0U)  /* 0 = get current temperature */
#define UPWR_TEMP_F_COUNT      (1U)
typedef uint32_t upwr_temp_f_t;

#define UPWR_DMETER_GET_DELAY_MARGIN (0U)  /* 0 = get delay margin */
#define UPWR_DMETER_SET_DELAY_MARGIN (1U) /* 1 = set delay margin */
#define UPWR_PMON_REQ                (2U) /* 2 = process monitor service */
#define UPWR_DMETER_F_COUNT          (3U)

typedef uint32_t upwr_dmeter_f_t;

typedef upwr_down_1w_msg upwr_volt_pmeter_meas_msg;
typedef upwr_down_1w_msg upwr_volt_pmic_set_mode_msg;
typedef upwr_down_1w_msg upwr_volt_vmeter_meas_msg;

struct upwr_reg_config_t {
	uint32_t reg;
};

 /* set of 32 switches */
struct upwr_switch_board_t {
	uint32_t on;   /* Switch on state,1 bit per instance */
	uint32_t mask; /* actuation mask, 1 bit per instance */
};

 /* set of 32 RAM/ROM switches */
struct upwr_mem_switches_t {
	uint32_t array;   /* RAM/ROM array state, 1 bit per instance */
	uint32_t perif;   /* RAM/ROM peripheral state, 1 bit per instance */
	uint32_t mask;    /* actuation mask, 1 bit per instance */
};

typedef upwr_down_1w_msg upwr_pwm_dom_pwron_msg;  /* domain power on message */
typedef upwr_down_1w_msg upwr_pwm_boot_start_msg; /* boot start message */

/* functions with complex arguments use the pointer message formats: */
typedef upwr_pointer_msg upwr_pwm_retain_msg;
typedef upwr_pointer_msg upwr_pwm_pmode_cfg_msg;

#if (UPWR_ARG_BITS < UPWR_DOMBIAS_ARG_BITS)
#if ((UPWR_ARG_BITS + 32) < UPWR_DOMBIAS_ARG_BITS)
#error "too few message bits for domain bias argument"
#endif
#endif

/* service upwr_pwm_chng_dom_bias message argument fields */
#define UPWR_DOMBIAS_MODE_BITS    (2U)
#define UPWR_DOMBIAS_RBB_BITS     (8U)
#define UPWR_DOMBIAS_RSV_BITS     (14U)
#define UPWR_DOMBIAS_ARG_BITS     (UPWR_DOMBIAS_RSV_BITS + \
				  (2U * UPWR_DOMBIAS_MODE_BITS) + \
				  (4U * UPWR_DOMBIAS_RBB_BITS) + 2U)
/*
 * upwr_pwm_dom_bias_args is an SoC-dependent message,
 */
typedef struct {
	uint32_t: 12U; /* TODO: find a way to use UPWR_HEADER_BITS */
	uint32_t dommode : UPWR_DOMBIAS_MODE_BITS;
	uint32_t avdmode : UPWR_DOMBIAS_MODE_BITS;
	uint32_t domapply : 1U;
	uint32_t avdapply : 1U;
	uint32_t rsv : UPWR_DOMBIAS_RSV_BITS;
	uint32_t domrbbn : UPWR_DOMBIAS_RBB_BITS; /* RTD/APD back bias N-well */
	uint32_t domrbbp : UPWR_DOMBIAS_RBB_BITS; /* RTD/APD back bias P-well */
	uint32_t avdrbbn : UPWR_DOMBIAS_RBB_BITS; /* AVD back bias N-well */
	uint32_t avdrbbp : UPWR_DOMBIAS_RBB_BITS; /* AVD back bias P-well */
} upwr_pwm_dom_bias_args;


typedef union {
	struct upwr_msg_hdr hdr; /* message header */
	struct {
		upwr_pwm_dom_bias_args B;
	} args;
} upwr_pwm_dom_bias_msg;

/* service upwr_pwm_chng_mem_bias message argument fields */
/*
 * upwr_pwm_mem_bias_args is an SoC-dependent message,
 * defined in upower_soc_defs.h
 */
typedef struct {
	uint32_t: 12U; /* TODO: find a way to use UPWR_HEADER_BITS */
	uint32_t en : 1U;
	uint32_t rsv : 19U;
} upwr_pwm_mem_bias_args;

typedef union {
	struct upwr_msg_hdr hdr; /* message header */
	struct {
		upwr_pwm_mem_bias_args B;
	} args;
} upwr_pwm_mem_bias_msg;

typedef upwr_pointer_msg upwr_pwm_pes_seq_msg;

/* upwr_pwm_reg_config-specific message format */
typedef upwr_pointer_msg upwr_pwm_regcfg_msg;

/* upwr_volt_pmic_volt-specific message format */
typedef union {
	struct upwr_msg_hdr hdr; /* message header */
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t domain : 8U;
		uint32_t rail : 8U;
	} args;
} upwr_volt_dom_pmic_rail_msg;

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t rail : 4U;  /* pmic rail id  */
		uint32_t volt : 12U; /* voltage value, accurate to mV, support 0~3.3V */
	} args;
} upwr_volt_pmic_set_volt_msg;

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t rail : 16U;  /* pmic rail id  */
	} args;
} upwr_volt_pmic_get_volt_msg;

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv :UPWR_HEADER_BITS;
		uint32_t domain : 8U;
		uint32_t mode : 8U; /* work mode */
	} args;
} upwr_volt_dva_req_domain_msg;

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t mode : 16U;  /* work mode  */
	} args;
} upwr_volt_dva_req_soc_msg;

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t addr_offset : 16U;  /* addr_offset to 0x28330000  */
	} args;
} upwr_volt_dva_dump_info_msg;

typedef upwr_pointer_msg upwr_volt_pmiccfg_msg;
typedef upwr_pointer_msg upwr_volt_dva_req_id_msg;
typedef upwr_down_1w_msg upwr_volt_pmic_cold_reset_msg;

/* upwr_pwm_volt-specific message format */
typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t reg : UPWR_HALF_ARG_BITS;  /* regulator id  */
		uint32_t volt : UPWR_HALF_ARG_BITS; /* voltage value */
	} args;
} upwr_pwm_volt_msg;

/* upwr_pwm_freq_setup-specific message format */
/**
 * DVA adjust stage
 */
#define DVA_ADJUST_STAGE_INVALID 0U
/* first stage, gross adjust, for increase frequency use */
#define DVA_ADJUST_STAGE_ONE 1U
/* second stage, fine adjust for increase frequency use */
#define DVA_ADJUST_STAGE_TWO 2U
/* combine first + second stage, for descrese frequency use */
#define DVA_ADJUST_STAGE_FULL 3U

/**
 * This message structure is used for DVFS feature
 * 1. Because user may use different PMIC or different board,
 * the pmic regulator of RTD/APD may change,
 * so, user need to tell uPower the regulator number.
 * The number must be matched with PMIC IC and board.
 * use 4 bits for pmic regulator, support to 16 regulator.
 *
 * use 2 bits for DVA stage
 *
 * use 10 bits for target frequency, accurate to MHz, support to 1024 MHz
 */
typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t rail : 4; /* pmic regulator  */
		uint32_t stage : 2; /* DVA stage */
		uint32_t target_freq : 10; /* target frequency */
	} args;
} upwr_pwm_freq_msg;

typedef upwr_down_2w_msg upwr_pwm_param_msg;

/* upwr_pwm_pmiccfg-specific message format */
typedef upwr_pointer_msg upwr_pwm_pmiccfg_msg;

/* functions that pass a pointer use message format upwr_pointer_msg */
typedef upwr_pointer_msg upwr_pwm_cfgptr_msg;

/* functions that pass 2 pointers use message format upwr_2pointer_msg
 */
typedef upwr_2pointer_msg upwr_pwm_switch_msg;
typedef upwr_2pointer_msg upwr_pwm_pwron_msg;
typedef upwr_2pointer_msg upwr_pwm_pwroff_msg;

/* Power Management all messages */
typedef union {
	struct upwr_msg_hdr     hdr;      /* message header */
	upwr_pwm_param_msg      param;    /* power management parameters */
	upwr_pwm_dom_bias_msg   dom_bias; /* domain bias message */
	upwr_pwm_mem_bias_msg   mem_bias; /* memory bias message */
	upwr_pwm_pes_seq_msg    pes;      /* PE seq. message */
	upwr_pwm_pmode_cfg_msg  pmode;    /* power mode config message */
	upwr_pwm_regcfg_msg     regcfg;   /* regulator config message */
	upwr_pwm_volt_msg       volt;     /* set voltage message */
	upwr_pwm_freq_msg       freq;     /* set frequency message */
	upwr_pwm_switch_msg     switches; /* switch control message */
	upwr_pwm_pwron_msg      pwron;    /* switch/RAM/ROM power on  message */
	upwr_pwm_pwroff_msg     pwroff;   /* switch/RAM/ROM power off message */
	upwr_pwm_retain_msg     retain;   /* memory retain message */
	upwr_pwm_cfgptr_msg     cfgptr;   /* configuration pointer message*/
	upwr_pwm_dom_pwron_msg  dompwron; /* domain power on message */
	upwr_pwm_boot_start_msg boot;     /* boot start      message */
} upwr_pwm_msg;

typedef union {
	struct upwr_msg_hdr     hdr;      /* message header */
	upwr_volt_pmic_set_volt_msg  set_pmic_volt;     /* set pmic voltage message */
	upwr_volt_pmic_get_volt_msg  get_pmic_volt;     /* set pmic voltage message */
	upwr_volt_pmic_set_mode_msg  set_pmic_mode;     /* set pmic mode message */
	upwr_volt_pmiccfg_msg    pmiccfg;  /* PMIC configuration message */
	upwr_volt_dom_pmic_rail_msg   dom_pmic_rail; /* domain bias message */
	upwr_volt_dva_dump_info_msg    dva_dump_info;  /* dump dva info message */
	upwr_volt_dva_req_id_msg    dva_req_id;  /* dump dva request id array message */
	upwr_volt_dva_req_domain_msg    dva_req_domain;  /* dump dva request domain message */
	upwr_volt_dva_req_soc_msg    dva_req_soc;  /* dump dva request whole soc message */
	upwr_volt_pmeter_meas_msg    pmeter_meas_msg;  /* pmeter measure message */
	upwr_volt_vmeter_meas_msg    vmeter_meas_msg;  /* vmeter measure message */
	upwr_volt_pmic_cold_reset_msg    cold_reset_msg;  /* pmic cold reset message */
} upwr_volt_msg;


typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t sensor_id : 16U; /* temperature sensor id  */
	} args;
} upwr_temp_get_cur_temp_msg;

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t index : 8U; /* the delay meter index  */
		uint32_t path : 8U; /* the critical path number  */
	} args;
} upwr_dmeter_get_delay_margin_msg;

#define MAX_DELAY_MARGIN 63U
#define MAX_DELAY_CRITICAL_PATH 7U
#define MAX_DELAY_METER_NUM 1U

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t index: 4U;  /* the delay meter index  */
		uint32_t path: 4U;  /* the critical path number  */
		uint32_t dm: 8U;  /* the delay margin value of delay meter  */
	} args;
} upwr_dmeter_set_delay_margin_msg;

#define MAX_PMON_CHAIN_SEL 1U

typedef union {
	struct upwr_msg_hdr hdr;
	struct {
		uint32_t rsv : UPWR_HEADER_BITS;
		uint32_t chain_sel : 16U;  /* the process monitor delay chain sel  */
	} args;
} upwr_pmon_msg;

typedef union {
	struct upwr_msg_hdr hdr; /* message header */
	upwr_temp_get_cur_temp_msg get_temp_msg; /* get current temperature message */
} upwr_temp_msg;

typedef union {
	struct upwr_msg_hdr hdr; /* message header */
	upwr_dmeter_get_delay_margin_msg  get_margin_msg; /* get delay margin message */
	upwr_dmeter_set_delay_margin_msg  set_margin_msg; /* set delay margin message */
	upwr_pmon_msg pmon_msg; /* process monitor message */
} upwr_dmeter_msg;

typedef upwr_down_2w_msg upwr_down_max_msg; /* longest downstream msg */

/*
 * upwr_dom_bias_cfg_t and upwr_mem_bias_cfg_t are SoC-dependent structs,
 * defined in upower_soc_defs.h
 */
/* Power and mem switches */
typedef struct {
	volatile struct upwr_switch_board_t swt_board[UPWR_PMC_SWT_WORDS];
	volatile struct upwr_mem_switches_t swt_mem[UPWR_PMC_MEM_WORDS];
} swt_config_t;

/* *************************************************************************
 * Service Group DIAGNOSE - downstream
 ***************************************************************************/
/* Diagnose Functions */
#define	UPWR_DGN_MODE              (0U) /* 0 = diagnose mode: upwr_dgn_mode */
#define	UPWR_DGN_F_COUNT           (1U)
#define UPWR_DGN_BUFFER_EN         (2U)
typedef uint32_t upwr_dgn_f_t;

#define UPWR_DGN_ALL2ERR            (0U) /* record all until an error occurs, freeze recording on error */
#define UPWR_DGN_ALL2HLT            (1U) /* record all until an error occurs, halt core on error */
#define UPWR_DGN_ALL                (2U) /* trace, warnings, errors, task state recorded */
#define UPWR_DGN_MAX                UPWR_DGN_ALL
#define UPWR_DGN_TRACE              (3U) /* trace, warnings, errors recorded */
#define UPWR_DGN_SRVREQ             (4U) /* service request activity recorded */
#define UPWR_DGN_WARN               (5U) /* warnings and errors recorded */
#define UPWR_DGN_ERROR              (6U) /* only errors recorded */
#define UPWR_DGN_NONE               (7U) /* no diagnostic recorded */
#define UPWR_DGN_COUNT              (8U)
typedef uint32_t upwr_dgn_mode_t;

typedef upwr_down_1w_msg upwr_dgn_mode_msg;

typedef union {
	struct upwr_msg_hdr hdr;
	upwr_dgn_mode_msg mode_msg;
} upwr_dgn_msg;

typedef struct {
	struct upwr_msg_hdr hdr;
	uint32_t buf_addr;
} upwr_dgn_v2_msg;

/* diagnostics log types in the shared RAM log buffer */

typedef enum {
	DGN_LOG_NONE       = 0x00000000,
	DGN_LOG_INFO       = 0x10000000,
	DGN_LOG_ERROR      = 0x20000000,
	DGN_LOG_ASSERT     = 0x30000000,
	DGN_LOG_EXCEPT     = 0x40000000,
	DGN_LOG_EVENT      = 0x50000000, // old event trace
	DGN_LOG_EVENTNEW   = 0x60000000, // new event trace
	DGN_LOG_SERVICE    = 0x70000000,
	DGN_LOG_TASKDEF    = 0x80000000,
	DGN_LOG_TASKEXE    = 0x90000000,
	DGN_LOG_MUTEX      = 0xA0000000,
	DGN_LOG_SEMAPH     = 0xB0000000,
	DGN_LOG_TIMER      = 0xC0000000,
	DGN_LOG_CALLTRACE  = 0xD0000000,
	DGN_LOG_DATA       = 0xE0000000,
	DGN_LOG_PCTRACE    = 0xF0000000
} upwr_dgn_log_t;

/* ****************************************************************************
 * UPSTREAM MESSAGES - RESPONSES
 * ****************************************************************************
 */
/* generic ok/ko response message */
#define UPWR_RESP_ERR_BITS (4U)
#define UPWR_RESP_HDR_BITS (UPWR_RESP_ERR_BITS+\
			    UPWR_SRVGROUP_BITS+UPWR_FUNCTION_BITS)
#define UPWR_RESP_RET_BITS (32U - UPWR_RESP_HDR_BITS)

#define UPWR_RESP_OK                (0U) /* no error */
#define UPWR_RESP_SG_BUSY           (1U) /* service group is busy */
#define UPWR_RESP_SHUTDOWN          (2U) /* services not up or shutting down */
#define UPWR_RESP_BAD_REQ           (3U) /* invalid request */
#define UPWR_RESP_BAD_STATE         (4U) /* system state doesn't allow perform the request */
#define UPWR_RESP_UNINSTALLD        (5U) /* service or function not installed */
#define UPWR_RESP_UNINSTALLED       (5U) /* service or function not installed (alias) */
#define UPWR_RESP_RESOURCE          (6U) /* resource not available */
#define UPWR_RESP_TIMEOUT           (7U) /* service timeout */
#define UPWR_RESP_COUNT             (8U)

typedef uint32_t upwr_resp_t;

struct upwr_resp_hdr {
	uint32_t errcode : UPWR_RESP_ERR_BITS;
	uint32_t srvgrp  : UPWR_SRVGROUP_BITS;      /* service group */
	uint32_t function: UPWR_FUNCTION_BITS;
	uint32_t ret     : UPWR_RESP_RET_BITS;      /* return value, if any */
};

/* generic 1-word upstream message format */
typedef union {
	struct upwr_resp_hdr hdr;
	uint32_t word;
} upwr_resp_msg;

/* generic 2-word upstream message format */
typedef struct {
	struct upwr_resp_hdr hdr;
	uint32_t word2;  /* message second word */
} upwr_up_2w_msg;

typedef upwr_up_2w_msg upwr_up_max_msg;

/* *************************************************************************
 * Exception/Initialization - upstream
 ***************************************************************************/
#define UPWR_SOC_BITS    (7U)
#define UPWR_VMINOR_BITS (4U)
#define UPWR_VFIXES_BITS (4U)
#define UPWR_VMAJOR_BITS \
		(32U - UPWR_HEADER_BITS - UPWR_SOC_BITS - UPWR_VMINOR_BITS - UPWR_VFIXES_BITS)

typedef struct {
	uint32_t soc_id;
	uint32_t vmajor;
	uint32_t vminor;
	uint32_t vfixes;
} upwr_code_vers_t;

/* message sent by firmware initialization, received by upwr_init */
typedef union {
	struct upwr_resp_hdr hdr;
	struct {
		uint32_t rsv : UPWR_RESP_HDR_BITS;
		uint32_t soc : UPWR_SOC_BITS;        /* SoC identification */
		uint32_t vmajor : UPWR_VMAJOR_BITS;  /* firmware major version */
		uint32_t vminor : UPWR_VMINOR_BITS;  /* firmware minor version */
		uint32_t vfixes : UPWR_VFIXES_BITS;  /* firmware fixes version */
	} args;
} upwr_init_msg;

/* message sent by firmware when the core platform is powered up */
typedef upwr_resp_msg upwr_power_up_msg;

/* message sent by firmware when the core reset is released for boot */
typedef upwr_resp_msg upwr_boot_up_msg;

/* message sent by firmware when ready for service requests */
#define UPWR_RAM_VMINOR_BITS (7)
#define UPWR_RAM_VFIXES_BITS (6)
#define UPWR_RAM_VMAJOR_BITS (32 - UPWR_HEADER_BITS \
		- UPWR_RAM_VFIXES_BITS - UPWR_RAM_VMINOR_BITS)
typedef union {
	struct upwr_resp_hdr hdr;
	struct {
		uint32_t rsv : UPWR_RESP_HDR_BITS;
		uint32_t vmajor : UPWR_RAM_VMAJOR_BITS; /* RAM fw major version */
		uint32_t vminor : UPWR_RAM_VMINOR_BITS; /* RAM fw minor version */
		uint32_t vfixes : UPWR_RAM_VFIXES_BITS; /* RAM fw fixes version */
	} args;
} upwr_ready_msg;

/* message sent by firmware when shutdown finishes */
typedef upwr_resp_msg upwr_shutdown_msg;

typedef union {
	struct upwr_resp_hdr hdr;
	upwr_init_msg        init;
	upwr_power_up_msg    pwrup;
	upwr_boot_up_msg     booted;
	upwr_ready_msg       ready;
} upwr_startup_up_msg;

/* message sent by firmware for uPower config setting */
typedef upwr_resp_msg upwr_config_resp_msg;

/* message sent by firmware for uPower alarm */
typedef upwr_resp_msg upwr_alarm_resp_msg;

/* *************************************************************************
 * Power Management - upstream
 ***************************************************************************/
typedef upwr_resp_msg upwr_param_resp_msg;

enum work_mode {
	OVER_DRIVE,
	NORMAL_DRIVE,
	LOW_DRIVE
};

#define UTIMER3_MAX_COUNT 0xFFFFU

#endif /* UPWR_DEFS_H */
