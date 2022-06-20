/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_INIT_DEF_H
#define MTK_INIT_DEF_H

/*
 * Define init call sections here. _func is for 2nd level expansion, init
 * section enum, and init section name.
 */
#define INIT_CALL_TABLE(_func) \
	_func(MTK_INIT_LVL_EARLY_PLAT, .mtk_plat_initcall_, 0) \
	_func(MTK_INIT_LVL_ARCH, .mtk_plat_initcall_, 1) \
	_func(MTK_INIT_LVL_PLAT_SETUP_0, .mtk_plat_initcall_, 2) \
	_func(MTK_INIT_LVL_PLAT_SETUP_1, .mtk_plat_initcall_, 3) \
	_func(MTK_INIT_LVL_PLAT_RUNTIME, .mtk_plat_initcall_, 4) \
	_func(MTK_INIT_LVL_BL33_DEFER, .mtk_plat_initcall_, 5)

#endif /* MTK_INIT_DEF_H */
