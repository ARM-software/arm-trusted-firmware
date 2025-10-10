/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCMI_IMX9_H
#define SCMI_IMX9_H

#include <stddef.h>
#include <stdint.h>

#define SCMI_CPU_SLEEP_RUN			0
#define SCMI_CPU_SLEEP_WAIT			1
#define SCMI_CPU_SLEEP_STOP			2
#define SCMI_CPU_SLEEP_SUSPEND			3

#define SCMI_CPU_PD_LPM_ON_NEVER		0U
#define SCMI_CPU_PD_LPM_ON_RUN			1U
#define SCMI_CPU_PD_LPM_ON_RUN_WAIT		2U
#define SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP	3U
#define SCMI_CPU_PD_LPM_ON_ALWAYS		4U

#define MAX_PER_LPI_CONFIGS_PER_CMD		9

#define IMX9_SCMI_PERF_PROTO_ID			0x13

#define IMX9_SCMI_CORE_PERFLEVELSET_MSG		0x7
#define IMX9_SCMI_CORE_PERFLEVELSET_MSG_LEN	12
#define IMX9_SCMI_CORE_PERFLEVELSET_RESP_LEN	8

#define IMX9_SCMI_CORE_PROTO_ID			0x82

#define IMX9_SCMI_CORE_START_MSG		0x4
#define IMX9_SCMI_CORE_START_MSG_LEN		8
#define IMX9_SCMI_CORE_START_RESP_LEN		8

#define IMX9_SCMI_CORE_STOP_MSG			0x4
#define IMX9_SCMI_CORE_STOP_MSG_LEN		8
#define IMX9_SCMI_CORE_STOP_RESP_LEN		8

#define IMX9_SCMI_CORE_RESET_ADDR_SET_MSG	0x6
#define IMX9_SCMI_CORE_RESET_ADDR_SET_MSG_LEN	20
#define IMX9_SCMI_CORE_RESET_ADDR_SET_RESP_LEN	8

#define IMX9_SCMI_CORE_SETSLEEPMODE_MSG		0x7
#define IMX9_SCMI_CORE_SETSLEEPMODE_MSG_LEN	16
#define IMX9_SCMI_CORE_SETSLEEPMODE_RESP_LEN	8

#define IMX9_SCMI_CORE_SETIRQWAKESET_MSG	0x8
#define IMX9_SCMI_CORE_SETIRQWAKESET_MSG_LEN	76
#define IMX9_SCMI_CORE_SETIRQWAKESET_RESP_LEN	8

#define IMX9_SCMI_CORE_NONIRQWAKESET_MSG       0x9
#define IMX9_SCMI_CORE_NONIRQWAKESET_MSG_LEN   64
#define IMX9_SCMI_CORE_NONIRQWAKESET_RESP_LEN  8

#define IMX9_SCMI_CORE_LPMMODESET_MSG		0xA
#define IMX9_SCMI_CORE_LPMMODESET_MSG_LEN	12
#define IMX9_SCMI_CORE_LPMMODESET_RESP_LEN	8

#define IMX9_SCMI_PER_LPMMODESET_MSG		0xB
#define IMX9_SCMI_PER_LPMMODESET_MSG_LEN	12
#define IMX9_SCMI_PER_LPMMODESET_RESP_LEN	8

#define IMX9_SCMI_CORE_GETINFO_MSG		0xC
#define IMX9_SCMI_CORE_GETINFO_MSG_LEN		8
#define IMX9_SCMI_CORE_GETINFO_RESP_LEN		24

#define SCMI_CPU_VEC_FLAGS_BOOT			BIT(30)
#define SCMI_CPU_VEC_FLAGS_RESUME		BIT(31)

#define SCMI_GPC_WAKEUP				0
#define SCMI_GIC_WAKEUP				1
#define SCMI_RESUME_CPU				BIT(1)

#define SCMI_IMX_SYS_POWER_STATE_MODE_MASK      0xC0000000U
#define SM_ACTIVE		BIT(0)
#define FRO_ACTIVE		BIT(1)
#define SYSCTR_ACTIVE		BIT(2)
#define PMIC_STBY_INACTIVE	BIT(3)
#define OSC24M_ACTIVE		BIT(4)

#define IMX9_SCMI_SYS_PWR_FULL_SHUTDOWN		0x80000001U
#define IMX9_SCMI_SYS_PWR_FULL_RESET		0x80000002U

/*
 * sleep_mode[3:0] – Currently unused.
 * sleep_mode[7:4] – sleep mode performance level
 */
#define SM_PERF_LVL_PRK U(0)
#define SM_PERF_LVL_LOW U(1)
#define SM_PERF_LVL_NOM U(2)
#define SM_PERF_LVL_ODV U(3)
#define SM_PERF_LVL_SOD U(4)

#define SYS_SLEEP_MODE_L(x)	(((x) & 0xF) << 16U)
#define SYS_SLEEP_MODE_H(x)	(((x) & 0xF) << 20U)
#define SYS_SLEEP_FLAGS(x)	(((x) & 0xFFFFU))

struct scmi_cpu_reset_addr_a2p {
	uint32_t cpu_id;
	uint32_t flags;
	uint32_t reset_vector_low;
	uint32_t reset_vector_high;
};

struct scmi_cpu_reset_addr_p2a {
	int32_t status;
};

struct scmi_cpu_start_a2p {
	uint32_t cpu_id;
};

struct scmi_cpu_start_p2a {
	int32_t status;
};

struct scmi_cpu_stop_a2p {
	uint32_t cpu_id;
};

struct scmi_cpu_stop_p2a {
	int32_t status;
};

struct scmi_lpm_config {
	uint32_t power_domain;
	uint32_t lpmsetting;
	uint32_t retentionmask;
};

struct scmi_cpu_pd_info {
	uint32_t cpu_id;
	uint32_t cpu_pd_id;
	uint32_t nmem;
	uint32_t *cpu_mem_pd_id;
};

/*
 * SCMI CPU peripheral LPM configuration
 */
struct scmi_per_lpm_config {
	uint32_t perId;
	uint32_t lpmSetting;
};

int scmi_core_set_reset_addr(void *p, uint64_t reset_addr, uint32_t cpu_id, uint32_t attr);
int scmi_core_start(void *p, uint32_t cpu_id);
int scmi_core_stop(void *p, uint32_t cpu_id);
int scmi_core_info_get(void *p, uint32_t cpu_id, uint32_t *run, uint32_t *sleep,
		       uint64_t *vector);
int scmi_core_set_sleep_mode(void *p, uint32_t cpu_id, uint32_t wakeup, uint32_t mode);
int scmi_core_Irq_wake_set(void *p, uint32_t cpu_id, uint32_t mask_idx,
			   uint32_t num_mask, uint32_t *mask);
int scmi_core_nonIrq_wake_set(void *p, uint32_t cpu_id, uint32_t mask_idx,
			uint32_t num_mask, uint32_t mask);
int scmi_core_lpm_mode_set(void *p, uint32_t cpu_id, uint32_t num_configs,
			   struct scmi_lpm_config *cfg);
int scmi_per_lpm_mode_set(void *p, uint32_t cpu_id, uint32_t num_configs,
			   struct scmi_per_lpm_config *cfg);
int scmi_perf_mode_set(void *p, uint32_t domain_id, uint32_t perf_level);

#endif /* SCMI_IMX9_H */
