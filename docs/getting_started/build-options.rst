Build Options
=============

The TF-A build system supports the following build options. Unless mentioned
otherwise, these options are expected to be specified at the build command
line and are not to be modified in any component makefiles. Note that the
build system doesn't track dependency for build options. Therefore, if any of
the build options are changed from a previous build, a clean build must be
performed.

.. _build_options_common:

Common build options
--------------------

-  ``AARCH32_INSTRUCTION_SET``: Choose the AArch32 instruction set that the
   compiler should use. Valid values are T32 and A32. It defaults to T32 due to
   code having a smaller resulting size.

-  ``AARCH32_SP`` : Choose the AArch32 Secure Payload component to be built as
   as the BL32 image when ``ARCH=aarch32``. The value should be the path to the
   directory containing the SP source, relative to the ``bl32/``; the directory
   is expected to contain a makefile called ``<aarch32_sp-value>.mk``.

-  ``AMU_RESTRICT_COUNTERS``: Register reads to the group 1 counters will return
   zero at all but the highest implemented exception level.  Reads from the
   memory mapped view are unaffected by this control.

-  ``ARCH`` : Choose the target build architecture for TF-A. It can take either
   ``aarch64`` or ``aarch32`` as values. By default, it is defined to
   ``aarch64``.

-  ``ARM_ARCH_FEATURE``: Optional Arm Architecture build option which specifies
   one or more feature modifiers. This option has the form ``[no]feature+...``
   and defaults to ``none``. It translates into compiler option
   ``-march=armvX[.Y]-a+[no]feature+...``. See compiler's documentation for the
   list of supported feature modifiers.

-  ``ARM_ARCH_MAJOR``: The major version of Arm Architecture to target when
   compiling TF-A. Its value must be numeric, and defaults to 8 . See also,
   *Armv8 Architecture Extensions* and *Armv7 Architecture Extensions* in
   :ref:`Firmware Design`.

-  ``ARM_ARCH_MINOR``: The minor version of Arm Architecture to target when
   compiling TF-A. Its value must be a numeric, and defaults to 0. See also,
   *Armv8 Architecture Extensions* in :ref:`Firmware Design`.

-  ``BL2``: This is an optional build option which specifies the path to BL2
   image for the ``fip`` target. In this case, the BL2 in the TF-A will not be
   built.

-  ``BL2U``: This is an optional build option which specifies the path to
   BL2U image. In this case, the BL2U in TF-A will not be built.

-  ``BL2_AT_EL3``: This is an optional build option that enables the use of
   BL2 at EL3 execution level.

-  ``BL2_ENABLE_SP_LOAD``: Boolean option to enable loading SP packages from the
   FIP. Automatically enabled if ``SP_LAYOUT_FILE`` is provided.

-  ``BL2_IN_XIP_MEM``: In some use-cases BL2 will be stored in eXecute In Place
   (XIP) memory, like BL1. In these use-cases, it is necessary to initialize
   the RW sections in RAM, while leaving the RO sections in place. This option
   enable this use-case. For now, this option is only supported when BL2_AT_EL3
   is set to '1'.

-  ``BL31``: This is an optional build option which specifies the path to
   BL31 image for the ``fip`` target. In this case, the BL31 in TF-A will not
   be built.

-  ``BL31_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL31 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BL32``: This is an optional build option which specifies the path to
   BL32 image for the ``fip`` target. In this case, the BL32 in TF-A will not
   be built.

-  ``BL32_EXTRA1``: This is an optional build option which specifies the path to
   Trusted OS Extra1 image for the  ``fip`` target.

-  ``BL32_EXTRA2``: This is an optional build option which specifies the path to
   Trusted OS Extra2 image for the ``fip`` target.

-  ``BL32_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL32 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BL33``: Path to BL33 image in the host file system. This is mandatory for
   ``fip`` target in case TF-A BL2 is used.

-  ``BL33_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL33 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BRANCH_PROTECTION``: Numeric value to enable ARMv8.3 Pointer Authentication
   and ARMv8.5 Branch Target Identification support for TF-A BL images themselves.
   If enabled, it is needed to use a compiler that supports the option
   ``-mbranch-protection``. Selects the branch protection features to use:
-  0: Default value turns off all types of branch protection
-  1: Enables all types of branch protection features
-  2: Return address signing to its standard level
-  3: Extend the signing to include leaf functions
-  4: Turn on branch target identification mechanism

   The table below summarizes ``BRANCH_PROTECTION`` values, GCC compilation options
   and resulting PAuth/BTI features.

   +-------+--------------+-------+-----+
   | Value |  GCC option  | PAuth | BTI |
   +=======+==============+=======+=====+
   |   0   |     none     |   N   |  N  |
   +-------+--------------+-------+-----+
   |   1   |   standard   |   Y   |  Y  |
   +-------+--------------+-------+-----+
   |   2   |   pac-ret    |   Y   |  N  |
   +-------+--------------+-------+-----+
   |   3   | pac-ret+leaf |   Y   |  N  |
   +-------+--------------+-------+-----+
   |   4   |     bti      |   N   |  Y  |
   +-------+--------------+-------+-----+

   This option defaults to 0.
   Note that Pointer Authentication is enabled for Non-secure world
   irrespective of the value of this option if the CPU supports it.

-  ``BUILD_MESSAGE_TIMESTAMP``: String used to identify the time and date of the
   compilation of each build. It must be set to a C string (including quotes
   where applicable). Defaults to a string that contains the time and date of
   the compilation.

-  ``BUILD_STRING``: Input string for VERSION_STRING, which allows the TF-A
   build to be uniquely identified. Defaults to the current git commit id.

-  ``BUILD_BASE``: Output directory for the build. Defaults to ``./build``

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

-  ``COT``: When Trusted Boot is enabled, selects the desired chain of trust.
   Defaults to ``tbbr``.

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

-  ``CTX_INCLUDE_EL2_REGS`` : This boolean option provides context save/restore
   operations when entering/exiting an EL2 execution context. This is of primary
   interest when Armv8.4-SecEL2 extension is implemented. Default is 0 (disabled).
   This option must be equal to 1 (enabled) when ``SPD=spmd`` and
   ``SPMD_SPM_AT_SEL2`` is set.

-  ``CTX_INCLUDE_FPREGS``: Boolean option that, when set to 1, will cause the FP
   registers to be included when saving and restoring the CPU context. Default
   is 0.

-  ``CTX_INCLUDE_MTE_REGS``: Numeric value to include Memory Tagging Extension
   registers in cpu context. This must be enabled, if the platform wants to use
   this feature in the Secure world and MTE is enabled at ELX. This flag can
   take values 0 to 2, to align with the ``FEATURE_DETECTION`` mechanism.
   Default value is 0.

-  ``CTX_INCLUDE_NEVE_REGS``: Numeric value, when set will cause the Armv8.4-NV
   registers to be saved/restored when entering/exiting an EL2 execution
   context. This flag can take values 0 to 2, to align with the
   ``FEATURE_DETECTION`` mechanism. Default value is 0.

-  ``CTX_INCLUDE_PAUTH_REGS``: Numeric value to enable the Pointer
   Authentication for Secure world. This will cause the ARMv8.3-PAuth registers
   to be included when saving and restoring the CPU context as part of world
   switch. This flag can take values 0 to 2, to align with ``FEATURE_DETECTION``
   mechanism. Default value is 0.

   Note that Pointer Authentication is enabled for Non-secure world irrespective
   of the value of this flag if the CPU supports it.

-  ``DEBUG``: Chooses between a debug and release build. It can take either 0
   (release) or 1 (debug) as values. 0 is the default.

-  ``DECRYPTION_SUPPORT``: This build flag enables the user to select the
   authenticated decryption algorithm to be used to decrypt firmware/s during
   boot. It accepts 2 values: ``aes_gcm`` and ``none``. The default value of
   this flag is ``none`` to disable firmware decryption which is an optional
   feature as per TBBR.

-  ``DISABLE_BIN_GENERATION``: Boolean option to disable the generation
   of the binary image. If set to 1, then only the ELF image is built.
   0 is the default.

-  ``DISABLE_MTPMU``: Boolean option to disable FEAT_MTPMU if implemented
   (Armv8.6 onwards). Its default value is 0 to keep consistency with platforms
   that do not implement FEAT_MTPMU. For more information on FEAT_MTPMU,
   check the latest Arm ARM.

-  ``DYN_DISABLE_AUTH``: Provides the capability to dynamically disable Trusted
   Board Boot authentication at runtime. This option is meant to be enabled only
   for development platforms. ``TRUSTED_BOARD_BOOT`` flag must be set if this
   flag has to be enabled. 0 is the default.

-  ``E``: Boolean option to make warnings into errors. Default is 1.

-  ``EL3_PAYLOAD_BASE``: This option enables booting an EL3 payload instead of
   the normal boot flow. It must specify the entry point address of the EL3
   payload. Please refer to the "Booting an EL3 payload" section for more
   details.

-  ``ENABLE_AMU``: Boolean option to enable Activity Monitor Unit extensions.
   This is an optional architectural feature available on v8.4 onwards. Some
   v8.2 implementations also implement an AMU and this option can be used to
   enable this feature on those systems as well. Default is 0.

-  ``ENABLE_AMU_AUXILIARY_COUNTERS``: Enables support for AMU auxiliary counters
   (also known as group 1 counters). These are implementation-defined counters,
   and as such require additional platform configuration. Default is 0.

-  ``ENABLE_AMU_FCONF``: Enables configuration of the AMU through FCONF, which
   allows platforms with auxiliary counters to describe them via the
   ``HW_CONFIG`` device tree blob. Default is 0.

-  ``ENABLE_ASSERTIONS``: This option controls whether or not calls to ``assert()``
   are compiled out. For debug builds, this option defaults to 1, and calls to
   ``assert()`` are left in place. For release builds, this option defaults to 0
   and calls to ``assert()`` function are compiled out. This option can be set
   independently of ``DEBUG``. It can also be used to hide any auxiliary code
   that is only required for the assertion and does not fit in the assertion
   itself.

-  ``ENABLE_BACKTRACE``: This option controls whether to enable backtrace
   dumps or not. It is supported in both AArch64 and AArch32. However, in
   AArch32 the format of the frame records are not defined in the AAPCS and they
   are defined by the implementation. This implementation of backtrace only
   supports the format used by GCC when T32 interworking is disabled. For this
   reason enabling this option in AArch32 will force the compiler to only
   generate A32 code. This option is enabled by default only in AArch64 debug
   builds, but this behaviour can be overridden in each platform's Makefile or
   in the build command line.

-  ``ENABLE_FEAT_AMUv1``: Numeric value to enable access to the HAFGRTR_EL2
   (Hypervisor Activity Monitors Fine-Grained Read Trap Register) during EL2
   to EL3 context save/restore operations. This flag can take the values 0 to 2,
   to align with the ``FEATURE_DETECTION`` mechanism. It is an optional feature
   available on v8.4 and onwards and must be set to either 1 or 2 alongside
   ``ENABLE_FEAT_FGT``, to access the HAFGRTR_EL2 register.
   Default value is ``0``.

-  ``ENABLE_FEAT_AMUv1p1``: Numeric value to enable the ``FEAT_AMUv1p1``
   extension. ``FEAT_AMUv1p1`` is an optional feature available on Arm v8.6
   onwards. This flag can take the values 0 to 2, to align with the
   ``FEATURE_DETECTION`` mechanism. Default value is ``0``.

-  ``ENABLE_FEAT_CSV2_2``: Numeric value to enable the ``FEAT_CSV2_2``
   extension. It allows access to the SCXTNUM_EL2 (Software Context Number)
   register during EL2 context save/restore operations. ``FEAT_CSV2_2`` is an
   optional feature available on Arm v8.0 onwards. This flag can take values
   0 to 2, to align with the ``FEATURE_DETECTION`` mechanism.
   Default value is ``0``.

-  ``ENABLE_FEAT_DIT``: Numeric value to enable ``FEAT_DIT`` (Data Independent
   Timing) extension. It allows setting the ``DIT`` bit of PSTATE in EL3.
   ``FEAT_DIT`` is a mandatory  architectural feature and is enabled from v8.4
   and upwards. This flag can take the values 0 to 2, to align  with the
   ``FEATURE_DETECTION`` mechanism. Default value is ``0``.

-  ``ENABLE_FEAT_ECV``: Numeric value to enable support for the Enhanced Counter
   Virtualization feature, allowing for access to the CNTPOFF_EL2 (Counter-timer
   Physical Offset register) during EL2 to EL3 context save/restore operations.
   Its a mandatory architectural feature and is enabled from v8.6 and upwards.
   This flag can take the values 0 to 2, to align  with the ``FEATURE_DETECTION``
   mechanism. Default value is ``0``.

-  ``ENABLE_FEAT_FGT``: Numeric value to enable support for FGT (Fine Grain Traps)
   feature allowing for access to the HDFGRTR_EL2 (Hypervisor Debug Fine-Grained
   Read Trap Register) during EL2 to EL3 context save/restore operations.
   Its a mandatory architectural feature and is enabled from v8.6 and upwards.
   This flag can take the values 0 to 2, to align  with the ``FEATURE_DETECTION``
   mechanism. Default value is ``0``.

-  ``ENABLE_FEAT_HCX``: Numeric value to set the bit SCR_EL3.HXEn in EL3 to
   allow access to HCRX_EL2 (extended hypervisor control register) from EL2 as
   well as adding HCRX_EL2 to the EL2 context save/restore operations. Its a
   mandatory architectural feature and is enabled from v8.7 and upwards. This
   flag can take the values 0 to 2, to align  with the ``FEATURE_DETECTION``
   mechanism. Default value is ``0``.

-  ``ENABLE_FEAT_PAN``: Numeric value to enable the ``FEAT_PAN`` (Privileged
   Access Never) extension. ``FEAT_PAN`` adds a bit to PSTATE, generating a
   permission fault for any privileged data access from EL1/EL2 to virtual
   memory address, accessible at EL0, provided (HCR_EL2.E2H=1). It is a
   mandatory architectural feature and is enabled from v8.1 and upwards. This
   flag can take values 0 to 2, to align  with the ``FEATURE_DETECTION``
   mechanism. Default value is ``0``.

-  ``ENABLE_FEAT_RNG``: Numeric value to enable the ``FEAT_RNG`` extension.
   ``FEAT_RNG`` is an optional feature available on Arm v8.5 onwards. This
   flag can take the values 0 to 2, to align with the ``FEATURE_DETECTION``
   mechanism. Default is ``0``.

-  ``ENABLE_FEAT_SB``: Numeric value to enable the ``FEAT_SB`` (Speculation
   Barrier) extension allowing access to ``sb`` instruction. ``FEAT_SB`` is an
   optional feature and defaults to ``0`` for pre-Armv8.5 CPUs but are mandatory
   for Armv8.5 or later CPUs. This flag can take values 0 to 2, to align with
   ``FEATURE_DETECTION`` mechanism. It is enabled from v8.5 and upwards and if
   needed could be overidden from platforms explicitly. Default value is ``0``.

-  ``ENABLE_FEAT_SEL2``: Numeric value to enable the ``FEAT_SEL2`` (Secure EL2)
   extension. ``FEAT_SEL2`` is a mandatory feature available on Arm v8.4.
   This flag can take values 0 to 2, to align with the ``FEATURE_DETECTION``
   mechanism. Default is ``0``.

-  ``ENABLE_FEAT_TWED``: Numeric value to enable the ``FEAT_TWED`` (Delayed
   trapping of WFE Instruction) extension. ``FEAT_TWED`` is a optional feature
   available on Arm v8.6. This flag can take values 0 to 2, to align with the
   ``FEATURE_DETECTION`` mechanism. Default is ``0``.

    When ``ENABLE_FEAT_TWED`` is set to ``1``, WFE instruction trapping gets
    delayed by the amount of value in ``TWED_DELAY``.

-  ``ENABLE_FEAT_VHE``: Numeric value to enable the ``FEAT_VHE`` (Virtualization
   Host Extensions) extension. It allows access to CONTEXTIDR_EL2 register
   during EL2 context save/restore operations.``FEAT_VHE`` is a mandatory
   architectural feature and is enabled from v8.1 and upwards. It can take
   values 0 to 2, to align  with the ``FEATURE_DETECTION`` mechanism.
   Default value is ``0``.

-  ``ENABLE_LTO``: Boolean option to enable Link Time Optimization (LTO)
   support in GCC for TF-A. This option is currently only supported for
   AArch64. Default is 0.

-  ``ENABLE_MPAM_FOR_LOWER_ELS``: Numeric value to enable lower ELs to use MPAM
   feature. MPAM is an optional Armv8.4 extension that enables various memory
   system components and resources to define partitions; software running at
   various ELs can assign themselves to desired partition to control their
   performance aspects.

   This flag can take values 0 to 2, to align  with the ``FEATURE_DETECTION``
   mechanism. When this option is set to ``1`` or ``2``, EL3 allows lower ELs to
   access their own MPAM registers without trapping into EL3. This option
   doesn't make use of partitioning in EL3, however. Platform initialisation
   code should configure and use partitions in EL3 as required. This option
   defaults to ``0``.

-  ``ENABLE_MPMM``: Boolean option to enable support for the Maximum Power
   Mitigation Mechanism supported by certain Arm cores, which allows the SoC
   firmware to detect and limit high activity events to assist in SoC processor
   power domain dynamic power budgeting and limit the triggering of whole-rail
   (i.e. clock chopping) responses to overcurrent conditions. Defaults to ``0``.

-  ``ENABLE_MPMM_FCONF``: Enables configuration of MPMM through FCONF, which
   allows platforms with cores supporting MPMM to describe them via the
   ``HW_CONFIG`` device tree blob. Default is 0.

-  ``ENABLE_PIE``: Boolean option to enable Position Independent Executable(PIE)
   support within generic code in TF-A. This option is currently only supported
   in BL2_AT_EL3, BL31, and BL32 (TSP) for AARCH64 binaries, and in BL32
   (SP_min) for AARCH32. Default is 0.

-  ``ENABLE_PMF``: Boolean option to enable support for optional Performance
   Measurement Framework(PMF). Default is 0.

-  ``ENABLE_PSCI_STAT``: Boolean option to enable support for optional PSCI
   functions ``PSCI_STAT_RESIDENCY`` and ``PSCI_STAT_COUNT``. Default is 0.
   In the absence of an alternate stat collection backend, ``ENABLE_PMF`` must
   be enabled. If ``ENABLE_PMF`` is set, the residency statistics are tracked in
   software.

- ``ENABLE_RME``: Numeric value to enable support for the ARMv9 Realm
   Management Extension. This flag can take the values 0 to 2, to align with
   the ``FEATURE_DETECTION`` mechanism. Default value is 0. This is currently
   an experimental feature.

-  ``ENABLE_RUNTIME_INSTRUMENTATION``: Boolean option to enable runtime
   instrumentation which injects timestamp collection points into TF-A to
   allow runtime performance to be measured. Currently, only PSCI is
   instrumented. Enabling this option enables the ``ENABLE_PMF`` build option
   as well. Default is 0.

-  ``ENABLE_SME_FOR_NS``: Boolean option to enable Scalable Matrix Extension
   (SME), SVE, and FPU/SIMD for the non-secure world only. These features share
   registers so are enabled together. Using this option without
   ENABLE_SME_FOR_SWD=1 will cause SME, SVE, and FPU/SIMD instructions in secure
   world to trap to EL3. SME is an optional architectural feature for AArch64
   and TF-A support is experimental. At this time, this build option cannot be
   used on systems that have SPD=spmd/SPM_MM or ENABLE_RME, and attempting to
   build with these options will fail. Default is 0.

-  ``ENABLE_SME_FOR_SWD``: Boolean option to enable the Scalable Matrix
   Extension for secure world use along with SVE and FPU/SIMD, ENABLE_SME_FOR_NS
   must also be set to use this. If enabling this, the secure world MUST
   handle context switching for SME, SVE, and FPU/SIMD registers to ensure that
   no data is leaked to non-secure world. This is experimental. Default is 0.

-  ``ENABLE_SPE_FOR_LOWER_ELS`` : Boolean option to enable Statistical Profiling
   extensions. This is an optional architectural feature for AArch64.
   The default is 1 but is automatically disabled when the target architecture
   is AArch32.

-  ``ENABLE_SVE_FOR_NS``: Boolean option to enable Scalable Vector Extension
   (SVE) for the Non-secure world only. SVE is an optional architectural feature
   for AArch64. Note that when SVE is enabled for the Non-secure world, access
   to SIMD and floating-point functionality from the Secure world is disabled by
   default and controlled with ENABLE_SVE_FOR_SWD.
   This is to avoid corruption of the Non-secure world data in the Z-registers
   which are aliased by the SIMD and FP registers. The build option is not
   compatible with the ``CTX_INCLUDE_FPREGS`` build option, and will raise an
   assert on platforms where SVE is implemented and ``ENABLE_SVE_FOR_NS`` set to
   1. The default is 1 but is automatically disabled when ENABLE_SME_FOR_NS=1
   since SME encompasses SVE. At this time, this build option cannot be used on
   systems that have SPM_MM enabled.

-  ``ENABLE_SVE_FOR_SWD``: Boolean option to enable SVE for the Secure world.
   SVE is an optional architectural feature for AArch64. Note that this option
   requires ENABLE_SVE_FOR_NS to be enabled. The default is 0 and it
   is automatically disabled when the target architecture is AArch32.

-  ``ENABLE_STACK_PROTECTOR``: String option to enable the stack protection
   checks in GCC. Allowed values are "all", "strong", "default" and "none". The
   default value is set to "none". "strong" is the recommended stack protection
   level if this feature is desired. "none" disables the stack protection. For
   all values other than "none", the ``plat_get_stack_protector_canary()``
   platform hook needs to be implemented. The value is passed as the last
   component of the option ``-fstack-protector-$ENABLE_STACK_PROTECTOR``.

-  ``ENCRYPT_BL31``: Binary flag to enable encryption of BL31 firmware. This
   flag depends on ``DECRYPTION_SUPPORT`` build flag.

-  ``ENCRYPT_BL32``: Binary flag to enable encryption of Secure BL32 payload.
   This flag depends on ``DECRYPTION_SUPPORT`` build flag.

-  ``ENC_KEY``: A 32-byte (256-bit) symmetric key in hex string format. It could
   either be SSK or BSSK depending on ``FW_ENC_STATUS`` flag. This value depends
   on ``DECRYPTION_SUPPORT`` build flag.

-  ``ENC_NONCE``: A 12-byte (96-bit) encryption nonce or Initialization Vector
   (IV) in hex string format. This value depends on ``DECRYPTION_SUPPORT``
   build flag.

-  ``ERROR_DEPRECATED``: This option decides whether to treat the usage of
   deprecated platform APIs, helper functions or drivers within Trusted
   Firmware as error. It can take the value 1 (flag the use of deprecated
   APIs as error) or 0. The default is 0.

-  ``EL3_EXCEPTION_HANDLING``: When set to ``1``, enable handling of exceptions
   targeted at EL3. When set ``0`` (default), no exceptions are expected or
   handled at EL3, and a panic will result. This is supported only for AArch64
   builds.

-  ``EVENT_LOG_LEVEL``: Chooses the log level to use for Measured Boot when
   ``MEASURED_BOOT`` is enabled. For a list of valid values, see ``LOG_LEVEL``.
   Default value is 40 (LOG_LEVEL_INFO).

-  ``FAULT_INJECTION_SUPPORT``: ARMv8.4 extensions introduced support for fault
   injection from lower ELs, and this build option enables lower ELs to use
   Error Records accessed via System Registers to inject faults. This is
   applicable only to AArch64 builds.

   This feature is intended for testing purposes only, and is advisable to keep
   disabled for production images.

-  ``FEATURE_DETECTION``: Boolean option to enable the architectural features
   detection mechanism. It detects whether the Architectural features enabled
   through feature specific build flags are supported by the PE or not by
   validating them either at boot phase or at runtime based on the value
   possessed by the feature flag (0 to 2) and report error messages at an early
   stage.

   This prevents and benefits us from EL3 runtime exceptions during context save
   and restore routines guarded by these build flags. Henceforth validating them
   before their usage provides more control on the actions taken under them.

   The mechanism permits the build flags to take values 0, 1 or 2 and
   evaluates them accordingly.

   Lets consider ``ENABLE_FEAT_HCX``, build flag for ``FEAT_HCX`` as an example:

   ::

     ENABLE_FEAT_HCX = 0: Feature disabled statically at compile time.
     ENABLE_FEAT_HCX = 1: Feature Enabled and the flag is validated at boottime.
     ENABLE_FEAT_HCX = 2: Feature Enabled and the flag is validated at runtime.

   In the above example, if the feature build flag, ``ENABLE_FEAT_HCX`` set to
   0, feature is disabled statically during compilation. If it is defined as 1,
   feature is validated, wherein FEAT_HCX is detected at boot time. In case not
   implemented by the PE, a hard panic is generated. Finally, if the flag is set
   to 2, feature is validated at runtime.

   Note that the entire implementation is divided into two phases, wherein as
   as part of phase-1 we are supporting the values 0,1. Value 2 is currently not
   supported and is planned to be handled explicilty in phase-2 implementation.

   FEATURE_DETECTION macro is disabled by default, and is currently an
   experimental procedure. Platforms can explicitly make use of this by
   mechanism, by enabling it to validate whether they have set their build flags
   properly at an early phase.

-  ``FIP_NAME``: This is an optional build option which specifies the FIP
   filename for the ``fip`` target. Default is ``fip.bin``.

-  ``FWU_FIP_NAME``: This is an optional build option which specifies the FWU
   FIP filename for the ``fwu_fip`` target. Default is ``fwu_fip.bin``.

-  ``FW_ENC_STATUS``: Top level firmware's encryption numeric flag, values:

   ::

     0: Encryption is done with Secret Symmetric Key (SSK) which is common
        for a class of devices.
     1: Encryption is done with Binding Secret Symmetric Key (BSSK) which is
        unique per device.

   This flag depends on ``DECRYPTION_SUPPORT`` build flag.

-  ``GENERATE_COT``: Boolean flag used to build and execute the ``cert_create``
   tool to create certificates as per the Chain of Trust described in
   :ref:`Trusted Board Boot`. The build system then calls ``fiptool`` to
   include the certificates in the FIP and FWU_FIP. Default value is '0'.

   Specify both ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=1`` to include support
   for the Trusted Board Boot feature in the BL1 and BL2 images, to generate
   the corresponding certificates, and to include those certificates in the
   FIP and FWU_FIP.

   Note that if ``TRUSTED_BOARD_BOOT=0`` and ``GENERATE_COT=1``, the BL1 and BL2
   images will not include support for Trusted Board Boot. The FIP will still
   include the corresponding certificates. This FIP can be used to verify the
   Chain of Trust on the host machine through other mechanisms.

   Note that if ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=0``, the BL1 and BL2
   images will include support for Trusted Board Boot, but the FIP and FWU_FIP
   will not include the corresponding certificates, causing a boot failure.

-  ``GICV2_G0_FOR_EL3``: Unlike GICv3, the GICv2 architecture doesn't have
   inherent support for specific EL3 type interrupts. Setting this build option
   to ``1`` assumes GICv2 *Group 0* interrupts are expected to target EL3, both
   by :ref:`platform abstraction layer<platform Interrupt Controller API>` and
   :ref:`Interrupt Management Framework<Interrupt Management Framework>`.
   This allows GICv2 platforms to enable features requiring EL3 interrupt type.
   This also means that all GICv2 Group 0 interrupts are delivered to EL3, and
   the Secure Payload interrupts needs to be synchronously handed over to Secure
   EL1 for handling. The default value of this option is ``0``, which means the
   Group 0 interrupts are assumed to be handled by Secure EL1.

-  ``HANDLE_EA_EL3_FIRST``: When set to ``1``, External Aborts and SError
   Interrupts will be always trapped in EL3 i.e. in BL31 at runtime. When set to
   ``0`` (default), these exceptions will be trapped in the current exception
   level (or in EL1 if the current exception level is EL0).

-  ``HW_ASSISTED_COHERENCY``: On most Arm systems to-date, platform-specific
   software operations are required for CPUs to enter and exit coherency.
   However, newer systems exist where CPUs' entry to and exit from coherency
   is managed in hardware. Such systems require software to only initiate these
   operations, and the rest is managed in hardware, minimizing active software
   management. In such systems, this boolean option enables TF-A to carry out
   build and run-time optimizations during boot and power management operations.
   This option defaults to 0 and if it is enabled, then it implies
   ``WARMBOOT_ENABLE_DCACHE_EARLY`` is also enabled.

   If this flag is disabled while the platform which TF-A is compiled for
   includes cores that manage coherency in hardware, then a compilation error is
   generated. This is based on the fact that a system cannot have, at the same
   time, cores that manage coherency in hardware and cores that don't. In other
   words, a platform cannot have, at the same time, cores that require
   ``HW_ASSISTED_COHERENCY=1`` and cores that require
   ``HW_ASSISTED_COHERENCY=0``.

   Note that, when ``HW_ASSISTED_COHERENCY`` is enabled, version 2 of
   translation library (xlat tables v2) must be used; version 1 of translation
   library is not supported.

-  ``INVERTED_MEMMAP``: memmap tool print by default lower addresses at the
   bottom, higher addresses at the top. This build flag can be set to '1' to
   invert this behavior. Lower addresses will be printed at the top and higher
   addresses at the bottom.

-  ``JUNO_AARCH32_EL3_RUNTIME``: This build flag enables you to execute EL3
   runtime software in AArch32 mode, which is required to run AArch32 on Juno.
   By default this flag is set to '0'. Enabling this flag builds BL1 and BL2 in
   AArch64 and facilitates the loading of ``SP_MIN`` and BL33 as AArch32 executable
   images.

-  ``KEY_ALG``: This build flag enables the user to select the algorithm to be
   used for generating the PKCS keys and subsequent signing of the certificate.
   It accepts 3 values: ``rsa``, ``rsa_1_5`` and ``ecdsa``. The option
   ``rsa_1_5`` is the legacy PKCS#1 RSA 1.5 algorithm which is not TBBR
   compliant and is retained only for compatibility. The default value of this
   flag is ``rsa`` which is the TBBR compliant PKCS#1 RSA 2.1 scheme.

-  ``KEY_SIZE``: This build flag enables the user to select the key size for
   the algorithm specified by ``KEY_ALG``. The valid values for ``KEY_SIZE``
   depend on the chosen algorithm and the cryptographic module.

   +-----------+------------------------------------+
   |  KEY_ALG  |        Possible key sizes          |
   +===========+====================================+
   |    rsa    | 1024 , 2048 (default), 3072, 4096* |
   +-----------+------------------------------------+
   |   ecdsa   |            unavailable             |
   +-----------+------------------------------------+

   * Only 2048 bits size is available with CryptoCell 712 SBROM release 1.
     Only 3072 bits size is available with CryptoCell 712 SBROM release 2.

-  ``HASH_ALG``: This build flag enables the user to select the secure hash
   algorithm. It accepts 3 values: ``sha256``, ``sha384`` and ``sha512``.
   The default value of this flag is ``sha256``.

-  ``LDFLAGS``: Extra user options appended to the linkers' command line in
   addition to the one set by the build system.

-  ``LOG_LEVEL``: Chooses the log level, which controls the amount of console log
   output compiled into the build. This should be one of the following:

   ::

       0  (LOG_LEVEL_NONE)
       10 (LOG_LEVEL_ERROR)
       20 (LOG_LEVEL_NOTICE)
       30 (LOG_LEVEL_WARNING)
       40 (LOG_LEVEL_INFO)
       50 (LOG_LEVEL_VERBOSE)

   All log output up to and including the selected log level is compiled into
   the build. The default value is 40 in debug builds and 20 in release builds.

-  ``MEASURED_BOOT``: Boolean flag to include support for the Measured Boot
   feature. This flag can be enabled with ``TRUSTED_BOARD_BOOT`` in order to
   provide trust that the code taking the measurements and recording them has
   not been tampered with.

   This option defaults to 0.

-  ``DRTM_SUPPORT``: Boolean flag to enable support for Dynamic Root of Trust
   for Measurement (DRTM). This feature has trust dependency on BL31 for taking
   the measurements and recording them as per `PSA DRTM specification`_. For
   platforms which use BL2 to load/authenticate BL31 ``TRUSTED_BOARD_BOOT`` can
   be used and for the platforms which use ``RESET_TO_BL31`` platform owners
   should have mechanism to authenticate BL31.

   This option defaults to 0.

-  ``NON_TRUSTED_WORLD_KEY``: This option is used when ``GENERATE_COT=1``. It
   specifies the file that contains the Non-Trusted World private key in PEM
   format. If ``SAVE_KEYS=1``, this file name will be used to save the key.

-  ``NS_BL2U``: Path to NS_BL2U image in the host file system. This image is
   optional. It is only needed if the platform makefile specifies that it
   is required in order to build the ``fwu_fip`` target.

-  ``NS_TIMER_SWITCH``: Enable save and restore for non-secure timer register
   contents upon world switch. It can take either 0 (don't save and restore) or
   1 (do save and restore). 0 is the default. An SPD may set this to 1 if it
   wants the timer registers to be saved and restored.

-  ``OVERRIDE_LIBC``: This option allows platforms to override the default libc
   for the BL image. It can be either 0 (include) or 1 (remove). The default
   value is 0.

-  ``PL011_GENERIC_UART``: Boolean option to indicate the PL011 driver that
   the underlying hardware is not a full PL011 UART but a minimally compliant
   generic UART, which is a subset of the PL011. The driver will not access
   any register that is not part of the SBSA generic UART specification.
   Default value is 0 (a full PL011 compliant UART is present).

-  ``PLAT``: Choose a platform to build TF-A for. The chosen platform name
   must be subdirectory of any depth under ``plat/``, and must contain a
   platform makefile named ``platform.mk``. For example, to build TF-A for the
   Arm Juno board, select PLAT=juno.

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
   possible for the PSCI power-state parameter: original and extended State-ID
   formats. This flag if set to 1, configures the generic PSCI layer to use the
   extended format. The default value of this flag is 0, which means by default
   the original power-state format is used by the PSCI implementation. This flag
   should be specified by the platform makefile and it governs the return value
   of PSCI_FEATURES API for CPU_SUSPEND smc function id. When this option is
   enabled on Arm platforms, the option ``ARM_RECOM_STATE_ID_ENC`` needs to be
   set to 1 as well.

-  ``RAS_EXTENSION``: Numeric value to enable Armv8.2 RAS features. RAS features
   are an optional extension for pre-Armv8.2 CPUs, but are mandatory for Armv8.2
   or later CPUs. This flag can take the values 0 to 2, to align with the
   ``FEATURE_DETECTION`` mechanism.

   When ``RAS_EXTENSION`` is set to ``1``, ``HANDLE_EA_EL3_FIRST`` must also be
   set to ``1``.

   This option is disabled by default.

-  ``RESET_TO_BL31``: Enable BL31 entrypoint as the CPU reset vector instead
   of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
   entrypoint) or 1 (CPU reset to BL31 entrypoint).
   The default value is 0.

-  ``RESET_TO_BL31_WITH_PARAMS``: If ``RESET_TO_BL31`` has been enabled, setting
   this additional option guarantees that the input registers are not cleared
   therefore allowing parameters to be passed to the BL31 entrypoint.
   The default value is 0.

-  ``RESET_TO_SP_MIN``: SP_MIN is the minimal AArch32 Secure Payload provided
   in TF-A. This flag configures SP_MIN entrypoint as the CPU reset vector
   instead of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
   entrypoint) or 1 (CPU reset to SP_MIN entrypoint). The default value is 0.

-  ``ROT_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the ROT private key in PEM format and enforces public key
   hash generation. If ``SAVE_KEYS=1``, this
   file name will be used to save the key.

-  ``SAVE_KEYS``: This option is used when ``GENERATE_COT=1``. It tells the
   certificate generation tool to save the keys used to establish the Chain of
   Trust. Allowed options are '0' or '1'. Default is '0' (do not save).

-  ``SCP_BL2``: Path to SCP_BL2 image in the host file system. This image is optional.
   If a SCP_BL2 image is present then this option must be passed for the ``fip``
   target.

-  ``SCP_BL2_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the SCP_BL2 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``SCP_BL2U``: Path to SCP_BL2U image in the host file system. This image is
   optional. It is only needed if the platform makefile specifies that it
   is required in order to build the ``fwu_fip`` target.

-  ``SDEI_SUPPORT``: Setting this to ``1`` enables support for Software
   Delegated Exception Interface to BL31 image. This defaults to ``0``.

   When set to ``1``, the build option ``EL3_EXCEPTION_HANDLING`` must also be
   set to ``1``.

-  ``SEPARATE_CODE_AND_RODATA``: Whether code and read-only data should be
   isolated on separate memory pages. This is a trade-off between security and
   memory usage. See "Isolating code and read-only data on separate memory
   pages" section in :ref:`Firmware Design`. This flag is disabled by default
   and affects all BL images.

-  ``SEPARATE_NOBITS_REGION``: Setting this option to ``1`` allows the NOBITS
   sections of BL31 (.bss, stacks, page tables, and coherent memory) to be
   allocated in RAM discontiguous from the loaded firmware image. When set, the
   platform is expected to provide definitions for ``BL31_NOBITS_BASE`` and
   ``BL31_NOBITS_LIMIT``. When the option is ``0`` (the default), NOBITS
   sections are placed in RAM immediately following the loaded firmware image.

-  ``SEPARATE_BL2_NOLOAD_REGION``: Setting this option to ``1`` allows the
   NOLOAD sections of BL2 (.bss, stacks, page tables) to be allocated in RAM
   discontiguous from loaded firmware images. When set, the platform need to
   provide definitions of ``BL2_NOLOAD_START`` and ``BL2_NOLOAD_LIMIT``. This
   flag is disabled by default and NOLOAD sections are placed in RAM immediately
   following the loaded firmware image.

-  ``SMC_PCI_SUPPORT``: This option allows platforms to handle PCI configuration
   access requests via a standard SMCCC defined in `DEN0115`_. When combined with
   UEFI+ACPI this can provide a certain amount of OS forward compatibility
   with newer platforms that aren't ECAM compliant.

-  ``SPD``: Choose a Secure Payload Dispatcher component to be built into TF-A.
   This build option is only valid if ``ARCH=aarch64``. The value should be
   the path to the directory containing the SPD source, relative to
   ``services/spd/``; the directory is expected to contain a makefile called
   ``<spd-value>.mk``. The SPM Dispatcher standard service is located in
   services/std_svc/spmd and enabled by ``SPD=spmd``. The SPM Dispatcher
   cannot be enabled when the ``SPM_MM`` option is enabled.

-  ``SPIN_ON_BL1_EXIT``: This option introduces an infinite loop in BL1. It can
   take either 0 (no loop) or 1 (add a loop). 0 is the default. This loop stops
   execution in BL1 just before handing over to BL31. At this point, all
   firmware images have been loaded in memory, and the MMU and caches are
   turned off. Refer to the "Debugging options" section for more details.

-  ``SPMC_AT_EL3`` : This boolean option is used jointly with the SPM
   Dispatcher option (``SPD=spmd``). When enabled (1) it indicates the SPMC
   component runs at the EL3 exception level. The default value is ``0`` (
   disabled). This configuration supports pre-Armv8.4 platforms (aka not
   implementing the ``FEAT_SEL2`` extension). This is an experimental feature.

-  ``SPMD_SPM_AT_SEL2`` : This boolean option is used jointly with the SPM
   Dispatcher option (``SPD=spmd``). When enabled (1) it indicates the SPMC
   component runs at the S-EL2 exception level provided by the ``FEAT_SEL2``
   extension. This is the default when enabling the SPM Dispatcher. When
   disabled (0) it indicates the SPMC component runs at the S-EL1 execution
   state or at EL3 if ``SPMC_AT_EL3`` is enabled. The latter configurations
   support pre-Armv8.4 platforms (aka not implementing the ``FEAT_SEL2``
   extension).

-  ``SPM_MM`` : Boolean option to enable the Management Mode (MM)-based Secure
   Partition Manager (SPM) implementation. The default value is ``0``
   (disabled). This option cannot be enabled (``1``) when SPM Dispatcher is
   enabled (``SPD=spmd``).

-  ``SP_LAYOUT_FILE``: Platform provided path to JSON file containing the
   description of secure partitions. The build system will parse this file and
   package all secure partition blobs into the FIP. This file is not
   necessarily part of TF-A tree. Only available when ``SPD=spmd``.

-  ``SP_MIN_WITH_SECURE_FIQ``: Boolean flag to indicate the SP_MIN handles
   secure interrupts (caught through the FIQ line). Platforms can enable
   this directive if they need to handle such interruption. When enabled,
   the FIQ are handled in monitor mode and non secure world is not allowed
   to mask these events. Platforms that enable FIQ handling in SP_MIN shall
   implement the api ``sp_min_plat_fiq_handler()``. The default value is 0.

-  ``SVE_VECTOR_LEN``: SVE vector length to configure in ZCR_EL3.
   Platforms can configure this if they need to lower the hardware
   limit, for example due to asymmetric configuration or limitations of
   software run at lower ELs. The default is the architectural maximum
   of 2048 which should be suitable for most configurations, the
   hardware will limit the effective VL to the maximum physically supported
   VL.

-  ``TRUSTED_BOARD_BOOT``: Boolean flag to include support for the Trusted Board
   Boot feature. When set to '1', BL1 and BL2 images include support to load
   and verify the certificates and images in a FIP, and BL1 includes support
   for the Firmware Update. The default value is '0'. Generation and inclusion
   of certificates in the FIP and FWU_FIP depends upon the value of the
   ``GENERATE_COT`` option.

   .. warning::
      This option depends on ``CREATE_KEYS`` to be enabled. If the keys
      already exist in disk, they will be overwritten without further notice.

-  ``TRUSTED_WORLD_KEY``: This option is used when ``GENERATE_COT=1``. It
   specifies the file that contains the Trusted World private key in PEM
   format. If ``SAVE_KEYS=1``, this file name will be used to save the key.

-  ``TSP_INIT_ASYNC``: Choose BL32 initialization method as asynchronous or
   synchronous, (see "Initializing a BL32 Image" section in
   :ref:`Firmware Design`). It can take the value 0 (BL32 is initialized using
   synchronous method) or 1 (BL32 is initialized using asynchronous method).
   Default is 0.

-  ``TSP_NS_INTR_ASYNC_PREEMPT``: A non zero value enables the interrupt
   routing model which routes non-secure interrupts asynchronously from TSP
   to EL3 causing immediate preemption of TSP. The EL3 is responsible
   for saving and restoring the TSP context in this routing model. The
   default routing model (when the value is 0) is to route non-secure
   interrupts to TSP allowing it to save its context and hand over
   synchronously to EL3 via an SMC.

   .. note::
      When ``EL3_EXCEPTION_HANDLING`` is ``1``, ``TSP_NS_INTR_ASYNC_PREEMPT``
      must also be set to ``1``.

-  ``TWED_DELAY``: Numeric value to be set in order to delay the trapping of
   WFE instruction. ``ENABLE_FEAT_TWED`` build option must be enabled to set
   this delay. It can take values in the range (0-15). Default value is ``0``
   and based on this value, 2^(TWED_DELAY + 8) cycles will be delayed.
   Platforms need to explicitly update this value based on their requirements.

-  ``USE_ARM_LINK``: This flag determines whether to enable support for ARM
   linker. When the ``LINKER`` build variable points to the armlink linker,
   this flag is enabled automatically. To enable support for armlink, platforms
   will have to provide a scatter file for the BL image. Currently, Tegra
   platforms use the armlink support to compile BL3-1 images.

-  ``USE_COHERENT_MEM``: This flag determines whether to include the coherent
   memory region in the BL memory map or not (see "Use of Coherent memory in
   TF-A" section in :ref:`Firmware Design`). It can take the value 1
   (Coherent memory region is included) or 0 (Coherent memory region is
   excluded). Default is 1.

-  ``USE_DEBUGFS``: When set to 1 this option activates an EXPERIMENTAL feature
   exposing a virtual filesystem interface through BL31 as a SiP SMC function.
   Default is 0.

-  ``ARM_IO_IN_DTB``: This flag determines whether to use IO based on the
   firmware configuration framework. This will move the io_policies into a
   configuration device tree, instead of static structure in the code base.

-  ``COT_DESC_IN_DTB``: This flag determines whether to create COT descriptors
   at runtime using fconf. If this flag is enabled, COT descriptors are
   statically captured in tb_fw_config file in the form of device tree nodes
   and properties. Currently, COT descriptors used by BL2 are moved to the
   device tree and COT descriptors used by BL1 are retained in the code
   base statically.

-  ``SDEI_IN_FCONF``: This flag determines whether to configure SDEI setup in
   runtime using firmware configuration framework. The platform specific SDEI
   shared and private events configuration is retrieved from device tree rather
   than static C structures at compile time. This is only supported if
   SDEI_SUPPORT build flag is enabled.

-  ``SEC_INT_DESC_IN_FCONF``: This flag determines whether to configure Group 0
   and Group1 secure interrupts using the firmware configuration framework. The
   platform specific secure interrupt property descriptor is retrieved from
   device tree in runtime rather than depending on static C structure at compile
   time.

-  ``USE_ROMLIB``: This flag determines whether library at ROM will be used.
   This feature creates a library of functions to be placed in ROM and thus
   reduces SRAM usage. Refer to :ref:`Library at ROM` for further details. Default
   is 0.

-  ``V``: Verbose build. If assigned anything other than 0, the build commands
   are printed. Default is 0.

-  ``VERSION_STRING``: String used in the log output for each TF-A image.
   Defaults to a string formed by concatenating the version number, build type
   and build string.

-  ``W``: Warning level. Some compiler warning options of interest have been
   regrouped and put in the root Makefile. This flag can take the values 0 to 3,
   each level enabling more warning options. Default is 0.

-  ``WARMBOOT_ENABLE_DCACHE_EARLY`` : Boolean option to enable D-cache early on
   the CPU after warm boot. This is applicable for platforms which do not
   require interconnect programming to enable cache coherency (eg: single
   cluster platforms). If this option is enabled, then warm boot path
   enables D-caches immediately after enabling MMU. This option defaults to 0.

-  ``SUPPORT_STACK_MEMTAG``: This flag determines whether to enable memory
   tagging for stack or not. It accepts 2 values: ``yes`` and ``no``. The
   default value of this flag is ``no``. Note this option must be enabled only
   for ARM architecture greater than Armv8.5-A.

-  ``ERRATA_SPECULATIVE_AT``: This flag determines whether to enable ``AT``
   speculative errata workaround or not. It accepts 2 values: ``1`` and ``0``.
   The default value of this flag is ``0``.

   ``AT`` speculative errata workaround disables stage1 page table walk for
   lower ELs (EL1 and EL0) in EL3 so that ``AT`` speculative fetch at any point
   produces either the correct result or failure without TLB allocation.

   This boolean option enables errata for all below CPUs.

   +---------+--------------+-------------------------+
   | Errata  |      CPU     |     Workaround Define   |
   +=========+==============+=========================+
   | 1165522 |  Cortex-A76  |  ``ERRATA_A76_1165522`` |
   +---------+--------------+-------------------------+
   | 1319367 |  Cortex-A72  |  ``ERRATA_A72_1319367`` |
   +---------+--------------+-------------------------+
   | 1319537 |  Cortex-A57  |  ``ERRATA_A57_1319537`` |
   +---------+--------------+-------------------------+
   | 1530923 |  Cortex-A55  |  ``ERRATA_A55_1530923`` |
   +---------+--------------+-------------------------+
   | 1530924 |  Cortex-A53  |  ``ERRATA_A53_1530924`` |
   +---------+--------------+-------------------------+

   .. note::
      This option is enabled by build only if platform sets any of above defines
      mentioned in ’Workaround Define' column in the table.
      If this option is enabled for the EL3 software then EL2 software also must
      implement this workaround due to the behaviour of the errata mentioned
      in new SDEN document which will get published soon.

- ``RAS_TRAP_LOWER_EL_ERR_ACCESS``: This flag enables/disables the SCR_EL3.TERR
  bit, to trap access to the RAS ERR and RAS ERX registers from lower ELs.
  This flag is disabled by default.

- ``OPENSSL_DIR``: This option is used to provide the path to a directory on the
  host machine where a custom installation of OpenSSL is located, which is used
  to build the certificate generation, firmware encryption and FIP tools. If
  this option is not set, the default OS installation will be used.

- ``USE_SP804_TIMER``: Use the SP804 timer instead of the Generic Timer for
  functions that wait for an arbitrary time length (udelay and mdelay). The
  default value is 0.

- ``ENABLE_BRBE_FOR_NS``: Numeric value to enable access to the branch record
  buffer registers from NS ELs when FEAT_BRBE is implemented. BRBE is an
  optional architectural feature for AArch64. This flag can take the values
  0 to 2, to align with the ``FEATURE_DETECTION`` mechanism. The default is 0
  and it is automatically disabled when the target architecture is AArch32.

- ``ENABLE_TRBE_FOR_NS``: Numeric value to enable access of trace buffer
  control registers from NS ELs, NS-EL2 or NS-EL1(when NS-EL2 is implemented
  but unused) when FEAT_TRBE is implemented. TRBE is an optional architectural
  feature for AArch64. This flag can take the values  0 to 2, to align with the
  ``FEATURE_DETECTION`` mechanism. The default is 0 and it is automatically
  disabled when the target architecture is AArch32.

- ``ENABLE_SYS_REG_TRACE_FOR_NS``: Boolean option to enable trace system
  registers access from NS ELs, NS-EL2 or NS-EL1 (when NS-EL2 is implemented
  but unused). This feature is available if trace unit such as ETMv4.x, and
  ETE(extending ETM feature) is implemented. This flag is disabled by default.

- ``ENABLE_TRF_FOR_NS``: Numeric value to enable trace filter control registers
  access from NS ELs, NS-EL2 or NS-EL1 (when NS-EL2 is implemented but unused),
  if FEAT_TRF is implemented. This flag can take the values 0 to 2, to align
  with the ``FEATURE_DETECTION`` mechanism. This flag is disabled by default.

- ``PLAT_RSS_NOT_SUPPORTED``: Boolean option to enable the usage of the PSA
  APIs on platforms that doesn't support RSS (providing Arm CCA HES
  functionalities). When enabled (``1``), a mocked version of the APIs are used.
  The default value is 0.

GICv3 driver options
--------------------

GICv3 driver files are included using directive:

``include drivers/arm/gic/v3/gicv3.mk``

The driver can be configured with the following options set in the platform
makefile:

-  ``GICV3_SUPPORT_GIC600``: Add support for the GIC-600 variants of GICv3.
   Enabling this option will add runtime detection support for the
   GIC-600, so is safe to select even for a GIC500 implementation.
   This option defaults to 0.

- ``GICV3_SUPPORT_GIC600AE_FMU``: Add support for the Fault Management Unit
   for GIC-600 AE. Enabling this option will introduce support to initialize
   the FMU. Platforms should call the init function during boot to enable the
   FMU and its safety mechanisms. This option defaults to 0.

-  ``GICV3_IMPL_GIC600_MULTICHIP``: Selects GIC-600 variant with multichip
   functionality. This option defaults to 0

-  ``GICV3_OVERRIDE_DISTIF_PWR_OPS``: Allows override of default implementation
   of ``arm_gicv3_distif_pre_save`` and ``arm_gicv3_distif_post_restore``
   functions. This is required for FVP platform which need to simulate GIC save
   and restore during SYSTEM_SUSPEND without powering down GIC. Default is 0.

-  ``GIC_ENABLE_V4_EXTN`` : Enables GICv4 related changes in GICv3 driver.
   This option defaults to 0.

-  ``GIC_EXT_INTID``: When set to ``1``, GICv3 driver will support extended
   PPI (1056-1119) and SPI (4096-5119) range. This option defaults to 0.

Debugging options
-----------------

To compile a debug version and make the build more verbose use

.. code:: shell

    make PLAT=<platform> DEBUG=1 V=1 all

AArch64 GCC 11 uses DWARF version 5 debugging symbols by default. Some tools
(for example Arm-DS) might not support this and may need an older version of
DWARF symbols to be emitted by GCC. This can be achieved by using the
``-gdwarf-<version>`` flag, with the version being set to 2, 3, 4 or 5. Setting
the version to 4 is recommended for Arm-DS.

When debugging logic problems it might also be useful to disable all compiler
optimizations by using ``-O0``.

.. warning::
   Using ``-O0`` could cause output images to be larger and base addresses
   might need to be recalculated (see the **Memory layout on Arm development
   platforms** section in the :ref:`Firmware Design`).

Extra debug options can be passed to the build system by setting ``CFLAGS`` or
``LDFLAGS``:

.. code:: shell

    CFLAGS='-O0 -gdwarf-2'                                     \
    make PLAT=<platform> DEBUG=1 V=1 all

Note that using ``-Wl,`` style compilation driver options in ``CFLAGS`` will be
ignored as the linker is called directly.

It is also possible to introduce an infinite loop to help in debugging the
post-BL2 phase of TF-A. This can be done by rebuilding BL1 with the
``SPIN_ON_BL1_EXIT=1`` build flag. Refer to the :ref:`build_options_common`
section. In this case, the developer may take control of the target using a
debugger when indicated by the console output. When using Arm-DS, the following
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

Firmware update options
-----------------------

-  ``NR_OF_FW_BANKS``: Define the number of firmware banks. This flag is used
   in defining the firmware update metadata structure. This flag is by default
   set to '2'.

-  ``NR_OF_IMAGES_IN_FW_BANK``: Define the number of firmware images in each
   firmware bank. Each firmware bank must have the same number of images as per
   the `PSA FW update specification`_.
   This flag is used in defining the firmware update metadata structure. This
   flag is by default set to '1'.

-  ``PSA_FWU_SUPPORT``: Enable the firmware update mechanism as per the
   `PSA FW update specification`_. The default value is 0, and this is an
   experimental feature.
   PSA firmware update implementation has some limitations, such as BL2 is
   not part of the protocol-updatable images, if BL2 needs to be updated, then
   it should be done through another platform-defined mechanism, and it assumes
   that the platform's hardware supports CRC32 instructions.

--------------

*Copyright (c) 2019-2022, Arm Limited. All rights reserved.*

.. _DEN0115: https://developer.arm.com/docs/den0115/latest
.. _PSA FW update specification: https://developer.arm.com/documentation/den0118/a/
.. _PSA DRTM specification: https://developer.arm.com/documentation/den0113/a
