#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DRIVERS_PATH :=	drivers/qti

PLAT_INCLUDES	+=	-Iinclude/drivers/qti \
			-Iinclude/drivers/qti/smem

BL31_SOURCES	+=	$(PLAT_DRIVERS_PATH)/smem/smem.c \
			$(PLAT_DRIVERS_PATH)/smem/smem_partition.c
