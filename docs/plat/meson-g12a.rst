Amlogic Meson S905X2 (G12A)
===========================

The Amlogic Meson S905X2 is a SoC with a quad core Arm Cortex-A53 running at
~1.8GHz. It also contains a Cortex-M3 used as SCP.

This port is a minimal implementation of BL31 capable of booting mainline U-Boot
and Linux:

- SCPI support.
- Basic PSCI support (CPU_ON, CPU_OFF, SYSTEM_RESET, SYSTEM_OFF). Note that CPU0
  can't be turned off, so there is a workaround to hide this from the caller.
- GICv2 driver set up.
- Basic SIP services (read efuse data, enable/disable JTAG).

In order to build it:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make DEBUG=1 PLAT=g12a

This port has been tested on a SEI510 board. After building it, follow the
instructions in the `gxlimg repository` or `U-Boot repository`_, replacing the
mentioned **bl31.img** by the one built from this port.

.. _gxlimg repository: https://github.com/repk/gxlimg/blob/master/README.g12a
.. _U-Boot repository: https://github.com/u-boot/u-boot/blob/master/board/amlogic/sei510/README
