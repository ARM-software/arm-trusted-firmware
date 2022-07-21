/*
 * Copyright (c) 2017-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSCI_LIB_H
#define PSCI_LIB_H

#include <common/ep_info.h>

#ifndef __ASSEMBLER__

#include <cdefs.h>
#include <stdint.h>

/*******************************************************************************
 * Optional structure populated by the Secure Payload Dispatcher to be given a
 * chance to perform any bookkeeping before PSCI executes a power management
 * operation. It also allows PSCI to determine certain properties of the SP e.g.
 * migrate capability etc.
 ******************************************************************************/
typedef struct spd_pm_ops {
	void (*svc_on)(u_register_t target_cpu);
	int32_t (*svc_off)(u_register_t __unused unused);
	void (*svc_suspend)(u_register_t max_off_pwrlvl);
	void (*svc_on_finish)(u_register_t __unused unused);
	void (*svc_suspend_finish)(u_register_t max_off_pwrlvl);
	int32_t (*svc_migrate)(u_register_t from_cpu, u_register_t to_cpu);
	int32_t (*svc_migrate_info)(u_register_t *resident_cpu);
	void (*svc_system_off)(void);
	void (*svc_system_reset)(void);
} spd_pm_ops_t;

/*
 * Function prototype for the warmboot entrypoint function which will be
 * programmed in the mailbox by the platform.
 */
typedef void (*mailbox_entrypoint_t)(void);

/******************************************************************************
 * Structure to pass PSCI Library arguments.
 *****************************************************************************/
typedef struct psci_lib_args {
	/* The version information of PSCI Library Interface */
	param_header_t		h;
	/* The warm boot entrypoint function */
	mailbox_entrypoint_t	mailbox_ep;
} psci_lib_args_t;

/* Helper macro to set the psci_lib_args_t structure at runtime */
#define SET_PSCI_LIB_ARGS_V1(_p, _entry)	do {			\
	SET_PARAM_HEAD(_p, PARAM_PSCI_LIB_ARGS, VERSION_1, 0);		\
	(_p)->mailbox_ep = (_entry);					\
	} while (0)

/* Helper macro to define the psci_lib_args_t statically */
#define DEFINE_STATIC_PSCI_LIB_ARGS_V1(_name, _entry)		\
	static const psci_lib_args_t (_name) = {		\
		.h.type = (uint8_t)PARAM_PSCI_LIB_ARGS,		\
		.h.version = (uint8_t)VERSION_1,		\
		.h.size = (uint16_t)sizeof(_name),		\
		.h.attr = 0U,					\
		.mailbox_ep = (_entry)				\
	}

/* Helper macro to verify the pointer to psci_lib_args_t structure */
#define VERIFY_PSCI_LIB_ARGS_V1(_p)	(((_p) != NULL)		\
		&& ((_p)->h.type == PARAM_PSCI_LIB_ARGS)	\
		&& ((_p)->h.version == VERSION_1)		\
		&& ((_p)->h.size == sizeof(*(_p)))		\
		&& ((_p)->h.attr == 0)				\
		&& ((_p)->mailbox_ep != NULL))

/******************************************************************************
 * PSCI Library Interfaces
 *****************************************************************************/
u_register_t psci_smc_handler(uint32_t smc_fid,
			  u_register_t x1,
			  u_register_t x2,
			  u_register_t x3,
			  u_register_t x4,
			  void *cookie,
			  void *handle,
			  u_register_t flags);
int psci_setup(const psci_lib_args_t *lib_args);
int psci_secondaries_brought_up(void);
void psci_warmboot_entrypoint(void);
void psci_register_spd_pm_hook(const spd_pm_ops_t *pm);
void psci_prepare_next_non_secure_ctx(
			  entry_point_info_t *next_image_info);
int psci_stop_other_cores(unsigned int wait_ms,
			  void (*stop_func)(u_register_t mpidr));
bool psci_is_last_on_cpu_safe(void);

#endif /* __ASSEMBLER__ */

#endif /* PSCI_LIB_H */
