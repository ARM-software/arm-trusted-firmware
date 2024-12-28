/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LPM_H
#define LPM_H

#include <types/short_types.h>

/**
 *  \brief  LPM enter sleep handler Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t dm_enter_sleep_handler(uint32_t *msg_recv);

/**
 *  \brief  LPM prepare sleep handler Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t dm_prepare_sleep_handler(uint32_t *msg_recv);

/**
 *  \brief  LPM wake reason handler Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t dm_lpm_wake_reason_handler(uint32_t *msg_recv);

/**
 *  \brief  enable/disable io isolation handler Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t dm_set_io_isolation_handler(uint32_t *msg_recv);

#endif
