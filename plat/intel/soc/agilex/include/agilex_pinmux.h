/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AGX_PINMUX_H
#define AGX_PINMUX_H

#define AGX_PINMUX_PIN0SEL		0xffd13000
#define AGX_PINMUX_IO0CTRL		0xffd13130
#define AGX_PINMUX_PINMUX_EMAC0_USEFPGA	0xffd13300
#define AGX_PINMUX_IO0_DELAY		0xffd13400

#include "agilex_handoff.h"

void config_pinmux(handoff *handoff);

#endif

