/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PSC_RAW_H__
#define __PSC_RAW_H__

#include <plat/common/platform.h>

#define MDCTL_STATE_SWRSTDISABLE       0x00U
#define MDCTL_STATE_SYNCRST            0x01U
#define MDCTL_STATE_DISABLE            0x02U
#define MDCTL_STATE_ENABLE             0x03U
#define MDCTL_STATE_AUTO_SLEEP         0x04U
#define MDCTL_STATE_AUTO_WAKE          0x05U

#define PDCTL_STATE_OFF                 0U
#define PDCTL_STATE_ON                  1U

/**
 * \brief Wait for a psc transition to complete, or timeout if it does not
 * \param psc_base Base address of the psc to wait for
 * \param pd Power Domain index to wait for a transition on
 *
 * \return SUCCESS on success, -ETIMEDOUT if timeout occurs
 */
int32_t psc_raw_pd_wait(uint64_t psc_base, uint8_t pd);

/**
 * \brief Initiate a psc transition for a power domain
 * \param psc_base Base address of the psc to transition
 * \param pd Power Domain index to start transition for
 */
void psc_raw_pd_initiate(uint64_t psc_base, uint8_t pd);

/**
 * \brief Set next state for a PDCTL
 * \param psc_base Base address of the psc
 * \param pd Power Domain index to set next state for
 * \param state PDCTL_STATE to set
 * \param force Boolean indicating whether to set FORCE bit
 */
void psc_raw_pd_set_state(uint64_t psc_base, uint8_t pd, uint32_t state, bool force);

/**
 * \brief Set next state for an MDCTL
 * \param psc_base Base address of the psc
 * \param lpsc LPSC index to set next state for
 * \param state MDCTL_STATE to set
 * \param force Boolean indicating whether to set FORCE bit
 */
void psc_raw_lpsc_set_state(uint64_t psc_base, uint8_t lpsc, uint32_t state, bool force);

#endif /* __PSC_RAW_H__ */
