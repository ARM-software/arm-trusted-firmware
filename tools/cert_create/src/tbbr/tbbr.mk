#
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

USE_TBBR_DEFS   := 1
$(eval $(call add_define,USE_TBBR_DEFS))

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

OBJECTS += src/tbbr/tbb_cert.o \
           src/tbbr/tbb_ext.o \
           src/tbbr/tbb_key.o
