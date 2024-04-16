RD-1 AE (Kronos) Platform
=========================

Some of the features of the RD-1 AE platform referenced in TF-A include:

- Neoverse-V3AE, Arm9.2-A application processor (64-bit mode)
- A GICv4-compatible GIC-720AE

Further information on RD1-AE is available at `rd1ae`_

Boot Sequence
-------------

BL2 –> BL31 –> BL33

The boot process starts from RSE (Runtime Security Engine) that loads the BL2 image
and signals the System Control Processor (SCP) to power up the Application Processor (AP).
The AP then runs BL2, which loads the rest of the images, including the runtime firmware
BL31, and proceeds to execute it. Finally, it passes control to the non-secure world
BL33 (u-boot).

BL2 performs the actions described in the `Trusted Board Boot (TBB)`_ document.

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain `Arm toolchain`_ and set the CROSS_COMPILE environment variable to
   point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make \
      PLAT=rd1ae \
      MBEDTLS_DIR=<mbedtls_dir> \
      ARCH=aarch64 \
      CREATE_KEYS=1 \
      GENERATE_COT=1 \
      TRUSTED_BOARD_BOOT=1 \
      COT=tbbr \
      ARM_ROTPK_LOCATION=devel_rsa \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem \
      BL33=<path to u-boot binary> \

*Copyright (c) 2024, Arm Limited. All rights reserved.*

.. _Arm Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads
.. _rd1ae: https://developer.arm.com/Tools%20and%20Software/Arm%20Reference%20Design-1%20AE
.. _Trusted Board Boot (TBB): https://trustedfirmware-a.readthedocs.io/en/latest/design/trusted-board-boot.html
