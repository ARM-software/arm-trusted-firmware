/*
 * Copyright (c) 2023-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

.section .bl2_image, "a"
.incbin BL2_BIN_PATH

.section .dtb_image, "a"
.incbin DTB_BIN_PATH
