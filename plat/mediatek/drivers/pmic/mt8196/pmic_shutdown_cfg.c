/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>

#include <drivers/pmic/pmic_shutdown_cfg.h>
#include <drivers/spmi/spmi_common.h>
#include <drivers/spmi_api.h>
#include <lib/mtk_init/mtk_init.h>

#define MASTER_ID				SPMI_MASTER_1

#ifndef MT8678_PMIC_SUPPORT
/* MT6316 will automatically disable wdt in poffs */
#define MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR	0x408
#define MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_MASK	0x1
#define MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT	1
#else
/* MT6319 will automatically disable wdt in poffs */
#define MT6319_TOP_RST_MISC_CLR			0x128
#define MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR	0x138
#define MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_MASK	0x1
#define MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT	2
#endif

#define MT6373_TOP_RST_MISC1_CLR		0x13B
#define MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR	0x408
#define MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_MASK	0x1
#define MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT	1

#define MT6685_TOP_RST_MISC_CLR			0x129
#define MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR	0x408
#define MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_MASK	0x1
#define MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT	1

struct spmi_device *sdev_arr[SPMI_MAX_SLAVE_ID];

struct cfg_t {
	uint8_t slvid;
	uint32_t addr;
	uint32_t shutdown_src_addr;
	uint32_t shutdown_src_mask;
	uint32_t shutdown_src_shift;
	uint8_t val;
};

#ifndef MT8678_PMIC_SUPPORT
static const struct cfg_t cfg_arr[] = {
	{
		.slvid = SPMI_SLAVE_6,
		.addr = 0,
		.shutdown_src_addr = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_7,
		.addr = 0,
		.shutdown_src_addr = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_8,
		.addr = 0,
		.shutdown_src_addr = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_15,
		.addr = 0,
		.shutdown_src_addr = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6316_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_5,
		.addr = MT6373_TOP_RST_MISC1_CLR,
		.shutdown_src_addr = MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_9,
		.addr = MT6685_TOP_RST_MISC_CLR,
		.shutdown_src_addr = MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}
};
#else /* MT8678_PMIC_SUPPORT */
static const struct cfg_t cfg_arr[] = {
	{
		.slvid = SPMI_SLAVE_6,
		.addr = MT6319_TOP_RST_MISC_CLR,
		.shutdown_src_addr = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_7,
		.addr = MT6319_TOP_RST_MISC_CLR,
		.shutdown_src_addr = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_8,
		.addr = MT6319_TOP_RST_MISC_CLR,
		.shutdown_src_addr = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_15,
		.addr = MT6319_TOP_RST_MISC_CLR,
		.shutdown_src_addr = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6319_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_5,
		.addr = MT6373_TOP_RST_MISC1_CLR,
		.shutdown_src_addr = MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6373_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}, {
		.slvid = SPMI_SLAVE_9,
		.addr = MT6685_TOP_RST_MISC_CLR,
		.shutdown_src_addr = MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_ADDR,
		.shutdown_src_mask = MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_MASK,
		.shutdown_src_shift = MT6685_PMIC_RG_SHUTDOWN_SRC_SEL_SHIFT,
		.val = 0x1
	}
};
#endif /* MT8678_PMIC_SUPPORT */

#define MT6316_TOP_ANA_KEY			0x3AA
#define MT6316_PMIC_RG_VI075_SINK_CUR_ADDR	0x994
#define MT6316_PMIC_RG_VI075_SINK_CUR_MASK	0xF
#define MT6316_PMIC_RG_VI075_SINK_CUR_SHIFT	4
#define MT6316_PMIC_RG_PSEQ_ELR_RSV2_ADDR	0xA2C
#define MT6316_PMIC_RG_PSEQ_ELR_RSV2_MASK	0x7
#define MT6316_PMIC_RG_PSEQ_ELR_RSV2_SHIFT	5
#define PSEQ_ELR_RSV2_VAL_MASK_1		0x3
#define PSEQ_ELR_RSV2_VAL_MASK_2		0x1
#define VI075_SINK_CUR_SOURCE_1			0x5
#define VI075_SINK_CUR_SOURCE_2			0
#define VI075_SINK_CUR_SOURCE_3			0xB
#define ARRAY_LENGTH_MAX			2

#ifndef MT8678_PMIC_SUPPORT
static void mt6316_key_lock_check(struct spmi_device *mt6316_dev, uint16_t key)
{
	int i, ret;
	uint16_t rdata;
	uint8_t work_val[ARRAY_LENGTH_MAX];
	uint8_t wdata[ARRAY_LENGTH_MAX];

	for (i = 0; i < 2; i++) {
		ret = spmi_ext_register_readl(mt6316_dev, key, &work_val[0], 2);
		if (ret < 0) {
			INFO("[%s]: read fail, addr = 0x%x, ret = %d\n"
			      , __func__, key, ret);
			i = 0;
			continue;
		}
		rdata = work_val[0] | (work_val[1] << 8);

		if (rdata != 0) {
			INFO("[%s] lock fail, addr = 0x%x, rdata = 0x%x.\n"
			      , __func__, key, rdata);
			wdata[0] = 0;
			wdata[1] = 0;
			spmi_ext_register_writel(mt6316_dev, key, &wdata[0], 2);
			i = 0;
		}
	}
}

static void wk_vio075_sink_cur(struct spmi_device *mt6316_dev, unsigned char en_seq_off)
{
	uint8_t rval, wval;
	int ret;
	uint8_t buf[ARRAY_LENGTH_MAX];

	ret = spmi_ext_register_readl(mt6316_dev, MT6316_PMIC_RG_PSEQ_ELR_RSV2_ADDR, &rval, 1);
	if (ret < 0)
		return;
	rval = (rval >> MT6316_PMIC_RG_PSEQ_ELR_RSV2_SHIFT) & MT6316_PMIC_RG_PSEQ_ELR_RSV2_MASK;

	if (!(rval & PSEQ_ELR_RSV2_VAL_MASK_1)) {
		wval = VI075_SINK_CUR_SOURCE_1;
	} else if (rval & PSEQ_ELR_RSV2_VAL_MASK_2) {
		if (en_seq_off)
			wval = VI075_SINK_CUR_SOURCE_2;
		else
			wval = VI075_SINK_CUR_SOURCE_3;
	} else {
		wval = VI075_SINK_CUR_SOURCE_2;
	}

	buf[0] = 0xDC;
	buf[1] = 0xF1;
	spmi_ext_register_writel(mt6316_dev,
				 MT6316_TOP_ANA_KEY,
				 &buf[0], 2); /* unlock TOP_ANA key */
	spmi_ext_register_writel_field(mt6316_dev,
				       MT6316_PMIC_RG_VI075_SINK_CUR_ADDR, wval,
				       MT6316_PMIC_RG_VI075_SINK_CUR_MASK,
				       MT6316_PMIC_RG_VI075_SINK_CUR_SHIFT);
	buf[0] = 0;
	buf[1] = 0;
	spmi_ext_register_writel(mt6316_dev,
				 MT6316_TOP_ANA_KEY,
				 &buf[0], 2); /* lock TOP_ANA key */
	mt6316_key_lock_check(mt6316_dev, MT6316_TOP_ANA_KEY);
}
#endif

static int pmic_shutdown_cfg_init(void)
{
	uint8_t i, slvid;

	for (i = 0; i < ARRAY_SIZE(cfg_arr); i++) {
		slvid = cfg_arr[i].slvid;
		if (sdev_arr[slvid] != NULL)
			continue;
		sdev_arr[slvid] = get_spmi_device(MASTER_ID, slvid);
		if (!sdev_arr[slvid])
			return -ENODEV;
	}
	return 0;
}
MTK_PLAT_SETUP_0_INIT(pmic_shutdown_cfg_init);

int pmic_shutdown_cfg(void)
{
	int ret;
	uint8_t i, slvid;
	uint32_t addr;
	uint8_t val;

	for (i = 0; i < ARRAY_SIZE(cfg_arr); i++) {
		slvid = cfg_arr[i].slvid;
		if (!sdev_arr[slvid])
			return -ENODEV;
		/* mt6316 vio075 sink current adjustment */
		if ((slvid >= SPMI_SLAVE_6 && slvid <= SPMI_SLAVE_8) || slvid == SPMI_SLAVE_15)
			wk_vio075_sink_cur(sdev_arr[slvid], 1);
		addr = cfg_arr[i].addr;
		val = cfg_arr[i].val;
		/* Disable WDTRSTB_EN */
		if (addr) {
			ret = spmi_ext_register_writel(sdev_arr[slvid], addr, &val, 1);
			if (ret < 0)
				return ret;
		}

		/* set RG_SHUTDOWN_SRC_SEL to 1, shutdown PMIC by SPMI command */
		spmi_ext_register_writel_field(sdev_arr[slvid],
					       cfg_arr[i].shutdown_src_addr, 1,
					       cfg_arr[i].shutdown_src_mask,
					       cfg_arr[i].shutdown_src_shift);
	}
	return 1; /* 1: use spmi_command_shutdown API */
}

/* shutdown PMIC by SPMI command */
int spmi_shutdown(void)
{
	struct spmi_device *mt6363_sdev = get_spmi_device(SPMI_MASTER_1, SPMI_SLAVE_4);

	if (!mt6363_sdev)
		return -ENODEV;

	/* set RG_SHUTDOWN_SRC_SEL to 1 */
	spmi_ext_register_writel_field(mt6363_sdev, 0x408, 1, 0x1, 1);
	spmi_command_shutdown(SPMI_MASTER_P_1, mt6363_sdev, 0x800);
	spmi_command_shutdown(SPMI_MASTER_1, mt6363_sdev, 0x800);

	return 0;
}
