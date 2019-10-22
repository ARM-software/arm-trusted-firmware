Trusted Firmware-A Documentation
================================

.. toctree::
   :maxdepth: 1
   :hidden:

   Home<self>
   about/index
   getting_started/index
   process/index
   components/index
   design/index
   plat/index
   perf/index
   security_advisories/index
   change-log
   glossary
   license

Trusted Firmware-A (TF-A) provides a reference implementation of secure world
software for `Armv7-A and Armv8-A`_, including a `Secure Monitor`_ executing
at Exception Level 3 (EL3). It implements various Arm interface standards,
such as:

-  The `Power State Coordination Interface (PSCI)`_
-  `Trusted Board Boot Requirements CLIENT (TBBR-CLIENT)`_
-  `SMC Calling Convention`_
-  `System Control and Management Interface (SCMI)`_
-  `Software Delegated Exception Interface (SDEI)`_

Where possible, the code is designed for reuse or porting to other Armv7-A and
Armv8-A model and hardware platforms.

This release provides a suitable starting point for productization of secure
world boot and runtime firmware, in either the AArch32 or AArch64 execution
states.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from TF-A.

In collaboration with interested parties, we will continue to enhance |TF-A|
with reference implementations of Arm standards to benefit developers working
with Armv7-A and Armv8-A TrustZone technology.

Getting Started
---------------

The |TF-A| documentation contains guidance for obtaining and building the
software for existing, supported platforms, as well as supporting information
for porting the software to a new platform.

The **About** chapter gives a high-level overview of |TF-A| features as well as
some information on the project and how it is organized.

Refer to the documents in the **Getting Started** chapter for information about
the prerequisites and requirements for building |TF-A|.

The **Processes & Policies** chapter explains the project's release schedule
and process, how security disclosures are handled, and the guidelines for
contributing to the project (including the coding style).

The **Components** chapter holds documents that explain specific components
that make up the |TF-A| software, the :ref:`Exception Handling Framework`, for
example.

In the **System Design** chapter you will find documents that explain the
design of portions of the software that involve more than one component, such
as the :ref:`Trusted Board Boot` process.

**Platform Ports** provides a list of the supported hardware and software-model
platforms that are supported upstream in |TF-A|. Most of these platforms also
have additional documentation that has been provided by the maintainers of the
platform.

The results of any performance evaluations are added to the
**Performance & Testing** chapter.

**Security Advisories** holds a list of documents relating to |CVE| entries that
have previously been raised against the software.

Platforms
---------

Various AArch32 and AArch64 builds of this release have been tested on r0, r1
and r2 variants of the `Juno Arm Development Platform`_.

The latest version of the AArch64 build of TF-A has been tested on the following
Arm FVPs without shifted affinities, and that do not support threaded CPU cores
(64-bit host machine only).

.. note::
   The FVP models used are Version 11.6 Build 45, unless otherwise stated.

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_AEMv8A-AEMv8A-AEMv8A-AEMv8A-CCN502``
-  ``FVP_Base_RevC-2xAEMv8A``
-  ``FVP_Base_Cortex-A32x4``
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A55x4+Cortex-A75x4``
-  ``FVP_Base_Cortex-A55x4``
-  ``FVP_Base_Cortex-A57x1-A53x1``
-  ``FVP_Base_Cortex-A57x2-A53x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``
-  ``FVP_Base_Cortex-A75x4``
-  ``FVP_Base_Cortex-A76x4``
-  ``FVP_Base_Cortex-A76AEx4`` (Tested with internal model)
-  ``FVP_Base_Cortex-A76AEx8`` (Tested with internal model)
-  ``FVP_Base_Cortex-A77x4`` (Version 11.7 build 36)
-  ``FVP_Base_Neoverse-N1x4`` (Tested with internal model)
-  ``FVP_CSS_SGI-575`` (Version 11.3 build 42)
-  ``FVP_CSS_SGM-775`` (Version 11.3 build 42)
-  ``FVP_RD_E1Edge`` (Version 11.3 build 42)
-  ``FVP_RD_N1Edge`` (Version 11.3 build 42)
-  ``Foundation_Platform``

The latest version of the AArch32 build of TF-A has been tested on the following
Arm FVPs without shifted affinities, and that do not support threaded CPU cores
(64-bit host machine only).

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_Cortex-A32x4``

.. note::
   The ``FVP_Base_RevC-2xAEMv8A`` FVP only supports shifted affinities.

The Foundation FVP can be downloaded free of charge. The Base FVPs can be
licensed from Arm. See the `Arm FVP website`_.

All the above platforms have been tested with `Linaro Release 19.06`_.

This release also contains the following platform support:

-  Allwinner sun50i_a64 and sun50i_h6
-  Amlogic Meson S905 (GXBB)
-  Arm Juno Software Development Platform
-  Arm Neoverse N1 System Development Platform (N1SDP)
-  Arm Neoverse Reference Design N1 Edge (RD-N1-Edge) FVP
-  Arm Neoverse Reference Design E1 Edge (RD-E1-Edge) FVP
-  Arm SGI-575 and SGM-775
-  Arm Versatile Express FVP
-  HiKey, HiKey960 and Poplar boards
-  Intel Stratix 10 SoC FPGA
-  Marvell Armada 3700 and 8K
-  MediaTek MT6795 and MT8173 SoCs
-  NVIDIA T132, T186 and T210 SoCs
-  NXP QorIQ LS1043A, i.MX8MM, i.MX8MQ, i.MX8QX, i.MX8QM and i.MX7Solo WaRP7
-  QEMU
-  Raspberry Pi 3
-  Renesas R-Car Generation 3
-  RockChip RK3328, RK3368 and RK3399 SoCs
-  Socionext UniPhier SoC family and SynQuacer SC2A11 SoCs
-  STMicroelectronics STM32MP1
-  Texas Instruments K3 SoCs
-  Xilinx Versal and Zynq UltraScale + MPSoC

--------------

*Copyright (c) 2013-2019, Arm Limited and Contributors. All rights reserved.*

.. _Armv7-A and Armv8-A: https://developer.arm.com/products/architecture/a-profile
.. _Secure Monitor: http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php
.. _Power State Coordination Interface (PSCI): http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
.. _Trusted Board Boot Requirements CLIENT (TBBR-CLIENT): https://developer.arm.com/docs/den0006/latest/trusted-board-boot-requirements-client-tbbr-client-armv8-a
.. _System Control and Management Interface (SCMI): http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf
.. _Software Delegated Exception Interface (SDEI): http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
.. _Juno Arm Development Platform: http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php
.. _Arm FVP website: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Linaro Release 19.06: http://releases.linaro.org/members/arm/platforms/19.06
.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/ARM_DEN0028B_SMC_Calling_Convention.pdf
