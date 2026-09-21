/*
 * Copyright (c) 2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_TL_CLIENT_H
#define PM_TL_CLIENT_H

#include <stdint.h>

/*
 * plat_get_tl_fw_base_address() - Query the PLM for the runtime transfer
 *                                 list (tlbin partition) load address.
 * @tl_base: Updated with the resolved TL load address when the query
 *           succeeds. Left untouched on failure. Must not be NULL.
 */
void plat_get_tl_fw_base_address(uintptr_t *tl_base);

#endif /* PM_TL_CLIENT_H */
