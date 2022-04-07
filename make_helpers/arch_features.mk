#
# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file lists all the checks related to the Architectural Feature
# Enablement flags, based on the Architectural version.

# Enable the features which are mandatory from ARCH version 8.1 and upwards.
ifeq "8.1" "$(word 1, $(sort 8.1 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_PAN		=	1
ENABLE_FEAT_VHE		=	1
endif

# Enable the features which are mandatory from ARCH version 8.4 and upwards.
ifeq "8.4" "$(word 1, $(sort 8.4 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_DIT		=	1
ENABLE_FEAT_SEL2	=	1
endif

# Enable the features which are mandatory from ARCH version 8.5 and upwards.
ifeq "8.5" "$(word 1, $(sort 8.5 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_SB		=	1
endif

# Enable the features which are mandatory from ARCH version 8.6 and upwards.
ifeq "8.6" "$(word 1, $(sort 8.6 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_FGT		=	1
ENABLE_FEAT_ECV		=	1
endif

# Enable the features which are mandatory from ARCH version 8.7 and upwards.
ifeq "8.7" "$(word 1, $(sort 8.7 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_HCX		=	1
endif
