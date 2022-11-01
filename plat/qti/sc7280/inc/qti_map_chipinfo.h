/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QTI_MAP_CHIPINFO_H
#define QTI_MAP_CHIPINFO_H

#include <stdint.h>

#include <qti_plat.h>

#define QTI_JTAG_ID_REG                         0x786130
#define QTI_JTAG_ID_SHIFT                       12
#define QTI_JTAG_ID_SC7280                      U(0x0193)
#define QTI_JTAG_ID_SC7280P                     U(0x01EB)
#define QTI_JTAG_ID_SC8270                      U(0x01E3)
#define QTI_JTAG_ID_SC8270P                     U(0x020A)
#define QTI_JTAG_ID_SC7270P                     U(0x0215)
#define QTI_CHIPINFO_ID_SC7280                  U(0x01E7)
#define QTI_CHIPINFO_ID_SC7280P                 U(0x0222)
#define QTI_CHIPINFO_ID_SC8270                  U(0x0229)
#define QTI_CHIPINFO_ID_SC8270P                 U(0x0233)
#define QTI_CHIPINFO_ID_SC7270P                 U(0x0237)
#define QTI_DEFAULT_CHIPINFO_ID                 U(0xFFFF)

static const chip_id_info_t g_map_jtag_chipinfo_id[] = {
	{QTI_JTAG_ID_SC7280, QTI_CHIPINFO_ID_SC7280},
	{QTI_JTAG_ID_SC7280P, QTI_CHIPINFO_ID_SC7280P},
	{QTI_JTAG_ID_SC8270, QTI_CHIPINFO_ID_SC8270},
	{QTI_JTAG_ID_SC8270P, QTI_CHIPINFO_ID_SC8270P},
	{QTI_JTAG_ID_SC7270P, QTI_CHIPINFO_ID_SC7270P},
};
#endif /* QTI_MAP_CHIPINFO_H */
