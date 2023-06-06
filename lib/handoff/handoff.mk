#
# Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

HANDOFF := 0
$(eval $(call assert_boolean,HANDOFF))
$(eval $(call add_define,HANDOFF))

ifeq (${HANDOFF},1)

HANDOFF_SOURCES	+=	$(addprefix lib/handoff/,	\
			transfer_list.c)

BL31_SOURCES	+=	$(HANDOFF_SOURCES)
BL2_SOURCES	+=	$(HANDOFF_SOURCES)

endif	# HANDOFF

