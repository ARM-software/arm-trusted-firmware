/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef BL32_BIN_PATH
.section .bl32_image
.incbin BL32_BIN_PATH
#endif

.section .bl2_image
.incbin BL2_BIN_PATH

.section .dtb_image
.incbin DTB_BIN_PATH
