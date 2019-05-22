Texas Instruments K3
====================

Trusted Firmware-A (TF-A) implements the EL3 firmware layer for Texas Instruments K3 SoCs.

Boot Flow
---------

::

   R5(U-Boot) --> TF-A BL31 --> BL32(OP-TEE) --> TF-A BL31 --> BL33(U-Boot) --> Linux
                                                       \
                                                   Optional direct to Linux boot
                                                           \
                                                           --> BL33(Linux)

Texas Instruments K3 SoCs contain an R5 processor used as the boot master, it
loads the needed images for A53 startup, because of this we do not need BL1 or
BL2 TF-A stages.

Build Instructions
------------------

https://github.com/ARM-software/arm-trusted-firmware.git

TF-A:

.. code:: shell

    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=k3 SPD=opteed all

OP-TEE:

.. code:: shell

    make ARCH=arm CROSS_COMPILE64=aarch64-linux-gnu- PLATFORM=k3 CFG_ARM64_core=y all

R5 U-Boot:

.. code:: shell

    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- am65x_evm_r5_defconfig
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- SYSFW=<path to SYSFW>

A53 U-Boot:

.. code:: shell

    make ARCH=arm CROSS_COMPILE=aarch64-linux-gnu- am65x_evm_a53_defconfig
    make ARCH=arm CROSS_COMPILE=aarch64-linux-gnu- ATF=<path> TEE=<path>

Deploy Images
-------------

.. code:: shell

    cp tiboot3.bin tispl.bin u-boot.img /sdcard/boot/
