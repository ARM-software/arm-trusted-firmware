Rockchip SoCs
=============

Trusted Firmware-A supports a number of Rockchip ARM SoCs from both
AARCH32 and AARCH64 fields.

This includes right now:
-  rk3288: Quad-Core Cortex-A17 (past A12)
-  rk3328: Quad-Core Cortex-A53
-  rk3368: Octa-Core Cortex-A53
-  rk3399: Hexa-Core Cortex-A53/A72


Boot Sequence
-------------

For AARCH32:
    Bootrom --> BL1/BL2 --> BL32 --> BL33 --> Linux kernel

For AARCH64:
    Bootrom --> BL1/BL2 --> BL31 --> BL33 --> Linux kernel

BL1/2 and BL33 can currently be supplied from either:
-  Coreboot + Depthcharge
-  U-Boot - either separately as TPL+SPL or only SPL


How to build
------------

Rockchip SoCs expect TF-A's BL31 (AARCH64) or BL32 (AARCH32) to get
integrated with other boot software like U-Boot or Coreboot, so only
these images need to get build from the TF-A repository.

For AARCH64 architectures the build command looks like

    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=rk3399 bl32

while AARCH32 needs a slightly different command

    make ARCH=aarch32 CROSS_COMPILE=arm-linux-gnueabihf- PLAT=rk3288 AARCH32_SP=sp_min bl32

Both need replacing the PLAT argument with the platform from above you
want to build for and the CROSS_COMPILE argument with you cross-
compilation toolchain.


How to deploy
-------------

Both upstream U-Boot and Coreboot projects contain instructions on where
to put the built images during their respective build process.
So after successfully building TF-A just follow their build instructions
to continue.
