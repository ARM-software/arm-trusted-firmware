#
# Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_E3})
BL2_SOURCES += drivers/staging/renesas/rcar/ddr/ddr_a/ddr_init_e3.c
else ifeq (${RCAR_LSI},${RCAR_D3})
BL2_SOURCES += drivers/staging/renesas/rcar/ddr/ddr_a/ddr_init_d3.c
else
BL2_SOURCES += drivers/staging/renesas/rcar/ddr/ddr_a/ddr_init_v3m.c
endif
