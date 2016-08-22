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

#include <assert.h>
#include <debug.h>
#include <fastboot/fastboot.h>
#include <fastboot/usb.h>
#include <mmio.h>
#include <platform.h>

static const usb_ops_t *usb_ops;
static int usb_configured;

static int usb_handle_setup_packet(void)
{
	setup_packet_t *setup = NULL;

	assert((usb_ops != NULL) && (usb_ops->handle_setup != NULL));
	setup = usb_ops->handle_setup();
	if ((setup != NULL) && (setup->request == USB_REQ_SET_CONFIGURATION))
		usb_configured = 1;
	return 0;
}

static int usb_is_configured(void)
{
	return usb_configured;
}

static int usb_download(uintptr_t buf, size_t size)
{
	usb_interrupt_t event;
	int result;
	size_t left, length;

	assert((usb_ops != NULL) &&
	       (usb_ops->poll != NULL) &&
	       (usb_ops->receive_epx != NULL));
	left = size;

	while (1) {
		memset(&event, 0, sizeof(usb_interrupt_t));
		result = usb_ops->poll(&event, &length);
		if (result != 0)
			continue;

		if (event == USB_INT_EPX_OUT) {
			buf += length;
			left = left - length;
			if (left == 0)
				return size;
			usb_ops->receive_epx(buf, left);
		} else if (event == USB_INT_EPX_IN) {
			usb_ops->receive_epx(buf, left);
		} else {
			assert(0);
		}
	}
}

static int usb_read(uintptr_t buf, size_t size)
{
	usb_interrupt_t event;
	int result;
	size_t length;

	assert((usb_ops != NULL) &&
	       (usb_ops->poll != NULL) &&
	       (usb_ops->prepare_setup != NULL) &&
	       (usb_ops->receive_epx != NULL));
	while (1) {
		memset(&event, 0, sizeof(usb_interrupt_t));
		result = usb_ops->poll(&event, &length);
		if (result != 0)
			continue;

		if (event == USB_INT_EP0_SETUP) {
			result = usb_handle_setup_packet();
			if (result == 0)
				usb_ops->receive_epx(0, RX_REQ_LEN);
		} else if (event == USB_INT_EP0_OUT) {
			usb_ops->prepare_setup();
		} else if (event == USB_INT_EPX_OUT) {
			return length;
		} else if (event == USB_INT_EPX_IN) {
			usb_ops->receive_epx(0, RX_REQ_LEN);
		}
	}
}

static int usb_write(uintptr_t buf, size_t size)
{
	assert((usb_ops != NULL) && (usb_ops->submit_packet != NULL));
	usb_ops->submit_packet(buf, size);
	return 0;
}

static fastboot_ops_t fb_ops = {
	.download	= usb_download,
	.is_attached	= usb_is_configured,
	.read		= usb_read,
	.write		= usb_write,
};

static int usb_enum(void)
{
	usb_interrupt_t event;
	int result;

	assert((usb_ops != NULL) &&
	       (usb_ops->init != NULL) &&
	       (usb_ops->poll != NULL) &&
	       (usb_ops->prepare_setup != NULL));
	usb_ops->init();
	usb_ops->prepare_setup();
	while (usb_configured == 0) {
		memset(&event, 0, sizeof(usb_interrupt_t));
		result = usb_ops->poll(&event, NULL);
		if (result != 0)
			continue;

		if (event == USB_INT_EP0_SETUP) {
			result = usb_handle_setup_packet();
			assert(result == 0);
			usb_ops->prepare_setup();
		} else if (event == USB_INT_EP0_OUT) {
			usb_ops->prepare_setup();
		} else if (event == USB_INT_EP0_IN) {
			usb_ops->prepare_setup();
		}
	}
	INFO("usb online\n");
	usb_ops->prepare_setup();
	return 0;
}

void usb_init(const usb_ops_t *ops_ptr)
{
	assert((ops_ptr != NULL) &&			\
	       (ops_ptr->handle_setup != NULL) &&	\
	       (ops_ptr->init != NULL) &&		\
	       (ops_ptr->poll != NULL) &&		\
	       (ops_ptr->prepare_setup != NULL) &&	\
	       (ops_ptr->receive_epx != NULL) &&	\
	       (ops_ptr->submit_packet != NULL));
	usb_ops = ops_ptr;

	usb_enum();
	fastboot_init(&fb_ops);
}
