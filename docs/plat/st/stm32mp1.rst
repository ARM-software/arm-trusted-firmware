STM32MP1
========

STM32MP1 is a microprocessor designed by STMicroelectronics
based on Arm Cortex-A7.
It is an Armv7-A platform, using dedicated code from TF-A.
More information can be found on `STM32MP1 Series`_ page.

For TF-A common configuration of STM32 MPUs, please check
:ref:`STM32 MPUs` page.

STM32MP1 Versions
-----------------

There are 2 variants for STM32MP1: STM32MP13 and STM32MP15

STM32MP13 Versions
~~~~~~~~~~~~~~~~~~
The STM32MP13 series is available in 3 different lines which are pin-to-pin compatible:

- STM32MP131: Single Cortex-A7 core
- STM32MP133: STM32MP131 + 2*CAN, ETH2(GMAC), ADC1
- STM32MP135: STM32MP133 + DCMIPP, LTDC

Each line comes with a security option (cryptography & secure boot) and a Cortex-A frequency option:

- A      Cortex-A7 @ 650 MHz
- C      Secure Boot + HW Crypto + Cortex-A7 @ 650 MHz
- D      Cortex-A7 @ 900 MHz
- F      Secure Boot + HW Crypto + Cortex-A7 @ 900 MHz

STM32MP15 Versions
~~~~~~~~~~~~~~~~~~
The STM32MP15 series is available in 3 different lines which are pin-to-pin compatible:

- STM32MP157: Dual Cortex-A7 cores, Cortex-M4 core @ 209 MHz, 3D GPU, DSI display interface and CAN FD
- STM32MP153: Dual Cortex-A7 cores, Cortex-M4 core @ 209 MHz and CAN FD
- STM32MP151: Single Cortex-A7 core, Cortex-M4 core @ 209 MHz

Each line comes with a security option (cryptography & secure boot) and a Cortex-A frequency option:

- A      Basic + Cortex-A7 @ 650 MHz
- C      Secure Boot + HW Crypto + Cortex-A7 @ 650 MHz
- D      Basic + Cortex-A7 @ 800 MHz
- F      Secure Boot + HW Crypto + Cortex-A7 @ 800 MHz

The `STM32MP1 part number codification`_ page gives more information about part numbers.

Memory mapping
--------------

::

    0x00000000 +-----------------+
               |                 |   ROM
    0x00020000 +-----------------+
               |                 |
               |       ...       |
               |                 |
    0x2FFC0000 +-----------------+ \
               |     BL32 DTB    | |
    0x2FFC5000 +-----------------+ |
               |       BL32      | |
    0x2FFDF000 +-----------------+ |
               |       ...       | |
    0x2FFE3000 +-----------------+ |
               |     BL2 DTB     | | Embedded SRAM
    0x2FFEA000 +-----------------+ |
               |       BL2       | |
    0x2FFFF000 +-----------------+ |
               |  SCMI mailbox   | |
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


Build Instructions
------------------

STM32MP1x specific flags
~~~~~~~~~~~~~~~~~~~~~~~~

Dedicated STM32MP1 flags:

- | ``STM32_TF_VERSION``: to manage BL2 monotonic counter.
  | Default: 0
- | ``STM32MP13``: to select STM32MP13 variant configuration.
  | Default: 0
- | ``STM32MP15``: to select STM32MP15 variant configuration.
  | Default: 1


Boot with FIP
~~~~~~~~~~~~~
You need to build BL2, BL32 (SP_min or OP-TEE) and BL33 (U-Boot) before building FIP binary.

U-Boot
______

.. code:: bash

    cd <u-boot_directory>
    make stm32mp15_trusted_defconfig
    make DEVICE_TREE=stm32mp157c-ev1 all

OP-TEE (recommended)
____________________
OP-TEE is the default BL32 supported for STMicroelectronics platforms.

.. code:: bash

    cd <optee_directory>
    make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm PLATFORM=stm32mp1 \
        CFG_EMBED_DTB_SOURCE_FILE=stm32mp157c-ev1.dts


TF-A BL32 (SP_min) (not recommended)
____________________________________
If you choose not to use OP-TEE, you can use TF-A SP_min.
This is not the recommended BL32 to use, and will have very limited support.
To build TF-A BL32, and its device tree file:

.. code:: bash

    make CROSS_COMPILE=arm-none-eabi- PLAT=stm32mp1 ARCH=aarch32 ARM_ARCH_MAJOR=7 \
        AARCH32_SP=sp_min DTB_FILE_NAME=stm32mp157c-ev1.dtb bl32 dtbs

TF-A BL2
________
To build TF-A BL2 with its STM32 header for SD-card boot:

.. code:: bash

    make CROSS_COMPILE=arm-none-eabi- PLAT=stm32mp1 ARCH=aarch32 ARM_ARCH_MAJOR=7 \
        DTB_FILE_NAME=stm32mp157c-ev1.dtb STM32MP_SDMMC=1

For other boot devices, you have to replace STM32MP_SDMMC in the previous command
with the desired device flag.

This BL2 is independent of the BL32 used (SP_min or OP-TEE)


FIP
___
With BL32 SP_min:

.. code:: bash

    make CROSS_COMPILE=arm-none-eabi- PLAT=stm32mp1 ARCH=aarch32 ARM_ARCH_MAJOR=7 \
        AARCH32_SP=sp_min \
        DTB_FILE_NAME=stm32mp157c-ev1.dtb \
        BL33=<u-boot_directory>/u-boot-nodtb.bin \
        BL33_CFG=<u-boot_directory>/u-boot.dtb \
        fip

With OP-TEE:

.. code:: bash

    make CROSS_COMPILE=arm-none-eabi- PLAT=stm32mp1 ARCH=aarch32 ARM_ARCH_MAJOR=7 \
        AARCH32_SP=optee \
        DTB_FILE_NAME=stm32mp157c-ev1.dtb \
        BL33=<u-boot_directory>/u-boot-nodtb.bin \
        BL33_CFG=<u-boot_directory>/u-boot.dtb \
        BL32=<optee_directory>/tee-header_v2.bin \
        BL32_EXTRA1=<optee_directory>/tee-pager_v2.bin
        BL32_EXTRA2=<optee_directory>/tee-pageable_v2.bin
        fip

Trusted Boot Board
__________________

.. code:: shell

    tools/cert_create/cert_create -n --rot-key build/stm32mp1/release/rot_key.pem \
        --tfw-nvctr 0 \
        --ntfw-nvctr 0 \
        --key-alg ecdsa --hash-alg sha256 \
        --trusted-key-cert build/stm32mp1/release/trusted_key.crt \
        --tos-fw <optee_directory>/tee-header_v2.bin \
        --tos-fw-extra1 <optee_directory>/tee-pager_v2.bin \
        --tos-fw-extra2 <optee_directory>/tee-pageable_v2.bin \
        --tos-fw-cert build/stm32mp1/release/tos_fw_content.crt \
        --tos-fw-key-cert build/stm32mp1/release/tos_fw_key.crt \
        --nt-fw <u-boot_directory>/u-boot-nodtb.bin \
        --nt-fw-cert build/stm32mp1/release/nt_fw_content.crt \
        --nt-fw-key-cert build/stm32mp1/release/nt_fw_key.crt \
        --hw-config <u-boot_directory>/u-boot.dtb \
        --fw-config build/stm32mp1/release/fdts/fw-config.dtb \
        --stm32mp-cfg-cert build/stm32mp1/release/stm32mp_cfg_cert.crt

    tools/fiptool/fiptool create --tos-fw <optee_directory>/tee-header_v2.bin \
        --tos-fw-extra1 <optee_directory>/tee-pager_v2.bin \
        --tos-fw-extra2 <optee_directory>/tee-pageable_v2.bin \
        --nt-fw <u-boot_directory>/u-boot-nodtb.bin \
        --hw-config <u-boot_directory>/u-boot.dtb \
        --fw-config build/stm32mp1/release/fdts/fw-config.dtb \
        --trusted-key-cert build/stm32mp1/release/trusted_key.crt \
        --tos-fw-cert build/stm32mp1/release/tos_fw_content.crt \
        --tos-fw-key-cert build/stm32mp1/release/tos_fw_key.crt \
        --nt-fw-cert build/stm32mp1/release/nt_fw_content.crt \
        --nt-fw-key-cert build/stm32mp1/release/nt_fw_key.crt \
        --stm32mp-cfg-cert build/stm32mp1/release/stm32mp_cfg_cert.crt \
        build/stm32mp1/release/stm32mp1.fip


.. _STM32MP1 Series: https://www.st.com/en/microcontrollers-microprocessors/stm32mp1-series.html
.. _STM32MP1 part number codification: https://wiki.st.com/stm32mpu/wiki/STM32MP15_microprocessor#Part_number_codification

*Copyright (c) 2023-2024, STMicroelectronics - All Rights Reserved*
