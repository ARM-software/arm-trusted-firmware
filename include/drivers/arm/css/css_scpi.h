/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CSS_SCPI_H
#define CSS_SCPI_H

#include <stddef.h>
#include <stdint.h>

/*
 * An SCPI command consists of a header and a payload.
 * The following structure describes the header. It is 64-bit long.
 */
typedef struct {
	/* Command ID */
	uint32_t id		: 7;
	/* Set ID. Identifies whether this is a standard or extended command. */
	uint32_t set		: 1;
	/* Sender ID to match a reply. The value is sender specific. */
	uint32_t sender		: 8;
	/* Size of the payload in bytes (0 - 511) */
	uint32_t size		: 9;
	uint32_t reserved	: 7;
	/*
	 * Status indicating the success of a command.
	 * See the enum below.
	 */
	uint32_t status;
} scpi_cmd_t;

typedef enum {
	SCPI_SET_NORMAL = 0,	/* Normal SCPI commands */
	SCPI_SET_EXTENDED	/* Extended SCPI commands */
} scpi_set_t;

enum {
	SCP_OK = 0,	/* Success */
	SCP_E_PARAM,	/* Invalid parameter(s) */
	SCP_E_ALIGN,	/* Invalid alignment */
	SCP_E_SIZE,	/* Invalid size */
	SCP_E_HANDLER,	/* Invalid handler or callback */
	SCP_E_ACCESS,	/* Invalid access or permission denied */
	SCP_E_RANGE,	/* Value out of range */
	SCP_E_TIMEOUT,	/* Time out has ocurred */
	SCP_E_NOMEM,	/* Invalid memory area or pointer */
	SCP_E_PWRSTATE,	/* Invalid power state */
	SCP_E_SUPPORT,	/* Feature not supported or disabled */
	SCPI_E_DEVICE,	/* Device error */
	SCPI_E_BUSY,	/* Device is busy */
};

typedef uint32_t scpi_status_t;

typedef enum {
	SCPI_CMD_SCP_READY = 0x01,
	SCPI_CMD_SET_CSS_POWER_STATE = 0x03,
	SCPI_CMD_GET_CSS_POWER_STATE = 0x04,
	SCPI_CMD_SYS_POWER_STATE = 0x05
} scpi_command_t;

/*
 * Macros to parse SCP response to GET_CSS_POWER_STATE command
 *
 *   [3:0] : cluster ID
 *   [7:4] : cluster state: 0 = on; 3 = off; rest are reserved
 *   [15:8]: on/off state for individual CPUs in the cluster
 *
 * Payload is in little-endian
 */
#define CLUSTER_ID(_resp)		((_resp) & 0xf)
#define CLUSTER_POWER_STATE(_resp)	(((_resp) >> 4) & 0xf)

/* Result is a bit mask of CPU on/off states in the cluster */
#define CPU_POWER_STATE(_resp)		(((_resp) >> 8) & 0xff)

/*
 * For GET_CSS_POWER_STATE, SCP returns the power states of every cluster. The
 * size of response depends on the number of clusters in the system. The
 * SCP-to-AP payload contains 2 bytes per cluster. Make sure the response is
 * large enough to contain power states of a given cluster
 */
#define CHECK_RESPONSE(_resp, _clus) \
	(_resp.size >= (((_clus) + 1) * 2))

typedef enum {
	scpi_power_on = 0,
	scpi_power_retention = 1,
	scpi_power_off = 3,
} scpi_power_state_t;

typedef enum {
	scpi_system_shutdown = 0,
	scpi_system_reboot = 1,
	scpi_system_reset = 2
} scpi_system_state_t;

int scpi_wait_ready(void);
void scpi_set_css_power_state(unsigned int mpidr,
				scpi_power_state_t cpu_state,
				scpi_power_state_t cluster_state,
				scpi_power_state_t css_state);
int scpi_get_css_power_state(unsigned int mpidr, unsigned int *cpu_state_p,
		unsigned int *cluster_state_p);
uint32_t scpi_sys_power_state(scpi_system_state_t system_state);

#endif /* CSS_SCPI_H */
