.. _build_options_arm_fvp_platform:

Arm FVP Platform Specific Build Options
---------------------------------------

-  ``FVP_CLUSTER_COUNT`` : Configures the cluster count to be used to
   build the topology tree within TF-A. By default TF-A is configured for dual
   cluster topology and this option can be used to override the default value.

-  ``FVP_INTERCONNECT_DRIVER``: Selects the interconnect driver to be built. The
   default interconnect driver depends on the value of ``FVP_CLUSTER_COUNT`` as
   explained in the options below:

   -  ``FVP_CCI`` : The CCI driver is selected. This is the default
      if 0 < ``FVP_CLUSTER_COUNT`` <= 2.
   -  ``FVP_CCN`` : The CCN driver is selected. This is the default
      if ``FVP_CLUSTER_COUNT`` > 2.

-  ``FVP_MAX_CPUS_PER_CLUSTER``: Sets the maximum number of CPUs implemented in
   a single cluster.  This option defaults to 4.

-  ``FVP_MAX_PE_PER_CPU``: Sets the maximum number of PEs implemented on any CPU
   in the system. This option defaults to 1. Note that the build option
   ``ARM_PLAT_MT`` doesn't have any effect on FVP platforms.

-  ``FVP_USE_GIC_DRIVER`` : Selects the GIC driver to be built. Options:

   -  ``FVP_GICV2`` : The GICv2 only driver is selected
   -  ``FVP_GICV3`` : The GICv3 only driver is selected (default option)
   -  ``FVP_GICV5`` : The GICv5 only driver is selected

-  ``FVP_HW_CONFIG_DTS`` : Specify the path to the DTS file to be compiled
   to DTB and packaged in FIP as the HW_CONFIG. See :ref:`Firmware Design` for
   details on HW_CONFIG. By default, this is initialized to a sensible DTS
   file in ``fdts/`` folder depending on other build options. But some cases,
   like shifted affinity format for MPIDR, cannot be detected at build time
   and this option is needed to specify the appropriate DTS file.

-  ``FVP_HW_CONFIG`` : Specify the path to the HW_CONFIG blob to be packaged in
   FIP. See :ref:`Firmware Design` for details on HW_CONFIG. This option is
   similar to the ``FVP_HW_CONFIG_DTS`` option, but it directly specifies the
   HW_CONFIG blob instead of the DTS file. This option is useful to override
   the default HW_CONFIG selected by the build system.

-  ``FVP_GICR_REGION_PROTECTION``: Mark the redistributor pages of
   inactive/fused CPU cores as read-only. The default value of this option
   is ``0``, which means the redistributor pages of all CPU cores are marked
   as read and write.

-  ``INITRD_SIZE`` : Enable the insertion of initrd properties to the device
   tree blob at build time. Takes an initrd size value in hex format

-  ``INITRD_PATH`` : Enable the insertion of initrd properties to the device tree
   blob at build time. Takes a path to an initrd file. Can be used as an
   alternative to ``INITRD_SIZE``. ``INITRD_SIZE`` takes precedence over
   ``INITRD_PATH`` if both values are provided.

-  ``INITRD_BASE`` : Provide the preloaded initrd base address in memory (hex format).

--------------

*Copyright (c) 2019-2025, Arm Limited. All rights reserved.*
