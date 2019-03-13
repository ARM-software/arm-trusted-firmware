/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SQ_SCPI_H
#define SQ_SCPI_H

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
	SCPI_CMD_SET_POWER_STATE = 0x03,
	SCPI_CMD_SYS_POWER_STATE = 0x05
} scpi_command_t;

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

extern int scpi_wait_ready(void);
extern void scpi_set_sq_power_state(unsigned int mpidr,
					scpi_power_state_t cpu_state,
					scpi_power_state_t cluster_state,
					scpi_power_state_t css_state);
uint32_t scpi_sys_power_state(scpi_system_state_t system_state);
uint32_t scpi_get_draminfo(struct draminfo *info);

#endif /* SQ_SCPI_H */
