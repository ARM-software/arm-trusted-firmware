STMicroelectronics STM32MP1
===========================

STM32MP1 is a microprocessor designed by STMicroelectronics
based on Arm Cortex-A7.
It is an Armv7-A platform, using dedicated code from TF-A.
More information can be found on `STM32MP1 Series`_ page.


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

Boot with FIP
~~~~~~~~~~~~~
The use of FIP is now the recommended way to boot STM32MP1 platform.
Only BL2 (with STM32 header) is loaded by ROM code. The other binaries are
inside the FIP binary: BL32 (SP_min or OP-TEE), U-Boot and their respective
device tree blobs.


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

Serial boot devices:

- ``STM32MP_UART_PROGRAMMER``
- ``STM32MP_USB_PROGRAMMER``


Other configuration flags:

- | ``DTB_FILE_NAME``: to precise board device-tree blob to be used.
  | Default: stm32mp157c-ev1.dtb
- | ``DWL_BUFFER_BASE``: the 'serial boot' load address of FIP,
  | default location (end of the first 128MB) is used when absent
- | ``STM32MP_EARLY_CONSOLE``: to enable early traces before clock driver is setup.
  | Default: 0 (disabled)
- | ``STM32MP_RECONFIGURE_CONSOLE``: to re-configure crash console (especially after BL2).
  | Default: 0 (disabled)
- | ``STM32MP_UART_BAUDRATE``: to select UART baud rate.
  | Default: 115200
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

OP-TEE (optional)
_________________

.. code:: bash

    cd <optee_directory>
    make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm PLATFORM=stm32mp1 \
        CFG_EMBED_DTB_SOURCE_FILE=stm32mp157c-ev1.dts


TF-A BL32 (SP_min)
__________________
If you choose not to use OP-TEE, you can use TF-A SP_min.
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

    tools/cert_create/cert_create -n --rot-key "build/stm32mp1/debug/rot_key.pem" \
        --tfw-nvctr 0 \
        --ntfw-nvctr 0 \
        --key-alg ecdsa --hash-alg sha256 \
        --trusted-key-cert build/stm32mp1/cert_images/trusted-key-cert.key-crt \
        --tos-fw <optee_directory>/tee-header_v2.bin \
        --tos-fw-extra1 <optee_directory>/tee-pager_v2.bin \
        --tos-fw-extra2 <optee_directory>/tee-pageable_v2.bin \
        --tos-fw-cert build/stm32mp1/cert_images/tee-header_v2.bin.crt \
        --tos-fw-key-cert build/stm32mp1/cert_images/tee-header_v2.bin.key-crt \
        --nt-fw <u-boot_directory>/u-boot-nodtb.bin \
        --nt-fw-cert build/stm32mp1/cert_images/u-boot.bin.crt \
        --nt-fw-key-cert build/stm32mp1/cert_images/u-boot.bin.key-crt \
        --hw-config <u-boot_directory>/u-boot.dtb \
        --fw-config build/stm32mp1/debug/fdts/fw-config.dtb \
        --stm32mp-cfg-cert build/stm32mp1/cert_images/stm32mp_cfg_cert.crt

    tools/fiptool/fiptool create --tos-fw <optee_directory>/tee-header_v2.bin \
        --tos-fw-extra1 <optee_directory>/tee-pager_v2.bin \
        --tos-fw-extra2 <optee_directory>/tee-pageable_v2.bin \
        --nt-fw <u-boot_directory>/u-boot-nodtb.bin \
        --hw-config <u-boot_directory>/u-boot.dtb \
        --fw-config build/stm32mp1/debug/fdts/fw-config.dtb \
        --tos-fw-cert build/stm32mp1/cert_images/tee-header_v2.bin.crt \
        --tos-fw-key-cert build/stm32mp1/cert_images/tee-header_v2.bin.key-crt \
        --nt-fw-cert build/stm32mp1/cert_images/u-boot.bin.crt \
        --nt-fw-key-cert build/stm32mp1/cert_images/u-boot.bin.key-crt \
        --stm32mp-cfg-cert build/stm32mp1/cert_images/stm32mp_cfg_cert.crt stm32mp1.fip



Populate SD-card
----------------

Boot with FIP
~~~~~~~~~~~~~
The SD-card has to be formatted with GPT.
It should contain at least those partitions:

- fsbl: to copy the tf-a-stm32mp157c-ev1.stm32 binary (BL2)
- fip: which contains the FIP binary

Usually, two copies of fsbl are used (fsbl1 and fsbl2) instead of one partition fsbl.


.. _STM32MP1 Series: https://www.st.com/en/microcontrollers-microprocessors/stm32mp1-series.html
.. _STM32MP1 part number codification: https://wiki.st.com/stm32mpu/wiki/STM32MP15_microprocessor#Part_number_codification
