/*
 * Copyright (c) 2016-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

.section .bl2_image, "a"
.incbin BL2_BIN_PATH

.section .dtb_image, "a"
.incbin DTB_BIN_PATH
