#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DEF_UUID := yes

ifeq (${PLAT_DEF_UUID}, yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
ifeq (${ARM_ETHOSN_NPU_TZMP1},1)
HOSTCCFLAGS += -DARM_ETHOSN_NPU_TZMP1
endif
INCLUDE_PATHS += -I./ -I${PLAT_DIR}fip -I../../include/
OBJECTS += ${PLAT_DIR}fip/plat_def_uuid_config.o
endif
