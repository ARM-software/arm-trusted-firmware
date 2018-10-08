/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef __ARMADA_COMMON_H__
#define __ARMADA_COMMON_H__

#include <io_addr_dec.h>
#include <stdint.h>

int marvell_get_io_dec_win_conf(struct dec_win_config **win, uint32_t *size);

#endif /* __ARMADA_COMMON_H__ */
