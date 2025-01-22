/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef TISCI_PM_TISCI_CORE_H
#define TISCI_PM_TISCI_CORE_H

#include <tisci/tisci_protocol.h>
#include <types/domgrps.h>

/**
 * \brief Empty request for TISCI_MSG_WAKE_REASON.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_wake_reason_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Response for TISCI_MSG_WAKE_REASON.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 * \param mode that was active before wake event.
 * \param reason the wakeup happened.
 * \param time_ms Time spent in low power mode.
 */
struct tisci_msg_wake_reason_resp {
	struct tisci_header	hdr;
	char			mode[32];
	char			reason[32];
	uint32_t			time_ms;
} __packed;

/**
 * \brief Empty request for TISCI_MSG_WAKE_RESET.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_wake_reset_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_WAKE_RESET.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_wake_reset_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Empty request for TISCI_MSG_ENABLE_WDT.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_enable_wdt_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_ENABLE_WDT.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_enable_wdt_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Empty request for TISCI_MSG_GOODBYE.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_goodbye_req {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_GOODBYE.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_goodbye_resp {
	struct tisci_header hdr;
} __packed;

/**
 * \brief Request for TISCI_MSG_SYS_RESET.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 * \param domain to be reset.
 */
struct tisci_msg_sys_reset_req {
	struct tisci_header	hdr;
	domgrp_t		domain;
} __packed;

/**
 * \brief Empty response for TISCI_MSG_SYS_RESET.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_sys_reset_resp {
	struct tisci_header hdr;
} __packed;

#endif /* TISCI_PM_TISCI_CORE_H */
