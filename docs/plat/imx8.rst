NXP i.MX 8 Series
=================

The i.MX 8 series of applications processors is a feature- and
performance-scalable multi-core platform that includes single-,
dual-, and quad-core families based on the Arm® Cortex®
architecture—including combined Cortex-A72 + Cortex-A53,
Cortex-A35, and Cortex-M4 based solutions for advanced graphics,
imaging, machine vision, audio, voice, video, and safety-critical
applications.

The i.MX8QM is with 2 Cortex-A72 ARM core, 4 Cortex-A53 ARM core
and 1 Cortex-M4 system controller.

The i.MX8QX is with 4 Cortex-A35 ARM core and 1 Cortex-M4 system
controller.

The System Controller (SC) represents the evolution of centralized
control for system-level resources on i.MX8. The heart of the system
controller is a Cortex-M4 that executes system controller firmware.

Boot Sequence
-------------

Bootrom --> BL31 --> BL33(u-boot) --> Linux kernel

How to build
------------

Build Procedure
~~~~~~~~~~~~~~~

-  Prepare AARCH64 toolchain.

-  Build System Controller Firmware and u-boot firstly, and get binary images: scfw_tcm.bin and u-boot.bin

-  Build TF-A

   Build bl31:

   .. code:: shell

       CROSS_COMPILE=aarch64-linux-gnu- make PLAT=<Target_SoC> bl31

   Target_SoC should be "imx8qm" for i.MX8QM SoC.
   Target_SoC should be "imx8qx" for i.MX8QX SoC.

Deploy TF-A Images
~~~~~~~~~~~~~~~~~~

TF-A binary(bl31.bin), scfw_tcm.bin and u-boot.bin are combined together
to generate a binary file called flash.bin, the imx-mkimage tool is used
to generate flash.bin, and flash.bin needs to be flashed into SD card
with certain offset for BOOT ROM. The system controller firmware,
u-boot and imx-mkimage will be upstreamed soon, this doc will be updated
once they are ready, and the link will be posted.

.. _i.MX8: https://www.nxp.com/products/processors-and-microcontrollers/applications-processors/i.mx-applications-processors/i.mx-8-processors/i.mx-8-family-arm-cortex-a53-cortex-a72-virtualization-vision-3d-graphics-4k-video:i.MX8
