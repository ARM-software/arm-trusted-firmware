/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

__asm__(
	".pushsection .text.hdcp_handler, \"ax\", %progbits\n"
	".global hdcp_handler\n"
	".balign 4\n"
	"hdcp_handler:\n"
	".incbin \"" __XSTRING(HDCPFW) "\"\n"
	".type hdcp_handler, %function\n"
	".size hdcp_handler, .- hdcp_handler\n"
	".popsection\n"
);

