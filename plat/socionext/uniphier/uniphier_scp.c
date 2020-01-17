/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include "uniphier.h"

#define UNIPHIER_ROM_RSV3		0x5980120c

#define UNIPHIER_STMBE2COM		0x5f800030
#define UNIPHIER_STMTOBEIRQ		0x5f800060
#define UNIPHIER_BETOSTMIRQ0PT		0x5f800070
#define UNIPHIER_BEIRQCLRPT		0x5f800072

#define UNIPHIER_SCP_READY_MAGIC	0x0000b6a5

#define UNIPHIER_SCP_PACKET_START	0xA0
#define UNIPHIER_SCP_PACKET_END		0xA5
#define UNIPHIER_SCP_PACKET_ESC		0xA6
#define UNIPHIER_SCP_IS_CTRL_CODE(c)	(0xA0 <= (c) && (c) <= 0xA6)

int uniphier_scp_is_running(void)
{
	return mmio_read_32(UNIPHIER_STMBE2COM) == UNIPHIER_SCP_READY_MAGIC;
}

void uniphier_scp_start(uint32_t scp_base)
{
	uint32_t tmp;

	mmio_write_32(UNIPHIER_STMBE2COM + 4, scp_base);
	mmio_write_32(UNIPHIER_STMBE2COM, UNIPHIER_SCP_READY_MAGIC);

	do {
		tmp = mmio_read_32(UNIPHIER_ROM_RSV3);
	} while (!(tmp & BIT(8)));

	mmio_write_32(UNIPHIER_ROM_RSV3, tmp | BIT(9));
}

static void uniphier_scp_send_packet(const uint8_t *packet, int packet_len)
{
	uintptr_t reg = UNIPHIER_STMBE2COM;
	uint32_t word;
	int len, i;

	while (packet_len) {
		len = MIN(packet_len, 4);
		word = 0;

		for (i = 0; i < len; i++)
			word |= *packet++ << (8 * i);

		mmio_write_32(reg, word);
		reg += 4;
		packet_len -= len;
	}

	mmio_write_8(UNIPHIER_BETOSTMIRQ0PT, 0x55);

	while (!(mmio_read_32(UNIPHIER_STMTOBEIRQ) & BIT(1)))
		;
	mmio_write_8(UNIPHIER_BEIRQCLRPT, BIT(1) | BIT(0));
}

static void uniphier_scp_send_cmd(const uint8_t *cmd, int cmd_len)
{
	uint8_t packet[32];	/* long enough */
	uint8_t *p = packet;
	uint8_t c;
	int i;

	*p++ = UNIPHIER_SCP_PACKET_START;
	*p++ = cmd_len;

	for (i = 0; i < cmd_len; i++) {
		c = *cmd++;
		if (UNIPHIER_SCP_IS_CTRL_CODE(c)) {
			*p++ = UNIPHIER_SCP_PACKET_ESC;
			*p++ = c ^ BIT(7);
		} else {
			*p++ = c;
		}
	}

	*p++ = UNIPHIER_SCP_PACKET_END;

	uniphier_scp_send_packet(packet, p - packet);
}

#define UNIPHIER_SCP_CMD(name, ...)					\
static const uint8_t __uniphier_scp_##name##_cmd[] = {			\
	__VA_ARGS__							\
};									\
void uniphier_scp_##name(void)						\
{									\
	uniphier_scp_send_cmd(__uniphier_scp_##name##_cmd,		\
			      ARRAY_SIZE(__uniphier_scp_##name##_cmd));	\
}

UNIPHIER_SCP_CMD(open_com, 0x00, 0x00, 0x05)
UNIPHIER_SCP_CMD(system_off, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01)
UNIPHIER_SCP_CMD(system_reset, 0x00, 0x02, 0x00)
