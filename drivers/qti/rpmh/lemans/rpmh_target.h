/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPMH_TARGET_H
#define RPMH_TARGET_H

#define TCS_SIZE	16U

#define RPMH_AOP_MSG_RAM_BASE		0x0c300000U
#define RPMH_AOP_MSG_RAM_SIZE		0x00100000U
#define RPMH_MSG_RAM_SECTION_SIZE	0x10000U
#define RPMH_AOP_MSG_RAM_DICT_BASE	(RPMH_AOP_MSG_RAM_BASE + \
					 RPMH_AOP_MSG_RAM_SIZE - \
					 RPMH_MSG_RAM_SECTION_SIZE)

#endif /* RPMH_TARGET_H */
