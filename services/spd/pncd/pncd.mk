# Copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

PNCD_DIR		:=	services/spd/pncd
SPD_INCLUDES		:=	-Iinclude/bl32/pnc
SPD_INCLUDES		+=	-Iinclude/common/

SPD_SOURCES		:=	services/spd/pncd/pncd_common.c		\
				services/spd/pncd/pncd_helpers.S	\
				services/spd/pncd/pncd_main.c

NEED_BL32		:=	yes

# The following constants need to be defined:
#   - SPD_PNCD_NS_IRQ: IRQ number used to notify NS world when SMC_ACTION_FROM_S is received
#   - SPD_PNCD_S_IRQ: IRQ number used to notify S world when SMC_ACTION_FROM_NS is received
$(eval $(call assert_numerics, SPD_PNCD_NS_IRQ SPD_PNCD_S_IRQ))

$(eval $(call add_defines,\
    $(sort \
        SPD_PNCD_NS_IRQ \
        SPD_PNCD_S_IRQ \
)))
