Texas Instruments K3low
========================

Trusted Firmware-A (TF-A) implements the EL3 firmware layer for Texas Instruments K3low
SoCs. The first generation of K3 architetcure based devices used a Cortex R5 processor as a
primary boot loader. But the K3low SoC architecture removed the R5 processor and the
MPU A53 acts as the primary bootloader. Due to the small static memory foot print on the
k3low devices, a two stage boot load architecture is used. In the first stage BL1 boot
loader configures and initializes the DDR4/LPDDR4 subsystem and resets back to the
ROMCode for second stage booting of BL31 etc.

Boot Flow
---------

::

   TF-A BL1 --> TF-A BL31 --> BL32(OP-TEE) --> TF-A BL31 --> BL33(U-Boot) --> Linux
                                                       \
                                                   Optional direct to Linux boot
                                                           \
                                                           --> BL33(Linux)

Build Instructions
------------------

For TF-A bl1 and bl31 builds clone https://github.com/ARM-software/arm-trusted-firmware.git

TF-A:

.. code:: shell

    make CROSS_COMPILE=aarch64-none-linux-gnu- ARCH=aarch64 PLAT=k3low TARGET_BOARD=am62lx

OP-TEE:

.. code:: shell

    make ARCH=arm CROSS_COMPILE64=aarch64-none-linux-gnu- PLATFORM=k3 CFG_ARM64_core=y all

A53 U-Boot:

.. code:: shell

    make ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- am62l_evm_defconfig
    make ARCH=arm CROSS_COMPILE=aarch64-none-linux-gnu- BL1=<path> BL31=<path> TEE=<path> BINMAN_INDIRS=<path>

Deploy Images
-------------

.. code:: shell

    cp tiboot3.bin tispl.bin u-boot.img /sdcard/boot/
