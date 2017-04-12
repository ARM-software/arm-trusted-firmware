/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <memctrl_v2.h>
#include <platform_def.h>
#include <smmu.h>
#include <string.h>
#include <tegra_private.h>

typedef struct smmu_regs {
	uint32_t reg;
	uint32_t val;
} smmu_regs_t;

#define mc_make_sid_override_cfg(name) \
	{ \
		.reg = TEGRA_MC_STREAMID_BASE + MC_STREAMID_OVERRIDE_CFG_ ## name, \
		.val = 0x00000000, \
	}

#define mc_make_sid_security_cfg(name) \
	{ \
		.reg = TEGRA_MC_STREAMID_BASE + \
		       MC_STREAMID_OVERRIDE_TO_SECURITY_CFG( \
				MC_STREAMID_OVERRIDE_CFG_ ## name), \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr0_sec_cfg(name) \
	{ \
		.reg = TEGRA_SMMU_BASE + SMMU_GNSR0_ ## name, \
		.val = 0x00000000, \
	}

/*
 * On ARM-SMMU, conditional offset to access secure aliases of non-secure registers
 * is 0x400. So, add it to register address
 */
#define smmu_make_gnsr0_nsec_cfg(name) \
	{ \
		.reg = TEGRA_SMMU_BASE + 0x400 + SMMU_GNSR0_ ## name, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr0_smr_cfg(n) \
	{ \
		.reg = TEGRA_SMMU_BASE + SMMU_GNSR0_SMR ## n, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr0_s2cr_cfg(n) \
	{ \
		.reg = TEGRA_SMMU_BASE + SMMU_GNSR0_S2CR ## n, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr1_cbar_cfg(n) \
	{ \
		.reg = TEGRA_SMMU_BASE + (1 << PGSHIFT) + SMMU_GNSR1_CBAR ## n, \
		.val = 0x00000000, \
	}

#define smmu_make_gnsr1_cba2r_cfg(n) \
	{ \
		.reg = TEGRA_SMMU_BASE + (1 << PGSHIFT) + SMMU_GNSR1_CBA2R ## n, \
		.val = 0x00000000, \
	}

#define make_smmu_cb_cfg(name, n) \
	{ \
		.reg = TEGRA_SMMU_BASE + (CB_SIZE >> 1) + (n * (1 << PGSHIFT)) \
			+ SMMU_CBn_ ## name, \
		.val = 0x00000000, \
	}

#define smmu_make_smrg_group(n)	\
	smmu_make_gnsr0_smr_cfg(n),	\
	smmu_make_gnsr0_s2cr_cfg(n),	\
	smmu_make_gnsr1_cbar_cfg(n),	\
	smmu_make_gnsr1_cba2r_cfg(n)	/* don't put "," here. */

#define smmu_make_cb_group(n)		\
	make_smmu_cb_cfg(SCTLR, n),	\
	make_smmu_cb_cfg(TCR2, n),	\
	make_smmu_cb_cfg(TTBR0_LO, n),	\
	make_smmu_cb_cfg(TTBR0_HI, n),	\
	make_smmu_cb_cfg(TCR, n),	\
	make_smmu_cb_cfg(PRRR_MAIR0, n),\
	make_smmu_cb_cfg(FSR, n),	\
	make_smmu_cb_cfg(FAR_LO, n),	\
	make_smmu_cb_cfg(FAR_HI, n),	\
	make_smmu_cb_cfg(FSYNR0, n)	/* don't put "," here. */

#define smmu_bypass_cfg \
	{ \
		.reg = TEGRA_MC_BASE + MC_SMMU_BYPASS_CONFIG, \
		.val = 0x00000000, \
	}

#define _START_OF_TABLE_ \
	{ \
		.reg = 0xCAFE05C7, \
		.val = 0x00000000, \
	}

#define _END_OF_TABLE_ \
	{ \
		.reg = 0xFFFFFFFF, \
		.val = 0xFFFFFFFF, \
	}

static __attribute__((aligned(16))) smmu_regs_t smmu_ctx_regs[] = {
	_START_OF_TABLE_,
	mc_make_sid_security_cfg(SCEW),
	mc_make_sid_security_cfg(AFIR),
	mc_make_sid_security_cfg(NVDISPLAYR1),
	mc_make_sid_security_cfg(XUSB_DEVR),
	mc_make_sid_security_cfg(VICSRD1),
	mc_make_sid_security_cfg(NVENCSWR),
	mc_make_sid_security_cfg(TSECSRDB),
	mc_make_sid_security_cfg(AXISW),
	mc_make_sid_security_cfg(SDMMCWAB),
	mc_make_sid_security_cfg(AONDMAW),
	mc_make_sid_security_cfg(GPUSWR2),
	mc_make_sid_security_cfg(SATAW),
	mc_make_sid_security_cfg(UFSHCW),
	mc_make_sid_security_cfg(AFIW),
	mc_make_sid_security_cfg(SDMMCR),
	mc_make_sid_security_cfg(SCEDMAW),
	mc_make_sid_security_cfg(UFSHCR),
	mc_make_sid_security_cfg(SDMMCWAA),
	mc_make_sid_security_cfg(APEDMAW),
	mc_make_sid_security_cfg(SESWR),
	mc_make_sid_security_cfg(MPCORER),
	mc_make_sid_security_cfg(PTCR),
	mc_make_sid_security_cfg(BPMPW),
	mc_make_sid_security_cfg(ETRW),
	mc_make_sid_security_cfg(GPUSRD),
	mc_make_sid_security_cfg(VICSWR),
	mc_make_sid_security_cfg(SCEDMAR),
	mc_make_sid_security_cfg(HDAW),
	mc_make_sid_security_cfg(ISPWA),
	mc_make_sid_security_cfg(EQOSW),
	mc_make_sid_security_cfg(XUSB_HOSTW),
	mc_make_sid_security_cfg(TSECSWR),
	mc_make_sid_security_cfg(SDMMCRAA),
	mc_make_sid_security_cfg(APER),
	mc_make_sid_security_cfg(VIW),
	mc_make_sid_security_cfg(APEW),
	mc_make_sid_security_cfg(AXISR),
	mc_make_sid_security_cfg(SDMMCW),
	mc_make_sid_security_cfg(BPMPDMAW),
	mc_make_sid_security_cfg(ISPRA),
	mc_make_sid_security_cfg(NVDECSWR),
	mc_make_sid_security_cfg(XUSB_DEVW),
	mc_make_sid_security_cfg(NVDECSRD),
	mc_make_sid_security_cfg(MPCOREW),
	mc_make_sid_security_cfg(NVDISPLAYR),
	mc_make_sid_security_cfg(BPMPDMAR),
	mc_make_sid_security_cfg(NVJPGSWR),
	mc_make_sid_security_cfg(NVDECSRD1),
	mc_make_sid_security_cfg(TSECSRD),
	mc_make_sid_security_cfg(NVJPGSRD),
	mc_make_sid_security_cfg(SDMMCWA),
	mc_make_sid_security_cfg(SCER),
	mc_make_sid_security_cfg(XUSB_HOSTR),
	mc_make_sid_security_cfg(VICSRD),
	mc_make_sid_security_cfg(AONDMAR),
	mc_make_sid_security_cfg(AONW),
	mc_make_sid_security_cfg(SDMMCRA),
	mc_make_sid_security_cfg(HOST1XDMAR),
	mc_make_sid_security_cfg(EQOSR),
	mc_make_sid_security_cfg(SATAR),
	mc_make_sid_security_cfg(BPMPR),
	mc_make_sid_security_cfg(HDAR),
	mc_make_sid_security_cfg(SDMMCRAB),
	mc_make_sid_security_cfg(ETRR),
	mc_make_sid_security_cfg(AONR),
	mc_make_sid_security_cfg(APEDMAR),
	mc_make_sid_security_cfg(SESRD),
	mc_make_sid_security_cfg(NVENCSRD),
	mc_make_sid_security_cfg(GPUSWR),
	mc_make_sid_security_cfg(TSECSWRB),
	mc_make_sid_security_cfg(ISPWB),
	mc_make_sid_security_cfg(GPUSRD2),
	mc_make_sid_override_cfg(APER),
	mc_make_sid_override_cfg(VICSRD),
	mc_make_sid_override_cfg(NVENCSRD),
	mc_make_sid_override_cfg(NVJPGSWR),
	mc_make_sid_override_cfg(AONW),
	mc_make_sid_override_cfg(BPMPR),
	mc_make_sid_override_cfg(BPMPW),
	mc_make_sid_override_cfg(HDAW),
	mc_make_sid_override_cfg(NVDISPLAYR1),
	mc_make_sid_override_cfg(APEDMAR),
	mc_make_sid_override_cfg(AFIR),
	mc_make_sid_override_cfg(AXISR),
	mc_make_sid_override_cfg(VICSRD1),
	mc_make_sid_override_cfg(TSECSRD),
	mc_make_sid_override_cfg(BPMPDMAW),
	mc_make_sid_override_cfg(MPCOREW),
	mc_make_sid_override_cfg(XUSB_HOSTR),
	mc_make_sid_override_cfg(GPUSWR),
	mc_make_sid_override_cfg(XUSB_DEVR),
	mc_make_sid_override_cfg(UFSHCW),
	mc_make_sid_override_cfg(XUSB_HOSTW),
	mc_make_sid_override_cfg(SDMMCWAB),
	mc_make_sid_override_cfg(SATAW),
	mc_make_sid_override_cfg(SCEDMAR),
	mc_make_sid_override_cfg(HOST1XDMAR),
	mc_make_sid_override_cfg(SDMMCWA),
	mc_make_sid_override_cfg(APEDMAW),
	mc_make_sid_override_cfg(SESWR),
	mc_make_sid_override_cfg(AXISW),
	mc_make_sid_override_cfg(AONDMAW),
	mc_make_sid_override_cfg(TSECSWRB),
	mc_make_sid_override_cfg(MPCORER),
	mc_make_sid_override_cfg(ISPWB),
	mc_make_sid_override_cfg(AONR),
	mc_make_sid_override_cfg(BPMPDMAR),
	mc_make_sid_override_cfg(HDAR),
	mc_make_sid_override_cfg(SDMMCRA),
	mc_make_sid_override_cfg(ETRW),
	mc_make_sid_override_cfg(GPUSWR2),
	mc_make_sid_override_cfg(EQOSR),
	mc_make_sid_override_cfg(TSECSWR),
	mc_make_sid_override_cfg(ETRR),
	mc_make_sid_override_cfg(NVDECSRD),
	mc_make_sid_override_cfg(TSECSRDB),
	mc_make_sid_override_cfg(SDMMCRAA),
	mc_make_sid_override_cfg(NVDECSRD1),
	mc_make_sid_override_cfg(SDMMCR),
	mc_make_sid_override_cfg(NVJPGSRD),
	mc_make_sid_override_cfg(SCEDMAW),
	mc_make_sid_override_cfg(SDMMCWAA),
	mc_make_sid_override_cfg(APEW),
	mc_make_sid_override_cfg(AONDMAR),
	mc_make_sid_override_cfg(PTCR),
	mc_make_sid_override_cfg(SCER),
	mc_make_sid_override_cfg(ISPRA),
	mc_make_sid_override_cfg(ISPWA),
	mc_make_sid_override_cfg(VICSWR),
	mc_make_sid_override_cfg(SESRD),
	mc_make_sid_override_cfg(SDMMCW),
	mc_make_sid_override_cfg(SDMMCRAB),
	mc_make_sid_override_cfg(EQOSW),
	mc_make_sid_override_cfg(GPUSRD2),
	mc_make_sid_override_cfg(SCEW),
	mc_make_sid_override_cfg(GPUSRD),
	mc_make_sid_override_cfg(NVDECSWR),
	mc_make_sid_override_cfg(XUSB_DEVW),
	mc_make_sid_override_cfg(SATAR),
	mc_make_sid_override_cfg(NVDISPLAYR),
	mc_make_sid_override_cfg(VIW),
	mc_make_sid_override_cfg(UFSHCR),
	mc_make_sid_override_cfg(NVENCSWR),
	mc_make_sid_override_cfg(AFIW),
	smmu_make_gnsr0_nsec_cfg(CR0),
	smmu_make_gnsr0_sec_cfg(IDR0),
	smmu_make_gnsr0_sec_cfg(IDR1),
	smmu_make_gnsr0_sec_cfg(IDR2),
	smmu_make_gnsr0_nsec_cfg(GFSR),
	smmu_make_gnsr0_nsec_cfg(GFSYNR0),
	smmu_make_gnsr0_nsec_cfg(GFSYNR1),
	smmu_make_gnsr0_nsec_cfg(TLBGSTATUS),
	smmu_make_gnsr0_nsec_cfg(PIDR2),
	smmu_make_smrg_group(0),
	smmu_make_smrg_group(1),
	smmu_make_smrg_group(2),
	smmu_make_smrg_group(3),
	smmu_make_smrg_group(4),
	smmu_make_smrg_group(5),
	smmu_make_smrg_group(6),
	smmu_make_smrg_group(7),
	smmu_make_smrg_group(8),
	smmu_make_smrg_group(9),
	smmu_make_smrg_group(10),
	smmu_make_smrg_group(11),
	smmu_make_smrg_group(12),
	smmu_make_smrg_group(13),
	smmu_make_smrg_group(14),
	smmu_make_smrg_group(15),
	smmu_make_smrg_group(16),
	smmu_make_smrg_group(17),
	smmu_make_smrg_group(18),
	smmu_make_smrg_group(19),
	smmu_make_smrg_group(20),
	smmu_make_smrg_group(21),
	smmu_make_smrg_group(22),
	smmu_make_smrg_group(23),
	smmu_make_smrg_group(24),
	smmu_make_smrg_group(25),
	smmu_make_smrg_group(26),
	smmu_make_smrg_group(27),
	smmu_make_smrg_group(28),
	smmu_make_smrg_group(29),
	smmu_make_smrg_group(30),
	smmu_make_smrg_group(31),
	smmu_make_smrg_group(32),
	smmu_make_smrg_group(33),
	smmu_make_smrg_group(34),
	smmu_make_smrg_group(35),
	smmu_make_smrg_group(36),
	smmu_make_smrg_group(37),
	smmu_make_smrg_group(38),
	smmu_make_smrg_group(39),
	smmu_make_smrg_group(40),
	smmu_make_smrg_group(41),
	smmu_make_smrg_group(42),
	smmu_make_smrg_group(43),
	smmu_make_smrg_group(44),
	smmu_make_smrg_group(45),
	smmu_make_smrg_group(46),
	smmu_make_smrg_group(47),
	smmu_make_smrg_group(48),
	smmu_make_smrg_group(49),
	smmu_make_smrg_group(50),
	smmu_make_smrg_group(51),
	smmu_make_smrg_group(52),
	smmu_make_smrg_group(53),
	smmu_make_smrg_group(54),
	smmu_make_smrg_group(55),
	smmu_make_smrg_group(56),
	smmu_make_smrg_group(57),
	smmu_make_smrg_group(58),
	smmu_make_smrg_group(59),
	smmu_make_smrg_group(60),
	smmu_make_smrg_group(61),
	smmu_make_smrg_group(62),
	smmu_make_smrg_group(63),
	smmu_make_cb_group(0),
	smmu_make_cb_group(1),
	smmu_make_cb_group(2),
	smmu_make_cb_group(3),
	smmu_make_cb_group(4),
	smmu_make_cb_group(5),
	smmu_make_cb_group(6),
	smmu_make_cb_group(7),
	smmu_make_cb_group(8),
	smmu_make_cb_group(9),
	smmu_make_cb_group(10),
	smmu_make_cb_group(11),
	smmu_make_cb_group(12),
	smmu_make_cb_group(13),
	smmu_make_cb_group(14),
	smmu_make_cb_group(15),
	smmu_make_cb_group(16),
	smmu_make_cb_group(17),
	smmu_make_cb_group(18),
	smmu_make_cb_group(19),
	smmu_make_cb_group(20),
	smmu_make_cb_group(21),
	smmu_make_cb_group(22),
	smmu_make_cb_group(23),
	smmu_make_cb_group(24),
	smmu_make_cb_group(25),
	smmu_make_cb_group(26),
	smmu_make_cb_group(27),
	smmu_make_cb_group(28),
	smmu_make_cb_group(29),
	smmu_make_cb_group(30),
	smmu_make_cb_group(31),
	smmu_make_cb_group(32),
	smmu_make_cb_group(33),
	smmu_make_cb_group(34),
	smmu_make_cb_group(35),
	smmu_make_cb_group(36),
	smmu_make_cb_group(37),
	smmu_make_cb_group(38),
	smmu_make_cb_group(39),
	smmu_make_cb_group(40),
	smmu_make_cb_group(41),
	smmu_make_cb_group(42),
	smmu_make_cb_group(43),
	smmu_make_cb_group(44),
	smmu_make_cb_group(45),
	smmu_make_cb_group(46),
	smmu_make_cb_group(47),
	smmu_make_cb_group(48),
	smmu_make_cb_group(49),
	smmu_make_cb_group(50),
	smmu_make_cb_group(51),
	smmu_make_cb_group(52),
	smmu_make_cb_group(53),
	smmu_make_cb_group(54),
	smmu_make_cb_group(55),
	smmu_make_cb_group(56),
	smmu_make_cb_group(57),
	smmu_make_cb_group(58),
	smmu_make_cb_group(59),
	smmu_make_cb_group(60),
	smmu_make_cb_group(61),
	smmu_make_cb_group(62),
	smmu_make_cb_group(63),
	smmu_bypass_cfg,	/* TBU settings */
	_END_OF_TABLE_,
};

/*
 * Save SMMU settings before "System Suspend" to TZDRAM
 */
void tegra_smmu_save_context(uint64_t smmu_ctx_addr)
{
	uint32_t i;
#if DEBUG
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t tzdram_base = params_from_bl2->tzdram_base;
	uint64_t tzdram_end = tzdram_base + params_from_bl2->tzdram_size;
	uint32_t reg_id1, pgshift, cb_size;

	/* sanity check SMMU settings c*/
	reg_id1 = mmio_read_32((TEGRA_SMMU_BASE + SMMU_GNSR0_IDR1));
	pgshift = (reg_id1 & ID1_PAGESIZE) ? 16 : 12;
	cb_size = (2 << pgshift) * \
	(1 << (((reg_id1 >> ID1_NUMPAGENDXB_SHIFT) & ID1_NUMPAGENDXB_MASK) + 1));

	assert(!((pgshift != PGSHIFT) || (cb_size != CB_SIZE)));
#endif

	assert((smmu_ctx_addr >= tzdram_base) && (smmu_ctx_addr <= tzdram_end));

	/* index of _END_OF_TABLE_ */
	smmu_ctx_regs[0].val = ARRAY_SIZE(smmu_ctx_regs) - 1;

	/* save SMMU register values */
	for (i = 1; i < ARRAY_SIZE(smmu_ctx_regs) - 1; i++)
		smmu_ctx_regs[i].val = mmio_read_32(smmu_ctx_regs[i].reg);

	/* Save SMMU config settings */
	memcpy16((void *)(uintptr_t)smmu_ctx_addr, (void *)smmu_ctx_regs,
		 sizeof(smmu_ctx_regs));

	/* save the SMMU table address */
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV11_LO,
		(uint32_t)smmu_ctx_addr);
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV11_HI,
		(uint32_t)(smmu_ctx_addr >> 32));
}

#define SMMU_NUM_CONTEXTS		64
#define SMMU_CONTEXT_BANK_MAX_IDX	64

/*
 * Init SMMU during boot or "System Suspend" exit
 */
void tegra_smmu_init(void)
{
	uint32_t val, i, ctx_base;

	/* Program the SMMU pagesize and reset CACHE_LOCK bit */
	val = tegra_smmu_read_32(SMMU_GSR0_SECURE_ACR);
	val |= SMMU_GSR0_PGSIZE_64K;
	val &= ~SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
	tegra_smmu_write_32(SMMU_GSR0_SECURE_ACR, val);

	/* reset CACHE LOCK bit for NS Aux. Config. Register */
	val = tegra_smmu_read_32(SMMU_GNSR_ACR);
	val &= ~SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
	tegra_smmu_write_32(SMMU_GNSR_ACR, val);

	/* disable TCU prefetch for all contexts */
	ctx_base = (SMMU_GSR0_PGSIZE_64K * SMMU_NUM_CONTEXTS) + SMMU_CBn_ACTLR;
	for (i = 0; i < SMMU_CONTEXT_BANK_MAX_IDX; i++) {
		val = tegra_smmu_read_32(ctx_base + (SMMU_GSR0_PGSIZE_64K * i));
		val &= ~SMMU_CBn_ACTLR_CPRE_BIT;
		tegra_smmu_write_32(ctx_base + (SMMU_GSR0_PGSIZE_64K * i), val);
	}

	/* set CACHE LOCK bit for NS Aux. Config. Register */
	val = tegra_smmu_read_32(SMMU_GNSR_ACR);
	val |= SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
	tegra_smmu_write_32(SMMU_GNSR_ACR, val);

	/* set CACHE LOCK bit for S Aux. Config. Register */
	val = tegra_smmu_read_32(SMMU_GSR0_SECURE_ACR);
	val |= SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
	tegra_smmu_write_32(SMMU_GSR0_SECURE_ACR, val);
}
