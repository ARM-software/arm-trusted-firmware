/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DBGTOP_H
#define DBGTOP_H

int mtk_dbgtop_dram_reserved(int enable);
int mtk_dbgtop_cfg_dvfsrc(int enable);
int mtk_dbgtop_dfd_count_en(int enable);
int mtk_dbgtop_drm_latch_en(int enable);
int mtk_dbgtop_dfd_pause_dvfsrc(int enable);

#endif /* DBGTOP_H */
