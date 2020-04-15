Broadcom Stingray
=================

Description
-----------
Broadcom's Stingray(BCM958742t) is a multi-core processor with 8 Cortex-A72 cores.
Trusted Firmware-A (TF-A) is used to implement secure world firmware, supporting
BL2 and BL31 for Broadcom Stingray SoCs.

On Poweron, Boot ROM will load bl2 image and Bl2 will initialize the hardware,
then loads bl31 and bl33 into DDR and boots to bl33.

Boot Sequence
-------------

Bootrom --> TF-A BL2 --> TF-A BL31 --> BL33(u-boot)

Code Locations
~~~~~~~~~~~~~~
-  Trusted Firmware-A:
   `link <https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/>`__

How to build
------------

Build Procedure
~~~~~~~~~~~~~~~

-  Prepare AARCH64 toolchain.

-  Build u-boot first, and get the binary image: u-boot.bin,

-  Build TF-A

   Build fip:

   .. code:: shell

       make CROSS_COMPILE=aarch64-linux-gnu- PLAT=stingray BOARD_CFG=bcm958742t all fip BL33=u-boot.bin

Deploy TF-A Images
~~~~~~~~~~~~~~~~~~
The u-boot will be upstreamed soon, this doc will be updated once they are ready, and the link will be posted.
