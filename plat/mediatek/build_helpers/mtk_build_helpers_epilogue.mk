#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Expand include modules
$(eval $(call INCLUDE_MODULES,$(MODULES-y)))

# Make next section align to page size
ifneq ($(MTK_EXTRA_LINKERFILE),)
$(eval $(call MAKE_LINKERFILE_ITER,$(MTK_LINKERFILE_SOURCE),bl31))

# EXTRA_GENERATED_LINKER_SCRIPT is a global variable of derived linker
# script list(from MTK_LINKERFILE_SOURCE) after MAKE_LINKERFILE_ITER
# function call
EXTRA_LINKERFILE += ${EXTRA_GENERATED_LINKER_SCRIPT}

# Expand derived linker script as build target
$(eval $(call MAKE_LD_ITER, $(EXTRA_GENERATED_LINKER_SCRIPT),$(MTK_LINKERFILE_SOURCE),bl31))

# mtk_align.ld MUST BE THE LAST LINKER SCRIPT!
EXTRA_LINKERFILE += ${MTK_PLAT}/include/mtk_align.ld

# bl31.ld should depend on EXTRA_LINKERFILE
$(eval ${BUILD_PLAT}/bl31/bl31.ld: ${EXTRA_LINKERFILE})
EXTRA_LINKERFILE := $(addprefix -T,$(EXTRA_LINKERFILE))
else
EXTRA_LINKERFILE :=
endif
