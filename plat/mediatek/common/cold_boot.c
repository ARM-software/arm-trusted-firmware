/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>

/* Vendors headers */
#include <cold_boot.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_sip_svc.h>

static struct kernel_info k_info;
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
static bool el1_is_2nd_bootloader = true;
static struct atf_arg_t atfarg;

static int init_mtk_bl32_arg(void)
{
	struct mtk_bl_param_t *p_mtk_bl_param;
	struct atf_arg_t *p_atfarg;

	p_mtk_bl_param = (struct mtk_bl_param_t *) get_mtk_bl31_fw_config(BOOT_ARG_FROM_BL2);
	if (p_mtk_bl_param == NULL) {
		ERROR("p_mtk_bl_param is NULL!\n");
		return -1;
	}
	p_atfarg = (struct atf_arg_t *)p_mtk_bl_param->atf_arg_addr;
	if (p_atfarg == NULL) {
		ERROR("bl32 argument is NULL!\n");
		return -1;
	}
	memcpy((void *)&atfarg, (void *)p_atfarg, sizeof(struct atf_arg_t));
	return 0;
}
MTK_EARLY_PLAT_INIT(init_mtk_bl32_arg);

static void save_kernel_info(uint64_t pc, uint64_t r0, uint64_t r1, uint64_t k32_64)
{
	k_info.k32_64 = k32_64;
	k_info.pc = pc;

	if (k32_64 == LINUX_KERNEL_32) {
		/* for 32 bits kernel */
		k_info.r0 = 0;
		/* machtype */
		k_info.r1 = r0;
		/* tags */
		k_info.r2 = r1;
	} else {
		/* for 64 bits kernel */
		k_info.r0 = r0;
		k_info.r1 = r1;
	}
}

static uint32_t plat_get_spsr_for_bl32_64_entry(void)
{
	return SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}

#if MTK_BL33_IS_64BIT
static uint32_t plat_get_spsr_for_bl33_entry(void)
{
	uint32_t spsr;
	uint32_t mode;

	mode = MODE_EL1;
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#else
static uint32_t plat_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;
	unsigned int ee;
	unsigned long daif;

	INFO("Secondary bootloader is AArch32\n");
	mode = MODE32_svc;
	ee = 0;
	/*
	 * TODO: Choose async. exception bits if HYP mode is not
	 * implemented according to the values of SCR.{AW, FW} bits
	 */
	daif = DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;

	spsr = SPSR_MODE32(mode, 0, ee, daif);
	return spsr;
}
#endif

static void populate_bl32_image_ep(entry_point_info_t *bl32_ep_instance,
		struct mtk_bl_param_t *p_mtk_bl_param)
{
	entry_point_info_t *populated_ep_bl32 = bl32_ep_instance;

	if (p_mtk_bl_param == NULL) {
		ERROR("p_mtk_bl_param is NULL!\n");
		panic();
	}
	SET_SECURITY_STATE(bl32_ep_instance->h.attr, SECURE);
	SET_PARAM_HEAD(populated_ep_bl32,
		       PARAM_EP,
		       VERSION_1,
		       populated_ep_bl32->h.attr);
	populated_ep_bl32->pc = atfarg.tee_entry;
	populated_ep_bl32->spsr = plat_get_spsr_for_bl32_64_entry();
}

static void populate_bl33_image_ep(entry_point_info_t *bl33_ep_instance,
		struct mtk_bl_param_t *p_mtk_bl_param)
{
	entry_point_info_t *populated_ep_bl33 = bl33_ep_instance;

	if (p_mtk_bl_param == NULL) {
		ERROR("p_mtk_bl_param is NULL!\n");
		panic();
	}
	SET_SECURITY_STATE(bl33_ep_instance->h.attr, NON_SECURE);
	SET_PARAM_HEAD(populated_ep_bl33,
		       PARAM_EP,
		       VERSION_1,
		       populated_ep_bl33->h.attr);
	populated_ep_bl33->pc = p_mtk_bl_param->bl33_start_addr;
	/* standardize 2nd bootloader input argument */
	populated_ep_bl33->args.arg0 = p_mtk_bl_param->bootarg_loc;
	/* compatible to old GZ version */
	populated_ep_bl33->args.arg4 = p_mtk_bl_param->bootarg_loc;
	populated_ep_bl33->args.arg5 = p_mtk_bl_param->bootarg_size;
	populated_ep_bl33->spsr = plat_get_spsr_for_bl33_entry();
}

static int populate_bl_images_ep(struct mtk_bl_param_t *p_mtk_bl_param)
{
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	populate_bl33_image_ep(&bl33_image_ep_info, p_mtk_bl_param);
	populate_bl32_image_ep(&bl32_image_ep_info, p_mtk_bl_param);
	return 0;
}

static int populate_bl_images_ep_init(void)
{
	return populate_bl_images_ep(get_mtk_bl31_fw_config(BOOT_ARG_FROM_BL2));
}
MTK_PLAT_SETUP_0_INIT(populate_bl_images_ep_init);

static entry_point_info_t *bl31_plat_get_next_kernel64_ep_info(void)
{
	entry_point_info_t *next_image_info;
	unsigned long el_status;
	unsigned int mode;

	el_status = 0;
	mode = 0;

	/* Kernel image is always non-secured */
	next_image_info = &bl33_image_ep_info;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	INFO("Kernel_EL %d\n", el_status?2:1);
	if (el_status) {
		mode = MODE_EL2;
	} else {
		mode = MODE_EL1;
	}
	INFO("Kernel is 64Bit\n");
	next_image_info->spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	next_image_info->pc = k_info.pc;
	next_image_info->args.arg0 = k_info.r0;
	next_image_info->args.arg1 = k_info.r1;

	INFO("pc=0x%lx, r0=0x%" PRIx64 ", r1=0x%" PRIx64 "\n",
	     next_image_info->pc,
	     next_image_info->args.arg0,
	     next_image_info->args.arg1);

	SET_SECURITY_STATE(next_image_info->h.attr, NON_SECURE);

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	}

	return NULL;
}

static entry_point_info_t *bl31_plat_get_next_kernel32_ep_info(void)
{
	entry_point_info_t *next_image_info;
	unsigned int mode;

	mode = 0;

	/* Kernel image is always non-secured */
	next_image_info = &bl33_image_ep_info;

	/* Figure out what mode we enter the non-secure world in */
	mode = MODE32_hyp;
	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */

	INFO("Kernel is 32Bit\n");
	next_image_info->spsr = SPSR_MODE32(mode, SPSR_T_ARM, SPSR_E_LITTLE,
					    (DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT));
	next_image_info->pc = k_info.pc;
	next_image_info->args.arg0 = k_info.r0;
	next_image_info->args.arg1 = k_info.r1;
	next_image_info->args.arg2 = k_info.r2;

	INFO("pc=0x%lx, r0=0x%" PRIx64 ", r1=0x%" PRIx64 ", r2=0x%" PRIx64 "\n",
	     next_image_info->pc,
	     next_image_info->args.arg0,
	     next_image_info->args.arg1,
	     next_image_info->args.arg2);

	SET_SECURITY_STATE(next_image_info->h.attr, NON_SECURE);

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	}

	return NULL;
}

static void bl31_prepare_kernel_entry(uint64_t k32_64)
{
	entry_point_info_t *next_image_info = NULL;
	uint32_t image_type;

	/* Determine which image to execute next */
	image_type = NON_SECURE; /* bl31_get_next_image_type(); */

	/* Leave 2nd bootloader then jump to kernel */
	el1_is_2nd_bootloader = false;

	/* Program EL3 registers to enable entry into the next EL */
	if (k32_64 == LINUX_KERNEL_32) {
		next_image_info = bl31_plat_get_next_kernel32_ep_info();
	} else {
		next_image_info = bl31_plat_get_next_kernel64_ep_info();
	}

	assert(next_image_info);
	assert(image_type == GET_SECURITY_STATE(next_image_info->h.attr));

	INFO("BL31: Preparing for EL3 exit to %s world, Kernel\n",
	     (image_type == SECURE) ? "secure" : "normal");
	INFO("BL31: Next image address = 0x%" PRIx64 "\n",
	     next_image_info->pc);
	INFO("BL31: Next image spsr = 0x%x\n", next_image_info->spsr);
	cm_init_my_context(next_image_info);
	cm_prepare_el3_exit(image_type);
}

bool is_el1_2nd_bootloader(void)
{
	return el1_is_2nd_bootloader;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	}
	return NULL;
}

u_register_t boot_to_kernel(u_register_t x1,
			    u_register_t x2,
			    u_register_t x3,
			    u_register_t x4,
			    void *handle,
			    struct smccc_res *smccc_ret)
{
	static uint8_t kernel_boot_once_flag;

	/* only support in booting flow */
	if (kernel_boot_once_flag == 0) {
		kernel_boot_once_flag = 1;

		INFO("save kernel info\n");
		save_kernel_info(x1, x2, x3, x4);
		bl31_prepare_kernel_entry(x4);
		INFO("el3_exit\n");
		/*
		 * FIXME: no better way so far to prevent from
		 * SiP root handler wipe x0~x3 if not assign smccc_ret
		 * return register
		 */
		smccc_ret->a1 = x3;

		mtk_init_one_level(MTK_INIT_LVL_BL33_DEFER);

#if MTK_CONSOLE_RUNTIME_DISABLE
		INFO("Turn off BL31 console\n");
		mtk_console_core_end();
#endif

		/* Re-assign as x0 register entering Linux kernel */
		return x2;
	}
	return 0;
}
/* Register SiP SMC service */
DECLARE_SMC_HANDLER(MTK_SIP_KERNEL_BOOT, boot_to_kernel);
