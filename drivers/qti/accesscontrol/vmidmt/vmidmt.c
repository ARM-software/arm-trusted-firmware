/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <vmidmt.h>
#include <vmidmt_hal.h>
#include <vmidmt_internal.h>
#include <vmidmt_target_hwio.h>

#include <qti_interrupt_svc.h>

#define VMIDMT_INSTANCE(xx) ((enum hal_vmidmt_instance)(xx).table)

/* Maximum value for VMIDMT error status bit position.
 * Error status register is 32 bit so position ranges
 * from 0 to 31
 */
#define VMIDMT_ERR_BIT_POS_MAX 31

extern struct hal_vmidmt_port_map g_vmidmt_info_cfg[];

extern const uint8_t g_vmidmt_info_cfg_count;
extern const struct vmidmt_cfg g_vmidmt_cfg[];
extern const struct vmidmt_map g_vmid_map[];
extern const uint32_t g_vmidmt_cfg_count;
extern const uint32_t g_vmid_map_count;

extern struct vmidmt_err_pos_to_hal_map
	vmidmt_err_pos_to_hal_map[ACC_VMIDMT_ERR_INT_STATUS_REG_NUM]
				 [ACC_VMIDMT_ERR_NUM_PER_REG];

/* No access permissions to VMIDMT config registers to any non secure entity */
struct hal_vmidmt_access_config acc_ctl = { { 0xFFFFFFFF }, { 0 } };

/* Interrupt contexts to distigunish type of error in VMIDMT error ISR */
enum vmidmt_err_id { CLT_SEC = 0, CLT_NONSEC, CFG_SEC, CFG_NONSEC };

static struct vmidmt_err_ctx {
	enum vmidmt_err_id id;
	uint32_t addr;
	uint32_t mask;
	uint32_t reg;
} vmidmt_err[] = {
	[CLT_SEC] = {
		.id = CLT_SEC,
		.addr = HWIO_TCSR_SS_VMIDMT_CLIENT_SEC_INTR_ADDR,
		.mask = HWIO_TCSR_SS_VMIDMT_CLIENT_SEC_INTR_RMSK,
		.reg = 0,
	},
	[CLT_NONSEC] = {
		.id = CLT_NONSEC,
		.addr = HWIO_TCSR_SS_VMIDMT_CLIENT_NON_SEC_INTR_ADDR,
		.mask = HWIO_TCSR_SS_VMIDMT_CLIENT_NON_SEC_INTR_RMSK,
		.reg = 0,
	},
	[CFG_SEC] = {
		.id = CFG_SEC,
		.addr = HWIO_TCSR_SS_VMIDMT_CFG_SEC_INTR_ADDR,
		.mask = HWIO_TCSR_SS_VMIDMT_CFG_SEC_INTR_RMSK,
		.reg = 0,
	},
	[CFG_NONSEC] = {
		.id = CFG_NONSEC,
		.addr = HWIO_TCSR_SS_VMIDMT_CFG_NON_SEC_INTR_ADDR,
		.mask = HWIO_TCSR_SS_VMIDMT_CFG_NON_SEC_INTR_RMSK,
		.reg = 0,
	},
};

static struct hal_vmidmt_port_map *get_info_cfg(uint8_t port)
{
	struct hal_vmidmt_port_map *p = g_vmidmt_info_cfg;

	for (size_t i = 0; i < g_vmidmt_info_cfg_count; i++, p++) {
		if (p->port != port)
			continue;

		return p;
	}

	return NULL;
}

static int map_vmid_internal(enum hal_vmidmt_instance master, uint32_t index,
			     const uint32_t *list, uint8_t len, uint32_t vmid,
			     uint32_t mem_type, bool secure)
{
	struct hal_vmidmt_context_config ctx_cfg = { 0 };
	struct hal_vmidmt_bus_attrib bus_attribs = { 0 };
	struct hal_vmidmt_port_map *cfg;
	enum hal_vmidmt_status rc;

	cfg = get_info_cfg(master);
	if (!cfg)
		return -1;

	if (mem_type != ACC_VMIDMT_MEMTYPE_DEFAULT) {
		bus_attribs.e_mtcfg = HAL_VMIDMT_MTCFG_MEMATTR;
		bus_attribs.mem_attr = mem_type;
	}

	if (secure)
		bus_attribs.e_nscfg = HAL_VMIDMT_NSCFG_SECURE;

	ctx_cfg.p_bus_attrib = &bus_attribs;
	ctx_cfg.u_vmid = vmid;

	rc = vmidmt_hal_config_ctx_ext(&cfg->vmidmt_info, index, list, len,
				       &ctx_cfg);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	vmidmt_hal_enable_client(&cfg->vmidmt_info, true);
	vmidmt_hal_enable_client(&cfg->vmidmt_info, false);

	return 0;
}

static int32_t configure_vmids(void)
{
	int ret = 0;
	uint32_t i;

	for (i = 0; i < g_vmid_map_count; i++) {
		if (!g_vmid_map[i].static_cfg)
			continue;

		ret = map_vmid_internal(
			VMIDMT_INSTANCE(g_vmid_map[i]), g_vmid_map[i].index,
			g_vmid_map[i].sid_list, g_vmid_map[i].num_sids,
			g_vmid_map[i].vmid, g_vmid_map[i].memattr, false);
		if (ret)
			goto error;
	}

	return 0;
error:
	ERROR("VMIDMT Config ERROR for g_vmid_map[%d]\n", i);
	return ret;
}

static int config_options_per_master(int index)
{
	struct hal_vmidmt_default_vmid_config vmid_cfg = {
		.b_vmid_private_namespace_enable = false,
		.bypass_vmid = ACC_VMID_NOACCESS,
		.p_access_control = &acc_ctl,
		.p_bypass_bus_attrib = NULL,
		.p_bypass_aux_config = NULL,
	};
	struct hal_vmidmt_default_secure_vmid_config secure_cfg = {
		.secure_extensions = HAL_VMIDMT_SECURE_EXT_DEFAULT,
		.b_glb_addr_space_restricted_acc_enable = 0,
		.p_default_secure_config = &vmid_cfg,
	};
	struct hal_vmidmt_bus_attrib bus_attribs;
	struct hal_vmidmt_port_map *cfg;
	enum hal_vmidmt_status rc;

	cfg = get_info_cfg(VMIDMT_INSTANCE(g_vmidmt_cfg[index]));
	if (!cfg)
		return -1;

	memset(&bus_attribs, 0, sizeof(bus_attribs));

	if (VMIDMT_INSTANCE(g_vmidmt_cfg[index]) == HAL_VMIDMT_DEHR) {
		bus_attribs.e_nscfg = HAL_VMIDMT_NSCFG_SECURE;
		vmid_cfg.p_bypass_bus_attrib = &bus_attribs;
		vmid_cfg.bypass_vmid = ACC_VMID_NOACCESS;
	}

	rc = vmidmt_hal_init(&cfg->vmidmt_info, &secure_cfg, NULL, NULL);
	if (rc != HAL_VMIDMT_NO_ERROR)
		return -1;

	rc = vmidmt_hal_config_ssdt(&cfg->vmidmt_info,
				    g_vmidmt_cfg[index].ssd_table,
				    g_vmidmt_cfg[index].ssd_table_num_elements);
	if (rc != HAL_VMIDMT_NO_ERROR)
		WARN("Vmidmt SSDT configuration failed");

	vmidmt_hal_cfg_err(&cfg->vmidmt_info, false,
			   g_vmidmt_cfg[index].err_opt);

	vmidmt_hal_cfg_err(&cfg->vmidmt_info, true,
			   g_vmidmt_cfg[index].err_opt);

	return 0;
}

static int32_t configure_options(void)
{
	const struct vmidmt_cfg *p = g_vmidmt_cfg;
	int ret = 0;

	for (size_t i = 0; i < g_vmidmt_cfg_count; i++, p++) {
		if (!p->static_cfg)
			continue;

		ret = config_options_per_master(i);
		if (ret)
			return ret;
	}

	return 0;
}

static void log_error(enum hal_vmidmt_instance vmidmt)
{
	struct hal_vmidmt_port_map *cfg = NULL;
	struct hal_vmidmt_error error = { 0 };

	cfg = get_info_cfg(vmidmt);
	if (!cfg) {
		WARN("Unknown Port: 0x%X\n", vmidmt);
		return;
	}

	if (!vmidmt_hal_is_error(&cfg->vmidmt_info, true)) {
		WARN("VMIDMT ERROR: VMIDMT %d discarded error\n", vmidmt);
		return;
	}

	vmidmt_hal_get_error(&cfg->vmidmt_info, true, &error);

	ERROR("Error Flags: 0x%X\n", error.u_error_flags);
	ERROR("Bus Flags: 0x%X\n", error.u_bus_flags);
	ERROR("SSD Idx: 0x%X SID: 0x%X\n", error.u_ssd_index, error.u_sid);
	ERROR("MID: 0x%X AVMID: 0x%X\n", error.u_master_id, error.u_avmid);
	ERROR("ATID: 0x%X ABID: 0x%X APID: 0x%X\n", error.u_atid, error.u_abid,
	      error.u_apid);
	ERROR("Phys Addr: 0x%X 0x%X\n", error.u_physical_address_upper32,
	      error.u_physical_address_lower32);

	vmidmt_hal_clear_error(&cfg->vmidmt_info, true);
}

static void log_errors(uint32_t reg, uint32_t pos)
{
	const struct vmidmt_err_pos_to_hal_map *row =
		vmidmt_err_pos_to_hal_map[reg];

	for (size_t i = 0; i < ACC_VMIDMT_ERR_NUM_PER_REG; i++, row++) {
		if (row->bit_pos != pos)
			continue;

		if (row->vmidmt != HAL_VMIDMT_COUNT)
			log_error(row->vmidmt);

		break;
	}
}

static void *error_handler(uint32_t int_num, void *ctx)
{
	struct vmidmt_err_ctx *err_ctx = (struct vmidmt_err_ctx *)ctx;
	uint32_t err = mmio_read_32(err_ctx->addr) & err_ctx->mask;

	if (err == 0U)
		return ctx;

	ERROR("VMIDMT INT_REG%d: 0x%08X", err_ctx->reg, err);

	for (uint32_t bit = 0U; bit <= VMIDMT_ERR_BIT_POS_MAX; bit++) {
		if (err & BIT(bit))
			log_errors(err_ctx->reg, bit);
	}

	return ctx;
}

static int32_t register_interrupts(void)
{
	const uint32_t enable = 0xFFFFFFFF;
	int ret = 0;

	ret = qti_interrupt_svc_register(QTISECLIB_INT_ID_VMIDMT_ERR_CLT_SEC,
					 error_handler, &vmidmt_err[CLT_SEC]);
	if (ret)
		return ret;

	ret = qti_interrupt_svc_register(QTISECLIB_INT_ID_VMIDMT_ERR_CLT_NONSEC,
					 error_handler,
					 &vmidmt_err[CLT_NONSEC]);
	if (ret)
		goto error3;

	ret = qti_interrupt_svc_register(QTISECLIB_INT_ID_VMIDMT_ERR_CFG_SEC,
					 error_handler, &vmidmt_err[CFG_SEC]);
	if (ret)
		goto error2;

	ret = qti_interrupt_svc_register(QTISECLIB_INT_ID_VMIDMT_ERR_CFG_NONSEC,
					 error_handler,
					 &vmidmt_err[CFG_NONSEC]);
	if (ret)
		goto error1;

	mmio_write_32(HWIO_TCSR_SS_VMIDMT_CFG_SEC_INTR_ENABLE_ADDR, enable);
	mmio_write_32(HWIO_TCSR_SS_VMIDMT_CFG_NON_SEC_INTR_ENABLE_ADDR, enable);
	mmio_write_32(HWIO_TCSR_SS_VMIDMT_CLIENT_SEC_INTR_ENABLE_ADDR, enable);
	mmio_write_32(HWIO_TCSR_SS_VMIDMT_CLIENT_NON_SEC_INTR_ENABLE_ADDR,
		      enable);

	return 0;

error1:
	qti_interrupt_svc_unregister(QTISECLIB_INT_ID_VMIDMT_ERR_CFG_SEC);
error2:
	qti_interrupt_svc_unregister(QTISECLIB_INT_ID_VMIDMT_ERR_CLT_NONSEC);
error3:
	qti_interrupt_svc_unregister(QTISECLIB_INT_ID_VMIDMT_ERR_CLT_SEC);

	return -1;
}

int vmidmt_configure(void)
{
	int rc;

	rc = configure_options();
	if (rc)
		return ACC_ERR_VMIDMT_CFG_FAIL;

	rc = configure_vmids();
	if (rc)
		return ACC_ERR_VMIDMT_CFG_FAIL;

	rc = register_interrupts();
	if (rc)
		return ACC_ERR_VMIDMT_CFG_FAIL;

	return 0;
}
