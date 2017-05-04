#
# Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

INCLUDES		+=	-Iinclude/bl32/tsp

BL32_SOURCES		+=	bl32/tsp/tsp_main.c			\
				bl32/tsp/aarch64/tsp_entrypoint.S	\
				bl32/tsp/aarch64/tsp_exceptions.S	\
				bl32/tsp/aarch64/tsp_request.S		\
				bl32/tsp/tsp_interrupt.c		\
				bl32/tsp/tsp_timer.c			\
				common/aarch64/early_exceptions.S	\
				lib/locks/exclusive/aarch64/spinlock.S

BL32_LINKERFILE		:=	bl32/tsp/tsp.ld.S

# This flag determines if the TSPD initializes BL32 in tspd_init() (synchronous
# method) or configures BL31 to pass control to BL32 instead of BL33
# (asynchronous method).
TSP_INIT_ASYNC         :=      0

$(eval $(call assert_boolean,TSP_INIT_ASYNC))
$(eval $(call add_define,TSP_INIT_ASYNC))

# Include the platform-specific TSP Makefile
# If no platform-specific TSP Makefile exists, it means TSP is not supported
# on this platform.
TSP_PLAT_MAKEFILE := $(wildcard ${PLAT_DIR}/tsp/tsp-${PLAT}.mk)
ifeq (,${TSP_PLAT_MAKEFILE})
  $(error TSP is not supported on platform ${PLAT})
else
  include ${TSP_PLAT_MAKEFILE}
endif
