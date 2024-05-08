/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include <common/bl_common.h>
#include <common/build_message.h>
#include <common/debug.h>
#include <common/feat_detect.h>
#include <common/runtime_svc.h>
#include <drivers/console.h>
#include <lib/bootmarker_capture.h>
#include <lib/el3_runtime/context_debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/pmf/pmf.h>
#include <lib/runtime_instr.h>
#include <plat/common/platform.h>
#include <services/std_svc.h>

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_REGISTER_SERVICE_SMC(rt_instr_svc, PMF_RT_INSTR_SVC_ID,
		RT_INSTR_TOTAL_IDS, PMF_STORE_ENABLE)
#endif

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_REGISTER_SERVICE(bl_svc, PMF_RT_INSTR_SVC_ID,
		BL_TOTAL_IDS, PMF_DUMP_ENABLE)
#endif

/*******************************************************************************
 * This function pointer is used to initialise the BL32 image. It's initialized
 * by SPD calling bl31_register_bl32_init after setting up all things necessary
 * for SP execution. In cases where both SPD and SP are absent, or when SPD
 * finds it impossible to execute SP, this pointer is left as NULL
 ******************************************************************************/
static int32_t (*bl32_init)(void);

/*****************************************************************************
 * Function used to initialise RMM if RME is enabled
 *****************************************************************************/
#if ENABLE_RME
static int32_t (*rmm_init)(void);
#endif

/*******************************************************************************
 * Variable to indicate whether next image to execute after BL31 is BL33
 * (non-secure & default) or BL32 (secure).
 ******************************************************************************/
static uint32_t next_image_type = NON_SECURE;

#ifdef SUPPORT_UNKNOWN_MPID
/*
 * Flag to know whether an unsupported MPID has been detected. To avoid having it
 * landing on the .bss section, it is initialized to a non-zero value, this way
 * we avoid potential WAW hazards during system bring up.
 * */
volatile uint32_t unsupported_mpid_flag = 1;
#endif

/*
 * Implement the ARM Standard Service function to get arguments for a
 * particular service.
 */
uintptr_t get_arm_std_svc_args(unsigned int svc_mask)
{
	/* Setup the arguments for PSCI Library */
	DEFINE_STATIC_PSCI_LIB_ARGS_V1(psci_args, bl31_warm_entrypoint);

	/* PSCI is the only ARM Standard Service implemented */
	assert(svc_mask == PSCI_FID_MASK);

	return (uintptr_t)&psci_args;
}

/*******************************************************************************
 * Simple function to initialise all BL31 helper libraries.
 ******************************************************************************/
static void __init bl31_lib_init(void)
{
	cm_init();
}

/*******************************************************************************
 * Setup function for BL31.
 ******************************************************************************/
void bl31_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
		u_register_t arg3)
{
	/* Enable early console if EARLY_CONSOLE flag is enabled */
	plat_setup_early_console();

	/* Perform early platform-specific setup */
	bl31_early_platform_setup2(arg0, arg1, arg2, arg3);

	/* Perform late platform-specific setup */
	bl31_plat_arch_setup();

#if CTX_INCLUDE_PAUTH_REGS
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* CTX_INCLUDE_PAUTH_REGS */

	/* Prints context_memory allocated for all the security states */
	report_ctx_memory_usage();
}

/*******************************************************************************
 * BL31 is responsible for setting up the runtime services for the primary cpu
 * before passing control to the bootloader or an Operating System. This
 * function calls runtime_svc_init() which initializes all registered runtime
 * services. The run time services would setup enough context for the core to
 * switch to the next exception level. When this function returns, the core will
 * switch to the programmed exception level via an ERET.
 ******************************************************************************/
void bl31_main(void)
{
	/* Init registers that never change for the lifetime of TF-A */
	cm_manage_extensions_el3();

	/* Init per-world context registers for non-secure world */
	manage_extensions_nonsecure_per_world();

	NOTICE("BL31: %s\n", build_version_string);
	NOTICE("BL31: %s\n", build_message);

#if FEATURE_DETECTION
	/* Detect if features enabled during compilation are supported by PE. */
	detect_arch_features();
#endif /* FEATURE_DETECTION */

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_CAPTURE_TIMESTAMP(bl_svc, BL31_ENTRY, PMF_CACHE_MAINT);
#endif

#ifdef SUPPORT_UNKNOWN_MPID
	if (unsupported_mpid_flag == 0) {
		NOTICE("Unsupported MPID detected!\n");
	}
#endif

	/* Perform platform setup in BL31 */
	bl31_platform_setup();

	/* Initialise helper libraries */
	bl31_lib_init();

#if EL3_EXCEPTION_HANDLING
	INFO("BL31: Initialising Exception Handling Framework\n");
	ehf_init();
#endif

	/* Initialize the runtime services e.g. psci. */
	INFO("BL31: Initializing runtime services\n");
	runtime_svc_init();

	/*
	 * All the cold boot actions on the primary cpu are done. We now need to
	 * decide which is the next image and how to execute it.
	 * If the SPD runtime service is present, it would want to pass control
	 * to BL32 first in S-EL1. In that case, SPD would have registered a
	 * function to initialize bl32 where it takes responsibility of entering
	 * S-EL1 and returning control back to bl31_main. Similarly, if RME is
	 * enabled and a function is registered to initialize RMM, control is
	 * transferred to RMM in R-EL2. After RMM initialization, control is
	 * returned back to bl31_main. Once this is done we can prepare entry
	 * into BL33 as normal.
	 */

	/*
	 * If SPD had registered an init hook, invoke it.
	 */
	if (bl32_init != NULL) {
		INFO("BL31: Initializing BL32\n");

		console_flush();
		int32_t rc = (*bl32_init)();

		if (rc == 0) {
			WARN("BL31: BL32 initialization failed\n");
		}
	}

	/*
	 * If RME is enabled and init hook is registered, initialize RMM
	 * in R-EL2.
	 */
#if ENABLE_RME
	if (rmm_init != NULL) {
		INFO("BL31: Initializing RMM\n");

		console_flush();
		int32_t rc = (*rmm_init)();

		if (rc == 0) {
			WARN("BL31: RMM initialization failed\n");
		}
	}
#endif

	/*
	 * We are ready to enter the next EL. Prepare entry into the image
	 * corresponding to the desired security state after the next ERET.
	 */
	bl31_prepare_next_image_entry();

	/*
	 * Perform any platform specific runtime setup prior to cold boot exit
	 * from BL31
	 */
	bl31_plat_runtime_setup();

#if ENABLE_RUNTIME_INSTRUMENTATION
	console_flush();
	PMF_CAPTURE_TIMESTAMP(bl_svc, BL31_EXIT, PMF_CACHE_MAINT);
#endif

	console_flush();
	console_switch_state(CONSOLE_FLAG_RUNTIME);
}

/*******************************************************************************
 * Accessor functions to help runtime services decide which image should be
 * executed after BL31. This is BL33 or the non-secure bootloader image by
 * default but the Secure payload dispatcher could override this by requesting
 * an entry into BL32 (Secure payload) first. If it does so then it should use
 * the same API to program an entry into BL33 once BL32 initialisation is
 * complete.
 ******************************************************************************/
void bl31_set_next_image_type(uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));
	next_image_type = security_state;
}

uint32_t bl31_get_next_image_type(void)
{
	return next_image_type;
}

/*******************************************************************************
 * This function programs EL3 registers and performs other setup to enable entry
 * into the next image after BL31 at the next ERET.
 ******************************************************************************/
void __init bl31_prepare_next_image_entry(void)
{
	entry_point_info_t *next_image_info;
	uint32_t image_type;

#if CTX_INCLUDE_AARCH32_REGS
	/*
	 * Ensure that the build flag to save AArch32 system registers in CPU
	 * context is not set for AArch64-only platforms.
	 */
	if (el_implemented(1) == EL_IMPL_A64ONLY) {
		ERROR("EL1 supports AArch64-only. Please set build flag "
				"CTX_INCLUDE_AARCH32_REGS = 0\n");
		panic();
	}
#endif

	/* Determine which image to execute next */
	image_type = bl31_get_next_image_type();

	/* Program EL3 registers to enable entry into the next EL */
	next_image_info = bl31_plat_get_next_image_ep_info(image_type);
	assert(next_image_info != NULL);
	assert(image_type == GET_SECURITY_STATE(next_image_info->h.attr));

	INFO("BL31: Preparing for EL3 exit to %s world\n",
		(image_type == SECURE) ? "secure" : "normal");
	print_entry_point_info(next_image_info);
	cm_init_my_context(next_image_info);

	/*
	* If we are entering the Non-secure world, use
	* 'cm_prepare_el3_exit_ns' to exit.
	*/
	if (image_type == NON_SECURE) {
		cm_prepare_el3_exit_ns();
	} else {
		cm_prepare_el3_exit(image_type);
	}
}

/*******************************************************************************
 * This function initializes the pointer to BL32 init function. This is expected
 * to be called by the SPD after it finishes all its initialization
 ******************************************************************************/
void bl31_register_bl32_init(int32_t (*func)(void))
{
	bl32_init = func;
}

#if ENABLE_RME
/*******************************************************************************
 * This function initializes the pointer to RMM init function. This is expected
 * to be called by the RMMD after it finishes all its initialization
 ******************************************************************************/
void bl31_register_rmm_init(int32_t (*func)(void))
{
	rmm_init = func;
}
#endif
