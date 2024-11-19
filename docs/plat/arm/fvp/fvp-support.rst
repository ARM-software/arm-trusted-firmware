Fixed Virtual Platform (FVP) Support
------------------------------------

This section lists the supported Arm |FVP| platforms. Please refer to the FVP
documentation for a detailed description of the model parameter options.

The latest version of the AArch64 build of TF-A has been tested on the following
Arm FVPs without shifted affinities, and that do not support threaded CPU cores
(64-bit host machine only).

.. note::
   The FVP models used are Version 11.26 Build 11, unless otherwise stated.

-  ``FVP_Base_AEMvA-AEMvA``
-  ``FVP_Base_RevC-2xAEMvA``
-  ``FVP_Base_Cortex-A32x4``
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A55``
-  ``FVP_Base_Cortex-A57x1-A53x1``
-  ``FVP_Base_Cortex-A57x2-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A65`` (Version 11.24/24)
-  ``FVP_Base_Cortex-A65AE`` (Version 11.24/24)
-  ``FVP_Base_Cortex-A710``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A75``
-  ``FVP_Base_Cortex-A76``
-  ``FVP_Base_Cortex-A76AE``
-  ``FVP_Base_Cortex-A77``
-  ``FVP_Base_Cortex-A78``
-  ``FVP_Base_Cortex-A78AE``
-  ``FVP_Base_Cortex-A78C``
-  ``FVP_Base_Cortex-X2``
-  ``FVP_Base_Neoverse-E1`` (Version 11.24/24)
-  ``FVP_Base_Neoverse-N1``
-  ``FVP_Base_Neoverse-N2``
-  ``FVP_Base_Neoverse-V1``
-  ``FVP_BaseR_AEMv8R``
-  ``FVP_Morello`` (Version 0.11/33)
-  ``FVP_RD_V1``
-  ``FVP_RD_1_AE`` (Version 11.27/20)
-  ``FVP_TC3`` (Version 11.26/16)
-  ``FVP_TC4`` (Version 0.0/8404)

The latest version of the AArch32 build of TF-A has been tested on the
following Arm FVPs without shifted affinities, and that do not support threaded
CPU cores (64-bit host machine only).

-  ``FVP_Base_AEMvA``
-  ``FVP_Base_AEMvA-AEMvA``
-  ``FVP_Base_Cortex-A32x4``

.. note::
   The ``FVP_Base_RevC-2xAEMv8A`` FVP only supports shifted affinities, which
   is not compatible with legacy GIC configurations. Therefore this FVP does not
   support these legacy GIC configurations.

The *Foundation* and *Base* FVPs can be downloaded free of charge. See the `Arm
FVP website`_. The Cortex-A models listed above are also available to download
from `Arm's website`_.

.. note::
   The build numbers quoted above are those reported by launching the FVP
   with the ``--version`` parameter.

.. note::
   Linaro provides a ramdisk image in prebuilt FVP configurations and full
   file systems that can be downloaded separately. To run an FVP with a virtio
   file system image an additional FVP configuration option
   ``-C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>`` can be
   used.

.. note::
   The software will not work on Version 1.0 of the Foundation FVP.
   The commands below would report an ``unhandled argument`` error in this case.

.. note::
   FVPs can be launched with ``--cadi-server`` option such that a
   CADI-compliant debugger (for example, Arm DS-5) can connect to and control
   its execution.

.. warning::
   Since FVP model Version 11.0 Build 11.0.34 and Version 8.5 Build 0.8.5202
   the internal synchronisation timings changed compared to older versions of
   the models. The models can be launched with ``-Q 100`` option if they are
   required to match the run time characteristics of the older versions.

All the above platforms have been tested with `Linaro Release 20.01`_.

--------------

*Copyright (c) 2019-2024, Arm Limited. All rights reserved.*

.. _Arm's website: `FVP models`_
.. _FVP models: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Linaro Release 20.01: http://releases.linaro.org/members/arm/platforms/20.01
.. _Arm FVP website: https://developer.arm.com/products/system-design/fixed-virtual-platforms
