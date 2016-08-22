/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DWUSB_H__
#define __DWUSB_H__

/* Core Global Registers */
#define GOTGCTL				(0x000)
#define GOTGINT				(0x004)
#define GOTGINT_DBNCE_DONE		(1 << 19)
#define GOTGINT_A_DEV_TOUT_CHG		(1 << 18)
#define GOTGINT_HST_NEG_DET		(1 << 17)
#define GOTGINT_HST_NEG_SUC_STS_CHNG	(1 << 9)
#define GOTGINT_SES_REQ_SUC_STS_CHNG	(1 << 8)
#define GOTGINT_SES_END_DET		(1 << 2)

#define GAHBCFG				(0x008)
#define GAHBCFG_P_TXF_EMP_LVL           (1 << 8)
#define GAHBCFG_NP_TXF_EMP_LVL          (1 << 7)
#define GAHBCFG_DMA_EN                  (1 << 5)
#define GAHBCFG_GLBL_INTR_EN            (1 << 0)
#define GAHBCFG_CTRL_MASK               (GAHBCFG_P_TXF_EMP_LVL | \
					 GAHBCFG_NP_TXF_EMP_LVL | \
					 GAHBCFG_DMA_EN | \
					 GAHBCFG_GLBL_INTR_EN)

#define GUSBCFG				(0x00C)
#define GRSTCTL				(0x010)
#define GRSTCTL_AHBIDLE			(1 << 31)
#define GRSTCTL_CSFTRST			(1 << 0)

#define GINTSTS				(0x014)
#define GINTMSK				(0x018)
#define GINTSTS_WKUPINT			(1 << 31)
#define GINTSTS_SESSREQINT		(1 << 30)
#define GINTSTS_DISCONNINT		(1 << 29)
#define GINTSTS_CONIDSTSCHNG		(1 << 28)
#define GINTSTS_LPMTRANRCVD		(1 << 27)
#define GINTSTS_PTXFEMP			(1 << 26)
#define GINTSTS_HCHINT			(1 << 25)
#define GINTSTS_PRTINT			(1 << 24)
#define GINTSTS_RESETDET		(1 << 23)
#define GINTSTS_FET_SUSP		(1 << 22)
#define GINTSTS_INCOMPL_IP		(1 << 21)
#define GINTSTS_INCOMPL_SOIN		(1 << 20)
#define GINTSTS_OEPINT			(1 << 19)
#define GINTSTS_IEPINT			(1 << 18)
#define GINTSTS_EPMIS			(1 << 17)
#define GINTSTS_RESTOREDONE		(1 << 16)
#define GINTSTS_EOPF			(1 << 15)
#define GINTSTS_ISOUTDROP		(1 << 14)
#define GINTSTS_ENUMDONE		(1 << 13)
#define GINTSTS_USBRST			(1 << 12)
#define GINTSTS_USBSUSP			(1 << 11)
#define GINTSTS_ERLYSUSP		(1 << 10)
#define GINTSTS_I2CINT			(1 << 9)
#define GINTSTS_ULPI_CK_INT		(1 << 8)
#define GINTSTS_GOUTNAKEFF		(1 << 7)
#define GINTSTS_GINNAKEFF		(1 << 6)
#define GINTSTS_NPTXFEMP		(1 << 5)
#define GINTSTS_RXFLVL			(1 << 4)
#define GINTSTS_SOF			(1 << 3)
#define GINTSTS_OTGINT			(1 << 2)
#define GINTSTS_MODEMIS			(1 << 1)
#define GINTSTS_CURMODE_HOST		(1 << 0)

#define GRXSTSR				(0x01C)
#define GRXSTSP				(0x020)
#define GRXFSIZ				(0x024)
#define GNPTXFSIZ			(0x028)
#define GNPTXSTS			(0x02C)

#define GHWCFG1				(0x044)
#define GHWCFG2				(0x048)
#define GHWCFG3				(0x04c)
#define GHWCFG4				(0x050)
#define GLPMCFG				(0x054)

#define GDFIFOCFG			(0x05c)
#define GDFIFOCFG_EPINFOBASE_MASK	(0xffff << 16)
#define GDFIFOCFG_EPINFOBASE_SHIFT	16
#define GDFIFOCFG_GDFIFOCFG_MASK	(0xffff << 0)
#define GDFIFOCFG_GDFIFOCFG_SHIFT	0

#define DIEPTXF(x)			(0x100 + 4 * (x))

/* Device Global Registers */
#define DCFG        			(0x800)
#define DCFG_EPMISCNT_MASK		(0x1f << 18)
#define DCFG_EPMISCNT_SHIFT		18
#define DCFG_NZ_STS_OUT_HSHK		(1 << 2)
#define DCFG_DEVADDR_MASK		(0x7f << 4)
#define DCFG_DEVADDR(_x)		(((_x) & 0x7f) << 4)

#define DCTL        			(0x804)
#define DCTL_CGNPINNAK			(1 << 8)

#define DSTS        			(0x808)
#define DIEPMSK     			(0x810)
#define DOEPMSK     			(0x814)
#define DAINT       			(0x818)
#define DAINTMSK    			(0x81C)
#define DAINT_OUT_MASK			(0xffff << 16)
#define DAINT_IN_MASK			(0xffff)
#define DAINT_OUTEP_SHIFT		16
#define DAINT_OUTEP(_x)			(1 << ((_x) + 16))
#define DAINT_INEP(_x)			(1 << (_x))

/* Device Logical IN Endpoint-Specific Registers */
#define DIEPCTL(x)  			(0x900 + 0x20 * (x))
#define DIEPINT(x)  			(0x908 + 0x20 * (x))
#define DIEPTSIZ(x) 			(0x910 + 0x20 * (x))
#define DIEPDMA(x)  			(0x914 + 0x20 * (x))
#define DTXFSTS(x)  			(0x918 + 0x20 * (x))

/* Device Logical OUT Endpoint-Specific Registers */
#define DOEPCTL(x)  			(0xB00 + 0x20 * (x))
#define DXEPCTL_EPENA			(1 << 31)
#define DXEPCTL_EPDIS			(1 << 30)
#define DXEPCTL_SETD1PID		(1 << 29)
#define DXEPCTL_SETODDFR		(1 << 29)
#define DXEPCTL_SETD0PID		(1 << 28)
#define DXEPCTL_SETEVENFR		(1 << 28)
#define DXEPCTL_SNAK			(1 << 27)
#define DXEPCTL_CNAK			(1 << 26)
#define DXEPCTL_TXFNUM(_x)		(((_x) & 0xf) << 22)
#define DXEPCTL_STALL			(1 << 21)
#define DXEPCTL_EPTYPE_MASK		(3 << 18)
#define DXEPCTL_EPTYPE(_x)		(((_x) & 0x3) << 18)
#define DXEPCTL_NAKSTS			(1 << 17)
#define DXEPCTL_DPID			(1 << 16)
#define DXEPCTL_EOFRNUM			(1 << 16)
#define DXEPCTL_USBACTEP		(1 << 15)
#define DXEPCTL_NEXTEP_MASK		(0xf << 11)
#define DXEPCTL_NEXTEP_SHIFT		11
#define DXEPCTL_NEXTEP_LIMIT		0xf
#define DXEPCTL_NEXTEP(_x)		((_x) << 11)
#define DXEPCTL_MPS_MASK		0x7ff
#define DXEPCTL_MPS(_x)			((_x) & 0x7ff)
#define DOEPCTL_USBACTEP		(1 << 15)


#define DOEPINT(x)  			(0xB08 + 0x20 * (x))
#define DXEPINT_NAKINTRPT		(1 << 13)
#define DXEPINT_INEPNAKEFF              (1 << 6)
#define DXEPINT_BACK2BACKSETUP          (1 << 6)
#define DXEPINT_INTKNEPMIS              (1 << 5)
#define DXEPINT_INTKNTXFEMP             (1 << 4)
#define DXEPINT_OUTTKNEPDIS             (1 << 4)
#define DXEPINT_TIMEOUT                 (1 << 3)
#define DXEPINT_SETUP                   (1 << 3)
#define DXEPINT_AHBERR                  (1 << 2)
#define DXEPINT_EPDISBLD                (1 << 1)
#define DXEPINT_XFERCOMPL               (1 << 0)

#define DOEPTSIZ(x) 			(0xB10 + 0x20 * (x))
#define DXEPTSIZ_MC_MASK		(0x3 << 29)
#define DXEPTSIZ_MC_SHIFT		29
#define DXEPTSIZ_MC_LIMIT		0x3
#define DXEPTSIZ_MC(_x)			((_x) << 29)
#define DXEPTSIZ_PKTCNT_MASK		(0x3ff << 19)
#define DXEPTSIZ_PKTCNT_SHIFT		19
#define DXEPTSIZ_PKTCNT_LIMIT		0x3ff
#define DXEPTSIZ_PKTCNT_GET(_v)		(((_v) >> 19) & 0x3ff)
#define DXEPTSIZ_PKTCNT(_x)		((_x) << 19)
#define DXEPTSIZ_XFERSIZE_MASK		(0x7ffff << 0)
#define DXEPTSIZ_XFERSIZE_SHIFT		0
#define DXEPTSIZ_XFERSIZE_LIMIT		0x7ffff
#define DXEPTSIZ_XFERSIZE_GET(_v)	(((_v) >> 0) & 0x7ffff)
#define DXEPTSIZ_XFERSIZE(_x)		((_x) << 0)

#define DOEPDMA(x)  			(0xB14 + 0x20 * (x))
#define DOEPCTL0    			(0xB00)
#define DOEPINT0    			(0xB08)
#define DOEPTSIZ0   			(0xB10)
#define DOEPTSIZ0_SUPCNT_MASK		(0x3 << 29)
#define DOEPTSIZ0_SUPCNT_SHIFT		29
#define DOEPTSIZ0_SUPCNT_LIMIT		0x3
#define DOEPTSIZ0_SUPCNT(_x)		((_x) << 29)
#define DOEPTSIZ0_PKTCNT		(1 << 19)
#define DOEPTSIZ0_XFERSIZE_MASK		(0x7f << 0)
#define DOEPTSIZ0_XFERSIZE_SHIFT	0

#define DWUSB_DMAC_BS_HOST_BUSY		(3 << 30)
#define DWUSB_DMAC_BS_DMA_DONE		(2 << 30)
#define DWUSB_DMAC_BS_DMA_BUSY		(1 << 30)
#define DWUSB_DMAC_BS_HOST_READY	(0 << 30)
#define DWUSB_DMAC_LAST			(1 << 27)
#define DWUSB_DMAC_SP			(1 << 26)
#define DWUSB_DMAC_IOC			(1 << 25)
#define DWUSB_DMAC_SR			(1 << 24)
#define DWUSB_DMAC_BYTES(x)		(x & 0xffff)

#define DW_USB_TIMEOUT			10000

typedef struct dw_usb_region {
	uintptr_t		base;
	size_t			size;
} dw_usb_region_t;

typedef struct dw_usb_params {
	uintptr_t		reg_base;
	dw_usb_region_t		desc;
	dw_usb_region_t		buffer;
	const char		*serialno;
} dw_usb_params_t;

typedef struct dw_usb_dma_desc {
	unsigned int		status;		/* DMA descriptor status */
	unsigned int		buf;		/* DMA data buffer */
} dw_usb_dma_desc_t;

void dw_usb_init(const dw_usb_params_t *params);

#endif	/* __DWUSB_H__ */
