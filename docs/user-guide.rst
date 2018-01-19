ARM Trusted Firmware User Guide
===============================


.. section-numbering::
    :suffix: .

.. contents::

This document describes how to build ARM Trusted Firmware (TF) and run it with a
tested set of other software components using defined configurations on the Juno
ARM development platform and ARM Fixed Virtual Platform (FVP) models. It is
possible to use other software components, configurations and platforms but that
is outside the scope of this document.

This document assumes that the reader has previous experience running a fully
bootable Linux software stack on Juno or FVP using the prebuilt binaries and
filesystems provided by `Linaro`_. Further information may be found in the
`Linaro instructions`_. It also assumes that the user understands the role of
the different software components required to boot a Linux system:

-  Specific firmware images required by the platform (e.g. SCP firmware on Juno)
-  Normal world bootloader (e.g. UEFI or U-Boot)
-  Device tree
-  Linux kernel image
-  Root filesystem

This document also assumes that the user is familiar with the `FVP models`_ and
the different command line options available to launch the model.

This document should be used in conjunction with the `Firmware Design`_.

Host machine requirements
-------------------------

The minimum recommended machine specification for building the software and
running the FVP models is a dual-core processor running at 2GHz with 12GB of
RAM. For best performance, use a machine with a quad-core processor running at
2.6GHz with 16GB of RAM.

The software has been tested on Ubuntu 14.04 LTS (64-bit). Packages used for
building the software were installed from that distribution unless otherwise
specified.

The software has also been built on Windows 7 Enterprise SP1, using CMD.EXE,
Cygwin, and Msys (MinGW) shells, using version 5.3.1 of the GNU toolchain.

Tools
-----

Install the required packages to build Trusted Firmware with the following
command:

::

    sudo apt-get install build-essential gcc make git libssl-dev

ARM TF has been tested with `Linaro Release 17.10`_.

Download and install the AArch32 or AArch64 little-endian GCC cross compiler.
The `Linaro Release Notes`_ documents which version of the compiler to use for a
given Linaro Release. Also, these `Linaro instructions`_ provide further
guidance and a script, which can be used to download Linaro deliverables
automatically.

Optionally, Trusted Firmware can be built using clang or ARM Compiler 6.
See instructions below on how to switch the default compiler.

In addition, the following optional packages and tools may be needed:

-  ``device-tree-compiler`` package if you need to rebuild the Flattened Device
   Tree (FDT) source files (``.dts`` files) provided with this software.

-  For debugging, ARM `Development Studio 5 (DS-5)`_.

-  To create and modify the diagram files included in the documentation, `Dia`_.
   This tool can be found in most Linux distributions. Inkscape is needed to
   generate the actual *.png files.

Getting the Trusted Firmware source code
----------------------------------------

Download the Trusted Firmware source code from Github:

::

    git clone https://github.com/ARM-software/arm-trusted-firmware.git

Building the Trusted Firmware
-----------------------------

-  Before building Trusted Firmware, the environment variable ``CROSS_COMPILE``
   must point to the Linaro cross compiler.

   For AArch64:

   ::

       export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-

   For AArch32:

   ::

       export CROSS_COMPILE=<path-to-aarch32-gcc>/bin/arm-linux-gnueabihf-

   It is possible to build Trusted Firmware using clang or ARM Compiler 6.
   To do so ``CC`` needs to point to the clang or armclang binary. Only the
   compiler is switched; the assembler and linker need to be provided by
   the GNU toolchain, thus ``CROSS_COMPILE`` should be set as described above.

   ARM Compiler 6 will be selected when the base name of the path assigned
   to ``CC`` matches the string 'armclang'.

   For AArch64 using ARM Compiler 6:

   ::

       export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-
       make CC=<path-to-armclang>/bin/armclang PLAT=<platform> all

   Clang will be selected when the base name of the path assigned to ``CC``
   contains the string 'clang'. This is to allow both clang and clang-X.Y
   to work.

   For AArch64 using clang:

   ::

       export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-
       make CC=<path-to-clang>/bin/clang PLAT=<platform> all

-  Change to the root directory of the Trusted Firmware source tree and build.

   For AArch64:

   ::

       make PLAT=<platform> all

   For AArch32:

   ::

       make PLAT=<platform> ARCH=aarch32 AARCH32_SP=sp_min all

   Notes:

   -  If ``PLAT`` is not specified, ``fvp`` is assumed by default. See the
      `Summary of build options`_ for more information on available build
      options.

   -  (AArch32 only) Currently only ``PLAT=fvp`` is supported.

   -  (AArch32 only) ``AARCH32_SP`` is the AArch32 EL3 Runtime Software and it
      corresponds to the BL32 image. A minimal ``AARCH32_SP``, sp\_min, is
      provided by ARM Trusted Firmware to demonstrate how PSCI Library can
      be integrated with an AArch32 EL3 Runtime Software. Some AArch32 EL3
      Runtime Software may include other runtime services, for example
      Trusted OS services. A guide to integrate PSCI library with AArch32
      EL3 Runtime Software can be found `here`_.

   -  (AArch64 only) The TSP (Test Secure Payload), corresponding to the BL32
      image, is not compiled in by default. Refer to the
      `Building the Test Secure Payload`_ section below.

   -  By default this produces a release version of the build. To produce a
      debug version instead, refer to the "Debugging options" section below.

   -  The build process creates products in a ``build`` directory tree, building
      the objects and binaries for each boot loader stage in separate
      sub-directories. The following boot loader binary files are created
      from the corresponding ELF files:

      -  ``build/<platform>/<build-type>/bl1.bin``
      -  ``build/<platform>/<build-type>/bl2.bin``
      -  ``build/<platform>/<build-type>/bl31.bin`` (AArch64 only)
      -  ``build/<platform>/<build-type>/bl32.bin`` (mandatory for AArch32)

      where ``<platform>`` is the name of the chosen platform and ``<build-type>``
      is either ``debug`` or ``release``. The actual number of images might differ
      depending on the platform.

-  Build products for a specific build variant can be removed using:

   ::

       make DEBUG=<D> PLAT=<platform> clean

   ... where ``<D>`` is ``0`` or ``1``, as specified when building.

   The build tree can be removed completely using:

   ::

       make realclean

Summary of build options
~~~~~~~~~~~~~~~~~~~~~~~~

ARM Trusted Firmware build system supports the following build options. Unless
mentioned otherwise, these options are expected to be specified at the build
command line and are not to be modified in any component makefiles. Note that
the build system doesn't track dependency for build options. Therefore, if any
of the build options are changed from a previous build, a clean build must be
performed.

Common build options
^^^^^^^^^^^^^^^^^^^^

-  ``AARCH32_SP`` : Choose the AArch32 Secure Payload component to be built as
   as the BL32 image when ``ARCH=aarch32``. The value should be the path to the
   directory containing the SP source, relative to the ``bl32/``; the directory
   is expected to contain a makefile called ``<aarch32_sp-value>.mk``.

-  ``ARCH`` : Choose the target build architecture for ARM Trusted Firmware.
   It can take either ``aarch64`` or ``aarch32`` as values. By default, it is
   defined to ``aarch64``.

-  ``ARM_ARCH_MAJOR``: The major version of ARM Architecture to target when
   compiling ARM Trusted Firmware. Its value must be numeric, and defaults to
   8 . See also, *ARMv8 Architecture Extensions* and
   *ARMv7 Architecture Extensions* in `Firmware Design`_.

-  ``ARM_ARCH_MINOR``: The minor version of ARM Architecture to target when
   compiling ARM Trusted Firmware. Its value must be a numeric, and defaults
   to 0. See also, *ARMv8 Architecture Extensions* in `Firmware Design`_.

-  ``ARM_GIC_ARCH``: Choice of ARM GIC architecture version used by the ARM
   Legacy GIC driver for implementing the platform GIC API. This API is used
   by the interrupt management framework. Default is 2 (that is, version 2.0).
   This build option is deprecated.

-  ``ARM_PLAT_MT``: This flag determines whether the ARM platform layer has to
   cater for the multi-threading ``MT`` bit when accessing MPIDR. When this flag
   is set, the functions which deal with MPIDR assume that the ``MT`` bit in
   MPIDR is set and access the bit-fields in MPIDR accordingly. Default value of
   this flag is 0. Note that this option is not used on FVP platforms.

-  ``BL2``: This is an optional build option which specifies the path to BL2
   image for the ``fip`` target. In this case, the BL2 in the ARM Trusted
   Firmware will not be built.

-  ``BL2U``: This is an optional build option which specifies the path to
   BL2U image. In this case, the BL2U in the ARM Trusted Firmware will not
   be built.

- ``BL2_AT_EL3``: This is an optional build option that enables the use of
   BL2 at EL3 execution level.

-  ``BL31``: This is an optional build option which specifies the path to
   BL31 image for the ``fip`` target. In this case, the BL31 in the ARM
   Trusted Firmware will not be built.

-  ``BL31_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL31 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BL32``: This is an optional build option which specifies the path to
   BL32 image for the ``fip`` target. In this case, the BL32 in the ARM
   Trusted Firmware will not be built.

- ``BL32_EXTRA1``: This is an optional build option which specifies the path to
   Trusted OS Extra1 image for the  ``fip`` target.

- ``BL32_EXTRA2``: This is an optional build option which specifies the path to
   Trusted OS Extra2 image for the ``fip`` target.

-  ``BL32_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL32 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BL33``: Path to BL33 image in the host file system. This is mandatory for
   ``fip`` target in case the BL2 from ARM Trusted Firmware is used.

-  ``BL33_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL33 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BUILD_MESSAGE_TIMESTAMP``: String used to identify the time and date of the
   compilation of each build. It must be set to a C string (including quotes
   where applicable). Defaults to a string that contains the time and date of
   the compilation.

-  ``BUILD_STRING``: Input string for VERSION\_STRING, which allows the TF build
   to be uniquely identified. Defaults to the current git commit id.

-  ``CFLAGS``: Extra user options appended on the compiler's command line in
   addition to the options set by the build system.

-  ``COLD_BOOT_SINGLE_CPU``: This option indicates whether the platform may
   release several CPUs out of reset. It can take either 0 (several CPUs may be
   brought up) or 1 (only one CPU will ever be brought up during cold reset).
   Default is 0. If the platform always brings up a single CPU, there is no
   need to distinguish between primary and secondary CPUs and the boot path can
   be optimised. The ``plat_is_my_cpu_primary()`` and
   ``plat_secondary_cold_boot_setup()`` platform porting interfaces do not need
   to be implemented in this case.

-  ``CRASH_REPORTING``: A non-zero value enables a console dump of processor
   register state when an unexpected exception occurs during execution of
   BL31. This option defaults to the value of ``DEBUG`` - i.e. by default
   this is only enabled for a debug build of the firmware.

-  ``CREATE_KEYS``: This option is used when ``GENERATE_COT=1``. It tells the
   certificate generation tool to create new keys in case no valid keys are
   present or specified. Allowed options are '0' or '1'. Default is '1'.

-  ``CTX_INCLUDE_AARCH32_REGS`` : Boolean option that, when set to 1, will cause
   the AArch32 system registers to be included when saving and restoring the
   CPU context. The option must be set to 0 for AArch64-only platforms (that
   is on hardware that does not implement AArch32, or at least not at EL1 and
   higher ELs). Default value is 1.

-  ``CTX_INCLUDE_FPREGS``: Boolean option that, when set to 1, will cause the FP
   registers to be included when saving and restoring the CPU context. Default
   is 0.

-  ``DEBUG``: Chooses between a debug and release build. It can take either 0
   (release) or 1 (debug) as values. 0 is the default.

-  ``EL3_PAYLOAD_BASE``: This option enables booting an EL3 payload instead of
   the normal boot flow. It must specify the entry point address of the EL3
   payload. Please refer to the "Booting an EL3 payload" section for more
   details.

-  ``ENABLE_AMU``: Boolean option to enable Activity Monitor Unit extensions.
   This is an optional architectural feature available on v8.4 onwards. Some
   v8.2 implementations also implement an AMU and this option can be used to
   enable this feature on those systems as well. Default is 0.

-  ``ENABLE_ASSERTIONS``: This option controls whether or not calls to ``assert()``
   are compiled out. For debug builds, this option defaults to 1, and calls to
   ``assert()`` are left in place. For release builds, this option defaults to 0
   and calls to ``assert()`` function are compiled out. This option can be set
   independently of ``DEBUG``. It can also be used to hide any auxiliary code
   that is only required for the assertion and does not fit in the assertion
   itself.

-  ``ENABLE_PMF``: Boolean option to enable support for optional Performance
   Measurement Framework(PMF). Default is 0.

-  ``ENABLE_PSCI_STAT``: Boolean option to enable support for optional PSCI
   functions ``PSCI_STAT_RESIDENCY`` and ``PSCI_STAT_COUNT``. Default is 0.
   In the absence of an alternate stat collection backend, ``ENABLE_PMF`` must
   be enabled. If ``ENABLE_PMF`` is set, the residency statistics are tracked in
   software.

-  ``ENABLE_RUNTIME_INSTRUMENTATION``: Boolean option to enable runtime
   instrumentation which injects timestamp collection points into
   Trusted Firmware to allow runtime performance to be measured.
   Currently, only PSCI is instrumented. Enabling this option enables
   the ``ENABLE_PMF`` build option as well. Default is 0.

-  ``ENABLE_SPE_FOR_LOWER_ELS`` : Boolean option to enable Statistical Profiling
   extensions. This is an optional architectural feature for AArch64.
   The default is 1 but is automatically disabled when the target architecture
   is AArch32.

-  ``ENABLE_SVE_FOR_NS``: Boolean option to enable Scalable Vector Extension
   (SVE) for the Non-secure world only. SVE is an optional architectural feature
   for AArch64. Note that when SVE is enabled for the Non-secure world, access
   to SIMD and floating-point functionality from the Secure world is disabled.
   This is to avoid corruption of the Non-secure world data in the Z-registers
   which are aliased by the SIMD and FP registers. The build option is not
   compatible with the ``CTX_INCLUDE_FPREGS`` build option, and will raise an
   assert on platforms where SVE is implemented and ``ENABLE_SVE_FOR_NS`` set to
   1. The default is 1 but is automatically disabled when the target
   architecture is AArch32.

-  ``ENABLE_STACK_PROTECTOR``: String option to enable the stack protection
   checks in GCC. Allowed values are "all", "strong" and "0" (default).
   "strong" is the recommended stack protection level if this feature is
   desired. 0 disables the stack protection. For all values other than 0, the
   ``plat_get_stack_protector_canary()`` platform hook needs to be implemented.
   The value is passed as the last component of the option
   ``-fstack-protector-$ENABLE_STACK_PROTECTOR``.

-  ``ERROR_DEPRECATED``: This option decides whether to treat the usage of
   deprecated platform APIs, helper functions or drivers within Trusted
   Firmware as error. It can take the value 1 (flag the use of deprecated
   APIs as error) or 0. The default is 0.

-  ``EL3_EXCEPTION_HANDLING``: When set to ``1``, enable handling of exceptions
   targeted at EL3. When set ``0`` (default), no exceptions are expected or
   handled at EL3, and a panic will result. This is supported only for AArch64
   builds.

-  ``FIP_NAME``: This is an optional build option which specifies the FIP
   filename for the ``fip`` target. Default is ``fip.bin``.

-  ``FWU_FIP_NAME``: This is an optional build option which specifies the FWU
   FIP filename for the ``fwu_fip`` target. Default is ``fwu_fip.bin``.

-  ``GENERATE_COT``: Boolean flag used to build and execute the ``cert_create``
   tool to create certificates as per the Chain of Trust described in
   `Trusted Board Boot`_. The build system then calls ``fiptool`` to
   include the certificates in the FIP and FWU\_FIP. Default value is '0'.

   Specify both ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=1`` to include support
   for the Trusted Board Boot feature in the BL1 and BL2 images, to generate
   the corresponding certificates, and to include those certificates in the
   FIP and FWU\_FIP.

   Note that if ``TRUSTED_BOARD_BOOT=0`` and ``GENERATE_COT=1``, the BL1 and BL2
   images will not include support for Trusted Board Boot. The FIP will still
   include the corresponding certificates. This FIP can be used to verify the
   Chain of Trust on the host machine through other mechanisms.

   Note that if ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=0``, the BL1 and BL2
   images will include support for Trusted Board Boot, but the FIP and FWU\_FIP
   will not include the corresponding certificates, causing a boot failure.

-  ``GICV2_G0_FOR_EL3``: Unlike GICv3, the GICv2 architecture doesn't have
   inherent support for specific EL3 type interrupts. Setting this build option
   to ``1`` assumes GICv2 *Group 0* interrupts are expected to target EL3, both
   by `platform abstraction layer`__ and `Interrupt Management Framework`__.
   This allows GICv2 platforms to enable features requiring EL3 interrupt type.
   This also means that all GICv2 Group 0 interrupts are delivered to EL3, and
   the Secure Payload interrupts needs to be synchronously handed over to Secure
   EL1 for handling. The default value of this option is ``0``, which means the
   Group 0 interrupts are assumed to be handled by Secure EL1.

   .. __: `platform-interrupt-controller-API.rst`
   .. __: `interrupt-framework-design.rst`

-  ``HANDLE_EA_EL3_FIRST``: When defined External Aborts and SError Interrupts
   will be always trapped in EL3 i.e. in BL31 at runtime.

-  ``HW_ASSISTED_COHERENCY``: On most ARM systems to-date, platform-specific
   software operations are required for CPUs to enter and exit coherency.
   However, there exists newer systems where CPUs' entry to and exit from
   coherency is managed in hardware. Such systems require software to only
   initiate the operations, and the rest is managed in hardware, minimizing
   active software management. In such systems, this boolean option enables ARM
   Trusted Firmware to carry out build and run-time optimizations during boot
   and power management operations. This option defaults to 0 and if it is
   enabled, then it implies ``WARMBOOT_ENABLE_DCACHE_EARLY`` is also enabled.

-  ``JUNO_AARCH32_EL3_RUNTIME``: This build flag enables you to execute EL3
   runtime software in AArch32 mode, which is required to run AArch32 on Juno.
   By default this flag is set to '0'. Enabling this flag builds BL1 and BL2 in
   AArch64 and facilitates the loading of ``SP_MIN`` and BL33 as AArch32 executable
   images.

-  ``KEY_ALG``: This build flag enables the user to select the algorithm to be
   used for generating the PKCS keys and subsequent signing of the certificate.
   It accepts 3 values viz. ``rsa``, ``rsa_1_5``, ``ecdsa``. The ``rsa_1_5`` is
   the legacy PKCS#1 RSA 1.5 algorithm which is not TBBR compliant and is
   retained only for compatibility. The default value of this flag is ``rsa``
   which is the TBBR compliant PKCS#1 RSA 2.1 scheme.

-  ``HASH_ALG``: This build flag enables the user to select the secure hash
   algorithm. It accepts 3 values viz. ``sha256``, ``sha384``, ``sha512``.
   The default value of this flag is ``sha256``.

-  ``LDFLAGS``: Extra user options appended to the linkers' command line in
   addition to the one set by the build system.

-  ``LOAD_IMAGE_V2``: Boolean option to enable support for new version (v2) of
   image loading, which provides more flexibility and scalability around what
   images are loaded and executed during boot. Default is 0.
   Note: ``TRUSTED_BOARD_BOOT`` is currently only supported for AArch64 when
   ``LOAD_IMAGE_V2`` is enabled.

-  ``LOG_LEVEL``: Chooses the log level, which controls the amount of console log
   output compiled into the build. This should be one of the following:

   ::

       0  (LOG_LEVEL_NONE)
       10 (LOG_LEVEL_NOTICE)
       20 (LOG_LEVEL_ERROR)
       30 (LOG_LEVEL_WARNING)
       40 (LOG_LEVEL_INFO)
       50 (LOG_LEVEL_VERBOSE)

   All log output up to and including the log level is compiled into the build.
   The default value is 40 in debug builds and 20 in release builds.

-  ``NON_TRUSTED_WORLD_KEY``: This option is used when ``GENERATE_COT=1``. It
   specifies the file that contains the Non-Trusted World private key in PEM
   format. If ``SAVE_KEYS=1``, this file name will be used to save the key.

-  ``NS_BL2U``: Path to NS\_BL2U image in the host file system. This image is
   optional. It is only needed if the platform makefile specifies that it
   is required in order to build the ``fwu_fip`` target.

-  ``NS_TIMER_SWITCH``: Enable save and restore for non-secure timer register
   contents upon world switch. It can take either 0 (don't save and restore) or
   1 (do save and restore). 0 is the default. An SPD may set this to 1 if it
   wants the timer registers to be saved and restored.

-  ``PL011_GENERIC_UART``: Boolean option to indicate the PL011 driver that
   the underlying hardware is not a full PL011 UART but a minimally compliant
   generic UART, which is a subset of the PL011. The driver will not access
   any register that is not part of the SBSA generic UART specification.
   Default value is 0 (a full PL011 compliant UART is present).

-  ``PLAT``: Choose a platform to build ARM Trusted Firmware for. The chosen
   platform name must be subdirectory of any depth under ``plat/``, and must
   contain a platform makefile named ``platform.mk``. For example to build ARM
   Trusted Firmware for ARM Juno board select PLAT=juno.

-  ``PRELOADED_BL33_BASE``: This option enables booting a preloaded BL33 image
   instead of the normal boot flow. When defined, it must specify the entry
   point address for the preloaded BL33 image. This option is incompatible with
   ``EL3_PAYLOAD_BASE``. If both are defined, ``EL3_PAYLOAD_BASE`` has priority
   over ``PRELOADED_BL33_BASE``.

-  ``PROGRAMMABLE_RESET_ADDRESS``: This option indicates whether the reset
   vector address can be programmed or is fixed on the platform. It can take
   either 0 (fixed) or 1 (programmable). Default is 0. If the platform has a
   programmable reset address, it is expected that a CPU will start executing
   code directly at the right address, both on a cold and warm reset. In this
   case, there is no need to identify the entrypoint on boot and the boot path
   can be optimised. The ``plat_get_my_entrypoint()`` platform porting interface
   does not need to be implemented in this case.

-  ``PSCI_EXTENDED_STATE_ID``: As per PSCI1.0 Specification, there are 2 formats
   possible for the PSCI power-state parameter viz original and extended
   State-ID formats. This flag if set to 1, configures the generic PSCI layer
   to use the extended format. The default value of this flag is 0, which
   means by default the original power-state format is used by the PSCI
   implementation. This flag should be specified by the platform makefile
   and it governs the return value of PSCI\_FEATURES API for CPU\_SUSPEND
   smc function id. When this option is enabled on ARM platforms, the
   option ``ARM_RECOM_STATE_ID_ENC`` needs to be set to 1 as well.

-  ``RESET_TO_BL31``: Enable BL31 entrypoint as the CPU reset vector instead
   of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
   entrypoint) or 1 (CPU reset to BL31 entrypoint).
   The default value is 0.

-  ``RESET_TO_SP_MIN``: SP\_MIN is the minimal AArch32 Secure Payload provided in
   ARM Trusted Firmware. This flag configures SP\_MIN entrypoint as the CPU
   reset vector instead of the BL1 entrypoint. It can take the value 0 (CPU
   reset to BL1 entrypoint) or 1 (CPU reset to SP\_MIN entrypoint). The default
   value is 0.

-  ``ROT_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the ROT private key in PEM format. If ``SAVE_KEYS=1``, this
   file name will be used to save the key.

-  ``SAVE_KEYS``: This option is used when ``GENERATE_COT=1``. It tells the
   certificate generation tool to save the keys used to establish the Chain of
   Trust. Allowed options are '0' or '1'. Default is '0' (do not save).

-  ``SCP_BL2``: Path to SCP\_BL2 image in the host file system. This image is optional.
   If a SCP\_BL2 image is present then this option must be passed for the ``fip``
   target.

-  ``SCP_BL2_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the SCP\_BL2 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``SCP_BL2U``: Path to SCP\_BL2U image in the host file system. This image is
   optional. It is only needed if the platform makefile specifies that it
   is required in order to build the ``fwu_fip`` target.

-  ``SDEI_SUPPORT``: Setting this to ``1`` enables support for Software
   Delegated Exception Interface to BL31 image. This defaults to ``0``.

   When set to ``1``, the build option ``EL3_EXCEPTION_HANDLING`` must also be
   set to ``1``.

-  ``SEPARATE_CODE_AND_RODATA``: Whether code and read-only data should be
   isolated on separate memory pages. This is a trade-off between security and
   memory usage. See "Isolating code and read-only data on separate memory
   pages" section in `Firmware Design`_. This flag is disabled by default and
   affects all BL images.

-  ``SPD``: Choose a Secure Payload Dispatcher component to be built into the
   Trusted Firmware. This build option is only valid if ``ARCH=aarch64``. The
   value should be the path to the directory containing the SPD source,
   relative to ``services/spd/``; the directory is expected to
   contain a makefile called ``<spd-value>.mk``.

-  ``SPIN_ON_BL1_EXIT``: This option introduces an infinite loop in BL1. It can
   take either 0 (no loop) or 1 (add a loop). 0 is the default. This loop stops
   execution in BL1 just before handing over to BL31. At this point, all
   firmware images have been loaded in memory, and the MMU and caches are
   turned off. Refer to the "Debugging options" section for more details.

- ``SP_MIN_WITH_SECURE_FIQ``: Boolean flag to indicate the SP_MIN handles
   secure interrupts (caught through the FIQ line). Platforms can enable
   this directive if they need to handle such interruption. When enabled,
   the FIQ are handled in monitor mode and non secure world is not allowed
   to mask these events. Platforms that enable FIQ handling in SP_MIN shall
   implement the api ``sp_min_plat_fiq_handler()``. The default value is 0.

-  ``TRUSTED_BOARD_BOOT``: Boolean flag to include support for the Trusted Board
   Boot feature. When set to '1', BL1 and BL2 images include support to load
   and verify the certificates and images in a FIP, and BL1 includes support
   for the Firmware Update. The default value is '0'. Generation and inclusion
   of certificates in the FIP and FWU\_FIP depends upon the value of the
   ``GENERATE_COT`` option.

   Note: This option depends on ``CREATE_KEYS`` to be enabled. If the keys
   already exist in disk, they will be overwritten without further notice.

-  ``TRUSTED_WORLD_KEY``: This option is used when ``GENERATE_COT=1``. It
   specifies the file that contains the Trusted World private key in PEM
   format. If ``SAVE_KEYS=1``, this file name will be used to save the key.

-  ``TSP_INIT_ASYNC``: Choose BL32 initialization method as asynchronous or
   synchronous, (see "Initializing a BL32 Image" section in
   `Firmware Design`_). It can take the value 0 (BL32 is initialized using
   synchronous method) or 1 (BL32 is initialized using asynchronous method).
   Default is 0.

-  ``TSP_NS_INTR_ASYNC_PREEMPT``: A non zero value enables the interrupt
   routing model which routes non-secure interrupts asynchronously from TSP
   to EL3 causing immediate preemption of TSP. The EL3 is responsible
   for saving and restoring the TSP context in this routing model. The
   default routing model (when the value is 0) is to route non-secure
   interrupts to TSP allowing it to save its context and hand over
   synchronously to EL3 via an SMC.

-  ``USE_COHERENT_MEM``: This flag determines whether to include the coherent
   memory region in the BL memory map or not (see "Use of Coherent memory in
   Trusted Firmware" section in `Firmware Design`_). It can take the value 1
   (Coherent memory region is included) or 0 (Coherent memory region is
   excluded). Default is 1.

-  ``V``: Verbose build. If assigned anything other than 0, the build commands
   are printed. Default is 0.

-  ``VERSION_STRING``: String used in the log output for each TF image. Defaults
   to a string formed by concatenating the version number, build type and build
   string.

-  ``WARMBOOT_ENABLE_DCACHE_EARLY`` : Boolean option to enable D-cache early on
   the CPU after warm boot. This is applicable for platforms which do not
   require interconnect programming to enable cache coherency (eg: single
   cluster platforms). If this option is enabled, then warm boot path
   enables D-caches immediately after enabling MMU. This option defaults to 0.

ARM development platform specific build options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  ``ARM_BL31_IN_DRAM``: Boolean option to select loading of BL31 in TZC secured
   DRAM. By default, BL31 is in the secure SRAM. Set this flag to 1 to load
   BL31 in TZC secured DRAM. If TSP is present, then setting this option also
   sets the TSP location to DRAM and ignores the ``ARM_TSP_RAM_LOCATION`` build
   flag.

-  ``ARM_BOARD_OPTIMISE_MEM``: Boolean option to enable or disable optimisation
   of the memory reserved for each image. This affects the maximum size of each
   BL image as well as the number of allocated memory regions and translation
   tables. By default this flag is 0, which means it uses the default
   unoptimised values for these macros. ARM development platforms that wish to
   optimise memory usage need to set this flag to 1 and must override the
   related macros.

-  ``ARM_CONFIG_CNTACR``: boolean option to unlock access to the ``CNTBase<N>``
   frame registers by setting the ``CNTCTLBase.CNTACR<N>`` register bits. The
   frame number ``<N>`` is defined by ``PLAT_ARM_NSTIMER_FRAME_ID``, which should
   match the frame used by the Non-Secure image (normally the Linux kernel).
   Default is true (access to the frame is allowed).

-  ``ARM_DISABLE_TRUSTED_WDOG``: boolean option to disable the Trusted Watchdog.
   By default, ARM platforms use a watchdog to trigger a system reset in case
   an error is encountered during the boot process (for example, when an image
   could not be loaded or authenticated). The watchdog is enabled in the early
   platform setup hook at BL1 and disabled in the BL1 prepare exit hook. The
   Trusted Watchdog may be disabled at build time for testing or development
   purposes.

-  ``ARM_RECOM_STATE_ID_ENC``: The PSCI1.0 specification recommends an encoding
   for the construction of composite state-ID in the power-state parameter.
   The existing PSCI clients currently do not support this encoding of
   State-ID yet. Hence this flag is used to configure whether to use the
   recommended State-ID encoding or not. The default value of this flag is 0,
   in which case the platform is configured to expect NULL in the State-ID
   field of power-state parameter.

-  ``ARM_ROTPK_LOCATION``: used when ``TRUSTED_BOARD_BOOT=1``. It specifies the
   location of the ROTPK hash returned by the function ``plat_get_rotpk_info()``
   for ARM platforms. Depending on the selected option, the proper private key
   must be specified using the ``ROT_KEY`` option when building the Trusted
   Firmware. This private key will be used by the certificate generation tool
   to sign the BL2 and Trusted Key certificates. Available options for
   ``ARM_ROTPK_LOCATION`` are:

   -  ``regs`` : return the ROTPK hash stored in the Trusted root-key storage
      registers. The private key corresponding to this ROTPK hash is not
      currently available.
   -  ``devel_rsa`` : return a development public key hash embedded in the BL1
      and BL2 binaries. This hash has been obtained from the RSA public key
      ``arm_rotpk_rsa.der``, located in ``plat/arm/board/common/rotpk``. To use
      this option, ``arm_rotprivk_rsa.pem`` must be specified as ``ROT_KEY`` when
      creating the certificates.
   -  ``devel_ecdsa`` : return a development public key hash embedded in the BL1
      and BL2 binaries. This hash has been obtained from the ECDSA public key
      ``arm_rotpk_ecdsa.der``, located in ``plat/arm/board/common/rotpk``. To use
      this option, ``arm_rotprivk_ecdsa.pem`` must be specified as ``ROT_KEY``
      when creating the certificates.

-  ``ARM_TSP_RAM_LOCATION``: location of the TSP binary. Options:

   -  ``tsram`` : Trusted SRAM (default option when TBB is not enabled)
   -  ``tdram`` : Trusted DRAM (if available)
   -  ``dram``  : Secure region in DRAM (default option when TBB is enabled,
                  configured by the TrustZone controller)

-  ``ARM_XLAT_TABLES_LIB_V1``: boolean option to compile the Trusted Firmware
   with version 1 of the translation tables library instead of version 2. It is
   set to 0 by default, which selects version 2.

-  ``ARM_CRYPTOCELL_INTEG`` : bool option to enable Trusted Firmware to invoke
   ARM® TrustZone® CryptoCell functionality for Trusted Board Boot on capable
   ARM platforms. If this option is specified, then the path to the CryptoCell
   SBROM library must be specified via ``CCSBROM_LIB_PATH`` flag.

For a better understanding of these options, the ARM development platform memory
map is explained in the `Firmware Design`_.

ARM CSS platform specific build options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  ``CSS_DETECT_PRE_1_7_0_SCP``: Boolean flag to detect SCP version
   incompatibility. Version 1.7.0 of the SCP firmware made a non-backwards
   compatible change to the MTL protocol, used for AP/SCP communication.
   Trusted Firmware no longer supports earlier SCP versions. If this option is
   set to 1 then Trusted Firmware will detect if an earlier version is in use.
   Default is 1.

-  ``CSS_LOAD_SCP_IMAGES``: Boolean flag, which when set, adds SCP\_BL2 and
   SCP\_BL2U to the FIP and FWU\_FIP respectively, and enables them to be loaded
   during boot. Default is 1.

-  ``CSS_USE_SCMI_SDS_DRIVER``: Boolean flag which selects SCMI/SDS drivers
   instead of SCPI/BOM driver for communicating with the SCP during power
   management operations and for SCP RAM Firmware transfer. If this option
   is set to 1, then SCMI/SDS drivers will be used. Default is 0.

ARM FVP platform specific build options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  ``FVP_CLUSTER_COUNT`` : Configures the cluster count to be used to
   build the topology tree within Trusted Firmware. By default the
   Trusted Firmware is configured for dual cluster topology and this option
   can be used to override the default value.

-  ``FVP_INTERCONNECT_DRIVER``: Selects the interconnect driver to be built. The
   default interconnect driver depends on the value of ``FVP_CLUSTER_COUNT`` as
   explained in the options below:

   -  ``FVP_CCI`` : The CCI driver is selected. This is the default
      if 0 < ``FVP_CLUSTER_COUNT`` <= 2.
   -  ``FVP_CCN`` : The CCN driver is selected. This is the default
      if ``FVP_CLUSTER_COUNT`` > 2.

-  ``FVP_MAX_PE_PER_CPU``: Sets the maximum number of PEs implemented on any CPU
   in the system. This option defaults to 1. Note that the build option
   ``ARM_PLAT_MT`` doesn't have any effect on FVP platforms.

-  ``FVP_USE_GIC_DRIVER`` : Selects the GIC driver to be built. Options:

   -  ``FVP_GIC600`` : The GIC600 implementation of GICv3 is selected
   -  ``FVP_GICV2`` : The GICv2 only driver is selected
   -  ``FVP_GICV3`` : The GICv3 only driver is selected (default option)
   -  ``FVP_GICV3_LEGACY``: The Legacy GICv3 driver is selected (deprecated)
      Note: If Trusted Firmware is compiled with this option on FVPs with
      GICv3 hardware, then it configures the hardware to run in GICv2
      emulation mode

-  ``FVP_USE_SP804_TIMER`` : Use the SP804 timer instead of the Generic Timer
   for functions that wait for an arbitrary time length (udelay and mdelay).
   The default value is 0.

Debugging options
~~~~~~~~~~~~~~~~~

To compile a debug version and make the build more verbose use

::

    make PLAT=<platform> DEBUG=1 V=1 all

AArch64 GCC uses DWARF version 4 debugging symbols by default. Some tools (for
example DS-5) might not support this and may need an older version of DWARF
symbols to be emitted by GCC. This can be achieved by using the
``-gdwarf-<version>`` flag, with the version being set to 2 or 3. Setting the
version to 2 is recommended for DS-5 versions older than 5.16.

When debugging logic problems it might also be useful to disable all compiler
optimizations by using ``-O0``.

NOTE: Using ``-O0`` could cause output images to be larger and base addresses
might need to be recalculated (see the **Memory layout on ARM development
platforms** section in the `Firmware Design`_).

Extra debug options can be passed to the build system by setting ``CFLAGS`` or
``LDFLAGS``:

.. code:: makefile

    CFLAGS='-O0 -gdwarf-2'                                     \
    make PLAT=<platform> DEBUG=1 V=1 all

Note that using ``-Wl,`` style compilation driver options in ``CFLAGS`` will be
ignored as the linker is called directly.

It is also possible to introduce an infinite loop to help in debugging the
post-BL2 phase of the Trusted Firmware. This can be done by rebuilding BL1 with
the ``SPIN_ON_BL1_EXIT=1`` build flag. Refer to the `Summary of build options`_
section. In this case, the developer may take control of the target using a
debugger when indicated by the console output. When using DS-5, the following
commands can be used:

::

    # Stop target execution
    interrupt

    #
    # Prepare your debugging environment, e.g. set breakpoints
    #

    # Jump over the debug loop
    set var $AARCH64::$Core::$PC = $AARCH64::$Core::$PC + 4

    # Resume execution
    continue

Building the Test Secure Payload
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The TSP is coupled with a companion runtime service in the BL31 firmware,
called the TSPD. Therefore, if you intend to use the TSP, the BL31 image
must be recompiled as well. For more information on SPs and SPDs, see the
`Secure-EL1 Payloads and Dispatchers`_ section in the `Firmware Design`_.

First clean the Trusted Firmware build directory to get rid of any previous
BL31 binary. Then to build the TSP image use:

::

    make PLAT=<platform> SPD=tspd all

An additional boot loader binary file is created in the ``build`` directory:

::

    build/<platform>/<build-type>/bl32.bin

Checking source code style
~~~~~~~~~~~~~~~~~~~~~~~~~~

When making changes to the source for submission to the project, the source
must be in compliance with the Linux style guide, and to assist with this check
the project Makefile contains two targets, which both utilise the
``checkpatch.pl`` script that ships with the Linux source tree.

To check the entire source tree, you must first download a copy of
``checkpatch.pl`` (or the full Linux source), set the ``CHECKPATCH`` environment
variable to point to the script and build the target checkcodebase:

::

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkcodebase

To just check the style on the files that differ between your local branch and
the remote master, use:

::

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkpatch

If you wish to check your patch against something other than the remote master,
set the ``BASE_COMMIT`` variable to your desired branch. By default, ``BASE_COMMIT``
is set to ``origin/master``.

Building and using the FIP tool
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Firmware Image Package (FIP) is a packaging format used by the Trusted Firmware
project to package firmware images in a single binary. The number and type of
images that should be packed in a FIP is platform specific and may include TF
images and other firmware images required by the platform. For example, most
platforms require a BL33 image which corresponds to the normal world bootloader
(e.g. UEFI or U-Boot).

The TF build system provides the make target ``fip`` to create a FIP file for the
specified platform using the FIP creation tool included in the TF project.
Examples below show how to build a FIP file for FVP, packaging TF images and a
BL33 image.

For AArch64:

::

    make PLAT=fvp BL33=<path/to/bl33.bin> fip

For AArch32:

::

    make PLAT=fvp ARCH=aarch32 AARCH32_SP=sp_min BL33=<path/to/bl33.bin> fip

Note that AArch32 support for Normal world boot loader (BL33), like U-boot or
UEFI, on FVP is not available upstream. Hence custom solutions are required to
allow Linux boot on FVP. These instructions assume such a custom boot loader
(BL33) is available.

The resulting FIP may be found in:

::

    build/fvp/<build-type>/fip.bin

For advanced operations on FIP files, it is also possible to independently build
the tool and create or modify FIPs using this tool. To do this, follow these
steps:

It is recommended to remove old artifacts before building the tool:

::

    make -C tools/fiptool clean

Build the tool:

::

    make [DEBUG=1] [V=1] fiptool

The tool binary can be located in:

::

    ./tools/fiptool/fiptool

Invoking the tool with ``--help`` will print a help message with all available
options.

Example 1: create a new Firmware package ``fip.bin`` that contains BL2 and BL31:

::

    ./tools/fiptool/fiptool create \
        --tb-fw build/<platform>/<build-type>/bl2.bin \
        --soc-fw build/<platform>/<build-type>/bl31.bin \
        fip.bin

Example 2: view the contents of an existing Firmware package:

::

    ./tools/fiptool/fiptool info <path-to>/fip.bin

Example 3: update the entries of an existing Firmware package:

::

    # Change the BL2 from Debug to Release version
    ./tools/fiptool/fiptool update \
        --tb-fw build/<platform>/release/bl2.bin \
        build/<platform>/debug/fip.bin

Example 4: unpack all entries from an existing Firmware package:

::

    # Images will be unpacked to the working directory
    ./tools/fiptool/fiptool unpack <path-to>/fip.bin

Example 5: remove an entry from an existing Firmware package:

::

    ./tools/fiptool/fiptool remove \
        --tb-fw build/<platform>/debug/fip.bin

Note that if the destination FIP file exists, the create, update and
remove operations will automatically overwrite it.

The unpack operation will fail if the images already exist at the
destination. In that case, use -f or --force to continue.

More information about FIP can be found in the `Firmware Design`_ document.

Migrating from fip\_create to fiptool
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The previous version of fiptool was called fip\_create. A compatibility script
that emulates the basic functionality of the previous fip\_create is provided.
However, users are strongly encouraged to migrate to fiptool.

-  To create a new FIP file, replace "fip\_create" with "fiptool create".
-  To update a FIP file, replace "fip\_create" with "fiptool update".
-  To dump the contents of a FIP file, replace "fip\_create --dump"
   with "fiptool info".

Building FIP images with support for Trusted Board Boot
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Trusted Board Boot primarily consists of the following two features:

-  Image Authentication, described in `Trusted Board Boot`_, and
-  Firmware Update, described in `Firmware Update`_

The following steps should be followed to build FIP and (optionally) FWU\_FIP
images with support for these features:

#. Fulfill the dependencies of the ``mbedtls`` cryptographic and image parser
   modules by checking out a recent version of the `mbed TLS Repository`_. It
   is important to use a version that is compatible with TF and fixes any
   known security vulnerabilities. See `mbed TLS Security Center`_ for more
   information. The latest version of TF is tested with tag ``mbedtls-2.6.0``.

   The ``drivers/auth/mbedtls/mbedtls_*.mk`` files contain the list of mbed TLS
   source files the modules depend upon.
   ``include/drivers/auth/mbedtls/mbedtls_config.h`` contains the configuration
   options required to build the mbed TLS sources.

   Note that the mbed TLS library is licensed under the Apache version 2.0
   license. Using mbed TLS source code will affect the licensing of
   Trusted Firmware binaries that are built using this library.

#. To build the FIP image, ensure the following command line variables are set
   while invoking ``make`` to build Trusted Firmware:

   -  ``MBEDTLS_DIR=<path of the directory containing mbed TLS sources>``
   -  ``TRUSTED_BOARD_BOOT=1``
   -  ``GENERATE_COT=1``

   In the case of ARM platforms, the location of the ROTPK hash must also be
   specified at build time. Two locations are currently supported (see
   ``ARM_ROTPK_LOCATION`` build option):

   -  ``ARM_ROTPK_LOCATION=regs``: the ROTPK hash is obtained from the Trusted
      root-key storage registers present in the platform. On Juno, this
      registers are read-only. On FVP Base and Cortex models, the registers
      are read-only, but the value can be specified using the command line
      option ``bp.trusted_key_storage.public_key`` when launching the model.
      On both Juno and FVP models, the default value corresponds to an
      ECDSA-SECP256R1 public key hash, whose private part is not currently
      available.

   -  ``ARM_ROTPK_LOCATION=devel_rsa``: use the ROTPK hash that is hardcoded
      in the ARM platform port. The private/public RSA key pair may be
      found in ``plat/arm/board/common/rotpk``.

   -  ``ARM_ROTPK_LOCATION=devel_ecdsa``: use the ROTPK hash that is hardcoded
      in the ARM platform port. The private/public ECDSA key pair may be
      found in ``plat/arm/board/common/rotpk``.

   Example of command line using RSA development keys:

   ::

       MBEDTLS_DIR=<path of the directory containing mbed TLS sources> \
       make PLAT=<platform> TRUSTED_BOARD_BOOT=1 GENERATE_COT=1        \
       ARM_ROTPK_LOCATION=devel_rsa                                    \
       ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
       BL33=<path-to>/<bl33_image>                                     \
       all fip

   The result of this build will be the bl1.bin and the fip.bin binaries. This
   FIP will include the certificates corresponding to the Chain of Trust
   described in the TBBR-client document. These certificates can also be found
   in the output build directory.

#. The optional FWU\_FIP contains any additional images to be loaded from
   Non-Volatile storage during the `Firmware Update`_ process. To build the
   FWU\_FIP, any FWU images required by the platform must be specified on the
   command line. On ARM development platforms like Juno, these are:

   -  NS\_BL2U. The AP non-secure Firmware Updater image.
   -  SCP\_BL2U. The SCP Firmware Update Configuration image.

   Example of Juno command line for generating both ``fwu`` and ``fwu_fip``
   targets using RSA development:

   ::

       MBEDTLS_DIR=<path of the directory containing mbed TLS sources> \
       make PLAT=juno TRUSTED_BOARD_BOOT=1 GENERATE_COT=1              \
       ARM_ROTPK_LOCATION=devel_rsa                                    \
       ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
       BL33=<path-to>/<bl33_image>                                     \
       SCP_BL2=<path-to>/<scp_bl2_image>                               \
       SCP_BL2U=<path-to>/<scp_bl2u_image>                             \
       NS_BL2U=<path-to>/<ns_bl2u_image>                               \
       all fip fwu_fip

   Note: The BL2U image will be built by default and added to the FWU\_FIP.
   The user may override this by adding ``BL2U=<path-to>/<bl2u_image>``
   to the command line above.

   Note: Building and installing the non-secure and SCP FWU images (NS\_BL1U,
   NS\_BL2U and SCP\_BL2U) is outside the scope of this document.

   The result of this build will be bl1.bin, fip.bin and fwu\_fip.bin binaries.
   Both the FIP and FWU\_FIP will include the certificates corresponding to the
   Chain of Trust described in the TBBR-client document. These certificates
   can also be found in the output build directory.

Building the Certificate Generation Tool
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``cert_create`` tool is built as part of the TF build process when the ``fip``
make target is specified and TBB is enabled (as described in the previous
section), but it can also be built separately with the following command:

::

    make PLAT=<platform> [DEBUG=1] [V=1] certtool

For platforms that do not require their own IDs in certificate files,
the generic 'cert\_create' tool can be built with the following command:

::

    make USE_TBBR_DEFS=1 [DEBUG=1] [V=1] certtool

``DEBUG=1`` builds the tool in debug mode. ``V=1`` makes the build process more
verbose. The following command should be used to obtain help about the tool:

::

    ./tools/cert_create/cert_create -h

Building a FIP for Juno and FVP
-------------------------------

This section provides Juno and FVP specific instructions to build Trusted
Firmware, obtain the additional required firmware, and pack it all together in
a single FIP binary. It assumes that a `Linaro Release`_ has been installed.

Note: Pre-built binaries for AArch32 are available from Linaro Release 16.12
onwards. Before that release, pre-built binaries are only available for AArch64.

Note: follow the full instructions for one platform before switching to a
different one. Mixing instructions for different platforms may result in
corrupted binaries.

#. Clean the working directory

   ::

       make realclean

#. Obtain SCP\_BL2 (Juno) and BL33 (all platforms)

   Use the fiptool to extract the SCP\_BL2 and BL33 images from the FIP
   package included in the Linaro release:

   ::

       # Build the fiptool
       make [DEBUG=1] [V=1] fiptool

       # Unpack firmware images from Linaro FIP
       ./tools/fiptool/fiptool unpack \
            <path/to/linaro/release>/fip.bin

   The unpack operation will result in a set of binary images extracted to the
   current working directory. The SCP\_BL2 image corresponds to
   ``scp-fw.bin`` and BL33 corresponds to ``nt-fw.bin``.

   Note: the fiptool will complain if the images to be unpacked already
   exist in the current directory. If that is the case, either delete those
   files or use the ``--force`` option to overwrite.

   Note for AArch32, the instructions below assume that nt-fw.bin is a custom
   Normal world boot loader that supports AArch32.

#. Build TF images and create a new FIP for FVP

   ::

       # AArch64
       make PLAT=fvp BL33=nt-fw.bin all fip

       # AArch32
       make PLAT=fvp ARCH=aarch32 AARCH32_SP=sp_min BL33=nt-fw.bin all fip

#. Build TF images and create a new FIP for Juno

   For AArch64:

   Building for AArch64 on Juno simply requires the addition of ``SCP_BL2``
   as a build parameter.

   ::

       make PLAT=juno all fip \
       BL33=<path-to-juno-oe-uboot>/SOFTWARE/bl33-uboot.bin \
       SCP_BL2=<path-to-juno-busybox-uboot>/SOFTWARE/scp_bl2.bin

   For AArch32:

   Hardware restrictions on Juno prevent cold reset into AArch32 execution mode,
   therefore BL1 and BL2 must be compiled for AArch64, and BL32 is compiled
   separately for AArch32.

   -  Before building BL32, the environment variable ``CROSS_COMPILE`` must point
      to the AArch32 Linaro cross compiler.

      ::

          export CROSS_COMPILE=<path-to-aarch32-gcc>/bin/arm-linux-gnueabihf-

   -  Build BL32 in AArch32.

      ::

          make ARCH=aarch32 PLAT=juno AARCH32_SP=sp_min \
          RESET_TO_SP_MIN=1 JUNO_AARCH32_EL3_RUNTIME=1 bl32

   -  Before building BL1 and BL2, the environment variable ``CROSS_COMPILE``
      must point to the AArch64 Linaro cross compiler.

      ::

          export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-

   -  The following parameters should be used to build BL1 and BL2 in AArch64
      and point to the BL32 file.

      ::

          make ARCH=aarch64 PLAT=juno LOAD_IMAGE_V2=1 JUNO_AARCH32_EL3_RUNTIME=1 \
          BL33=<path-to-juno32-oe-uboot>/SOFTWARE/bl33-uboot.bin \
          SCP_BL2=<path-to-juno32-oe-uboot>/SOFTWARE/scp_bl2.bin \
          BL32=<path-to-bl32>/bl32.bin all fip

The resulting BL1 and FIP images may be found in:

::

    # Juno
    ./build/juno/release/bl1.bin
    ./build/juno/release/fip.bin

    # FVP
    ./build/fvp/release/bl1.bin
    ./build/fvp/release/fip.bin


Booting Firmware Update images
-------------------------------------

When Firmware Update (FWU) is enabled there are at least 2 new images
that have to be loaded, the Non-Secure FWU ROM (NS-BL1U), and the
FWU FIP.

Juno
~~~~

The new images must be programmed in flash memory by adding
an entry in the ``SITE1/HBI0262x/images.txt`` configuration file
on the Juno SD card (where ``x`` depends on the revision of the Juno board).
Refer to the `Juno Getting Started Guide`_, section 2.3 "Flash memory
programming" for more information. User should ensure these do not
overlap with any other entries in the file.

::

	NOR10UPDATE: AUTO                       ;Image Update:NONE/AUTO/FORCE
	NOR10ADDRESS: 0x00400000                ;Image Flash Address [ns_bl2u_base_address]
	NOR10FILE: \SOFTWARE\fwu_fip.bin        ;Image File Name
	NOR10LOAD: 00000000                     ;Image Load Address
	NOR10ENTRY: 00000000                    ;Image Entry Point

	NOR11UPDATE: AUTO                       ;Image Update:NONE/AUTO/FORCE
	NOR11ADDRESS: 0x03EB8000                ;Image Flash Address [ns_bl1u_base_address]
	NOR11FILE: \SOFTWARE\ns_bl1u.bin        ;Image File Name
	NOR11LOAD: 00000000                     ;Image Load Address

The address ns_bl1u_base_address is the value of NS_BL1U_BASE - 0x8000000.
In the same way, the address ns_bl2u_base_address is the value of
NS_BL2U_BASE - 0x8000000.

FVP
~~~

The additional fip images must be loaded with:

::

    --data cluster0.cpu0="<path_to>/ns_bl1u.bin"@0x0beb8000	[ns_bl1u_base_address]
    --data cluster0.cpu0="<path_to>/fwu_fip.bin"@0x08400000	[ns_bl2u_base_address]

The address ns_bl1u_base_address is the value of NS_BL1U_BASE.
In the same way, the address ns_bl2u_base_address is the value of
NS_BL2U_BASE.


EL3 payloads alternative boot flow
----------------------------------

On a pre-production system, the ability to execute arbitrary, bare-metal code at
the highest exception level is required. It allows full, direct access to the
hardware, for example to run silicon soak tests.

Although it is possible to implement some baremetal secure firmware from
scratch, this is a complex task on some platforms, depending on the level of
configuration required to put the system in the expected state.

Rather than booting a baremetal application, a possible compromise is to boot
``EL3 payloads`` through the Trusted Firmware instead. This is implemented as an
alternative boot flow, where a modified BL2 boots an EL3 payload, instead of
loading the other BL images and passing control to BL31. It reduces the
complexity of developing EL3 baremetal code by:

-  putting the system into a known architectural state;
-  taking care of platform secure world initialization;
-  loading the SCP\_BL2 image if required by the platform.

When booting an EL3 payload on ARM standard platforms, the configuration of the
TrustZone controller is simplified such that only region 0 is enabled and is
configured to permit secure access only. This gives full access to the whole
DRAM to the EL3 payload.

The system is left in the same state as when entering BL31 in the default boot
flow. In particular:

-  Running in EL3;
-  Current state is AArch64;
-  Little-endian data access;
-  All exceptions disabled;
-  MMU disabled;
-  Caches disabled.

Booting an EL3 payload
~~~~~~~~~~~~~~~~~~~~~~

The EL3 payload image is a standalone image and is not part of the FIP. It is
not loaded by the Trusted Firmware. Therefore, there are 2 possible scenarios:

-  The EL3 payload may reside in non-volatile memory (NVM) and execute in
   place. In this case, booting it is just a matter of specifying the right
   address in NVM through ``EL3_PAYLOAD_BASE`` when building the TF.

-  The EL3 payload needs to be loaded in volatile memory (e.g. DRAM) at
   run-time.

To help in the latter scenario, the ``SPIN_ON_BL1_EXIT=1`` build option can be
used. The infinite loop that it introduces in BL1 stops execution at the right
moment for a debugger to take control of the target and load the payload (for
example, over JTAG).

It is expected that this loading method will work in most cases, as a debugger
connection is usually available in a pre-production system. The user is free to
use any other platform-specific mechanism to load the EL3 payload, though.

Booting an EL3 payload on FVP
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The EL3 payloads boot flow requires the CPU's mailbox to be cleared at reset for
the secondary CPUs holding pen to work properly. Unfortunately, its reset value
is undefined on the FVP platform and the FVP platform code doesn't clear it.
Therefore, one must modify the way the model is normally invoked in order to
clear the mailbox at start-up.

One way to do that is to create an 8-byte file containing all zero bytes using
the following command:

::

    dd if=/dev/zero of=mailbox.dat bs=1 count=8

and pre-load it into the FVP memory at the mailbox address (i.e. ``0x04000000``)
using the following model parameters:

::

    --data cluster0.cpu0=mailbox.dat@0x04000000   [Base FVPs]
    --data=mailbox.dat@0x04000000                 [Foundation FVP]

To provide the model with the EL3 payload image, the following methods may be
used:

#. If the EL3 payload is able to execute in place, it may be programmed into
   flash memory. On Base Cortex and AEM FVPs, the following model parameter
   loads it at the base address of the NOR FLASH1 (the NOR FLASH0 is already
   used for the FIP):

   ::

       -C bp.flashloader1.fname="/path/to/el3-payload"

   On Foundation FVP, there is no flash loader component and the EL3 payload
   may be programmed anywhere in flash using method 3 below.

#. When using the ``SPIN_ON_BL1_EXIT=1`` loading method, the following DS-5
   command may be used to load the EL3 payload ELF image over JTAG:

   ::

       load /path/to/el3-payload.elf

#. The EL3 payload may be pre-loaded in volatile memory using the following
   model parameters:

   ::

       --data cluster0.cpu0="/path/to/el3-payload"@address  [Base FVPs]
       --data="/path/to/el3-payload"@address                [Foundation FVP]

   The address provided to the FVP must match the ``EL3_PAYLOAD_BASE`` address
   used when building the Trusted Firmware.

Booting an EL3 payload on Juno
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the EL3 payload is able to execute in place, it may be programmed in flash
memory by adding an entry in the ``SITE1/HBI0262x/images.txt`` configuration file
on the Juno SD card (where ``x`` depends on the revision of the Juno board).
Refer to the `Juno Getting Started Guide`_, section 2.3 "Flash memory
programming" for more information.

Alternatively, the same DS-5 command mentioned in the FVP section above can
be used to load the EL3 payload's ELF file over JTAG on Juno.

Preloaded BL33 alternative boot flow
------------------------------------

Some platforms have the ability to preload BL33 into memory instead of relying
on Trusted Firmware to load it. This may simplify packaging of the normal world
code and improve performance in a development environment. When secure world
cold boot is complete, Trusted Firmware simply jumps to a BL33 base address
provided at build time.

For this option to be used, the ``PRELOADED_BL33_BASE`` build option has to be
used when compiling the Trusted Firmware. For example, the following command
will create a FIP without a BL33 and prepare to jump to a BL33 image loaded at
address 0x80000000:

::

    make PRELOADED_BL33_BASE=0x80000000 PLAT=fvp all fip

Boot of a preloaded bootwrapped kernel image on Base FVP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following example uses the AArch64 boot wrapper. This simplifies normal
world booting while also making use of TF features. It can be obtained from its
repository with:

::

    git clone git://git.kernel.org/pub/scm/linux/kernel/git/mark/boot-wrapper-aarch64.git

After compiling it, an ELF file is generated. It can be loaded with the
following command:

::

    <path-to>/FVP_Base_AEMv8A-AEMv8A              \
        -C bp.secureflashloader.fname=bl1.bin     \
        -C bp.flashloader0.fname=fip.bin          \
        -a cluster0.cpu0=<bootwrapped-kernel.elf> \
        --start cluster0.cpu0=0x0

The ``-a cluster0.cpu0=<bootwrapped-kernel.elf>`` option loads the ELF file. It
also sets the PC register to the ELF entry point address, which is not the
desired behaviour, so the ``--start cluster0.cpu0=0x0`` option forces the PC back
to 0x0 (the BL1 entry point address) on CPU #0. The ``PRELOADED_BL33_BASE`` define
used when compiling the FIP must match the ELF entry point.

Boot of a preloaded bootwrapped kernel image on Juno
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The procedure to obtain and compile the boot wrapper is very similar to the case
of the FVP. The execution must be stopped at the end of bl2\_main(), and the
loading method explained above in the EL3 payload boot flow section may be used
to load the ELF file over JTAG on Juno.

Running the software on FVP
---------------------------

The latest version of the AArch64 build of ARM Trusted Firmware has been tested
on the following ARM FVPs (64-bit host machine only).

NOTE: Unless otherwise stated, the model version is Version 11.2 Build 11.2.33.

-  ``Foundation_Platform``
-  ``FVP_Base_AEMv8A-AEMv8A`` (Version 9.0, Build 0.8.9005)
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``

The latest version of the AArch32 build of ARM Trusted Firmware has been tested
on the following ARM FVPs (64-bit host machine only).

-  ``FVP_Base_AEMv8A-AEMv8A`` (Version 9.0, Build 0.8.9005)
-  ``FVP_Base_Cortex-A32x4``

NOTE: The build numbers quoted above are those reported by launching the FVP
with the ``--version`` parameter.

NOTE: Linaro provides a ramdisk image in prebuilt FVP configurations and full
file systems that can be downloaded separately. To run an FVP with a virtio
file system image an additional FVP configuration option
``-C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>`` can be
used.

NOTE: The software will not work on Version 1.0 of the Foundation FVP.
The commands below would report an ``unhandled argument`` error in this case.

NOTE: FVPs can be launched with ``--cadi-server`` option such that a
CADI-compliant debugger (for example, ARM DS-5) can connect to and control its
execution.

NOTE: Since FVP model Version 11.0 Build 11.0.34 and Version 8.5 Build 0.8.5202
the internal synchronisation timings changed compared to older versions of the
models. The models can be launched with ``-Q 100`` option if they are required
to match the run time characteristics of the older versions.

The Foundation FVP is a cut down version of the AArch64 Base FVP. It can be
downloaded for free from `ARM's website`_.

The Cortex-A models listed above are also available to download from
`ARM's website`_.

Please refer to the FVP documentation for a detailed description of the model
parameter options. A brief description of the important ones that affect the ARM
Trusted Firmware and normal world software behavior is provided below.

Obtaining the Flattened Device Trees
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Depending on the FVP configuration and Linux configuration used, different
FDT files are required. FDTs for the Foundation and Base FVPs can be found in
the Trusted Firmware source directory under ``fdts/``. The Foundation FVP has a
subset of the Base FVP components. For example, the Foundation FVP lacks CLCD
and MMC support, and has only one CPU cluster.

Note: It is not recommended to use the FDTs built along the kernel because not
all FDTs are available from there.

-  ``fvp-base-gicv2-psci.dtb``

   For use with both AEMv8 and Cortex-A57-A53 Base FVPs with
   Base memory map configuration.

-  ``fvp-base-gicv2-psci-aarch32.dtb``

   For use with AEMv8 and Cortex-A32 Base FVPs running Linux in AArch32 state
   with Base memory map configuration.

-  ``fvp-base-gicv3-psci.dtb``

   (Default) For use with both AEMv8 and Cortex-A57-A53 Base FVPs with Base
   memory map configuration and Linux GICv3 support.

-  ``fvp-base-gicv3-psci-aarch32.dtb``

   For use with AEMv8 and Cortex-A32 Base FVPs running Linux in AArch32 state
   with Base memory map configuration and Linux GICv3 support.

-  ``fvp-foundation-gicv2-psci.dtb``

   For use with Foundation FVP with Base memory map configuration.

-  ``fvp-foundation-gicv3-psci.dtb``

   (Default) For use with Foundation FVP with Base memory map configuration
   and Linux GICv3 support.

Running on the Foundation FVP with reset to BL1 entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``Foundation_Platform`` parameters should be used to boot Linux with
4 CPUs using the AArch64 build of ARM Trusted Firmware.

::

    <path-to>/Foundation_Platform                   \
    --cores=4                                       \
    --secure-memory                                 \
    --visualization                                 \
    --gicv3                                         \
    --data="<path-to>/<bl1-binary>"@0x0             \
    --data="<path-to>/<FIP-binary>"@0x08000000      \
    --data="<path-to>/<fdt>"@0x82000000             \
    --data="<path-to>/<kernel-binary>"@0x80080000   \
    --data="<path-to>/<ramdisk-binary>"@0x84000000

Notes:

-  BL1 is loaded at the start of the Trusted ROM.
-  The Firmware Image Package is loaded at the start of NOR FLASH0.
-  The Linux kernel image and device tree are loaded in DRAM.
-  The default use-case for the Foundation FVP is to use the ``--gicv3`` option
   and enable the GICv3 device in the model. Note that without this option,
   the Foundation FVP defaults to legacy (Versatile Express) memory map which
   is not supported by ARM Trusted Firmware.

Running on the AEMv8 Base FVP with reset to BL1 entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

::

    <path-to>/FVP_Base_AEMv8A-AEMv8A                            \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cluster0.NUM_CORES=4                                     \
    -C cluster1.NUM_CORES=4                                     \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the AEMv8 Base FVP (AArch32) with reset to BL1 entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of ARM Trusted Firmware.

::

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
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the Cortex-A57-A53 Base FVP with reset to BL1 entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_Cortex-A57x4-A53x4`` model parameters should be used to
boot Linux with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

::

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                       \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the Cortex-A32 Base FVP (AArch32) with reset to BL1 entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_Cortex-A32x4`` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of ARM Trusted Firmware.

::

    <path-to>/FVP_Base_Cortex-A32x4                             \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the AEMv8 Base FVP with reset to BL31 entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

::

    <path-to>/FVP_Base_AEMv8A-AEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBAR=0x04020000                            \
    -C cluster0.cpu1.RVBAR=0x04020000                            \
    -C cluster0.cpu2.RVBAR=0x04020000                            \
    -C cluster0.cpu3.RVBAR=0x04020000                            \
    -C cluster1.cpu0.RVBAR=0x04020000                            \
    -C cluster1.cpu1.RVBAR=0x04020000                            \
    -C cluster1.cpu2.RVBAR=0x04020000                            \
    -C cluster1.cpu3.RVBAR=0x04020000                            \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04020000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Notes:

-  Since a FIP is not loaded when using BL31 as reset entrypoint, the
   ``--data="<path-to><bl31|bl32|bl33-binary>"@<base-address-of-binary>``
   parameter is needed to load the individual bootloader images in memory.
   BL32 image is only needed if BL31 has been built to expect a Secure-EL1
   Payload.

-  The ``-C cluster<X>.cpu<Y>.RVBAR=@<base-address-of-bl31>`` parameter, where
   X and Y are the cluster and CPU numbers respectively, is used to set the
   reset vector for each core.

-  Changing the default value of ``ARM_TSP_RAM_LOCATION`` will also require
   changing the value of
   ``--data="<path-to><bl32-binary>"@<base-address-of-bl32>`` to the new value of
   ``BL32_BASE``.

Running on the AEMv8 Base FVP (AArch32) with reset to SP\_MIN entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of ARM Trusted Firmware.

::

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
    -C cluster0.cpu0.RVBAR=0x04001000                            \
    -C cluster0.cpu1.RVBAR=0x04001000                            \
    -C cluster0.cpu2.RVBAR=0x04001000                            \
    -C cluster0.cpu3.RVBAR=0x04001000                            \
    -C cluster1.cpu0.RVBAR=0x04001000                            \
    -C cluster1.cpu1.RVBAR=0x04001000                            \
    -C cluster1.cpu2.RVBAR=0x04001000                            \
    -C cluster1.cpu3.RVBAR=0x04001000                            \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Note: The load address of ``<bl32-binary>`` depends on the value ``BL32_BASE``.
It should match the address programmed into the RVBAR register as well.

Running on the Cortex-A57-A53 Base FVP with reset to BL31 entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_Cortex-A57x4-A53x4`` model parameters should be used to
boot Linux with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

::

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                        \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBARADDR=0x04020000                        \
    -C cluster0.cpu1.RVBARADDR=0x04020000                        \
    -C cluster0.cpu2.RVBARADDR=0x04020000                        \
    -C cluster0.cpu3.RVBARADDR=0x04020000                        \
    -C cluster1.cpu0.RVBARADDR=0x04020000                        \
    -C cluster1.cpu1.RVBARADDR=0x04020000                        \
    -C cluster1.cpu2.RVBARADDR=0x04020000                        \
    -C cluster1.cpu3.RVBARADDR=0x04020000                        \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04020000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the Cortex-A32 Base FVP (AArch32) with reset to SP\_MIN entrypoint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following ``FVP_Base_Cortex-A32x4`` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of ARM Trusted Firmware.

::

    <path-to>/FVP_Base_Cortex-A32x4                             \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C cluster0.cpu0.RVBARADDR=0x04001000                       \
    -C cluster0.cpu1.RVBARADDR=0x04001000                       \
    -C cluster0.cpu2.RVBARADDR=0x04001000                       \
    -C cluster0.cpu3.RVBARADDR=0x04001000                       \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000   \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000   \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running the software on Juno
----------------------------

This version of the ARM Trusted Firmware has been tested on variants r0, r1 and
r2 of Juno.

To execute the software stack on Juno, the version of the Juno board recovery
image indicated in the `Linaro Release Notes`_ must be installed. If you have an
earlier version installed or are unsure which version is installed, please
re-install the recovery image by following the
`Instructions for using Linaro's deliverables on Juno`_.

Preparing Trusted Firmware images
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After building Trusted Firmware, the files ``bl1.bin`` and ``fip.bin`` need copying
to the ``SOFTWARE/`` directory of the Juno SD card.

Other Juno software information
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please visit the `ARM Platforms Portal`_ to get support and obtain any other Juno
software information. Please also refer to the `Juno Getting Started Guide`_ to
get more detailed information about the Juno ARM development platform and how to
configure it.

Testing SYSTEM SUSPEND on Juno
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The SYSTEM SUSPEND is a PSCI API which can be used to implement system suspend
to RAM. For more details refer to section 5.16 of `PSCI`_. To test system suspend
on Juno, at the linux shell prompt, issue the following command:

::

    echo +10 > /sys/class/rtc/rtc0/wakealarm
    echo -n mem > /sys/power/state

The Juno board should suspend to RAM and then wakeup after 10 seconds due to
wakeup interrupt from RTC.

--------------

*Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.*

.. _Linaro: `Linaro Release Notes`_
.. _Linaro Release: `Linaro Release Notes`_
.. _Linaro Release Notes: https://community.arm.com/dev-platforms/w/docs/226/old-linaro-release-notes
.. _Linaro Release 17.10: https://community.arm.com/dev-platforms/w/docs/226/old-linaro-release-notes#1710
.. _Linaro instructions: https://community.arm.com/dev-platforms/w/docs/304/linaro-software-deliverables
.. _Instructions for using Linaro's deliverables on Juno: https://community.arm.com/dev-platforms/w/docs/303/juno
.. _ARM Platforms Portal: https://community.arm.com/dev-platforms/
.. _Development Studio 5 (DS-5): http://www.arm.com/products/tools/software-tools/ds-5/index.php
.. _Dia: https://wiki.gnome.org/Apps/Dia/Download
.. _here: psci-lib-integration-guide.rst
.. _Trusted Board Boot: trusted-board-boot.rst
.. _Secure-EL1 Payloads and Dispatchers: firmware-design.rst#user-content-secure-el1-payloads-and-dispatchers
.. _Firmware Update: firmware-update.rst
.. _Firmware Design: firmware-design.rst
.. _mbed TLS Repository: https://github.com/ARMmbed/mbedtls.git
.. _mbed TLS Security Center: https://tls.mbed.org/security
.. _ARM's website: `FVP models`_
.. _FVP models: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Juno Getting Started Guide: http://infocenter.arm.com/help/topic/com.arm.doc.dui0928e/DUI0928E_juno_arm_development_platform_gsg.pdf
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
