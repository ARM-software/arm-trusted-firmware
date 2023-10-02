#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${TRANSFER_LIST},1)

ifeq (${ARCH},aarch32)
$(eval $(call add_define,TRANSFER_LIST_AARCH32))
endif

TRANSFER_LIST_SOURCES	+=	$(addprefix lib/transfer_list/,	\
				transfer_list.c)

BL31_SOURCES	+=	$(TRANSFER_LIST_SOURCES)
BL2_SOURCES	+=	$(TRANSFER_LIST_SOURCES)

endif	# TRANSFER_LIST

