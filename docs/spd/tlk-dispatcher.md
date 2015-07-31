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

Once a BL32 is ready, TLKD can be included in the image by adding "SPD=tlkd"
to the build command.

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

Input parameters to TLK
=======================
TLK expects the TZDRAM size and a structure containing the boot arguments. BL2
passes this information to the EL3 software as members of the bl32_ep_info
struct, where bl32_ep_info is part of bl31_params_t (passed by BL2 in X0)

Example:
--------
    bl32_ep_info->args.arg0 = TZDRAM size available for BL32
    bl32_ep_info->args.arg1 = unused (used only on ARMv7)
    bl32_ep_info->args.arg2 = pointer to boot args

