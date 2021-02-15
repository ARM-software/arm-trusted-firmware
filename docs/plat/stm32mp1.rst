STMicroelectronics STM32MP1
===========================

STM32MP1 is a microprocessor designed by STMicroelectronics
based on a dual Arm Cortex-A7.
It is an Armv7-A platform, using dedicated code from TF-A.
The STM32MP1 chip also embeds a Cortex-M4.
More information can be found on `STM32MP1 Series`_ page.


STM32MP1 Versions
-----------------
The STM32MP1 series is available in 3 different lines which are pin-to-pin compatible:

- STM32MP157: Dual Cortex-A7 cores, Cortex-M4 core @ 209 MHz, 3D GPU, DSI display interface and CAN FD
- STM32MP153: Dual Cortex-A7 cores, Cortex-M4 core @ 209 MHz and CAN FD
- STM32MP151: Single Cortex-A7 core, Cortex-M4 core @ 209 MHz

Each line comes with a security option (cryptography & secure boot) and a Cortex-A frequency option:

- A      Basic + Cortex-A7 @ 650 MHz
- C      Secure Boot + HW Crypto + Cortex-A7 @ 650 MHz
- D      Basic + Cortex-A7 @ 800 MHz
- F      Secure Boot + HW Crypto + Cortex-A7 @ 800 MHz

The `STM32MP1 part number codification`_ page gives more information about part numbers.

Design
------
The STM32MP1 resets in the ROM code of the Cortex-A7.
The primary boot core (core 0) executes the boot sequence while
secondary boot core (core 1) is kept in a holding pen loop.
The ROM code boot sequence loads the TF-A binary image from boot device
to embedded SRAM.

The TF-A image must be properly formatted with a STM32 header structure
for ROM code is able to load this image.
Tool stm32image can be used to prepend this header to the generated TF-A binary.

At compilation step, BL2, BL32 and DTB file are linked together in a single
binary. The stm32image tool is also generated and the header is added to TF-A
binary. This binary file with header is named tf-a-stm32mp157c-ev1.stm32.
It can then be copied in the first partition of the boot device.


Memory mapping
~~~~~~~~~~~~~~

::

    0x00000000 +-----------------+
               |                 |   ROM
    0x00020000 +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x2FFC0000 +-----------------+ \
               |                 | |
               |       ...       | |
               |                 | |
    0x2FFD8000 +-----------------+ |
               |    TF-A DTB     | | Embedded SRAM
    0x2FFDC000 +-----------------+ |
               |       BL2       | |
    0x2FFEF000 +-----------------+ |
               |       BL32      | |
    0x30000000 +-----------------+ /
               |                 |
               |       ...       |
               |                 |
    0x40000000 +-----------------+
               |                 |
               |                 |   Devices
               |                 |
    0xC0000000 +-----------------+ \
               |                 | |
    0xC0100000 +-----------------+ |
               |       BL33      | | Non-secure RAM (DDR)
               |       ...       | |
               |                 | |
    0xFFFFFFFF +-----------------+ /


Boot sequence
~~~~~~~~~~~~~

ROM code -> BL2 (compiled with BL2_AT_EL3) -> BL32 (SP_min) -> BL33 (U-Boot)

or if Op-TEE is used:

ROM code -> BL2 (compiled with BL2_AT_EL3) -> OP-TEE -> BL33 (U-Boot)


Build Instructions
------------------
Boot media(s) supported by BL2 must be specified in the build command.
Available storage medias are:

- ``STM32MP_SDMMC``
- ``STM32MP_EMMC``
- ``STM32MP_RAW_NAND``
- ``STM32MP_SPI_NAND``
- ``STM32MP_SPI_NOR``

To build with SP_min and support for all bootable devices:

.. code:: bash

    make CROSS_COMPILE=arm-linux-gnueabihf- PLAT=stm32mp1 ARCH=aarch32 ARM_ARCH_MAJOR=7 AARCH32_SP=sp_min STM32MP_SDMMC=1 STM32MP_EMMC=1 STM32MP_RAW_NAND=1 STM32MP_SPI_NAND=1
    STM32MP_SPI_NOR=1 DTB_FILE_NAME=stm32mp157c-ev1.dtb
    cd <u-boot_directory>
    make stm32mp15_trusted_defconfig
    make DEVICE_TREE=stm32mp157c-ev1 all

To build TF-A with OP-TEE support for all bootable devices:

.. code:: bash

    make CROSS_COMPILE=arm-linux-gnueabihf- PLAT=stm32mp1 ARCH=aarch32 ARM_ARCH_MAJOR=7 AARCH32_SP=optee STM32MP_SDMMC=1 STM32MP_EMMC=1 STM32MP_RAW_NAND=1 STM32MP_SPI_NAND=1 STM32MP_SPI_NOR=1 DTB_FILE_NAME=stm32mp157c-ev1.dtb
    cd <optee_directory>
    make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm PLATFORM=stm32mp1 CFG_EMBED_DTB_SOURCE_FILE=stm32mp157c-ev1.dts
    cd <u-boot_directory>
    make stm32mp15_trusted_defconfig
    make DEVICE_TREE=stm32mp157c-ev1 all


The following build options are supported:

- ``ENABLE_STACK_PROTECTOR``: To enable the stack protection.


Populate SD-card
----------------

The SD-card has to be formated with GPT.
It should contain at least those partitions:

- fsbl: to copy the tf-a-stm32mp157c-ev1.stm32 binary
- ssbl: to copy the u-boot.stm32 binary

Usually, two copies of fsbl are used (fsbl1 and fsbl2) instead of one partition fsbl.

OP-TEE artifacts go into separate partitions as follows:

- teeh: tee-header_v2.stm32
- teed: tee-pageable_v2.stm32
- teex: tee-pager_v2.stm32


.. _STM32MP1 Series: https://www.st.com/en/microcontrollers-microprocessors/stm32mp1-series.html
.. _STM32MP1 part number codification: https://wiki.st.com/stm32mpu/wiki/STM32MP15_microprocessor#Part_number_codification
