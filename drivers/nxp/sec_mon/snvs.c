/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <snvs.h>

static uintptr_t g_nxp_snvs_addr;

void snvs_init(uintptr_t nxp_snvs_addr)
{
	g_nxp_snvs_addr = nxp_snvs_addr;
}

uint32_t get_snvs_state(void)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (g_nxp_snvs_addr);

	return (snvs_read32(&snvs->hp_stat) & HPSTS_MASK_SSM_ST);
}

static uint32_t do_snvs_state_transition(uint32_t state_transtion_bit,
					 uint32_t target_state)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (g_nxp_snvs_addr);
	uint32_t sts = get_snvs_state();
	uint32_t fetch_cnt = 16U;
	uint32_t val = snvs_read32(&snvs->hp_com) | state_transtion_bit;

	snvs_write32(&snvs->hp_com, val);

	/* polling loop till SNVS is in target state */
	do {
		sts = get_snvs_state();
	} while ((sts != target_state) && ((--fetch_cnt) != 0));

	return sts;
}
void transition_snvs_non_secure(void)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (g_nxp_snvs_addr);
	uint32_t sts = get_snvs_state();

	switch (sts) {
		/* If initial state is check or Non-Secure, then
		 * set the Software Security Violation Bit and
		 * transition to Non-Secure State.
		 */
	case HPSTS_CHECK_SSM_ST:
		sts = do_snvs_state_transition(HPCOM_SW_SV, HPSTS_NON_SECURE_SSM_ST);
		break;

		/* If initial state is Trusted, Secure or Soft-Fail, then
		 * first set the Software Security Violation Bit and
		 * transition to Soft-Fail State.
		 */
	case HPSTS_TRUST_SSM_ST:
	case HPSTS_SECURE_SSM_ST:
	case HPSTS_SOFT_FAIL_SSM_ST:
		sts = do_snvs_state_transition(HPCOM_SW_SV, HPSTS_NON_SECURE_SSM_ST);

		/* If SSM Soft Fail to Non-Secure State Transition
		 * Disable is not set, then set SSM_ST bit and
		 * transition to Non-Secure State.
		 */
		if ((snvs_read32(&snvs->hp_com) & HPCOM_SSM_SFNS_DIS) == 0) {
			sts = do_snvs_state_transition(HPCOM_SSM_ST, HPSTS_NON_SECURE_SSM_ST);
		}
		break;
	default:
		break;
	}
}

void transition_snvs_soft_fail(void)
{
	do_snvs_state_transition(HPCOM_SW_FSV, HPSTS_SOFT_FAIL_SSM_ST);
}

uint32_t transition_snvs_trusted(void)
{
	struct snvs_regs *snvs = (struct snvs_regs *) (g_nxp_snvs_addr);
	uint32_t sts = get_snvs_state();

	switch (sts) {
		/* If initial state is check, set the SSM_ST bit to
		 * change the state to trusted.
		 */
	case HPSTS_CHECK_SSM_ST:
		sts = do_snvs_state_transition(HPCOM_SSM_ST, HPSTS_TRUST_SSM_ST);
		break;
		/* If SSM Secure to Trusted State Transition Disable
		 * is not set, then set SSM_ST bit and
		 * transition to Trusted State.
		 */
	case HPSTS_SECURE_SSM_ST:
		if ((snvs_read32(&snvs->hp_com) & HPCOM_SSM_ST_DIS) == 0) {
			sts = do_snvs_state_transition(HPCOM_SSM_ST, HPSTS_TRUST_SSM_ST);
		}
		break;
		/* If initial state is Soft-Fail or Non-Secure, then
		 * transition to Trusted is not Possible.
		 */
	default:
		break;
	}

	return sts;
}

uint32_t transition_snvs_secure(void)
{
	uint32_t sts = get_snvs_state();

	if (sts == HPSTS_SECURE_SSM_ST) {
		return sts;
	}

	if (sts != HPSTS_TRUST_SSM_ST) {
		sts = transition_snvs_trusted();
		if (sts != HPSTS_TRUST_SSM_ST) {
			return sts;
		}
	}

	sts = do_snvs_state_transition(HPCOM_SSM_ST, HPSTS_TRUST_SSM_ST);

	return sts;
}

void snvs_write_lp_gpr_bit(uint32_t offset, uint32_t bit_pos, bool flag_val)
{
	if (flag_val) {
		snvs_write32(g_nxp_snvs_addr + offset,
			     (snvs_read32(g_nxp_snvs_addr + offset))
			     | (1 << bit_pos));
	} else {
		snvs_write32(g_nxp_snvs_addr + offset,
			     (snvs_read32(g_nxp_snvs_addr + offset))
			     & ~(1 << bit_pos));
	}
}

uint32_t snvs_read_lp_gpr_bit(uint32_t offset, uint32_t bit_pos)
{
	return (snvs_read32(g_nxp_snvs_addr + offset) & (1 << bit_pos));
}

void snvs_disable_zeroize_lp_gpr(void)
{
	snvs_write_lp_gpr_bit(NXP_LPCR_OFFSET,
			  NXP_GPR_Z_DIS_BIT,
			  true);
}

#if defined(NXP_NV_SW_MAINT_LAST_EXEC_DATA) && defined(NXP_COINED_BB)
void snvs_write_app_data_bit(uint32_t bit_pos)
{
	snvs_write_lp_gpr_bit(NXP_APP_DATA_LP_GPR_OFFSET,
			      bit_pos,
			      true);
}

uint32_t snvs_read_app_data(void)
{
	return snvs_read32(g_nxp_snvs_addr + NXP_APP_DATA_LP_GPR_OFFSET);
}

uint32_t snvs_read_app_data_bit(uint32_t bit_pos)
{
	uint8_t ret = snvs_read_lp_gpr_bit(NXP_APP_DATA_LP_GPR_OFFSET, bit_pos);

	return ((ret != 0U) ? 1U : 0U);
}

void snvs_clear_app_data(void)
{
	snvs_write32(g_nxp_snvs_addr + NXP_APP_DATA_LP_GPR_OFFSET, 0x0);
}
#endif
