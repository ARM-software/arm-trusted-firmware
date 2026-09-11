#
# Copyright (c) 2026, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${TRUSTED_BOARD_BOOT},1)
# To gain place in SYSRAM for MbedTLS and crypto lib, disable DEBUG
$(info Force DEBUG=0 for TRUSTED_BOARD_BOOT)
override DEBUG			:=	0
endif
