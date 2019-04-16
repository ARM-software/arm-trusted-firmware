#
# Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


N1SDP_BASE		:=	plat/arm/board/n1sdp

INTERCONNECT_SOURCES	:=	${N1SDP_BASE}/n1sdp_interconnect.c

PLAT_INCLUDES		:=	-I${N1SDP_BASE}/include


N1SDP_CPU_SOURCES	:=	lib/cpus/aarch64/neoverse_n1.S


N1SDP_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c		\
				drivers/arm/gic/v3/gic600.c

PLAT_BL_COMMON_SOURCES	:=	${N1SDP_BASE}/n1sdp_plat.c	        \
				${N1SDP_BASE}/aarch64/n1sdp_helper.S

BL1_SOURCES		+=	drivers/arm/sbsa/sbsa.c

BL31_SOURCES		:=	${N1SDP_CPU_SOURCES}			\
				${INTERCONNECT_SOURCES}			\
				${N1SDP_GIC_SOURCES}			\
				${N1SDP_BASE}/n1sdp_bl31_setup.c	        \
				${N1SDP_BASE}/n1sdp_topology.c	        \
				${N1SDP_BASE}/n1sdp_security.c


# TF-A not required to load the SCP Images
override CSS_LOAD_SCP_IMAGES	  	:=	0

# BL1/BL2 Image not a part of the capsule Image for n1sdp
override NEED_BL1		  	:=	no
override NEED_BL2		  	:=	no
override NEED_BL2U		  	:=	no

#TFA for n1sdp starts from BL31
override RESET_TO_BL31            	:=	1

# 32 bit mode not supported
override CTX_INCLUDE_AARCH32_REGS 	:=	0

override ARM_PLAT_MT              	:=	1

# Select SCMI/SDS drivers instead of SCPI/BOM driver for communicating with the
# SCP during power management operations and for SCP RAM Firmware transfer.
CSS_USE_SCMI_SDS_DRIVER		  	:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY			:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM			:=	0
include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk

