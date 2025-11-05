/*
 * Copyright (c) 2015-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/usb_dwc3.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "usb_dwc3_regs.h"

/*
 * @brief USB EP Type
 */
#define EP_TYPE_CTRL	0U
#define EP_TYPE_ISOC	1U
#define EP_TYPE_BULK	2U
#define EP_TYPE_INTR	3U
#define EP_TYPE_MSK	3U

#define USB_DWC3_GLOBAL_BASE	0xc100
#define USB_DWC3_DEVICE_BASE	0xc700

/* Global Registers */
#define DWC3_GSBUSCFG0	_DWC3_GSBUSCFG0
#define DWC3_GSBUSCFG1	_DWC3_GSBUSCFG1
#define DWC3_GTXTHRCFG	_DWC3_GTXTHRCFG
#define DWC3_GRXTHRCFG	_DWC3_GRXTHRCFG
#define DWC3_GCTL	_DWC3_GCTL
#define DWC3_GEVTEN	_DWC3_GEVTEN
#define DWC3_GSTS	_DWC3_GSTS
#define DWC3_GUCTL1	_DWC3_GUCTL1
#define DWC3_GSNPSID	_DWC3_GSNPSID
#define DWC3_GGPIO	_DWC3_GGPIO
#define DWC3_GUID	_DWC3_GUID
#define DWC3_GUCTL	_DWC3_GUCTL
#define DWC3_GBUSERRADDR0	_DWC3_GBUSERRADDRLO
#define DWC3_GBUSERRADDR1	_DWC3_GBUSERRADDRHI
#define DWC3_GPRTBIMAP0	_DWC3_GPRTBIMAPLO
#define DWC3_GPRTBIMAP1	_DWC3_GPRTBIMAPHI
#define DWC3_GHWPARAMS0	_DWC3_GHWPARAMS0
#define DWC3_GHWPARAMS1	_DWC3_GHWPARAMS1
#define DWC3_GHWPARAMS2	_DWC3_GHWPARAMS2
#define DWC3_GHWPARAMS3	_DWC3_GHWPARAMS3
#define DWC3_GHWPARAMS4	_DWC3_GHWPARAMS4
#define DWC3_GHWPARAMS5	_DWC3_GHWPARAMS5
#define DWC3_GHWPARAMS6	_DWC3_GHWPARAMS6
#define DWC3_GHWPARAMS7	_DWC3_GHWPARAMS7
#define DWC3_GDBGFIFOSPACE	_DWC3_GDBGFIFOSPACE
#define DWC3_GDBGLTSSM	_DWC3_GDBGLTSSM
#define DWC3_GPRTBIMAP_HS0	_DWC3_GPRTBIMAP_HSLO
#define DWC3_GPRTBIMAP_HS1	_DWC3_GPRTBIMAP_HSHI
#define DWC3_GPRTBIMAP_FS0	_DWC3_GPRTBIMAP_FSLO
#define DWC3_GPRTBIMAP_FS1	_DWC3_GPRTBIMAP_FSHI
#define DWC3_GUCTL2	_DWC3_GUCTL2

#define DWC3_GUSB2PHYCFG(n)	(_DWC3_GUSB2PHYCFG + (4UL * (n)))
#define DWC3_GUSB2I2CCTL(n)	(_DWC3_GUSB2I2CCTL + (4UL * (n)))

#define DWC3_GUSB2PHYACC(n)	(_DWC3_GUSB2PHYACC + (4UL * (n)))

#define DWC3_GUSB3PIPECTL(n)	(_DWC3_GUSB3PIPECTL + (4UL * (n)))

#define DWC3_GTXFIFOSIZ(n)	(_DWC3_GTXFIFOSIZ0 + (4UL * (n)))
#define DWC3_GRXFIFOSIZ(n)	(_DWC3_GRXFIFOSIZ0 + (4UL * (n)))

#define DWC3_GEVNTADRLO(n)	(_DWC3_GEVNTADRLO + (16UL * (n)))
#define DWC3_GEVNTADRHI(n)	(_DWC3_GEVNTADRHI + (16UL * (n)))
#define DWC3_GEVNTSIZ(n)	(_DWC3_GEVNTSIZ + (16UL * (n)))
#define DWC3_GEVNTCOUNT(n)	(_DWC3_GEVNTCOUNT + (16UL * (n)))

#define DWC3_GUSB2PHYACC_ADDR(n)	((n) << USB3_GUSB2PHYACC_ULPI_REGADDR_POS)
#define DWC3_GUSB2PHYACC_EXTEND_ADDR(n)	((n) << USB3_GUSB2PHYACC_ULPI_EXTREGADDR_POS)
#define DWC3_GUSB2PHYACC_DATA(n)	((n) & USB3_GUSB2PHYACC_ULPI_REGDATA_MSK)

#define DWC3_GUCTL2_RST_ACTBITLATER	_DWC3_GUCTL2_RST_ACTBITLATER

/* Device Registers */
#define DWC3_DCFG	_DWC3_DCFG
#define DWC3_DCTL	_DWC3_DCTL
#define DWC3_DEVTEN	_DWC3_DEVTEN
#define DWC3_DSTS	_DWC3_DSTS
#define DWC3_DGCMDPAR	_DWC3_DGCMDPAR
#define DWC3_DGCMD	_DWC3_DGCMD
#define DWC3_DALEPENA	_DWC3_DALEPENA
#define DWC3_DEPCMDPAR2(n)	(_DWC3_DEPCMDPAR2 + (16UL * (n)))
#define DWC3_DEPCMDPAR1(n)	(_DWC3_DEPCMDPAR1 + (16UL * (n)))
#define DWC3_DEPCMDPAR0(n)	(_DWC3_DEPCMDPAR0 + (16UL * (n)))
#define DWC3_DEPCMD(n)	(_DWC3_DEPCMD + (16UL * (n)))

/* The EP number goes 0..31 so ep0 is always out and ep1 is always in */
#define DWC3_DALEPENA_EP(n)	(1UL << (n))

/*
 * @brief USB EP Type
 */
#define EP_TYPE_CTRL	0U
#define EP_TYPE_ISOC	1U
#define EP_TYPE_BULK	2U
#define EP_TYPE_INTR	3U
#define EP_TYPE_MSK	3U

/* Event Size */
#define USB_DWC3_EVENT_SIZE	4U

/* USB_ExecuteDevEPCmd::USB_DWC3_EPCmd */
#define USB_DWC3_DEPCMD_DEPSTARTCFG	(0x09UL << USB3_DEPCMD_CMDTYP_POS)
#define USB_DWC3_DEPCMD_ENDTRANSFER	(0x08UL << USB3_DEPCMD_CMDTYP_POS)
#define USB_DWC3_DEPCMD_UPDATETRANSFER	(0x07UL << USB3_DEPCMD_CMDTYP_POS)
#define USB_DWC3_DEPCMD_STARTTRANSFER	(0x06UL << USB3_DEPCMD_CMDTYP_POS)
#define USB_DWC3_DEPCMD_CLEARSTALL	(0x05UL << USB3_DEPCMD_CMDTYP_POS)
#define USB_DWC3_DEPCMD_SETSTALL	(0x04UL << USB3_DEPCMD_CMDTYP_POS)
/* This applies for core versions 1.94a and later */
#define USB_DWC3_DEPCMD_GETEPSTATE	(0x03UL << USB3_DEPCMD_CMDTYP_POS)
#define USB_DWC3_DEPCMD_SETTRANSFRESOURCE	(0x02UL << USB3_DEPCMD_CMDTYP_POS)
#define USB_DWC3_DEPCMD_SETEPCONFIG	(0x01UL << USB3_DEPCMD_CMDTYP_POS)

/* USB_ConfigureEP::action */
#define USB_DWC3_DEPCFG_ACTION_INIT	0
#define USB_DWC3_DEPCFG_ACTION_RESTORE	1
#define USB_DWC3_DEPCFG_ACTION_MODIFY	2

/* USB_ReadEndpointEventType @return EPEventType */
#define USB_DWC3_DEPEVT_XFERCOMPLETE	0x01
#define USB_DWC3_DEPEVT_XFERINPROGRESS	0x02
#define USB_DWC3_DEPEVT_XFERNOTREADY	0x03
#define USB_DWC3_DEPEVT_RXTXFIFOEVT	0x04
#define USB_DWC3_DEPEVT_STREAMEVT	0x06
#define USB_DWC3_DEPEVT_EPCMDCMPLT	0x07

/* USB_ReadEndpointEventStatus @return EPEventStatus */
#define USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_DATA	0x01
#define USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_STATUS	0x02

/* USB_ReadDeviceEventType @return DevEventType */
#define USB_DWC3_DEVICE_EVENT_DISCONNECT	0
#define USB_DWC3_DEVICE_EVENT_RESET	1
#define USB_DWC3_DEVICE_EVENT_CONNECT_DONE	2
#define USB_DWC3_DEVICE_EVENT_LINK_STATUS_CHANGE	3
#define USB_DWC3_DEVICE_EVENT_WAKEUP	4
#define USB_DWC3_DEVICE_EVENT_HIBER_REQ	5
#define USB_DWC3_DEVICE_EVENT_EOPF	6
#define USB_DWC3_DEVICE_EVENT_SOF	7
#define USB_DWC3_DEVICE_EVENT_L1SUSPEND	8
#define USB_DWC3_DEVICE_EVENT_ERRATIC_ERROR	9
#define USB_DWC3_DEVICE_EVENT_CMD_CMPL	10
#define USB_DWC3_DEVICE_EVENT_OVERFLOW	11
#define USB_DWC3_DEVICE_EVENT_L1WAKEUP	14

#define USB_DWC3_TRBSTS_OK	0
#define USB_DWC3_TRBSTS_MISSED_ISOC	1
#define USB_DWC3_TRBSTS_SETUP_PENDING	2
#define USB_DWC3_TRBSTS_XFER_IN_PROG	4

#define USB_DWC3_SPEED_SUPER	0U
#define USB_DWC3_SPEED_HIGH	1U
#define USB_DWC3_SPEED_FULL_48	2U
#define USB_DWC3_SPEED_FULL	3U
#define USB_DWC3_SPEED_LOW	4U
#define USB_DWC3_SPEED_INVALID	15U

#define DWC3_DEPCMD_TYPE_CONTROL	0
#define DWC3_DEPCMD_TYPE_ISOC	1
#define DWC3_DEPCMD_TYPE_BULK	2
#define DWC3_DEPCMD_TYPE_INTR	3

#define DWC3_GCTL_PRTCAP(n)	(((n) & USB3_GCTL_PRTCAPDIR_MSK) >> USB3_GCTL_PRTCAPDIR_POS)
#define DWC3_GCTL_PRTCAPDIR(n)	((n) << USB3_GCTL_PRTCAPDIR_POS)
#define DWC3_GCTL_PRTCAP_HOST	1
#define DWC3_GCTL_PRTCAP_DEVICE	2
#define DWC3_GCTL_PRTCAP_OTG	3

#define DWC3_DSTS_SUPERSPEED	(4U << USB3_DSTS_CONNECTSPD_POS)
#define DWC3_DSTS_HIGHSPEED	(0U << USB3_DSTS_CONNECTSPD_POS)
#define DWC3_DSTS_FULLSPEED2	BIT(USB3_DSTS_CONNECTSPD_POS)
#define DWC3_DSTS_LOWSPEED	(2U << USB3_DSTS_CONNECTSPD_POS)
#define DWC3_DSTS_FULLSPEED1	(3U << USB3_DSTS_CONNECTSPD_POS)

#define DWC3_GEVNTCOUNT_MASK	0xfffcU
#define DWC3_GSNPSID_MASK	0xffff0000
#define DWC3_GSNPSREV_MASK	0xffff

#define DWC3_DCFG_DEVADDR(addr)	((addr) << 3)

#define DWC3_EVENT_TYPE_DEV	0
#define DWC3_EVENT_TYPE_CARKIT	3
#define DWC3_EVENT_TYPE_I2C	4

#define DWC3_DEVICE_EVENT_DISCONNECT	0
#define DWC3_DEVICE_EVENT_RESET	1
#define DWC3_DEVICE_EVENT_CONNECT_DONE	2
#define DWC3_DEVICE_EVENT_LINK_STATUS_CHANGE	3
#define DWC3_DEVICE_EVENT_WAKEUP	4
#define DWC3_DEVICE_EVENT_HIBER_REQ	5
#define DWC3_DEVICE_EVENT_EOPF	6
#define DWC3_DEVICE_EVENT_SOF	7
#define DWC3_DEVICE_EVENT_ERRATIC_ERROR	9
#define DWC3_DEVICE_EVENT_CMD_CMPL	10
#define DWC3_DEVICE_EVENT_OVERFLOW	11

/* DEPCFG parameter 1 */
#define DWC3_DEPCFG_INT_NUM(n)	(((n) & 0x1f) << 0)
#define DWC3_DEPCFG_XFER_COMPLETE_EN	BIT(8)
#define DWC3_DEPCFG_XFER_IN_PROGRESS_EN	BIT(9)
#define DWC3_DEPCFG_XFER_NOT_READY_EN	BIT(10)
#define DWC3_DEPCFG_FIFO_ERROR_EN	BIT(11)
#define DWC3_DEPCFG_STREAM_EVENT_EN	BIT(13)
#define DWC3_DEPCFG_BINTERVAL_M1(n)	(((n) & 0xffU) << 16)
#define DWC3_DEPCFG_STREAM_CAPABLE	BIT(24)
#define DWC3_DEPCFG_EP_NUMBER(n)	(((n) & 0x1fUL) << 25)
#define DWC3_DEPCFG_BULK_BASED	BIT(30)
#define DWC3_DEPCFG_FIFO_BASED	BIT(31)

/* DEPCFG parameter 0 */
#define DWC3_DEPCFG_EP_TYPE(n)	(((n) & 0x3UL) << 1)
#define DWC3_DEPCFG_MAX_PACKET_SIZE(n)	(((n) & 0x7ffU) << 3)
#define DWC3_DEPCFG_FIFO_NUMBER(n)	(((n) & 0x1fUL) << 17)
#define DWC3_DEPCFG_BURST_SIZE(n)	(((n) & 0xf) << 22)
#define DWC3_DEPCFG_DATA_SEQ_NUM(n)	((n) << 26)
/* DEPCFG parameter 1 */
#define DWC3_DEPCFG_EP_INTR_NUM(n)	((n) & 0x1fUL)
/* This applies for core versions earlier than 1.94a */
#define DWC3_DEPCFG_IGN_SEQ_NUM	BIT(31)
/* These apply for core versions 1.94a and later */
#define DWC3_DEPCFG_ACTION_INIT	(0U << 30)
#define DWC3_DEPCFG_ACTION_RESTORE	BIT(30)
#define DWC3_DEPCFG_ACTION_MODIFY	(2U << 30)
#define DWC3_DEPCFG_ACTION_VALUE(n)	(((n) & 0x3) << 30)

/* DEPXFERCFG parameter 0 */
#define DWC3_DEPXFERCFG_NUM_XFER_RES(n)	((n) & 0xffffU)

#define DWC3_DEPCMD_GET_RSC_IDX(x)	(((x) >> USB3_DEPCMD_COMMANDPARAM_POS) & 0x7fU)
#define DWC3_DEPCMD_STATUS(x)	(((x) & USB3_DEPCMD_CMDSTATUS_MSK) >> 12)
#define DWC3_DGCMD_STATUS(n)	(((n) & USB3_DGCMD_CMDSTATUS_MSK) >> USB3_DGCMD_CMDSTATUS_POS)

#define DWC3_DEPCMD_PARAM(x)	((x) << USB3_DEPCMD_COMMANDPARAM_POS)

#define DWC3_LINK_STATE_SS_U0	0x00 /* in HS, means ON */
#define DWC3_LINK_STATE_SS_U1	0x01
#define DWC3_LINK_STATE_SS_U2	0x02 /* in HS, means SLEEP */
#define DWC3_LINK_STATE_SS_U3	0x03 /* in HS, means SUSPEND */
#define DWC3_LINK_STATE_SS_DIS	0x04 /* Default State */
#define DWC3_LINK_STATE_SS_RX_DET	0x05 /* in HS, means Early Suspend */
#define DWC3_LINK_STATE_SS_INACT	0x06
#define DWC3_LINK_STATE_SS_POLL	0x07
#define DWC3_LINK_STATE_SS_RECOV	0x08
#define DWC3_LINK_STATE_SS_HRESET	0x09
#define DWC3_LINK_STATE_SS_CMPLY	0x0a
#define DWC3_LINK_STATE_SS_LPBK	0x0b
#define DWC3_LINK_STATE_SS_RESET	0x0e
#define DWC3_LINK_STATE_SS_RESUME	0x0f

#define DWC3_LINK_STATE_ON	0x00U /* in HS, means ON */
#define DWC3_LINK_STATE_L1	0x02U /* in HS, means SLEEP */
#define DWC3_LINK_STATE_L2	0x03U /* in HS, means SUSPEND */
#define DWC3_LINK_STATE_DIS	0x04U /* Default State */
#define DWC3_LINK_STATE_EARLY_SUS	0x05U /* in HS, means Early Suspend */
#define DWC3_LINK_STATE_RESET	0x0eU
#define DWC3_LINK_STATE_RESUME	0x0fU

#define DWC3_DCTL_ULSTCHNG_NO_ACTION	0
#define DWC3_DCTL_ULSTCHNG_SS_DISABLED	4
#define DWC3_DCTL_ULSTCHNG_RX_DETECT	5
#define DWC3_DCTL_ULSTCHNG_SS_INACTIVE	6
#define DWC3_DCTL_ULSTCHNG_RECOVERY	8
#define DWC3_DCTL_ULSTCHNG_COMPLIANCE	10
#define DWC3_DCTL_ULSTCHNG_LOOPBACK	11

#define DWC3_DCTL_ULSTCHNGREQ(n)	(((n) << USB3_DCTL_ULSTCHNGREQ_POS) & \
					USB3_DCTL_ULSTCHNGREQ_MSK)

#define DWC3_DSTS_USBLNKST(n)	(((n) & USB3_DSTS_USBLNKST_MSK) >> USB3_DSTS_USBLNKST_POS)

/* TRB Length, PCM and Status */
#define DWC3_TRB_SIZE_MASK	(0x00ffffffU)
#define DWC3_TRB_SIZE_LENGTH(n)	((n) & DWC3_TRB_SIZE_MASK)
#define DWC3_TRB_SIZE_PCM1(n)	(((n) & 0x03) << 24)
#define DWC3_TRB_SIZE_TRBSTS(n)	(((n) & (0x0fU << 28)) >> 28)

#define DWC3_TRBSTS_OK	0
#define DWC3_TRBSTS_MISSED_ISOC	1
#define DWC3_TRBSTS_SETUP_PENDING	2
#define DWC3_TRBSTS_XFER_IN_PROG	4

/* TRB Control */
#define DWC3_TRB_CTRL_HWO	BIT(0)
#define DWC3_TRB_CTRL_LST	BIT(1)
#define DWC3_TRB_CTRL_CHN	BIT(2)
#define DWC3_TRB_CTRL_CSP	BIT(3)
#define DWC3_TRB_CTRL_TRBCTL(n)	(((n) & 0x3fU) << 4)
#define DWC3_TRB_CTRL_ISP_IMI	BIT(10)
#define DWC3_TRB_CTRL_IOC	BIT(11)
#define DWC3_TRB_CTRL_SID_SOFN(n)	(((n) & 0xffffU) << 14)

#define DWC3_TRBCTL_NORMAL	DWC3_TRB_CTRL_TRBCTL(1U)
#define DWC3_TRBCTL_CONTROL_SETUP	DWC3_TRB_CTRL_TRBCTL(2U)
#define DWC3_TRBCTL_CONTROL_STATUS2	DWC3_TRB_CTRL_TRBCTL(3U)
#define DWC3_TRBCTL_CONTROL_STATUS3	DWC3_TRB_CTRL_TRBCTL(4U)
#define DWC3_TRBCTL_CONTROL_DATA	DWC3_TRB_CTRL_TRBCTL(5U)
#define DWC3_TRBCTL_ISOCHRONOUS_FIRST	DWC3_TRB_CTRL_TRBCTL(6U)
#define DWC3_TRBCTL_ISOCHRONOUS	DWC3_TRB_CTRL_TRBCTL(7U)
#define DWC3_TRBCTL_LINK_TRB	DWC3_TRB_CTRL_TRBCTL(8U)

#define _MASK(len, pos)	GENMASK_32((len-1) + pos, pos)

/* event */
#define DWC3_EVT_TYPE_EP	0x0U
#define DWC3_EVT_TYPE_LEN	0x1
#define DWC3_EVT_TYPE_BITPOS	0x0
#define DWC3_EVT_TYPE_MASK	_MASK(DWC3_EVT_TYPE_LEN, DWC3_EVT_TYPE_BITPOS)
#define DWC3_EVT_TYPE_DEVSPEC	0x1U
#define DWC3_EVT_TYPE_NEP_TYPE_BITPOS	1
#define DWC3_EVT_TYPE_NEP_TYPE_LEN	7
#define DWC3_EVT_TYPE_NEP_TYPE_MASK	_MASK(DWC3_EVT_TYPE_NEP_TYPE_LEN, \
					      DWC3_EVT_TYPE_NEP_TYPE_BITPOS)

#define DWC3_DEPEVT_XFERCOMPLETE	0x01
#define DWC3_DEPEVT_XFERINPROGRESS	0x02
#define DWC3_DEPEVT_XFERNOTREADY	0x03
#define DWC3_DEPEVT_RXTXFIFOEVT	0x04
#define DWC3_DEPEVT_STREAMEVT	0x06
#define DWC3_DEPEVT_EPCMDCMPLT	0x07

#define DWC3_EVT_DEPEVT_TYPE_BITPOS	6
#define DWC3_EVT_DEPEVT_TYPE_LEN	4
#define DWC3_EVT_DEPEVT_TYPE_MASK	_MASK(DWC3_EVT_DEPEVT_TYPE_LEN, DWC3_EVT_DEPEVT_TYPE_BITPOS)

#define DWC3_EVT_DEPEVT_EPNUM_BITPOS	1
#define DWC3_EVT_DEPEVT_EPNUM_LEN	5
#define DWC3_EVT_DEPEVT_EPNUM_MASK	_MASK(DWC3_EVT_DEPEVT_EPNUM_LEN, \
					      DWC3_EVT_DEPEVT_EPNUM_BITPOS)

#define DWC3_EVT_DEPEVT_STATUS_BITPOS	12
#define DWC3_EVT_DEPEVT_STATUS_LEN	4
#define DWC3_EVT_DEPEVT_STATUS_MASK	_MASK(DWC3_EVT_DEPEVT_STATUS_LEN, \
					      DWC3_EVT_DEPEVT_STATUS_BITPOS)

/* Control-only Status */
#define DWC3_EVT_DEPEVT_STATUS_CONTROL_DATA	1
#define DWC3_EVT_DEPEVT_STATUS_CONTROL_STATUS	2

#define DWC3_EVT_DEPEVT_PARAM_BITPOS	16
#define DWC3_EVT_DEPEVT_PARAM_LEN	16
#define DWC3_EVT_DEPEVT_PARAM_MASK	_MASK(DWC3_EVT_DEPEVT_PARAM_LEN, \
					      DWC3_EVT_DEPEVT_PARAM_BITPOS)

#define DWC3_EVT_DEVEVT_TYPE_BITPOS	8
#define DWC3_EVT_DEVEVT_TYPE_LEN	4
#define DWC3_EVT_DEVEVT_TYPE_MASK	_MASK(DWC3_EVT_DEVEVT_TYPE_LEN, DWC3_EVT_DEVEVT_TYPE_BITPOS)

#define DWC3_EVT_DEVEVT_LNKSTS_BITPOS	16
#define DWC3_EVT_DEVEVT_LNKSTS_LEN	4
#define DWC3_EVT_DEVEVT_LNKSTS_MASK	_MASK(DWC3_EVT_DEVEVT_LNKSTS_LEN, \
					DWC3_EVT_DEVEVT_LNKSTS_BITPOS)

/* Bit fields for USB3_GCTL register */
#define USB3_GCTL_CORESOFTRESET	_DWC3_GCTL_CORESOFTRESET
#define USB3_GCTL_PRTCAPDIR_POS	_DWC3_GCTL_PRTCAPDIR_SHIFT
#define USB3_GCTL_PRTCAPDIR_MSK	_DWC3_GCTL_PRTCAPDIR_MASK

/* Bit fields for USB3_GUSB2PHYCFG register */
#define USB3_GUSB2PHYCFG_ULPI_UTMI_SEL	_DWC3_GUSB2PHYCFG_ULPI_UTMI_SEL
#define USB3_GUSB2PHYCFG_PHYSOFTRST	_DWC3_GUSB2PHYCFG_PHYSOFTRST

/* Bit fields for USB3_GUSB3PIPECTL register */
#define USB3_GUSB3PIPECTL_PHYSOFTRST	_DWC3_GUSB3PIPECTL_PHYSOFTRST

/* Bit fields for USB3_GEVNTSIZ register */
#define USB3_GEVNTSIZ_EVNTINTRPTMASK_MSK	_DWC3_GEVNTSIZ_EVNTINTRPTMASK

/* Bit fields for USB3_DCFG register */
#define USB3_DCFG_DEVSPD_POS	_DWC3_DCFG_DEVSPD_SHIFT
#define USB3_DCFG_DEVSPD_MSK	_DWC3_DCFG_DEVSPD_MASK
#define USB3_DCFG_DEVADDR_POS	_DWC3_DCFG_DEVADDR_SHIFT
#define USB3_DCFG_DEVADDR_MSK	_DWC3_DCFG_DEVADDR_MASK
#define USB3_DCFG_INTRNUM_POS	_DWC3_DCFG_INTRNUM_SHIFT
#define USB3_DCFG_INTRNUM_MSK	_DWC3_DCFG_INTRNUM_MASK
#define USB3_DCFG_NUMP_POS	_DWC3_DCFG_NUMP_SHIFT
#define USB3_DCFG_NUMP_MSK	_DWC3_DCFG_NUMP_MASK
#define USB3_DCFG_LPMCAP	_DWC3_DCFG_LPMCAP
#define USB3_DCFG_IGNSTRMPP	_DWC3_DCFG_IGNSTRMPP

/* Bit fields for USB3_DCTL register */
#define USB3_DCTL_TSTCTL_POS	_DWC3_DCTL_TSTCTL_SHIFT
#define USB3_DCTL_TSTCTL_MSK	_DWC3_DCTL_TSTCTL_MASK
#define USB3_DCTL_ULSTCHNGREQ_POS	_DWC3_DCTL_ULSTCHNGREQ_SHIFT
#define USB3_DCTL_ULSTCHNGREQ_MSK	_DWC3_DCTL_ULSTCHNGREQ_MASK
#define USB3_DCTL_ACCEPTU1ENA	_DWC3_DCTL_ACCEPTU1ENA
#define USB3_DCTL_INITU1ENA	_DWC3_DCTL_INITU1ENA
#define USB3_DCTL_ACCEPTU2ENA	_DWC3_DCTL_ACCEPTU2ENA
#define USB3_DCTL_INITU2ENA	_DWC3_DCTL_INITU2ENA
#define USB3_DCTL_CSS	_DWC3_DCTL_CSS
#define USB3_DCTL_CRS	_DWC3_DCTL_CRS
#define USB3_DCTL_L1HIBERNATIONEN	_DWC3_DCTL_L1HIBERNATIONEN
#define USB3_DCTL_KEEPCONNECT	_DWC3_DCTL_KEEPCONNECT
#define USB3_DCTL_LPM_NYET_THRES_POS	_DWC3_DCTL_LPM_NYET_THRES_SHIFT
#define USB3_DCTL_LPM_NYET_THRES_MSK	_DWC3_DCTL_LPM_NYET_THRES_MASK
#define USB3_DCTL_HIRDTHRES_POS	_DWC3_DCTL_HIRDTHRES_SHIFT
#define USB3_DCTL_HIRDTHRES_MSK	_DWC3_DCTL_HIRDTHRES_MASK
#define USB3_DCTL_CSFTRST	_DWC3_DCTL_CSFTRST
#define USB3_DCTL_RUN_STOP	_DWC3_DCTL_RUN_STOP

/* Bit fields for USB3_DEVTEN register */
#define USB3_DEVTEN_DISSCONNEVTEN	_DWC3_DEVTEN_DISSCONNEVTEN
#define USB3_DEVTEN_USBRSTEVTEN	_DWC3_DEVTEN_USBRSTEVTEN
#define USB3_DEVTEN_CONNECTDONEEVTEN	_DWC3_DEVTEN_CONNECTDONEEVTEN
#define USB3_DEVTEN_ULSTCNGEN	_DWC3_DEVTEN_ULSTCNGEN
#define USB3_DEVTEN_WKUPEVTEN	_DWC3_DEVTEN_WKUPEVTEN
#define USB3_DEVTEN_HIBERNATIONREQEVTEN	_DWC3_DEVTEN_HIBERNATIONREQEVTEN
#define USB3_DEVTEN_U3L2L1SUSPEN	_DWC3_DEVTEN_U3L2L1SUSPEN
#define USB3_DEVTEN_SOFTEVTEN	_DWC3_DEVTEN_SOFTEVTEN
#define USB3_DEVTEN_L1SUSPEN	_DWC3_DEVTEN_L1SUSPEN
#define USB3_DEVTEN_ERRTICERREVTEN	_DWC3_DEVTEN_ERRTICERREVTEN
#define USB3_DEVTEN_CMDCMPLTEN	_DWC3_DEVTEN_CMDCMPLTEN
#define USB3_DEVTEN_EVNTOVERFLOWEN	_DWC3_DEVTEN_EVNTOVERFLOWEN
#define USB3_DEVTEN_VENDEVTSTRCVDEN	_DWC3_DEVTEN_VENDEVTSTRCVDEN
#define USB3_DEVTEN_L1WKUPEVTEN	_DWC3_DEVTEN_L1WKUPEVTEN
#define USB3_DEVTEN_ECCERREN	_DWC3_DEVTEN_ECCERREN

/* Bit fields for USB3_DSTS register */
#define USB3_DSTS_CONNECTSPD_POS	_DWC3_DSTS_CONNECTSPD_SHIFT
#define USB3_DSTS_CONNECTSPD_MSK	_DWC3_DSTS_CONNECTSPD_MASK
#define USB3_DSTS_CONNECTSPD	_DWC3_DSTS_CONNECTSPD_MASK
#define USB3_DSTS_SOFFN_POS	_DWC3_DSTS_SOFFN_SHIFT
#define USB3_DSTS_SOFFN_MSK	_DWC3_DSTS_SOFFN_MASK
#define USB3_DSTS_RXFIFOEMPTY	_DWC3_DSTS_RXFIFOEMPTY
#define USB3_DSTS_USBLNKST_POS	_DWC3_DSTS_USBLNKST_SHIFT
#define USB3_DSTS_USBLNKST_MSK	_DWC3_DSTS_USBLNKST_MASK
#define USB3_DSTS_DEVCTRLHLT	_DWC3_DSTS_DEVCTRLHLT
#define USB3_DSTS_COREIDLE	_DWC3_DSTS_COREIDLE
#define USB3_DSTS_SSS	_DWC3_DSTS_SSS
#define USB3_DSTS_RSS	_DWC3_DSTS_RSS
#define USB3_DSTS_SRE	_DWC3_DSTS_SRE
#define USB3_DSTS_DCNRD	_DWC3_DSTS_DCNRD

/* Bit fields for USB3_DGCMD register */
#define USB3_DGCMD_CMDTYP_POS	_DWC3_DEPCMD_CMDTYP_SHIFT
#define USB3_DGCMD_CMDTYP_MSK	_DWC3_DEPCMD_CMDTYP_MASK
#define USB3_DGCMD_CMDIOC	_DWC3_DGCMD_CMDIOC
#define USB3_DGCMD_CMDACT	_DWC3_DGCMD_CMDACT
#define USB3_DGCMD_CMDSTATUS_POS	_DWC3_DGCMD_CMDSTATUS_SHIFT
#define USB3_DGCMD_CMDSTATUS_MSK	_DWC3_DGCMD_CMDSTATUS_MASK

/* Bit fields for USB3_DEPCMD register */
#define USB3_DEPCMD_CMDTYP_POS	_DWC3_DEPCMD_CMDTYP_SHIFT
#define USB3_DEPCMD_CMDTYP_MSK	_DWC3_DEPCMD_CMDTYP_MASK
#define USB3_DEPCMD_CMDTYP	_DWC3_DEPCMD_CMDTYP_MASK
#define USB3_DEPCMD_CMDIOC	_DWC3_DEPCMD_CMDIOC
#define USB3_DEPCMD_CMDACT	_DWC3_DEPCMD_CMDACT
#define USB3_DEPCMD_HIPRI_FORCERM	_DWC3_DEPCMD_HIPRI_FORCERM
#define USB3_DEPCMD_CMDSTATUS_POS	_DWC3_DEPCMD_CMDSTATUS_SHIFT
#define USB3_DEPCMD_CMDSTATUS_MSK	_DWC3_DEPCMD_CMDSTATUS_MASK
#define USB3_DEPCMD_COMMANDPARAM_POS	_DWC3_DEPCMD_COMMANDPARAM_SHIFT
#define USB3_DEPCMD_COMMANDPARAM_MSK	_DWC3_DEPCMD_COMMANDPARAM_MASK

/* Bit fields for USB3_DEV_IMOD register */
#define USB3_DEV_IMOD_DEVICE_IMODI_POS	_DWC3_DEV_IMOD_DEVICE_IMODI_SHIFT
#define USB3_DEV_IMOD_DEVICE_IMODI_MSK	_DWC3_DEV_IMOD_DEVICE_IMODI_MASK
#define USB3_DEV_IMOD_DEVICE_IMODC_POS	_DWC3_DEV_IMOD_DEVICE_IMODC_SHIFT
#define USB3_DEV_IMOD_DEVICE_IMODC_MSK	_DWC3_DEV_IMOD_DEVICE_IMODC_MASK

#define IS_MULTIPLE(x, a)	(((x) % (a)) == 0U)
#define ALIGN_MULTIPLE(x, a)	((a) * (((x) / (a)) + ((((x) % (a)) != 0U) ? 1U : 0U)))
#define PAGE_ALIGN(addr, mask)	((addr) & ~(mask))

#define upper_32_bits(n)	((uint32_t)(((n) >> 16) >> 16))
#define lower_32_bits(n)	((uint32_t)(n))

/* DWC3 IP Parameter */
#define DWC3_IP_DEVICE_NUM_INT	2
#define DWC3_IP_NUM_EPS	32U
#define DWC3_IP_NUM_IN_EPS	16

/* HAL_PCD defines */
#define __HAL_PCD_ENABLE_INTR(__HANDLE__, intr)	usb_dwc3_enable_eventint(__HANDLE__, intr)
#define __HAL_PCD_DISABLE_INTR(__HANDLE__, intr)	usb_dwc3_disable_eventint(__HANDLE__, intr)

#define __HAL_PCD_SETUP_REQ_LEN(addr)	(((uint16_t)(*((uint8_t *)(addr) + 6))) + \
						(((uint16_t)(*((uint8_t *)(addr) + 7))) << 8))

#define __HAL_PCD_SETUP_REQ_DATA_DIR_IN(addr)	((*(uint8_t *)(addr)) & EP_DIR_IN)

#define __HAL_PCD_INCR_EVENT_POS(__HANDLE__, intr, incr) \
						(__HANDLE__)->intbuffers.evtbufferpos[intr] = \
						((__HANDLE__)->intbuffers.evtbufferpos[intr] + \
						 (incr)) % USB_DWC3_EVENT_BUFFER_SIZE
#define __HAL_PCD_READ_EVENT(__HANDLE__, intr)	*(volatile uint32_t *)&((__HANDLE__)->\
						intbuffers.evtbuffer_addr[intr][(__HANDLE__)->\
						intbuffers.evtbufferpos[intr]])

#define __HAL_PCD_EPADDR_TO_PHYEPNUM(ep_addr)	((2U * ((ep_addr) & ADDRESS_MASK)) + \
						((((ep_addr) & EP_DIR_MASK) != 0U) ? 1U : 0U))
#define __HAL_PCD_PHYEPNUM_TO_EPADDR(phy_epnum)	(((phy_epnum) / 2U) | \
						((((phy_epnum) & 0x1U) != 0U) ? EP_DIR_IN : 0U))

#define PCD_DEV_EVENTS_INTR	0 /* Interrupt to use for device events */

#define GET_DWC3EP_FROM_USBEP(__HANDLE__, usb_ep)	*(volatile uint32_t *)&((__HANDLE__)->\
						intbuffers.evtbuffer_addr[intr][(__HANDLE__)->\
						intbuffers.evtbufferpos[intr]])

typedef uint32_t dwc3_epcmd_t;
typedef struct {
	uint32_t  param2;
	uint32_t  param1;
	uint32_t  param0;
} dwc3_epcmd_params_t;

static uint32_t DWC3_regread(void *base, uint32_t offset)
{
	return mmio_read_32((uintptr_t)base + offset);
}

static void DWC3_regwrite(void *base, uint32_t offset, uint32_t value)
{
	mmio_write_32((uintptr_t)base + offset, value);
}

static void DWC3_regupdateset(void *base, uint32_t offset, uint32_t set_mask)
{
	mmio_setbits_32((uintptr_t)base + offset, set_mask);
}

static void DWC3_regupdateclr(void *base, uint32_t offset, uint32_t clr_mask)
{
	mmio_clrbits_32((uintptr_t)base + offset, clr_mask);
}

static void usb_dwc3_enable_eventint(dwc3_handle_t *handle, uint8_t intr_num)
{
	DWC3_regupdateclr(handle->usb_global, DWC3_GEVNTSIZ(intr_num),
			  USB3_GEVNTSIZ_EVNTINTRPTMASK_MSK);
}

static void usb_dwc3_disable_eventint(dwc3_handle_t *handle, uint8_t intr_num)
{
	DWC3_regupdateset(handle->usb_global, DWC3_GEVNTSIZ(intr_num),
			  USB3_GEVNTSIZ_EVNTINTRPTMASK_MSK);
}

static uintptr_t api_mapdmaaddr(uint8_t buf[], uint32_t size, uint8_t to_device)
{
	if (to_device) {
		clean_dcache_range((uintptr_t)buf, size);
	} else {
		inv_dcache_range((uintptr_t)buf, size);
	}

	return (uintptr_t)buf;
}

static void api_unmapdmaaddr(uintptr_t dma_addr __unused, uint32_t size __unused,
			     uint8_t to_device __unused)
{
}

static uintptr_t api_getdmaaddr(uint8_t buf[], uint32_t size __unused, uint8_t to_device __unused)
{
	return (uintptr_t)buf;
}

__unused static void api_putdmaaddr(uintptr_t dma_addr __unused, uint32_t size __unused,
				    uint8_t to_device __unused)
{
}

static void api_memcpy(void *dest, const void *src, uint32_t n)
{
#ifdef AVOID_COMPILER_MEMCPY
	uint8_t *pcdst = (uint8_t *)dest;
	uint8_t const *pcsrc = (uint8_t const *)src;

	while (n--) {
		*pcdst++ = *pcsrc++;
	}
#else
	(void)memcpy(dest, src, n);
#endif
}

static uint32_t dwc3_get_trb_ctltype(uint32_t trb_type)
{
	uint32_t ret;

	switch (trb_type) {
	case USB_DWC3_TRBCTL_NORMAL:
		ret = DWC3_TRBCTL_NORMAL;
		break;
	case USB_DWC3_TRBCTL_CONTROL_SETUP:
		ret = DWC3_TRBCTL_CONTROL_SETUP;
		break;
	case USB_DWC3_TRBCTL_CONTROL_STATUS2:
		ret = DWC3_TRBCTL_CONTROL_STATUS2;
		break;
	case USB_DWC3_TRBCTL_CONTROL_STATUS3:
		ret = DWC3_TRBCTL_CONTROL_STATUS3;
		break;
	case USB_DWC3_TRBCTL_CONTROL_DATA:
		ret = DWC3_TRBCTL_CONTROL_DATA;
		break;
	case USB_DWC3_TRBCTL_ISOCHRONOUS_FIRST:
		ret = DWC3_TRBCTL_ISOCHRONOUS_FIRST;
		break;
	case USB_DWC3_TRBCTL_ISOCHRONOUS:
		ret = DWC3_TRBCTL_ISOCHRONOUS;
		break;
	case USB_DWC3_TRBCTL_LINK_TRB:
		ret = DWC3_TRBCTL_LINK_TRB;
		break;
	default:
		ret = 0U;
		break;
	}

	return ret;
}

static inline const char *dwc3_get_ep_cmd_str(uint32_t cmd)
{
	const char *ret;

	switch (cmd & USB3_DEPCMD_CMDTYP_MSK) {
	case USB_DWC3_DEPCMD_DEPSTARTCFG:
		ret = "Start New Configuration";
		break;
	case USB_DWC3_DEPCMD_ENDTRANSFER:
		ret = "End Transfer";
		break;
	case USB_DWC3_DEPCMD_UPDATETRANSFER:
		ret = "Update Transfer";
		break;
	case USB_DWC3_DEPCMD_STARTTRANSFER:
		ret = "Start Transfer";
		break;
	case USB_DWC3_DEPCMD_CLEARSTALL:
		ret = "Clear Stall";
		break;
	case USB_DWC3_DEPCMD_SETSTALL:
		ret = "Set Stall";
		break;
	case USB_DWC3_DEPCMD_GETEPSTATE:
		ret = "Get Endpoint State";
		break;
	case USB_DWC3_DEPCMD_SETTRANSFRESOURCE:
		ret = "Set Endpoint Transfer Resource";
		break;
	case USB_DWC3_DEPCMD_SETEPCONFIG:
		ret = "Set Endpoint Configuration";
		break;
	default:
		ret = "UNKNOWN command";
		break;
	}

	return ret;
}

static enum usb_status dwc3_execute_dep_cmd(dwc3_handle_t *dwc3_handle, uint8_t phy_epnum,
					    dwc3_epcmd_t cmd, dwc3_epcmd_params_t *params)
{
	uint32_t reg;
	uint64_t timeout;

	VERBOSE("PHYEP%d: cmd '%s' %08x params %08x %08x %08x\n", phy_epnum,
		   dwc3_get_ep_cmd_str(cmd), (uint32_t)cmd, (uint32_t)params->param0,
		   (uint32_t)params->param1, (uint32_t)params->param2);

	DWC3_regwrite(dwc3_handle->usb_device, DWC3_DEPCMDPAR0(phy_epnum), params->param0);
	DWC3_regwrite(dwc3_handle->usb_device, DWC3_DEPCMDPAR1(phy_epnum), params->param1);
	DWC3_regwrite(dwc3_handle->usb_device, DWC3_DEPCMDPAR2(phy_epnum), params->param2);

	DWC3_regwrite(dwc3_handle->usb_device, DWC3_DEPCMD(phy_epnum), cmd | USB3_DEPCMD_CMDACT);

	timeout = timeout_init_us(500); /* usec */
	do {
		reg = DWC3_regread(dwc3_handle->usb_device, DWC3_DEPCMD(phy_epnum));
		/* "Command Complete" */
		if ((reg & USB3_DEPCMD_CMDACT) == 0U) {
			VERBOSE("Command Complete --> %u PHYEP%u %x\n", DWC3_DEPCMD_STATUS(reg),
				   phy_epnum, reg);
			if (DWC3_DEPCMD_STATUS(reg) != 0U) {
				return USBD_FAIL;
			}
			return USBD_OK;
		}

		/* Can be called from interrupt context hence cannot wait for Tick */
		if (timeout_elapsed(timeout)) {
			ERROR("TIMEOUT Command Complete --> %u PHYEP%u %x\n",
				   DWC3_DEPCMD_STATUS(reg), phy_epnum, reg);
			/* "Command Timed Out" */
			return USBD_TIMEOUT;
		}
	} while (true);

	return USBD_OK;
}

static bool dwc3_is_ep_enabled(dwc3_handle_t *dwc3_handle, uint8_t phy_epnum)
{
	if ((DWC3_regread(dwc3_handle->usb_device,
			  DWC3_DALEPENA) & DWC3_DALEPENA_EP(phy_epnum)) != 0U) {
		return true;
	}

	return false;
}

static enum usb_status dwc3_ep_start_xfer(dwc3_handle_t *dwc3_handle, struct usbd_ep *ep)
{
	enum usb_status ret;
	dwc3_epcmd_t cmd;
	dwc3_epcmd_params_t params;

	usb_dwc3_endpoint_t *dwc3_ep = ((ep->is_in) ? &dwc3_handle->IN_ep[ep->num] :
					&dwc3_handle->OUT_ep[ep->num]);

	VERBOSE("%s PHYEP%d %x\n", __func__, dwc3_ep->phy_epnum, dwc3_ep->flags);

	/* Wait for XferNotReady to get the uF interval to srat ISOC transfers */
	if ((ep->type == EP_TYPE_ISOC) &&
	    ((dwc3_ep->flags & USB_DWC3_EP_ISOC_START_PENDING) == 0U) &&
	    ((dwc3_ep->flags & USB_DWC3_EP_ISOC_STARTED) == 0U)) {
		dwc3_ep->flags |= USB_DWC3_EP_ISOC_START_PENDING;
		VERBOSE("INSIDE IF\n");
		return USBD_OK;
	}

	if ((ep->type == EP_TYPE_ISOC) &&
	    ((dwc3_ep->flags & USB_DWC3_EP_ISOC_START_PENDING) != 0U)) {
		dwc3_ep->flags &= ~USB_DWC3_EP_ISOC_START_PENDING;
	}

	dwc3_ep->trb_addr->bpl = lower_32_bits(dwc3_ep->dma_addr);
	dwc3_ep->trb_addr->bph = upper_32_bits(dwc3_ep->dma_addr);
	dwc3_ep->trb_addr->size = DWC3_TRB_SIZE_LENGTH(ep->xfer_len);
	/* also initializes other bits to 0 */
	dwc3_ep->trb_addr->ctrl = dwc3_get_trb_ctltype(dwc3_ep->trb_flag & USB_DWC3_TRBCTL_MASK);

	dwc3_ep->trb_addr->ctrl |= ((ep->num == 0U) ? DWC3_TRB_CTRL_ISP_IMI : 0U);

	dwc3_ep->trb_addr->ctrl |= DWC3_TRB_CTRL_IOC;

	if ((ep->type == EP_TYPE_ISOC) && ((dwc3_ep->flags & USB_DWC3_EP_ISOC_STARTED) != 0U)) {
		cmd = USB_DWC3_DEPCMD_UPDATETRANSFER;
	} else {
		cmd = USB_DWC3_DEPCMD_STARTTRANSFER;
	}

	if (ep->type != EP_TYPE_ISOC) {
		dwc3_ep->trb_addr->ctrl |= DWC3_TRB_CTRL_LST;
	}

	dwc3_ep->trb_addr->ctrl |= DWC3_TRB_CTRL_HWO;

	(void)memset(&params, 0x00, sizeof(params));

	if ((cmd & USB3_DEPCMD_CMDTYP_MSK) == USB_DWC3_DEPCMD_STARTTRANSFER) {
		params.param0 = upper_32_bits(dwc3_ep->trb_dma_addr);
		params.param1 = lower_32_bits(dwc3_ep->trb_dma_addr);
	}

	dwc3_ep->flags |= USB_DWC3_EP_REQ_QUEUED;
	if ((ep->type == EP_TYPE_ISOC) &&
	    ((cmd & USB3_DEPCMD_CMDTYP_MSK) == USB_DWC3_DEPCMD_STARTTRANSFER)) {
		dwc3_ep->flags |= USB_DWC3_EP_ISOC_STARTED;
	}

	VERBOSE("EP%d%s: trb %p:%08x:%08x:%08x:%08x length %u %d\n",
		   ep->num, ep->is_in ? "IN" : "OUT", dwc3_ep->trb_addr,
		   (uint32_t)dwc3_ep->trb_addr->bph, (uint32_t)dwc3_ep->trb_addr->bpl,
		   (uint32_t)dwc3_ep->trb_addr->size, (uint32_t)dwc3_ep->trb_addr->ctrl,
		   ep->xfer_len, dwc3_ep->flags);

	ret = dwc3_execute_dep_cmd(dwc3_handle, dwc3_ep->phy_epnum, cmd, &params);
	if (ret != USBD_OK) {
		dwc3_ep->flags &= ~USB_DWC3_EP_REQ_QUEUED;
		if ((ep->type == EP_TYPE_ISOC) &&
		    ((cmd & USB3_DEPCMD_CMDTYP_MSK) == USB_DWC3_DEPCMD_STARTTRANSFER)) {
			dwc3_ep->flags &= ~USB_DWC3_EP_ISOC_STARTED;
		}
		return ret;
	}

	if ((cmd & USB3_DEPCMD_CMDTYP_MSK) == USB_DWC3_DEPCMD_STARTTRANSFER) {
		dwc3_ep->resc_idx =
		(uint8_t)DWC3_DEPCMD_GET_RSC_IDX(DWC3_regread(dwc3_handle->usb_device,
							      DWC3_DEPCMD(dwc3_ep->phy_epnum)));
	}

	return ret;
}

static enum usb_status usb_dwc3_ep_start_xfer(void *handle, struct usbd_ep *ep)
{
	dwc3_handle_t *dwc3_handle = (dwc3_handle_t *)handle;
	usb_dwc3_endpoint_t *dwc3_ep = ((ep->is_in) ? &dwc3_handle->IN_ep[ep->num] :
					&dwc3_handle->OUT_ep[ep->num]);
	uint32_t len = ep->xfer_len;

	if (!dwc3_is_ep_enabled(dwc3_handle, dwc3_ep->phy_epnum) && ep->num != 0U) {
		return USBD_FAIL;
	}

	if (!ep->is_in) {
		if ((len > ep->maxpacket) && !IS_MULTIPLE(len, ep->maxpacket)) {
			VERBOSE("Packet size exceeds Max_packet and is not an multiple of it\n");
		}

		/*setup and start the Xfer */
		if ((len != 0U) && (len < ep->maxpacket)) {
			dwc3_ep->xfer_dest_buff = ep->xfer_buff;
			dwc3_ep->xfer_dest_len = ep->xfer_len;
			ep->xfer_buff = dwc3_ep->bounce_buf;
			ep->xfer_len = ALIGN_MULTIPLE(len, ep->maxpacket);
		} else {
//			ep->xfer_buff = pBuf;
			ep->xfer_len = ALIGN_MULTIPLE(len, ep->maxpacket);
		}
	}

	dwc3_ep->dma_addr = ((ep->xfer_buff != 0) ?
			     api_mapdmaaddr(ep->xfer_buff, ep->xfer_len, ep->is_in ? 1 : 0) :
			     0x0U);

	if (ep->num == 0U) {
		/* For Status ZLP packet */
		if (len == 0U) {
			/* 2-stage/3-stage control transfer */
			if (__HAL_PCD_SETUP_REQ_LEN(dwc3_handle->setup_addr) == 0U) {
				dwc3_ep->trb_flag = USB_DWC3_TRBCTL_CONTROL_STATUS2;
			} else {
				dwc3_ep->trb_flag = USB_DWC3_TRBCTL_CONTROL_STATUS3;
			}
			//udelay(10);
		} else {
			dwc3_ep->trb_flag = USB_DWC3_TRBCTL_CONTROL_DATA;
		}
	} else {
		if ((ep->type == EP_TYPE_INTR) || (ep->type == EP_TYPE_BULK)) {
			dwc3_ep->trb_flag = USB_DWC3_TRBCTL_NORMAL;
		} else {
			dwc3_ep->trb_flag = USB_DWC3_TRBCTL_ISOCHRONOUS_FIRST;
		}
	}

	return dwc3_ep_start_xfer(dwc3_handle, ep);
}

static enum usb_status dwc3_ep0_out_start(dwc3_handle_t *dwc3_handle, uintptr_t setup_buf_dma_addr)
{
	dwc3_handle->OUT_ep[0].dma_addr = setup_buf_dma_addr;
	dwc3_handle->pcd_handle->out_ep[0].xfer_len = 8;
	dwc3_handle->OUT_ep[0].trb_flag = USB_DWC3_TRBCTL_CONTROL_SETUP;

	return dwc3_ep_start_xfer(dwc3_handle, &dwc3_handle->pcd_handle->out_ep[0]);
}

static enum usb_status usb_dwc3_ep0_out_start(void *handle)
{
	dwc3_handle_t *dwc3_handle = (dwc3_handle_t *)handle;

	dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_QUEUED;
	return dwc3_ep0_out_start(dwc3_handle, dwc3_handle->setup_dma_addr);
}

static enum usb_status usb_dwc3_start_device(void *handle)
{
	dwc3_handle_t *dwc3_handle = (dwc3_handle_t *)handle;
	enum usb_status ret;

	if (dwc3_handle->EP0_State != HAL_PCD_EP0_SETUP_QUEUED) {
		dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_QUEUED;

		ret = dwc3_ep0_out_start(dwc3_handle, dwc3_handle->setup_dma_addr);
		if (ret != USBD_OK) {
			ERROR("%s: %d\n", __func__, __LINE__);
			return ret;
		}
	}

	DWC3_regupdateset(dwc3_handle->usb_device, DWC3_DCTL, USB3_DCTL_RUN_STOP);

	return USBD_OK;
}

static inline void dwc3_ack_evt_count(dwc3_handle_t *dwc3_handle, uint8_t intr_num,
				      uint32_t evt_count)
{
	DWC3_regwrite(dwc3_handle->usb_global, DWC3_GEVNTCOUNT(intr_num), evt_count);
}

static inline uint32_t dwc3_read_intr_count(dwc3_handle_t *dwc3_handle, uint8_t intr_num)
{
	return DWC3_regread(dwc3_handle->usb_global, DWC3_GEVNTCOUNT(intr_num)) &
	       DWC3_GEVNTCOUNT_MASK;
}

static enum usb_status dwc3_ep_stop_xfer(dwc3_handle_t *dwc3_handle, struct usbd_ep *ep)
{
	usb_dwc3_endpoint_t *dwc3_ep = ((ep->is_in) ? &dwc3_handle->IN_ep[ep->num] :
					&dwc3_handle->OUT_ep[ep->num]);
	enum usb_status ret;
	dwc3_epcmd_t cmd;
	dwc3_epcmd_params_t params;

	VERBOSE("%s PHYEP%d %x\n", __func__, dwc3_ep->phy_epnum, dwc3_ep->flags);

	/* Reset ISOC flags */
	if (ep->type == EP_TYPE_ISOC) {
		dwc3_ep->flags &= ~(USB_DWC3_EP_ISOC_START_PENDING | USB_DWC3_EP_ISOC_STARTED);
	}

	if ((dwc3_ep->flags & USB_DWC3_EP_REQ_QUEUED) == 0U) {
		return USBD_FAIL;
	}

	(void)memset(&params, 0x00, sizeof(params));
	cmd = USB_DWC3_DEPCMD_ENDTRANSFER | USB3_DEPCMD_HIPRI_FORCERM | USB3_DEPCMD_CMDIOC |
	      DWC3_DEPCMD_PARAM((uint32_t)dwc3_ep->resc_idx);

	ret = dwc3_execute_dep_cmd(dwc3_handle, dwc3_ep->phy_epnum, cmd, &params);
	/* Need Delay 100us as mentioned in Linux Driver */
	udelay(100);

	ep->xfer_count = ep->xfer_len - (dwc3_ep->trb_addr->size & DWC3_TRB_SIZE_MASK);

	dwc3_ep->flags &= ~USB_DWC3_EP_REQ_QUEUED;

	if ((!ep->is_in) && (ep->num == 0U)) {
		dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_COMPLETED;
	}

	return ret;
}

static enum usb_status dwc3_ep_set_stall(dwc3_handle_t *dwc3_handle,
					 const usb_dwc3_endpoint_t *dwc3_ep)
{
	dwc3_epcmd_params_t params;

	(void)memset(&params, 0x00, sizeof(params));

	return dwc3_execute_dep_cmd(dwc3_handle, dwc3_ep->phy_epnum, USB_DWC3_DEPCMD_SETSTALL,
				    &params);
}

static enum usb_status usb_dwc3_stop_device(void *handle)
{
	dwc3_handle_t *dwc3_handle = (dwc3_handle_t *)handle;
	uint64_t timeout;
	uint8_t i;
	uint32_t evtcnt;
	enum usb_status ret;

	/*
	 * Stop transfers for all(USB_DWC3_NUM_IN_EPS) EP
	 * except EP0IN k = USB_DWC3_NUM_IN_EP
	 */
	for (i = 0; i < USB_DWC3_NUM_IN_EP; i++) {
		dwc3_ep_stop_xfer(dwc3_handle, &dwc3_handle->pcd_handle->in_ep[i]);
	}

	/* Stop transfers for all EP except EP0OUT k = USB_DWC3_NUM_OUT_EP */
	for (i = 0; i < USB_DWC3_NUM_OUT_EP; i++) {
		dwc3_ep_stop_xfer(dwc3_handle, &dwc3_handle->pcd_handle->out_ep[i]);
	}

	/* Issue SetStall on EP0 to reset Ctrl-EP state machine */
	ret = dwc3_ep_set_stall(dwc3_handle, &dwc3_handle->OUT_ep[0]);
	if (ret != USBD_OK) {
		ERROR("%s: EP0 stall failed %u\n", __func__, ret);
	}

	/*
	 * In the Synopsis DesignWare Cores USB3 Databook Rev. 3.30a
	 * Section 1.3.4, it mentions that for the DEVCTRLHLT bit, the
	 * "software needs to acknowledge the events that are generated
	 * (by writing to GEVNTCOUNTn) while it is waiting for this bit
	 * to be set to '1'."
	 */

	/* Check for all Event Buffer interrupt k = USB_DWC3_INT_INUSE */
	for (i = 0; i < USB_DWC3_INT_INUSE; i++) {
		evtcnt = dwc3_read_intr_count(dwc3_handle, i);

		if (!evtcnt) {
			continue;
		}

		__HAL_PCD_INCR_EVENT_POS(dwc3_handle, i, evtcnt);

		dwc3_ack_evt_count(dwc3_handle, i, evtcnt);
	}


	DWC3_regupdateclr(dwc3_handle->usb_device, DWC3_DCTL, USB3_DCTL_RUN_STOP);

	timeout = timeout_init_us(500); /* usec */

	while ((DWC3_regread(dwc3_handle->usb_device, DWC3_DSTS) &
		     USB3_DSTS_DEVCTRLHLT) == 0U) {
		/* Can be called from interrupt context hence cannot wait for Tick */
		if (timeout_elapsed(timeout)) { /* "Reset Timed Out" */
			ERROR("TIMEOUT Stop Device\n");
			return USBD_TIMEOUT;
		}
	}

	// "Halt Complete"

	return USBD_OK;
}

static enum usb_status usb_dwc3_set_address(void *handle, uint8_t address)
{
	dwc3_handle_t *dwc3_handle = (dwc3_handle_t *)handle;

	VERBOSE("%s: %d\n", __func__, address);

	/* set device address */
	DWC3_regupdateclr(dwc3_handle->usb_device, DWC3_DCFG, USB3_DCFG_DEVADDR_MSK);
	DWC3_regupdateset(dwc3_handle->usb_device, DWC3_DCFG, DWC3_DCFG_DEVADDR((uint32_t)address));

	return USBD_OK;
}

static enum usb_status usb_dwc3_ep0_start_xfer(void *handle, struct usbd_ep *ep)
{
	return usb_dwc3_ep_start_xfer(handle, ep);
}

static enum usb_status usb_dwc3_ep_set_stall(void *handle, struct usbd_ep *ep)
{
	dwc3_handle_t *dwc3_handle = (dwc3_handle_t *)handle;
	usb_dwc3_endpoint_t *dwc3_ep = ((ep->is_in) ? &dwc3_handle->IN_ep[ep->num] :
					&dwc3_handle->OUT_ep[ep->num]);

	if (dwc3_ep->is_stall) {
		return USBD_OK;
	}

	dwc3_ep->is_stall = true;

	return dwc3_ep_set_stall(dwc3_handle, dwc3_ep);
}

static uint8_t dwc3_read_ep_evt_type(uint32_t event)
{
	uint8_t ret;

	switch ((event & DWC3_EVT_DEPEVT_TYPE_MASK) >> DWC3_EVT_DEPEVT_TYPE_BITPOS) {
	case DWC3_DEPEVT_XFERCOMPLETE:
		ret =  USB_DWC3_DEPEVT_XFERCOMPLETE;
		break;
	case DWC3_DEPEVT_XFERINPROGRESS:
		ret =  USB_DWC3_DEPEVT_XFERINPROGRESS;
		break;
	case DWC3_DEPEVT_XFERNOTREADY:
		ret =  USB_DWC3_DEPEVT_XFERNOTREADY;
		break;
	case DWC3_DEPEVT_RXTXFIFOEVT:
		ret =  USB_DWC3_DEPEVT_RXTXFIFOEVT;
		break;
	case DWC3_DEPEVT_STREAMEVT:
		ret =  USB_DWC3_DEPEVT_STREAMEVT;
		break;
	case DWC3_DEPEVT_EPCMDCMPLT:
		ret =  USB_DWC3_DEPEVT_EPCMDCMPLT;
		break;
	default:
		ret = (event & DWC3_EVT_DEPEVT_TYPE_MASK) >> DWC3_EVT_DEPEVT_TYPE_BITPOS;
		break;
	}

	return ret;
}

static uint8_t dwc3_read_ep_evt_epnum(uint32_t event)
{
	return (uint8_t)((event & DWC3_EVT_DEPEVT_EPNUM_MASK) >> DWC3_EVT_DEPEVT_EPNUM_BITPOS);
}

static uint8_t dwc3_read_ep_evt_status(uint32_t event)
{
	uint8_t ret;

	switch ((event & DWC3_EVT_DEPEVT_STATUS_MASK) >> DWC3_EVT_DEPEVT_STATUS_BITPOS) {
	case DWC3_EVT_DEPEVT_STATUS_CONTROL_DATA:
		ret = USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_DATA;
		break;
	case DWC3_EVT_DEPEVT_STATUS_CONTROL_STATUS:
		ret = USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_STATUS;
		break;
	default:
		ret = (event & DWC3_EVT_DEPEVT_STATUS_MASK) >> DWC3_EVT_DEPEVT_STATUS_BITPOS;
		break;
	}

	return ret;
}

static enum usb_status dwc3_epaddr_set_stall(dwc3_handle_t *dwc3_handle, uint8_t ep_addr)
{
	struct usbd_ep *ep;
	enum usb_status ret;
	usb_dwc3_endpoint_t *dwc3_ep;

	if ((ep_addr & EP_DIR_MASK) == EP_DIR_IN) {
		ep = &dwc3_handle->pcd_handle->in_ep[ep_addr & ADDRESS_MASK];
		dwc3_ep = &dwc3_handle->IN_ep[ep_addr & ADDRESS_MASK];
	} else {
		ep = &dwc3_handle->pcd_handle->out_ep[ep_addr];
		dwc3_ep = &dwc3_handle->OUT_ep[ep_addr];
	}

	/* For control endpoints, the application issues only the Set Stall command, and only on the
	 * OUT direction of the control endpoint. The controller automatically clears the STALL when
	 * it receives a SETUP token for the endpoint. The application must not issue the Clear
	 * Stall command on a control endpoint
	 */
	if (ep_addr == EP0_IN) {
		return USBD_OK;
	}

	if (dwc3_ep->is_stall) {
		return USBD_OK;
	}

	dwc3_ep->is_stall = true;
	ep->num   = ep_addr & ADDRESS_MASK;
	ep->is_in = ((ep_addr & EP_DIR_MASK) == EP_DIR_IN);

	ret = dwc3_ep_set_stall(dwc3_handle, dwc3_ep);
	if (ret != USBD_OK) {
		return ret;
	}

	if ((ep_addr & ADDRESS_MASK) == 0U) {
		dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_QUEUED;
		ret = dwc3_ep0_out_start(dwc3_handle, dwc3_handle->setup_dma_addr);
	}

	return ret;
}

static uint32_t dwc3_get_ep_trblen(usb_dwc3_endpoint_t *ep)
{
	return DWC3_TRB_SIZE_LENGTH(ep->trb_addr->size);
}

static uint32_t dwc3_get_ep_trbstatus(usb_dwc3_endpoint_t *ep)
{
	return DWC3_TRB_SIZE_TRBSTS(ep->trb_addr->size);
}

static enum usb_action dwc3_handle_ep0_xfernotready_event(dwc3_handle_t *dwc3_handle,
							  uint32_t event, uint32_t *param)
{
	enum usb_action action = USB_NOTHING;
	uint8_t phy_epnum = dwc3_read_ep_evt_epnum(event);
	struct pcd_handle *pcd_handle = dwc3_handle->pcd_handle;
	uint8_t status = dwc3_read_ep_evt_status(event);
	uint8_t ep_addr, ep_num;
	enum usb_status ret;

	ep_addr = __HAL_PCD_PHYEPNUM_TO_EPADDR(phy_epnum);
	ep_num = ep_addr & ADDRESS_MASK;

	if (__HAL_PCD_SETUP_REQ_LEN(dwc3_handle->setup_addr) == 0U) {
		switch (status) {
		case USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_STATUS:
			api_memcpy(dwc3_handle->pcd_handle->setup,
				   dwc3_handle->setup_addr,
				   sizeof(dwc3_handle->pcd_handle->setup));
			action = USB_SETUP;
			break;
		case USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_DATA:
			ret = dwc3_epaddr_set_stall(dwc3_handle, ep_num); // OUT EP0
			if (ret != USBD_OK) {
				ERROR("%s: %d\n", __func__, __LINE__);
			}
			break;
		default:
			VERBOSE("Invalid Status %d: %s: %d\n", status, __func__, __LINE__);
			break;
		}
	} else {
		switch (status) {
		case USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_STATUS:
			if ((ep_addr & EP_DIR_MASK) == EP_DIR_IN) {
				*param = ep_num;
				action = USB_DATA_OUT;
			} else {
				*param = ep_num;
				action = USB_DATA_IN;
			}
			break;
		case USB_DWC3_DEPEVT_XFERNOTREADY_STATUS_CTRL_DATA:
			if ((ep_addr & EP_DIR_MASK) == EP_DIR_IN) {
				/* if wrong direction */
				if (__HAL_PCD_SETUP_REQ_DATA_DIR_IN(dwc3_handle->setup_addr) ==
				    0U) {
					ret = dwc3_ep_stop_xfer(dwc3_handle,
								&pcd_handle->out_ep[0]);
					if (ret != USBD_OK) {
						ERROR("%s: %d\n", __func__, __LINE__);
					}
					ret = dwc3_epaddr_set_stall(dwc3_handle, ep_num); // OUT EP0
					if (ret != USBD_OK) {
						ERROR("%s: %d\n", __func__, __LINE__);
					}
				}
				/*
				 * last packet is MPS multiple, so send ZLP packet,
				 * handled by USBD_core.c
				 */
			} else {
				/* if wrong direction */
				if (__HAL_PCD_SETUP_REQ_DATA_DIR_IN(dwc3_handle->setup_addr) !=
				    0U) {
					ret = dwc3_ep_stop_xfer(dwc3_handle,
								&pcd_handle->in_ep[0]);
					if (ret != USBD_OK) {
						ERROR("%s: %d\n", __func__, __LINE__);
					}
					ret = dwc3_epaddr_set_stall(dwc3_handle, ep_num); // OUT EP0
					if (ret != USBD_OK) {
						ERROR("%s: %d\n", __func__, __LINE__);
					}
				}
			}
			break;
		default:
			VERBOSE("Invalid Status %d: %s: %d\n", status, __func__, __LINE__);
			break;
		}
	}

	return action;
}

static enum usb_action dwc3_handle_ep_event(dwc3_handle_t *dwc3_handle, uint32_t event,
					    uint32_t *param)
{
	enum usb_action action = USB_NOTHING;
	uint8_t type = dwc3_read_ep_evt_type(event);
	uint8_t phy_epnum = dwc3_read_ep_evt_epnum(event);
	uint8_t ep_addr, ep_num;
	struct usbd_ep *ep;
	usb_dwc3_endpoint_t *dwc3_ep;
	enum usb_status ret;

	ep_addr = __HAL_PCD_PHYEPNUM_TO_EPADDR(phy_epnum);
	ep_num = ep_addr & ADDRESS_MASK;

	if ((ep_addr & EP_DIR_MASK) == EP_DIR_IN) {
		ep = &dwc3_handle->pcd_handle->in_ep[ep_num];
		dwc3_ep = &dwc3_handle->IN_ep[ep_num];
	} else {
		ep = &dwc3_handle->pcd_handle->out_ep[ep_num];
		dwc3_ep = &dwc3_handle->OUT_ep[ep_num];
	}

	switch (type) {
	case USB_DWC3_DEPEVT_XFERCOMPLETE:
		VERBOSE("EP%d%s: Transfer Complete Event=%x\n", ep_num,
			   ep->is_in ? "IN" : "OUT", event);

		if (ep->type == EP_TYPE_ISOC) {
			VERBOSE("EP%d%s is an Isochronous endpoint\n", ep_num,
				   ep->is_in ? "IN" : "OUT");
			break;
		}

		/* Just forr Log, since ctrl expects sw to follow the whole control
		 * programming sequence, as also mentioned in Linux driver
		 */
		if ((ep_num == 0U) &&
		    (dwc3_get_ep_trbstatus(dwc3_ep) == USB_DWC3_TRBSTS_SETUP_PENDING)) {
			VERBOSE("EP%d%s Setup Pending received\n", ep_num,
				   ep->is_in ? "IN" : "OUT");
		}

		VERBOSE("EP%d%s: Transfer Complete trb %p:%08x:%08x:%08x:%08x TRB-Status %u\n",
			   ep_num, ep->is_in ? "IN" : "OUT", dwc3_ep->trb_addr,
			   (uint32_t)dwc3_ep->trb_addr->bph, (uint32_t)dwc3_ep->trb_addr->bpl,
			   (uint32_t)dwc3_ep->trb_addr->size, (uint32_t)dwc3_ep->trb_addr->ctrl,
			   dwc3_get_ep_trbstatus(dwc3_ep));

		if ((ep_addr & EP_DIR_MASK) == EP_DIR_IN) {  /* IN EP */
			ep->xfer_count = ep->xfer_len - dwc3_get_ep_trblen(dwc3_ep);

			dwc3_ep->flags &= ~USB_DWC3_EP_REQ_QUEUED;

			if (ep->xfer_buff != NULL) {
				api_unmapdmaaddr(dwc3_ep->dma_addr, ep->xfer_len, 1);
			}

			/*
			 * For EP0IN, if packet is not sent completely,
			 * then error, as done in Linux driver
			 */
			if ((ep_num == 0U) && (ep->xfer_len != ep->xfer_count)) {
				ret = dwc3_epaddr_set_stall(dwc3_handle, ep_num); // OUT EP0
				if (ret != USBD_OK) {
					ERROR("%s: %d\n", __func__, __LINE__);
				}
			}

			/*
			 * if 3-stage then wait for XFERNOTREADY(control-status) before
			 * sending data to upper layer
			 */
			if ((ep_num != 0U) || (ep->xfer_len == 0U)) {
				*param = ep_num;
				action = USB_DATA_IN;
			}

			/* For EP0, this is ZLP, so prepare EP0 for next setup */
			if ((ep_num == 0U) && (ep->xfer_len == 0U)) {
				/* prepare to rx more setup packets */
				dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_QUEUED;

				ret = dwc3_ep0_out_start(dwc3_handle, dwc3_handle->setup_dma_addr);
				if (ret != USBD_OK) {
					ERROR("%s: %d\n", __func__, __LINE__);
					return action;
				}
			}
		} else {
			ep->xfer_count = ep->xfer_len - dwc3_get_ep_trblen(dwc3_ep);

			dwc3_ep->flags &= ~USB_DWC3_EP_REQ_QUEUED;

			/* For EP0OUT */
			if ((ep_addr == 0U) &&
			    (dwc3_handle->EP0_State == HAL_PCD_EP0_SETUP_QUEUED)) {

				dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_COMPLETED;
				dwc3_ep->is_stall = false;
				VERBOSE("EP%d%s: SetupData %02x%02x%02x%02x:%02x%02x%02x%02x\n",
					   ep_num, ep->is_in ? "IN" : "OUT",
					   dwc3_handle->setup_addr[0], dwc3_handle->setup_addr[1],
					   dwc3_handle->setup_addr[2], dwc3_handle->setup_addr[3],
					   dwc3_handle->setup_addr[4], dwc3_handle->setup_addr[5],
					   dwc3_handle->setup_addr[6], dwc3_handle->setup_addr[7]);

				// if 2-stage then wait for the XFERNOTREADY(control-status)
				if (__HAL_PCD_SETUP_REQ_LEN(dwc3_handle->setup_addr) != 0U) {
					api_memcpy(dwc3_handle->pcd_handle->setup,
						   dwc3_handle->setup_addr,
						   sizeof(dwc3_handle->pcd_handle->setup));
					action = USB_SETUP;
				}
			} else {

				if (ep->xfer_buff != NULL) {
					api_unmapdmaaddr(dwc3_ep->dma_addr, ep->xfer_len, 0);
				}

				if (ep->xfer_buff == dwc3_ep->bounce_buf) {
					api_memcpy(dwc3_ep->xfer_dest_buff, ep->xfer_buff,
						   ep->xfer_count);
					ep->xfer_buff = dwc3_ep->xfer_dest_buff;
				}

				/*
				 * if 3-stage then wait for XFERNOTREADY(control-status) before
				 * sending data to upper layer
				 */
				if ((ep_num != 0U) || (ep->xfer_len == 0U)) {
					*param = ep_num;
					action = USB_DATA_OUT;
				}

				/* For EP0, this is ZLP, so prepare EP0 for next setup */
				if ((ep_num == 0U) && (ep->xfer_len == 0U)) {
					/* prepare to rx more setup packets */
					dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_QUEUED;

					ret = dwc3_ep0_out_start(dwc3_handle,
								 dwc3_handle->setup_dma_addr);
					if (ret != USBD_OK) {
						ERROR("%s: %d\n", __func__, __LINE__);
						return action;
					}
				}
			}
		}

		break;

	case USB_DWC3_DEPEVT_XFERINPROGRESS:
		VERBOSE("EP%d%s: Transfer In-Progress %u Event=%x\n", ep_num,
			   ep->is_in ? "IN" : "OUT", dwc3_handle->intbuffers.evtbufferpos[0],
			   event);

		if (ep_num == 0U) { // ***ToCheck
			break;
		}

		/*
		 * currently not planned to issue multiple TRB transfer, hence there shouldn't
		 * be any event like this
		 */

		/*
		 * But for isoc transfers we need get it since transfer started with
		 * loop(link) trb
		 */
		if (ep->type != EP_TYPE_ISOC) {
			break;
		}

		break;

	case USB_DWC3_DEPEVT_XFERNOTREADY:
		VERBOSE("EP%d%s: Transfer Not Ready Event=%x\n", ep_num,
			   ep->is_in ? "IN" : "OUT", event);
		/* For EP0IN or EP0OUT, currently no plan to use xfernotready interrupt */

		//active = PCD_READ_DEPEVT_STATUS(event);

		if (ep_num == 0U) {

			if (dwc3_handle->EP0_State == HAL_PCD_EP0_SETUP_QUEUED) {
				ret = dwc3_epaddr_set_stall(dwc3_handle, ep_num);  // OUT EP0
				if (ret != USBD_OK) {
					ERROR("%s: %d\n", __func__, __LINE__);
					return action;
				}
			} else {
				action = dwc3_handle_ep0_xfernotready_event(dwc3_handle, event,
									    param);
			}
		}

		/*
		 * Currently we dont implement provide request queue to support On-demand transfers
		 * for non-isoc EP, it only supports preset transfers so this event will not be
		 * used, Section 9.2.4 "Transfer Setup Recommendations"
		 */

		/*
		 * But for isoc transfers we need to issue the request from here using the
		 * micro-frame number
		 */
		if (ep->type != EP_TYPE_ISOC) {
			break;
		}

		break;

	case USB_DWC3_DEPEVT_STREAMEVT: /* Stream event, Not used */
		break;
	case USB_DWC3_DEPEVT_RXTXFIFOEVT: /* FIFO Overrun */
		break;
	case USB_DWC3_DEPEVT_EPCMDCMPLT: /* Endpoint Command Complete */
		VERBOSE("EP%d%s: Endpoint Command Complete Event=%x\n", ep_num,
			   ep->is_in ? "IN" : "OUT", event);
		break;
	default:
		VERBOSE("Invalid type %d: %s: %d\n", type, __func__, __LINE__);
		break;
	}

	return action;
}

static uint8_t dwc3_read_dev_evt_type(uint32_t event)
{
	uint8_t ret;

	switch ((event & DWC3_EVT_DEVEVT_TYPE_MASK) >> DWC3_EVT_DEVEVT_TYPE_BITPOS) {
	case DWC3_DEVICE_EVENT_DISCONNECT:
		ret = USB_DWC3_DEVICE_EVENT_DISCONNECT;
		break;
	case DWC3_DEVICE_EVENT_RESET:
		ret = USB_DWC3_DEVICE_EVENT_RESET;
		break;
	case DWC3_DEVICE_EVENT_CONNECT_DONE:
		ret = USB_DWC3_DEVICE_EVENT_CONNECT_DONE;
		break;
	case DWC3_DEVICE_EVENT_LINK_STATUS_CHANGE:
		ret = USB_DWC3_DEVICE_EVENT_LINK_STATUS_CHANGE;
		break;
	case DWC3_DEVICE_EVENT_WAKEUP:
		ret = USB_DWC3_DEVICE_EVENT_WAKEUP;
		break;
	case DWC3_DEVICE_EVENT_HIBER_REQ:
		ret = USB_DWC3_DEVICE_EVENT_HIBER_REQ;
		break;
	case DWC3_DEVICE_EVENT_EOPF:
		ret = USB_DWC3_DEVICE_EVENT_EOPF;
		break;
	case DWC3_DEVICE_EVENT_SOF:
		ret = USB_DWC3_DEVICE_EVENT_SOF;
		break;
	case DWC3_DEVICE_EVENT_ERRATIC_ERROR:
		ret = USB_DWC3_DEVICE_EVENT_ERRATIC_ERROR;
		break;
	case DWC3_DEVICE_EVENT_CMD_CMPL:
		ret = USB_DWC3_DEVICE_EVENT_CMD_CMPL;
		break;
	case DWC3_DEVICE_EVENT_OVERFLOW:
		ret = USB_DWC3_DEVICE_EVENT_OVERFLOW;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

#if (USB_DWC3_NUM_IN_EP > 1) || (USB_DWC3_NUM_OUT_EP > 1)
static enum usb_status dwc3_ep_clear_stall(dwc3_handle_t *dwc3_handle, usb_dwc3_endpoint_t *dwc3_ep)
{
	dwc3_epcmd_params_t params;

	(void)memset(&params, 0x00, sizeof(params));

	return dwc3_execute_dep_cmd(dwc3_handle, dwc3_ep->phy_epnum, USB_DWC3_DEPCMD_CLEARSTALL,
				    &params);
}
#endif

static uint8_t dwc3_get_dev_speed(dwc3_handle_t *dwc3_handle)
{
	uint32_t reg;
	uint8_t ret;

	reg = DWC3_regread(dwc3_handle->usb_device, DWC3_DSTS) & USB3_DSTS_CONNECTSPD;

	switch (reg) {
	case DWC3_DSTS_SUPERSPEED:
		ret = USB_DWC3_SPEED_SUPER;
		INFO("%s = SuperSpeed\n", __func__);
		break;
	case DWC3_DSTS_HIGHSPEED:
		ret = USB_DWC3_SPEED_HIGH;
		INFO("%s = HighSpeed\n", __func__);
		break;
	case DWC3_DSTS_FULLSPEED1:
		ret = USB_DWC3_SPEED_FULL_48;
		INFO("%s = FullSpeed_48M\n", __func__);
		break;
	case DWC3_DSTS_FULLSPEED2:
		ret = USB_DWC3_SPEED_FULL;
		INFO("%s = FullSpeed\n", __func__);
		break;
	case DWC3_DSTS_LOWSPEED:
		ret = USB_DWC3_SPEED_LOW;
		INFO("%s = LowSpeed\n", __func__);
		break;
	default: /* Invalid */
		ret = USB_DWC3_SPEED_INVALID;
		INFO("%s = Invalid\n", __func__);
		break;
	}

	return ret;
}

static uint32_t dwc3_get_epcfg_action(uint8_t action)
{
	uint32_t ret;

	switch (action) {
	case USB_DWC3_DEPCFG_ACTION_INIT:
		ret = DWC3_DEPCFG_ACTION_INIT;
		break;
	case USB_DWC3_DEPCFG_ACTION_RESTORE:
		ret = DWC3_DEPCFG_ACTION_RESTORE;
		break;
	case USB_DWC3_DEPCFG_ACTION_MODIFY:
		ret = DWC3_DEPCFG_ACTION_MODIFY;
		break;
	default:
		ret = 0U;
		break;
	}

	return ret;
}

static enum usb_status dwc3_ep_configure(dwc3_handle_t *dwc3_handle, uint8_t epnum, bool is_in,
					 uint8_t type, uint32_t max_packet, uint8_t tx_fifo,
					 uint16_t binterval, uint8_t phy_epnum, uint8_t intr_num,
					 uint8_t action)
{
	dwc3_epcmd_params_t params;

	(void)memset(&params, 0x00, sizeof(params));
	params.param0 = DWC3_DEPCFG_EP_TYPE(type) | DWC3_DEPCFG_MAX_PACKET_SIZE(max_packet);

	/* Burst size is only needed in SuperSpeed mode */
	//params.param0 |= DWC3_DEPCFG_BURST_SIZE(0);

	params.param0 |= dwc3_get_epcfg_action(action);

	params.param1 = DWC3_DEPCFG_XFER_COMPLETE_EN | DWC3_DEPCFG_XFER_NOT_READY_EN;
	if (type == EP_TYPE_ISOC) {
		params.param1 |= DWC3_DEPCFG_XFER_IN_PROGRESS_EN;
	}

	params.param1 |= DWC3_DEPCFG_EP_NUMBER(((uint32_t)epnum << 1) + (is_in ? 1UL : 0UL));

	params.param1 |= DWC3_DEPCFG_EP_INTR_NUM(intr_num);

	if (is_in) {
		params.param0 |= DWC3_DEPCFG_FIFO_NUMBER(tx_fifo);
	}

	if (binterval != 0U) {
		params.param1 |= DWC3_DEPCFG_BINTERVAL_M1(binterval - 1UL);
	}

	return dwc3_execute_dep_cmd(dwc3_handle, phy_epnum, USB_DWC3_DEPCMD_SETEPCONFIG, &params);
}

static inline uint8_t dwc3_read_dev_evt_linkstate(uint32_t event)
{
	return (uint8_t)((event & DWC3_EVT_DEVEVT_LNKSTS_MASK) >> DWC3_EVT_DEVEVT_LNKSTS_BITPOS);
}

static inline const char *dwc3_get_linkstate_str(uint8_t state)
{
	const char *ret;

	switch (state) {
	case DWC3_LINK_STATE_ON:
		ret = "ON";
		break;
	case DWC3_LINK_STATE_L1:
		ret = "L1";
		break;
	case DWC3_LINK_STATE_L2:
		ret = "L2";
		break;
	case DWC3_LINK_STATE_DIS:
		ret = "Disconnect";
		break;
	case DWC3_LINK_STATE_EARLY_SUS:
		ret = "Early Suspend";
		break;
	case DWC3_LINK_STATE_RESET:
		ret = "Reset";
		break;
	case DWC3_LINK_STATE_RESUME:
		ret = "Resume";
		break;
	default:
		ret = "UNKNOWN state";
		break;
	}

	return ret;
}

static enum usb_action dwc3_handle_dev_event(dwc3_handle_t *dwc3_handle, uint32_t event,
					     uint32_t *param)
{
	enum usb_action action = USB_NOTHING;
	uint8_t type = dwc3_read_dev_evt_type(event);
	uint8_t i, speed;
	enum usb_status ret;
	uint32_t ep0_mps = USB3_MAX_PACKET_SIZE;

	switch (type) {
	case USB_DWC3_DEVICE_EVENT_DISCONNECT:
		INFO("Event: Device Disconnect\n");
		/* For SuperSpeed, set DCTL[8:5] to 5*/

		action = USB_DISCONNECT;
		break;

	case USB_DWC3_DEVICE_EVENT_RESET:
		INFO("Event: Device Reset\n");
		/*
		 * Spec says, Wait till EP0 finishes current transfer and reaches Setup-Stage.
		 * But there is no wait in Linux driver which means that Host will take care while
		 * issuing a reset. Also its tricky to implement since we can't wait here
		 * (in interrupt context) for further control EP interrupts
		 */

		if (dwc3_handle->EP0_State != HAL_PCD_EP0_SETUP_QUEUED) {
			ret = dwc3_ep_stop_xfer(dwc3_handle, &dwc3_handle->pcd_handle->out_ep[0]);
			if (ret != USBD_OK) {
				ERROR("%s: %d\n", __func__, __LINE__);
			}
			ret = dwc3_epaddr_set_stall(dwc3_handle, EP0_OUT); // OUT EP0
			if (ret != USBD_OK) {
				ERROR("%s: %d\n", __func__, __LINE__);
			}
		}

		/*
		 * Stop transfers for all(USB_DWC3_NUM_IN_EPS) EP
		 * except EP0IN k = USB_DWC3_NUM_IN_EP
		 */
		for (i = 0; i < USB_DWC3_NUM_IN_EP; i++) {
			dwc3_ep_stop_xfer(dwc3_handle,
					      &dwc3_handle->pcd_handle->in_ep[i]);
		}

#if USB_DWC3_NUM_OUT_EP > 1
		/* Stop transfers for all EP except EP0OUT k = USB_DWC3_NUM_OUT_EP */
		for (i = 1; i < USB_DWC3_NUM_OUT_EP; i++) {
			dwc3_ep_stop_xfer(dwc3_handle,
					      &dwc3_handle->pcd_handle->out_ep[i]);
		}
#endif

#if USB_DWC3_NUM_IN_EP > 1
		/* Clear Stall for all EP except EP0IN k = USB_DWC3_NUM_IN_EP */
		for (i = 1; i < USB_DWC3_NUM_IN_EP; i++) {
			usb_dwc3_endpoint_t *ep = &dwc3_handle->IN_ep[i];

			if (!ep->is_stall) {
				continue;
			}

			ep->is_stall = false;

			ret = dwc3_ep_clear_stall(dwc3_handle, ep);
			if (ret != USBD_OK) {
				ERROR("%s: %d\n", __func__, __LINE__);
				return action;
			}
		}
#endif

#if USB_DWC3_NUM_OUT_EP > 1
		/* Clear Stall for all EP except EP0OUT k = USB_DWC3_NUM_OUT_EP */
		for (i = 1; i < USB_DWC3_NUM_OUT_EP; i++) {
			usb_dwc3_endpoint_t *ep = &dwc3_handle->OUT_ep[i];

			if (!ep->is_stall) {
				continue;
			}

			ep->is_stall = false;
			//ep->num   = ep_addr & ADDRESS_MASK;
			//ep->is_in = ((ep_addr & 0x80) == 0x80);

			ret = dwc3_ep_clear_stall(dwc3_handle, ep);
			if (ret != USBD_OK) {
				ERROR("%s: %d\n", __func__, __LINE__);
				return action;
			}
		}
#endif

		/* Reset device address to zero */
		ret = usb_dwc3_set_address(dwc3_handle, 0);
		if (ret != USBD_OK) {
			ERROR("%s: %d\n", __func__, __LINE__);
			return action;
		}

		action = USB_RESET;

		break;

	case USB_DWC3_DEVICE_EVENT_CONNECT_DONE:
		INFO("Event: Device Connection Done\n");

		/*
		 * hpcd->Init.ep0_mps = MaxPacketSize to 512 (SuperSpeed), 64 (High-Speed),
		 * 8/16/32/64 (Full-Speed), or 8 (Low-Speed).
		 */
		speed = dwc3_get_dev_speed(dwc3_handle);
		switch (speed) {
		case USB_DWC3_SPEED_SUPER:
			ep0_mps = 512;
			break;
		case USB_DWC3_SPEED_HIGH:
			ep0_mps = 64;
			break;
		case USB_DWC3_SPEED_FULL_48:
		case USB_DWC3_SPEED_FULL:
			ep0_mps = 64;
			break;
		case USB_DWC3_SPEED_LOW:
			ep0_mps = 8;
			break;
		default:
			ERROR("%s: %d\n", __func__, __LINE__);
			break;
		}

		/* Update DWC3_GCTL_RAMCLKSEL, but only for Superspeed */

		/* Modify max packet size for EP 0 & 1 */
		ret = dwc3_ep_configure(dwc3_handle, 0, false, EP_TYPE_CTRL, ep0_mps, 0, 0, 0,
					dwc3_handle->OUT_ep[0].intr_num,
					USB_DWC3_DEPCFG_ACTION_MODIFY);
		if (ret != USBD_OK) {
			ERROR("%s: %d\n", __func__, __LINE__);
			return action;
		}
		ret = dwc3_ep_configure(dwc3_handle, 0, true, EP_TYPE_CTRL, ep0_mps, 0, 0, 1,
					dwc3_handle->IN_ep[0].intr_num,
					USB_DWC3_DEPCFG_ACTION_MODIFY);
		if (ret != USBD_OK) {
			ERROR("%s: %d\n", __func__, __LINE__);
			return action;
		}

		dwc3_handle->pcd_handle->out_ep[0].maxpacket = ep0_mps;
		dwc3_handle->pcd_handle->in_ep[0].maxpacket = ep0_mps;

		/*
		 * Depending on the connected speed, write to the other
		 * PHY's control register to suspend it.
		 */

		/* Use default values for GTXFIFOSIZn and GRXFIFOSIZ0 */

		action = USB_ENUM_DONE;

		break;

	case USB_DWC3_DEVICE_EVENT_LINK_STATUS_CHANGE:
		INFO("Event: Link Status Change : %s(%d)\n",
			   dwc3_get_linkstate_str(dwc3_read_dev_evt_linkstate(event)),
			   dwc3_read_dev_evt_linkstate(event));
		/* Handle link state changes, U0,U1,U2,U3 with suspend/resume functions */

		break;

	case USB_DWC3_DEVICE_EVENT_WAKEUP:
		INFO("Event: Wake-UP\n");
		// resume function
		action = USB_RESUME;
		break;

	case USB_DWC3_DEVICE_EVENT_HIBER_REQ:
		INFO("Event: Hibernation Request\n");
		/* hibernation function */
		break;

	case USB_DWC3_DEVICE_EVENT_EOPF: /* End of Periodic Frame */
		INFO("Event: Suspend\n");
		action = USB_SUSPEND;
		break;

	case USB_DWC3_DEVICE_EVENT_SOF: /* Start of Periodic Frame */
		INFO("Event: Start of Periodic Frame\n");
		break;

	case USB_DWC3_DEVICE_EVENT_L1SUSPEND:
		INFO("Event: L1 Suspend\n");
		action = USB_LPM;
		break;

	case USB_DWC3_DEVICE_EVENT_ERRATIC_ERROR: /* Erratic Error */
		INFO("Event: Erratic Error\n");
		break;

	case USB_DWC3_DEVICE_EVENT_CMD_CMPL: /* Command Complete */
		INFO("Event: Command Complete\n");
		break;

	case USB_DWC3_DEVICE_EVENT_OVERFLOW: /* Overflow */
		INFO("Event: Overflow\n");
		break;

	case USB_DWC3_DEVICE_EVENT_L1WAKEUP:
		INFO("Event: L1 Wake-UP\n");
		// resume function
		action = USB_RESUME;
		break;

	default:  /* UNKNOWN IRQ type */
		break;
	}

	return action;
}

static inline bool dwc3_is_ep_event(uint32_t event)
{
	return ((event & DWC3_EVT_TYPE_MASK) == (DWC3_EVT_TYPE_EP << DWC3_EVT_TYPE_BITPOS));
}

static inline bool dwc3_is_dev_event(uint32_t event)
{
	return ((event & DWC3_EVT_TYPE_MASK) == (DWC3_EVT_TYPE_DEVSPEC << DWC3_EVT_TYPE_BITPOS));
}

static enum usb_action usb_dwc3_it_handler(void *handle, uint32_t *param)
{
	dwc3_handle_t *dwc3_handle = (dwc3_handle_t *)handle;
	enum usb_action action = USB_NOTHING;
	uint32_t evtcnt, evt;
	uint8_t i;

	/* Use same handler for all the Evt-Buf IRQs */

	/* Check for all Event Buffer interrupt k = USB_DWC3_INT_INUSE */
	for (i = 0; i < USB_DWC3_INT_INUSE; i++) {
		evtcnt = dwc3_read_intr_count(dwc3_handle, i);

		if (!evtcnt) {
			continue;
		}

		VERBOSE("Interrupt Count %u\n", evtcnt);

		evt = __HAL_PCD_READ_EVENT(dwc3_handle, i);
		VERBOSE("Event %08x\n", evt);

		if (dwc3_is_ep_event(evt)) {   /* EP event */
			action = dwc3_handle_ep_event(dwc3_handle, evt, param);
		} else {     /* NON-EP event */
			if (dwc3_is_dev_event(evt)) {  /* Device specific event */
				action = dwc3_handle_dev_event(dwc3_handle, evt, param);
			} else {
			  /* Error, non supported events */
			}
		}

		__HAL_PCD_INCR_EVENT_POS(dwc3_handle, i, USB_DWC3_EVENT_SIZE);

		dwc3_ack_evt_count(dwc3_handle, i, USB_DWC3_EVENT_SIZE);
	}

	return action;
}

static enum usb_status usb_dwc3_write_packet(void *handle __unused, uint8_t *src __unused,
					     uint8_t ch_ep_num __unused, uint16_t len __unused)
{
	return USBD_OK;
}

static void *usb_dwc3_read_packet(void *handle __unused, uint8_t *dest __unused,
				  uint16_t len __unused)
{
	return NULL;
}

static enum usb_status usb_dwc3_write_empty_tx_fifo(void *handle __unused, uint32_t epnum,
						    uint32_t xfer_len __unused,
						    uint32_t *xfer_count __unused,
						    uint32_t maxpacket __unused,
						    uint8_t **xfer_buff __unused)
{
	return USBD_OK;
}

static const struct usb_driver usb_dwc3driver = {
	.ep0_out_start = usb_dwc3_ep0_out_start,
	.ep_start_xfer = usb_dwc3_ep_start_xfer,
	.ep0_start_xfer = usb_dwc3_ep0_start_xfer,
	.write_packet = usb_dwc3_write_packet,
	.read_packet = usb_dwc3_read_packet,
	.ep_set_stall = usb_dwc3_ep_set_stall,
	.start_device = usb_dwc3_start_device,
	.stop_device = usb_dwc3_stop_device,
	.set_address = usb_dwc3_set_address,
	.write_empty_tx_fifo = usb_dwc3_write_empty_tx_fifo,
	.it_handler = usb_dwc3_it_handler
};

/* USB2 PHY Mask 0xf */
#define USBPHY_ULPI	1U
#define USBPHY_UTMI	2U
#define USBPHY_EMBEDDED_USB2	8U
#define USB2PHY_MASK	15U
/* USB3 PHY Mask (0xf << 4) */
#define USBPHY_PIPE3	16U
#define USBPHY_EMBEDDED_USB3	128U
#define USB3PHY_MASK	(15U << 4)

static enum usb_status dwc3_soft_reset(dwc3_handle_t *dwc3_handle)
{
	uint64_t timeout;

	DWC3_regwrite(dwc3_handle->usb_device, DWC3_DCTL, USB3_DCTL_CSFTRST);

	timeout = timeout_init_us(500*1000); /* msec */

	while (DWC3_regread(dwc3_handle->usb_device, DWC3_DCTL) & USB3_DCTL_CSFTRST) {

		if (timeout_elapsed(timeout)) { /* "Reset Timed Out" */
			return USBD_TIMEOUT;
		}
	}

	return USBD_OK;
}

static enum usb_status dwc3_core_init(dwc3_handle_t *dwc3_handle, uint32_t phy_itface)
{
	enum usb_status ret;

	VERBOSE("Core ID %08x\n", (uint32_t)DWC3_regread(dwc3_handle->usb_global, DWC3_GSNPSID));

	/* Set GUSB3PIPECTL for all USB3 ports (1-n), currently doing only for 0 */
	if ((phy_itface & USB3PHY_MASK) != 0U) {
		/* Relying on default value */
	}

	/* Set GUSB2PHYCFG for all USB2 ports (1-m), currently doing only for 0 */
	if ((phy_itface & USB2PHY_MASK) != 0U) {
		switch (phy_itface) {
		case USBPHY_UTMI:
			DWC3_regupdateclr(dwc3_handle->usb_global, DWC3_GUSB2PHYCFG(0UL),
					  USB3_GUSB2PHYCFG_ULPI_UTMI_SEL);
			break;
		case USBPHY_ULPI:
			DWC3_regupdateset(dwc3_handle->usb_global, DWC3_GUSB2PHYCFG(0UL),
					  USB3_GUSB2PHYCFG_ULPI_UTMI_SEL);
			break;
		default:
			/* Relying on default value. */
			break;
		}
	}

	/* issue device SoftReset */
	ret = dwc3_soft_reset(dwc3_handle);
	if (ret != USBD_OK) {
		ERROR("%s: %d\n", __func__, __LINE__);
		return ret;
	}

	/* Put PHYs in reset */
	/* Before Resetting PHY, put Core in Reset */
	DWC3_regupdateset(dwc3_handle->usb_global, DWC3_GCTL, USB3_GCTL_CORESOFTRESET);

	/* Assert USB3 PHY reset for all USB3 ports (1-n), currently doing only for 0 */
	if ((phy_itface & USB3PHY_MASK) != 0U) {
		DWC3_regupdateset(dwc3_handle->usb_global, DWC3_GUSB3PIPECTL(0UL),
				  USB3_GUSB3PIPECTL_PHYSOFTRST);
	}

	/* Assert USB2 PHY reset for all USB2 ports (1-m), currently doing only for 0 */
	if ((phy_itface & USB2PHY_MASK) != 0U) {
		DWC3_regupdateset(dwc3_handle->usb_global, DWC3_GUSB2PHYCFG(0UL),
				  USB3_GUSB2PHYCFG_PHYSOFTRST);
	}

	// Program PHY signals - PHY specific


	// Phy Init not needed since will be done by bootrom

	/* Release PHY out of reset */

	/* Clear USB3 PHY reset for all USB3 ports (1-n), currently doing only for 0 */
	if ((phy_itface & USB3PHY_MASK) != 0U) {
		DWC3_regupdateclr(dwc3_handle->usb_global, DWC3_GUSB3PIPECTL(0UL),
				  USB3_GUSB3PIPECTL_PHYSOFTRST);
	}

	/* Clear USB2 PHY reset */
	if ((phy_itface & USB2PHY_MASK) != 0U) {
		DWC3_regupdateclr(dwc3_handle->usb_global, DWC3_GUSB2PHYCFG(0UL),
				  USB3_GUSB2PHYCFG_PHYSOFTRST);
	}

	// delay needed for phy reset
	mdelay(100); // 100ms

	/* After PHYs are stable we can take Core out of reset state */
	DWC3_regupdateclr(dwc3_handle->usb_global, DWC3_GCTL, USB3_GCTL_CORESOFTRESET);

	/* Disable Scale down mode - check if disabled by default */
	//DWC3_regupdateclr(USBx, DWC3_GCTL, DWC3_GCTL_SCALEDOWN_MASK); ***ToCheck

	/* Alloc and Setup Scratch buffers for hibernation */
	// ***ToCheck

	// From Linux Driver
	DWC3_regwrite(dwc3_handle->usb_global, DWC3_GSBUSCFG0, 0xe);
	DWC3_regwrite(dwc3_handle->usb_global, DWC3_GSBUSCFG1, 0xf00);

	/* Use default values for GTXFIFOSIZn and GRXFIFOSIZ0 */

	return USBD_OK;
}

typedef enum {
	USB_DWC3_DEVICE_MODE  = 0U,
	USB_DWC3_HOST_MODE    = 1U,
	USB_DWC3_DRD_MODE     = 2U,
	USB_DWC3_INVALID_MODE = 0xFU
} USB_DWC3_modetypedef;

static enum usb_status dwc3_set_current_mode(dwc3_handle_t *dwc3_handle, USB_DWC3_modetypedef mode)
{
	uint32_t reg;
	enum usb_status ret = USBD_OK;

	reg = DWC3_regread(dwc3_handle->usb_global, DWC3_GCTL) & ~USB3_GCTL_PRTCAPDIR_MSK;

	switch (mode) {
	case USB_DWC3_DEVICE_MODE:
		DWC3_regwrite(dwc3_handle->usb_global, DWC3_GCTL, reg |
			      DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_DEVICE));
		break;
	case USB_DWC3_HOST_MODE:
		DWC3_regwrite(dwc3_handle->usb_global, DWC3_GCTL, reg |
			      DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_HOST));
		break;
	case USB_DWC3_DRD_MODE:
		DWC3_regwrite(dwc3_handle->usb_global, DWC3_GCTL, reg |
			      DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_OTG));
		break;
	default:
		ret = USBD_FAIL;
		break;
	}

	return ret;
}

static enum usb_status dwc3_set_dev_speed(dwc3_handle_t *dwc3_handle, uint8_t speed)
{
	uint32_t reg;
	enum usb_status ret = USBD_OK;

	reg = DWC3_regread(dwc3_handle->usb_device, DWC3_DCFG) & ~USB3_DCFG_DEVSPD_MSK;

	switch (speed) {
	case USB_DWC3_SPEED_SUPER:
		DWC3_regwrite(dwc3_handle->usb_device, DWC3_DCFG, reg | DWC3_DSTS_SUPERSPEED);
		INFO("%s = SuperSpeed\n", __func__);
		break;
	case USB_DWC3_SPEED_HIGH:
		DWC3_regwrite(dwc3_handle->usb_device, DWC3_DCFG, reg |  DWC3_DSTS_HIGHSPEED);
		INFO("%s = HighSpeed\n", __func__);
		break;
	case USB_DWC3_SPEED_FULL_48:
		DWC3_regwrite(dwc3_handle->usb_device, DWC3_DCFG, reg |  DWC3_DSTS_FULLSPEED1);
		INFO("%s = FullSpeed_48M\n", __func__);
		break;
	case USB_DWC3_SPEED_FULL:
		DWC3_regwrite(dwc3_handle->usb_device, DWC3_DCFG, reg |  DWC3_DSTS_FULLSPEED2);
		INFO("%s = FullSpeed\n", __func__);
		break;
	case USB_DWC3_SPEED_LOW:
		DWC3_regwrite(dwc3_handle->usb_device, DWC3_DCFG, reg |  DWC3_DSTS_LOWSPEED);
		INFO("%s = LowSpeed\n", __func__);
		break;
	default:
		ret = USBD_FAIL;
		break;
	}

	return ret;
}

/*
 * @brief  USB_DevInit : Initializes the USB3 controller registers
 *         for device mode
 * @param  USBx  Selected device
 * @param  cfg   pointer to a USB_DWC3_CfgTypeDef structure that contains
 *         the configuration information for the specified USBx peripheral.
 * @retval HAL status
 */
enum usb_status dwc3_dev_init(dwc3_handle_t *dwc3_handle, uint8_t speed, uint8_t intr_dev)
{
	uint32_t reg;
	uint8_t i;
	dwc3_epcmd_params_t params;
	enum usb_status ret;

	/* Setup event buffers k = USB_DWC3_INT_INUSE */
	for (i = 0; i < USB_DWC3_INT_INUSE; i++) {
		DWC3_regwrite(dwc3_handle->usb_global, DWC3_GEVNTADRLO(i),
			      lower_32_bits(dwc3_handle->intbuffers.evtbuffer_dma_addr[i]));
		DWC3_regwrite(dwc3_handle->usb_global, DWC3_GEVNTADRHI(i),
			      upper_32_bits(dwc3_handle->intbuffers.evtbuffer_dma_addr[i]));
		DWC3_regwrite(dwc3_handle->usb_global, DWC3_GEVNTSIZ(i),
			      USB_DWC3_EVENT_BUFFER_SIZE);
		DWC3_regwrite(dwc3_handle->usb_global, DWC3_GEVNTCOUNT(i), 0);

		/* EvtBufferPos[i] = 0;  Implicit since static done in HAL */
	}

	/*
	 * Need to set GUCTL2 RST_ACTBITLATER, so the driver can poll for CMDACT bit
	 * when issuing the ENDTRANSFER command.
	 */
	DWC3_regupdateset(dwc3_handle->usb_global, DWC3_GUCTL2, DWC3_GUCTL2_RST_ACTBITLATER);

	ret = dwc3_set_current_mode(dwc3_handle, USB_DWC3_DEVICE_MODE);
	if (ret != USBD_OK) {
		ERROR("%s: %d\n", __func__, __LINE__);
		return ret;
	}

	ret = dwc3_set_dev_speed(dwc3_handle, speed);
	if (ret != USBD_OK) {
		ERROR("%s: %d\n", __func__, __LINE__);
		return ret;
	}

	/* Issue a DEPSTARTCFG command with DEPCMD0.XferRscIdx set to 0,
	 * to initialize the transfer resource allocation
	 */
	zeromem(&params, sizeof(params));
	ret = dwc3_execute_dep_cmd(dwc3_handle, 0, DWC3_DEPCMD_PARAM(0) |
				   USB_DWC3_DEPCMD_DEPSTARTCFG, &params);
	if (ret != USBD_OK) {
		ERROR("%s: %d\n", __func__, __LINE__);
		return ret;
	}

	/* Configure Control EP 0 & 1 mapped to physical EP 0 & 1*/
	ret = dwc3_ep_configure(dwc3_handle, 0, false, EP_TYPE_CTRL,
				dwc3_handle->pcd_handle->out_ep[0].maxpacket, 0, 0, 0,
				dwc3_handle->OUT_ep[0].intr_num, USB_DWC3_DEPCFG_ACTION_INIT);
	if (ret != USBD_OK) {
		ERROR("%s: %d\n", __func__, __LINE__);
		return ret;
	}
	ret = dwc3_ep_configure(dwc3_handle, 0, true, EP_TYPE_CTRL,
				dwc3_handle->pcd_handle->in_ep[0].maxpacket, 0, 0, 1,
				dwc3_handle->IN_ep[0].intr_num, USB_DWC3_DEPCFG_ACTION_INIT);
	if (ret != USBD_OK) {
		ERROR("%s: %d\n", __func__, __LINE__);
		return ret;
	}

	/* Transfer Resource for Control EP 0 & 1*/
	(void)memset(&params, 0x00, sizeof(params));
	params.param0 = DWC3_DEPXFERCFG_NUM_XFER_RES(1U);
	/* As per databook, "Issue a DEPSTARTCFG command with DEPCMD0.XferRscIdx set to 2 to
	 * re-initialize the transfer resource allocation on SetConfiguration or SetInterface
	 * Request", value is 2 since the EP0 & EP1 take up the transfer resource 1 & 2, hence
	 * further EPs will take resources 2,3,&up.. But in HAL we are not decoding the
	 * Setup-packet requests, this should ideally come from USB-MW core.
	 * Instead we follow the same method as Linux dwc3-gadget controller driver, All hardware
	 * endpoints can be assigned a transfer resource and this setting will stay persistent
	 * until either a core reset or hibernation. So whenever we do a DEPSTARTCFG(0) we can go
	 * ahead and do DEPXFERCFG for every hardware endpoint as well.
	 * For all k = DWC3_IP_NUM_EPS
	 */
	for (i = 0; i < DWC3_IP_NUM_EPS; i++) {
		ret = dwc3_execute_dep_cmd(dwc3_handle, i, USB_DWC3_DEPCMD_SETTRANSFRESOURCE,
					   &params);
		if (ret != USBD_OK) {
			ERROR("%s: %d\n", __func__, __LINE__);
			return ret;
		}
	}

	/* setup EP0 to receive SETUP packets */
	ret = dwc3_ep0_out_start(dwc3_handle, dwc3_handle->setup_dma_addr);
	if (ret != USBD_OK) {
		ERROR("%s: %d\n", __func__, __LINE__);
		return ret;
	}

	/* Enable EP 0 & 1 */
	DWC3_regupdateset(dwc3_handle->usb_device, DWC3_DALEPENA, DWC3_DALEPENA_EP(0) |
			  DWC3_DALEPENA_EP(1));

	/*
	 * Set interrupt/EventQ number on which non-endpoint-specific device-related
	 * interrupts are generated
	 */
	reg = DWC3_regread(dwc3_handle->usb_device, DWC3_DCFG) & ~USB3_DCFG_INTRNUM_MSK;
	DWC3_regwrite(dwc3_handle->usb_device, DWC3_DCFG, reg |
		      ((uint32_t)intr_dev << USB3_DCFG_INTRNUM_POS));

	/* Enable all events but Start and End of Frame IRQs */
	DWC3_regupdateset(dwc3_handle->usb_device, DWC3_DEVTEN, USB3_DEVTEN_VENDEVTSTRCVDEN |
			  USB3_DEVTEN_EVNTOVERFLOWEN | USB3_DEVTEN_CMDCMPLTEN |
			  USB3_DEVTEN_ERRTICERREVTEN | USB3_DEVTEN_U3L2L1SUSPEN |
			  USB3_DEVTEN_HIBERNATIONREQEVTEN | USB3_DEVTEN_WKUPEVTEN |
			  USB3_DEVTEN_ULSTCNGEN | USB3_DEVTEN_CONNECTDONEEVTEN |
			  USB3_DEVTEN_USBRSTEVTEN | USB3_DEVTEN_DISSCONNEVTEN |
			  USB3_DEVTEN_L1SUSPEN | USB3_DEVTEN_L1WKUPEVTEN
			  /* | USB3_DEVTEN_SOFTEVTEN*/);

	/* Enable Event Buffer interrupt k = USB_DWC3_INT_INUSE */
	for (i = 0; i < USB_DWC3_INT_INUSE; i++) {
		__HAL_PCD_ENABLE_INTR(dwc3_handle, i);
	}

	return USBD_OK;
}

void usb_dwc3_init_driver(struct usb_handle *usb_core_handle, struct pcd_handle *pcd_handle,
			  dwc3_handle_t *dwc3_handle, void *base_addr)
{
	uint32_t i = 0;
	enum usb_status ret;
	uintptr_t base = (uintptr_t)base_addr;

	dwc3_handle->usb_global = (usb_dwc3_global_t *)
					(base + USB_DWC3_GLOBAL_BASE);

	dwc3_handle->usb_device = (usb_dwc3_device_t *)
					(base + USB_DWC3_DEVICE_BASE);

	dwc3_handle->pcd_handle = pcd_handle;

	/* Check hpcd->State is HAL_PCD_STATE_RESET, otherwise error */
	assert(dwc3_handle->State == HAL_PCD_STATE_RESET);

	dwc3_handle->State = HAL_PCD_STATE_BUSY;

	/* Disable the Interrupts */
	// Not required since USB device and EP interrupts are disabled at boot, ***ToCheck
	for (i = 0; i < USB_DWC3_INT_INUSE; i++) {
		__HAL_PCD_DISABLE_INTR(dwc3_handle, i);
	}

	/* Init the Core (common init.) */
	ret = dwc3_core_init(dwc3_handle, USBPHY_UTMI);
	if (ret != USBD_OK) {
		panic();
	}

	/* Init endpoints structures */
	for (i = 0; i < USB_DWC3_NUM_IN_EP ; i++) {
		/* Init ep structure */
		pcd_handle->in_ep[i].is_in = true;
		pcd_handle->in_ep[i].num = i;
		dwc3_handle->IN_ep[i].tx_fifo_num = i;
		/* Control until ep is activated */
		pcd_handle->in_ep[i].type = EP_TYPE_CTRL;

		/*
		 * We are doing 1:1 alternate mapping for endpoints, meaning
		 * IN Endpoint X maps to Physical Endpoint 2*X +1 and
		 * OUT Endpoint X maps to Physical Endpoint 2*X.
		 * So USB endpoint 0x81 is 0x03.
		 */
#if USB_DWC3_NUM_IN_EP > 1
		dwc3_handle->IN_ep[i].phy_epnum = __HAL_PCD_EPADDR_TO_PHYEPNUM(i | EP_DIR_IN);
#else
		dwc3_handle->IN_ep[i].phy_epnum = 1U;
#endif

		dwc3_handle->IN_ep[i].intr_num = PCD_DEV_EVENTS_INTR;
	}

	for (i = 0; i < USB_DWC3_NUM_OUT_EP ; i++) {
		/* Init ep structure */
		pcd_handle->out_ep[i].is_in = false;
		pcd_handle->out_ep[i].num = i;
		dwc3_handle->OUT_ep[i].tx_fifo_num = i;
		/* Control until ep is activated */
		pcd_handle->out_ep[i].type = EP_TYPE_CTRL;

		/*
		 * We are doing 1:1 alternate mapping for endpoints, meaning
		 * IN Endpoint X maps to Physical Endpoint 2*X +1 and
		 * OUT Endpoint X maps to Physical Endpoint 2*X.
		 * So USB endpoint 0x81 is 0x03.
		 */
		dwc3_handle->OUT_ep[i].phy_epnum = __HAL_PCD_EPADDR_TO_PHYEPNUM(i);

		dwc3_handle->OUT_ep[i].intr_num = PCD_DEV_EVENTS_INTR;

		dwc3_handle->OUT_ep[i].bounce_buf = dwc3_handle->bounce_bufs[i].bounce_buf;
	}

#define PHYS_AREA	STM32MP_USB_DWC3_BASE

#define EVTBUF_AREA_OFFSET	0U
#define TRB_OUT_AREA_OFFSET	(EVTBUF_AREA_OFFSET + USB_DWC3_EVENT_BUFFER_SIZE)
#define TRB_IN_AREA_OFFSET	(TRB_OUT_AREA_OFFSET + sizeof(usb_dwc3_trb_t))
#define SETUP_AREA_OFFSET	(TRB_IN_AREA_OFFSET + sizeof(usb_dwc3_trb_t))

	void *coh_area = (void *)(uintptr_t)PHYS_AREA;

#define EVTBUF_AREA	(coh_area + EVTBUF_AREA_OFFSET)
#define TRB_OUT_AREA	(coh_area + TRB_OUT_AREA_OFFSET)
#define TRB_IN_AREA	(coh_area + TRB_IN_AREA_OFFSET)
#define SETUP_AREA	(coh_area + SETUP_AREA_OFFSET)

	dwc3_handle->setup_dma_addr = (uintptr_t)api_getdmaaddr((void *)SETUP_AREA,
								USB_SETUP_PACKET_SIZE, 1);
	assert(dwc3_handle->setup_dma_addr != 0U);

	dwc3_handle->setup_addr = SETUP_AREA;
	assert(dwc3_handle->setup_addr != NULL);

	/* Map DMA and Coherent address for event buffers k = USB_DWC3_INT_INUSE */
	for (i = 0; i < USB_DWC3_INT_INUSE; i++) {
		dwc3_handle->intbuffers.evtbuffer_dma_addr[i] =
			api_getdmaaddr((void *)EVTBUF_AREA, USB_DWC3_EVENT_BUFFER_SIZE, 1);
		assert(dwc3_handle->intbuffers.evtbuffer_dma_addr[i] != 0U);

		dwc3_handle->intbuffers.evtbuffer_addr[i] = EVTBUF_AREA;
		assert(dwc3_handle->intbuffers.evtbuffer_addr[i] != NULL);

		dwc3_handle->intbuffers.evtbufferpos[i] = 0;

		INFO("EventBuffer%u: BuffArea=%lx DmaAddr=%08x CoherentMapAddr=%p\n", i,
			   (PHYS_AREA + EVTBUF_AREA_OFFSET),
			   (uint32_t)dwc3_handle->intbuffers.evtbuffer_dma_addr[i],
			   dwc3_handle->intbuffers.evtbuffer_addr[i]);
	}

	/* MAP TRB Coherent and DMA address for EP0IN and EP0OUT */
	dwc3_handle->IN_ep[0].trb_dma_addr = (uint32_t)api_getdmaaddr((void *)TRB_IN_AREA,
								      sizeof(usb_dwc3_trb_t), 1);
	assert(dwc3_handle->IN_ep[0].trb_dma_addr != 0U);

	dwc3_handle->IN_ep[0].trb_addr = (usb_dwc3_trb_t *)TRB_IN_AREA;
	assert(dwc3_handle->IN_ep[0].trb_addr != NULL);

	dwc3_handle->OUT_ep[0].trb_dma_addr = (uint32_t)api_getdmaaddr((void *)TRB_OUT_AREA,
								       sizeof(usb_dwc3_trb_t),
								       1);
	assert(dwc3_handle->OUT_ep[0].trb_dma_addr != 0U);

	dwc3_handle->OUT_ep[0].trb_addr = (usb_dwc3_trb_t *)TRB_OUT_AREA;
	assert(dwc3_handle->OUT_ep[0].trb_addr != NULL);

	/* Init Device */
	dwc3_handle->EP0_State = HAL_PCD_EP0_SETUP_QUEUED;

	ret = dwc3_dev_init(dwc3_handle, USB_DWC3_SPEED_HIGH, PCD_DEV_EVENTS_INTR);
	assert(ret == USBD_OK);

	dwc3_handle->State = HAL_PCD_STATE_READY;

	register_usb_driver(usb_core_handle, pcd_handle, &usb_dwc3driver,
			    dwc3_handle);
}
