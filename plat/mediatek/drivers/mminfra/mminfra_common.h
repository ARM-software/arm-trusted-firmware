/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MMINFRA_COMMON_H
#define MMINFRA_COMMON_H

#define mminfra_info(fmt, args...)	INFO("[mminfra] %s: "fmt"\n", __func__, ##args)
#define mminfra_err(fmt, args...)	ERROR("[mminfra] %s: "fmt"\n", __func__, ##args)

#endif
