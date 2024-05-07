Running on the Cortex-A32 Base FVP (AArch32)
============================================

With reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_Cortex-A32x4`` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_Cortex-A32x4                             \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

With reset to SP_MIN entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_Cortex-A32x4`` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_Cortex-A32x4                             \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C cluster0.cpu0.RVBARADDR=0x04002000                       \
    -C cluster0.cpu1.RVBARADDR=0x04002000                       \
    -C cluster0.cpu2.RVBARADDR=0x04002000                       \
    -C cluster0.cpu3.RVBARADDR=0x04002000                       \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04002000   \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000   \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

--------------

*Copyright (c) 2019-2024, Arm Limited. All rights reserved.*
