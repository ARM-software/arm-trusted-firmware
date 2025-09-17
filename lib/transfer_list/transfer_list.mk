#
# Copyright (c) 2023-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${TRANSFER_LIST},1)

# Default path if not set externally
LIBTL_PATH	?=	contrib/libtl

# Common include paths (always needed)
INCLUDES	+=	-I$(LIBTL_PATH)/include \
			-I$(LIBTL_PATH)/include/arm

LIBTL_SRC_PATH	:=	$(LIBTL_PATH)/src

LIBTL_SRCS	:=	$(addprefix $(LIBTL_SRC_PATH)/, \
				arm/ep_info.c \
				generic/logging.c \
				generic/transfer_list.c)

ifeq ($(MEASURED_BOOT), 1)
LIBTL_SRCS	+=	$(LIBTL_SRC_PATH)/generic/tpm_event_log.c
endif

$(eval $(call MAKE_LIB,tl))

endif	# TRANSFER_LIST
