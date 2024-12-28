/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYS_RESET_H
#define SYS_RESET_H

#include <types.h>
#include <types/domgrps.h>
#include <stddef.h>
#include <types/short_types.h>

/**
 * \brief Resets a domain group
 *
 * \param domain
 * The domain to be reset.
 *
 * \return SUCCESS if domain reset succeeds. Error value if fails.
 */
int32_t system_reset(domgrp_t domain);

/**
 * \brief Register handler for the tisci system reset.
 *
 * \param handler Pointer to the tisci system reset handler function
 */
void sys_reset_handler_register(int32_t (*handler)(domgrp_t domain));

#endif
