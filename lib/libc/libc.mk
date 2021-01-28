#
# Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBC_SRCS	:=	$(addprefix lib/libc/,		\
			abort.c				\
			assert.c			\
			exit.c				\
			memchr.c			\
			memcmp.c			\
			memcpy.c			\
			memmove.c			\
			memrchr.c			\
			memset.c			\
			printf.c			\
			putchar.c			\
			puts.c				\
			snprintf.c			\
			strchr.c			\
			strcmp.c			\
			strlcat.c			\
			strlcpy.c			\
			strlen.c			\
			strncmp.c			\
			strnlen.c			\
			strrchr.c			\
			strtok.c			\
			strtoul.c			\
			strtoll.c			\
			strtoull.c			\
			strtol.c)

ifeq (${ARCH},aarch64)
LIBC_SRCS	+=	$(addprefix lib/libc/aarch64/,	\
			setjmp.S)
endif

INCLUDES	+=	-Iinclude/lib/libc		\
			-Iinclude/lib/libc/$(ARCH)	\
