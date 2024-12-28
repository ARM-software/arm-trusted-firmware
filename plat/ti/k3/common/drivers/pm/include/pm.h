/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_H
#define PM_H

#include <types/short_types.h>
#include <tisci/pm/tisci_pm_clock.h>
#include <tisci/pm/tisci_pm_device.h>
/**
 *  \brief  Set Clock Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t set_clock_handler(struct tisci_msg_set_clock_req *msg_recv);

/**
 *  \brief  Get Clock Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t get_clock_handler(struct tisci_msg_get_clock_req *msg_recv);

/**
 *  \brief  Set Clock Parent Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t set_clock_parent_handler(struct tisci_msg_set_clock_parent_req *msg_recv);

/**
 *  \brief  Get Clock Parent Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t get_clock_parent_handler(struct tisci_msg_get_clock_parent_req *msg_recv);

/**
 *  \brief  Get Number of Clock Parents Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t get_num_clock_parents_handler(struct tisci_msg_get_num_clock_parents_req *msg_recv);

/**
 *  \brief  Set clock frequency Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t set_freq_handler(struct tisci_msg_set_freq_req *msg_recv);

/**
 *  \brief  Query clock frequency Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t query_freq_handler(uint32_t *msg_recv);

/**
 *  \brief  Get clock frequency Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t get_freq_handler(struct tisci_msg_get_freq_req *msg_recv);

/**
 *  \brief  Set Device State Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t set_device_handler(struct tisci_msg_set_device_req *msg_recv);

/**
 *  \brief  Get Device State Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t get_device_handler(struct tisci_msg_get_device_resp *msg_recv);

/**
 *  \brief  Set Device Resets Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t set_device_resets_handler(uint32_t *msg_recv);

/**
 *  \brief  Drop Device Ref counts Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t device_drop_powerup_ref_handler(uint32_t *msg_recv);

/**
 *  \brief  Wake up reason Get Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t wake_reason_handler(uint32_t *msg_recv);

/**
 *  \brief  Wake reset Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t wake_reset_handler(uint32_t *msg_recv);

/**
 *  \brief  Enable WDT Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t enable_wdt_handler(uint32_t *msg_recv);

/**
 *  \brief  Device Goodbye Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t goodbye_handler(uint32_t *msg_recv);

/**
 *  \brief  System Reset Handler PM Function.
 *
 *  \param  msg_recv Pointer to the received and returned message. Same buffer
 *		     used.
 *  \return ret	     SUCCESS if the API executed successfully.
 *		     EFAIL   if the API failed to execute.
 */
int32_t sys_reset_handler(uint32_t *msg_recv);

#endif /* PM_H */
