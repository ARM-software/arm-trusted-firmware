#
# Copyright (c) 2020, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${USE_DEBUGFS}, 1)
    $(error "Debugfs requires functionality from the dynamic translation \
             library and is incompatible with ALLOW_RO_XLAT_TABLES.")
endif

ifeq (${ARCH},aarch32)
    ifeq (${RESET_TO_SP_MIN},1)
       $(error "RESET_TO_SP_MIN requires functionality from the dynamic \
                translation library and is incompatible with \
                ALLOW_RO_XLAT_TABLES.")
    endif
else # if AArch64
    ifeq (${PLAT},tegra)
        $(error "Tegra requires functionality from the dynamic translation \
                 library and is incompatible with ALLOW_RO_XLAT_TABLES.")
    endif
    ifeq (${RESET_TO_BL31},1)
        $(error "RESET_TO_BL31 requires functionality from the dynamic \
                 translation library and is incompatible with \
                 ALLOW_RO_XLAT_TABLES.")
    endif
    ifeq (${SPD},trusty)
        $(error "Trusty requires functionality from the dynamic translation \
                 library and is incompatible with ALLOW_RO_XLAT_TABLES.")
    endif
    ifeq (${SPM_MM},1)
        $(error "SPM_MM requires functionality to change memory region \
                 attributes, which is not possible once the translation tables \
                 have been made read-only.")
    endif
endif
