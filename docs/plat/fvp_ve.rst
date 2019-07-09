Arm Versatile Express
=====================

Versatile Express (VE) family development platform provides an ultra fast
environment for prototyping Armv7 System-on-Chip designs. VE Fixed Virtual
Platforms (FVP) are simulations of Versatile Express boards. The platform in
Trusted Firmware-A has been verified with Arm Cortex-A5 and Cortex-A7 VE FVP's.
This platform is tested on and only expected to work with single core models.

Boot Sequence
-------------

BL1 --> BL2 --> BL32(sp_min) --> BL33(u-boot) --> Linux kernel

How to build
------------

Code Locations
~~~~~~~~~~~~~~
-  `U-boot <https://git.linaro.org/landing-teams/working/arm/u-boot.git>`__

-  `Trusted Firmware-A <https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git>`__

Build Procedure
~~~~~~~~~~~~~~~

-  Obtain arm toolchain. The software stack has been verified with linaro 6.2
   `arm-linux-gnueabihf <https://releases.linaro.org/components/toolchain/binaries/6.2-2016.11/arm-linux-gnueabihf/>`__.
   Set the CROSS_COMPILE environment variable to point to the toolchain folder.

-  Fetch and build u-boot.
   Make the .config file using the command:

   .. code:: shell

      make ARCH=arm vexpress_aemv8a_aarch32_config

   Make the u-boot binary for Cortex-A5 using the command:

   .. code:: shell

     make ARCH=arm SUPPORT_ARCH_TIMER=no

   Make the u-boot binary for Cortex-A7 using the command:

   .. code:: shell

     make ARCH=arm


-  Build TF-A:

   The make command for Cortex-A5 is:

   .. code:: shell

       make PLAT=fvp_ve ARCH=aarch32 ARM_ARCH_MAJOR=7 ARM_CORTEX_A5=yes \
       AARCH32_SP=sp_min FVP_HW_CONFIG_DTS=fdts/fvp-ve-Cortex-A5x1.dts \
       ARM_XLAT_TABLES_LIB_V1=1 BL33=<path_to_u-boot.bin> all fip

   The make command for Cortex-A7 is:

   .. code:: shell

      make PLAT=fvp_ve ARCH=aarch32 ARM_ARCH_MAJOR=7 ARM_CORTEX_A7=yes \
      AARCH32_SP=sp_min FVP_HW_CONFIG_DTS=fdts/fvp-ve-Cortex-A7x1.dts  \
      BL33=<path_to_u-boot.bin> all fip

Run Procedure
~~~~~~~~~~~~~

The following model parameters should be used to boot Linux using the build of
Trusted Firmware-A made using the above make commands:

  .. code:: shell

    ./<path_to_model> <path_to_bl1.elf> \
          -C motherboard.flashloader1.fname=<path_to_fip.bin> \
          --data cluster.cpu0=<path_to_zImage>@0x80080000  \
          --data cluster.cpu0=<path_to_ramdisk>@0x84000000
