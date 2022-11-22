/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <stdlib.h>
#include <stdint.h>
#include <services/std_svc.h>
#include <string.h>
#include <platform_def.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <imx_sip_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <sci/sci.h>

#if defined(PLAT_imx8qm) || defined(PLAT_imx8qx)

#ifdef PLAT_imx8qm
static const int ap_cluster_index[PLATFORM_CLUSTER_COUNT] = {
	SC_R_A53, SC_R_A72,
};
#endif

static int imx_srtc_set_time(uint32_t year_mon,
			unsigned long day_hour,
			unsigned long min_sec)
{
	return sc_timer_set_rtc_time(ipc_handle,
		year_mon >> 16, year_mon & 0xffff,
		day_hour >> 16, day_hour & 0xffff,
		min_sec >> 16, min_sec & 0xffff);
}

int imx_srtc_handler(uint32_t smc_fid,
		    void *handle,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	int ret;

	switch (x1) {
	case IMX_SIP_SRTC_SET_TIME:
		ret = imx_srtc_set_time(x2, x3, x4);
		break;
	default:
		ret = SMC_UNK;
	}

	SMC_RET1(handle, ret);
}

static void imx_cpufreq_set_target(uint32_t cluster_id, unsigned long freq)
{
	sc_pm_clock_rate_t rate = (sc_pm_clock_rate_t)freq;

#ifdef PLAT_imx8qm
	sc_pm_set_clock_rate(ipc_handle, ap_cluster_index[cluster_id], SC_PM_CLK_CPU, &rate);
#endif
#ifdef PLAT_imx8qx
	sc_pm_set_clock_rate(ipc_handle, SC_R_A35, SC_PM_CLK_CPU, &rate);
#endif
}

int imx_cpufreq_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch (x1) {
	case IMX_SIP_SET_CPUFREQ:
		imx_cpufreq_set_target(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}

static bool wakeup_src_irqsteer;

bool imx_is_wakeup_src_irqsteer(void)
{
	return wakeup_src_irqsteer;
}

int imx_wakeup_src_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch (x1) {
	case IMX_SIP_WAKEUP_SRC_IRQSTEER:
		wakeup_src_irqsteer = true;
		break;
	case IMX_SIP_WAKEUP_SRC_SCU:
		wakeup_src_irqsteer = false;
		break;
	default:
		return SMC_UNK;
	}

	return SMC_OK;
}

int imx_otp_handler(uint32_t smc_fid,
		void *handle,
		u_register_t x1,
		u_register_t x2)
{
	int ret;
	uint32_t fuse;

	switch (smc_fid) {
	case IMX_SIP_OTP_READ:
		ret = sc_misc_otp_fuse_read(ipc_handle, x1, &fuse);
		SMC_RET2(handle, ret, fuse);
		break;
	case IMX_SIP_OTP_WRITE:
		ret = sc_misc_otp_fuse_write(ipc_handle, x1, x2);
		SMC_RET1(handle, ret);
		break;
	default:
		ret = SMC_UNK;
		SMC_RET1(handle, ret);
		break;
	}

	return ret;
}

int imx_misc_set_temp_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	return sc_misc_set_temp(ipc_handle, x1, x2, x3, x4);
}

#endif /* defined(PLAT_imx8qm) || defined(PLAT_imx8qx) */

#if defined(PLAT_imx8mm) || defined(PLAT_imx8mq)
int imx_src_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    void *handle)
{
	uint32_t val;

	switch (x1) {
	case IMX_SIP_SRC_SET_SECONDARY_BOOT:
		if (x2 != 0U) {
			mmio_setbits_32(IMX_SRC_BASE + SRC_GPR10_OFFSET,
					SRC_GPR10_PERSIST_SECONDARY_BOOT);
		} else {
			mmio_clrbits_32(IMX_SRC_BASE + SRC_GPR10_OFFSET,
					SRC_GPR10_PERSIST_SECONDARY_BOOT);
		}
		break;
	case IMX_SIP_SRC_IS_SECONDARY_BOOT:
		val = mmio_read_32(IMX_SRC_BASE + SRC_GPR10_OFFSET);
		return !!(val & SRC_GPR10_PERSIST_SECONDARY_BOOT);
	default:
		return SMC_UNK;

	};

	return 0;
}
#endif /* defined(PLAT_imx8mm) || defined(PLAT_imx8mq) */

static uint64_t imx_get_commit_hash(u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	/* Parse the version_string */
	char *parse = (char *)version_string;
	uint64_t hash = 0;

	do {
		parse = strchr(parse, '-');
		if (parse) {
			parse += 1;
			if (*(parse) == 'g') {
				/* Default is 7 hexadecimal digits */
				memcpy((void *)&hash, (void *)(parse + 1), 7);
				break;
			}
		}

	} while (parse != NULL);

	return hash;
}

uint64_t imx_buildinfo_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	uint64_t ret;

	switch (x1) {
	case IMX_SIP_BUILDINFO_GET_COMMITHASH:
		ret = imx_get_commit_hash(x2, x3, x4);
		break;
	default:
		return SMC_UNK;
	}

	return ret;
}

int imx_kernel_entry_handler(uint32_t smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4)
{
	static entry_point_info_t bl33_image_ep_info;
	entry_point_info_t *next_image_info;
	unsigned int mode;

	if (x1 < (PLAT_NS_IMAGE_OFFSET & 0xF0000000))
		return SMC_UNK;

	mode = MODE32_svc;

	next_image_info = &bl33_image_ep_info;

	next_image_info->pc = x1;

	next_image_info->spsr = SPSR_MODE32(mode, SPSR_T_ARM, SPSR_E_LITTLE,
			(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT));

	next_image_info->args.arg0 = 0;
	next_image_info->args.arg1 = 0;
	next_image_info->args.arg2 = x3;

	SET_SECURITY_STATE(next_image_info->h.attr, NON_SECURE);

	cm_init_my_context(next_image_info);
	cm_prepare_el3_exit(NON_SECURE);

	return 0;
}
