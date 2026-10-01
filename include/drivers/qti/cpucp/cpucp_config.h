/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_CONFIG_H
#define QTI_CPUCP_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#include <cpucp_hwio.h>
#include <cpucp_shared.h>

#define SHARED(member)	(CPUCP_SHARED_DATA_BASE + \
			 offsetof(struct cpucp_shared_data, member))

/*
 * An APSS clock the CPUCP subsystem runs from: its source mux is switched to
 * GPLL0 and its divider set so the output is GPLL0 / (div + 1).
 */
struct apss_clk {
	uintptr_t cdiv;
	unsigned int cdiv_lsb;
	unsigned int cdiv_width;
	uintptr_t gfmux;
	unsigned int div;
};

struct fuse_part {
	uintptr_t reg;
	unsigned int lsb;
	unsigned int width;
};

/* A fused value split over up to two fields, least significant part first. */
struct fuse_field {
	struct fuse_part lo;
	struct fuse_part hi;
};

#define ROW_LSB(n)	QFPROM_CORR_CALIBRATION_ROW_LSB(n)
#define ROW_MSB(n)	QFPROM_CORR_CALIBRATION_ROW_MSB(n)

#define F(reg, lsb, width)	{ { (reg), (lsb), (width) }, { 0U, 0U, 0U } }
#define F2(reg0, lsb0, width0, reg1, lsb1, width1) \
	{ { (reg0), (lsb0), (width0) }, { (reg1), (lsb1), (width1) } }

/*
 * Per-target CPR fuse layout. The APSS CPR instances [first_cpr, first_cpr +
 * num_cpr) are provisioned from the target fuse tables, each of which is a
 * [num_cpr][CPUCP_NUM_FUSED_CORNERS] array flattened to a pointer. quot_offset
 * is optional: when NULL the firmware receives a zero offset, otherwise the
 * fused value scaled by quot_offset_step. aging is a per-instance table of
 * num_cpr entries. num_acc_domains ACC fuses are cleared.
 */
struct cpucp_cpr_config {
	const struct fuse_field *targ_volt;
	const struct fuse_field *quot_offset;
	const struct fuse_field *quot_vmin;
	const struct fuse_field *aging;
	unsigned int first_cpr;
	unsigned int num_cpr;
	unsigned int quot_offset_step;
	unsigned int num_acc_domains;
};

/*
 * Target description consumed by the common hoya CPUCP start sequence. Every
 * value that varies between hoya SoCs is data here; the algorithm that acts on
 * it lives in hoya/cpucp_start.c.
 */
struct cpucp_config {
	const struct apss_clk *apss_clks;
	unsigned int num_apss_clks;
	struct cpucp_cpr_config cpr;
	/*
	 * EPSS non-secure write override. The boot firmware opens it to load
	 * CPUCP; only version 1 silicon keeps it for kernel L3 voting. Left
	 * zero on targets that never touch the override.
	 */
	uintptr_t secure_access_reg;
	uint32_t secure_access_en;
};

/* Provided by <target>/cpucp_config.c. */
const struct cpucp_config *cpucp_get_config(void);
void cpucp_fill_soc_info(void);

/* Shared register/fuse helpers implemented by the common start sequence. */
uint32_t cpucp_read_bits(uintptr_t reg, unsigned int lsb, unsigned int width);
uint32_t cpucp_soft_sku_lval(uint32_t enabled, uint32_t lval);

#endif /* QTI_CPUCP_CONFIG_H */
