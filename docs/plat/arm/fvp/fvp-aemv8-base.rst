Running on the AEMv8 Base FVP
=============================

AArch64 with reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_RevC-2xAEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_RevC-2xAEMv8A                            \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cluster0.NUM_CORES=4                                     \
    -C cluster1.NUM_CORES=4                                     \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

.. note::
   The ``FVP_Base_RevC-2xAEMv8A`` has shifted affinities and requires
   a specific DTS for all the CPUs to be loaded.

AArch32 with reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                            \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cluster0.NUM_CORES=4                                     \
    -C cluster1.NUM_CORES=4                                     \
    -C cache_state_modelled=1                                   \
    -C cluster0.cpu0.CONFIG64=0                                 \
    -C cluster0.cpu1.CONFIG64=0                                 \
    -C cluster0.cpu2.CONFIG64=0                                 \
    -C cluster0.cpu3.CONFIG64=0                                 \
    -C cluster1.cpu0.CONFIG64=0                                 \
    -C cluster1.cpu1.CONFIG64=0                                 \
    -C cluster1.cpu2.CONFIG64=0                                 \
    -C cluster1.cpu3.CONFIG64=0                                 \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

AArch64 with reset to BL31 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_RevC-2xAEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_RevC-2xAEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBAR=0x04010000                            \
    -C cluster0.cpu1.RVBAR=0x04010000                            \
    -C cluster0.cpu2.RVBAR=0x04010000                            \
    -C cluster0.cpu3.RVBAR=0x04010000                            \
    -C cluster1.cpu0.RVBAR=0x04010000                            \
    -C cluster1.cpu1.RVBAR=0x04010000                            \
    -C cluster1.cpu2.RVBAR=0x04010000                            \
    -C cluster1.cpu3.RVBAR=0x04010000                            \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04010000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0xff000000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Notes:

-  Position Independent Executable (PIE) support is enabled in this
   config allowing BL31 to be loaded at any valid address for execution.

-  Since a FIP is not loaded when using BL31 as reset entrypoint, the
   ``--data="<path-to><bl31|bl32|bl33-binary>"@<base-address-of-binary>``
   parameter is needed to load the individual bootloader images in memory.
   BL32 image is only needed if BL31 has been built to expect a Secure-EL1
   Payload. For the same reason, the FDT needs to be compiled from the DT source
   and loaded via the ``--data cluster0.cpu0="<path-to>/<fdt>"@0x82000000``
   parameter.

-  The ``FVP_Base_RevC-2xAEMv8A`` has shifted affinities and requires a
   specific DTS for all the CPUs to be loaded.

-  The ``-C cluster<X>.cpu<Y>.RVBAR=@<base-address-of-bl31>`` parameter, where
   X and Y are the cluster and CPU numbers respectively, is used to set the
   reset vector for each core.

-  Changing the default value of ``ARM_TSP_RAM_LOCATION`` will also require
   changing the value of
   ``--data="<path-to><bl32-binary>"@<base-address-of-bl32>`` to the new value of
   ``BL32_BASE``.

AArch32 with reset to SP_MIN entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.CONFIG64=0                                  \
    -C cluster0.cpu1.CONFIG64=0                                  \
    -C cluster0.cpu2.CONFIG64=0                                  \
    -C cluster0.cpu3.CONFIG64=0                                  \
    -C cluster1.cpu0.CONFIG64=0                                  \
    -C cluster1.cpu1.CONFIG64=0                                  \
    -C cluster1.cpu2.CONFIG64=0                                  \
    -C cluster1.cpu3.CONFIG64=0                                  \
    -C cluster0.cpu0.RVBAR=0x04002000                            \
    -C cluster0.cpu1.RVBAR=0x04002000                            \
    -C cluster0.cpu2.RVBAR=0x04002000                            \
    -C cluster0.cpu3.RVBAR=0x04002000                            \
    -C cluster1.cpu0.RVBAR=0x04002000                            \
    -C cluster1.cpu1.RVBAR=0x04002000                            \
    -C cluster1.cpu2.RVBAR=0x04002000                            \
    -C cluster1.cpu3.RVBAR=0x04002000                            \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04002000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

.. note::
   Position Independent Executable (PIE) support is enabled in this
   config allowing SP_MIN to be loaded at any valid address for execution.

--------------

*Copyright (c) 2019-2024, Arm Limited. All rights reserved.*
