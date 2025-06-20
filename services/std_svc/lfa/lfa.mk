#
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LFA_SOURCES	+=	$(addprefix services/std_svc/lfa/, \
			  lfa_main.c \
			  bl31_lfa.c \
			  lfa_holding_pen.c)

ifeq (${ENABLE_RME}, 1)
LFA_SOURCES	+=	services/std_svc/rmmd/rmmd_rmm_lfa.c
endif
