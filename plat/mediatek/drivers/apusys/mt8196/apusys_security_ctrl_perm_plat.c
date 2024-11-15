/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "apusys_security_ctrl_perm.h"
#include "apusys_security_ctrl_perm_plat.h"

#define SEC_CTRL_APU_SEC_CON_BASE	(0x190F5000)
#define SEC_CTRL_RV_DOMAIN_OFS		(0x60)
#define SEC_CTRL_RV_NS_OFS		(0x64)
#define SEC_CTRL_RV_DOMAIN_SHF		(4)
#define SEC_CTRL_RV_NS_SHF		(1)

#define SEC_LEVEL_NORMAL_DOMAIN		(7)
#define SEC_LEVEL_NORMAL_NS		(1)
#define SEC_LEVEL_SAPU_DOMAIN		(5)
#define SEC_LEVEL_SAPU_NS		(1)
#define SEC_LEVEL_AOV_DOMAIN		(14)
#define SEC_LEVEL_AOV_NS		(1)
#define SEC_LEVEL_UP_SECURE_DOMAIN	(5)
#define SEC_LEVEL_UP_SECURE_NS		(0)
#define SEC_LEVEL_MVPU_SECURE_DOMAIN	(7)
#define SEC_LEVEL_MVPU_SECURE_NS	(0)
#define SEC_LEVEL_MDLA_SECURE_DOMAIN	(14)
#define SEC_LEVEL_MDLA_SECURE_NS	(0)
#define DOMAIN(SEC_LVL)	SEC_LEVEL_##SEC_LVL##_DOMAIN
#define NS(SEC_LVL)	SEC_LEVEL_##SEC_LVL##_NS

int sec_get_dns(enum apusys_dev_type dev_type, enum apusys_sec_level sec_level,
		uint8_t *domain, uint8_t *ns)
{
	if ((dev_type < 0) || (dev_type >= APUSYS_DEVICE_NUM)) {
		ERROR("invalid dev type %d\n", dev_type);
		return -EINVAL;
	}

	if ((sec_level < 0) || (sec_level >= SEC_LEVEL_NUM)) {
		ERROR("invalid sec_level %d\n", sec_level);
		return -EINVAL;
	}

	switch (sec_level) {
	case SEC_LEVEL_NORMAL:
		*domain = DOMAIN(NORMAL);
		*ns = NS(NORMAL);
		break;
	case SEC_LEVEL_SECURE:
		switch (dev_type) {
		case APUSYS_DEVICE_MVPU:
			*domain = DOMAIN(MVPU_SECURE);
			*ns = NS(MVPU_SECURE);
			break;
		case APUSYS_DEVICE_MDLA:
			*domain = DOMAIN(MDLA_SECURE);
			*ns = NS(MDLA_SECURE);
			break;
		case APUSYS_DEVICE_UP:
			*domain = DOMAIN(UP_SECURE);
			*ns = NS(UP_SECURE);
			break;
		default:
			ERROR("invalid dev type %d\n", dev_type);
			return -EINVAL;
		};
		break;
	case SEC_LEVEL_SAPU:
		*domain = DOMAIN(SAPU);
		*ns = NS(SAPU);
		break;
	case SEC_LEVEL_AOV:
		*domain = DOMAIN(AOV);
		*ns = NS(AOV);
		break;
	default:
		ERROR("invalid sec_level %d\n", sec_level);
		return -EINVAL;
	};

	return 0;
}

int sec_set_rv_dns(void)
{
	uint8_t normal_domain;
	uint8_t normal_ns;
	uint8_t sec_domain;
	uint8_t sec_ns;
	int ret;

	ret = sec_get_dns(APUSYS_DEVICE_UP, SEC_LEVEL_SECURE, &sec_domain, &sec_ns);
	if (ret) {
		ERROR("%s failed.\n", __func__);
		return ret;
	}

	ret = sec_get_dns(APUSYS_DEVICE_UP, SEC_LEVEL_NORMAL, &normal_domain, &normal_ns);
	if (ret) {
		ERROR("%s failed.\n", __func__);
		return ret;
	}

	mmio_write_32(SEC_CTRL_APU_SEC_CON_BASE + SEC_CTRL_RV_DOMAIN_OFS,
		      (sec_domain << SEC_CTRL_RV_DOMAIN_SHF) | normal_domain);
	mmio_write_32(SEC_CTRL_APU_SEC_CON_BASE + SEC_CTRL_RV_NS_OFS,
		      (sec_ns << SEC_CTRL_RV_NS_SHF) | normal_ns);

	return 0;
}
