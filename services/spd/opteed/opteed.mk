#
# Copyright (c) 2013-2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

OPTEED_DIR		:=	services/spd/opteed
SPD_INCLUDES		:=

SPD_SOURCES		:=	services/spd/opteed/opteed_common.c	\
				services/spd/opteed/opteed_helpers.S	\
				services/spd/opteed/opteed_main.c	\
				services/spd/opteed/opteed_pm.c

NEED_BL32		:=	yes

# required so that optee code can control access to the timer registers
NS_TIMER_SWITCH		:=	1

# WARNING: This enables loading of OP-TEE via an SMC, which can be potentially
# insecure. This removes the boundary between the startup of the secure and
# non-secure worlds until the point where this SMC is invoked. Only use this
# setting if you can ensure that the non-secure OS can remain trusted up until
# the point where this SMC is invoked.
OPTEE_ALLOW_SMC_LOAD		:=	0
ifeq ($(OPTEE_ALLOW_SMC_LOAD),1)
ifeq ($(PLAT_XLAT_TABLES_DYNAMIC),0)
$(error When OPTEE_ALLOW_SMC_LOAD=1, PLAT_XLAT_TABLES_DYNAMIC must also be 1)
endif
$(warning "OPTEE_ALLOW_SMC_LOAD is enabled which may result in an insecure \
	platform")
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))
$(eval $(call add_define,OPTEE_ALLOW_SMC_LOAD))
include lib/libfdt/libfdt.mk
endif

CROS_WIDEVINE_SMC		:=	0
ifeq ($(CROS_WIDEVINE_SMC),1)
ifeq ($(OPTEE_ALLOW_SMC_LOAD),0)
$(error When CROS_WIDEVINE_SMC=1, OPTEE_ALLOW_SMC_LOAD must also be 1)
endif
$(eval $(call add_define,CROS_WIDEVINE_SMC))
endif
