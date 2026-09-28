#
# Copyright (c) 2022-2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file lists all of the architectural features, and initializes
# and enables them based on the configured architecture version.

# This file follows the following format:
#   - Force all mandatory features to FEAT_STATE_ALWAYS (1) based on the configured
#     architecture version, overriding any platform settings. This ensures
#     architecturally mandatory features are always enabled when
#     ARM_ARCH_{MAJOR,MINOR} is set.
#   - By default, leave any optional features disabled if they have not been
#     defined yet, and let them be enabled/disabled as needed by platform.

#
################################################################################
# Force mandatory features based on configured architecture version.
################################################################################
#

# Usage: $(call version-at-least, <major>, <minor>, <limit_major>, <limit_minor>)
version-at-least = $(shell \
    test $(strip $(1)) -gt $(strip $(3)) || \
    { test $(strip $(1)) -eq $(strip $(3)) && \
      test $(strip $(2)) -ge $(strip $(4)); } && echo 1)

# Enable the features which are mandatory from ARCH version 8.1 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,1),)
armv8-1-a-feats         := ENABLE_FEAT_PAN ENABLE_FEAT_VHE

FEAT_LIST               := ${armv8-1-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.2 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,2),)
armv8-2-a-feats         := ENABLE_FEAT_RAS
# 8.1 Compliant
armv8-2-a-feats         += ${armv8-1-a-feats}

FEAT_LIST               := ${armv8-2-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.3 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,3),)
# 8.2 Compliant
armv8-3-a-feats         += ${armv8-2-a-feats}

FEAT_LIST               := ${armv8-3-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.4 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,4),)
armv8-4-a-feats         := ENABLE_FEAT_SEL2 ENABLE_TRF_FOR_NS ENABLE_FEAT_DIT
# 8.3 Compliant
armv8-4-a-feats         += ${armv8-3-a-feats}

FEAT_LIST               := ${armv8-4-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.5 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,5),)
armv8-5-a-feats         := ENABLE_FEAT_SB
# 8.4 Compliant
armv8-5-a-feats         += ${armv8-4-a-feats}

FEAT_LIST               := ${armv8-5-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.6 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,6),)
armv8-6-a-feats         := ENABLE_FEAT_ECV ENABLE_FEAT_FGT
# 8.5 Compliant
armv8-6-a-feats         += ${armv8-5-a-feats}
FEAT_LIST               := ${armv8-6-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.7 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,7),)
armv8-7-a-feats         := ENABLE_FEAT_HCX
# 8.6 Compliant
armv8-7-a-feats         += ${armv8-6-a-feats}
FEAT_LIST               := ${armv8-7-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.8 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,8),)
armv8-8-a-feats		:= ENABLE_FEAT_MOPS
# 8.7 Compliant
armv8-8-a-feats         += ${armv8-7-a-feats}
FEAT_LIST               := ${armv8-8-a-feats}
endif

# Enable the features which are mandatory from ARCH version 8.9 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),8,9),)
armv8-9-a-feats         := ENABLE_FEAT_TCR2 ENABLE_FEAT_DEBUGV8P9	\
			   ENABLE_FEAT_SCTLR2 ENABLE_FEAT_CLRBHB
# 8.8 Compliant
armv8-9-a-feats         += ${armv8-8-a-feats}
FEAT_LIST               := ${armv8-9-a-feats}
endif

# Enable the features which are mandatory from ARCH version 9.0 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),9,0),)
# 8.5 Compliant
armv9-0-a-feats         += ${armv8-5-a-feats}
FEAT_LIST               := ${armv9-0-a-feats}
endif

# Enable the features which are mandatory from ARCH version 9.1 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),9,1),)
# 8.6 and 9.0 Compliant
armv9-1-a-feats         += ${armv8-6-a-feats} ${armv9-0-a-feats}
FEAT_LIST               := ${armv9-1-a-feats}
endif

# Enable the features which are mandatory from ARCH version 9.2 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),9,2),)
# 8.7 and 9.1 Compliant
armv9-2-a-feats         += ${armv8-7-a-feats} ${armv9-1-a-feats}
FEAT_LIST               := ${armv9-2-a-feats}
endif

# Enable the features which are mandatory from ARCH version 9.3 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),9,3),)
# 8.8 and 9.2 Compliant
armv9-3-a-feats         += ${armv8-8-a-feats} ${armv9-2-a-feats}
FEAT_LIST               := ${armv9-3-a-feats}
endif

# Enable the features which are mandatory from ARCH version 9.4 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),9,4),)
# 8.9 and 9.3 Compliant
armv9-4-a-feats         += ${armv8-9-a-feats} ${armv9-3-a-feats}
FEAT_LIST               := ${armv9-4-a-feats}
endif

# Enable the features which are mandatory from ARCH version 9.5 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),9,5),)
# 9.4 Compliant
armv9-5-a-feats         := ENABLE_FEAT_STEP2
armv9-5-a-feats         += ${armv9-4-a-feats}
FEAT_LIST               := ${armv9-5-a-feats}
endif

# Enable the features which are mandatory from ARCH version 9.6 and upwards.
ifneq ($(call version-at-least,$(ARM_ARCH_MAJOR),$(ARM_ARCH_MINOR),9,6),)
# 9.5 Compliant
armv9-6-a-feats         := ENABLE_FEAT_UINJ ENABLE_FEAT_SRMASK
armv9-6-a-feats         += ${armv9-5-a-feats}
FEAT_LIST               := ${armv9-6-a-feats}
endif

# Force all FEAT_* in FEAT_LIST to FEAT_STATE_ALWAYS (1), overriding any
# previous settings from platform makefiles.
$(call set_ones, ${sort ${FEAT_LIST}})
