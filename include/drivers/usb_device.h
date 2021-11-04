/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include <stdint.h>

#include <lib/utils_def.h>

#define USBD_MAX_NUM_INTERFACES			1U
#define USBD_MAX_NUM_CONFIGURATION		1U

#define USB_LEN_DEV_QUALIFIER_DESC		0x0AU
#define USB_LEN_DEV_DESC			0x12U
#define USB_LEN_CFG_DESC			0x09U
#define USB_LEN_IF_DESC				0x09U
#define USB_LEN_EP_DESC				0x07U
#define USB_LEN_OTG_DESC			0x03U
#define USB_LEN_LANGID_STR_DESC			0x04U
#define USB_LEN_OTHER_SPEED_DESC_SIZ		0x09U

#define USBD_IDX_LANGID_STR			0x00U
#define USBD_IDX_MFC_STR			0x01U
#define USBD_IDX_PRODUCT_STR			0x02U
#define USBD_IDX_SERIAL_STR			0x03U
#define USBD_IDX_CONFIG_STR			0x04U
#define USBD_IDX_INTERFACE_STR			0x05U
#define USBD_IDX_USER0_STR			0x06U

#define USB_REQ_TYPE_STANDARD			0x00U
#define USB_REQ_TYPE_CLASS			0x20U
#define USB_REQ_TYPE_VENDOR			0x40U
#define USB_REQ_TYPE_MASK			0x60U

#define USB_REQ_RECIPIENT_DEVICE		0x00U
#define USB_REQ_RECIPIENT_INTERFACE		0x01U
#define USB_REQ_RECIPIENT_ENDPOINT		0x02U
#define USB_REQ_RECIPIENT_MASK			0x1FU

#define USB_REQ_DIRECTION			0x80U

#define USB_REQ_GET_STATUS			0x00U
#define USB_REQ_CLEAR_FEATURE			0x01U
#define USB_REQ_SET_FEATURE			0x03U
#define USB_REQ_SET_ADDRESS			0x05U
#define USB_REQ_GET_DESCRIPTOR			0x06U
#define USB_REQ_SET_DESCRIPTOR			0x07U
#define USB_REQ_GET_CONFIGURATION		0x08U
#define USB_REQ_SET_CONFIGURATION		0x09U
#define USB_REQ_GET_INTERFACE			0x0AU
#define USB_REQ_SET_INTERFACE			0x0BU
#define USB_REQ_SYNCH_FRAME			0x0CU

#define USB_DESC_TYPE_DEVICE			0x01U
#define USB_DESC_TYPE_CONFIGURATION		0x02U
#define USB_DESC_TYPE_STRING			0x03U
#define USB_DESC_TYPE_INTERFACE			0x04U
#define USB_DESC_TYPE_ENDPOINT			0x05U
#define USB_DESC_TYPE_DEVICE_QUALIFIER		0x06U
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION	0x07U
#define USB_DESC_TYPE_BOS			0x0FU

#define USB_CONFIG_REMOTE_WAKEUP		2U
#define USB_CONFIG_SELF_POWERED			1U

#define USB_MAX_EP0_SIZE			64U

/* Device Status */
#define USBD_STATE_DEFAULT			1U
#define USBD_STATE_ADDRESSED			2U
#define USBD_STATE_CONFIGURED			3U
#define USBD_STATE_SUSPENDED			4U

/* EP0 State */
#define USBD_EP0_IDLE				0U
#define USBD_EP0_SETUP				1U
#define USBD_EP0_DATA_IN			2U
#define USBD_EP0_DATA_OUT			3U
#define USBD_EP0_STATUS_IN			4U
#define USBD_EP0_STATUS_OUT			5U
#define USBD_EP0_STALL				6U

#define USBD_EP_TYPE_CTRL			0U
#define USBD_EP_TYPE_ISOC			1U
#define USBD_EP_TYPE_BULK			2U
#define USBD_EP_TYPE_INTR			3U

#define  USBD_OUT_EPNUM_MASK			GENMASK(15, 0)
#define  USBD_OUT_COUNT_MASK			GENMASK(31, 16)
#define  USBD_OUT_COUNT_SHIFT			16U

/* Number of EP supported, allow to reduce footprint: default max = 15 */
#ifndef CONFIG_USBD_EP_NB
#define  USBD_EP_NB				15U
#else
#define  USBD_EP_NB				CONFIG_USBD_EP_NB
#endif

#define LOBYTE(x)	((uint8_t)((x) & 0x00FF))
#define HIBYTE(x)	((uint8_t)(((x) & 0xFF00) >> 8))

struct usb_setup_req {
	uint8_t bm_request;
	uint8_t b_request;
	uint16_t value;
	uint16_t index;
	uint16_t length;
};

struct usb_handle;

struct usb_class {
	uint8_t (*init)(struct usb_handle *pdev, uint8_t cfgidx);
	uint8_t (*de_init)(struct usb_handle *pdev, uint8_t cfgidx);
	/* Control Endpoints */
	uint8_t (*setup)(struct usb_handle *pdev, struct usb_setup_req *req);
	uint8_t (*ep0_tx_sent)(struct usb_handle *pdev);
	uint8_t (*ep0_rx_ready)(struct usb_handle *pdev);
	/* Class Specific Endpoints */
	uint8_t (*data_in)(struct usb_handle *pdev, uint8_t epnum);
	uint8_t (*data_out)(struct usb_handle *pdev, uint8_t epnum);
	uint8_t (*sof)(struct usb_handle *pdev);
	uint8_t (*iso_in_incomplete)(struct usb_handle *pdev, uint8_t epnum);
	uint8_t (*iso_out_incomplete)(struct usb_handle *pdev, uint8_t epnum);
};

/* Following USB Device status */
enum usb_status {
	USBD_OK = 0U,
	USBD_BUSY,
	USBD_FAIL,
	USBD_TIMEOUT
};

/* Action to do after IT handling */
enum usb_action {
	USB_NOTHING = 0U,
	USB_DATA_OUT,
	USB_DATA_IN,
	USB_SETUP,
	USB_ENUM_DONE,
	USB_READ_DATA_PACKET,
	USB_READ_SETUP_PACKET,
	USB_RESET,
	USB_RESUME,
	USB_SUSPEND,
	USB_LPM,
	USB_SOF,
	USB_DISCONNECT,
	USB_WRITE_EMPTY
};

/* USB Device descriptors structure */
struct usb_desc {
	uint8_t *(*get_device_desc)(uint16_t *length);
	uint8_t *(*get_lang_id_desc)(uint16_t *length);
	uint8_t *(*get_manufacturer_desc)(uint16_t *length);
	uint8_t *(*get_product_desc)(uint16_t *length);
	uint8_t *(*get_serial_desc)(uint16_t *length);
	uint8_t *(*get_configuration_desc)(uint16_t *length);
	uint8_t *(*get_interface_desc)(uint16_t *length);
	uint8_t *(*get_usr_desc)(uint8_t index, uint16_t *length);
	uint8_t *(*get_config_desc)(uint16_t *length);
	uint8_t *(*get_device_qualifier_desc)(uint16_t *length);
	/* optional: high speed capable device operating at its other speed */
	uint8_t *(*get_other_speed_config_desc)(uint16_t *length);
};

/* USB Device handle structure */
struct usb_endpoint {
	uint32_t status;
	uint32_t total_length;
	uint32_t rem_length;
	uint32_t maxpacket;
};

/*
 * EndPoint descriptor
 * num : Endpoint number, between 0 and 15 (limited by USBD_EP_NB)
 * is_in: Endpoint direction
 * type : Endpoint type
 * maxpacket:  Endpoint Max packet size: between 0 and 64KB
 * xfer_buff: Pointer to transfer buffer
 * xfer_len: Current transfer lengt
 * hxfer_count: Partial transfer length in case of multi packet transfer
 */
struct usbd_ep {
	uint8_t num;
	bool is_in;
	uint8_t type;
	uint32_t maxpacket;
	uint8_t *xfer_buff;
	uint32_t xfer_len;
	uint32_t xfer_count;
};

enum pcd_lpm_state {
	LPM_L0 = 0x00U, /* on */
	LPM_L1 = 0x01U, /* LPM L1 sleep */
	LPM_L2 = 0x02U, /* suspend */
	LPM_L3 = 0x03U, /* off */
};

/* USB Device descriptors structure */
struct usb_driver {
	enum usb_status (*ep0_out_start)(void *handle);
	enum usb_status (*ep_start_xfer)(void *handle, struct usbd_ep *ep);
	enum usb_status (*ep0_start_xfer)(void *handle, struct usbd_ep *ep);
	enum usb_status (*write_packet)(void *handle, uint8_t *src,
				     uint8_t ch_ep_num, uint16_t len);
	void *(*read_packet)(void *handle, uint8_t *dest, uint16_t len);
	enum usb_status (*ep_set_stall)(void *handle, struct usbd_ep *ep);
	enum usb_status (*start_device)(void *handle);
	enum usb_status (*stop_device)(void *handle);
	enum usb_status (*set_address)(void *handle, uint8_t address);
	enum usb_status (*write_empty_tx_fifo)(void *handle,
					    uint32_t epnum, uint32_t xfer_len,
					    uint32_t *xfer_count,
					    uint32_t maxpacket,
					    uint8_t **xfer_buff);
	enum usb_action (*it_handler)(void *handle, uint32_t *param);
};

/* USB Peripheral Controller Drivers */
struct pcd_handle {
	void *instance; /* Register base address */
	struct usbd_ep in_ep[USBD_EP_NB]; /* IN endpoint parameters */
	struct usbd_ep out_ep[USBD_EP_NB]; /* OUT endpoint parameters */
	uint32_t setup[12]; /* Setup packet buffer */
	enum pcd_lpm_state lpm_state; /* LPM State */
};

/* USB Device handle structure */
struct usb_handle {
	uint8_t id;
	uint32_t dev_config;
	uint32_t dev_config_status;
	struct usb_endpoint ep_in[USBD_EP_NB];
	struct usb_endpoint ep_out[USBD_EP_NB];
	uint32_t ep0_state;
	uint32_t ep0_data_len;
	uint8_t dev_state;
	uint8_t dev_old_state;
	uint8_t dev_address;
	uint32_t dev_remote_wakeup;
	struct usb_setup_req request;
	const struct usb_desc *desc;
	struct usb_class *class;
	void *class_data;
	void *user_data;
	struct pcd_handle *data;
	const struct usb_driver *driver;
};

enum usb_status usb_core_handle_it(struct usb_handle *pdev);
enum usb_status usb_core_receive(struct usb_handle *pdev, uint8_t ep_addr,
				 uint8_t *p_buf, uint32_t len);
enum usb_status usb_core_transmit(struct usb_handle *pdev, uint8_t ep_addr,
				  uint8_t *p_buf, uint32_t len);
enum usb_status usb_core_receive_ep0(struct usb_handle *pdev, uint8_t *p_buf,
				     uint32_t len);
enum usb_status usb_core_transmit_ep0(struct usb_handle *pdev, uint8_t *p_buf,
				      uint32_t len);
void usb_core_ctl_error(struct usb_handle *pdev);
enum usb_status usb_core_start(struct usb_handle *pdev);
enum usb_status usb_core_stop(struct usb_handle *pdev);
enum usb_status register_usb_driver(struct usb_handle *pdev,
				    struct pcd_handle *pcd_handle,
				    const struct usb_driver *driver,
				    void *driver_handle);
enum usb_status register_platform(struct usb_handle *pdev,
				  const struct usb_desc *plat_call_back);

#endif /* USB_DEVICE_H */
