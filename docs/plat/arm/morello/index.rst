Morello Platform
================

Morello is an ARMv8-A platform that implements the capability architecture extension.
The platform port present at `site <https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git>`_
provides ARMv8-A architecture enablement.

Capability architecture specific changes will be added `here <https://git.morello-project.org/morello>`_

Further information on Morello Platform is available at `info <https://developer.arm.com/architectures/cpu-architecture/a-profile/morello>`_

Boot Sequence
-------------

The execution begins from SCP_BL1 which loads the SCP_BL2 and starts its
execution. SCP_BL2 powers up the AP which starts execution at AP_BL31. The AP
then continues executing and hands off execution to Non-secure world (UEFI).

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain arm `toolchain <https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads>`_.
   Set the CROSS_COMPILE environment variable to point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf-

      make PLAT=morello all

*Copyright (c) 2020, Arm Limited. All rights reserved.*
