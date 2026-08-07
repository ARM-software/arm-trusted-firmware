/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VMIDMT_HAL_H
#define VMIDMT_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/utils_def.h>

#define VMIDMT_INTERFACE_V1 0

#define HAL_VMIDMT_MAX_VMID 31
#define HAL_VMIDMT_SECURE_EXT_DEFAULT 0xFF
#define HAL_VMIDMT_ALL_ERROR_STATUS_BITS 0x7FFFFFFE
#define HAL_VMIDMT_VMID_ENABLE_BIT 1
#define HAL_VMIDMT_PERM_MASK 0x1
#define HAL_VMIDMT_PERM_WIDTH 1
#define HAL_VMIDMT_MASKS_PER_FIELD 32
#define HAL_VMIDMT_PERMS_PER_FIELD 32

#define HAL_VMIDMT_MASTER_MASK_WORDS \
	((HAL_VMIDMT_MAX_VMID / HAL_VMIDMT_MASKS_PER_FIELD) + 1)

#define HAL_VMIDMT_MASTER_PERM_WORDS \
	((HAL_VMIDMT_MAX_VMID / HAL_VMIDMT_PERMS_PER_FIELD) + 1)

enum hal_vmidmt_status {
	HAL_VMIDMT_NO_ERROR, /* successful operation */
	HAL_VMIDMT_INVALID_INSTANCE, /* out of range VMIDMT instance */
	HAL_VMIDMT_UNSUPPORTED_INSTANCE_FOR_TARGET,
	/* unsupported VMIDMT instance for the current target */
	HAL_VMIDMT_UNSUPPORTED_HANDLER, /* unsupported handler */
	HAL_VMIDMT_INVALID_BASE_ADDR, /* invalid VMIDMT instance base address */
	HAL_VMIDMT_INVALID_PARAM, /* invalid passed in parameter */
	HAL_VMIDMT_INVALID_HW_VALUE, /* invalid HW generic value */
	HAL_VMIDMT_READ_WRITE_MISMATCH /* read-after-write values not match */
};

enum hal_vmidmt_instance {
	HAL_VMIDMT_CRYPTO0_AXI = 0, /* CRYPTO AXI - 0 */
	HAL_VMIDMT_CRYPTO1_AXI, /* CRYPTO AXI - 1 */
	HAL_VMIDMT_CRYPTO0_BAM, /* CRYPTO BAM - 0 */
	HAL_VMIDMT_CRYPTO0_CRYPTO = HAL_VMIDMT_CRYPTO0_BAM,
	HAL_VMIDMT_CRYPTO1_BAM, /* CRYPTO BAM - 1 */
	HAL_VMIDMT_DEHR, /* DEHR */
	HAL_VMIDMT_DEHR_BIMC_WRAPPER = HAL_VMIDMT_DEHR,
	HAL_VMIDMT_LPASS_DM, /* LPASS DM */
	HAL_VMIDMT_LPASS_LPAIF, /* LPASS LPAIF */
	HAL_VMIDMT_LPASS_MIDI, /* LPASS MIDI */
	HAL_VMIDMT_LPASS_Q6AHB, /* LPASS Q6AHB */
	HAL_VMIDMT_LPASS_Q6AXI, /* LPASS Q6AXI */
	HAL_VMIDMT_LPASS_RESAMPLER, /* LPASS Resampler */
	HAL_VMIDMT_LPASS_SLIMBUS, /* LPASS Slimbus */
	HAL_VMIDMT_CAMERA_SS, /* CAMERA Subsystem */
	HAL_VMIDMT_CAMSS_VBIF_JPEG, /* CAMERA VBIF JPEG */
	HAL_VMIDMT_CAMSS_VBIF_VFE, /* CAMERA VBIF VFE */
	HAL_VMIDMT_MMSS_DDR, /* MMSS DDR */
	HAL_VMIDMT_OCMEM, /* OCMEM */
	HAL_VMIDMT_OXILI, /* OXILI */
	HAL_VMIDMT_VENUS_CPUSS, /* Venus CPU SS */
	HAL_VMIDMT_VENUS_VBIF, /* Venus VBIF */
	HAL_VMIDMT_MSS_A2BAM, /* MSS A2BAM */
	HAL_VMIDMT_MSS_NAV_CE, /* MSS NAV CE */
	HAL_VMIDMT_MSS_Q6, /* MSS Q6 */
	HAL_VMIDMT_BAM_DMA, /* BAM DMA */
	HAL_VMIDMT_BAM_BLSP1_DMA, /* BAM BLSP DMA - 1 */
	HAL_VMIDMT_BLSP1_BLSP_BAM = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_QUPV3_0 = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_QUPV3_WEST = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_BAM_BLSP2_DMA, /* BAM BLSP DMA - 2 */
	HAL_VMIDMT_BLSP2_BLSP_BAM = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_QUPV3_1 = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_QUPV3_EAST1 = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_BAM_SDCC1, /* BAM SDCC1 */
	HAL_VMIDMT_BAM_SDCC2, /* BAM SDCC2 */
	HAL_VMIDMT_BAM_SDCC3, /* BAM SDCC3 */
	HAL_VMIDMT_BAM_SDCC4, /* BAM SDCC4 */
	HAL_VMIDMT_SDC_SDCC4_BAM = HAL_VMIDMT_BAM_SDCC4,
	HAL_VMIDMT_TSIF, /* TSIF */
	HAL_VMIDMT_USB1_HS, /* BAM USB OTG */
	HAL_VMIDMT_USB2_HSIC, /* USB OTG */
	HAL_VMIDMT_QDSS_VMIDDAP, /* QDSS VMIDDAP */
	HAL_VMIDMT_QDSS_VMIDETR, /* QDSS VMIDETR */
	HAL_VMIDMT_RPM_MSGRAM, /* RPM MSGRAM */
	HAL_VMIDMT_RPM = HAL_VMIDMT_RPM_MSGRAM,
	HAL_VMIDMT_AOP = HAL_VMIDMT_RPM_MSGRAM,
	HAL_VMIDMT_SPDM_WRAPPER, /* SPDM TOP */
	HAL_VMIDMT_USB30, /* USB30 */
	HAL_VMIDMT_PRONTO, /* PRONTO */
	HAL_VMIDMT_QPIC_BAM, /* QPIC BAM */
	HAL_VMIDMT_QPIC = HAL_VMIDMT_QPIC_BAM,
	HAL_VMIDMT_IPA, /* IPA */
	HAL_VMIDMT_IPA_0_IPA = HAL_VMIDMT_IPA,
	HAL_VMIDMT_IPA_WRAPPER = HAL_VMIDMT_IPA,
	HAL_VMIDMT_APCS, /* APCS */
	HAL_VMIDMT_CRYPTO2_AXI, /* CRYPTO AXI - 2 */
	HAL_VMIDMT_CRYPTO2_BAM, /* CRYPTO BAM - 2 */
	HAL_VMIDMT_EMAC,
	HAL_VMIDMT_LPASS_HDMI,
	HAL_VMIDMT_LPASS_SPDIF,
	HAL_VMIDMT_VENUS_VBIF2,
	HAL_VMIDMT_MMSS_VPU_MAPLE,
	HAL_VMIDMT_USB_HS_SEC,
	HAL_VMIDMT_SATA,
	HAL_VMIDMT_PCIE20,
	HAL_VMIDMT_PCIE_0_PCIE20 = HAL_VMIDMT_PCIE20,
	HAL_VMIDMT_PCIE0,
	HAL_VMIDMT_PCIE1,
	HAL_VMIDMT_USB3_HSIC,
	HAL_VMIDMT_UFS,
	HAL_VMIDMT_LPASS_SB1,
	HAL_VMIDMT_SSC_BLSP_BAM,
	HAL_VMIDMT_SSC_SSC_BLSP_BAM = HAL_VMIDMT_SSC_BLSP_BAM,
	HAL_VMIDMT_SSC_QUPV3 = HAL_VMIDMT_SSC_BLSP_BAM,
	HAL_VMIDMT_LPASS_SENSOR_BLSP_BAM,
	HAL_VMIDMT_SSC_SDC,
	HAL_VMIDMT_LPASS_SSC_SDC,
	HAL_VMIDMT_SPMI,
	HAL_VMIDMT_SPMI_FETCHER = HAL_VMIDMT_SPMI,
	HAL_VMIDMT_BAM_BLSP3_DMA,
	HAL_VMIDMT_QUPV3_SOUTH = HAL_VMIDMT_BAM_BLSP2_DMA,
	HAL_VMIDMT_QUPV3_NORTH = HAL_VMIDMT_BAM_BLSP1_DMA,
	HAL_VMIDMT_QUPV3_2 = HAL_VMIDMT_BAM_BLSP3_DMA,
	HAL_VMIDMT_QUPV3_EAST = HAL_VMIDMT_BAM_BLSP3_DMA,
	HAL_VMIDMT_QSPI,
	HAL_VMIDMT_LPASS_RXTX,
	HAL_VMIDMT_LPASS_WSA,
	HAL_VMIDMT_LPASS_VA,
	HAL_VMIDMT_MSS_NAV,
	HAL_VMIDMT_PCNOC_SNOC1,
	HAL_VMIDMT_PCNOC_SNOC2,
	HAL_VMIDMT_PCNOC_SNOC3,
	HAL_VMIDMT_PCNOC_SNOC4,
	HAL_VMIDMT_DEHR_BIMC,
	HAL_VMIDMT_COUNT,
};

struct hal_vmidmt_access_config {
	uint32_t au_vmid[HAL_VMIDMT_MASTER_MASK_WORDS]; /* VMID to mask */
	uint32_t au_vmid_perm[HAL_VMIDMT_MASTER_PERM_WORDS]; /* VMID permissions */
};

enum hal_vmidmt_error_option {
	HAL_VMIDMT_ERROR_O_SMCFCFG_EN = 0x1, /* stream match conflict fault */
	HAL_VMIDMT_ERROR_O_USFCFG_EN = 0x2, /* unidentified stream fault */
	HAL_VMIDMT_ERROR_O_GCFGFIE = 0x4, /* interrupt on global config fault */
	HAL_VMIDMT_ERROR_O_GCFGFRE = 0x8, /* report fault to offending master */
	HAL_VMIDMT_ERROR_O_GFIE = 0x10, /* interrupt on global client fault */
};

/* Store error reporting options ORed in this */
typedef uint32_t hal_vmidmt_error_option_config;

struct hal_vmidmt_device_params {
	uint16_t u_entry_count; /* Total number of table entries */
	uint16_t u_num_vmid; /* number of VMID supported by the core */
	uint8_t b_stream_map_support; /* Is stream mapping mode supported? */
	uint8_t u_num_ssd_index_bits; /* number of SSD index bits */
	uint8_t u_num_stream_id_bits; /* number of SID bits available */
};

enum hal_vmidmt_error_flags {
	HAL_VMIDMT_ERROR_F_CLMULTI = 0x1, /* multiple error; 2nd is client */
	HAL_VMIDMT_ERROR_F_CFGMULTI = 0x2, /* multiple error; 2nd is config */
	HAL_VMIDMT_ERROR_F_PF = 0x4, /* permission fault */
	HAL_VMIDMT_ERROR_F_CAF = 0x8, /* configuration access fault */
	HAL_VMIDMT_ERROR_F_SMCF = 0x10, /* stream match conflict fault */
	HAL_VMIDMT_ERROR_F_USF = 0x20, /* unidentified stream fault */
};

/* Store error flags ORed in this */
typedef uint32_t hal_vmidmt_error_flags_config;

enum hal_vmidmt_bus_error_flags {
	HAL_VMIDMT_BUS_F_ERROR_NSATTR = 0x1, /* non-secure attribute */
	HAL_VMIDMT_BUS_F_ERROR_NSSTATE = 0x2, /* non-secure state */
	HAL_VMIDMT_BUS_F_ERROR_WNR = 0x4, /* write not read */
};

/* Store bus flags ORed in this */
typedef uint32_t hal_vmidmt_bus_error_flags_mask;

struct hal_vmidmt_error {
	hal_vmidmt_error_flags_config u_error_flags; /* Error Flags */
	hal_vmidmt_bus_error_flags_mask u_bus_flags; /* Bus specific flags */
	uint32_t u_physical_address_lower32; /* Lower 32 bits of PA */
	uint32_t u_physical_address_upper32; /* Upper 32 bits of PA */
	uint32_t u_ssd_index; /* SSD index */
	uint32_t u_sid; /* Stream ID */
	uint32_t u_master_id; /* Master ID */
	uint32_t u_avmid; /* Virtual master ID */
	uint32_t u_atid; /* ATID */
	uint32_t u_abid; /* ABID */
	uint32_t u_apid; /* APID */
};

typedef uint32_t hal_vmidmt_secure_status_det;

enum hal_vmidmt_non_secure_alloc_config {
	HAL_VMIDMT_NSCFG_XTRAN = 0, /* use x_protns from transaction */
	HAL_VMIDMT_NSCFG_SECURE = 2, /* secure */
	HAL_VMIDMT_NSCFG_NONSECURE = 3, /* non-secure */
	HAL_VMIDMT_NSCFG_DEFAULT = 4 /* use default */
};

enum hal_vmidmt_write_alloc_config {
	HAL_VMIDMT_WACFG_XTRAN = 0, /* use attributes from transaction */
	HAL_VMIDMT_WACFG_ALLOC = 2, /* allocate */
	HAL_VMIDMT_WACFG_NONALLOC = 3, /* non-allocate */
	HAL_VMIDMT_WACFG_DEFAULT = 4 /* use default */
};

enum hal_vmidmt_read_alloc_config {
	HAL_VMIDMT_RACFG_XTRAN = 0, /* use attributes from transaction */
	HAL_VMIDMT_RACFG_ALLOC = 2, /* allocate */
	HAL_VMIDMT_RACFG_NONALLOC = 3, /* non-allocate */
	HAL_VMIDMT_RACFG_DEFAULT = 4 /* use default */
};

enum hal_vmidmt_shared_config {
	HAL_VMIDMT_SHCFG_XTRAN = 0, /* use attributes from transaction */
	HAL_VMIDMT_SHCFG_OUTER_SHARE = 1, /* outer-shareable */
	HAL_VMIDMT_SHCFG_INNER_SHARE = 2, /* inner-shareable */
	HAL_VMIDMT_SHCFG_NON_SHARE = 3, /* non-shareable */
	HAL_VMIDMT_SHCFG_DEFAULT = 4 /* use default */
};

enum hal_vmidmt_mem_type_config {
	HAL_VMIDMT_MTCFG_XTRAN = 0, /* use attributes from transaction */
	HAL_VMIDMT_MTCFG_MEMATTR = 1, /* use MemAttr field */
	HAL_VMIDMT_MTCFG_DEFAULT = 2 /* use default */
};

enum hal_vmidmt_transient_config {
	HAL_VMIDMT_TRANSIENTCFG_XTRAN = 0, /* use transaction attr */
	HAL_VMIDMT_TRANSIENTCFG_NON_TRANSIENT = 2, /* non-transient */
	HAL_VMIDMT_TRANSIENTCFG_TRANSIENT = 3, /* transient */
	HAL_VMIDMT_TRANSIENTCFG_DEFAULT = 4 /* use default */
};

struct hal_vmidmt_bus_attrib {
	enum hal_vmidmt_non_secure_alloc_config e_nscfg; /* non-secure allocate */
	enum hal_vmidmt_write_alloc_config e_wacfg; /* write allocate */
	enum hal_vmidmt_read_alloc_config e_racfg; /* read allocate */
	enum hal_vmidmt_shared_config e_shcfg; /* shareable attribute */
	enum hal_vmidmt_mem_type_config e_mtcfg; /* memory type config */
	uint8_t mem_attr; /* memory attributes (3b) */
	enum hal_vmidmt_transient_config e_transient_cfg; /* transient config */
};

enum hal_vmidmt_redir_cache_non_shareable_config {
	HAL_VMIDMT_RCNSH_DISABLE = 0, /* do not redirect */
	HAL_VMIDMT_RCNSH_ENABLE = 1, /* enable redirect */
	HAL_VMIDMT_RCNSH_DEFAULT = 2 /* use default */
};

enum hal_vmidmt_redir_cache_inner_shareable_config {
	HAL_VMIDMT_RCISH_DISABLE = 0, /* do not redirect */
	HAL_VMIDMT_RCISH_ENABLE = 1, /* enable redirect */
	HAL_VMIDMT_RCISH_DEFAULT = 2 /* use default */
};

enum hal_vmidmt_redir_cache_outer_shareable_config {
	HAL_VMIDMT_RCOSH_DISABLE = 0, /* do not redirect */
	HAL_VMIDMT_RCOSH_ENABLE = 1, /* enable redirect */
	HAL_VMIDMT_RCOSH_DEFAULT = 2 /* use default */
};

enum hal_vmidmt_req_priority_config {
	HAL_VMIDMT_REQPRICFG_XTRAN = 0, /* use transaction attr */
	HAL_VMIDMT_REQPRICFG_ACR_REQPRI =
		1, /* use (S)ACR.REQPRIORITY override */
	HAL_VMIDMT_REQPRICFG_DEFAULT = 2 /* use default */
};

enum hal_vmidmt_req_priority {
	HAL_VMIDMT_REQPRI_NORMAL = 0, /* normal priority */
	HAL_VMIDMT_REQPRI_HIGH = 1, /* high priority */
	HAL_VMIDMT_REQPRI_HIGHER = 2, /* higher priority */
	HAL_VMIDMT_REQPRI_HIGHEST = 3, /* highest priority */
	HAL_VMIDMT_REQPRI_DEFAULT = 4 /* use default */
};

struct hal_vmidmt_aux_config {
	enum hal_vmidmt_redir_cache_non_shareable_config
		e_rcnsh; /* redirect C-NSH */
	enum hal_vmidmt_redir_cache_inner_shareable_config
		e_rcish; /* redirect C-ISH */
	enum hal_vmidmt_redir_cache_outer_shareable_config
		e_rcosh; /* redirect C-OSH */
	enum hal_vmidmt_req_priority_config e_req_priority_cfg; /* req pri cfg */
	enum hal_vmidmt_req_priority e_req_priority; /* req priority */
};

struct hal_vmidmt_default_vmid_config {
	struct hal_vmidmt_bus_attrib
		*p_bypass_bus_attrib; /* bypass bus attributes */
	struct hal_vmidmt_aux_config *p_bypass_aux_config; /* bypass aux config */
	struct hal_vmidmt_access_config
		*p_access_control; /* RPU access control */
	bool b_vmid_private_namespace_enable; /* private NS */
	uint8_t bypass_vmid; /* bypass VMID */
};

struct hal_vmidmt_default_secure_vmid_config {
	struct hal_vmidmt_default_vmid_config
		*p_default_secure_config; /* default cfg */
	bool b_glb_addr_space_restricted_acc_enable;
	/* global address space restricted access enable */
	uint8_t secure_extensions;
	/* non-secure number of stream mapping register groups override
	 * HAL_VMIDMT_SECURE_EXT_DEFAULT can be used for default setting
	 */
};

struct hal_vmidmt_context_config {
	struct hal_vmidmt_bus_attrib *p_bus_attrib; /* config S2VRn */
	struct hal_vmidmt_aux_config *p_aux_config; /* config AS2VRn */
	uint8_t u_vmid; /* config VMID in S2VRn */
};

enum hal_vmidmt_int_status {
	HAL_VMIDMT_INT_NO_ERROR,
	HAL_VMIDMT_INT_PROPERTY_HANDLE_ERROR,
	HAL_VMIDMT_INT_UNSUP_INST_FOR_TARGET,
};

enum hal_vmidmt_access {
	HAL_VMIDMT_NO_ACCESS,
	HAL_VMIDMT_FULL_ACCESS,
	HAL_VMIDMT_ACCESS_COUNT,
};

struct hal_vmidmt_int_vmidmt_dev_params {
	uint16_t entry_count;
	uint16_t num_vmid;
	uint8_t num_ssd_index_bits;
	uint8_t num_stream_id_bits;
	uint8_t input_addr_size;
	uint8_t stream_match_support;
};

struct hal_vmidmt_info {
	uint64_t base_addr;
	struct hal_vmidmt_int_vmidmt_dev_params dev_params;
};

struct hal_vmidmt_port_map {
	uint8_t port;
	struct hal_vmidmt_info vmidmt_info;
};

/* Initialize VMIDMT */
enum hal_vmidmt_status
vmidmt_hal_init(struct hal_vmidmt_info *info,
		const struct hal_vmidmt_default_secure_vmid_config *secure_def,
		const struct hal_vmidmt_default_vmid_config *defcfg,
		char **err_str);

/* Enable or disable client interface. */
enum hal_vmidmt_status
vmidmt_hal_enable_client(const struct hal_vmidmt_info *info, bool enable);

/* Configure Secure Status Determination Table (SSDT). */
enum hal_vmidmt_status
vmidmt_hal_config_ssdt(const struct hal_vmidmt_info *info,
		       const hal_vmidmt_secure_status_det *ssdt,
		       uint32_t count);

/* Configure a single context entry. */
enum hal_vmidmt_status
vmidmt_hal_config_ctx(const struct hal_vmidmt_info *info, uint32_t index,
		      const struct hal_vmidmt_context_config *cfg);

/* Configure multiple context entries (extended). */
enum hal_vmidmt_status
vmidmt_hal_config_ctx_ext(const struct hal_vmidmt_info *info,
			  uint32_t first_index, const uint32_t *index_list,
			  uint32_t list_len,
			  const struct hal_vmidmt_context_config *cfg);

/* Configure error reporting/handling options. */
enum hal_vmidmt_status vmidmt_hal_cfg_err(const struct hal_vmidmt_info *info,
					  bool global_cfg,
					  hal_vmidmt_error_option_config opts);

/* Check if an error is latched. */
bool vmidmt_hal_is_error(const struct hal_vmidmt_info *info, bool global_err);

/* Read back the latched error record. */
void vmidmt_hal_get_error(const struct hal_vmidmt_info *info, bool global_err,
			  struct hal_vmidmt_error *err);

/* Clear latched error(s). */
enum hal_vmidmt_status
vmidmt_hal_clear_error(const struct hal_vmidmt_info *info, bool global_err);

#endif /* VMIDMT_HAL_H */
