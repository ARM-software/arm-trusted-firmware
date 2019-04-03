#
# Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${RCAR_LSI},${RCAR_E3})
    include drivers/staging/renesas/rcar/ddr/ddr_a/ddr_a.mk
    BL2_SOURCES += drivers/staging/renesas/rcar/ddr/dram_sub_func.c
else ifeq (${RCAR_LSI},${RCAR_D3})
    include drivers/staging/renesas/rcar/ddr/ddr_a/ddr_a.mk
else ifeq (${RCAR_LSI},${RCAR_V3M})
    include drivers/staging/renesas/rcar/ddr/ddr_a/ddr_a.mk
else
    include drivers/staging/renesas/rcar/ddr/ddr_b/ddr_b.mk
    BL2_SOURCES += drivers/staging/renesas/rcar/ddr/dram_sub_func.c
endif
