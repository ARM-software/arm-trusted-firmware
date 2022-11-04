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
#define QTI_SOC_VERSION_MASK                    U(0xFFFF)
#define QTI_SOC_REVISION_REG                    0x1FC8000
#define QTI_SOC_REVISION_MASK                   U(0xFFFF)
#define QTI_JTAG_ID_SHIFT                       12
#define QTI_JTAG_ID_SC7180                      U(0x012C)
#define QTI_JTAG_ID_SC7180P                     U(0x0195)
#define QTI_CHIPINFO_ID_SC7180                  U(0x01A9)
#define QTI_CHIPINFO_ID_SC7180P                 U(0x01EF)
#define QTI_DEFAULT_CHIPINFO_ID                 U(0xFFFF)

static const chip_id_info_t g_map_jtag_chipinfo_id[] = {
	{QTI_JTAG_ID_SC7180, QTI_CHIPINFO_ID_SC7180},
	{QTI_JTAG_ID_SC7180P, QTI_CHIPINFO_ID_SC7180P},
};
#endif /* QTI_MAP_CHIPINFO_H */
