NXP i.MX 8M Series
==================

The i.MX 8M family of applications processors based on Arm Corte-A53 and Cortex-M4
cores provide high-performance computing, power efficiency, enhanced system
reliability and embedded security needed to drive the growth of fast-growing
edge node computing, streaming multimedia, and machine learning applications.

Boot Sequence
-------------

Bootrom --> SPL --> BL31 --> BL33(u-boot) --> Linux kernel

How to build
------------

Build Procedure
~~~~~~~~~~~~~~~

-  Prepare AARCH64 toolchain.

-  Build spl and u-boot firstly, and get binary images: u-boot-spl.bin,
   u-boot-nodtb.bin and dtb for the target board.

-  Build TF-A

   Build bl31:

   .. code:: shell

       CROSS_COMPILE=aarch64-linux-gnu- make PLAT=<Target_SoC> bl31

   Target_SoC should be "imx8mq" for i.MX8MQ SoC.
   Target_SoC should be "imx8mm" for i.MX8MM SoC.
   Target_SoC should be "imx8mn" for i.MX8MN SoC.
   Target_SoC should be "imx8mp" for i.MX8MP SoC.

Deploy TF-A Images
~~~~~~~~~~~~~~~~~~

TF-A binary(bl31.bin), u-boot-spl.bin u-boot-nodtb.bin and dtb are combined
together to generate a binary file called flash.bin, the imx-mkimage tool is
used to generate flash.bin, and flash.bin needs to be flashed into SD card
with certain offset for BOOT ROM. the u-boot and imx-mkimage will be upstreamed
soon, this doc will be updated once they are ready, and the link will be posted.

TBBR Boot Sequence
------------------

When setting NEED_BL2=1 on imx8mm. We support an alternative way of
boot sequence to support TBBR.

Bootrom --> SPL --> BL2 --> BL31 --> BL33(u-boot with UEFI) --> grub

This helps us to fulfill the SystemReady EBBR standard.
BL2 will be in the FIT image and SPL will verify it.
All of the BL3x will be put in the FIP image. BL2 will verify them.
In U-boot we turn on the UEFI secure boot features so it can verify
grub. And we use grub to verify linux kernel.
