#
# Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

GIC_REVISIONS_ := 1 2 3
ifeq ($(filter $(USE_GIC_DRIVER),$(GIC_REVISIONS_)),)
$(error USE_GIC_DRIVER can only be one of $(GIC_REVISIONS_))
endif

ifeq (${USE_GIC_DRIVER},2)
include drivers/arm/gic/v2/gicv2.mk
GIC_SOURCES	:=	${GICV2_SOURCES}			\
			drivers/arm/gic/v2/gicv2_base.c	\
			plat/common/plat_gicv2.c
else ifeq (${USE_GIC_DRIVER},3)
include drivers/arm/gic/v3/gicv3.mk
GIC_SOURCES	:=	${GICV3_SOURCES}			\
			drivers/arm/gic/v3/gicv3_base.c	\
			plat/common/plat_gicv3.c
endif

ifeq ($(ARCH),aarch64)
BL31_SOURCES	+=	${GIC_SOURCES}
else
BL32_SOURCES	+=	${GIC_SOURCES}
endif

$(eval $(call add_defines,\
	USE_GIC_DRIVER \
))
