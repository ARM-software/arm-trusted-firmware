#
# Copyright (c) 2021 Sipeed
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Without a management processor there is no SCPI support.
SUNXI_PSCI_USE_SCPI	:=	0
SUNXI_PSCI_USE_NATIVE	:=	1

# The differences between the platforms are covered by the include files.
include plat/allwinner/common/allwinner-common.mk

# the above could be overwritten on the command line
ifeq (${SUNXI_PSCI_USE_SCPI}, 1)
    $(error "R329 does not support SCPI PSCI ops")
endif

# Put NOBITS memory in the first 64K of SRAM A2, overwriting U-Boot's SPL.
SEPARATE_NOBITS_REGION	:=	1
