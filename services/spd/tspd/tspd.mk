#
# Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

TSPD_DIR		:=	services/spd/tspd
SPD_INCLUDES		:=	-Iinclude/bl32/tsp

SPD_SOURCES		:=	services/spd/tspd/tspd_common.c		\
				services/spd/tspd/tspd_helpers.S	\
				services/spd/tspd/tspd_main.c		\
				services/spd/tspd/tspd_pm.c

# This dispatcher is paired with a Test Secure Payload source and we intend to
# build the Test Secure Payload along with this dispatcher.
#
# In cases where an associated Secure Payload lies outside this build
# system/source tree, the the dispatcher Makefile can either invoke an external
# build command or assume it pre-built

BL32_ROOT		:=	bl32/tsp

# Include SP's Makefile. The assumption is that the TSP's build system is
# compatible with that of Trusted Firmware, and it'll add and populate necessary
# build targets and variables
include ${BL32_ROOT}/tsp.mk

# Let the top-level Makefile know that we intend to build the SP from source
NEED_BL32		:=	yes

# Flag used to enable routing of non-secure interrupts to EL3 when they are
# generated while the code is executing in S-EL1/0.
TSP_NS_INTR_ASYNC_PREEMPT	:=	0

# If TSPD_ROUTE_IRQ_TO_EL3 build flag is defined, use it to define value for
# TSP_NS_INTR_ASYNC_PREEMPT for backward compatibility.
ifdef TSPD_ROUTE_IRQ_TO_EL3
ifeq (${ERROR_DEPRECATED},1)
$(error "TSPD_ROUTE_IRQ_TO_EL3 is deprecated. Please use the new build flag TSP_NS_INTR_ASYNC_PREEMPT")
endif
$(warning "TSPD_ROUTE_IRQ_TO_EL3 is deprecated. Please use the new build flag TSP_NS_INTR_ASYNC_PREEMPT")
TSP_NS_INTR_ASYNC_PREEMPT	:= ${TSPD_ROUTE_IRQ_TO_EL3}
endif

$(eval $(call assert_boolean,TSP_NS_INTR_ASYNC_PREEMPT))
$(eval $(call add_define,TSP_NS_INTR_ASYNC_PREEMPT))
