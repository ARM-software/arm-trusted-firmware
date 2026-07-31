/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * pwr_utils.c
 *
 * Implements the APIs to convert SW corners/levels (vlvl) to HW corners/levels
 * (hlvl) and vice versa, using the per-resource aux data stored in the RPMh
 * command DB.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <drivers/qti/cmd_db/cmd_db.h>
#include <drivers/qti/pwr_utils/pwr_utils.h>
#include <lib/utils_def.h>

/* Max aux data length (in bytes) supported per resource. */
#define MAX_AUX_DATA_LEN	32U

/* Describes the supported vlvl<->hlvl mapping for a single rail resource. */
struct pwr_utils_lvl_res {
	const char	*name;	/* resource name, e.g. "cx.lvl", "mx.lvl" */
	uint16_t	*vlvls;	/* supported vlvls, indexed by hlvl */
	size_t		count;	/* number of valid entries in vlvls */
};

/*
 * Target specific rail resources.
 *
 * This information would normally live in a dedicated target file or devcfg,
 * but the power utils are meant to be light on code footprint, so it is kept
 * here. Targets that share the same set of rail resources can share this list.
 */
static struct pwr_utils_lvl_res resource_list[] = {
	{"cx.lvl",   NULL, 0},
	{"mx.lvl",   NULL, 0},
	{"ebi.lvl",  NULL, 0},
	{"lcx.lvl",  NULL, 0},
	{"lmx.lvl",  NULL, 0},
	{"gfx.lvl",  NULL, 0},
	{"mss.lvl",  NULL, 0},
	{"ddr.lvl",  NULL, 0},
	{"xo.lvl",   NULL, 0},
	{"mmcx.lvl", NULL, 0},
};

/* Backing store for the per-resource vlvl tables (no malloc available). */
static uint16_t lvl_buf[512];

static bool valid_resource_idx(int resource_idx)
{
	return (resource_idx >= 0) &&
	       ((size_t)resource_idx < ARRAY_SIZE(resource_list));
}

void qti_pwr_utils_init(void)
{
	size_t used = 0U;
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(resource_list); i++) {
		struct pwr_utils_lvl_res *res = &resource_list[i];
		uint32_t data_len = cmd_db_query_len(res->name);
		uint8_t aux_len = (uint8_t)data_len;
		size_t avail, j;

		/* No aux data: leave vlvls NULL and count 0. */
		if (data_len == 0U) {
			continue;
		}

		assert(data_len <= MAX_AUX_DATA_LEN);

		/* Carve the resource's table out of the static buffer. */
		assert((used + (data_len / sizeof(uint16_t))) <=
		       ARRAY_SIZE(lvl_buf));
		res->vlvls = &lvl_buf[used];

		cmd_db_query_aux_data(res->name, &aux_len,
				      (uint8_t *)res->vlvls);
		assert(data_len == (uint32_t)aux_len);

		/*
		 * Count the valid levels. A zero entry past index 0 marks the
		 * end of the table.
		 */
		avail = data_len / sizeof(uint16_t);
		for (j = 0U; j < avail; j++) {
			if ((j != 0U) && (res->vlvls[j] == 0U)) {
				break;
			}
		}

		res->count = j;
		used += avail;
	}
}

int pwr_utils_lvl_resource_idx(const char *res_name)
{
	size_t i;

	if (res_name == NULL) {
		return -1;
	}

	for (i = 0U; i < ARRAY_SIZE(resource_list); i++) {
		if (strcmp(resource_list[i].name, res_name) == 0) {
			return (int)i;
		}
	}

	return -1;	/* Resource not found */
}

static int pwr_utils_hlvl(int resource_idx, int vlvl, int *mapped_vlvl)
{
	const struct pwr_utils_lvl_res *res;
	int discard;
	size_t i;

	if (mapped_vlvl == NULL) {
		mapped_vlvl = &discard;
	}

	if (!valid_resource_idx(resource_idx) || (vlvl < 0)) {
		*mapped_vlvl = RAIL_VOLTAGE_LEVEL_INVALID;
		return -1;
	}

	res = &resource_list[resource_idx];

	/* Return the first hlvl whose vlvl meets or exceeds the request. */
	for (i = 0U; i < res->count; i++) {
		if (res->vlvls[i] >= vlvl) {
			*mapped_vlvl = res->vlvls[i];
			return (int)i;
		}
	}

	/* Requested vlvl exceeds the max supported vlvl for this resource. */
	*mapped_vlvl = RAIL_VOLTAGE_LEVEL_OVERLIMIT;
	return -1;
}

int pwr_utils_hlvl_named_resource(const char *resource, int vlvl,
				  int *mapped_vlvl)
{
	/* pwr_utils_hlvl() validates the index, so no check is needed here. */
	return pwr_utils_hlvl(pwr_utils_lvl_resource_idx(resource), vlvl,
			      mapped_vlvl);
}
