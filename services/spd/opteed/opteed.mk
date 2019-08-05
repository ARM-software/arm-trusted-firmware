#
# Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
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
