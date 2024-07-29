#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Get local directory path
define GET_LOCAL_DIR
$(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))
endef

# Clear module source variable
define CLEAR_LOCAL_SRCS
$(eval $(1) :=)
endef

define EXPAND_SUB_MAKEFILE
include $(S)
endef

# Expand sub rules.mk
define INCLUDE_MAKEFILE
$(eval MODULES_SUB_MAKEFILE := $(patsubst %,%/rules.mk,$(1)))
$(foreach S,$(MODULES_SUB_MAKEFILE),$(eval $(EXPAND_SUB_MAKEFILE)))
endef

# Determine option variable is defined or not then define it
define add_defined_option
ifdef $(1)
ifeq ($(strip $(value $(1))),y)
DEFINES += -D$(1)$(if $(value $(1)),=1,)
else ifneq ($(strip $(value $(1))),n)
DEFINES += -D$(1)$(if $(value $(1)),=$(value $(1)),)
endif
endif
endef

define EXPAND_RULES_MAKEFILE
LOCAL_SRCS-y :=
MODULE :=
SUB_RULES-y :=
include $(S)
endef

# INCLUDE_MODULES macro expand included modules rules.mk
# Arguments:
#   $(1) = MODULES variables
define INCLUDE_MODULES
$(eval MODULES_TEMP := $(1))
$(eval MODULES_MAKEFILE := $(patsubst %,%/rules.mk,$(MODULES_TEMP)))
$(foreach S,$(MODULES_MAKEFILE),$(eval $(EXPAND_RULES_MAKEFILE)))
endef

# MAKE_LOCALS expand module source file variable to BL${BL}_SOURCES
# Arguments:
#   $(1) = source file
#   $(2) = BL stage (1, 2, 2u, 31, 32)
define MAKE_LOCALS
$(eval $(call uppercase,$(2))_SOURCES += $(1))
endef

# MAKE_MODULE reference MAKE_OBJS.
# Create module folder under out/bl$(BL)/$(module)
# Arguments:
#   $(1) = module name
#   $(2) = source file
#   $(3) = BL stage
define MAKE_MODULE
        $(eval MODULE := $(strip $(1)))
        $(eval BUILD_DIR  := ${BUILD_PLAT}/${3})
        $(eval SOURCES    := $(2))
        $(eval OBJS_TEMP  := $(addprefix $(BUILD_DIR)/$(MODULE)/,$(call SOURCES_TO_OBJS,$(SOURCES))))
        $(eval MODULE_OBJS += $(OBJS_TEMP))

$(eval $(call MAKE_OBJS,$(BUILD_DIR)/$(MODULE),$(SOURCES),${3}))

libraries: $(OBJS_TEMP)
endef

# Include MTK configuration files

# MTK makefile variables
ifeq (${COREBOOT},1)
MTK_COMMON_CFG := $(MTK_PLAT)/common/coreboot_config.mk
else
MTK_COMMON_CFG := $(MTK_PLAT)/common/common_config.mk
endif
MTK_PLAT      := plat/mediatek
MTK_PLAT_SOC  := ${MTK_PLAT}/${MTK_SOC}
MTK_PLAT_CFG := $(MTK_PLAT_SOC)/plat_config.mk
MTK_PROJECT_CFG := $(MTK_PLAT)/project/$(PLAT)/project_config.mk
MTK_OPTIONS := $(MTK_PLAT)/build_helpers/options.mk
MTK_COND_EVAL := $(MTK_PLAT)/build_helpers/conditional_eval_options.mk

MTK_BL := bl31

# Include common, platform, board level config
include $(MTK_COMMON_CFG)
include $(MTK_PLAT_CFG)
-include $(MTK_PROJECT_CFG)
include $(MTK_COND_EVAL)
include $(MTK_OPTIONS)
