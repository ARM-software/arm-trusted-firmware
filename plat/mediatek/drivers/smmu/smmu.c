/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include <drivers/apusys_rv_public.h>
#include <drivers/mminfra_public.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_sip_svc.h>

#define TAG		"[MTK_SMMU]"

#ifdef SMMU_DBG
#define SMMUDBG(fmt, args...)	INFO(TAG fmt, ##args)
#else
#define SMMUDBG(fmt, args...)	VERBOSE(TAG fmt, ##args)
#endif

#define SMMU_SUCCESS			0
#define SMMU_ID_ERR			1
#define SMMU_CMD_ERR			2

#define F_MSK_SHIFT(val, h, l)		(((val) & GENMASK(h, l)) >> (l))

#define SMMU_SMC_ID_H			(10)
#define SMMU_SMC_ID_L			(8)
#define SMMU_SMC_CMD_H			(7)
#define SMMU_SMC_CMD_L			(0)

/* SMMU CMD Definition From Rich OS */
enum smc_cmd {
	SMMU_SECURE_PM_GET,
	SMMU_SECURE_PM_PUT,
	SMMU_CMD_NUM
};

enum smmu_id {
	MTK_SMMU_ID_MM,
	MTK_SMMU_ID_APU,
	MTK_SMMU_ID_SOC,
	MTK_SMMU_ID_GPU,
	MTK_SMMU_ID_NUM,
};

enum cmd_source {
	SMMU_CMD_SOURCE_KERNEL = 0, /* Command comes from kernel */
	SMMU_CMD_SOURCE_TFA,
	SMMU_CMD_SOURCE_HYP,	    /* Command comes from hypervisor */
	SMMU_CMD_SOURCE_NUM
};

struct hw_sema_t {
	enum smmu_id id;
	uint32_t vote[SMMU_CMD_SOURCE_NUM]; /* SW vote count */
	spinlock_t lock;
	bool active;
};

static struct hw_sema_t *hw_semas;

static inline uint32_t vote_count_inc(struct hw_sema_t *sema, enum cmd_source id)
{
	if (sema->vote[id] < UINT32_MAX) {
		sema->vote[id]++;
		return sema->vote[id];
	}

	ERROR(TAG "%s:id:%u:source_id:%u overflow\n", __func__, sema->id, id);
	return 0;
}

static inline uint32_t vote_count_dec(struct hw_sema_t *sema, enum cmd_source id)
{
	if (sema->vote[id] > 0) {
		sema->vote[id]--;
		return sema->vote[id];
	}

	ERROR(TAG "%s:id:%u:source_id:%u underflow\n", __func__, sema->id, id);
	return 0;
}

static inline uint32_t vote_count(struct hw_sema_t *sema)
{
	uint32_t i, count = 0;

	for (i = 0; i < SMMU_CMD_SOURCE_NUM; i++)
		count += sema->vote[i];

	return count;
}

static struct hw_sema_t *mtk_smmu_get_hw_sema_cfg(enum smmu_id id)
{
	if (hw_semas == NULL) {
		ERROR(TAG "%s failed, hw_sema config not ready\n", __func__);
		return NULL;
	}

	if (id >= MTK_SMMU_ID_NUM) {
		ERROR(TAG "%s id:%u not support\n", __func__, id);
		return NULL;
	}
	return &hw_semas[id];
}

static int mm_pm_get_if_in_use(struct hw_sema_t *sema, enum cmd_source id)
{
	uint32_t count;
	int ret;

	ret = mminfra_get_if_in_use();
	if (ret != MMINFRA_RET_POWER_ON) {
		count = vote_count(sema);
		VERBOSE(TAG "%s:id:%u:source_id:%u:vote:%u:vote_count:%u ret:%d\n",
			__func__, sema->id, id, sema->vote[id], count, ret);
		return SMMU_CMD_ERR;
	}
	return SMMU_SUCCESS;
}

static int mm_pm_put(struct hw_sema_t *sema, enum cmd_source id)
{
	uint32_t count;
	int ret;

	ret = mminfra_put();
	if (ret < 0) {
		count = vote_count(sema);
		VERBOSE(TAG "%s:id:%u:source_id:%u:vote:%u:vote_count:%u ret:%d\n",
			__func__, sema->id, id, sema->vote[id], count, ret);
		return SMMU_CMD_ERR;
	}
	return SMMU_SUCCESS;
}

static int mtk_smmu_pm_get(enum smmu_id id, enum cmd_source source_id)
{
	struct hw_sema_t *hw_sema = mtk_smmu_get_hw_sema_cfg(id);
	uint32_t count;
	int ret = SMMU_SUCCESS;

	if (!hw_sema || !hw_sema->active)
		return 0; /* hw_sema not ready or support, bypass */

	spin_lock(&hw_sema->lock);
	count = vote_count(hw_sema);

	SMMUDBG("%s:id:%u:source_id:%u:vote:%u:vote_count:%u start\n",
		__func__, id, source_id, hw_sema->vote[source_id], count);

	if (count > 0) {
		/* hw_sem was already got */
		vote_count_inc(hw_sema, source_id);
		goto out;
	}

	if (id == MTK_SMMU_ID_APU) {
		ret = apusys_rv_iommu_hw_sem_trylock();
	} else if (id == MTK_SMMU_ID_MM) {
		ret = mm_pm_get_if_in_use(hw_sema, source_id);
	}

	if (ret == SMMU_SUCCESS)
		vote_count_inc(hw_sema, source_id);

out:
	count = vote_count(hw_sema);
	SMMUDBG("%s:id:%u:source_id:%u:vote:%u:vote_count:%u end ret:%d\n",
		__func__, id, source_id, hw_sema->vote[source_id], count, ret);

	spin_unlock(&hw_sema->lock);
	return ret;
}

static int mtk_smmu_pm_put(enum smmu_id id, enum cmd_source source_id)
{
	struct hw_sema_t *hw_sema = mtk_smmu_get_hw_sema_cfg(id);
	uint32_t count;
	int ret = SMMU_SUCCESS;

	if (!hw_sema || !hw_sema->active)
		return 0; /* hw_sema not ready or support, bypass */

	spin_lock(&hw_sema->lock);
	count = vote_count(hw_sema);

	SMMUDBG("%s:id:%u:source_id:%u:vote:%u:vote_count:%u start\n",
		__func__, id, source_id, hw_sema->vote[source_id], count);

	if (count == 0) {
		/* hw_sem was already released */
		ERROR(TAG "%s:id:%u, hw_sem already released\n", __func__, id);
		goto out;
	}

	if (hw_sema->vote[source_id] == 0) {
		/* hw_sem was already released */
		ERROR(TAG "%s:id:%u:source_id:%u, hw_sem already released\n",
		      __func__, id, source_id);
		goto out;
	}

	vote_count_dec(hw_sema, source_id);
	count = vote_count(hw_sema);
	if (count > 0)
		goto out; /* hw_sem only vote */

	if (id == MTK_SMMU_ID_APU) {
		ret = apusys_rv_iommu_hw_sem_unlock();
	} else if (id == MTK_SMMU_ID_MM) {
		ret = mm_pm_put(hw_sema, source_id);
	}
out:
	SMMUDBG("%s:id:%u:source_id:%u:vote:%u:vote_count:%u end ret:%d\n",
		__func__, id, source_id, hw_sema->vote[source_id], count, ret);

	spin_unlock(&hw_sema->lock);
	return ret;
}

/*
 * The function is used handle some request from Rich OS.
 * x1: TF-A cmd (format: sec[11:11] + smmu_id[10:8] + cmd_id[7:0])
 * x2: other parameters
 */
static u_register_t mtk_smmu_handler(u_register_t x1, u_register_t x2,
				     u_register_t x3, u_register_t x4,
				     void *handle, struct smccc_res *smccc_ret)
{
	uint32_t ret = SMMU_CMD_ERR;
	uint32_t cmd_id = F_MSK_SHIFT(x1, SMMU_SMC_CMD_H, SMMU_SMC_CMD_L);
	enum smmu_id smmu_id = F_MSK_SHIFT(x1, SMMU_SMC_ID_H, SMMU_SMC_ID_L);
	enum cmd_source source_id = (enum cmd_source)x2;

	if (smmu_id >= MTK_SMMU_ID_NUM || source_id >= SMMU_CMD_SOURCE_NUM)
		return SMMU_ID_ERR;

	switch (cmd_id) {
	case SMMU_SECURE_PM_GET:
		ret = mtk_smmu_pm_get(smmu_id, source_id);
		break;
	case SMMU_SECURE_PM_PUT:
		ret = mtk_smmu_pm_put(smmu_id, source_id);
		break;
	default:
		break;
	}

	return ret;
}
/* Register MTK SMMU service */
DECLARE_SMC_HANDLER(MTK_SIP_IOMMU_CONTROL, mtk_smmu_handler);

#if defined(MTK_SMMU_MT8196)
static struct hw_sema_t smmu_hw_semas[MTK_SMMU_ID_NUM] = {
	{
		.id = MTK_SMMU_ID_MM,
		.active = true,
	},
	{
		.id = MTK_SMMU_ID_APU,
		.active = true,
	},
	{
		.id = MTK_SMMU_ID_SOC,
		.active = false,
	},
	{
		.id = MTK_SMMU_ID_GPU,
		.active = true,
	},
};
#else
static struct hw_sema_t *smmu_hw_semas;
#endif

/* Register MTK SMMU driver setup init function */
static int mtk_smmu_init(void)
{
	hw_semas = smmu_hw_semas;

	if (!hw_semas) {
		ERROR("%s: failed.\n", __func__);
		return -ENODEV;
	}
	SMMUDBG("%s done.\n", __func__);
	return 0;
}
MTK_PLAT_SETUP_0_INIT(mtk_smmu_init);
