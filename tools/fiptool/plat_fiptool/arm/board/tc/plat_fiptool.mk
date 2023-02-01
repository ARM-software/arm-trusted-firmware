#
# Copyright (c) 2021, NXP. All rights reserved.
# Copyright (c) 2022-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

INCLUDE_PATHS += -I./ \
		 -I../../plat/arm/board/tc

HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
OBJECTS += plat_fiptool/arm/board/tc/plat_def_uuid_config.o
