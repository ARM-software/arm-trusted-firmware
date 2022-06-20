#
# Copyright (c) 2018-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


N1SDP_BASE		:=	plat/arm/board/n1sdp

INTERCONNECT_SOURCES	:=	${N1SDP_BASE}/n1sdp_interconnect.c

PLAT_INCLUDES		:=	-I${N1SDP_BASE}/include


N1SDP_CPU_SOURCES	:=	lib/cpus/aarch64/neoverse_n1.S

# GIC-600 configuration
GICV3_SUPPORT_GIC600		:=	1
GICV3_IMPL_GIC600_MULTICHIP	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

N1SDP_GIC_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c		\

PLAT_BL_COMMON_SOURCES	:=	${N1SDP_BASE}/n1sdp_plat.c	        \
				${N1SDP_BASE}/aarch64/n1sdp_helper.S

BL1_SOURCES		:=	${N1SDP_CPU_SOURCES}                \
				${INTERCONNECT_SOURCES}             \
				${N1SDP_BASE}/n1sdp_err.c           \
				${N1SDP_BASE}/n1sdp_trusted_boot.c  \
				${N1SDP_BASE}/n1sdp_bl1_setup.c     \
				drivers/arm/sbsa/sbsa.c

BL2_SOURCES		:=	${N1SDP_BASE}/n1sdp_security.c      \
				${N1SDP_BASE}/n1sdp_err.c           \
				${N1SDP_BASE}/n1sdp_trusted_boot.c  \
				lib/utils/mem_region.c              \
				${N1SDP_BASE}/n1sdp_bl2_setup.c     \
				${N1SDP_BASE}/n1sdp_image_load.c     \
				drivers/arm/css/sds/sds.c

BL31_SOURCES		:=	${N1SDP_CPU_SOURCES}			\
				${INTERCONNECT_SOURCES}			\
				${N1SDP_GIC_SOURCES}			\
				${N1SDP_BASE}/n1sdp_bl31_setup.c	\
				${N1SDP_BASE}/n1sdp_topology.c	        \
				${N1SDP_BASE}/n1sdp_security.c		\
				drivers/arm/css/sds/sds.c

FDT_SOURCES		+=	fdts/${PLAT}-single-chip.dts	\
				fdts/${PLAT}-multi-chip.dts	\
				${N1SDP_BASE}/fdts/n1sdp_fw_config.dts	\
				${N1SDP_BASE}/fdts/n1sdp_tb_fw_config.dts	\
				${N1SDP_BASE}/fdts/n1sdp_nt_fw_config.dts

FW_CONFIG		:=	${BUILD_PLAT}/fdts/n1sdp_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/n1sdp_tb_fw_config.dtb
NT_FW_CONFIG		:=	${BUILD_PLAT}/fdts/n1sdp_nt_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))
# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${NT_FW_CONFIG},--nt-fw-config,${NT_FW_CONFIG}))

N1SDP_SPMC_MANIFEST_DTS	:=	${N1SDP_BASE}/fdts/${PLAT}_optee_spmc_manifest.dts
FDT_SOURCES		+=	${N1SDP_SPMC_MANIFEST_DTS}
N1SDP_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_optee_spmc_manifest.dtb

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${N1SDP_TOS_FW_CONFIG},--tos-fw-config,${N1SDP_TOS_FW_CONFIG}))

# Setting to 0 as no NVCTR in N1SDP
N1SDP_FW_NVCTR_VAL	:=	0
TFW_NVCTR_VAL		:=	${N1SDP_FW_NVCTR_VAL}
NTFW_NVCTR_VAL		:=	${N1SDP_FW_NVCTR_VAL}

# Add N1SDP_FW_NVCTR_VAL
$(eval $(call add_define,N1SDP_FW_NVCTR_VAL))

# TF-A not required to load the SCP Images
override CSS_LOAD_SCP_IMAGES	  	:=	0

override NEED_BL2U		  	:=	no

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

# Enable the flag since N1SDP has a system level cache
NEOVERSE_Nx_EXTERNAL_LLC		:=	1
include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk
