#
# Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${PSCI_EXTENDED_STATE_ID}, 1)
  $(error "PSCI Compatibility mode can be enabled only if \
				 PSCI_EXTENDED_STATE_ID is not set")
endif

ifneq (${ARCH}, aarch64)
  $(error "PSCI Compatibility mode is only supported for AArch64 platforms")
endif

PLAT_BL_COMMON_SOURCES	+=	plat/compat/aarch64/plat_helpers_compat.S

BL31_SOURCES		+=	plat/common/plat_psci_common.c		\
				plat/compat/plat_pm_compat.c		\
				plat/compat/plat_topology_compat.c

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0
