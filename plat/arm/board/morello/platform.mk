#
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MORELLO_BASE		:=	plat/arm/board/morello

INTERCONNECT_SOURCES	:=	${MORELLO_BASE}/morello_interconnect.c

PLAT_INCLUDES		:=	-I${MORELLO_BASE}/include

MORELLO_CPU_SOURCES	:=	lib/cpus/aarch64/rainier.S

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

MORELLO_GIC_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c		\

PLAT_BL_COMMON_SOURCES	:=	${MORELLO_BASE}/morello_plat.c		\
				${MORELLO_BASE}/aarch64/morello_helper.S

BL31_SOURCES		:=	${MORELLO_CPU_SOURCES}			\
				${INTERCONNECT_SOURCES}			\
				${MORELLO_GIC_SOURCES}			\
				${MORELLO_BASE}/morello_bl31_setup.c	\
				${MORELLO_BASE}/morello_topology.c	\
				${MORELLO_BASE}/morello_security.c	\
				drivers/arm/css/sds/sds.c

FDT_SOURCES		+=	fdts/morello-fvp.dts

# TF-A not required to load the SCP Images
override CSS_LOAD_SCP_IMAGES		:=	0

# BL1/BL2 Image not a part of the capsule Image for morello
override NEED_BL1			:=	no
override NEED_BL2			:=	no
override NEED_BL2U			:=	no

#TF-A for morello starts from BL31
override RESET_TO_BL31			:=	1

# 32 bit mode not supported
override CTX_INCLUDE_AARCH32_REGS	:=	0

override ARM_PLAT_MT			:=	1

# Select SCMI/SDS drivers instead of SCPI/BOM driver for communicating with the
# SCP during power management operations and for SCP RAM Firmware transfer.
CSS_USE_SCMI_SDS_DRIVER			:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY			:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM			:=	0

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk
