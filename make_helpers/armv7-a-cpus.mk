#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch32)
$(error ARM_ARCH_MAJOR=7 mandates ARCH=aarch32)
endif

# For ARMv7, set march32 from platform directive ARMV7_CORTEX_Ax=yes
# and ARM_WITH_NEON=yes/no.
#
# GCC and Clang require -march=armv7-a for C-A9 and -march=armv7ve for C-A15.
# armClang requires -march=armv7-a for all ARMv7 Cortex-A. To comply with
# all, just drop -march and supply only -mcpu.

# Platform can override march32-directive through MARCH32_DIRECTIVE
ifdef MARCH32_DIRECTIVE
march32-directive		:= $(MARCH32_DIRECTIVE)
else
march32-set-${ARM_CORTEX_A5}	:= -mcpu=cortex-a5
march32-set-${ARM_CORTEX_A7}	:= -mcpu=cortex-a7
march32-set-${ARM_CORTEX_A9}	:= -mcpu=cortex-a9
march32-set-${ARM_CORTEX_A12}	:= -mcpu=cortex-a12
march32-set-${ARM_CORTEX_A15}	:= -mcpu=cortex-a15
march32-set-${ARM_CORTEX_A17}	:= -mcpu=cortex-a17
march32-neon-$(ARM_WITH_NEON)	:= -mfpu=neon

# default to -march=armv7-a as target directive
march32-set-yes			?= -march=armv7-a
march32-directive		:= ${march32-set-yes} ${march32-neon-yes}
endif

# Platform may override these extension support directives:
#
# ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING
# Defined if core supports the Large Page Addressing extension.
#
# ARMV7_SUPPORTS_VIRTUALIZATION
# Defined if ARMv7 core supports the Virtualization extension.
#
# ARMV7_SUPPORTS_GENERIC_TIMER
# Defined if ARMv7 core supports the Generic Timer extension.

ifeq ($(filter yes,$(ARM_CORTEX_A7) $(ARM_CORTEX_A12) $(ARM_CORTEX_A15) $(ARM_CORTEX_A17)),yes)
$(eval $(call add_defines,\
    $(sort \
        ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING \
        ARMV7_SUPPORTS_VIRTUALIZATION \
        ARMV7_SUPPORTS_GENERIC_TIMER \
        ARMV7_SUPPORTS_VFP \
)))
endif

ifeq ($(ARM_CORTEX_A5),yes)
$(eval $(call add_define,ARM_CORTEX_A5))
endif
