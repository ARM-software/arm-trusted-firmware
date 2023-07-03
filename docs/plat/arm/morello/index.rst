Morello Platform
================

Morello is an ARMv8-A platform that implements the capability architecture extension.
The platform port present at `site <https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git>`_
provides ARMv8-A architecture enablement.

Capability architecture specific changes will be added `here <https://git.morello-project.org/morello>`_

Further information on Morello Platform is available at `info <https://developer.arm.com/architectures/cpu-architecture/a-profile/morello>`_

Boot Sequence
-------------

The SCP initializes the RVBAR registers to point to the AP_BL1. Once RVBAR is
initialized, the primary core is powered on. The primary core boots the AP_BL1.
It performs minimum initialization necessary to load and authenticate the AP
firmware image (the FIP image) from the AP QSPI NOR Flash Memory into the
Trusted SRAM.

AP_BL1 authenticates and loads the AP_BL2 image. AP_BL2 performs additional
initializations, and then authenticates and loads the AP_BL31 and AP_BL33.
AP_BL2 then transfers execution control to AP_BL31, which is the EL3 runtime
firmware. Execution is finally handed off to AP_BL33, which is the non-secure
world (UEFI).

SCP -> AP_BL1 -> AP_BL2 -> AP_BL31 -> AP_BL33

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain arm `toolchain <https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads>`_.
   Set the CROSS_COMPILE environment variable to point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf-

      make PLAT=morello all

*Copyright (c) 2020-2023, Arm Limited. All rights reserved.*
