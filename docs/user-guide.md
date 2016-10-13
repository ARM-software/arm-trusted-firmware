ARM Trusted Firmware User Guide
===============================

Contents :

1.  [Introduction](#1--introduction)
2.  [Host machine requirements](#2--host-machine-requirements)
3.  [Tools](#3--tools)
4.  [Getting the Trusted Firmware source code](#4--getting-the-trusted-firmware-source-code)
5.  [Building the Trusted Firmware](#5--building-the-trusted-firmware)
6.  [Building a FIP for Juno and FVP](#6--building-a-fip-for-juno-and-fvp)
7.  [EL3 payloads alternative boot flow](#7--el3-payloads-alternative-boot-flow)
8.  [Preloaded BL33 alternative boot flow](#8--preloaded-bl33-alternative-boot-flow)
9.  [Running the software on FVP](#9--running-the-software-on-fvp)
10. [Running the software on Juno](#10--running-the-software-on-juno)


1.  Introduction
----------------

This document describes how to build ARM Trusted Firmware (TF) and run it with a
tested set of other software components using defined configurations on the Juno
ARM development platform and ARM Fixed Virtual Platform (FVP) models. It is
possible to use other software components, configurations and platforms but that
is outside the scope of this document.

This document assumes that the reader has previous experience running a fully
bootable Linux software stack on Juno or FVP using the prebuilt binaries and
filesystems provided by [Linaro][Linaro Release Notes]. Further information may
be found in the [Instructions for using the Linaro software deliverables]
[Linaro SW Instructions]. It also assumes that the user understands the role of
the different software components required to boot a Linux system:

*   Specific firmware images required by the platform (e.g. SCP firmware on Juno)
*   Normal world bootloader (e.g. UEFI or U-Boot)
*   Device tree
*   Linux kernel image
*   Root filesystem

This document also assumes that the user is familiar with the FVP models and
the different command line options available to launch the model.

This document should be used in conjunction with the [Firmware Design].


2.  Host machine requirements
-----------------------------

The minimum recommended machine specification for building the software and
running the FVP models is a dual-core processor running at 2GHz with 12GB of
RAM.  For best performance, use a machine with a quad-core processor running at
2.6GHz with 16GB of RAM.

The software has been tested on Ubuntu 14.04 LTS (64-bit). Packages used for
building the software were installed from that distribution unless otherwise
specified.

The software has also been built on Windows 7 Enterprise SP1, using CMD.EXE,
Cygwin, and Msys (MinGW) shells, using version 4.9.1 of the GNU toolchain.

3.  Tools
---------

Install the required packages to build Trusted Firmware with the following
command:

    sudo apt-get install build-essential gcc make git libssl-dev

Download and install the AArch64 little-endian GCC cross compiler as indicated
in the [Linaro instructions][Linaro SW Instructions].

In addition, the following optional packages and tools may be needed:

*   `device-tree-compiler` package if you need to rebuild the Flattened Device
    Tree (FDT) source files (`.dts` files) provided with this software.

*   For debugging, ARM [Development Studio 5 (DS-5)][DS-5].


4.  Getting the Trusted Firmware source code
--------------------------------------------

Download the Trusted Firmware source code from Github:

    git clone https://github.com/ARM-software/arm-trusted-firmware.git


5.  Building the Trusted Firmware
---------------------------------

*   Before building Trusted Firmware, the environment variable `CROSS_COMPILE`
    must point to the Linaro cross compiler.

    For AArch64:

        export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-

    For AArch32:

        export CROSS_COMPILE=<path-to-aarch32-gcc>/bin/arm-linux-gnueabihf-

*   Change to the root directory of the Trusted Firmware source tree and build.

    For AArch64:

        make PLAT=<platform> all

    For AArch32:

        make PLAT=<platform> ARCH=aarch32 AARCH32_SP=sp_min all


   Notes:

    *   If `PLAT` is not specified, `fvp` is assumed by default. See the
        "Summary of build options" for more information on available build
        options.

    *   (AArch32 only) Currently only `PLAT=fvp` is supported.

    *   (AArch32 only) `AARCH32_SP` is the AArch32 EL3 Runtime Software and it
        corresponds to the BL32 image. A minimal `AARCH32_SP`, sp_min, is
        provided by ARM Trusted Firmware to demonstrate how PSCI Library can
        be integrated with an AArch32 EL3 Runtime Software. Some AArch32 EL3
        Runtime Software may include other runtime services, for example
        Trusted OS services. A guide to integrate PSCI library with AArch32
        EL3 Runtime Software can be found [here][PSCI Lib Integration].

    *   (AArch64 only) The TSP (Test Secure Payload), corresponding to the BL32
        image, is not compiled in by default. Refer to the "Building the Test
        Secure Payload" section below.

    *   By default this produces a release version of the build. To produce a
        debug version instead, refer to the "Debugging options" section below.

    *   The build process creates products in a `build` directory tree, building
        the objects and binaries for each boot loader stage in separate
        sub-directories.  The following boot loader binary files are created
        from the corresponding ELF files:

        *   `build/<platform>/<build-type>/bl1.bin`
        *   `build/<platform>/<build-type>/bl2.bin`
        *   `build/<platform>/<build-type>/bl31.bin` (AArch64 only)
        *   `build/<platform>/<build-type>/bl32.bin` (mandatory for AArch32)

        where `<platform>` is the name of the chosen platform and `<build-type>`
        is either `debug` or `release`. The actual number of images might differ
        depending on the platform.

*   Build products for a specific build variant can be removed using:

        make DEBUG=<D> PLAT=<platform> clean

    ... where `<D>` is `0` or `1`, as specified when building.

    The build tree can be removed completely using:

        make realclean

### Summary of build options

ARM Trusted Firmware build system supports the following build options. Unless
mentioned otherwise, these options are expected to be specified at the build
command line and are not to be modified in any component makefiles. Note that
the build system doesn't track dependency for build options. Therefore, if any
of the build options are changed from a previous build, a clean build must be
performed.

#### Common build options

*   `SCP_BL2`: Path to SCP_BL2 image in the host file system. This image is optional.
    If a SCP_BL2 image is present then this option must be passed for the `fip`
    target.

*   `BL33`: Path to BL33 image in the host file system. This is mandatory for
    `fip` target in case the BL2 from ARM Trusted Firmware is used.

*   `BL2`: This is an optional build option which specifies the path to BL2
    image for the `fip` target. In this case, the BL2 in the ARM Trusted
    Firmware will not be built.

*   `BL31`:  This is an optional build option which specifies the path to
    BL31 image for the `fip` target. In this case, the BL31 in the ARM
    Trusted Firmware will not be built.

*   `BL32`:  This is an optional build option which specifies the path to
    BL32 image for the `fip` target. In this case, the BL32 in the ARM
    Trusted Firmware will not be built.

*   `FIP_NAME`: This is an optional build option which specifies the FIP
    filename for the `fip` target. Default is `fip.bin`.

*   `FWU_FIP_NAME`: This is an optional build option which specifies the FWU
    FIP filename for the `fwu_fip` target. Default is `fwu_fip.bin`.

*   `BL2U`:  This is an optional build option which specifies the path to
    BL2U image. In this case, the BL2U in the ARM Trusted Firmware will not
    be built.

*   `SCP_BL2U`: Path to SCP_BL2U image in the host file system. This image is
    optional. It is only needed if the platform makefile specifies that it
    is required in order to build the `fwu_fip` target.

*   `NS_BL2U`: Path to NS_BL2U image in the host file system. This image is
    optional. It is only needed if the platform makefile specifies that it
    is required in order to build the `fwu_fip` target.

*   `DEBUG`: Chooses between a debug and release build. It can take either 0
    (release) or 1 (debug) as values. 0 is the default.

*   `LOG_LEVEL`: Chooses the log level, which controls the amount of console log
    output compiled into the build. This should be one of the following:

        0  (LOG_LEVEL_NONE)
        10 (LOG_LEVEL_NOTICE)
        20 (LOG_LEVEL_ERROR)
        30 (LOG_LEVEL_WARNING)
        40 (LOG_LEVEL_INFO)
        50 (LOG_LEVEL_VERBOSE)

    All log output up to and including the log level is compiled into the build.
    The default value is 40 in debug builds and 20 in release builds.

*   `NS_TIMER_SWITCH`: Enable save and restore for non-secure timer register
    contents upon world switch. It can take either 0 (don't save and restore) or
    1 (do save and restore). 0 is the default. An SPD may set this to 1 if it
    wants the timer registers to be saved and restored.

*   `PLAT`: Choose a platform to build ARM Trusted Firmware for. The chosen
    platform name must be subdirectory of any depth under `plat/`, and must
    contain a platform makefile named `platform.mk`.

*   `ARCH` : Choose the target build architecture for ARM Trusted Firmware.
    It can take either `aarch64` or `aarch32` as values. By default, it is
    defined to `aarch64`.

*   `SPD`: Choose a Secure Payload Dispatcher component to be built into the
    Trusted Firmware. This build option is only valid if `ARCH=aarch64`. The
    value should be the path to the directory containing the SPD source,
    relative to `services/spd/`; the directory is expected to
    contain a makefile called `<spd-value>.mk`.

*   `AARCH32_SP` : Choose the AArch32 Secure Payload component to be built as
    as the BL32 image when `ARCH=aarch32`. The value should be the path to the
    directory containing the SP source, relative to the `bl32/`; the directory
    is expected to contain a makefile called `<aarch32_sp-value>.mk`.

*   `V`: Verbose build. If assigned anything other than 0, the build commands
    are printed. Default is 0.

*   `ARM_GIC_ARCH`: Choice of ARM GIC architecture version used by the ARM
    Legacy GIC driver for implementing the platform GIC API. This API is used
    by the interrupt management framework. Default is 2 (that is, version 2.0).
    This build option is deprecated.

*   `ARM_CCI_PRODUCT_ID`: Choice of ARM CCI product used by the platform. This
    is used to determine the number of valid slave interfaces available in the
    ARM CCI driver. Default is 400 (that is, CCI-400).

*   `RESET_TO_BL31`: Enable BL31 entrypoint as the CPU reset vector instead
    of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
    entrypoint) or 1 (CPU reset to BL31 entrypoint).
    The default value is 0.

*   `RESET_TO_SP_MIN`: SP_MIN is the minimal AArch32 Secure Payload provided in
    ARM Trusted Firmware. This flag configures SP_MIN entrypoint as the CPU
    reset vector instead of the BL1 entrypoint.  It can take the value 0 (CPU
    reset to BL1 entrypoint) or 1 (CPU reset to SP_MIN entrypoint). The default
    value is 0.

*   `CRASH_REPORTING`: A non-zero value enables a console dump of processor
    register state when an unexpected exception occurs during execution of
    BL31. This option defaults to the value of `DEBUG` - i.e. by default
    this is only enabled for a debug build of the firmware.

*   `ASM_ASSERTION`: This flag determines whether the assertion checks within
    assembly source files are enabled or not. This option defaults to the
    value of `DEBUG` - that is, by default this is only enabled for a debug
    build of the firmware.

*   `TSP_INIT_ASYNC`: Choose BL32 initialization method as asynchronous or
    synchronous, (see "Initializing a BL32 Image" section in [Firmware
    Design]). It can take the value 0 (BL32 is initialized using
    synchronous method) or 1 (BL32 is initialized using asynchronous method).
    Default is 0.

*   `USE_COHERENT_MEM`: This flag determines whether to include the coherent
    memory region in the BL memory map or not (see "Use of Coherent memory in
    Trusted Firmware" section in [Firmware Design]). It can take the value 1
    (Coherent memory region is included) or 0 (Coherent memory region is
    excluded). Default is 1.

*   `TSP_NS_INTR_ASYNC_PREEMPT`: A non zero value enables the interrupt
    routing model which routes non-secure interrupts asynchronously from TSP
    to EL3 causing immediate preemption of TSP. The EL3 is responsible
    for saving and restoring the TSP context in this routing model. The
    default routing model (when the value is 0) is to route non-secure
    interrupts to TSP allowing it to save its context and hand over
    synchronously to EL3 via an SMC.

*   `TRUSTED_BOARD_BOOT`: Boolean flag to include support for the Trusted Board
    Boot feature. When set to '1', BL1 and BL2 images include support to load
    and verify the certificates and images in a FIP, and BL1 includes support
    for the Firmware Update. The default value is '0'. Generation and inclusion
    of certificates in the FIP and FWU_FIP depends upon the value of the
    `GENERATE_COT` option.

*   `GENERATE_COT`: Boolean flag used to build and execute the `cert_create`
    tool to create certificates as per the Chain of Trust described in
    [Trusted Board Boot].  The build system then calls `fiptool` to
    include the certificates in the FIP and FWU_FIP. Default value is '0'.

    Specify both `TRUSTED_BOARD_BOOT=1` and `GENERATE_COT=1` to include support
    for the Trusted Board Boot feature in the BL1 and BL2 images, to generate
    the corresponding certificates, and to include those certificates in the
    FIP and FWU_FIP.

    Note that if `TRUSTED_BOARD_BOOT=0` and `GENERATE_COT=1`, the BL1 and BL2
    images will not include support for Trusted Board Boot. The FIP will still
    include the corresponding certificates. This FIP can be used to verify the
    Chain of Trust on the host machine through other mechanisms.

    Note that if `TRUSTED_BOARD_BOOT=1` and `GENERATE_COT=0`, the BL1 and BL2
    images will include support for Trusted Board Boot, but the FIP and FWU_FIP
    will not include the corresponding certificates, causing a boot failure.

*   `CREATE_KEYS`: This option is used when `GENERATE_COT=1`. It tells the
    certificate generation tool to create new keys in case no valid keys are
    present or specified. Allowed options are '0' or '1'. Default is '1'.

*   `SAVE_KEYS`: This option is used when `GENERATE_COT=1`. It tells the
    certificate generation tool to save the keys used to establish the Chain of
    Trust. Allowed options are '0' or '1'. Default is '0' (do not save).

    Note: This option depends on 'CREATE_KEYS' to be enabled. If the keys
    already exist in disk, they will be overwritten without further notice.

*   `ROT_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the ROT private key in PEM format. If `SAVE_KEYS=1`, this
    file name will be used to save the key.

*   `TRUSTED_WORLD_KEY`: This option is used when `GENERATE_COT=1`. It
    specifies the file that contains the Trusted World private key in PEM
    format. If `SAVE_KEYS=1`, this file name will be used to save the key.

*   `NON_TRUSTED_WORLD_KEY`: This option is used when `GENERATE_COT=1`. It
    specifies the file that contains the Non-Trusted World private key in PEM
    format. If `SAVE_KEYS=1`, this file name will be used to save the key.

*   `SCP_BL2_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the SCP_BL2 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `BL31_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL31 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `BL32_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL32 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `BL33_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL33 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `PROGRAMMABLE_RESET_ADDRESS`: This option indicates whether the reset
    vector address can be programmed or is fixed on the platform. It can take
    either 0 (fixed) or 1 (programmable). Default is 0. If the platform has a
    programmable reset address, it is expected that a CPU will start executing
    code directly at the right address, both on a cold and warm reset. In this
    case, there is no need to identify the entrypoint on boot and the boot path
    can be optimised. The `plat_get_my_entrypoint()` platform porting interface
    does not need to be implemented in this case.

*   `COLD_BOOT_SINGLE_CPU`: This option indicates whether the platform may
    release several CPUs out of reset. It can take either 0 (several CPUs may be
    brought up) or 1 (only one CPU will ever be brought up during cold reset).
    Default is 0. If the platform always brings up a single CPU, there is no
    need to distinguish between primary and secondary CPUs and the boot path can
    be optimised. The `plat_is_my_cpu_primary()` and
    `plat_secondary_cold_boot_setup()` platform porting interfaces do not need
    to be implemented in this case.

*   `PSCI_EXTENDED_STATE_ID`: As per PSCI1.0 Specification, there are 2 formats
    possible for the PSCI power-state parameter viz original and extended
    State-ID formats. This flag if set to 1, configures the generic PSCI layer
    to use the extended format. The default value of this flag is 0, which
    means by default the original power-state format is used by the PSCI
    implementation. This flag should be specified by the platform makefile
    and it governs the return value of PSCI_FEATURES API for CPU_SUSPEND
    smc function id.

*   `ERROR_DEPRECATED`: This option decides whether to treat the usage of
    deprecated platform APIs, helper functions or drivers within Trusted
    Firmware as error. It can take the value 1 (flag the use of deprecated
    APIs as error) or 0. The default is 0.

*   `SPIN_ON_BL1_EXIT`: This option introduces an infinite loop in BL1. It can
    take either 0 (no loop) or 1 (add a loop). 0 is the default. This loop stops
    execution in BL1 just before handing over to BL31. At this point, all
    firmware images have been loaded in memory, and the MMU and caches are
    turned off. Refer to the "Debugging options" section for more details.

*   `EL3_PAYLOAD_BASE`: This option enables booting an EL3 payload instead of
    the normal boot flow. It must specify the entry point address of the EL3
    payload. Please refer to the "Booting an EL3 payload" section for more
    details.

*   `PRELOADED_BL33_BASE`: This option enables booting a preloaded BL33 image
    instead of the normal boot flow. When defined, it must specify the entry
    point address for the preloaded BL33 image. This option is incompatible with
    `EL3_PAYLOAD_BASE`. If both are defined, `EL3_PAYLOAD_BASE` has priority
    over `PRELOADED_BL33_BASE`.

*   `PL011_GENERIC_UART`: Boolean option to indicate the PL011 driver that
    the underlying hardware is not a full PL011 UART but a minimally compliant
    generic UART, which is a subset of the PL011. The driver will not access
    any register that is not part of the SBSA generic UART specification.
    Default value is 0 (a full PL011 compliant UART is present).

*   `CTX_INCLUDE_AARCH32_REGS` : Boolean option that, when set to 1, will cause
    the AArch32 system registers to be included when saving and restoring the
    CPU context. The option must be set to 0 for AArch64-only platforms (that
    is on hardware that does not implement AArch32, or at least not at EL1 and
    higher ELs). Default value is 1.

*   `CTX_INCLUDE_FPREGS`: Boolean option that, when set to 1, will cause the FP
    registers to be included when saving and restoring the CPU context. Default
    is 0.

*   `DISABLE_PEDANTIC`: When set to 1 it will disable the -pedantic option in
    the GCC command line. Default is 0.

*   `BUILD_STRING`: Input string for VERSION_STRING, which allows the TF build
    to be uniquely identified. Defaults to the current git commit id.

*   `VERSION_STRING`: String used in the log output for each TF image. Defaults
    to a string formed by concatenating the version number, build type and build
    string.

*   `BUILD_MESSAGE_TIMESTAMP`: String used to identify the time and date of the
    compilation of each build. It must be set to a C string (including quotes
    where applicable). Defaults to a string that contains the time and date of
    the compilation.

*   `HANDLE_EA_EL3_FIRST`: When defined External Aborts and SError Interrupts
    will be always trapped in EL3 i.e. in BL31 at runtime.

*   `ENABLE_PMF`: Boolean option to enable support for optional Performance
     Measurement Framework(PMF). Default is 0.

*   `ENABLE_PSCI_STAT`: Boolean option to enable support for optional PSCI
     functions `PSCI_STAT_RESIDENCY` and `PSCI_STAT_COUNT`. Default is 0.
     Enabling this option enables the `ENABLE_PMF` build option as well.
     The PMF is used for collecting the statistics.

*   `SEPARATE_CODE_AND_RODATA`: Whether code and read-only data should be
    isolated on separate memory pages. This is a trade-off between security and
    memory usage. See "Isolating code and read-only data on separate memory
    pages" section in [Firmware Design]. This flag is disabled by default and
    affects all BL images.

*   `LOAD_IMAGE_V2`: Boolean option to enable support for new version (v2) of
    image loading, which provides more flexibility and scalability around what
    images are loaded and executed during boot. Default is 0.
    Note: `TRUSTED_BOARD_BOOT` is currently not supported when `LOAD_IMAGE_V2`
    is enabled.

#### ARM development platform specific build options

*   `ARM_TSP_RAM_LOCATION`: location of the TSP binary. Options:
    -   `tsram` : Trusted SRAM (default option)
    -   `tdram` : Trusted DRAM (if available)
    -   `dram`  : Secure region in DRAM (configured by the TrustZone controller)

For a better understanding of these options, the ARM development platform memory
map is explained in the [Firmware Design].

*   `ARM_ROTPK_LOCATION`: used when `TRUSTED_BOARD_BOOT=1`. It specifies the
    location of the ROTPK hash returned by the function `plat_get_rotpk_info()`
    for ARM platforms. Depending on the selected option, the proper private key
    must be specified using the `ROT_KEY` option when building the Trusted
    Firmware. This private key will be used by the certificate generation tool
    to sign the BL2 and Trusted Key certificates. Available options for
    `ARM_ROTPK_LOCATION` are:

    -   `regs` : return the ROTPK hash stored in the Trusted root-key storage
        registers. The private key corresponding to this ROTPK hash is not
        currently available.
    -   `devel_rsa` : return a development public key hash embedded in the BL1
        and BL2 binaries. This hash has been obtained from the RSA public key
        `arm_rotpk_rsa.der`, located in `plat/arm/board/common/rotpk`. To use
        this option, `arm_rotprivk_rsa.pem` must be specified as `ROT_KEY` when
        creating the certificates.

*   `ARM_RECOM_STATE_ID_ENC`: The PSCI1.0 specification recommends an encoding
    for the construction of composite state-ID in the power-state parameter.
    The existing PSCI clients currently do not support this encoding of
    State-ID yet. Hence this flag is used to configure whether to use the
    recommended State-ID encoding or not. The default value of this flag is 0,
    in which case the platform is configured to expect NULL in the State-ID
    field of power-state parameter.

*   `ARM_DISABLE_TRUSTED_WDOG`: boolean option to disable the Trusted Watchdog.
    By default, ARM platforms use a watchdog to trigger a system reset in case
    an error is encountered during the boot process (for example, when an image
    could not be loaded or authenticated). The watchdog is enabled in the early
    platform setup hook at BL1 and disabled in the BL1 prepare exit hook. The
    Trusted Watchdog may be disabled at build time for testing or development
    purposes.

*   `ARM_CONFIG_CNTACR`: boolean option to unlock access to the CNTBase<N>
    frame registers by setting the CNTCTLBase.CNTACR<N> register bits. The
    frame number <N> is defined by 'PLAT_ARM_NSTIMER_FRAME_ID', which should
    match the frame used by the Non-Secure image (normally the Linux kernel).
    Default is true (access to the frame is allowed).

*   `ARM_BOARD_OPTIMISE_MEM`: Boolean option to enable or disable optimisation
    of the memory reserved for each image. This affects the maximum size of each
    BL image as well as the number of allocated memory regions and translation
    tables. By default this flag is 0, which means it uses the default
    unoptimised values for these macros. ARM development platforms that wish to
    optimise memory usage need to set this flag to 1 and must override the
    related macros.

*   'ARM_BL31_IN_DRAM': Boolean option to select loading of BL31 in TZC secured
    DRAM. By default, BL31 is in the secure SRAM. Set this flag to 1 to load
    BL31 in TZC secured DRAM. If TSP is present, then setting this option also
    sets the TSP location to DRAM and ignores the `ARM_TSP_RAM_LOCATION` build
    flag.

#### ARM CSS platform specific build options

*   `CSS_DETECT_PRE_1_7_0_SCP`: Boolean flag to detect SCP version
    incompatibility. Version 1.7.0 of the SCP firmware made a non-backwards
    compatible change to the MTL protocol, used for AP/SCP communication.
    Trusted Firmware no longer supports earlier SCP versions. If this option is
    set to 1 then Trusted Firmware will detect if an earlier version is in use.
    Default is 1.

*   `CSS_LOAD_SCP_IMAGES`: Boolean flag, which when set, adds SCP_BL2 and
    SCP_BL2U to the FIP and FWU_FIP respectively, and enables them to be loaded
    during boot. Default is 1.

#### ARM FVP platform specific build options

*   `FVP_USE_GIC_DRIVER`   : Selects the GIC driver to be built. Options:
    -    `FVP_GICV2`       : The GICv2 only driver is selected
    -    `FVP_GICV3`       : The GICv3 only driver is selected (default option)
    -    `FVP_GICV3_LEGACY`: The Legacy GICv3 driver is selected (deprecated)
          Note: If Trusted Firmware is compiled with this option on FVPs with
          GICv3 hardware, then it configures the hardware to run in GICv2
          emulation mode

*   `FVP_CLUSTER_COUNT`    : Configures the cluster count to be used to
     build the topology tree within Trusted Firmware. By default the
     Trusted Firmware is configured for dual cluster topology and this option
     can be used to override the default value.

*   `FVP_USE_SP804_TIMER`  : Use the SP804 timer instead of the Generic Timer
     for functions that wait for an arbitrary time length (udelay and mdelay).
     The default value is 0.

*    `FVP_INTERCONNECT_DRIVER`: Selects the interconnect driver to be built. The
     default interconnect driver depends on the value of `FVP_CLUSTER_COUNT` as
     explained in the options below:
     -    `FVP_CCI`           : The CCI driver is selected. This is the default
                                if 0 < `FVP_CLUSTER_COUNT` <= 2.
     -    `FVP_CCN`           : The CCN driver is selected. This is the default
                                if `FVP_CLUSTER_COUNT` > 2.

### Debugging options

To compile a debug version and make the build more verbose use

    make PLAT=<platform> DEBUG=1 V=1 all

AArch64 GCC uses DWARF version 4 debugging symbols by default. Some tools (for
example DS-5) might not support this and may need an older version of DWARF
symbols to be emitted by GCC. This can be achieved by using the
`-gdwarf-<version>` flag, with the version being set to 2 or 3. Setting the
version to 2 is recommended for DS-5 versions older than 5.16.

When debugging logic problems it might also be useful to disable all compiler
optimizations by using `-O0`.

NOTE: Using `-O0` could cause output images to be larger and base addresses
might need to be recalculated (see the **Memory layout on ARM development
platforms** section in the [Firmware Design]).

Extra debug options can be passed to the build system by setting `CFLAGS`:

    CFLAGS='-O0 -gdwarf-2'                                     \
    make PLAT=<platform> DEBUG=1 V=1 all

It is also possible to introduce an infinite loop to help in debugging the
post-BL2 phase of the Trusted Firmware. This can be done by rebuilding BL1 with
the `SPIN_ON_BL1_EXIT=1` build flag. Refer to the "Summary of build options"
section. In this case, the developer may take control of the target using a
debugger when indicated by the console output. When using DS-5, the following
commands can be used:

    # Stop target execution
    interrupt

    #
    # Prepare your debugging environment, e.g. set breakpoints
    #

    # Jump over the debug loop
    set var $AARCH64::$Core::$PC = $AARCH64::$Core::$PC + 4

    # Resume execution
    continue


### Building the Test Secure Payload

The TSP is coupled with a companion runtime service in the BL31 firmware,
called the TSPD. Therefore, if you intend to use the TSP, the BL31 image
must be recompiled as well. For more information on SPs and SPDs, see the
"Secure-EL1 Payloads and Dispatchers" section in the [Firmware Design].

First clean the Trusted Firmware build directory to get rid of any previous
BL31 binary. Then to build the TSP image use:

    make PLAT=<platform> SPD=tspd all

An additional boot loader binary file is created in the `build` directory:

    build/<platform>/<build-type>/bl32.bin

### Checking source code style

When making changes to the source for submission to the project, the source
must be in compliance with the Linux style guide, and to assist with this check
the project Makefile contains two targets, which both utilise the
`checkpatch.pl` script that ships with the Linux source tree.

To check the entire source tree, you must first download a copy of
`checkpatch.pl` (or the full Linux source), set the `CHECKPATCH` environment
variable to point to the script and build the target checkcodebase:

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkcodebase

To just check the style on the files that differ between your local branch and
the remote master, use:

    make CHECKPATCH=<path-to-linux>/linux/scripts/checkpatch.pl checkpatch

If you wish to check your patch against something other than the remote master,
set the `BASE_COMMIT` variable to your desired branch. By default, `BASE_COMMIT`
is set to `origin/master`.


### Building and using the FIP tool

Firmware Image Package (FIP) is a packaging format used by the Trusted Firmware
project to package firmware images in a single binary. The number and type of
images that should be packed in a FIP is platform specific and may include TF
images and other firmware images required by the platform. For example, most
platforms require a BL33 image which corresponds to the normal world bootloader
(e.g. UEFI or U-Boot).

The TF build system provides the make target `fip` to create a FIP file for the
specified platform using the FIP creation tool included in the TF project.
Examples below show how to build a FIP file for FVP, packaging TF images and a
BL33 image.

For AArch64:

    make PLAT=fvp BL33=<path/to/bl33.bin> fip

For AArch32:

    make PLAT=fvp ARCH=aarch32 AARCH32_SP=sp_min BL33=<path/to/bl33.bin> fip

Note that AArch32 support for Normal world boot loader (BL33), like U-boot or
UEFI, on FVP is not available upstream. Hence custom solutions are required to
allow Linux boot on FVP. These instructions assume such a custom boot loader
(BL33) is available.

The resulting FIP may be found in:

    build/fvp/<build-type>/fip.bin

For advanced operations on FIP files, it is also possible to independently build
the tool and create or modify FIPs using this tool. To do this, follow these
steps:

It is recommended to remove old artifacts before building the tool:

    make -C tools/fiptool clean

Build the tool:

    make [DEBUG=1] [V=1] fiptool

The tool binary can be located in:

    ./tools/fiptool/fiptool

Invoking the tool with `--help` will print a help message with all available
options.

Example 1: create a new Firmware package `fip.bin` that contains BL2 and BL31:

    ./tools/fiptool/fiptool create \
        --tb-fw build/<platform>/<build-type>/bl2.bin \
        --soc-fw build/<platform>/<build-type>/bl31.bin \
        fip.bin

Example 2: view the contents of an existing Firmware package:

    ./tools/fiptool/fiptool info <path-to>/fip.bin

Example 3: update the entries of an existing Firmware package:

    # Change the BL2 from Debug to Release version
    ./tools/fiptool/fiptool update \
        --tb-fw build/<platform>/release/bl2.bin \
        build/<platform>/debug/fip.bin

Example 4: unpack all entries from an existing Firmware package:

    # Images will be unpacked to the working directory
    ./tools/fiptool/fiptool unpack <path-to>/fip.bin

Example 5: remove an entry from an existing Firmware package:

    ./tools/fiptool/fiptool remove \
        --tb-fw build/<platform>/debug/fip.bin

Note that if the destination FIP file exists, the create, update and
remove operations will automatically overwrite it.

The unpack operation will fail if the images already exist at the
destination.  In that case, use -f or --force to continue.

More information about FIP can be found in the [Firmware Design document]
[Firmware Design].

#### Migrating from fip_create to fiptool

The previous version of fiptool was called fip_create.  A compatibility script
that emulates the basic functionality of the previous fip_create is provided.
However, users are strongly encouraged to migrate to fiptool.

*   To create a new FIP file, replace "fip_create" with "fiptool create".
*   To update a FIP file, replace "fip_create" with "fiptool update".
*   To dump the contents of a FIP file, replace "fip_create --dump"
    with "fiptool info".

### Building FIP images with support for Trusted Board Boot

Trusted Board Boot primarily consists of the following two features:

*   Image Authentication, described in [Trusted Board Boot], and
*   Firmware Update, described in [Firmware Update]

The following steps should be followed to build FIP and (optionally) FWU_FIP
images with support for these features:

1.  Fulfill the dependencies of the `mbedtls` cryptographic and image parser
    modules by checking out a recent version of the [mbed TLS Repository]. It
    is important to use a version that is compatible with TF and fixes any
    known security vulnerabilities. See [mbed TLS Security Center] for more
    information. This version of TF is tested with tag `mbedtls-2.2.1`.

    The `drivers/auth/mbedtls/mbedtls_*.mk` files contain the list of mbed TLS
    source files the modules depend upon.
    `include/drivers/auth/mbedtls/mbedtls_config.h` contains the configuration
    options required to build the mbed TLS sources.

    Note that the mbed TLS library is licensed under the Apache version 2.0
    license. Using mbed TLS source code will affect the licensing of
    Trusted Firmware binaries that are built using this library.

2.  To build the FIP image, ensure the following command line variables are set
    while invoking `make` to build Trusted Firmware:

    *   `MBEDTLS_DIR=<path of the directory containing mbed TLS sources>`
    *   `TRUSTED_BOARD_BOOT=1`
    *   `GENERATE_COT=1`

    In the case of ARM platforms, the location of the ROTPK hash must also be
    specified at build time. Two locations are currently supported (see
    `ARM_ROTPK_LOCATION` build option):

    *   `ARM_ROTPK_LOCATION=regs`: the ROTPK hash is obtained from the Trusted
        root-key storage registers present in the platform. On Juno, this
        registers are read-only. On FVP Base and Cortex models, the registers
        are read-only, but the value can be specified using the command line
        option `bp.trusted_key_storage.public_key` when launching the model.
        On both Juno and FVP models, the default value corresponds to an
        ECDSA-SECP256R1 public key hash, whose private part is not currently
        available.

    *   `ARM_ROTPK_LOCATION=devel_rsa`: use the ROTPK hash that is hardcoded
        in the ARM platform port. The private/public RSA key pair may be
        found in `plat/arm/board/common/rotpk`.

    Example of command line using RSA development keys:

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

3.  The optional FWU_FIP contains any additional images to be loaded from
    Non-Volatile storage during the [Firmware Update] process. To build the
    FWU_FIP, any FWU images required by the platform must be specified on the
    command line. On ARM development platforms like Juno, these are:

    *   NS_BL2U. The AP non-secure Firmware Updater image.
    *   SCP_BL2U. The SCP Firmware Update Configuration image.

    Example of Juno command line for generating both `fwu` and `fwu_fip`
    targets using RSA development:

        MBEDTLS_DIR=<path of the directory containing mbed TLS sources> \
        make PLAT=juno TRUSTED_BOARD_BOOT=1 GENERATE_COT=1              \
        ARM_ROTPK_LOCATION=devel_rsa                                    \
        ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
        BL33=<path-to>/<bl33_image>                                     \
        SCP_BL2=<path-to>/<scp_bl2_image>                               \
        SCP_BL2U=<path-to>/<scp_bl2u_image>                             \
        NS_BL2U=<path-to>/<ns_bl2u_image>                               \
        all fip fwu_fip

    Note:   The BL2U image will be built by default and added to the FWU_FIP.
            The user may override this by adding `BL2U=<path-to>/<bl2u_image>`
            to the command line above.

    Note:   Building and installing the non-secure and SCP FWU images (NS_BL1U,
            NS_BL2U and SCP_BL2U) is outside the scope of this document.

    The result of this build will be bl1.bin, fip.bin and fwu_fip.bin binaries.
    Both the FIP and FWU_FIP will include the certificates corresponding to the
    Chain of Trust described in the TBBR-client document. These certificates
    can also be found in the output build directory.


### Building the Certificate Generation Tool

The `cert_create` tool is built as part of the TF build process when the `fip`
make target is specified and TBB is enabled (as described in the previous
section), but it can also be built separately with the following command:

    make PLAT=<platform> [DEBUG=1] [V=1] certtool

Specifying the platform is mandatory since the tool is platform specific.
`DEBUG=1` builds the tool in debug mode. `V=1` makes the build process more
verbose. The following command should be used to obtain help about the tool:

    ./tools/cert_create/cert_create -h


6.  Building a FIP for Juno and FVP
-----------------------------------

This section provides Juno and FVP specific instructions to build Trusted
Firmware, obtain the additional required firmware, and pack it all together in
a single FIP binary. It assumes that a [Linaro Release][Linaro Release Notes]
has been installed.

Note currently [Linaro Release][Linaro Release Notes] only includes pre-built
binaries for AArch64. For AArch32, pre-built binaries are not available.

Note: follow the full instructions for one platform before switching to a
different one. Mixing instructions for different platforms may result in
corrupted binaries.

1.  Clean the working directory

        make realclean

2.  Obtain SCP_BL2 (Juno) and BL33 (all platforms)

    Use the fiptool to extract the SCP_BL2 and BL33 images from the FIP
    package included in the Linaro release:

        # Build the fiptool
        make [DEBUG=1] [V=1] fiptool

        # Unpack firmware images from Linaro FIP
        ./tools/fiptool/fiptool unpack \
             <path/to/linaro/release>/fip.bin

    The unpack operation will result in a set of binary images extracted to the
    working directory. The SCP_BL2 image corresponds to `scp-fw.bin` and BL33
    corresponds to `nt-fw.bin`.

    Note: the fiptool will complain if the images to be unpacked already
    exist in the current directory. If that is the case, either delete those
    files or use the `--force` option to overwrite.

    Note for AArch32, the instructions below assume that nt-fw.bin is a custom
    Normal world boot loader that supports AArch32.

3.  Build TF images and create a new FIP

        # Juno
        make PLAT=juno SCP_BL2=scp-fw.bin BL33=nt-fw.bin all fip

        # FVP AArch64
        make PLAT=fvp BL33=nt-fw.bin all fip

        # FVP AArch32
        make PLAT=fvp ARCH=aarch32 AARCH32_SP=sp_min BL33=nt-fw.bin all fip

The resulting BL1 and FIP images may be found in:

    # Juno
    ./build/juno/release/bl1.bin
    ./build/juno/release/fip.bin

    # FVP
    ./build/fvp/release/bl1.bin
    ./build/fvp/release/fip.bin


7.  EL3 payloads alternative boot flow
--------------------------------------

On a pre-production system, the ability to execute arbitrary, bare-metal code at
the highest exception level is required. It allows full, direct access to the
hardware, for example to run silicon soak tests.

Although it is possible to implement some baremetal secure firmware from
scratch, this is a complex task on some platforms, depending on the level of
configuration required to put the system in the expected state.

Rather than booting a baremetal application, a possible compromise is to boot
`EL3 payloads` through the Trusted Firmware instead. This is implemented as an
alternative boot flow, where a modified BL2 boots an EL3 payload, instead of
loading the other BL images and passing control to BL31. It reduces the
complexity of developing EL3 baremetal code by:

*   putting the system into a known architectural state;
*   taking care of platform secure world initialization;
*   loading the SCP_BL2 image if required by the platform.

When booting an EL3 payload on ARM standard platforms, the configuration of the
TrustZone controller is simplified such that only region 0 is enabled and is
configured to permit secure access only. This gives full access to the whole
DRAM to the EL3 payload.

The system is left in the same state as when entering BL31 in the default boot
flow. In particular:

*   Running in EL3;
*   Current state is AArch64;
*   Little-endian data access;
*   All exceptions disabled;
*   MMU disabled;
*   Caches disabled.

### Booting an EL3 payload

The EL3 payload image is a standalone image and is not part of the FIP. It is
not loaded by the Trusted Firmware. Therefore, there are 2 possible scenarios:

*   The EL3 payload may reside in non-volatile memory (NVM) and execute in
    place. In this case, booting it is just a matter of specifying the right
    address in NVM through `EL3_PAYLOAD_BASE` when building the TF.

*   The EL3 payload needs to be loaded in volatile memory (e.g. DRAM) at
    run-time.

To help in the latter scenario, the `SPIN_ON_BL1_EXIT=1` build option can be
used. The infinite loop that it introduces in BL1 stops execution at the right
moment for a debugger to take control of the target and load the payload (for
example, over JTAG).

It is expected that this loading method will work in most cases, as a debugger
connection is usually available in a pre-production system. The user is free to
use any other platform-specific mechanism to load the EL3 payload, though.

#### Booting an EL3 payload on FVP

The EL3 payloads boot flow requires the CPU's mailbox to be cleared at reset for
the secondary CPUs holding pen to work properly. Unfortunately, its reset value
is undefined on the FVP platform and the FVP platform code doesn't clear it.
Therefore, one must modify the way the model is normally invoked in order to
clear the mailbox at start-up.

One way to do that is to create an 8-byte file containing all zero bytes using
the following command:

    dd if=/dev/zero of=mailbox.dat bs=1 count=8

and pre-load it into the FVP memory at the mailbox address (i.e. `0x04000000`)
using the following model parameters:

    --data cluster0.cpu0=mailbox.dat@0x04000000   [Base FVPs]
    --data=mailbox.dat@0x04000000                 [Foundation FVP]

To provide the model with the EL3 payload image, the following methods may be
used:

1.  If the EL3 payload is able to execute in place, it may be programmed into
    flash memory. On Base Cortex and AEM FVPs, the following model parameter
    loads it at the base address of the NOR FLASH1 (the NOR FLASH0 is already
    used for the FIP):

        -C bp.flashloader1.fname="/path/to/el3-payload"

    On Foundation FVP, there is no flash loader component and the EL3 payload
    may be programmed anywhere in flash using method 3 below.

2.  When using the `SPIN_ON_BL1_EXIT=1` loading method, the following DS-5
    command may be used to load the EL3 payload ELF image over JTAG:

        load /path/to/el3-payload.elf

3.  The EL3 payload may be pre-loaded in volatile memory using the following
    model parameters:

        --data cluster0.cpu0="/path/to/el3-payload"@address  [Base FVPs]
        --data="/path/to/el3-payload"@address                [Foundation FVP]

    The address provided to the FVP must match the `EL3_PAYLOAD_BASE` address
    used when building the Trusted Firmware.

#### Booting an EL3 payload on Juno

If the EL3 payload is able to execute in place, it may be programmed in flash
memory by adding an entry in the `SITE1/HBI0262x/images.txt` configuration file
on the Juno SD card (where `x` depends on the revision of the Juno board).
Refer to the [Juno Getting Started Guide], section 2.3 "Flash memory
programming" for more information.

Alternatively, the same DS-5 command mentioned in the FVP section above can
be used to load the EL3 payload's ELF file over JTAG on Juno.


8.  Preloaded BL33 alternative boot flow
----------------------------------------

Some platforms have the ability to preload BL33 into memory instead of relying
on Trusted Firmware to load it. This may simplify packaging of the normal world
code and improve performance in a development environment. When secure world
cold boot is complete, Trusted Firmware simply jumps to a BL33 base address
provided at build time.

For this option to be used, the `PRELOADED_BL33_BASE` build option has to be
used when compiling the Trusted Firmware. For example, the following command
will create a FIP without a BL33 and prepare to jump to a BL33 image loaded at
address 0x80000000:

    make PRELOADED_BL33_BASE=0x80000000 PLAT=fvp all fip

#### Boot of a preloaded bootwrapped kernel image on Base FVP

The following example uses the AArch64 boot wrapper. This simplifies normal
world booting while also making use of TF features. It can be obtained from its
repository with:

    git clone git://git.kernel.org/pub/scm/linux/kernel/git/mark/boot-wrapper-aarch64.git

After compiling it, an ELF file is generated. It can be loaded with the
following command:

    <path-to>/FVP_Base_AEMv8A-AEMv8A              \
        -C bp.secureflashloader.fname=bl1.bin     \
        -C bp.flashloader0.fname=fip.bin          \
        -a cluster0.cpu0=<bootwrapped-kernel.elf> \
        --start cluster0.cpu0=0x0

The `-a cluster0.cpu0=<bootwrapped-kernel.elf>` option loads the ELF file. It
also sets the PC register to the ELF entry point address, which is not the
desired behaviour, so the `--start cluster0.cpu0=0x0` option forces the PC back
to 0x0 (the BL1 entry point address) on CPU #0. The `PRELOADED_BL33_BASE` define
used when compiling the FIP must match the ELF entry point.

#### Boot of a preloaded bootwrapped kernel image on Juno

The procedure to obtain and compile the boot wrapper is very similar to the case
of the FVP. The execution must be stopped at the end of bl2_main(), and the
loading method explained above in the EL3 payload boot flow section may be used
to load the ELF file over JTAG on Juno.


9.  Running the software on FVP
-------------------------------

The AArch64 build of this version of ARM Trusted Firmware has been tested on
the following ARM FVPs (64-bit host machine only).

*   `Foundation_Platform` (Version 10.1, Build 10.1.32)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 7.7, Build 0.8.7701)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 7.7, Build 0.8.7701)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 7.7, Build 0.8.7701)
*   `FVP_Base_Cortex-A57x2-A53x4` (Version 7.7, Build 0.8.7701)

The AArch32 build of this version of ARM Trusted Firmware has been tested on
the following ARM FVPs (64-bit host machine only).

*   `FVP_Base_AEMv8A-AEMv8A` (Version 7.7, Build 0.8.7701)
*   `FVP_Base_Cortex-A32x4` (Version 10.1, Build 10.1.32)

NOTE: The build numbers quoted above are those reported by launching the FVP
with the `--version` parameter.

NOTE: The software will not work on Version 1.0 of the Foundation FVP.
The commands below would report an `unhandled argument` error in this case.

NOTE: The Foundation FVP does not provide a debugger interface.

The Foundation FVP is a cut down version of the AArch64 Base FVP. It can be
downloaded for free from [ARM's website][ARM FVP website].

Please refer to the FVP documentation for a detailed description of the model
parameter options. A brief description of the important ones that affect the ARM
Trusted Firmware and normal world software behavior is provided below.

### Obtaining the Flattened Device Trees

Depending on the FVP configuration and Linux configuration used, different
FDT files are required. FDTs for the Foundation and Base FVPs can be found in
the Trusted Firmware source directory under `fdts/`. The Foundation FVP has a
subset of the Base FVP components. For example, the Foundation FVP lacks CLCD
and MMC support, and has only one CPU cluster.

Note: It is not recommended to use the FDTs built along the kernel because not
all FDTs are available from there.

*   `fvp-base-gicv2-psci.dtb`

    For use with both AEMv8 and Cortex-A57-A53 Base FVPs with
    Base memory map configuration.

*   `fvp-base-gicv2-psci-aarch32.dtb`

    For use with AEMv8 and Cortex-A32 Base FVPs running Linux in AArch32 state
    with Base memory map configuration.

*   `fvp-base-gicv3-psci.dtb`

    (Default) For use with both AEMv8 and Cortex-A57-A53 Base FVPs with Base
    memory map configuration and Linux GICv3 support.

*   `fvp-base-gicv3-psci-aarch32.dtb`

    For use with AEMv8 and Cortex-A32 Base FVPs running Linux in AArch32 state
    with Base memory map configuration and Linux GICv3 support.

*   `fvp-foundation-gicv2-psci.dtb`

    For use with Foundation FVP with Base memory map configuration.

*   `fvp-foundation-gicv3-psci.dtb`

    (Default) For use with Foundation FVP with Base memory map configuration
    and Linux GICv3 support.

### Running on the Foundation FVP with reset to BL1 entrypoint

The following `Foundation_Platform` parameters should be used to boot Linux with
4 CPUs using the AArch64 build of ARM Trusted Firmware.

    <path-to>/Foundation_Platform                   \
    --cores=4                                       \
    --secure-memory                                 \
    --visualization                                 \
    --gicv3                                         \
    --data="<path-to>/<bl1-binary>"@0x0             \
    --data="<path-to>/<FIP-binary>"@0x08000000      \
    --data="<path-to>/<fdt>"@0x83000000             \
    --data="<path-to>/<kernel-binary>"@0x80080000   \
    --block-device="<path-to>/<file-system-image>"

Notes:
*   BL1 is loaded at the start of the Trusted ROM.
*   The Firmware Image Package is loaded at the start of NOR FLASH0.
*   The Linux kernel image and device tree are loaded in DRAM.
*   The default use-case for the Foundation FVP is to use the `--gicv3` option
    and enable the GICv3 device in the model. Note that without this option,
    the Foundation FVP defaults to legacy (Versatile Express) memory map which
    is not supported by ARM Trusted Firmware.

### Running on the AEMv8 Base FVP with reset to BL1 entrypoint

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

    <path-to>/FVP_Base_AEMv8A-AEMv8A                            \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cluster0.NUM_CORES=4                                     \
    -C cluster1.NUM_CORES=4                                     \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the AEMv8 Base FVP (AArch32) with reset to BL1 entrypoint

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of ARM Trusted Firmware.

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
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the Cortex-A57-A53 Base FVP with reset to BL1 entrypoint

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                       \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the Cortex-A32 Base FVP (AArch32) with reset to BL1 entrypoint

The following `FVP_Base_Cortex-A32x4` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of ARM Trusted Firmware.

    <path-to>/FVP_Base_Cortex-A32x4                             \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the AEMv8 Base FVP with reset to BL31 entrypoint

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

    <path-to>/FVP_Base_AEMv8A-AEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBAR=0x04023000                            \
    -C cluster0.cpu1.RVBAR=0x04023000                            \
    -C cluster0.cpu2.RVBAR=0x04023000                            \
    -C cluster0.cpu3.RVBAR=0x04023000                            \
    -C cluster1.cpu0.RVBAR=0x04023000                            \
    -C cluster1.cpu1.RVBAR=0x04023000                            \
    -C cluster1.cpu2.RVBAR=0x04023000                            \
    -C cluster1.cpu3.RVBAR=0x04023000                            \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04023000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

Notes:

*   Since a FIP is not loaded when using BL31 as reset entrypoint, the
    `--data="<path-to><bl31|bl32|bl33-binary>"@<base-address-of-binary>`
    parameter is needed to load the individual bootloader images in memory.
    BL32 image is only needed if BL31 has been built to expect a Secure-EL1
    Payload.

*   The `-C cluster<X>.cpu<Y>.RVBAR=@<base-address-of-bl31>` parameter, where
    X and Y are the cluster and CPU numbers respectively, is used to set the
    reset vector for each core.

*   Changing the default value of `ARM_TSP_RAM_LOCATION` will also require
    changing the value of
    `--data="<path-to><bl32-binary>"@<base-address-of-bl32>` to the new value of
    `BL32_BASE`.

### Running on the AEMv8 Base FVP (AArch32) with reset to SP_MIN entrypoint

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of ARM Trusted Firmware.

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
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

Note: The load address of `<bl32-binary>` depends on the value `BL32_BASE`.
It should match the address programmed into the RVBAR register as well.

### Running on the Cortex-A57-A53 Base FVP with reset to BL31 entrypoint

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the AArch64 build of ARM Trusted Firmware.

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                        \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBARADDR=0x04023000                        \
    -C cluster0.cpu1.RVBARADDR=0x04023000                        \
    -C cluster0.cpu2.RVBARADDR=0x04023000                        \
    -C cluster0.cpu3.RVBARADDR=0x04023000                        \
    -C cluster1.cpu0.RVBARADDR=0x04023000                        \
    -C cluster1.cpu1.RVBARADDR=0x04023000                        \
    -C cluster1.cpu2.RVBARADDR=0x04023000                        \
    -C cluster1.cpu3.RVBARADDR=0x04023000                        \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04023000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04001000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

### Running on the Cortex-A32 Base FVP (AArch32) with reset to SP_MIN entrypoint

The following `FVP_Base_Cortex-A32x4` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of ARM Trusted Firmware.

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
    --data cluster0.cpu0="<path-to>/<fdt>"@0x83000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    -C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>"

10.  Running the software on Juno
---------------------------------

This version of the ARM Trusted Firmware has been tested on Juno r0 and Juno r1.

To execute the software stack on Juno, the version of the Juno board recovery
image indicated in the [Linaro Release Notes] must be installed. If you have an
earlier version installed or are unsure which version is installed, please
re-install the recovery image by following the [Instructions for using Linaro's
deliverables on Juno][Juno Instructions].

### Preparing Trusted Firmware images

After building Trusted Firmware, the files `bl1.bin` and `fip.bin` need copying
to the `SOFTWARE/` directory of the Juno SD card.

### Other Juno software information

Please visit the [ARM Platforms Portal] to get support and obtain any other Juno
software information. Please also refer to the [Juno Getting Started Guide] to
get more detailed information about the Juno ARM development platform and how to
configure it.

### Testing SYSTEM SUSPEND on Juno

The SYSTEM SUSPEND is a PSCI API which can be used to implement system suspend
to RAM. For more details refer to section 5.16 of [PSCI]. To test system suspend
on Juno, at the linux shell prompt, issue the following command:

    echo +10 > /sys/class/rtc/rtc0/wakealarm
    echo -n mem > /sys/power/state

The Juno board should suspend to RAM and then wakeup after 10 seconds due to
wakeup interrupt from RTC.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved._


[Firmware Design]:             firmware-design.md
[ARM FVP website]:             http://www.arm.com/fvp
[Linaro Release Notes]:        https://community.arm.com/docs/DOC-10952#jive_content_id_Linaro_Release_1606
[ARM Platforms Portal]:        https://community.arm.com/groups/arm-development-platforms
[Linaro SW Instructions]:      https://community.arm.com/docs/DOC-10803
[Juno Instructions]:           https://community.arm.com/docs/DOC-10804
[FVP Instructions]:            https://community.arm.com/docs/DOC-10831
[Juno Getting Started Guide]:  http://infocenter.arm.com/help/topic/com.arm.doc.dui0928e/DUI0928E_juno_arm_development_platform_gsg.pdf
[DS-5]:                        http://www.arm.com/products/tools/software-tools/ds-5/index.php
[mbed TLS Repository]:         https://github.com/ARMmbed/mbedtls.git
[mbed TLS Security Center]:    https://tls.mbed.org/security
[PSCI]:                        http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf "Power State Coordination Interface PDD (ARM DEN 0022C)"
[Trusted Board Boot]:          trusted-board-boot.md
[Firmware Update]:             ./firmware-update.md
[PSCI Lib Integration]:        ./psci-lib-integration-guide.md
