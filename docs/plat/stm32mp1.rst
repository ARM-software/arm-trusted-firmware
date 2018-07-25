Trusted Firmware-A for STM32MP1
===============================

STM32MP1 is a microprocessor designed by STMicroelectronics
based on a dual Arm Cortex-A7.
It is an Armv7-A platform, using dedicated code from TF-A.


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


Build Instructions
------------------

To build:

.. code:: bash

    make CROSS_COMPILE=arm-linux-gnueabihf- PLAT=stm32mp1 ARCH=aarch32 ARM_ARCH_MAJOR=7 AARCH32_SP=sp_min

The following build options are supported:

- ``ENABLE_STACK_PROTECTOR``: To enable the stack protection.
