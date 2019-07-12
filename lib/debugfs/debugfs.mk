#
# Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

DEBUGFS_SRCS	:=	$(addprefix lib/debugfs/,	\
			dev.c				\
			devc.c				\
			devroot.c			\
			devfip.c)

DEBUGFS_SRCS    += lib/debugfs/debugfs_smc.c
