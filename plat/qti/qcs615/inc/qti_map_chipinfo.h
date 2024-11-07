/*
 * Copyright (c) 2024, The Linux Foundation. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QTI_MAP_CHIPINFO_H
#define QTI_MAP_CHIPINFO_H

#include <stdint.h>

#include <qti_plat.h>

#define QTI_JTAG_ID_REG                         0x786130
#define QTI_JTAG_ID_SHIFT                       12
#define QTI_JTAG_ID_QCS615                      U(0x02E9)
#define QTI_JTAG_ID_SA6155P                     U(0x00EE)
#define QTI_CHIPINFO_ID_QCS615                  U(0x01E7)
#define QTI_CHIPINFO_ID_SA6155P                 U(0x0179)
#define QTI_DEFAULT_CHIPINFO_ID                 U(0xFFFF)

static const chip_id_info_t g_map_jtag_chipinfo_id[] = {
	{QTI_JTAG_ID_QCS615,  QTI_CHIPINFO_ID_QCS615},
	{QTI_JTAG_ID_SA6155P, QTI_CHIPINFO_ID_SA6155P},
};

#endif /* QTI_MAP_CHIPINFO_H */
