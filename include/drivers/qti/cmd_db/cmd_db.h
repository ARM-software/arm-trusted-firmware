/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CMD_DB_H
#define CMD_DB_H

#include <stdint.h>

/*
 * cmd_db_query_addr - look up a resource address in the RPMh command DB.
 *
 * @res_id: null-terminated resource identifier string (max 8 chars, e.g.
 *          "cx.lvl", "mx.lvl", "xo.lvl", "vrm.soc").
 *
 * Returns the TCS address (slave ID + offset) on success, 0 on failure.
 */
uint32_t cmd_db_query_addr(const char *res_id);

#endif /* CMD_DB_H */
