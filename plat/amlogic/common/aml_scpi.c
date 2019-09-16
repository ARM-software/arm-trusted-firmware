/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <crypto/sha_dma.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <string.h>

#include "aml_private.h"

#define SIZE_SHIFT	20
#define SIZE_MASK	0x1FF
#define SIZE_FWBLK	0x200UL

/*
 * Note: The Amlogic SCP firmware uses the legacy SCPI protocol.
 */
#define SCPI_CMD_SET_CSS_POWER_STATE	0x04
#define SCPI_CMD_SET_SYS_POWER_STATE	0x08

#define SCPI_CMD_JTAG_SET_STATE		0xC0
#define SCPI_CMD_EFUSE_READ		0xC2
#define SCPI_CMD_CHIP_ID		0xC6

#define SCPI_CMD_COPY_FW 0xd4
#define SCPI_CMD_SET_FW_ADDR 0xd3
#define SCPI_CMD_FW_SIZE 0xd2

static inline uint32_t aml_scpi_cmd(uint32_t command, uint32_t size)
{
	return command | (size << SIZE_SHIFT);
}

static void aml_scpi_secure_message_send(uint32_t command, uint32_t size)
{
	aml_mhu_secure_message_send(aml_scpi_cmd(command, size));
}

static uint32_t aml_scpi_secure_message_receive(void **message_out, size_t *size_out)
{
	uint32_t response = aml_mhu_secure_message_wait();

	size_t size = (response >> SIZE_SHIFT) & SIZE_MASK;

	response &= ~(SIZE_MASK << SIZE_SHIFT);

	if (size_out != NULL)
		*size_out = size;

	if (message_out != NULL)
		*message_out = (void *)AML_MHU_SECURE_SCP_TO_AP_PAYLOAD;

	return response;
}

void aml_scpi_set_css_power_state(u_register_t mpidr, uint32_t cpu_state,
			      uint32_t cluster_state, uint32_t css_state)
{
	uint32_t state = (mpidr & 0x0F) | /* CPU ID */
			 ((mpidr & 0xF00) >> 4) | /* Cluster ID */
			 (cpu_state << 8) |
			 (cluster_state << 12) |
			 (css_state << 16);

	aml_mhu_secure_message_start();
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD, state);
	aml_mhu_secure_message_send(aml_scpi_cmd(SCPI_CMD_SET_CSS_POWER_STATE, 4));
	aml_mhu_secure_message_wait();
	aml_mhu_secure_message_end();
}

uint32_t aml_scpi_sys_power_state(uint64_t system_state)
{
	uint32_t *response;
	size_t size;

	aml_mhu_secure_message_start();
	mmio_write_8(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD, system_state);
	aml_mhu_secure_message_send(aml_scpi_cmd(SCPI_CMD_SET_SYS_POWER_STATE, 1));
	aml_scpi_secure_message_receive((void *)&response, &size);
	aml_mhu_secure_message_end();

	return *response;
}

void aml_scpi_jtag_set_state(uint32_t state, uint8_t select)
{
	assert(state <= AML_JTAG_STATE_OFF);

	if (select > AML_JTAG_A53_EE) {
		WARN("BL31: Invalid JTAG select (0x%x).\n", select);
		return;
	}

	aml_mhu_secure_message_start();
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD,
		      (state << 8) | (uint32_t)select);
	aml_mhu_secure_message_send(aml_scpi_cmd(SCPI_CMD_JTAG_SET_STATE, 4));
	aml_mhu_secure_message_wait();
	aml_mhu_secure_message_end();
}

uint32_t aml_scpi_efuse_read(void *dst, uint32_t base, uint32_t size)
{
	uint32_t *response;
	size_t resp_size;

	if (size > 0x1FC)
		return 0;

	aml_mhu_secure_message_start();
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD, base);
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD + 4, size);
	aml_mhu_secure_message_send(aml_scpi_cmd(SCPI_CMD_EFUSE_READ, 8));
	aml_scpi_secure_message_receive((void *)&response, &resp_size);
	aml_mhu_secure_message_end();

	/*
	 * response[0] is the size of the response message.
	 * response[1 ... N] are the contents.
	 */
	if (*response != 0)
		memcpy(dst, response + 1, *response);

	return *response;
}

void aml_scpi_unknown_thermal(uint32_t arg0, uint32_t arg1,
			      uint32_t arg2, uint32_t arg3)
{
	aml_mhu_secure_message_start();
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0x0, arg0);
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0x4, arg1);
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0x8, arg2);
	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD + 0xC, arg3);
	aml_mhu_secure_message_send(aml_scpi_cmd(0xC3, 16));
	aml_mhu_secure_message_wait();
	aml_mhu_secure_message_end();
}

uint32_t aml_scpi_get_chip_id(uint8_t *obuff, uint32_t osize)
{
	uint32_t *response;
	size_t resp_size;

	if ((osize != 16) && (osize != 12))
		return 0;

	aml_mhu_secure_message_start();
	aml_mhu_secure_message_send(aml_scpi_cmd(SCPI_CMD_CHIP_ID, osize));
	aml_scpi_secure_message_receive((void *)&response, &resp_size);
	aml_mhu_secure_message_end();

	if (!((resp_size == 16) && (osize == 16)) &&
	    !((resp_size == 0) && (osize == 12)))
		return 0;

	memcpy((void *)obuff, (const void *)response, osize);

	return osize;
}

static inline void aml_scpi_copy_scp_data(uint8_t *data, size_t len)
{
	void *dst = (void *)AML_MHU_SECURE_AP_TO_SCP_PAYLOAD;
	size_t sz;

	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD, len);
	aml_scpi_secure_message_send(SCPI_CMD_FW_SIZE, len);
	aml_mhu_secure_message_wait();

	for (sz = 0; sz < len; sz += SIZE_FWBLK) {
		memcpy(dst, data + sz, MIN(SIZE_FWBLK, len - sz));
		aml_mhu_secure_message_send(SCPI_CMD_COPY_FW);
	}
}

static inline void aml_scpi_set_scp_addr(uint64_t addr, size_t len)
{
	volatile uint64_t *dst = (uint64_t *)AML_MHU_SECURE_AP_TO_SCP_PAYLOAD;

	/*
	 * It is ok as AML_MHU_SECURE_AP_TO_SCP_PAYLOAD is mapped as
	 * non cachable
	 */
	*dst = addr;
	aml_scpi_secure_message_send(SCPI_CMD_SET_FW_ADDR, sizeof(addr));
	aml_mhu_secure_message_wait();

	mmio_write_32(AML_MHU_SECURE_AP_TO_SCP_PAYLOAD, len);
	aml_scpi_secure_message_send(SCPI_CMD_FW_SIZE, len);
	aml_mhu_secure_message_wait();
}

static inline void aml_scpi_send_fw_hash(uint8_t hash[], size_t len)
{
	void *dst = (void *)AML_MHU_SECURE_AP_TO_SCP_PAYLOAD;

	memcpy(dst, hash, len);
	aml_mhu_secure_message_send(0xd0);
	aml_mhu_secure_message_send(0xd1);
	aml_mhu_secure_message_send(0xd5);
	aml_mhu_secure_message_end();
}

/**
 * Upload a FW to SCP.
 *
 * @param addr: firmware data address
 * @param size: size of firmware
 * @param send: If set, actually copy the firmware in SCP memory otherwise only
 *  send the firmware address.
 */
void aml_scpi_upload_scp_fw(uintptr_t addr, size_t size, int send)
{
	struct asd_ctx ctx;

	asd_sha_init(&ctx, ASM_SHA256);
	asd_sha_update(&ctx, (void *)addr, size);
	asd_sha_finalize(&ctx);

	aml_mhu_secure_message_start();
	if (send == 0)
		aml_scpi_set_scp_addr(addr, size);
	else
		aml_scpi_copy_scp_data((void *)addr, size);

	aml_scpi_send_fw_hash(ctx.digest, sizeof(ctx.digest));
}
