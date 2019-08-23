/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "gxbb_private.h"

#define SIZE_SHIFT	20
#define SIZE_MASK	0x1FF

/*
 * Note: The Amlogic SCP firmware uses the legacy SCPI protocol.
 */
#define SCPI_CMD_SET_CSS_POWER_STATE	0x04
#define SCPI_CMD_SET_SYS_POWER_STATE	0x08

#define SCPI_CMD_JTAG_SET_STATE		0xC0
#define SCPI_CMD_EFUSE_READ		0xC2

static inline uint32_t scpi_cmd(uint32_t command, uint32_t size)
{
	return command | (size << SIZE_SHIFT);
}

void scpi_secure_message_send(uint32_t command, uint32_t size)
{
	mhu_secure_message_send(scpi_cmd(command, size));
}

uint32_t scpi_secure_message_receive(void **message_out, size_t *size_out)
{
	uint32_t response = mhu_secure_message_wait();

	size_t size = (response >> SIZE_SHIFT) & SIZE_MASK;

	response &= ~(SIZE_MASK << SIZE_SHIFT);

	if (size_out != NULL)
		*size_out = size;

	if (message_out != NULL)
		*message_out = (void *)GXBB_MHU_SECURE_SCP_TO_AP_PAYLOAD;

	return response;
}

void scpi_set_css_power_state(u_register_t mpidr, uint32_t cpu_state,
			      uint32_t cluster_state, uint32_t css_state)
{
	uint32_t state = (mpidr & 0x0F) | /* CPU ID */
			 ((mpidr & 0xF00) >> 4) | /* Cluster ID */
			 (cpu_state << 8) |
			 (cluster_state << 12) |
			 (css_state << 16);

	mhu_secure_message_start();
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD, state);
	mhu_secure_message_send(scpi_cmd(SCPI_CMD_SET_CSS_POWER_STATE, 4));
	mhu_secure_message_wait();
	mhu_secure_message_end();
}

uint32_t scpi_sys_power_state(uint64_t system_state)
{
	uint32_t *response;
	size_t size;

	mhu_secure_message_start();
	mmio_write_8(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD, system_state);
	mhu_secure_message_send(scpi_cmd(SCPI_CMD_SET_SYS_POWER_STATE, 1));
	scpi_secure_message_receive((void *)&response, &size);
	mhu_secure_message_end();

	return *response;
}

void scpi_jtag_set_state(uint32_t state, uint8_t select)
{
	assert(state <= GXBB_JTAG_STATE_OFF);

	if (select > GXBB_JTAG_A53_EE) {
		WARN("BL31: Invalid JTAG select (0x%x).\n", select);
		return;
	}

	mhu_secure_message_start();
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD,
		      (state << 8) | (uint32_t)select);
	mhu_secure_message_send(scpi_cmd(SCPI_CMD_JTAG_SET_STATE, 4));
	mhu_secure_message_wait();
	mhu_secure_message_end();
}

uint32_t scpi_efuse_read(void *dst, uint32_t base, uint32_t size)
{
	uint32_t *response;
	size_t resp_size;

	if (size > 0x1FC)
		return 0;

	mhu_secure_message_start();
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD, base);
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD + 4, size);
	mhu_secure_message_send(scpi_cmd(SCPI_CMD_EFUSE_READ, 8));
	scpi_secure_message_receive((void *)&response, &resp_size);
	mhu_secure_message_end();

	/*
	 * response[0] is the size of the response message.
	 * response[1 ... N] are the contents.
	 */
	if (*response != 0)
		memcpy(dst, response + 1, *response);

	return *response;
}

void scpi_unknown_thermal(uint32_t arg0, uint32_t arg1,
			  uint32_t arg2, uint32_t arg3)
{
	mhu_secure_message_start();
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0x0, arg0);
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0x4, arg1);
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0x8, arg2);
	mmio_write_32(GXBB_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0xC, arg3);
	mhu_secure_message_send(scpi_cmd(0xC3, 16));
	mhu_secure_message_wait();
	mhu_secure_message_end();
}
