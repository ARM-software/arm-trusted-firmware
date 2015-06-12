Trusted Little Kernel (TLK) Dispatcher
=======================================
TLK dispatcher adds support for NVIDIA's Trusted Little Kernel (TLK) to work
with the Trusted Firmware. TLK-D can be compiled by including it in the
platform's makefile. TLK is primarily meant to work with Tegra SoCs, so until
Trusted Firmware starts supporting Tegra, the dispatcher code can only be
compiled for other platforms.

In order to compile TLK-D, we need a BL32 image to be present. Since, TLKD
just needs to compile, any BL32 image would do. To use TLK as the BL32, please
refer to the "Build TLK" section.

Once a BL32 is ready, TLKD can be included in the image using the following
command:

CROSS_COMPILE=<path_to_linaro_chain>/bin/aarch64-none-elf- make NEED_BL1=0
NEED_BL2=0 BL32=<path_to_BL32_image> PLAT=<platform> SPD=tlkd all
_
Trusted Little Kernel (TLK)
===========================
TLK is a Trusted OS running as Secure EL1. It is a Free Open Source Software
(FOSS) release of the NVIDIA® Trusted Little Kernel (TLK) technology, which
extends technology made available with the development of the Little Kernel (LK).
You can download the LK modular embedded preemptive kernel for use on ARM,
x86, and AVR32 systems from https://github.com/travisg/lk

NVIDIA implemented its Trusted Little Kernel (TLK) technology, designed as a
free and open-source trusted execution environment (OTE).

TLK features include:

• Small, pre-emptive kernel
• Supports multi-threading, IPCs, and thread scheduling
• Added TrustZone features
• Added Secure Storage
• Under MIT/FreeBSD license

NVIDIA extensions to Little Kernel (LK) include:

• User mode
• Address-space separation for TAs
• TLK Client Application (CA) library
• TLK TA library
• Crypto library (encrypt/decrypt, key handling) via OpenSSL
• Linux kernel driver
• Cortex A9/A15 support
• Power Management
• TrustZone memory carve-out (reconfigurable)
• Page table management
• Debugging support over UART (USB planned)

TLK is hosted by NVIDIA on http://nv-tegra.nvidia.com under the
3rdparty/ote_partner/tlk.git repository. Detailed information about
TLK and OTE can be found in the Tegra_BSP_for_Android_TLK_FOSS_Reference.pdf
manual located under the "documentation" directory_.

Build TLK
=========
To build and execute TLK, follow the instructions from "Building a TLK Device"
section from Tegra_BSP_for_Android_TLK_FOSS_Reference.pdf manual.
