/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <xpu3.h>
#include <xpu_target_info.h>

#define FIELD_GET(mask, reg) (((reg) & (mask)) >> __builtin_ctz(mask))

#define XPU_TYPE_MPU 0x2
#define XPU_INITIALIZED 0x2
#define XPU_TYPE_BITMASK GENMASK(1, 0)
#define XPU_INVALID_ADDR 0xffffffffUL

#define XPU3_IDR0_NRG_BMSK GENMASK(25, 16)
#define XPU3_GCR0_DOMAIN_ENABLE BIT(0)
#define XPU3_GCR0_LOG_MODE_DISABLE BIT(1)
#define XPU3_GCR0_BASE_CFG XPU3_GCR0_DOMAIN_ENABLE
#define XPU3_CR0_INTR_ENABLE (GENMASK(3, 0) | BIT(8))
#define XPU3_GCR0_OFFSET 0x0
#define XPU3_SCR0_OFFSET 0x8
#define XPU3_CR0_OFFSET 0x10
#define XPU3_QAD0_GCR0_OFFSET 0x80
#define XPU3_QAD0_CR0_OFFSET 0x90
#define XPU3_QAD1_GCR0_OFFSET 0x100
#define XPU3_QAD1_CR0_OFFSET 0x110
#define XPU3_UMR_GCR0_OFFSET 0x300
#define XPU3_IDR0_OFFSET 0x3F8
#define XPU3_IDR2_OFFSET 0x3F0
#define XPU3_REV_OFFSET 0x3FC
#define XPU3_LOG_MODE_DIS_OFFSET 0x400
#define XPU3_RGn_GCR0_OFFSET 0x1000
#define XPU3_RGn_CR0_OFFSET 0x1010
#define XPU3_RGn_CR1_OFFSET 0x1014
#define XPU3_RGn_CR2_OFFSET 0x1018
#define XPU3_RGn_CR3_OFFSET 0x101C
#define XPU3_RGn_START0_OFFSET 0x1030
#define XPU3_RGn_START1_OFFSET 0x1034
#define XPU3_RGn_END0_OFFSET 0x1038
#define XPU3_RGn_END1_OFFSET 0x103C
#define XPU3_UMR_CR1_OFFSET 0x314
#define XPU3_UMR_CR3_OFFSET 0x31C
#define XPU3_UMR_CR0_OFFSET 0x310
#define XPU3_UMR_CR2_OFFSET 0x318
#define XPU3_RGn_REG_SPACE_SIZE 0x80
#define XPU3_REV_SHFT 0x10
#define XPU3_EAR0_OFFSET 0
#define XPU3_EAR1_OFFSET 1
#define XPU3_ESR_OFFSET 2
#define XPU3_SRRESTORE_OFFSET 3
#define XPU3_ESYNR0_OFFSET 4
#define XPU3_ESYNR1_OFFSET 5
#define XPU3_ESYNR2_OFFSET 6
#define XPU3_ESYNR3_OFFSET 7
#define XPU3_ESYNR4_OFFSET 8

#define XPU3_RGn_RACR_OFFSET 0x1040
#define XPU3_RGn_WACR_OFFSET 0x1060
#define XPU3_UMR_RACR_OFFSET 0x40
#define XPU3_UMR_WACR_OFFSET 0x60

#define XPU_IDR2_VMID_SUPPORT_BITMASK   0xFF00U
#define XPU_IDR2_VMID_SUPPORT_BITSHIFT  0x8U

#define DEFAULT_VMID_0 (1U << 0)

static uint8_t get_xpu_type(struct xpu_instance *xpu)
{
	uintptr_t base = xpu->xpu_base_addr;
	uint32_t idr0;

	idr0 = mmio_read_32(base + XPU3_IDR0_OFFSET);

	return (uint8_t)(idr0 & XPU_TYPE_BITMASK);
}

static void dump_log(enum xpu xpu, int type)
{
	char sec_char[2] = { ' ', 0 };
	uint32_t xpu_addr;
	uint32_t offset;

	switch (type) {
	case XPU_ERR_SEC_CTX:
		offset = 0x800;
		sec_char[0] = 'S';
		break;
	case XPU_ERR_NON_SEC_CTX:
		offset = 0x880;
		break;
	default:
		return;
	}

	for (size_t i = 0; i < g_xpu_base_addr_array_count; i++) {
		if (g_xpu_base_addr_array[i].e_xpu != xpu)
			continue;

		xpu_addr = g_xpu_base_addr_array[i].base_addr + offset;

		ERROR("%s_ear0 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_EAR0_OFFSET));
		ERROR("%s_ear1 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_EAR1_OFFSET));
		ERROR("%s_esr 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESR_OFFSET));
		ERROR("%s_esynr0 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR0_OFFSET));
		ERROR("%s_esynr1 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR1_OFFSET));
		ERROR("%s_esynr2 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR2_OFFSET));
		ERROR("%s_esynr3 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR3_OFFSET));
		ERROR("%s_esynr4 0x%x\n", sec_char,
		      mmio_read_32(xpu_addr + XPU3_ESYNR4_OFFSET));

		/* clear error (SRRESTORE) as xpu errors are non fatal */
		mmio_write_32(xpu_addr + XPU3_SRRESTORE_OFFSET, 0);
		break;
	}
}

void xpu_print_log(void *ctx)
{
	uint32_t err_bitmask[ACC_XPU_ERR_INT_REG_NUM] = { 0 };
	const struct xpu_intr_reg_dtls *p = NULL;
	int err_type = *(int *)ctx;
	uint32_t addr;
	uint32_t mask;

	ERROR("xpu: ISR\n");
	switch (err_type) {
	case XPU_ERR_SEC_CTX:
		ERROR("XPU ERROR: secure\n");
		p = xpu_sec_intr_status_reg;
		break;
	case XPU_ERR_NON_SEC_CTX:
		ERROR("XPU ERROR: non secure\n");
		p = xpu_non_sec_intr_status_reg;
		break;
	default:
		return;
	}

	for (size_t i = 0; i < ACC_XPU_ERR_INT_REG_NUM; i++, p++) {
		addr = p->xpu_intr_reg_addr;
		mask = p->xpu_intr_reg_mask;

		err_bitmask[i] = mmio_read_32(addr) & mask;
	}

	if (ACC_XPU_ERR_INT_REG_NUM == 1) {
		ERROR("XPU INTR 0 >> %08x\n", err_bitmask[0]);
	} else {
		ERROR("XPU INTR 0:1 >> %08x:%08x\n", err_bitmask[0],
		      err_bitmask[1]);
	}

	for (size_t reg = 0; reg < ACC_XPU_ERR_INT_REG_NUM; reg++) {
		struct xpu_err_pos_to_hal_map *row;

		mask = err_bitmask[reg];
		if (!mask)
			continue;

		row = xpu_err_pos_to_hal_map[reg];

		for (size_t i = 0; row[i].bit_mask != 0; i++) {
			const struct xpu_err_pos_to_hal_map *m;

			if (i >= ACC_XPU_ERR_NUM_PER_REG)
				break;

			m = &row[i];

			if (!(m->bit_mask & mask))
				continue;

			if (m->xpu >= XPU_TYPE_COUNT)
				continue;

			if (m->xpu == XPU_TYPE_NONE)
				continue;

			dump_log(m->xpu, err_type);
		}
	}
}

static uint32_t get_idr0_nrg(struct xpu_instance *xpu)
{
	uint32_t id_r0 = mmio_read_32(xpu->xpu_base_addr + XPU3_IDR0_OFFSET);

	return FIELD_GET(XPU3_IDR0_NRG_BMSK, id_r0) + 1;
}

static void enable_domain(struct xpu_instance *xpu)
{
	uint32_t gcr0_cfg = XPU3_GCR0_DOMAIN_ENABLE;
	uint32_t gcr0_val;
	uint32_t rev;

	/* Already initialized from this context */
	if ((xpu->flag & XPU_INITIALIZED) != 0U)
		return;

	xpu->flag |= XPU_INITIALIZED;

	/* If GCR0 is already programmed */
	gcr0_val = mmio_read_32(xpu->xpu_base_addr + XPU3_GCR0_OFFSET);
	if (gcr0_val != 0U)
		return;

	/* HW revision check: enable log-mode disable on newer revisions */
	rev = mmio_read_32(xpu->xpu_base_addr + XPU3_REV_OFFSET);
	rev >>= XPU3_REV_SHFT;

	if (rev > 0x3002U) {
		mmio_write_32(xpu->xpu_base_addr + XPU3_LOG_MODE_DIS_OFFSET, 1);
		gcr0_cfg |= XPU3_GCR0_LOG_MODE_DISABLE;
	}

	/* Main XPU instance */
	mmio_write_32(xpu->xpu_base_addr + XPU3_GCR0_OFFSET, gcr0_cfg);
	mmio_write_32(xpu->xpu_base_addr + XPU3_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);

	/* QAD0 */
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD0_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD0_GCR0_OFFSET, gcr0_cfg);

	/* QAD1 */
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD1_CR0_OFFSET,
		      XPU3_CR0_INTR_ENABLE);
	mmio_write_32(xpu->xpu_base_addr + XPU3_QAD1_GCR0_OFFSET, gcr0_cfg);

	dmbsy();
	isb();
}

static inline bool vmid_supported(const struct xpu_instance *xpu)
{
	uint32_t idr2;

	idr2 = mmio_read_32(xpu->xpu_base_addr + XPU3_IDR2_OFFSET);

	return ((idr2 & XPU_IDR2_VMID_SUPPORT_BITMASK) >>
		XPU_IDR2_VMID_SUPPORT_BITSHIFT) != 0U;
}

static void set_mpu_permissions(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_partition_range *range = xpu->partition_range;
	struct rg_domain_ownership *owner = xpu->rg_owner;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t cr0, cr1, cr2, cr3;
	uintptr_t racr, racw;
	bool no_perms;
	uint32_t tmp;

	if (get_xpu_type(xpu) != XPU_TYPE_MPU)
		return;

	for (int i = 0; i < xpu->part_range_arr_size; i++, range++, owner++) {
		if (rg_num != XPU_RG_ALL && range->rg_num != rg_num)
			continue;

		if (i >= xpu->owner_arr_size)
			goto out;

		no_perms = (owner->perm_r == 0 && owner->perm_w == 0);

		if (owner->rg_num == XPU_UMR_RG || no_perms)
			continue;

		cr1 = base + XPU3_RGn_CR1_OFFSET +
		      XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		cr3 = base + XPU3_RGn_CR3_OFFSET +
		      XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		cr0 = base + XPU3_RGn_CR0_OFFSET +
		      XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		cr2 = base + XPU3_RGn_CR2_OFFSET +
		      XPU3_RGn_REG_SPACE_SIZE * range->rg_num;

		/* Set permissions */
		mmio_write_32(cr1, owner->perm_r);
		mmio_write_32(cr3, owner->perm_w);

		if (owner->perm_r & APPS_S_DOMAIN)
			mmio_write_32(cr0, 1);

		if (owner->perm_w & APPS_S_DOMAIN)
			mmio_write_32(cr2, 1);

		if (vmid_supported(xpu)) {
			/* RACR */
			racr = base + XPU3_RGn_RACR_OFFSET +
				XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
			tmp = owner->perm_r & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racr, tmp);

			/* RACW */
			racw = base + XPU3_RGn_WACR_OFFSET +
				XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
			tmp = owner->perm_w & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racw, tmp);
		}
	}

	/* Handle UMR region */
	no_perms = (owner->perm_r == 0 && owner->perm_w == 0);

	if ((owner->rg_num == XPU_UMR_RG) && !no_perms) {
		cr1 = base + XPU3_UMR_CR1_OFFSET;
		cr3 = base + XPU3_UMR_CR3_OFFSET;
		cr0 = base + XPU3_UMR_CR0_OFFSET;
		cr2 = base + XPU3_UMR_CR2_OFFSET;

		mmio_write_32(cr1, owner->perm_r);
		mmio_write_32(cr3, owner->perm_w);

		if (owner->perm_r & APPS_S_DOMAIN)
			mmio_write_32(cr0, 1);

		if (owner->perm_w & APPS_S_DOMAIN)
			mmio_write_32(cr2, 1);

		if (vmid_supported(xpu)) {
			/* RACR */
			racr = base + XPU3_UMR_RACR_OFFSET;
			tmp = owner->perm_r & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racr, tmp);

			/* RACW */
			racw = base + XPU3_UMR_WACR_OFFSET;
			tmp = owner->perm_w & APPS_NS_DOMAIN ?
				DEFAULT_VMID_0 : 0;
			mmio_write_32(racw, tmp);
		}
	}
out:
	dmbsy();
	isb();
}

static void set_mpu_dynamic_permissions(struct xpu_instance *xpu,
					uint32_t rg_num, uint32_t perm_r,
					uint32_t perm_w)
{
	struct rg_partition_range *range = xpu->partition_range;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t racr, racw;
	uintptr_t cr1, cr3;
	uint32_t tmp;

	if (get_xpu_type(xpu) != XPU_TYPE_MPU)
		return;

	for (size_t i = 0; i < xpu->part_range_arr_size; i++, range++) {
		if (rg_num != XPU_RG_ALL && range->rg_num != rg_num)
			continue;

		cr1 = base + XPU3_RGn_CR1_OFFSET +
		      XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		cr3 = base + XPU3_RGn_CR3_OFFSET +
		      XPU3_RGn_REG_SPACE_SIZE * range->rg_num;

		mmio_write_32(cr1, perm_r);
		mmio_write_32(cr3, perm_w);

		if (!vmid_supported(xpu))
			continue;

		racr = base + XPU3_RGn_RACR_OFFSET +
			XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		tmp = perm_r & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
		mmio_write_32(racr, tmp);

		racw = base + XPU3_RGn_WACR_OFFSET +
			XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		tmp = perm_w & APPS_NS_DOMAIN ? DEFAULT_VMID_0 : 0;
		mmio_write_32(racw, tmp);
	}
}

static void program_mpu_partitions(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_partition_range *range = xpu->partition_range;
	uint32_t start_lo, start_hi, end_lo, end_hi;
	uintptr_t start_0, start_1, end_0, end_1;

	if (get_xpu_type(xpu) != XPU_TYPE_MPU)
		return;

	for (size_t i = 0; i < xpu->part_range_arr_size; i++, range++) {
		if (rg_num != XPU_RG_ALL && range->rg_num != rg_num)
			continue;

		start_lo = (uint32_t)range->start_addr;
		start_hi = (uint32_t)(range->start_addr >> 32);

		end_lo = (uint32_t)range->end_addr;
		end_hi = (uint32_t)(range->end_addr >> 32);

		start_0 = xpu->xpu_base_addr + XPU3_RGn_START0_OFFSET +
			  XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		start_1 = xpu->xpu_base_addr + XPU3_RGn_START1_OFFSET +
			  XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		end_0 = xpu->xpu_base_addr + XPU3_RGn_END0_OFFSET +
			XPU3_RGn_REG_SPACE_SIZE * range->rg_num;
		end_1 = xpu->xpu_base_addr + XPU3_RGn_END1_OFFSET +
			XPU3_RGn_REG_SPACE_SIZE * range->rg_num;

		/* Set the specified address range in the partition */
		mmio_write_32(start_0, start_lo);
		mmio_write_32(start_1, start_hi);
		mmio_write_32(end_0, end_lo);
		mmio_write_32(end_1, end_hi);
	}

	dmbsy();
	isb();
}

static void set_region_ownership(struct xpu_instance *xpu, uint32_t rg_num)
{
	struct rg_domain_ownership *owner = xpu->rg_owner;
	uintptr_t base = xpu->xpu_base_addr;
	uintptr_t gcr0;

	for (size_t i = 0; i < xpu->owner_arr_size; i++, owner++) {
		/* Handle unmapped region ownership */
		if (owner->rg_num == XPU_UMR_RG) {
			gcr0 = base + XPU3_UMR_GCR0_OFFSET;
			mmio_write_32(gcr0, owner->owner_domain);
			continue;
		}

		/* Apply to ALL or specific region */
		if (rg_num != XPU_RG_ALL && owner->rg_num != rg_num)
			continue;

		gcr0 = base + XPU3_RGn_GCR0_OFFSET +
		       XPU3_RGn_REG_SPACE_SIZE * owner->rg_num;

		/* Set the specified domain as the owner */
		mmio_write_32(gcr0, owner->owner_domain);
	}
}

void xpu_lock_down_assets(struct xpu_instance *xpus, uint8_t xpu_count)
{
	for (size_t i = 0; i < xpu_count; i++, xpus++) {
		if (!(xpus->flag & XPU_PROTECTION_STATIC))
			continue;

		enable_domain(xpus);

		program_mpu_partitions(xpus, XPU_RG_ALL);
		set_region_ownership(xpus, XPU_RG_ALL);
		set_mpu_permissions(xpus, XPU_RG_ALL);
	}
}

int xpu_lock_down_assets_dynamic(struct xpu_instance *xpus, uint8_t xpu_count,
				 uint32_t xpu_id, uint32_t rg_num,
				 uint32_t perm_r, uint32_t perm_w)
{
	uint32_t i;

	for (i = 0; i < xpu_count; i++, xpus++) {
		if (xpus->xpu_id == xpu_id)
			break;
	}

	if (i >= xpu_count)
		return -1;

	enable_domain(xpus);

	program_mpu_partitions(xpus, rg_num);
	set_region_ownership(xpus, rg_num);
	set_mpu_dynamic_permissions(xpus, rg_num, perm_r, perm_w);

	return 0;
}

static void xpu_init_unused_regions(struct xpu_instance *mpu, uint32_t dev_cnt)
{
	uint32_t part_len = (uint32_t)mpu->part_range_arr_size;
	uint32_t owner_len = (uint32_t)mpu->owner_arr_size;
	uint32_t num_regions = get_idr0_nrg(mpu);

	if ((dev_cnt == 0U) || (dev_cnt > num_regions) ||
	    (dev_cnt > owner_len) || (dev_cnt > part_len)) {
		ERROR("XPU invalid board configuration\n");
		return;
	}

	uint32_t rg_start = num_regions - dev_cnt;

	struct rg_partition_range *part =
		mpu->partition_range + (part_len - dev_cnt);

	struct rg_domain_ownership *owner =
		mpu->rg_owner + (owner_len - dev_cnt);

	/* Prepare un-initialized regions for dynamic configuration */
	for (uint32_t k = 0U; k < dev_cnt; k++) {
		uint32_t rg = rg_start + k;

		owner->owner_domain = NO_DOMAIN;
		owner->rg_num = rg;

		part->start_addr = XPU_INVALID_ADDR;
		part->end_addr = XPU_INVALID_ADDR;
		part->rg_num = rg;

		owner++;
		part++;
	}
}

static void xpu_init_mpu_instances(struct mpu_ranges *range)
{
	for (uint32_t i = 0; i < range->mpus_count; i++) {
		struct xpu_instance *mpu = &range->mpus[i];

		if (get_xpu_type(mpu) != XPU_TYPE_MPU)
			continue;

		uint32_t dev_cnt = range->device_prtn_cnt;

		xpu_init_unused_regions(mpu, dev_cnt);
		enable_domain(mpu);
	}
}

void xpu_master_mpu_init(struct mpu_ranges *ranges, const uint32_t range_count)
{
	struct mpu_ranges *range = ranges;

	for (size_t i = 0; i < range_count; i++, range++)
		xpu_init_mpu_instances(range);
}
