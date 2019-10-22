Platform Ports
==============

.. toctree::
   :maxdepth: 1
   :caption: Contents
   :numbered:
   :hidden:

   allwinner
   meson-gxbb
   meson-gxl
   meson-g12a
   fvp_ve
   hikey
   hikey960
   intel-agilex
   intel-stratix10
   marvell/index
   mt8183
   nvidia-tegra
   warp7
   imx8
   imx8m
   ls1043a
   poplar
   qemu
   qemu-sbsa
   rpi3
   rpi4
   rcar-gen3
   rockchip
   socionext-uniphier
   synquacer
   stm32mp1
   ti-k3
   xilinx-versal
   xilinx-zynqmp

This section provides a list of supported upstream *platform ports* and the
documentation associated with them. The list of suported Arm |FVP| platforms is
outlined in the following section.

.. note::
   In addition to the platforms ports listed within the table of contents, there
   are several additional platforms that are supported upstream but which do not
   currently have associated documentation:

   - Arm Juno Software Development Platform. Various |AArch32| and |AArch64|
     builds of this release have been tested on r0, r1 and r2 variants of the
     `Juno Arm Development Platform`_.
   - Arm Neoverse N1 System Development Platform (N1SDP)
   - Arm Neoverse Reference Design N1 Edge (RD-N1-Edge) FVP
   - Arm Neoverse Reference Design E1 Edge (RD-E1-Edge) FVP
   - Arm SGI-575 and SGM-775
   - MediaTek MT6795 and MT8173 SoCs

Fixed Virtual Platform (FVP) Support
------------------------------------

The latest version of the AArch64 build of TF-A has been tested on the
following Arm FVPs without shifted affinities, and that do not support threaded
CPU cores (64-bit host machine only).

.. note::
   The FVP models used are Version 11.6 Build 45, unless otherwise stated.

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_AEMv8A-AEMv8A-AEMv8A-AEMv8A-CCN502``
-  ``FVP_Base_RevC-2xAEMv8A``
-  ``FVP_Base_Cortex-A32x4``
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A55x4+Cortex-A75x4``
-  ``FVP_Base_Cortex-A55x4``
-  ``FVP_Base_Cortex-A57x1-A53x1``
-  ``FVP_Base_Cortex-A57x2-A53x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``
-  ``FVP_Base_Cortex-A75x4``
-  ``FVP_Base_Cortex-A76x4``
-  ``FVP_Base_Cortex-A76AEx4`` (Tested with internal model)
-  ``FVP_Base_Cortex-A76AEx8`` (Tested with internal model)
-  ``FVP_Base_Cortex-A77x4`` (Version 11.7 build 36)
-  ``FVP_Base_Neoverse-N1x4`` (Tested with internal model)
-  ``FVP_CSS_SGI-575`` (Version 11.3 build 42)
-  ``FVP_CSS_SGM-775`` (Version 11.3 build 42)
-  ``FVP_RD_E1Edge`` (Version 11.3 build 42)
-  ``FVP_RD_N1Edge`` (Version 11.3 build 42)
-  ``Foundation_Platform``

The latest version of the AArch32 build of TF-A has been tested on the
following Arm FVPs without shifted affinities, and that do not support threaded
CPU cores (64-bit host machine only).

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_Cortex-A32x4``

.. note::
   The ``FVP_Base_RevC-2xAEMv8A`` FVP only supports shifted affinities.

The *Foundation* FVP can be downloaded free of charge. The *Base* FVPs can be
licensed from Arm. See the `Arm FVP website`_.

All the above platforms have been tested with `Linaro Release 19.06`_.

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*

.. _Juno Arm Development Platform: http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php
.. _Arm FVP website: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Linaro Release 19.06: http://releases.linaro.org/members/arm/platforms/19.06
