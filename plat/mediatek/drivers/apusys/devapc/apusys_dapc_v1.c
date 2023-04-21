/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>
#include <lib/utils_def.h>

/* Vendor header */
#include "apusys.h"
#include "apusys_dapc_v1.h"
#include <platform_def.h>

enum apusys_apc_err_status set_apusys_dapc_v1(const struct apc_dom_16 *dapc,
					      uint32_t size, dapc_cfg_func cfg)
{
	enum apusys_apc_err_status ret = APUSYS_APC_OK;
	uint32_t i;

	if ((dapc == NULL) || (cfg == NULL)) {
		return APUSYS_APC_ERR_GENERIC;
	}

	for (i = 0; i < size; i++) {
		ret += cfg(i, DOMAIN_0,	 dapc[i].d0_permission);
		ret += cfg(i, DOMAIN_1,	 dapc[i].d1_permission);
		ret += cfg(i, DOMAIN_2,	 dapc[i].d2_permission);
		ret += cfg(i, DOMAIN_3,	 dapc[i].d3_permission);
		ret += cfg(i, DOMAIN_4,	 dapc[i].d4_permission);
		ret += cfg(i, DOMAIN_5,	 dapc[i].d5_permission);
		ret += cfg(i, DOMAIN_6,	 dapc[i].d6_permission);
		ret += cfg(i, DOMAIN_7,	 dapc[i].d7_permission);
		ret += cfg(i, DOMAIN_8,	 dapc[i].d8_permission);
		ret += cfg(i, DOMAIN_9,	 dapc[i].d9_permission);
		ret += cfg(i, DOMAIN_10, dapc[i].d10_permission);
		ret += cfg(i, DOMAIN_11, dapc[i].d11_permission);
		ret += cfg(i, DOMAIN_12, dapc[i].d12_permission);
		ret += cfg(i, DOMAIN_13, dapc[i].d13_permission);
		ret += cfg(i, DOMAIN_14, dapc[i].d14_permission);
		ret += cfg(i, DOMAIN_15, dapc[i].d15_permission);
	}

	if (ret != APUSYS_APC_OK) {
		ret = APUSYS_APC_ERR_GENERIC;
	}

	return ret;
}

void dump_apusys_dapc_v1(const char *name, uintptr_t base, uint32_t reg_num, uint32_t dom_num)
{
	uint32_t d, i;

	if ((name == NULL) || (base == 0)) {
		return;
	}

	for (d = 0; d < dom_num; d++) {
		for (i = 0; i <= reg_num; i++) {
			INFO(MODULE_TAG "[%s] D%d_APC_%d: 0x%x\n", name, d, i,
			     mmio_read_32(base + d * DEVAPC_DOM_SIZE + i * DEVAPC_REG_SIZE));
		}
	}

	INFO(MODULE_TAG "[%s] APC_CON: 0x%x\n", name, mmio_read_32(APUSYS_DAPC_CON(base)));
}
