/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MESSAGES_CORE_H
#define MESSAGES_CORE_H

#include <tisci/tisci_protocol.h>
#include <types/devgrps.h>

/** Bit fields of TISCI_MSG_QUERY_FW_CAPS message */
#define TISCI_MSG_FLAG_FW_CAP_GENERIC_CAP	   BIT(0)
#define TISCI_MSG_FLAG_FW_CAP_LPM_DEEP_SLEEP	   BIT(1)
#define TISCI_MSG_FLAG_FW_CAP_LPM_MCU_ONLY	   BIT(2)
#define TISCI_MSG_FLAG_FW_CAP_LPM_STANDBY	   BIT(3)
#define TISCI_MSG_FLAG_FW_CAP_LPM_PARTIAL_IO_ON	   BIT(4)

/**
 * \brief Notification message to indicate the DMSC is available.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_msg_version_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_VERSION request to provide version info about
 *	  currently running firmware.
 * \param hdr TISCI header.
 * \param str Human readable version string.
 * \param version number of the firmware.
 * \param abi_major Major version number of ABI in use by firmware.
 * \param abi_minor Minor version number of ABI in use by firmware.
 * \param sub_version Sub-version number of the firmware.
 * \param patch_version Patch-version number of the firmware.
 */
struct tisci_msg_version_resp {
	struct tisci_header	hdr;
	char			str[32];
	uint16_t			version;
	uint8_t			abi_major;
	uint8_t			abi_minor;
	uint8_t			sub_version;
	uint8_t			patch_version;
} __packed;

/**
 * \brief Notification message to indicate the DMSC is available.
 *
 * \param hdr TISCI header
 * \param extboot_status Status of extended boot. Applicable only for combined image
 */
struct tisci_msg_boot_notification_req {
	struct tisci_header	hdr;
	uint32_t			extboot_status;
} __packed;

/**
 * \brief Response to notification message about firmware availability.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_msg_boot_notification_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_BOARD_CONFIG_RM request to provide the location and
 *	  size of the boardcfg Resource Management configuration structure.
 *
 * \param hdr TISCI header
 * \param boardcfg_rmp_low Low 32-bits of physical pointer to boardcfg
 *			   RM configuration struct.
 * \param boardcfg_rmp_high High 32-bits of physical pointer to boardcfg
 *			    RM configuration struct.
 * \param boardcfg_rm_size Size of RM configuration data.
 * \param boardcfg_rm_devgrp Device group for the RM configuration.
 */
struct tisci_msg_board_config_rm_req {
	struct tisci_header	hdr;
	uint32_t			boardcfg_rmp_low;
	uint32_t			boardcfg_rmp_high;
	uint16_t			boardcfg_rm_size;
	devgrp_t		boardcfg_rm_devgrp;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_RM.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_rm_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_RESASG.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_resasg_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_BOARD_CONFIG request to provide the location and
 *	  size of the boardcfg structure.
 *
 * \param hdr TISCI header
 * \param boardcfgp_low Low 32-bits of physical pointer to boardcfg struct.
 * \param boardcfgp_high High 32-bits of physical pointer to boardcfg struct.
 * \param boardcfg_size Size of configuration data.
 * \param boardcfg_devgrp Device group for the base board configuration.
 *
 */
struct tisci_msg_board_config_req {
	struct tisci_header	hdr;
	uint32_t			boardcfgp_low;
	uint32_t			boardcfgp_high;
	uint16_t			boardcfg_size;
	devgrp_t		boardcfg_devgrp;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_BOARD_CONFIG_SECURITY request to provide the location and
 *	  size of the boardcfg security configuration structure.
 *
 * \param hdr TISCI header
 * \param boardcfg_securityp_low Low 32-bits of physical pointer to boardcfg
 *			       security configuration struct.
 * \param boardcfg_securityp_high High 32-bits of physical pointer to boardcfg
 *				security configuration struct.
 * \param boardcfg_security_size Size of security configuration data.
 * \param boardcfg_security_devgrp Device group for the security board configuration.
 *
 */
struct tisci_msg_board_config_security_req {
	struct tisci_header	hdr;
	uint32_t			boardcfg_securityp_low;
	uint32_t			boardcfg_securityp_high;
	uint16_t			boardcfg_security_size;
	devgrp_t		boardcfg_security_devgrp;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_SECURITY.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_security_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_BOARD_CONFIG_PM request to provide the location and
 *	  size of the boardcfg Power Management configuration structure.
 *
 * \param hdr TISCI header
 * \param boardcfg_pmp_low Low 32-bits of physical pointer to boardcfg
 *			       PM configuration struct.
 * \param boardcfg_pmp_high High 32-bits of physical pointer to boardcfg
 *				PM configuration struct.
 * \param boardcfg_pm_size Size of PM configuration data.
 * \param boardcfg_pm_devgrp  Device group for the PM configuration
 *
 */
struct tisci_msg_board_config_pm_req {
	struct tisci_header	hdr;
	uint32_t			boardcfg_pmp_low;
	uint32_t			boardcfg_pmp_high;
	uint16_t			boardcfg_pm_size;
	devgrp_t		boardcfg_pm_devgrp;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_PM.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_pm_resp {
	struct tisci_header hdr;
} __packed;


/**
 * \brief TISCI_MSG_QUERY_MSMC request to provide currently available MSMC
 * memory if present in the SoC
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_query_msmc_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_QUERY_MSMC request response providing currently available
 * MSMC memory if present in the SoC
 * \param hdr TISCI header.
 * \param msmc_start_low Low 32-bits of physical address start of MSMC memory
 * \param msmc_start_high High 32-bits of physical address start of MSMC
 *			  memory
 * \param msmc_end_low Low 32-bits of physical addressable address
 *			(byte aligned) of valid range of MSMC memory.
 * \param msmc_end_high High 32-bits of physical addressable address
 *			(byte aligned) of valid range of MSMC memory.
 */
struct tisci_query_msmc_resp {
	struct tisci_header	hdr;
	uint32_t			msmc_start_low;
	uint32_t			msmc_start_high;
	uint32_t			msmc_end_low;
	uint32_t			msmc_end_high;
} __packed;

/**
 * \brief TISCI_MSG_GET_TRACE_CONFIG request to get the debug trace config from
 * base board config
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_get_trace_config_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_GET_TRACE_CONFIG request response providing the debug config
 * from base board config
 *
 * \param hdr TISCI header.
 * \param trace_dst_enables enabled destination traces
 * \param trace_src_enables enabled source traces
 */
struct tisci_get_trace_config_resp {
	struct tisci_header	hdr;
	uint16_t			trace_dst_enables;
	uint16_t			trace_src_enables;
} __packed;

/**
 * \brief TISCI_MSG_QUERY_FW_CAPS request to provide the firmware/SOC capabilities
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_query_fw_caps_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief TISCI_MSG_QUERY_FW_CAPS request response providing currently available
 * SOC/firmware capabilities
 *
 * \param hdr TISCI header.
 * \param fw_caps Each bit in fw_caps indicating one FW/SOC capability
 *
 */
struct tisci_query_fw_caps_resp {
	struct tisci_header	hdr;
	uint64_t			fw_caps;
} __packed;

#endif /* MESSAGES_CORE_H */
