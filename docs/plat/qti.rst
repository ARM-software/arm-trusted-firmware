Qualcomm Technologies, Inc.
===========================

Trusted Firmware-A (TF-A) implements the EL3 firmware layer for QTI SC7180,
SC7280.

Boot Trace
-------------

Bootrom --> BL1/BL2 --> BL31 --> BL33 --> Linux kernel

BL1/2 and BL33 can currently be supplied from Coreboot + Depthcharge

How to build
------------

Code Locations
~~~~~~~~~~~~~~

-  Trusted Firmware-A:
   `link <https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git>`__

Build Procedure
~~~~~~~~~~~~~~~

QTI SoC expects TF-A's BL31 to get integrated with other boot software
Coreboot, so only bl31.elf need to get build from the TF-A repository.

The build command looks like

    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=sc7180 COREBOOT=1

update value of CROSS_COMPILE argument with your cross-compilation toolchain.

Additional QTISECLIB_PATH=<path to qtiseclib> can be added in build command.
if QTISECLIB_PATH is not added in build command stub implementation of qtiseclib
is picked. qtiseclib with stub implementation doesn't boot device. This was
added to satisfy compilation.

QTISELIB for SC7180 is available at
`link <https://github.com/coreboot/qc_blobs/blob/master/sc7180/qtiseclib/libqtisec.a?raw=true>`__
QTISELIB for SC7280 is available at
`link <https://github.com/coreboot/qc_blobs/blob/master/sc7280/qtiseclib/libqtisec.a?raw=true>`__
