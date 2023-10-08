NXP i.MX 8ULP
==================

i.MX 8ULP is part of the ULP family with emphasis on extreme low-power techniques
using the 28 nm fully depleted silicon on insulator process. Like i.MX 7ULP,
i.MX 8ULP continues to be based on asymmetric architecture.

The i.MX 8ULP family of processors features NXP’s advanced implementation of the
dual Arm Cortex-A35 cores alongside an Arm Cortex-M33. This combined architecture
enables the device to run a rich operating system (such as Linux) on the Cortex-A35
core and an RTOS (such as FreeRTOS) on the Cortex-M33 core. It also includes a Cadence
Tensilica Fusion DSP for low-power audio and a HiFi4 DSP for advanced audio and machine
learning applications.

The design enables clean separation between two processing domains, where each has
separate power, clocking and peripheral islands, but the bus fabric of each domain
is tightly integrated for efficient communication. The part is streamlined to minimize
pin count, enabling small packages and simple system integration. This microprocessor
is intended for applications where efficiency and simple system integration is important.
`i.MX8ULP Applications Processors`_.

Boot Sequence
-------------

BootROM --> SPL --> BL31 --> BL33(u-boot) --> Linux kernel

How to build
------------

Build Procedure
~~~~~~~~~~~~~~~

-  Prepare AARCH64 toolchain.

- Get the ELE FW image from NXP linux SDK package

-  Build SPL and u-boot firstly, and get binary images: u-boot-spl.bin,
   u-boot.bin and dtb

-  Build TF-A

   Build bl31:

   .. code:: shell

       CROSS_COMPILE=aarch64-linux-gnu- make PLAT=<Target_SoC> bl31

   Target_SoC should be "imx8ulp" for i.MX8ULP SoC.

Deploy TF-A Images
~~~~~~~~~~~~~~~~~~

TF-A binary(bl31.bin), u-boot-spl.bin u-boot.bin, ELE FW image are combined
together to generate a binary file called flash.bin, the imx-mkimage tool is
used to generate flash.bin, and flash.bin needs to be flashed into SD card
with certain offset for BOOT ROM.

Reference Documentation
~~~~~~~~~~~~~~~~~~~~~~~

Details on how to prepare, generate & deploy the boot image be found in following documents:

- i.MX Linux User's Guide
  `link <https://www.nxp.com/design/software/embedded-software/i-mx-software/embedded-linux-for-i-mx-applications-processors:IMXLINUX>`__
- i.MX Linux Reference Manual
  `link <https://www.nxp.com/design/software/embedded-software/i-mx-software/embedded-linux-for-i-mx-applications-processors:IMXLINUX>`__

.. _i.MX8ULP Applications Processors: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/i-mx-applications-processors/i-mx-8-applications-processors/i-mx-8ulp-applications-processor-family:i.MX8ULP

