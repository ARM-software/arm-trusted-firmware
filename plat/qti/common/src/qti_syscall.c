/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <debug.h>
#include <context.h>
#include <xlat_tables_v2.h>
#include <runtime_svc.h>
#include <uuid.h>
#include <smccc_helpers.h>
#include <qtiseclib_interface.h>

/*----------------------------------------------------------------------------
 * SIP service - SMC function IDs for SiP Service queries
 * -------------------------------------------------------------------------*/
#define	QTI_SIP_SVC_CALL_COUNT_ID			U(0x0200ff00)
#define	QTI_SIP_SVC_UID_ID				U(0x0200ff01)
/*							0x8200ff02 is reserved */
#define	QTI_SIP_SVC_VERSION_ID				U(0x0200ff03)

#define	FUNCID_OEN_NUM_MASK  ((FUNCID_OEN_MASK << FUNCID_OEN_SHIFT)\
				|(FUNCID_NUM_MASK << FUNCID_NUM_SHIFT) )

/* QTI SiP Service UUID */
DEFINE_SVC_UUID2(qti_sip_svc_uid,
		0x43864748, 0x217f, 0x41ad, 0xaa, 0x5a,
		0xba, 0xe7, 0x0f, 0xa5, 0x52, 0xaf);

/*
 * This function handles QTI specific syscalls. Currently only SiP calls are present.
 * Both FAST & YIELD type call land here.
 */
static uintptr_t qti_sip_handler(uint32_t smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie, void *handle, u_register_t flags)
{
	uint32_t l_smc_fid = smc_fid & FUNCID_OEN_NUM_MASK;

	switch (l_smc_fid) {
	case QTI_SIP_SVC_CALL_COUNT_ID:
		{
			SMC_RET1(handle, QTISECLIB_SIP_SVC_CALL_COUNT);
		}

	case QTI_SIP_SVC_UID_ID:
		{
			/* Return UID to the caller */
			SMC_UUID_RET(handle, qti_sip_svc_uid);
		}

	case QTI_SIP_SVC_VERSION_ID:
		{
			/* Return the version of current implementation */
			SMC_RET2(handle, QTISECLIB_SIP_SVC_VERSION_MAJOR,
				 QTISECLIB_SIP_SVC_VERSION_MINOR);
		}
	default:
		{
			const gp_regs_t *regs = get_gpregs_ctx(handle);

			if (NULL != regs) {
				qtiseclib_smc_rsp_t rsps;

				QTISECLIB_SMC_RSP1((&rsps), SMC_UNK);	/* Default return failure. */

				qtiseclib_smc_param_t params;

				QTISECLIB_SMC_PARAM((&params), 0) = smc_fid;
				QTISECLIB_SMC_PARAM((&params), 1) = x1;
				QTISECLIB_SMC_PARAM((&params), 2) = x2;
				QTISECLIB_SMC_PARAM((&params), 3) = x3;
				QTISECLIB_SMC_PARAM((&params), 4) = x4;
				QTISECLIB_SMC_PARAM((&params), 5) =
				    read_ctx_reg(regs, CTX_GPREG_X5);
				QTISECLIB_SMC_PARAM((&params), 6) =
				    read_ctx_reg(regs, CTX_GPREG_X6);
				QTISECLIB_SMC_PARAM((&params), 7) =
				    read_ctx_reg(regs, CTX_GPREG_X7);
				QTISECLIB_SMC_PARAM((&params), 8) =
				    read_ctx_reg(regs, CTX_GPREG_X8);
				qtiseclib_sip_syscall((&params), &rsps);

				SMC_RET4(handle, rsps.rsp[0], rsps.rsp[1],
					 rsps.rsp[2], rsps.rsp[3])
			} else {
				SMC_RET1(handle, SMC_UNK);
			}
		}
	}
	return (uintptr_t) handle;
}

/* Define a runtime service descriptor for both fast & yield SiP calls */
DECLARE_RT_SVC(qti_sip_fast_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_FAST, NULL, qti_sip_handler);

DECLARE_RT_SVC(qti_sip_yield_svc, OEN_SIP_START,
	       OEN_SIP_END, SMC_TYPE_YIELD, NULL, qti_sip_handler);
