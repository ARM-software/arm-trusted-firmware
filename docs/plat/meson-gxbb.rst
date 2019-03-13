Amlogic Meson S905 (GXBB)
=========================

The Amlogic Meson S905 is a SoC with a quad core Arm Cortex-A53 running at
1.5Ghz. It also contains a Cortex-M3 used as SCP.

This port is a minimal implementation of BL31 capable of booting mainline U-Boot
and Linux:

- SCPI support.
- Basic PSCI support (CPU_ON, CPU_OFF, SYSTEM_RESET, SYSTEM_OFF). Note that CPU0
  can't be turned off, so there is a workaround to hide this from the caller.
- GICv2 driver set up.
- Basic SIP services (read efuse data, enable/disable JTAG).

In order to build it:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu- make DEBUG=1 PLAT=gxbb bl31

This port has been tested in a ODROID-C2. After building it, follow the
instructions in the `U-Boot repository`_, replacing the mentioned **bl31.bin**
by the one built from this port.

.. _U-Boot repository: https://github.com/u-boot/u-boot/blob/master/board/amlogic/odroid-c2/README.odroid-c2
