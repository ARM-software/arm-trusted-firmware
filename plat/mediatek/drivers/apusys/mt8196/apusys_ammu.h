/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_AMMU_H
#define APUSYS_AMMU_H

#include <platform_def.h>

/* CMU */
#define APUMMU_CMU_TOP_BASE		(APU_CMU_TOP)
#define APUMMU_CMU_TOP_TOPOLOGY		(APUMMU_CMU_TOP_BASE + 0x04)
#define APUMMU_VSID_ENABLE_OFFSET	(0x50)
#define APUMMU_VSID_VALID_OFFSET	(0xb0)

#define VSID_OFFSET(vsid_idx)		(((vsid_idx) >> 5) * 0x4)

#define APUMMU_VSID_ENABLE_BASE(vsid_idx) \
	(APUMMU_CMU_TOP_BASE + VSID_OFFSET(vsid_idx) + APUMMU_VSID_ENABLE_OFFSET)
#define APUMMU_VSID_VALID_BASE(vsid_idx) \
	(APUMMU_CMU_TOP_BASE + VSID_OFFSET(vsid_idx) + APUMMU_VSID_VALID_OFFSET)

/* VSID SRAM */
#define APUMMU_VSID_BASE		(APUMMU_CMU_TOP_BASE + 0x1000)
#define APUMMU_VSID_DESC_BASE		(APUMMU_VSID_BASE + 0x400)
#define APUMMU_VSID_SRAM_SZIE		(0x5C00)
#define APUMMU_VSID_TBL_SZIE		(0xF4)

#define APUMMU_VSID(vsid_idx)		(APUMMU_VSID_BASE + (vsid_idx) * 4)
#define APUMMU_VSID_DESC(vsid_idx) \
	(APUMMU_VSID_DESC_BASE + (vsid_idx) * APUMMU_VSID_TBL_SZIE)

/* TCU RCX */
#define APU_VCORE_CONFIG_BASE		(APU_RCX_VCORE_CONFIG)
#define APUMMU_RCX_EXTM_TCU_BASE	(APU_RCX_EXTM_TCU)
#define APUMMU_RCX_UPRV_TCU_BASE	(APU_RCX_UPRV_TCU)

#define APUMMU_SSID_SID_WIDTH_CTRL	(0xCC0)
#define CSR_SMMU_AXMMUSID_WIDTH		BIT(7)
#define APUMMU_1M_SIZE			(0x100000)

#define SMMU_NORMAL_0_1G_SID		(0x8)
#define SMMU_NORMAL_1_4G_SID		(0x9)
#define SMMU_NORMAL_4_16G_SID		(0xA)

enum apummu_page_size {
	APUMMU_PAGE_LEN_128KB = 0,
	APUMMU_PAGE_LEN_256KB,
	APUMMU_PAGE_LEN_512KB,
	APUMMU_PAGE_LEN_1MB,
	APUMMU_PAGE_LEN_128MB,
	APUMMU_PAGE_LEN_256MB,
	APUMMU_PAGE_LEN_512MB,
	APUMMU_PAGE_LEN_4GB,
};

#define APUMMU_VSID_SEGMENT_BASE(vsid_idx, seg_idx, seg_offset) \
	(APUMMU_VSID_DESC(vsid_idx) + (seg_idx) * 0xC + (seg_offset) * 0x04 + 0x4)

#define APUMMU_VSID_SEGMENT_ENABLE(vsid_idx)	(APUMMU_VSID_DESC(vsid_idx))

#define APUMMU_VSID_SRAM_TOTAL		(APUMMU_VSID_SRAM_SZIE / APUMMU_VSID_TBL_SZIE)
#define APUMMU_RSV_VSID_DESC_IDX_END	(APUMMU_VSID_SRAM_TOTAL - 1)
#define APUMMU_UPRV_RSV_DESC_IDX	(APUMMU_RSV_VSID_DESC_IDX_END)		/* 53 */
#define APUMMU_LOGGER_RSV_DESC_IDX	(APUMMU_RSV_VSID_DESC_IDX_END - 1)
#define APUMMU_APMCU_RSV_DESC_IDX	(APUMMU_RSV_VSID_DESC_IDX_END - 2)
#define APUMMU_GPU_RSV_DESC_IDX		(APUMMU_RSV_VSID_DESC_IDX_END - 3)

#define APUMMU_SEG_OFFSET_0		(0)
#define APUMMU_SEG_OFFSET_1		(1)
#define APUMMU_SEG_OFFSET_2		(2)
#define APUMMU_VSID_EN_MASK		(0x1f)

#define APUMMU_HW_THREAD_MAX		(7)
#define APUMMU_SEG_MAX			(9)
#define APUMMU_ADDR_SHIFT		(12)

#define VSID_THREAD_SZ			(0x4)
#define VSID_CORID_MASK			(0x7f)
#define VSID_CORID_OFF			(11)
#define VSID_IDX_MASK			(0xff)
#define VSID_IDX_OFF			(3)
#define VSID_VALID_MASK			(0x1)
#define VSID_COR_VALID_OFF		(1)
#define VSID_VALID_OFF			(0)

#define APUMMU_VSID_ACTIVE		(32)
#define APUMMU_VSID_RSV			(4)
#define APUMMU_VSID_UNUSED		(12)
#define APUMMU_VSID_USE_MAX		(APUMMU_VSID_ACTIVE + APUMMU_VSID_RSV)

#if ((APUMMU_VSID_RSV + APUMMU_VSID_ACTIVE + APUMMU_VSID_UNUSED + 1) > APUMMU_VSID_SRAM_TOTAL)
#error APUMMU VSID Overflow
#endif

#define APUMMU_RSV_VSID_IDX_END		(254)
#define APUMMU_RSV_VSID_IDX_START	(APUMMU_RSV_VSID_IDX_END - APUMMU_VSID_RSV + 1)

#if ((APUMMU_RSV_VSID_IDX_END - APUMMU_RSV_VSID_IDX_START) > APUMMU_VSID_RSV)
#error APUMMU VSID RSV Overflow
#endif

/* Reserve */
#define APUMMU_UPRV_RSV_VSID		(APUMMU_RSV_VSID_IDX_END)
#define APUMMU_LOGGER_RSV_VSID		(APUMMU_RSV_VSID_IDX_END - 1)
#define APUMMU_APMCU_RSV_VSID		(APUMMU_RSV_VSID_IDX_END - 2)
#define APUMMU_GPU_RSV_VSID		(APUMMU_RSV_VSID_IDX_END - 3)

/* VSID bit mask */
#define APUMMU_VSID_MAX_MASK_WORD	((APUMMU_VSID_USE_MAX + 32 - 1) / 32)

/* VSID fields */
#define READ_VSID_FIELD(vids, sg, offset, shift, mask) \
	((mmio_read_32(APUMMU_VSID_SEGMENT_BASE(vsid, seg, offset)) >> sift) & mask)
#define READ_VSID_FIELD_OFFESET0(vids, sg, shift, mask) \
	READ_VSID_FIELD(vids, sg, 0, shift, mask)
#define READ_VSID_FIELD_OFFESET1(vids, sg, shift, mask) \
	READ_VSID_FIELD(vids, sg, 1, shift, mask)
#define READ_VSID_FIELD_OFFESET2(vids, sg, shift, mask) \
	READ_VSID_FIELD(vids, sg, 2, shift, mask)

/* Get segment offset 0 data - 0x00 */
#define APUMMU_SEGMENT_GET_INPUT(vsid, seg) \
	READ_VSID_FIELD_OFFESET0(vsid, seg, 10, 0x3FFFFF)
#define APUMMU_SEGMENT_GET_OFFSET0_RSRV(vsid, seg) \
	READ_VSID_FIELD_OFFESET0(vsid, seg, 6, 0xF)
#define APUMMU_SEGMENT_GET_PAGELEN(vsid, seg) \
	READ_VSID_FIELD_OFFESET0(vsid, seg, 0, 0x7)
#define APUMMU_SEGMENT_GET_PAGESEL(vsid, seg) \
	READ_VSID_FIELD_OFFESET0(vsid, seg, 3, 0x7)

/* Get segment offset 1 data - 0x04 */
#define APUMMU_SEGMENT_GET_IOMMU_EN(vsid, seg) \
	READ_VSID_FIELD_OFFESET1(vsid, seg, 1, 0x1)
#define APUMMU_SEGMENT_GET_OFFSET1_RSRV0(vsid, seg) \
	READ_VSID_FIELD_OFFESET1(vsid, seg, 2, 0xFF)
#define APUMMU_SEGMENT_GET_OFFSET1_RSRV1(vsid, seg) \
	READ_VSID_FIELD_OFFESET1(vsid, seg, 0, 0x1)
#define APUMMU_SEGMENT_GET_OUTPUT(vsid, seg) \
	READ_VSID_FIELD_OFFESET1(vsid, seg, 10, 0x3FFFFF)

/* Get segment offset 2 data - 0x08 */
#define APUMMU_SEGMENT_GET_ACP_EN(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 12, 0x1)
#define APUMMU_SEGMENT_GET_AR_CACHE_ALLOC(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 4, 0x1)
#define APUMMU_SEGMENT_GET_AR_EXCLU(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 9, 0x1)
#define APUMMU_SEGMENT_GET_AR_SEPCU(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 8, 0x1)
#define APUMMU_SEGMENT_GET_AR_SLB_EN(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 2, 0x1)
#define APUMMU_SEGMENT_GET_AR_SLC_EN(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 3, 0x1)
#define APUMMU_SEGMENT_GET_AW_CACHE_ALLOC(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 7, 0x1)
#define APUMMU_SEGMENT_GET_AW_CLR(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 11, 0x1)
#define APUMMU_SEGMENT_GET_AW_INVALID(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 10, 0x1)
#define APUMMU_SEGMENT_GET_AW_SLB_EN(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 5, 0x1)
#define APUMMU_SEGMENT_GET_AW_SLC_EN(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 6, 0x1)
#define APUMMU_SEGMENT_GET_DOMAIN(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 13, 0xF)
#define APUMMU_SEGMENT_GET_NS(vsid, seg) \
	READ_VSID_FIELD_OFFESET2(vsid, seg, 0, 0x1)

/* Build segment data */
/* Build segment offset 0 (0x00) data */
#define APUMMU_VSID_SEGMENT_00_INPUT(input_adr)		(((input_adr) & 0x3fffff) << 10)
#define APUMMU_VSID_SEGMENT_00_PAGESEL(page_sel)	(((page_sel) & 0x7) << 3)
#define APUMMU_VSID_SEGMENT_00_PAGELEN(page_len)	(((page_len) & 0x7) << 0)
#define APUMMU_VSID_SEGMENT_00_RESV(resv)		(((resv) & 0xf) << 6)

#define APUMMU_BUILD_SEGMENT_OFFSET0(input_adr, resv, page_sel, page_len) \
				(APUMMU_VSID_SEGMENT_00_INPUT(input_adr) | \
				 APUMMU_VSID_SEGMENT_00_RESV(resv) | \
				 APUMMU_VSID_SEGMENT_00_PAGESEL(page_sel) | \
				 APUMMU_VSID_SEGMENT_00_PAGELEN(page_len))

/* Build segment offset 1 (0x04) data */
#define APUMMU_VSID_SEGMENT_04_IOMMU_EN(iommu_en)	(((iommu_en) & 0x1) << 1)
#define APUMMU_VSID_SEGMENT_04_OUTPUT(output_adr)	(((output_adr) & 0x3fffff) << 10)
#define APUMMU_VSID_SEGMENT_04_RESV0(resv0)		(((resv0) & 0xff) << 2)
#define APUMMU_VSID_SEGMENT_04_RESV1(resv1)		(((resv1) & 0x1) << 0)

#define APUMMU_BUILD_SEGMENT_OFFSET1(output_adr, resv0, iommu_en, resv1) \
				(APUMMU_VSID_SEGMENT_04_OUTPUT(output_adr) | \
				 APUMMU_VSID_SEGMENT_04_RESV0(resv0) | \
				 APUMMU_VSID_SEGMENT_04_IOMMU_EN(iommu_en) | \
				 APUMMU_VSID_SEGMENT_04_RESV1(resv1))

/* Build segment offset 2 (0x08) data */
#define APUMMU_VSID_SEGMENT_08_DOMAIN_MASK	(0xf)
#define APUMMU_VSID_SEGMENT_08_DOMAIN_SHIFT	(13)
#define APUMMU_VSID_SEGMENT_08_RESV_MASK	(0x7fff)
#define APUMMU_VSID_SEGMENT_08_RESV_SHIFT	(17)

#define APUMMU_VSID_SEGMENT_08_DOMAIN(domain) \
	(((domain) & APUMMU_VSID_SEGMENT_08_DOMAIN_MASK) << APUMMU_VSID_SEGMENT_08_DOMAIN_SHIFT)
#define APUMMU_VSID_SEGMENT_08_RESV(resv) \
	(((resv) & APUMMU_VSID_SEGMENT_08_RESV_MASK) << APUMMU_VSID_SEGMENT_08_RESV_SHIFT)

#define APUMMU_VSID_SEGMENT_08_ACP_EN(acp_en)		(((acp_en) & 0x1) << 12)
#define APUMMU_VSID_SEGMENT_08_AR_EXCLU(ar_exclu)	(((ar_exclu) & 0x1) << 9)
#define APUMMU_VSID_SEGMENT_08_AR_SEPCU(ar_sepcu)	(((ar_sepcu) & 0x1) << 8)
#define APUMMU_VSID_SEGMENT_08_AR_SLB_EN(ar_slb_en)	(((ar_slb_en) & 0x1) << 2)
#define APUMMU_VSID_SEGMENT_08_AR_SLC_EN(ar_slc_en)	(((ar_slc_en) & 0x1) << 3)
#define APUMMU_VSID_SEGMENT_08_AW_CLR(aw_clr)		(((aw_clr) & 0x1) << 11)
#define APUMMU_VSID_SEGMENT_08_AW_INVALID(aw_invalid)	(((aw_invalid) & 0x1) << 10)
#define APUMMU_VSID_SEGMENT_08_AW_SLB_EN(aw_slb_en)	(((aw_slb_en) & 0x1) << 5)
#define APUMMU_VSID_SEGMENT_08_AW_SLC_EN(aw_slc_en)	(((aw_slc_en) & 0x1) << 6)
#define APUMMU_VSID_SEGMENT_08_NS(ns)			(((ns) & 0x1) << 0)
#define APUMMU_VSID_SEGMENT_08_RO(ro)			(((ro) & 0x1) << 1)

#define APUMMU_VSID_SEGMENT_08_AR_CACHE_ALLOCATE(ar_cache_allocate) \
	(((ar_cache_allocate) & 0x1) << 4)
#define APUMMU_VSID_SEGMENT_08_AW_CACHE_ALLOCATE(aw_cache_allocate) \
	(((aw_cache_allocate) & 0x1) << 7)

#define APUMMU_BUILD_SEGMENT_OFFSET2(resv, domain, acp_en, aw_clr, \
		aw_invalid, ar_exclu, ar_sepcu, \
		aw_cache_allocate, aw_slc_en, aw_slb_en, ar_cache_allocate, \
		ar_slc_en, ar_slb_en, ro, ns) \
		((APUMMU_VSID_SEGMENT_08_RESV(resv)) |\
		 (APUMMU_VSID_SEGMENT_08_DOMAIN(domain)) |\
		 (APUMMU_VSID_SEGMENT_08_ACP_EN(acp_en)) |\
		 (APUMMU_VSID_SEGMENT_08_AW_CLR(aw_clr)) |\
		 (APUMMU_VSID_SEGMENT_08_AW_INVALID(aw_invalid)) |\
		 (APUMMU_VSID_SEGMENT_08_AR_EXCLU(ar_exclu)) |\
		 (APUMMU_VSID_SEGMENT_08_AR_SEPCU(ar_sepcu)) |\
		 (APUMMU_VSID_SEGMENT_08_AW_CACHE_ALLOCATE(aw_cache_allocate)) |\
		 (APUMMU_VSID_SEGMENT_08_AW_SLC_EN(aw_slc_en)) |\
		 (APUMMU_VSID_SEGMENT_08_AW_SLB_EN(aw_slb_en)) |\
		 (APUMMU_VSID_SEGMENT_08_AR_CACHE_ALLOCATE(ar_cache_allocate)) |\
		 (APUMMU_VSID_SEGMENT_08_AR_SLC_EN(ar_slc_en)) |\
		 (APUMMU_VSID_SEGMENT_08_AR_SLB_EN(ar_slb_en)) |\
		 (APUMMU_VSID_SEGMENT_08_RO(ro)) | (APUMMU_VSID_SEGMENT_08_NS(ns)))

/* Build segment offset 3 (0x0c) data */
#define APUMMU_VSID_SEGMENT_0C_RESV(rsv)		(((rsv) & 0x7fffffff) << 0)
#define APUMMU_VSID_SEGMENT_0C_SEG_VALID(seg_valid)	(((seg_valid) & 0x1U) << 31)
#define APUMMU_BUILD_SEGMENT_OFFSET3(seg_valid, rsv) \
	((uint32_t)APUMMU_VSID_SEGMENT_0C_SEG_VALID(seg_valid) | \
	 APUMMU_VSID_SEGMENT_0C_RESV(rsv))

#define APUMMU_INT_D2T_TBL0_OFS	(0x40)

#define APUSYS_TCM		(0x4d100000)

enum {
	APUMMU_THD_ID_APMCU_NORMAL = 0,
	APUMMU_THD_ID_TEE,
};

int rv_boot(uint32_t uP_seg_output, uint8_t uP_hw_thread,
	    enum apummu_page_size logger_page_size, uint32_t XPU_seg_output,
	    enum apummu_page_size XPU_page_size);

#endif
