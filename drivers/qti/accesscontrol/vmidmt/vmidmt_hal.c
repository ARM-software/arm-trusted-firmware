/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <lib/utils_def.h>
#include <vmidmt_hal.h>
#include <vmidmt_hal_hwio.h>

#define HAL_VMIDMT_DEFAULT_CR0 0x00000111
#define HAL_VMIDMT_DEFAULT_CR2 0
#define HAL_VMIDMT_DEFAULT_ACR 0
#define HAL_VMIDMT_DEFAULT_S2VR 0x00010000
#define HAL_VMIDMT_MAX_SSD 4

#define VMIDMT_ALL_VMID_MASK 0xFFFFFFFF
#define VMIDMT_NUM_NSSTATE_BITS 32
#define VMIDMT_MEMATTR_MAX_BITS 3

#define IS_NUM_SSD_VALID(num_ssd) (num_ssd <= HAL_VMIDMT_MAX_SSD)

#define ALL_ERROR_OPTIONS                                               \
	(HAL_VMIDMT_ERROR_O_SMCFCFG_EN | HAL_VMIDMT_ERROR_O_USFCFG_EN | \
	 HAL_VMIDMT_ERROR_O_GCFGFIE | HAL_VMIDMT_ERROR_O_GCFGFRE |      \
	 HAL_VMIDMT_ERROR_O_GFIE)

#define CR0_ERE_MASK                                             \
	(VMIDMT_FMSK(CR0, SMCFCFG) | VMIDMT_FMSK(CR0, USFCFG) |  \
	 VMIDMT_FMSK(CR0, GCFGFIE) | VMIDMT_FMSK(CR0, GCFGFRE) | \
	 VMIDMT_FMSK(CR0, GFIE))

#define CR0_BUS_MASK                                         \
	(VMIDMT_FMSK(CR0, WACFG) | VMIDMT_FMSK(CR0, RACFG) | \
	 VMIDMT_FMSK(CR0, SHCFG) | VMIDMT_FMSK(CR0, MTCFG) | \
	 VMIDMT_FMSK(CR0, MEMATTR) | VMIDMT_FMSK(CR0, TRANSIENTCFG))

static void read_id_registers(struct hal_vmidmt_info *info)
{
	uint32_t reg;

	reg = VMIDMT_IN(info->base_addr, IDR0);
	info->dev_params.stream_match_support = VMIDMT_INFC(reg, IDR0, SMS);
	info->dev_params.num_stream_id_bits = VMIDMT_INFC(reg, IDR0, NUMSIDB);
	info->dev_params.entry_count = VMIDMT_INFC(reg, IDR0, NUMSMRG);
	reg = VMIDMT_IN(info->base_addr, IDR1);
	info->dev_params.num_ssd_index_bits =
		VMIDMT_INFC(reg, IDR1, NUMSSDNDXB);
	reg = VMIDMT_IN(info->base_addr, IDR2);
	info->dev_params.input_addr_size = VMIDMT_INFC(reg, IDR2, IAS);
	reg = VMIDMT_IN(info->base_addr, IDR5);
	info->dev_params.num_vmid = VMIDMT_INFC(reg, IDR5, NVMID);
}

static inline enum hal_vmidmt_access
get_permissions(const struct hal_vmidmt_access_config *cfg, uint32_t vmid)
{
	uint32_t field = vmid % HAL_VMIDMT_PERMS_PER_FIELD;
	uint32_t word = vmid / HAL_VMIDMT_PERMS_PER_FIELD;
	uint32_t shift = field * HAL_VMIDMT_PERM_WIDTH;
	uint32_t perm = cfg->au_vmid_perm[word];

	return (enum hal_vmidmt_access)((perm >> shift) & HAL_VMIDMT_PERM_MASK);
}

static inline bool mask_bit_set(const struct hal_vmidmt_access_config *cfg,
				uint32_t vmid)
{
	uint32_t field = vmid % HAL_VMIDMT_MASKS_PER_FIELD;
	uint32_t word = vmid / HAL_VMIDMT_MASKS_PER_FIELD;
	uint32_t bit = field * HAL_VMIDMT_PERM_WIDTH;
	uint32_t mask = (uint32_t)BIT(bit);

	return (cfg->au_vmid[word] & mask) != 0U;
}

static enum hal_vmidmt_status
configure_acr(uint64_t addr, struct hal_vmidmt_access_config *cfg)
{
	enum hal_vmidmt_access perm;
	uint32_t acr_val = 0;
	uint32_t acr = 0;

	for (size_t vmid = 0; vmid <= HAL_VMIDMT_MAX_VMID; vmid++) {
		if (!mask_bit_set(cfg, vmid))
			continue;

		/* only set bits for full access masters */
		perm = get_permissions(cfg, vmid);
		if (perm == HAL_VMIDMT_FULL_ACCESS)
			acr |= 1 << vmid;
	}

	VMIDMT_OUTM(addr, VMIDMTACR, cfg->au_vmid[0], acr);
	acr_val = VMIDMT_INM(addr, VMIDMTACR, cfg->au_vmid[0]);
	if (acr_val != acr)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static uint32_t set_default_config_bus(struct hal_vmidmt_bus_attrib *bus)
{
	uint32_t attrib = 0;

	if (bus->e_wacfg < HAL_VMIDMT_WACFG_DEFAULT)
		attrib |= bus->e_wacfg << VMIDMT_SHFT(CR0, WACFG);

	if (bus->e_racfg < HAL_VMIDMT_RACFG_DEFAULT)
		attrib |= bus->e_racfg << VMIDMT_SHFT(CR0, RACFG);

	if (bus->e_shcfg < HAL_VMIDMT_SHCFG_DEFAULT)
		attrib |= bus->e_shcfg << VMIDMT_SHFT(CR0, SHCFG);

	if (bus->e_mtcfg < HAL_VMIDMT_MTCFG_DEFAULT)
		attrib |= bus->e_mtcfg << VMIDMT_SHFT(CR0, MTCFG);

	attrib |= bus->mem_attr << VMIDMT_SHFT(CR0, MEMATTR);

	if (bus->e_transient_cfg < HAL_VMIDMT_TRANSIENTCFG_DEFAULT)
		attrib |= bus->e_transient_cfg
			  << VMIDMT_SHFT(CR0, TRANSIENTCFG);

	return attrib;
}

static uint32_t set_default_config_aux(struct hal_vmidmt_aux_config *aux)
{
	uint32_t config = 0;

	if (aux->e_rcnsh < HAL_VMIDMT_RCNSH_DEFAULT)
		config |= aux->e_rcnsh << VMIDMT_SHFT(ACR, BPRCNSH);

	if (aux->e_rcish < HAL_VMIDMT_RCISH_DEFAULT)
		config |= aux->e_rcish << VMIDMT_SHFT(ACR, BPRCISH);

	if (aux->e_rcosh < HAL_VMIDMT_RCOSH_DEFAULT)
		config |= aux->e_rcosh << VMIDMT_SHFT(ACR, BPRCOSH);

	if (aux->e_req_priority_cfg < HAL_VMIDMT_REQPRICFG_DEFAULT)
		config |= aux->e_req_priority_cfg
			  << VMIDMT_SHFT(ACR, REQPRIORITYCFG);

	if (aux->e_req_priority < HAL_VMIDMT_REQPRI_DEFAULT)
		config |= aux->e_req_priority << VMIDMT_SHFT(ACR, REQPRIORITY);

	return config;
}

static enum hal_vmidmt_status
set_default_config(const struct hal_vmidmt_info *p,
		   const struct hal_vmidmt_default_vmid_config *cfg, bool sec)
{
	struct hal_vmidmt_bus_attrib *bypass_bus = cfg->p_bypass_bus_attrib;
	struct hal_vmidmt_aux_config *bypass_aux = cfg->p_bypass_aux_config;
	uint32_t attrib = 0;
	uint32_t config = 0;
	uint32_t val = 0;
	enum hal_vmidmt_status rc = HAL_VMIDMT_NO_ERROR;

	if (!cfg)
		return HAL_VMIDMT_INVALID_PARAM;

	if (bypass_bus &&
	    bypass_bus->mem_attr > (1 << VMIDMT_MEMATTR_MAX_BITS) - 1)
		return HAL_VMIDMT_INVALID_PARAM;

	if (cfg->p_access_control) {
		rc = configure_acr(p->base_addr, cfg->p_access_control);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	}

	if (bypass_bus)
		attrib |= set_default_config_bus(bypass_bus);

	if (bypass_aux)
		config |= set_default_config_aux(bypass_aux);

	if (sec) {
		if (bypass_bus &&
		    bypass_bus->e_nscfg < HAL_VMIDMT_NSCFG_DEFAULT)
			attrib |= bypass_bus->e_nscfg
				  << VMIDMT_SHFT(SCR0, NSCFG);

		VMIDMT_OUTM(p->base_addr, SCR0,
			    (CR0_BUS_MASK | VMIDMT_FMSK(SCR0, NSCFG)), attrib);
		val = VMIDMT_INM(p->base_addr, SCR0,
				 (CR0_BUS_MASK | VMIDMT_FMSK(SCR0, NSCFG)));
		if (val != attrib)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->base_addr, SCR0, VMIDPNE,
			    cfg->b_vmid_private_namespace_enable);
		val = VMIDMT_INF(p->base_addr, SCR0, VMIDPNE);
		if (val != (uint32_t)cfg->b_vmid_private_namespace_enable)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->base_addr, SCR2, BPVMID, cfg->bypass_vmid);
		val = VMIDMT_INF(p->base_addr, SCR2, BPVMID);
		if (val != (uint32_t)cfg->bypass_vmid)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTM(p->base_addr, SACR, VMIDMT_RMSK(SACR), config);
		val = VMIDMT_INM(p->base_addr, SACR, VMIDMT_RMSK(SACR));
		if (val != config)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;
	} else {
		VMIDMT_OUTM(p->base_addr, NSCR0, CR0_BUS_MASK, attrib);
		val = VMIDMT_INM(p->base_addr, NSCR0, CR0_BUS_MASK);
		if (val != attrib)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->base_addr, NSCR0, VMIDPNE,
			    cfg->b_vmid_private_namespace_enable);
		val = VMIDMT_INF(p->base_addr, NSCR0, VMIDPNE);
		if (val != (uint32_t)cfg->b_vmid_private_namespace_enable)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTF(p->base_addr, NSCR2, BPVMID, cfg->bypass_vmid);
		val = VMIDMT_INF(p->base_addr, NSCR2, BPVMID);
		if (val != (uint32_t)cfg->bypass_vmid)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;

		VMIDMT_OUTM(p->base_addr, NSACR, VMIDMT_RMSK(NSACR), config);
		val = VMIDMT_INM(p->base_addr, NSACR, VMIDMT_RMSK(NSACR));
		if (val != config)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;
	}

	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status swap_context(const struct hal_vmidmt_info *info,
					   uint32_t old_idx, uint32_t new_idx)
{
	uint32_t s2vr_msk = VMIDMT_RMSK(S2VRn);
	uint32_t src_val = 0;
	uint32_t dst_val = 0;

	src_val = VMIDMT_INI(info->base_addr, SMRn, old_idx);

	VMIDMT_OUTI(info->base_addr, SMRn, new_idx, src_val);

	dst_val = VMIDMT_INI(info->base_addr, SMRn, new_idx);
	if (dst_val != src_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	src_val = VMIDMT_INMI(info->base_addr, S2VRn, old_idx, s2vr_msk);

	VMIDMT_OUTMI(info->base_addr, S2VRn, new_idx, s2vr_msk, src_val);

	dst_val = VMIDMT_INMI(info->base_addr, S2VRn, new_idx, s2vr_msk);
	if (dst_val != src_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	src_val = VMIDMT_INI(info->base_addr, AS2VRn, old_idx);

	VMIDMT_OUTI(info->base_addr, AS2VRn, new_idx, src_val);

	dst_val = VMIDMT_INI(info->base_addr, AS2VRn, new_idx);
	if (dst_val != src_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status erase_context(const struct hal_vmidmt_info *info,
					    uint32_t ctx_idx)
{
	uint32_t s2vr_mask = VMIDMT_RMSK(S2VRn);
	uint32_t rd_val;

	VMIDMT_OUTI(info->base_addr, SMRn, ctx_idx, 0);

	rd_val = VMIDMT_INI(info->base_addr, SMRn, ctx_idx);
	if (rd_val != 0U)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	VMIDMT_OUTMI(info->base_addr, S2VRn, ctx_idx, s2vr_mask, 0);

	rd_val = VMIDMT_INMI(info->base_addr, S2VRn, ctx_idx, s2vr_mask);
	if (rd_val != 0U)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	VMIDMT_OUTI(info->base_addr, AS2VRn, ctx_idx, 0);

	rd_val = VMIDMT_INI(info->base_addr, AS2VRn, ctx_idx);
	if (rd_val != 0U)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static uint32_t configure_context_bus(const struct hal_vmidmt_bus_attrib *bus)
{
	uint32_t s2vr = 0;

	if (bus->e_nscfg < HAL_VMIDMT_NSCFG_DEFAULT)
		s2vr |= bus->e_nscfg << VMIDMT_SHFT(S2VRn, NSCFG);

	if (bus->e_wacfg < HAL_VMIDMT_WACFG_DEFAULT)
		s2vr |= bus->e_wacfg << VMIDMT_SHFT(S2VRn, WACFG);

	if (bus->e_racfg < HAL_VMIDMT_RACFG_DEFAULT)
		s2vr |= bus->e_racfg << VMIDMT_SHFT(S2VRn, RACFG);

	if (bus->e_shcfg < HAL_VMIDMT_SHCFG_DEFAULT)
		s2vr |= bus->e_shcfg << VMIDMT_SHFT(S2VRn, SHCFG);

	if (bus->e_mtcfg < HAL_VMIDMT_MTCFG_DEFAULT)
		s2vr |= bus->e_mtcfg << VMIDMT_SHFT(S2VRn, MTCFG);

	s2vr |= bus->mem_attr << VMIDMT_SHFT(S2VRn, MEMATTR);

	if (bus->e_transient_cfg < HAL_VMIDMT_TRANSIENTCFG_DEFAULT)
		s2vr |= bus->e_transient_cfg
			<< VMIDMT_SHFT(S2VRn, TRANSIENTCFG);

	return s2vr;
}

static uint32_t configure_context_aux(const struct hal_vmidmt_aux_config *aux)
{
	uint32_t as2vr = 0;

	if (aux->e_rcnsh < HAL_VMIDMT_RCNSH_DEFAULT)
		as2vr |= aux->e_rcnsh << VMIDMT_SHFT(AS2VRn, RCNSH);

	if (aux->e_rcish < HAL_VMIDMT_RCISH_DEFAULT)
		as2vr |= aux->e_rcish << VMIDMT_SHFT(AS2VRn, RCISH);

	if (aux->e_rcosh < HAL_VMIDMT_RCOSH_DEFAULT)
		as2vr |= aux->e_rcosh << VMIDMT_SHFT(AS2VRn, RCOSH);

	if (aux->e_req_priority_cfg < HAL_VMIDMT_REQPRICFG_DEFAULT)
		as2vr |= aux->e_req_priority_cfg
			 << VMIDMT_SHFT(AS2VRn, REQPRIORITYCFG);

	if (aux->e_req_priority < HAL_VMIDMT_REQPRI_DEFAULT)
		as2vr |= aux->e_req_priority
			 << VMIDMT_SHFT(AS2VRn, REQPRIORITY);

	return as2vr;
}

static enum hal_vmidmt_status
configure_context(const struct hal_vmidmt_info *info, uint32_t index,
		  const struct hal_vmidmt_context_config *config)
{
	const struct hal_vmidmt_bus_attrib *bus_attrib = config->p_bus_attrib;
	const struct hal_vmidmt_aux_config *aux_config = config->p_aux_config;
	uint32_t as2vr_val = 0;
	uint32_t s2vr_val = 0;
	uint32_t readback;

	s2vr_val |= config->u_vmid << VMIDMT_SHFT(S2VRn, VMID);

	if (bus_attrib != NULL)
		s2vr_val |= configure_context_bus(bus_attrib);

	VMIDMT_OUTMI(info->base_addr, S2VRn, index, VMIDMT_RMSK(S2VRn),
		     s2vr_val);

	readback =
		VMIDMT_INMI(info->base_addr, S2VRn, index, VMIDMT_RMSK(S2VRn));
	if (readback != s2vr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	if (aux_config != NULL)
		as2vr_val |= configure_context_aux(aux_config);

	VMIDMT_OUTI(info->base_addr, AS2VRn, index, as2vr_val);

	readback = VMIDMT_INI(info->base_addr, AS2VRn, index);
	if (readback != as2vr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

enum hal_vmidmt_status vmidmt_hal_enable_client(const struct hal_vmidmt_info *p,
						bool sec)
{
	uint32_t read_back = 0;

	if (sec) {
		VMIDMT_OUTF(p->base_addr, SCR0, CLIENTPD, read_back);
		read_back = VMIDMT_INF(p->base_addr, SCR0, CLIENTPD);
	} else {
		VMIDMT_OUTF(p->base_addr, NSCR0, CLIENTPD, read_back);
		read_back = VMIDMT_INF(p->base_addr, NSCR0, CLIENTPD);
	}

	return read_back ? HAL_VMIDMT_READ_WRITE_MISMATCH : HAL_VMIDMT_NO_ERROR;
}

enum hal_vmidmt_status
vmidmt_hal_config_ssdt(const struct hal_vmidmt_info *p,
		       const hal_vmidmt_secure_status_det *status,
		       uint32_t elements)
{
	uint32_t length = 0;

	if (!p || !status || !IS_NUM_SSD_VALID(elements))
		return HAL_VMIDMT_INVALID_PARAM;

	length = 1 << p->dev_params.num_ssd_index_bits;

	if (length < VMIDMT_NUM_NSSTATE_BITS) {
		if (elements > 1)
			return HAL_VMIDMT_INVALID_PARAM;
	} else {
		if (elements > length / VMIDMT_NUM_NSSTATE_BITS)
			return HAL_VMIDMT_INVALID_PARAM;
	}

	for (size_t i = 0; i < elements; i++)
		VMIDMT_OUTMI(p->base_addr, SSDRn, i, ~status[i], status[i]);

	return HAL_VMIDMT_NO_ERROR;
}

enum hal_vmidmt_status
vmidmt_hal_init(struct hal_vmidmt_info *info,
		const struct hal_vmidmt_default_secure_vmid_config *sec_cfg,
		const struct hal_vmidmt_default_vmid_config *nsec_cfg,
		char **ver)
{
	uint32_t ssd_words = 0;
	uint32_t ssd_mask_bits = 0;
	uint8_t sec_ext = 0;
	uint32_t scr1_init = 0;
	uint32_t rb_val = 0;
	uint32_t idx;
	enum hal_vmidmt_status st = HAL_VMIDMT_NO_ERROR;

	if (sec_cfg && nsec_cfg)
		return HAL_VMIDMT_INVALID_PARAM;

	read_id_registers(info);

	VMIDMT_OUT(info->base_addr, VMIDMTACR, VMIDMT_ALL_VMID_MASK);

	if (sec_cfg) {
		sec_ext = sec_cfg->secure_extensions;

		if (sec_cfg->secure_extensions ==
		    HAL_VMIDMT_SECURE_EXT_DEFAULT) {
			sec_ext = info->dev_params.entry_count;

		} else if (sec_cfg->secure_extensions >
			   info->dev_params.entry_count) {
			st = HAL_VMIDMT_INVALID_PARAM;
			goto out;
		}

		st = set_default_config(info, sec_cfg->p_default_secure_config,
					true);
		if (st != HAL_VMIDMT_NO_ERROR)
			goto out;

		VMIDMT_OUTF(info->base_addr, SCR1, GASRAE,
			    sec_cfg->b_glb_addr_space_restricted_acc_enable);

		rb_val = VMIDMT_INF(info->base_addr, SCR1, GASRAE);
		if (rb_val != sec_cfg->b_glb_addr_space_restricted_acc_enable) {
			st = HAL_VMIDMT_READ_WRITE_MISMATCH;
			goto out;
		}

		VMIDMT_OUTF(info->base_addr, SCR1, NSNUMSMRGO, sec_ext);

		rb_val = VMIDMT_INF(info->base_addr, SCR1, NSNUMSMRGO);
		if (rb_val != sec_ext) {
			st = HAL_VMIDMT_READ_WRITE_MISMATCH;
			goto out;
		}

		VMIDMT_OUT(info->base_addr, NSCR0, HAL_VMIDMT_DEFAULT_CR0);

		VMIDMT_OUT(info->base_addr, NSCR2, HAL_VMIDMT_DEFAULT_CR2);

		VMIDMT_OUT(info->base_addr, NSACR, HAL_VMIDMT_DEFAULT_ACR);

	} else {
		VMIDMT_OUT(info->base_addr, SCR0, HAL_VMIDMT_DEFAULT_CR0);

		scr1_init = info->dev_params.entry_count
			    << VMIDMT_SHFT(SCR1, NSNUMSMRGO);

		VMIDMT_OUT(info->base_addr, SCR1, scr1_init);

		VMIDMT_OUT(info->base_addr, SCR2, HAL_VMIDMT_DEFAULT_CR2);

		VMIDMT_OUT(info->base_addr, SACR, HAL_VMIDMT_DEFAULT_ACR);

		st = set_default_config(info, nsec_cfg, false);
		if (st != HAL_VMIDMT_NO_ERROR)
			goto out;
	}

	VMIDMT_OUT(info->base_addr, SVMIDMTCR0, 1);
	VMIDMT_OUT(info->base_addr, NSVMIDMTCR0, 1);
	VMIDMT_OUT(info->base_addr, SGFSRRESTORE, 0);
	VMIDMT_OUT(info->base_addr, NSGFSRRESTORE, 0);

	ssd_words = 1;
	ssd_mask_bits = BIT(info->dev_params.num_ssd_index_bits);

	if (ssd_mask_bits >= VMIDMT_NUM_NSSTATE_BITS) {
		ssd_words = ssd_mask_bits / VMIDMT_NUM_NSSTATE_BITS;

		if (!IS_NUM_SSD_VALID(ssd_words)) {
			st = HAL_VMIDMT_INVALID_HW_VALUE;
			goto out;
		}
	}

	for (idx = 0; idx < ssd_words; idx++) {
		VMIDMT_OUTI(info->base_addr, SSDRn, idx, 0xFFFFFFFF);
	}

out:
	for (idx = 0; idx < info->dev_params.entry_count; idx++) {
		if (info->dev_params.stream_match_support)
			VMIDMT_OUTI(info->base_addr, SMRn, idx, 0);

		VMIDMT_OUTI(info->base_addr, S2VRn, idx,
			    HAL_VMIDMT_DEFAULT_S2VR);

		VMIDMT_OUTI(info->base_addr, AS2VRn, idx, 0);
	}

	return st;
}

static enum hal_vmidmt_status
validate_ctx_args(const struct hal_vmidmt_info *info, uint32_t stream,
		  const struct hal_vmidmt_context_config *cfg, bool *secure)
{
	const uint32_t max_attr = (1U << VMIDMT_MEMATTR_MAX_BITS) - 1U;
	uint32_t max_stream_id;

	if (cfg == NULL)
		return HAL_VMIDMT_INVALID_PARAM;

	max_stream_id = (1U << info->dev_params.num_stream_id_bits) - 1U;

	if (stream > max_stream_id && stream >= info->dev_params.entry_count)
		return HAL_VMIDMT_INVALID_PARAM;

	*secure = false;

	if (!cfg->p_bus_attrib)
		return HAL_VMIDMT_NO_ERROR;

	if (cfg->p_bus_attrib->mem_attr > max_attr)
		return HAL_VMIDMT_INVALID_PARAM;

	if (cfg->p_bus_attrib->e_nscfg == HAL_VMIDMT_NSCFG_SECURE)
		*secure = true;

	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status update_ssdrn(const struct hal_vmidmt_info *info,
					   uint32_t stream, bool is_secure)
{
	uint32_t ssd_index;
	uint32_t bit_mask;
	uint32_t desired;
	uint32_t ssdrn_val;

	ssd_index = stream / VMIDMT_NUM_NSSTATE_BITS;
	bit_mask = BIT(stream % VMIDMT_NUM_NSSTATE_BITS);

	/* 0 = secure, 1 = non-secure */
	desired = is_secure ? 0U : bit_mask;

	VMIDMT_OUTMI(info->base_addr, SSDRn, ssd_index, bit_mask, desired);

	ssdrn_val = VMIDMT_INMI(info->base_addr, SSDRn, ssd_index, bit_mask);

	if ((ssdrn_val & bit_mask) != desired)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static int32_t find_matching_smr(const struct hal_vmidmt_info *info,
				 uint32_t entry_count, uint32_t smr_val)
{
	for (int32_t i = 0; i < (int32_t)entry_count; i++) {
		if (VMIDMT_INI(info->base_addr, SMRn, (uint32_t)i) == smr_val)
			return i;
	}

	return -1;
}

static enum hal_vmidmt_status
move_to_ns_tail(const struct hal_vmidmt_info *info, uint32_t ns_smr_count,
		int32_t *idx)
{
	enum hal_vmidmt_status rc = HAL_VMIDMT_NO_ERROR;
	int32_t i = *idx;

	for (; (uint32_t)i < ns_smr_count - 1U; i++) {
		rc = swap_context(info, (uint32_t)i + 1U, (uint32_t)i);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	}

	*idx = i;
	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status
move_to_ns_head(const struct hal_vmidmt_info *info, uint32_t *ns_smr_count,
		int32_t *idx)
{
	enum hal_vmidmt_status rc = HAL_VMIDMT_NO_ERROR;
	uint32_t read_back;
	int32_t i = *idx;

	for (; (uint32_t)i > *ns_smr_count; i--) {
		rc = swap_context(info, (uint32_t)i - 1U, (uint32_t)i);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	}

	*idx = i;
	(*ns_smr_count)++;

	VMIDMT_OUTF(info->base_addr, SCR1, NSNUMSMRGO, *ns_smr_count);

	read_back = VMIDMT_INF(info->base_addr, SCR1, NSNUMSMRGO);
	if (read_back != *ns_smr_count)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status
find_secure_slot(const struct hal_vmidmt_info *info, uint32_t entry_count,
		 uint32_t ns_smr_count, uint32_t stream, int32_t *idx)
{
	int32_t i;

	for (i = (int32_t)entry_count - 1; i >= 0; i--) {
		if (VMIDMT_INFI(info->base_addr, SMRn, (uint32_t)i, VALID) !=
		    1U)
			break;

		if (VMIDMT_INFI(info->base_addr, SMRn, (uint32_t)i, ID) ==
		    stream)
			break;
	}

	if (i < 0 || (uint32_t)i < ns_smr_count - 1U)
		return HAL_VMIDMT_INVALID_PARAM;

	/* found */
	*idx = i;
	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status
find_nonsecure_slot(const struct hal_vmidmt_info *info, uint32_t ns_smr_count,
		    uint32_t stream, int32_t *idx)
{
	int32_t i;

	for (i = 0; (uint32_t)i < ns_smr_count; i++) {
		if (VMIDMT_INFI(info->base_addr, SMRn, (uint32_t)i, VALID) !=
		    1U)
			break;

		if (VMIDMT_INFI(info->base_addr, SMRn, (uint32_t)i, ID) ==
		    stream)
			break;
	}

	if ((uint32_t)i >= ns_smr_count)
		return HAL_VMIDMT_INVALID_PARAM;

	/* found */
	*idx = i;
	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status write_smr(const struct hal_vmidmt_info *info,
					bool is_secure, uint32_t smr_val,
					int32_t idx, uint32_t *ns_smr_count)
{
	uint32_t rd_val;
	uint32_t i = (uint32_t)idx;

	VMIDMT_OUTI(info->base_addr, SMRn, i, smr_val);

	rd_val = VMIDMT_INI(info->base_addr, SMRn, i);
	if (rd_val != smr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	if (is_secure && i < *ns_smr_count) {
		(*ns_smr_count)--;

		VMIDMT_OUTF(info->base_addr, SCR1, NSNUMSMRGO, *ns_smr_count);

		rd_val = VMIDMT_INF(info->base_addr, SCR1, NSNUMSMRGO);
		if (rd_val != *ns_smr_count)
			return HAL_VMIDMT_READ_WRITE_MISMATCH;
	}

	return HAL_VMIDMT_NO_ERROR;
}

static enum hal_vmidmt_status update_smr(const struct hal_vmidmt_info *info,
					 uint32_t stream, bool is_secure,
					 uint32_t *index_out)
{
	enum hal_vmidmt_status rc = HAL_VMIDMT_NO_ERROR;
	uint32_t entry_count = info->dev_params.entry_count;
	uint32_t ns_smr_count;
	uint32_t smr_val = 0U;
	int32_t i;

	ns_smr_count = VMIDMT_INF(info->base_addr, SCR1, NSNUMSMRGO);

	smr_val |= BIT(VMIDMT_SHFT(SMRn, VALID));
	smr_val |= stream << VMIDMT_SHFT(SMRn, ID);

	i = find_matching_smr(info, entry_count, smr_val);

	if (i >= 0) {
		/* Existing SMR entry with same value. */
		if (is_secure && (uint32_t)i >= ns_smr_count - 1U)
			goto write_and_adjust;

		if (!is_secure && (uint32_t)i < ns_smr_count)
			goto write_and_adjust;

		if ((uint32_t)i < ns_smr_count) {
			rc = move_to_ns_tail(info, ns_smr_count, &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		} else {
			rc = move_to_ns_head(info, &ns_smr_count, &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		}

		rc = erase_context(info, (uint32_t)i);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	} else {
		/* No existing SMR entry with this value; find a slot. */
		if (is_secure) {
			rc = find_secure_slot(info, entry_count, ns_smr_count,
					      stream, &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		} else {
			rc = find_nonsecure_slot(info, ns_smr_count, stream,
						 &i);
			if (rc != HAL_VMIDMT_NO_ERROR)
				return rc;
		}
	}

write_and_adjust:
	rc = write_smr(info, is_secure, smr_val, i, &ns_smr_count);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return rc;

	*index_out = (uint32_t)i;

	return HAL_VMIDMT_NO_ERROR;
}

enum hal_vmidmt_status
vmidmt_hal_config_ctx(const struct hal_vmidmt_info *info, uint32_t stream,
		      const struct hal_vmidmt_context_config *cfg)
{
	enum hal_vmidmt_status rc;
	bool secure = false;
	uint32_t ctx_index = 0;

	rc = validate_ctx_args(info, stream, cfg, &secure);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return rc;

	rc = update_ssdrn(info, stream, secure);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return rc;

	if (info->dev_params.stream_match_support) {
		rc = update_smr(info, stream, secure, &ctx_index);
		if (rc != HAL_VMIDMT_NO_ERROR)
			return rc;
	} else {
		ctx_index = 0;
	}

	return configure_context(info, ctx_index, cfg);
}

enum hal_vmidmt_status
vmidmt_hal_config_ctx_ext(const struct hal_vmidmt_info *info,
			  uint32_t smr_index, const uint32_t *stream_list,
			  uint32_t stream_count,
			  const struct hal_vmidmt_context_config *ctx)
{
	const uint32_t max_attr = (1U << VMIDMT_MEMATTR_MAX_BITS) - 1U;
	uint32_t valid_range_mask = 0;
	uint32_t id_or = 0;
	uint32_t mask_or = 0;
	uint8_t sid_bits = 0;
	uint32_t smr_val = 0;
	uint32_t rd_val = 0;
	uint32_t i;

	if (!info || !ctx)
		return HAL_VMIDMT_INVALID_PARAM;

	/* Simple case: single stream → reuse normal API */
	if (stream_count == 1U)
		return vmidmt_hal_config_ctx(info, smr_index, ctx);

	if (smr_index >= info->dev_params.entry_count)
		return HAL_VMIDMT_INVALID_PARAM;

	if (ctx->p_bus_attrib && ctx->p_bus_attrib->mem_attr > max_attr)
		return HAL_VMIDMT_INVALID_PARAM;

	if (!info->dev_params.stream_match_support)
		goto configure_ctx;

	if (!stream_list || stream_count == 0U)
		return HAL_VMIDMT_INVALID_PARAM;

	sid_bits = info->dev_params.num_stream_id_bits;
	if (sid_bits == 0U) {
		valid_range_mask = 1U;
	} else if (sid_bits >= 32U) {
		valid_range_mask = UINT32_MAX;
	} else {
		valid_range_mask = (1U << sid_bits) - 1U;
	}

	for (i = 0; i < stream_count; i++) {
		id_or |= stream_list[i];

		if ((i + 1U) < stream_count)
			mask_or |= (stream_list[i] ^ stream_list[i + 1U]);
	}

	if ((id_or | mask_or) & ~valid_range_mask)
		return HAL_VMIDMT_INVALID_PARAM;

	smr_val = (1UL << VMIDMT_SHFT(SMRn, VALID)) |
		  (mask_or << VMIDMT_SHFT(SMRn, MASK)) |
		  (id_or << VMIDMT_SHFT(SMRn, ID));

	VMIDMT_OUTI(info->base_addr, SMRn, smr_index, smr_val);

	rd_val = VMIDMT_INI(info->base_addr, SMRn, smr_index);

	if (rd_val != smr_val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

configure_ctx:
	return configure_context(info, smr_index, ctx);
}

enum hal_vmidmt_status vmidmt_hal_cfg_err(const struct hal_vmidmt_info *p,
					  bool sec,
					  hal_vmidmt_error_option_config err)
{
	uint32_t val = 0;
	uint32_t raw = 0;

	if (err & ~ALL_ERROR_OPTIONS)
		return HAL_VMIDMT_INVALID_PARAM;

	if ((err & HAL_VMIDMT_ERROR_O_SMCFCFG_EN) &&
	    p->dev_params.stream_match_support)
		val |= BIT(VMIDMT_SHFT(CR0, SMCFCFG));

	if (err & HAL_VMIDMT_ERROR_O_USFCFG_EN)
		val |= BIT(VMIDMT_SHFT(CR0, USFCFG));

	if (err & HAL_VMIDMT_ERROR_O_GCFGFIE)
		val |= BIT(VMIDMT_SHFT(CR0, GCFGFIE));

	if (err & HAL_VMIDMT_ERROR_O_GCFGFRE)
		val |= BIT(VMIDMT_SHFT(CR0, GCFGFRE));

	if (err & HAL_VMIDMT_ERROR_O_GFIE)
		val |= BIT(VMIDMT_SHFT(CR0, GFIE));

	if (sec) {
		VMIDMT_OUTM(p->base_addr, SCR0, CR0_ERE_MASK, val);
		raw = VMIDMT_INM(p->base_addr, SCR0, CR0_ERE_MASK);
	} else {
		VMIDMT_OUTM(p->base_addr, NSCR0, CR0_ERE_MASK, val);
		raw = VMIDMT_INM(p->base_addr, NSCR0, CR0_ERE_MASK);
	}

	if (raw != val)
		return HAL_VMIDMT_READ_WRITE_MISMATCH;

	return HAL_VMIDMT_NO_ERROR;
}

bool vmidmt_hal_is_error(const struct hal_vmidmt_info *p_vmidmt, bool sec)
{
	if (sec)
		return VMIDMT_IN(p_vmidmt->base_addr, SGFSR) != 0;

	return VMIDMT_IN(p_vmidmt->base_addr, NSGFSR) != 0;
}

static void vmidmt_hal_get_error_sec(const struct hal_vmidmt_info *p,
				     struct hal_vmidmt_error *err, uint32_t *sr,
				     uint32_t *s0, uint32_t *s1, uint32_t *s2)
{
	*sr = VMIDMT_IN(p->base_addr, SGFSR);
	err->u_physical_address_lower32 = VMIDMT_IN(p->base_addr, SGFAR0);

	if (p->dev_params.input_addr_size)
		err->u_physical_address_upper32 =
			VMIDMT_IN(p->base_addr, SGFAR1);

	*s0 = VMIDMT_IN(p->base_addr, SGFSYNDR0);
	*s1 = VMIDMT_IN(p->base_addr, SGFSYNDR1);
	*s2 = VMIDMT_IN(p->base_addr, SGFSYNDR2);
}

static void vmidmt_hal_get_error_nsec(const struct hal_vmidmt_info *p,
				      struct hal_vmidmt_error *err,
				      uint32_t *sr, uint32_t *s0, uint32_t *s1,
				      uint32_t *s2)
{
	*sr = VMIDMT_IN(p->base_addr, NSGFSR);

	err->u_physical_address_lower32 = VMIDMT_IN(p->base_addr, NSGFAR0);

	if (p->dev_params.input_addr_size)
		err->u_physical_address_upper32 =
			VMIDMT_IN(p->base_addr, NSGFAR1);

	*s0 = VMIDMT_IN(p->base_addr, NSGFSYNDR0);

	*s1 = VMIDMT_IN(p->base_addr, NSGFSYNDR1);

	*s2 = VMIDMT_IN(p->base_addr, NSGFSYNDR2);
}

void vmidmt_hal_get_error(const struct hal_vmidmt_info *p, bool sec,
			  struct hal_vmidmt_error *err)
{
	uint32_t s0, s1, s2, sr;

	if (!err)
		return;

	memset(err, 0, sizeof(*err));

	if (sec) {
		vmidmt_hal_get_error_sec(p, err, &sr, &s0, &s1, &s2);
	} else {
		vmidmt_hal_get_error_nsec(p, err, &sr, &s0, &s1, &s2);
	}

	if (sr & VMIDMT_FMSK(GFSR, CLMULTI))
		err->u_error_flags |= HAL_VMIDMT_ERROR_F_CLMULTI;

	if (sr & VMIDMT_FMSK(GFSR, CFGMULTI))
		err->u_error_flags |= HAL_VMIDMT_ERROR_F_CFGMULTI;

	if (sr & VMIDMT_FMSK(GFSR, PF))
		err->u_error_flags |= HAL_VMIDMT_ERROR_F_PF;

	if (sr & VMIDMT_FMSK(GFSR, CAF))
		err->u_error_flags |= HAL_VMIDMT_ERROR_F_CAF;

	if (sr & VMIDMT_FMSK(GFSR, SMCF))
		err->u_error_flags |= HAL_VMIDMT_ERROR_F_SMCF;

	if (sr & VMIDMT_FMSK(GFSR, USF))
		err->u_error_flags |= HAL_VMIDMT_ERROR_F_USF;

	if (s0 & VMIDMT_FMSK(GFSYNDR0, NSATTR))
		err->u_bus_flags |= HAL_VMIDMT_BUS_F_ERROR_NSATTR;

	if (s0 & VMIDMT_FMSK(GFSYNDR0, NSSTATE))
		err->u_bus_flags |= HAL_VMIDMT_BUS_F_ERROR_NSSTATE;

	if (s0 & VMIDMT_FMSK(GFSYNDR0, WNR))
		err->u_bus_flags |= HAL_VMIDMT_BUS_F_ERROR_WNR;

	err->u_ssd_index = VMIDMT_INFC(s1, GFSYNDR1, SSD_INDEX);

	err->u_master_id = VMIDMT_INFC(s2, GFSYNDR2, AMID);

	err->u_avmid = VMIDMT_INFC(s2, GFSYNDR2, AVMID);

	err->u_atid = VMIDMT_INFC(s2, GFSYNDR2, ATID);

	err->u_abid = VMIDMT_INFC(s2, GFSYNDR2, ABID);

	err->u_apid = VMIDMT_INFC(s2, GFSYNDR2, APID);

	err->u_sid = VMIDMT_INFC(s1, GFSYNDR1, SID);
}

enum hal_vmidmt_status vmidmt_hal_clear_error(const struct hal_vmidmt_info *p,
					      bool sec)
{
	uint32_t read_back = 0;

	if (sec) {
		VMIDMT_OUT(p->base_addr, SGFSRRESTORE, 0);

		read_back = VMIDMT_IN(p->base_addr, SGFSRRESTORE);
	} else {
		VMIDMT_OUT(p->base_addr, NSGFSRRESTORE, 0);

		read_back = VMIDMT_IN(p->base_addr, NSGFSRRESTORE);
	}

	return read_back ? HAL_VMIDMT_READ_WRITE_MISMATCH : HAL_VMIDMT_NO_ERROR;
}
