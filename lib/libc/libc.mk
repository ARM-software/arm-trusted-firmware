#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBC_SRCS	:=	$(addprefix lib/libc/,	\
			abort.c				\
			assert.c			\
			exit.c				\
			mem.c				\
			printf.c			\
			putchar.c			\
			puts.c				\
			sscanf.c			\
			strchr.c			\
			strcmp.c			\
			strlen.c			\
			strncmp.c			\
			strnlen.c			\
			subr_prf.c			\
			timingsafe_bcmp.c)

INCLUDES	+=	-Iinclude/lib/libc		\
			-Iinclude/lib/libc/sys
