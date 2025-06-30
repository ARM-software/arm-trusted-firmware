#
# Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

# TSP source files specific to Versal NET platform

PLAT_XILINX_COMMON := plat/xilinx/common/
BL32_SOURCES	   += ${GIC_SOURCES}

include ${PLAT_XILINX_COMMON}/tsp/tsp.mk
