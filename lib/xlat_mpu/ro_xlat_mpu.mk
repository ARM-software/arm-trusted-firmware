#
# Copyright (c) 2021, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${USE_DEBUGFS}, 1)
    $(error "Debugfs requires functionality from the dynamic translation \
             library and is incompatible with ALLOW_RO_XLAT_TABLES.")
endif

ifeq (${ARCH},aarch32)
    $(error "The xlat_mpu library does not currently support AArch32.")
endif
