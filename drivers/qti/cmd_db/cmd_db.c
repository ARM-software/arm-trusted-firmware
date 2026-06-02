/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <drivers/qti/cmd_db/cmd_db.h>

#include <platform_def.h>

/* CMD DB SMEM layout constants */
#define CMD_DB_VERSION		0x00000001U
#define CMD_DB_MAGIC_NUM	0x0c0330dbU

#define CMD_DB_MAX_RES_ID_LEN	8U
#define CMD_DB_MAX_SLV_ID	8U	/* 3 actual + 5 reserved */

#define CMD_DB_SLV_ID_VALID_LOW		3U
#define CMD_DB_SLV_ID_VALID_HIGH	5U

/* Extract slave ID from a TCS address */
#define ADDR_SLV_ID(addr)	(((addr) >> 16) & 0xfU)

/*
 * SMEM layout structures - must match the AOP firmware layout exactly.
 * Do not reorder fields.
 */
struct cmd_db_entry {
	uint64_t	res_id;
	uint32_t	priority[2];
	uint32_t	addr;
	uint16_t	len;
	uint16_t	offset;
};

struct cmd_db_slv_id_info {
	uint16_t	slv_id;
	uint16_t	header_offset;
	uint16_t	data_offset;
	uint16_t	cnt;
	uint16_t	version;
	uint16_t	reserved[3];
};

struct cmd_db_header {
	uint32_t			version;
	uint32_t			magic_num;
	struct cmd_db_slv_id_info	slv_id_info[CMD_DB_MAX_SLV_ID];
	uint32_t			check_sum;
	uint32_t			reserved;
	uint8_t				data[];
};

static struct cmd_db_header *g_cmd_db;

static int cmd_db_init(void)
{
	struct cmd_db_header *hdr =
		(struct cmd_db_header *)(uintptr_t)QTI_AOP_CMD_DB_BASE;

	if ((hdr->version != CMD_DB_VERSION) ||
	    (hdr->magic_num != CMD_DB_MAGIC_NUM)) {
		return -1;
	}

	g_cmd_db = hdr;
	return 0;
}

static uint64_t res_id_to_u64(const char *res_id)
{
	uint64_t val = 0U;
	unsigned int i;

	for (i = 0U; i < CMD_DB_MAX_RES_ID_LEN; i++) {
		if (res_id[i] == '\0') {
			break;
		}
		val |= (uint64_t)(unsigned char)res_id[i] << (8U * i);
	}

	return val;
}

uint32_t cmd_db_query_addr(const char *res_id)
{
	uint64_t key;
	unsigned int slv;
	unsigned int idx;
	const struct cmd_db_slv_id_info *info;
	const struct cmd_db_entry *entry;

	if (res_id == NULL) {
		return 0U;
	}

	if (g_cmd_db == NULL) {
		if (cmd_db_init() != 0) {
			return 0U;
		}
	}

	key = res_id_to_u64(res_id);

	for (slv = 0U; slv < CMD_DB_MAX_SLV_ID; slv++) {
		info = &g_cmd_db->slv_id_info[slv];

		if ((info->slv_id < CMD_DB_SLV_ID_VALID_LOW) ||
		    (info->slv_id > CMD_DB_SLV_ID_VALID_HIGH)) {
			continue;
		}

		for (idx = 0U; idx < info->cnt; idx++) {
			entry = (const struct cmd_db_entry *)
				(g_cmd_db->data + info->header_offset +
				 idx * sizeof(struct cmd_db_entry));

			if (entry->res_id == key) {
				return entry->addr;
			}
		}
	}

	return 0U;
}
