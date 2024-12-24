/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include "apusys_ammu.h"
#include <apusys_security_ctrl_perm.h>
#include <mtk_mmap_pool.h>

static void apummu_set_segment_offset0(uint32_t vsid_idx, uint8_t seg_idx, uint32_t input_adr,
				       uint8_t res_bits, uint8_t page_sel, uint8_t page_len)
{
	mmio_write_32(APUMMU_VSID_SEGMENT_BASE(vsid_idx, seg_idx, APUMMU_SEG_OFFSET_0),
		      APUMMU_BUILD_SEGMENT_OFFSET0(input_adr, res_bits, page_sel, page_len));
}

static void apummu_set_segment_offset1(uint32_t vsid_idx, uint8_t seg_idx, uint32_t output_adr,
				       uint8_t res0, uint8_t iommu_en, uint8_t res1)
{
	mmio_write_32(APUMMU_VSID_SEGMENT_BASE(vsid_idx, seg_idx, APUMMU_SEG_OFFSET_1),
		      APUMMU_BUILD_SEGMENT_OFFSET1(output_adr, res0, iommu_en, res1));
}

static void apummu_set_segment_offset2(uint32_t vsid_idx, uint8_t seg_idx, uint8_t resv,
				       uint8_t domain, uint8_t acp_en, uint8_t aw_clr,
				       uint8_t aw_invalid, uint8_t ar_exclu, uint8_t ar_sepcu,
				       uint8_t aw_cache_allocate, uint8_t aw_slc_en,
				       uint8_t aw_slb_en, uint8_t ar_cache_allocate,
				       uint8_t ar_slc_en, uint8_t ar_slb_en, uint8_t ro,
				       uint8_t ns)
{
	mmio_write_32(APUMMU_VSID_SEGMENT_BASE(vsid_idx, seg_idx, APUMMU_SEG_OFFSET_2),
		      APUMMU_BUILD_SEGMENT_OFFSET2(resv, domain, acp_en, aw_clr, aw_invalid,
						   ar_exclu, ar_sepcu, aw_cache_allocate,
						   aw_slc_en, aw_slb_en, ar_cache_allocate,
						   ar_slc_en, ar_slb_en, ro, ns));
}

static void apummu_vsid_segment_enable_init(uint8_t vsid_idx)
{
	mmio_write_32(APUMMU_VSID_SEGMENT_ENABLE(vsid_idx), 0);
}

static void apummu_set_single_segment(uint8_t vsid_idx, uint8_t seg_idx)
{
	mmio_setbits_32(APUMMU_VSID_SEGMENT_ENABLE(vsid_idx), BIT(seg_idx));
}

static int apummu_enable_vsid(uint32_t vsid_idx)
{
	if (vsid_idx > (APUMMU_VSID_ACTIVE - 1) &&
	    vsid_idx < (APUMMU_RSV_VSID_IDX_END - APUMMU_VSID_RSV + 1)) {
		ERROR("invalid vsid index %d\n", vsid_idx);
		return -1;
	}

	mmio_write_32(APUMMU_VSID_ENABLE_BASE(vsid_idx), BIT(vsid_idx & APUMMU_VSID_EN_MASK));
	mmio_write_32(APUMMU_VSID_VALID_BASE(vsid_idx), BIT(vsid_idx & APUMMU_VSID_EN_MASK));

	return 0;
}

static void apummu_enable(void)
{
	mmio_setbits_32(APUMMU_CMU_TOP_BASE, 0x1);
}

static void apummu_vsid_sram_config(void)
{
	uint32_t idx;
	uint32_t base = (APUMMU_VSID_SRAM_TOTAL - APUMMU_VSID_RSV);

	for (idx = 0; idx < APUMMU_VSID_RSV; idx++) {
		mmio_write_32(APUMMU_VSID(APUMMU_RSV_VSID_IDX_START + idx),
			      APUMMU_VSID_DESC(base + idx));
		apummu_vsid_segment_enable_init(base + idx);
	}
}

static void apummu_bind_vsid(uint32_t tcu_base, uint32_t vsid_idx, uint8_t cor_id,
			     uint8_t hw_thread, uint8_t cor_valid, uint8_t vsid_valid)
{
	mmio_write_32((tcu_base + hw_thread * VSID_THREAD_SZ),
		      (((cor_id & VSID_CORID_MASK) << VSID_CORID_OFF) |
		       ((vsid_idx & VSID_IDX_MASK) << VSID_IDX_OFF) |
		       ((cor_valid & VSID_VALID_MASK) << VSID_COR_VALID_OFF) |
		       ((vsid_valid & VSID_VALID_MASK) << VSID_VALID_OFF)));
}

static int apummu_rv_bind_vsid(uint8_t hw_thread)
{
	uint8_t cor_id = 0, cor_valid = 0, vsid_valid = 1;

	if (hw_thread > APUMMU_HW_THREAD_MAX) {
		ERROR("%s: the hw thread id (%d) is not valid for rv/logger\n", __func__,
		       hw_thread);
		return -EINVAL;
	}

	apummu_bind_vsid(APUMMU_RCX_UPRV_TCU_BASE, APUMMU_UPRV_RSV_VSID, cor_id, hw_thread,
			 cor_valid, vsid_valid);

	return 0;
}

static int apummu_apmcu_bind_vsid(uint8_t hw_thread)
{
	uint8_t cor_id = 0, cor_valid = 0, vsid_valid = 1;

	if (hw_thread > APUMMU_HW_THREAD_MAX) {
		ERROR("%s: the hw thread id (%d) is not valid for apmcu\n", __func__, hw_thread);
		return -EINVAL;
	}

	apummu_bind_vsid(APUMMU_RCX_EXTM_TCU_BASE, APUMMU_APMCU_RSV_VSID, cor_id, hw_thread,
			 cor_valid, vsid_valid);

	return 0;
}

static int apummu_add_map(uint32_t vsid_idx, uint8_t seg_idx, uint64_t input_adr,
			  uint64_t output_adr, uint8_t page_sel, uint8_t page_len,
			  uint8_t domain, uint8_t ns)
{
	uint8_t smmu_sid;
	bool smmu_sec_id;

	if (seg_idx > APUMMU_SEG_MAX) {
		ERROR("seg_idx is illegal (0x%x)\n", seg_idx);
		return -EINVAL;
	}

	smmu_sec_id = false;
	if (ns == 0)
		smmu_sid = SMMU_NORMAL_1_4G_SID;
	else
		smmu_sid = (output_adr > 0xFFFFFFFF) ? SMMU_NORMAL_4_16G_SID
						     : SMMU_NORMAL_1_4G_SID;

	/* fill segment */
	apummu_set_segment_offset0(vsid_idx, seg_idx, (input_adr >> APUMMU_ADDR_SHIFT), 0,
				   page_sel, page_len);
	apummu_set_segment_offset1(vsid_idx, seg_idx, (output_adr >> APUMMU_ADDR_SHIFT),
				   smmu_sid, 0, smmu_sec_id);
	apummu_set_segment_offset2(vsid_idx, seg_idx, 0, domain,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ns);
	apummu_set_single_segment(vsid_idx, seg_idx);

	return 0;
}

static int apummu_get_dns(enum apusys_dev_type engine_type, enum apusys_sec_level sec_level,
			  uint8_t *domain, uint8_t *ns)
{
	int ret = 0;

	if (engine_type != APUSYS_DEVICE_NUM) {
		ret = sec_get_dns(engine_type, sec_level, domain, ns);
		if (ret)
			ERROR("engine:%d, sec: %d\n", engine_type, sec_level);
	} else {
		*domain = 7;
		*ns = 1;
	}

	return ret;
}

static void apummu_init(void)
{
	apummu_vsid_sram_config();
	mmio_write_32((APU_VCORE_CONFIG_BASE + APUMMU_SSID_SID_WIDTH_CTRL),
		      CSR_SMMU_AXMMUSID_WIDTH);
	apummu_enable();
}

static void virtual_engine_thread(void)
{
	mmio_write_32((APUMMU_RCX_EXTM_TCU_BASE + APUMMU_INT_D2T_TBL0_OFS), APUMMU_THD_ID_TEE);
}

static int apummu_add_apmcu_map(uint32_t seg0_input, uint32_t seg0_output,
				enum apummu_page_size page_size)
{
	int i, ret;
	uint8_t domain, ns, seg;

	ret = apummu_get_dns(APUSYS_DEVICE_NUM, SEC_LEVEL_SECURE, &domain, &ns);
	if (ret) {
		return ret;
	}

	seg = 0;
	ret = apummu_add_map(APUMMU_APMCU_RSV_DESC_IDX, seg, seg0_input, seg0_output, 0,
			     page_size, domain, ns);
	seg += 1;
	if (ret)
		return ret;

	for (i = 0; i < 4; i++) {
		ret = apummu_add_map(APUMMU_APMCU_RSV_DESC_IDX, seg,
				     APUSYS_TCM + (i * APUMMU_1M_SIZE),
				     APUSYS_TCM + (i * APUMMU_1M_SIZE),
				     0, APUMMU_PAGE_LEN_1MB, domain, ns);
		seg += 1;
		if (ret)
			return ret;
	}

	ret = apummu_enable_vsid(APUMMU_APMCU_RSV_VSID);

	return ret;
}

static int apummu_add_rv_boot_map(uint32_t seg0_output, uint32_t seg1_output, uint32_t seg2_output)
{
	int ret;
	uint8_t domain, ns;

	ret = apummu_get_dns(APUSYS_DEVICE_UP, SEC_LEVEL_SECURE, &domain, &ns);
	if (ret) {
		ERROR("sec get dns fail %d\n", ret);
		return ret;
	}

	/* must be in order */
	ret |= apummu_add_map(APUMMU_RSV_VSID_DESC_IDX_END, 0, 0, seg0_output, 0,
			      APUMMU_PAGE_LEN_1MB, domain, ns);
	ret |= apummu_add_map(APUMMU_RSV_VSID_DESC_IDX_END, 1, 0, seg1_output, 0,
			      APUMMU_PAGE_LEN_512MB, domain, ns);

	ret |= apummu_get_dns(APUSYS_DEVICE_UP, SEC_LEVEL_NORMAL, &domain, &ns);
	if (ret) {
		return ret;
	}

	ret |= apummu_add_map(APUMMU_RSV_VSID_DESC_IDX_END, 2,
			      0, seg2_output, 0, APUMMU_PAGE_LEN_4GB,
			      domain, ns);
	if (ret) {
		ERROR("sec add map fail %d\n", ret);
		return ret;
	}

	ret = apummu_enable_vsid(APUMMU_UPRV_RSV_VSID);

	return ret;
}

int rv_boot(uint32_t uP_seg_output, uint8_t uP_hw_thread,
	    enum apummu_page_size logger_page_size,
	    uint32_t XPU_seg_output, enum apummu_page_size XPU_page_size)
{
	int ret = 0;

	apummu_init();

	ret = apummu_add_rv_boot_map(uP_seg_output, 0, 0);
	if (ret) {
		return ret;
	}

	ret = apummu_rv_bind_vsid(uP_hw_thread);
	if (ret)
		return ret;

	ret = apummu_rv_bind_vsid(uP_hw_thread + 1);
	if (ret)
		return ret;

	virtual_engine_thread();

	ret = apummu_add_apmcu_map(XPU_seg_output, XPU_seg_output,
				   XPU_page_size);
	if (ret)
		return ret;

	ret = apummu_apmcu_bind_vsid(APUMMU_THD_ID_TEE);

	return ret;
}
