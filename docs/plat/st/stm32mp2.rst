STM32MP2
========

STM32MP2 is a microprocessor designed by STMicroelectronics
based on Arm Cortex-A35.

For TF-A common configuration of STM32 MPUs, please check
:ref:`STM32 MPUs` page.

STM32MP2 Versions
-----------------

The STM32MP25 series is available in 4 different lines which are pin-to-pin compatible:

- STM32MP257: Dual Cortex-A35 cores, Cortex-M33 core - 3x Ethernet (2+1 switch) - 3x CAN FD – H264 - 3D GPU – AI / NN - LVDS
- STM32MP255: Dual Cortex-A35 cores, Cortex-M33 core - 2x Ethernet – 3x CAN FD - H264 - 3D GPU – AI / NN - LVDS
- STM32MP253: Dual Cortex-A35 cores, Cortex-M33 core - 2x Ethernet – 3x CAN FD - LVDS
- STM32MP251: Single Cortex-A35 core, Cortex-M33 core - 1x Ethernet

Each line comes with a security option (cryptography & secure boot) and a Cortex-A frequency option:

- A      Basic + Cortex-A35 @ 1GHz
- C      Secure Boot + HW Crypto + Cortex-A35 @ 1GHz
- D      Basic + Cortex-A35 @ 1.5GHz
- F      Secure Boot + HW Crypto + Cortex-A35 @ 1.5GHz

Memory mapping
--------------

::

    0x00000000 +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x0E000000 +-----------------+ \
               |       BL31      | |
               +-----------------+ |
               |       ...       | |
    0x0E012000 +-----------------+ |
               |     BL2 DTB     | | Embedded SRAM
    0x0E016000 +-----------------+ |
               |       BL2       | |
    0x0E040000 +-----------------+ /
               |                 |
               |       ...       |
               |                 |
    0x40000000 +-----------------+
               |                 |
               |                 |   Devices
               |                 |
    0x80000000 +-----------------+ \
               |                 | |
               |                 | | Non-secure RAM (DDR)
               |                 | |
    0xFFFFFFFF +-----------------+ /


Build Instructions
------------------

STM32MP2x specific flags
~~~~~~~~~~~~~~~~~~~~~~~~

Dedicated STM32MP2 build flags:

- | ``STM32MP_DDR_FIP_IO_STORAGE``: to store DDR firmware in FIP.
  | Default: 1
- | ``STM32MP25``: to select STM32MP25 variant configuration.
  | Default: 1

To compile the correct DDR driver, one flag must be set among:

- | ``STM32MP_DDR3_TYPE``: to compile DDR3 driver and DT.
  | Default: 0
- | ``STM32MP_DDR4_TYPE``: to compile DDR4 driver and DT.
  | Default: 0
- | ``STM32MP_LPDDR4_TYPE``: to compile LpDDR4 driver and DT.
  | Default: 0


Boot with FIP
~~~~~~~~~~~~~
You need to build BL2, BL31, BL32 (OP-TEE) and BL33 (U-Boot) before building FIP binary.

U-Boot
______

.. code:: bash

    cd <u-boot_directory>
    make stm32mp25_defconfig
    make DEVICE_TREE=stm32mp257f-ev1 all

OP-TEE
______

.. code:: bash

    cd <optee_directory>
    make CROSS_COMPILE64=aarch64-none-elf- CROSS_COMPILE32=arm-none-eabi-
        ARCH=arm PLATFORM=stm32mp2 \
        CFG_EMBED_DTB_SOURCE_FILE=stm32mp257f-ev1.dts

TF-A BL2 & BL31
_______________
To build TF-A BL2 with its STM32 header and BL31 for SD-card boot:

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=stm32mp2 \
        STM32MP_DDR4_TYPE=1 SPD=opteed \
        DTB_FILE_NAME=stm32mp257f-ev1.dtb STM32MP_SDMMC=1

For other boot devices, you have to replace STM32MP_SDMMC in the previous command
with the desired device flag.


FIP
___

.. code:: bash

    make CROSS_COMPILE=aarch64-none-elf- PLAT=stm32mp2 \
        STM32MP_DDR4_TYPE=1 SPD=opteed \
        DTB_FILE_NAME=stm32mp257f-ev1.dtb \
        BL33=<u-boot_directory>/u-boot-nodtb.bin \
        BL33_CFG=<u-boot_directory>/u-boot.dtb \
        BL32=<optee_directory>/tee-header_v2.bin \
        BL32_EXTRA1=<optee_directory>/tee-pager_v2.bin
        fip

*Copyright (c) 2023, STMicroelectronics - All Rights Reserved*
