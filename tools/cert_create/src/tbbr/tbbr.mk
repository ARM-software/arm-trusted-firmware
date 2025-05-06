#
# Copyright (c) 2020-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

USE_TBBR_DEFS   := 1
CRTTOOL_DEFINES	+= USE_TBBR_DEFS=${USE_TBBR_DEFS}

ifeq (${USE_TBBR_DEFS},1)
# In this case, cert_tool is platform-independent
PLAT_MSG		:=	TBBR Generic
PLAT_INCLUDE		:=	../../include/tools_share
else
PLAT_MSG		:=	${PLAT}

TF_PLATFORM_ROOT	:=	../../plat/
include ${MAKE_HELPERS_DIRECTORY}plat_helpers.mk

PLAT_INCLUDE		:=	$(wildcard ${PLAT_DIR}include)

ifeq ($(PLAT_INCLUDE),)
  $(error "Error: Invalid platform '${PLAT}' has no include directory.")
endif
endif

CRTTOOL_SOURCES +=	src/tbbr/tbb_cert.c \
			src/tbbr/tbb_ext.c \
			src/tbbr/tbb_key.c
