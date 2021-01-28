#
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
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
			memset.S			\
			setjmp.S)
else
LIBC_SRCS	+=	$(addprefix lib/libc/aarch32/,	\
			memset.S)
endif

INCLUDES	+=	-Iinclude/lib/libc		\
			-Iinclude/lib/libc/$(ARCH)	\
