/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_INIT_H
#define MTK_INIT_H

#include <cdefs.h>
#include <lib/mtk_init/mtk_init_def.h>

#define INIT_CALL_EXPAND_AS_ENUMERATION(_section_enum, _section_name, _level) \
	_section_enum = _level,

#define EXPAND_AS_LINK_SECTION(_section_enum, _section_name, _level) \
	__##_section_enum##_START__ = .; \
	KEEP(*(_section_name##_level));

#define EXPAND_AS_EXTERN(_section_enum, _section_name, _level) \
	extern struct initcall __##_section_enum##_START__[];

#define EXPAND_AS_SYMBOL_ARR(_section_enum, _section_name, _level) \
	__##_section_enum##_START__,

#define DECLARE_MTK_INITCALL(_fn, _level) \
	const struct initcall _mtk_initcall_##_fn \
	__used \
	__aligned(sizeof(void *)) \
	__section(".mtk_plat_initcall_"#_level) \
	= { \
		.name = #_fn, \
		.fn = _fn \
	}

/* initcall helpers  */
#define MTK_EARLY_PLAT_INIT(_fn) DECLARE_MTK_INITCALL(_fn, 0)
#define MTK_ARCH_INIT(_fn) DECLARE_MTK_INITCALL(_fn, 1)
#define MTK_PLAT_SETUP_0_INIT(_fn) DECLARE_MTK_INITCALL(_fn, 2)
#define MTK_PLAT_SETUP_1_INIT(_fn) DECLARE_MTK_INITCALL(_fn, 3)
#define MTK_PLAT_RUNTIME_INIT(_fn) DECLARE_MTK_INITCALL(_fn, 4)
#define MTK_PLAT_BL33_DEFER_INIT(_fn) DECLARE_MTK_INITCALL(_fn, 5)

#ifndef __ASSEMBLER__
struct initcall {
	const char *name;
	int (*fn)(void);
};

enum {
	INIT_CALL_TABLE(INIT_CALL_EXPAND_AS_ENUMERATION)
	MTK_INIT_LVL_MAX
};

void mtk_init_one_level(unsigned int level);
#endif

#endif /* MTK_INIT_H */
