/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __SCPI_H__
#define __SCPI_H__

#include <stddef.h>
#include <stdint.h>

extern void *scpi_secure_message_start(void);
extern void scpi_secure_message_send(unsigned command, size_t size);
extern unsigned scpi_secure_message_receive(void **message_out, size_t *size_out);
extern void scpi_secure_message_end(void);


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
};

typedef uint32_t scpi_status_t;

typedef enum {
	SCPI_CMD_SCP_READY = 0x01,
	SCPI_CMD_SET_CSS_POWER_STATE = 0x04,
	SCPI_CMD_SYS_POWER_STATE = 0x08
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
extern void scpi_set_css_power_state(unsigned mpidr, scpi_power_state_t cpu_state,
		scpi_power_state_t cluster_state, scpi_power_state_t css_state);
uint32_t scpi_sys_power_state(scpi_system_state_t system_state);

#endif	/* __SCPI_H__ */
