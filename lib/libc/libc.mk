#
# Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBC_SRCS	:=	$(addprefix lib/libc/,	\
			abort.c				\
			assert.c			\
			exit.c				\
			memchr.c			\
			memcmp.c			\
			memcpy.c			\
			memmove.c			\
			memset.c			\
			printf.c			\
			putchar.c			\
			puts.c				\
			snprintf.c			\
			strchr.c			\
			strcmp.c			\
			strlen.c			\
			strncmp.c			\
			strnlen.c)

INCLUDES	+=	-Iinclude/lib/libc		\
			-Iinclude/lib/libc/$(ARCH)	\
