/*
 * Copyright (c) 2023, Texas Instruments. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdio.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/scmi-msg.h>

#include <lib/mmio.h>
#include <tools_share/uuid.h>
#include <k3_sip_svc.h>
#include <ti_sci.h>

/* K3 SiP Service UUID */
DEFINE_SVC_UUID2(ti_sip_svc_uid,
    0xe2c76fe8, 0x3e31, 0xe611, 0xb7, 0x0d,
    0x8f, 0x88, 0xee, 0x74, 0x7b, 0x72);


/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uintptr_t sip_smc_handler(uint32_t smc_fid,
			  u_register_t x1,
			  u_register_t x2,
			  u_register_t x3,
			  u_register_t x4,
			  void *cookie,
			  void *handle,
			  u_register_t flags)
{
	int ret = SMC_UNK;

        VERBOSE("sim_smc handler smc_fid = %u\n", smc_fid);
        switch (smc_fid) {
	case SIP_SVC_CALL_COUNT:
		/* Return the number of TI K3 SiP Service Calls. */
		SMC_RET1(handle, K3_COMMON_SIP_NUM_CALLS);

	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, ti_sip_svc_uid);

	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, K3_SIP_SVC_VERSION_MAJOR,
			 K3_SIP_SVC_VERSION_MINOR);

	case K3_SIP_SCMI_AGENT0:
		scmi_smt_fastcall_smc_entry(0);
		SMC_RET1(handle, 0);

	case K3_SIP_OTP_WRITEBUFF:
		SMC_RET1(handle, ti_fuse_writebuff_handler(x1));

	case K3_SIP_OTP_READ:
		uint32_t mmr_val = 0;

		/*
		 * 0x00 - 0xFE is reserved for user OTP, 0xFF is
		 * reserved for bootmode OTP which doesnt support
		 * readback
		 */
		if (x1 < 0xff)
			ret = ti_sci_read_otp(x1, x2, &mmr_val);

		SMC_RET2(handle, ret, mmr_val);

	case K3_SIP_OTP_WRITE:
		/*
		 * 0x00 - 0xFE is reserved for user OTP, 0xFF is
		 * reserved for bootmode OTP programming
		 */
		if (x1 < 0xff) {
			uint32_t row_val = 0;

			ret = ti_sci_write_otp(x1, x2, x3, x4, &row_val);

			SMC_RET2(handle, ret, row_val);
		}

		if (x1 == 0xff)
			SMC_RET2(handle, ti_sci_set_otp_bootmode(x2, x3), x3);

		SMC_RET1(handle, SMC_UNK);

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
    k3_svc,
    OEN_SIP_START,
    OEN_SIP_END,
    SMC_TYPE_FAST,
    NULL,
    sip_smc_handler
);
