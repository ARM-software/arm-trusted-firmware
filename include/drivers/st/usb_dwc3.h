/*
 * Copyright (c) 2015-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DWC3_H
#define __USB_DWC3_H

#include <drivers/usb_device.h>
#include <platform_def.h>

/* define value use in register */

#define USB3_HS_MAX_PACKET_SIZE     512U
#define USB3_FS_MAX_PACKET_SIZE     64U
#ifdef USE_USB_FS
#define USB3_MAX_PACKET_SIZE        USB3_FS_MAX_PACKET_SIZE
#else
#define USB3_MAX_PACKET_SIZE        USB3_HS_MAX_PACKET_SIZE
#endif
#define USB_SETUP_PACKET_SIZE       12U

/* USB_DWC3_EPTypeDef->trb_flag */
#define USB_DWC3_TRBCTL_NORMAL            1
#define USB_DWC3_TRBCTL_CONTROL_SETUP     2
#define USB_DWC3_TRBCTL_CONTROL_STATUS2   3
#define USB_DWC3_TRBCTL_CONTROL_STATUS3   4
#define USB_DWC3_TRBCTL_CONTROL_DATA      5
#define USB_DWC3_TRBCTL_ISOCHRONOUS_FIRST 6
#define USB_DWC3_TRBCTL_ISOCHRONOUS       7
#define USB_DWC3_TRBCTL_LINK_TRB          8
#define USB_DWC3_TRBCTL_MASK              0x3fUL

/* USB_DWC3_EPTypeDef->flags */
#define USB_DWC3_EP_REQ_QUEUED          BIT(0)
#define USB_DWC3_EP_ISOC_START_PENDING  BIT(1)
#define USB_DWC3_EP_ISOC_STARTED        BIT(2)

/*
 * DWC3 Specific Configurable macros
 */
#define USB_DWC3_INT_INUSE          1U
#define USB_DWC3_EVENT_BUFFER_SIZE  2048U
#define USB_DWC3_NUM_IN_EP          USBD_EP_NB
#define USB_DWC3_NUM_OUT_EP         USBD_EP_NB

typedef struct {
	uint32_t dcfg;             /* @ offset 0x0 */
	uint32_t dctl;             /* @ offset 0x4 */
	uint32_t devten;           /* @ offset 0x8 */
	uint32_t dsts;             /* @ offset 0xc */
	uint32_t dgcmdpar;         /* @ offset 0x10 */
	uint32_t dgcmd;            /* @ offset 0x14 */
	uint32_t reserved_18[2];   /* Reserved @ offset 0x18 */
	uint32_t dalepena;         /* @ offset 0x20 */
	uint32_t reserved_24;      /* @ offset 0x24 */
	uint32_t reserved_28[54];  /* Reserved @ offset 0x28 */
	uint32_t depcmdpar2;       /* @ offset 0x100 */
	uint32_t depcmdpar1;       /* @ offset 0x104 */
	uint32_t depcmdpar0;       /* @ offset 0x108 */
	uint32_t depcmd;           /* @ offset 0x10c */
	uint32_t reserved_110[124];/* Reserved @ offset 0x110 */
	uint32_t dev_imod;         /* @ offset 0x300 */
	uint32_t reserved_304[0xfc / 4]; /* Reserved @ offset 0x304 */
} usb_dwc3_device_t;

typedef struct {
	uint32_t gsbuscfg0;         /* @ offset 0x0 */
	uint32_t gsbuscfg1;         /* @ offset 0x4 */
	uint32_t gtxthrcfg;         /* @ offset 0x8 */
	uint32_t grxthrcfg;         /* @ offset 0xc */
	uint32_t gctl;              /* @ offset 0x10 */
	uint32_t gpmsts;            /* @ offset 0x14 */
	uint32_t gsts;              /* @ offset 0x18 */
	uint32_t guctl1;            /* @ offset 0x1c */
	uint32_t gsnpsid;           /* @ offset 0x20 */
	uint32_t ggpio;             /* @ offset 0x24 */
	uint32_t guid;              /* @ offset 0x28 */
	uint32_t guctl;             /* @ offset 0x2c */
	uint32_t gbuserraddrlo;     /* @ offset 0x30 */
	uint32_t gbuserraddrhi;     /* @ offset 0x34 */
	uint32_t gprtbimaplo;       /* @ offset 0x38 */
	uint32_t gprtbimaphi;       /* @ offset 0x3c */
	uint32_t ghwparams0;        /* @ offset 0x40 */
	uint32_t ghwparams1;        /* @ offset 0x44 */
	uint32_t ghwparams2;        /* @ offset 0x48 */
	uint32_t ghwparams3;        /* @ offset 0x4c */
	uint32_t ghwparams4;        /* @ offset 0x50 */
	uint32_t ghwparams5;        /* @ offset 0x54 */
	uint32_t ghwparams6;        /* @ offset 0x58 */
	uint32_t ghwparams7;        /* @ offset 0x5c */
	uint32_t gdbgfifospace;     /* @ offset 0x60 */
	uint32_t gdbgltssm;         /* @ offset 0x64 */
	uint32_t gdbglnmcc;         /* @ offset 0x68 */
	uint32_t gdbgbmu;           /* @ offset 0x6c */
	uint32_t gdbglspmux_hst;    /* @ offset 0x70 */
	uint32_t gdbglsp;           /* @ offset 0x74 */
	uint32_t gdbgepinfo0;       /* @ offset 0x78 */
	uint32_t gdbgepinfo1;       /* @ offset 0x7c */
	uint32_t gprtbimap_hslo;    /* @ offset 0x80 */
	uint32_t gprtbimap_hshi;    /* @ offset 0x84 */
	uint32_t gprtbimap_fslo;    /* @ offset 0x88 */
	uint32_t gprtbimap_fshi;    /* @ offset 0x8c */
	uint32_t reserved_90;       /* Reserved @ offset 0x90 */
	uint32_t reserved_94;       /* @ offset 0x94 */
	uint32_t reserved_98;       /* @ offset 0x98 */
	uint32_t guctl2;            /* @ offset 0x9c */
	uint32_t reserved_A0[24];   /* Reserved @ offset 0xa0 */
	uint32_t gusb2phycfg;       /* @ offset 0x100 */
	uint32_t reserved_104[15];  /* Reserved @ offset 0x104 */
	uint32_t gusb2i2cctl;       /* @ offset 0x140 */
	uint32_t reserved_144[15];  /* Reserved @ offset 0x144 */
	uint32_t gusb2phyacc_ulpi;  /* @ offset 0x180 */
	uint32_t reserved_184[15];  /* Reserved @ offset 0x184 */
	uint32_t gusb3pipectl;      /* @ offset 0x1c0 */
	uint32_t reserved_1c4[15];  /* Reserved @ offset 0x1c4 */
	uint32_t gtxfifosiz0;       /* @ offset 0x200 */
	uint32_t gtxfifosiz1;       /* @ offset 0x204 */
	uint32_t gtxfifosiz2;       /* @ offset 0x208 */
	uint32_t gtxfifosiz3;       /* @ offset 0x20c */
	uint32_t gtxfifosiz4;       /* @ offset 0x210 */
	uint32_t gtxfifosiz5;       /* @ offset 0x214 */
	uint32_t gtxfifosiz6;       /* @ offset 0x218 */
	uint32_t gtxfifosiz7;       /* @ offset 0x21c */
	uint32_t gtxfifosiz8;       /* @ offset 0x220 */
	uint32_t gtxfifosiz9;       /* @ offset 0x224 */
	uint32_t gtxfifosiz10;      /* @ offset 0x228 */
	uint32_t gtxfifosiz11;      /* @ offset 0x22c */
	uint32_t reserved_230[20];  /* Reserved @ offset 0x230 */
	uint32_t grxfifosiz0;       /* @ offset 0x280 */
	uint32_t grxfifosiz1;       /* @ offset 0x284 */
	uint32_t grxfifosiz2;       /* @ offset 0x288 */
	uint32_t reserved_28c[29];  /* Reserved @ offset 0x28c */
	uint32_t gevntadrlo;        /* @ offset 0x300 */
	uint32_t gevntadrhi;        /* @ offset 0x304 */
	uint32_t gevntsiz;          /* @ offset 0x308 */
	uint32_t gevntcount;        /* @ offset 0x30c */
	uint32_t reserved_310[124]; /* Reserved @ offset 0x310 */
	uint32_t ghwparams8;        /* @ offset 0x500 */
	uint32_t reserved_504[3];   /* Reserved @ offset 0x504 */
	uint32_t gtxfifopridev;     /* @ offset 0x510 */
	uint32_t reserved_514;      /* Reserved @ offset 0x514 */
	uint32_t gtxfifoprihst;     /* @ offset 0x518 */
	uint32_t grxfifoprihst;     /* @ offset 0x51c */
	uint32_t reserved_520;      /* Reserved @ offset 0x520 */
	uint32_t gdmahlratio;       /* @ offset 0x524 */
	uint32_t reserved_528[2];   /* Reserved @ offset 0x528 */
	uint32_t gfladj;            /* @ offset 0x530 */
	uint32_t reserved_534[0xcc / 4];  /* Reserved @ offset 0x534 */
} usb_dwc3_global_t;

typedef struct {
	uint32_t bpl;      /* Buffer Pointer Low Address */
	uint32_t bph;      /* Buffer Pointer High Address */
	uint32_t size;     /* Buffer Size */
	uint32_t ctrl;     /* Control and Status field */
} __packed usb_dwc3_trb_t __aligned(16);

#define CACHED_MEMORY
#define CACHE_LINE_SIZE CACHE_WRITEBACK_GRANULE

typedef struct {
	bool is_stall;				/* Endpoint stall condition */

	volatile usb_dwc3_trb_t *trb_addr; /* Trb descriptor coherent address */
	uintptr_t trb_dma_addr;   /* 32 bits aligned trb bus address */
	uint8_t trb_flag;         /* TRB Control Flag */
	uint8_t flags;            /* Endpoint state flags condition */
	uint8_t phy_epnum;        /* physical EP number 0 - 31 */
	uint8_t intr_num;         /* Interrupt number to get events for this EP */
	/* to be used for receiving packet < EP-Max-Packet-Size */
	uint8_t *bounce_buf;
	/* Transmission FIFO number. Number between Min_Data = 1 and Max_Data = 15 */
	uint8_t tx_fifo_num;
	uint8_t resc_idx;       /* Resource index */
	uintptr_t dma_addr;     /* 32 bits aligned transfer buffer address */
	/* Pointer to destination buffer, used for receive case, where bounce_buf is used */
	uint8_t *xfer_dest_buff;
	/* length of destination buffer, used for receive case, where bounce_buf is used */
	uint32_t xfer_dest_len;
} usb_dwc3_endpoint_t;

typedef struct {
	uint8_t bounce_buf[USB3_MAX_PACKET_SIZE];  /* Event Buffer area */
} usb_dwc3_endpoint_bouncebuf_t __aligned(CACHE_LINE_SIZE);

/* Aligned to make EvtBuffer start address evtbuffer-size aligned */
typedef struct {
	uint8_t EvtBufferArea[USB_DWC3_EVENT_BUFFER_SIZE];  /* Event Buffer area */
} PCD_intbufferareadef __aligned(USB_DWC3_EVENT_BUFFER_SIZE);

typedef struct {
	/* shield needed for the cache line (8 words) intersection with other members of structure,
	 * only when USB_DWC3_EVENT_BUFFER_SIZE is non-cache_line aligned
	 */

	uint8_t *evtbuffer_addr[USB_DWC3_INT_INUSE]; /* Event Buffer coherent addr */
	uintptr_t evtbuffer_dma_addr[USB_DWC3_INT_INUSE]; /* Event Buffer dma addr */
	uint32_t evtbufferpos[USB_DWC3_INT_INUSE];   /*!< Read Position inside the Event Buffer */
} PCD_intbuffersdef;

typedef enum {
	HAL_PCD_STATE_RESET   = 0x00U,
	HAL_PCD_STATE_READY   = 0x01U,
	HAL_PCD_STATE_ERROR   = 0x02U,
	HAL_PCD_STATE_BUSY    = 0x03U,
	HAL_PCD_STATE_TIMEOUT = 0x04U
} usb_dwc3_state_t;

typedef enum {
	HAL_PCD_EP0_SETUP_COMPLETED = 0x00U,
	HAL_PCD_EP0_SETUP_QUEUED  = 0x01U
} usb_dwc3_ep0state_t;

typedef struct {
	/*
	 * Kept at top to avoid struct size wastage since bounce_buf
	 * needs to be CACHE_LINE_SIZE aligned
	 */
	usb_dwc3_endpoint_bouncebuf_t bounce_bufs[USB_DWC3_NUM_OUT_EP];
	PCD_intbuffersdef intbuffers;
	usb_dwc3_global_t *usb_global;
	usb_dwc3_device_t *usb_device;
	usb_dwc3_endpoint_t IN_ep[USB_DWC3_NUM_IN_EP];
	usb_dwc3_endpoint_t OUT_ep[USB_DWC3_NUM_OUT_EP];

	usb_dwc3_state_t State;      /*!< PCD communication state            */
	usb_dwc3_ep0state_t EP0_State;  /*!< EP0 State */

	uint8_t *setup_addr;  /*!< Setup packet coherent address    */
	uintptr_t setup_dma_addr;  /*!< Setup packet buffer DMA Bus address */

	struct pcd_handle *pcd_handle;
} dwc3_handle_t;

void usb_dwc3_init_driver(struct usb_handle *usb_core_handle,
			  struct pcd_handle *pcd_handle, dwc3_handle_t *dwc3_handle,
			  void *base_addr);

#endif /* __USB_DWC3_H */
