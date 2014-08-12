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

#include <arch_helpers.h>
#include <platform.h>
#include "juno_def.h"
#include "mhu.h"
#include "scpi.h"

#define MHU_SECURE_SCP_TO_AP_PAYLOAD	(MHU_SECURE_BASE+0x0080)
#define MHU_SECURE_AP_TO_SCP_PAYLOAD	(MHU_SECURE_BASE+0x0280)

#define SIZE_SHIFT	20	/* Bit position for size value in MHU header */
#define SIZE_MASK	0x1ff	/* Mask to extract size value in MHU header*/


void *scpi_secure_message_start(void)
{
	mhu_secure_message_start();

	/* Return address of payload area. */
	return (void *)MHU_SECURE_AP_TO_SCP_PAYLOAD;
}

void scpi_secure_message_send(unsigned command, size_t size)
{
	/* Make sure payload can be seen by SCP */
	if (MHU_PAYLOAD_CACHED)
		flush_dcache_range(MHU_SECURE_AP_TO_SCP_PAYLOAD, size);

	mhu_secure_message_send(command | (size << SIZE_SHIFT));
}

unsigned scpi_secure_message_receive(void **message_out, size_t *size_out)
{
	uint32_t response =  mhu_secure_message_wait();

	/* Get size of payload */
	size_t size = (response >> SIZE_SHIFT) & SIZE_MASK;

	/* Clear size from response */
	response &= ~(SIZE_MASK << SIZE_SHIFT);

	/* Make sure we don't read stale data */
	if (MHU_PAYLOAD_CACHED)
		inv_dcache_range(MHU_SECURE_SCP_TO_AP_PAYLOAD, size);

	if (size_out)
		*size_out = size;

	if (message_out)
		*message_out = (void *)MHU_SECURE_SCP_TO_AP_PAYLOAD;

	return response;
}

void scpi_secure_message_end(void)
{
	mhu_secure_message_end();
}

static void scpi_secure_send32(unsigned command, uint32_t message)
{
	*(__typeof__(message) *)scpi_secure_message_start() = message;
	scpi_secure_message_send(command, sizeof(message));
	scpi_secure_message_end();
}

int scpi_wait_ready(void)
{
	/* Get a message from the SCP */
	scpi_secure_message_start();
	size_t size;
	unsigned command = scpi_secure_message_receive(NULL, &size);
	scpi_secure_message_end();

	/* We are expecting 'SCP Ready', produce correct error if it's not */
	scpi_status_t response = SCP_OK;
	if (command != SCPI_CMD_SCP_READY)
		response = SCP_E_SUPPORT;
	else if (size != 0)
		response = SCP_E_SIZE;

	/* Send our response back to SCP */
	scpi_secure_send32(command, response);

	return response == SCP_OK ? 0 : -1;
}

void scpi_set_css_power_state(unsigned mpidr, scpi_power_state_t cpu_state,
		scpi_power_state_t cluster_state, scpi_power_state_t css_state)
{
	uint32_t state = mpidr & 0x0f;	/* CPU ID */
	state |= (mpidr & 0xf00) >> 4;	/* Cluster ID */
	state |= cpu_state << 8;
	state |= cluster_state << 12;
	state |= css_state << 16;
	scpi_secure_send32(SCPI_CMD_SET_CSS_POWER_STATE, state);
}

uint32_t scpi_sys_power_state(scpi_system_state_t system_state)
{
	uint32_t *response;
	size_t size;
	uint8_t state = system_state & 0xff;

	/* Send the command */
	*(__typeof__(state) *)scpi_secure_message_start() = state;
	scpi_secure_message_send(SCPI_CMD_SYS_POWER_STATE, sizeof(state));
	scpi_secure_message_receive((void *)&response, &size);
	scpi_secure_message_end();
	return *response;
}
