#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Expand include modules
$(eval $(call INCLUDE_MODULES,$(MODULES-y)))

# Make next section align to page size
ifneq ($(MTK_EXTRA_LINKERFILE),)
        # mtk_align.ld MUST BE THE LAST LINKER SCRIPT!
        BL31_LINKER_SCRIPT_SOURCES += $(MTK_LINKERFILE_SOURCE)
        BL31_LINKER_SCRIPT_SOURCES += ${MTK_PLAT}/include/mtk_align.ld
endif
