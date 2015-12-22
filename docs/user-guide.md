ARM Trusted Firmware User Guide
===============================

Contents :

1.  [Introduction](#1--introduction)
2.  [Host machine requirements](#2--host-machine-requirements)
3.  [Tools](#3--tools)
4.  [Getting the Trusted Firmware source code](#4--getting-the-trusted-firmware-source-code)
5.  [Building the Trusted Firmware](#5--building-the-trusted-firmware)
6.  [Building the rest of the software stack](#6--building-the-rest-of-the-software-stack)
7.  [EL3 payloads alternative boot flow](#7--el3-payloads-alternative-boot-flow)
8.  [Preparing the images to run on FVP](#8--preparing-the-images-to-run-on-fvp)
9.  [Running the software on FVP](#9--running-the-software-on-fvp)
10. [Running the software on Juno](#10--running-the-software-on-juno)
11. [Changes required for booting Linux on FVP in GICv3 mode](#11--changes-required-for-booting-linux-on-fvp-in-gicv3-mode)


1.  Introduction
----------------

This document describes how to build ARM Trusted Firmware and run it with a
tested set of other software components using defined configurations on the Juno
ARM development platform and ARM Fixed Virtual Platform (FVP) models. It is
possible to use other software components, configurations and platforms but that
is outside the scope of this document.

This document should be used in conjunction with the [Firmware Design] and the
[Instructions for using the Linaro software deliverables][Linaro SW Instructions].


2.  Host machine requirements
-----------------------------

The minimum recommended machine specification for building the software and
running the FVP models is a dual-core processor running at 2GHz with 12GB of
RAM.  For best performance, use a machine with a quad-core processor running at
2.6GHz with 16GB of RAM.

The software has been tested on Ubuntu 14.04 LTS (64-bit). Packages used for
building the software were installed from that distribution unless otherwise
specified.

3.  Tools
---------

In addition to the mandatory prerequisite tools listed in the [instructions for
using the Linaro software deliverables][Linaro SW Instructions], the following
optional tools may be needed:

*   `device-tree-compiler` package if you need to rebuild the Flattened Device
    Tree (FDT) source files (`.dts` files) provided with this software.

*   For debugging, ARM [Development Studio 5 (DS-5)][DS-5] v5.22.


4.  Getting the Trusted Firmware source code
--------------------------------------------

The Trusted Firmware (TF) source code can be obtained as part of the standard
Linaro releases, which provide a full software stack, including TF, normal
world firmware, Linux kernel and device tree, file system as well as any
additional micro-controller firmware required by the platform. This TF version
is tested with the [Linaro 15.10 Release][Linaro Release Notes].

Note 1: Both the LSK kernel or the latest tracking kernel can be used with TF;
choose the one that best suits your needs.

Note 2: Currently to run the latest tracking kernel on FVP with GICv3 driver,
some modifications are required to UEFI. Refer
[here](#11--changes-required-for-booting-linux-on-fvp-in-gicv3-mode)
for more details.

The TF source code will then be in `arm-tf/`. This is the upstream git
repository cloned from GitHub. The revision checked out by the `repo` tool is
indicated by the manifest file. Depending on the manifest file you're using,
this might not be the latest upstream version. To synchronize your copy of the
repository and get the latest updates, use the following commands:

    # Change to the Trusted Firmware directory.
    cd arm-tf

    # Download the latest code from GitHub.
    git fetch github

    # Update your working copy to the latest master.
    # This command will create a local branch master that tracks the remote
    # branch master from GitHub.
    git checkout --track github/master


Alternatively, the TF source code can be separately cloned from the upstream
GitHub repository:

    git clone https://github.com/ARM-software/arm-trusted-firmware.git


5.  Building the Trusted Firmware
---------------------------------

To build the Trusted Firmware images, change to the root directory of the
Trusted Firmware source tree and follow these steps:

1.  Set the compiler path, specify a Non-trusted Firmware image (BL33) and
    a valid platform, and then build:

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu- \
        BL33=<path-to>/<bl33_image>                                \
        make PLAT=<platform> all fip

    If `PLAT` is not specified, `fvp` is assumed by default. See the "Summary of
    build options" for more information on available build options.

    The BL33 image corresponds to the software that is executed after switching
    to the non-secure world. UEFI can be used as the BL33 image. Refer to the
    "Building the rest of the software stack" section below.

    The TSP (Test Secure Payload), corresponding to the BL32 image, is not
    compiled in by default. Refer to the "Building the Test Secure Payload"
    section below.

    By default this produces a release version of the build. To produce a debug
    version instead, refer to the "Debugging options" section below.

    The build process creates products in a `build` directory tree, building
    the objects and binaries for each boot loader stage in separate
    sub-directories.  The following boot loader binary files are created from
    the corresponding ELF files:

    *   `build/<platform>/<build-type>/bl1.bin`
    *   `build/<platform>/<build-type>/bl2.bin`
    *   `build/<platform>/<build-type>/bl31.bin`

    where `<platform>` is the name of the chosen platform and `<build-type>` is
    either `debug` or `release`. A Firmware Image Package (FIP) will be created
    as part of the build. It contains all boot loader images except for
    `bl1.bin`.

    *   `build/<platform>/<build-type>/fip.bin`

    For more information on FIPs, see the "Firmware Image Package" section in
    the [Firmware Design].

2.  (Optional) Some platforms may require a SCP_BL2 image to boot. This image can
    be included in the FIP when building the Trusted Firmware by specifying the
    `SCP_BL2` build option:

        SCP_BL2=<path-to>/<scp_bl2_image>

3.  Output binary files `bl1.bin` and `fip.bin` are both required to boot the
    system. How these files are used is platform specific. Refer to the
    platform documentation on how to use the firmware images.

4.  (Optional) Build products for a specific build variant can be removed using:

        make DEBUG=<D> PLAT=<platform> clean

    ... where `<D>` is `0` or `1`, as specified when building.

    The build tree can be removed completely using:

        make realclean

5.  (Optional) Path to binary for certain BL stages (BL2, BL31 and BL32) can be
    provided by specifying the BLx=<path-to>/<blx_image> where BLx is the BL stage.
    This will bypass the build of the BL component from source, but will include
    the specified binary in the final FIP image. Please note that BL32 will be
    included in the build, only if the `SPD` build option is specified.

    For example, specifying `BL2=<path-to>/<bl2_image>` in the build option,
    will skip compilation of BL2 source in trusted firmware, but include the BL2
    binary specified in the final FIP image.

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

*   `CROSS_COMPILE`: Prefix to toolchain binaries. Please refer to examples in
    this document for usage.

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

*   `SPD`: Choose a Secure Payload Dispatcher component to be built into the
    Trusted Firmware. The value should be the path to the directory containing
    the SPD source, relative to `services/spd/`; the directory is expected to
    contain a makefile called `<spd-value>.mk`.

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
    [Trusted Board Boot].  The build system then calls the `fip_create` tool to
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

*   `PL011_GENERIC_UART`: Boolean option to indicate the PL011 driver that
    the underlying hardware is not a full PL011 UART but a minimally compliant
    generic UART, which is a subset of the PL011. The driver will not access
    any register that is not part of the SBSA generic UART specification.
    Default value is 0 (a full PL011 compliant UART is present).

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

#### ARM CSS platform specific build options

*   `CSS_DETECT_PRE_1_7_0_SCP`: Boolean flag to detect SCP version
    incompatibility. Version 1.7.0 of the SCP firmware made a non-backwards
    compatible change to the MTL protocol, used for AP/SCP communication.
    Trusted Firmware no longer supports earlier SCP versions. If this option is
    set to 1 then Trusted Firmware will detect if an earlier version is in use.
    Default is 1.

#### ARM FVP platform specific build options

*   `FVP_USE_GIC_DRIVER`   : Selects the GIC driver to be built. Options:
    -    `FVP_GICV2`       : The GICv2 only driver is selected
    -    `FVP_GICV3`       : The GICv3 only driver is selected (default option)
    -    `FVP_GICV3_LEGACY`: The Legacy GICv3 driver is selected (deprecated).

    Note that if the FVP is configured for legacy VE memory map, then ARM
    Trusted Firmware must be compiled with GICv2 only driver using
    `FVP_USE_GIC_DRIVER=FVP_GICV2` build option.

### Creating a Firmware Image Package

FIPs are automatically created as part of the build instructions described in
the previous section. It is also possible to independently build the FIP
creation tool and FIPs if required. To do this, follow these steps:

Build the tool:

    make -C tools/fip_create

It is recommended to remove the build artifacts before rebuilding:

    make -C tools/fip_create clean

Create a Firmware package that contains existing BL2 and BL31 images:

    # fip_create --help to print usage information
    # fip_create <fip_name> <images to add> [--dump to show result]
    ./tools/fip_create/fip_create fip.bin --dump \
       --bl2 build/<platform>/debug/bl2.bin --bl31 build/<platform>/debug/bl31.bin

     Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x81E8
      file: 'build/<platform>/debug/bl2.bin'
    - EL3 Runtime Firmware BL31: offset=0x8270, size=0xC218
      file: 'build/<platform>/debug/bl31.bin'
    ---------------------------
    Creating "fip.bin"

View the contents of an existing Firmware package:

    ./tools/fip_create/fip_create fip.bin --dump

     Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x81E8
    - EL3 Runtime Firmware BL31: offset=0x8270, size=0xC218
    ---------------------------

Existing package entries can be individually updated:

    # Change the BL2 from Debug to Release version
    ./tools/fip_create/fip_create fip.bin --dump \
      --bl2 build/<platform>/release/bl2.bin

    Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x7240
      file: 'build/<platform>/release/bl2.bin'
    - EL3 Runtime Firmware BL31: offset=0x72C8, size=0xC218
    ---------------------------
    Updating "fip.bin"


### Debugging options

To compile a debug version and make the build more verbose use

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu- \
    BL33=<path-to>/<bl33_image>                                \
    make PLAT=<platform> DEBUG=1 V=1 all fip

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
    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu- \
    BL33=<path-to>/<bl33_image>                                \
    make PLAT=<platform> DEBUG=1 V=1 all fip

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
BL31 binary. Then to build the TSP image and include it into the FIP use:

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu- \
    BL33=<path-to>/<bl33_image>                                \
    make PLAT=<platform> SPD=tspd all fip

An additional boot loader binary file is created in the `build` directory:

*   `build/<platform>/<build-type>/bl32.bin`

The FIP will now contain the additional BL32 image. Here is an example
output from an FVP build in release mode including BL32 and using
`FVP_AARCH64_EFI.fd` as BL33 image:

    Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0xD8, size=0x6000
      file: './build/fvp/release/bl2.bin'
    - EL3 Runtime Firmware BL31: offset=0x60D8, size=0x9000
      file: './build/fvp/release/bl31.bin'
    - Secure Payload BL32 (Trusted OS): offset=0xF0D8, size=0x3000
      file: './build/fvp/release/bl32.bin'
    - Non-Trusted Firmware BL33: offset=0x120D8, size=0x280000
      file: '../FVP_AARCH64_EFI.fd'
    ---------------------------
    Creating "build/fvp/release/fip.bin"


### Building the Certificate Generation Tool

The `cert_create` tool can be built separately through the following commands:

    $ cd tools/cert_create
    $ make PLAT=<platform> [DEBUG=1] [V=1]

`DEBUG=1` builds the tool in debug mode. `V=1` makes the build process more
verbose. The following command should be used to obtain help about the tool:

    $ ./cert_create -h

The `cert_create` tool is automatically built with the `fip` target when
`GENERATE_COT=1`.


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
    information. This version of TF is tested with tag `mbedtls-2.2.0`.

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

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-      \
        BL33=<path-to>/<bl33_image>                                     \
        MBEDTLS_DIR=<path of the directory containing mbed TLS sources> \
        make PLAT=<platform> TRUSTED_BOARD_BOOT=1 GENERATE_COT=1        \
        ARM_ROTPK_LOCATION=devel_rsa                                    \
        ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
        all fip

    The result of this build will be the bl1.bin and the fip.bin binaries, with
    the difference that the FIP will include the certificates corresponding to
    the Chain of Trust described in the TBBR-client document. These certificates
    can also be found in the output build directory.

3.  The optional FWU_FIP contains any additional images to be loaded from
    Non-Volatile storage during the [Firmware Update] process. To build the
    FWU_FIP, any FWU images required by the platform must be specified on the
    command line. On ARM development platforms like Juno, these are:

    *   NS_BL2U. The AP non-secure Firmware Updater image.
    *   SCP_BL2U. The SCP Firmware Update Configuration image.

    Example of Juno command line for generating both `fwu` and `fwu_fip`
    targets using RSA development:

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf-       \
        BL33=<path-to>/<bl33_image>                                     \
        SCP_BL2=<path-to>/<scp_bl2_image>                               \
        SCP_BL2U=<path-to>/<scp_bl2u_image>                             \
        NS_BL2U=<path-to>/<ns_bl2u_image>                               \
        MBEDTLS_DIR=<path of the directory containing mbed TLS sources> \
        make PLAT=juno TRUSTED_BOARD_BOOT=1 GENERATE_COT=1              \
        ARM_ROTPK_LOCATION=devel_rsa                                    \
        ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
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


6.  Building the rest of the software stack
-------------------------------------------

The Linaro release provides a set of scripts that automate the process of
building all components of the software stack. However, the scripts only support
a limited number of Trusted Firmware build options. Therefore, it is recommended
to modify these scripts to build all components except Trusted Firmware, and
build Trusted Firmware separately as described in the section "Building the
Trusted Firmware" above.

The instructions below are targeted at an OpenEmbedded filesystem.

1.  To exclude Trusted Firmware from the automated build process, edit the
    variant file `build-scripts/variants/<platform>-oe`, where `<platform>`
    is either `fvp` or `juno`. Add the following lines at the end of the file:

        # Disable ARM Trusted Firmware build
        ARM_TF_BUILD_ENABLED=0

2.  Launch the build script:

        CROSS_COMPILE=aarch64-linux-gnu- \
        build-scripts/build-all.sh <platform>-oe

### Preparing the Firmware Image Package

The EDK2 binary should be specified as `BL33` in the `make` command line when
building the Trusted Firmware. See the "Building the Trusted Firmware" section
above. The EDK2 binary for use with the ARM Trusted Firmware can be found here:

    uefi/edk2/Build/ArmVExpress-FVP-AArch64-Minimal/DEBUG_GCC49/FV/FVP_AARCH64_EFI.fd   [for FVP]
    uefi/edk2/Build/ArmJuno/DEBUG_GCC49/FV/BL33_AP_UEFI.fd                              [for Juno]

### Building an alternative EDK2

*   By default, EDK2 is built in debug mode. To build a release version instead,
    change the following line in the variant file:

        UEFI_BUILD_MODE=DEBUG

    into:

        UEFI_BUILD_MODE=RELEASE

*   On FVP, if legacy GICv2 locations are used, the EDK2 platform makefile must
    be updated. This is required as EDK2 does not support probing for the GIC
    location. To do this, first clean the EDK2 build directory:

        build-scripts/build-uefi.sh fvp-oe clean

    Then edit the following file:

        uefi/edk2/ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.mak

    and add the following build flag into the `EDK2_MACROS` variable:

        -D ARM_FVP_LEGACY_GICV2_LOCATION=1

    Then rebuild everything as described above in step 2.

    Finally rebuild the Trusted Firmware to generate a new FIP using the
    instructions in the "Building the Trusted Firmware" section.


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


8.  Preparing the images to run on FVP
--------------------------------------

Note: This section can be ignored when booting an EL3 payload, as no Flattened
Device Tree or kernel image is needed in this case.

### Obtaining the Flattened Device Trees

Depending on the FVP configuration and Linux configuration used, different
FDT files are required. FDTs for the Foundation and Base FVPs can be found in
the Trusted Firmware source directory under `fdts/`. The Foundation FVP has a
subset of the Base FVP components. For example, the Foundation FVP lacks CLCD
and MMC support, and has only one CPU cluster.

Note: It is not recommended to use the FDTs built along the kernel because not
all FDTs are available from there.

*   `fvp-base-gicv2-psci.dtb`

    (Default) For use with both AEMv8 and Cortex-A57-A53 Base FVPs with
    Base memory map configuration.

*   `fvp-base-gicv2legacy-psci.dtb`

    For use with AEMv8 Base FVP with legacy VE GIC memory map configuration.

*   `fvp-base-gicv3-psci.dtb`

    For use with both AEMv8 and Cortex-A57-A53 Base FVPs with Base memory map
    configuration and Linux GICv3 support.

*   `fvp-foundation-gicv2-psci.dtb`

    (Default) For use with Foundation FVP with Base memory map configuration.

*   `fvp-foundation-gicv2legacy-psci.dtb`

    For use with Foundation FVP with legacy VE GIC memory map configuration.

*   `fvp-foundation-gicv3-psci.dtb`

    For use with Foundation FVP with Base memory map configuration and Linux
    GICv3 support.

Copy the chosen FDT blob as `fdt.dtb` to the directory from which the FVP
is launched. Alternatively a symbolic link may be used.

### Preparing the kernel image

Copy the kernel image file `linux/arch/arm64/boot/Image` to the directory from
which the FVP is launched. Alternatively a symbolic link may be used.


9.  Running the software on FVP
-------------------------------

This version of the ARM Trusted Firmware has been tested on the following ARM
FVPs (64-bit versions only).

*   `Foundation_Platform` (Version 9.4, Build 9.4.59)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 7.0, Build 0.8.7004)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 7.0, Build 0.8.7004)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 7.0, Build 0.8.7004)
*   `FVP_Base_Cortex-A57x2-A53x4` (Version 7.0, Build 0.8.7004)

NOTE: The build numbers quoted above are those reported by launching the FVP
with the `--version` parameter.

NOTE: The software will not work on Version 1.0 of the Foundation FVP.
The commands below would report an `unhandled argument` error in this case.

NOTE: The Foundation FVP does not provide a debugger interface.

The Foundation FVP is a cut down version of the AArch64 Base FVP. It can be
downloaded for free from [ARM's website][ARM FVP website].

The Linaro release provides a script to run the software on FVP. However, it
only supports a limited number of model parameter options. Therefore, it is
recommended to launch the FVP manually for all use cases described below.

Please refer to the FVP documentation for a detailed description of the model
parameter options. A brief description of the important ones that affect the ARM
Trusted Firmware and normal world software behavior is provided below.


### Running on the Foundation FVP with reset to BL1 entrypoint

The following `Foundation_Platform` parameters should be used to boot Linux with
4 CPUs using the ARM Trusted Firmware.

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

1.  The `--data="<path-to-some-binary>"@0x...` parameters are used to load
    binaries into memory.

    *   BL1 is loaded at the start of the Trusted ROM.
    *   The Firmware Image Package is loaded at the start of NOR FLASH0.
    *   The Linux kernel image and device tree are loaded in DRAM.

2.  The `--block-device` parameter is used to specify the path to the file
    system image provided to Linux via VirtioBlock. Note that it must point to
    the real file and that a symbolic link to this file cannot be used with the
    FVP.

The default use-case for the Foundation FVP is to enable the GICv3 device in
the model but use the GICv2 FDT, in order for Linux to drive the GIC in GICv2
emulation mode.

### Notes regarding Base FVP configuration options

Please refer to these notes in the subsequent "Running on the Base FVP"
sections.

1.  The `-C bp.flashloader0.fname` parameter is used to load a Firmware Image
    Package at the start of NOR FLASH0 (see the "Building the Trusted Firmware"
    section above).

2.  Using `cache_state_modelled=1` makes booting very slow. The software will
    still work (and run much faster) without this option but this will hide any
    cache maintenance defects in the software.

3.  The `-C bp.virtioblockdevice.image_path` parameter is used to specify the
    path to the file system image provided to Linux via VirtioBlock. Note that
    it must point to the real file and that a symbolic link to this file cannot
    be used with the FVP. Ensure that the FVP doesn't output any error messages.
    If the following error message is displayed:

        ERROR: BlockDevice: Failed to open "<path-to>/<file-system-image>"!

    then make sure the path to the file-system image in the model parameter is
    correct and that read permission is correctly set on the file-system image
    file.

4.  Setting the `-C bp.secure_memory` parameter to `1` is only supported on
    Base FVP versions 5.4 and newer. Setting this parameter to `0` is also
    supported. The `-C bp.tzc_400.diagnostics=1` parameter is optional. It
    instructs the FVP to provide some helpful information if a secure memory
    violation occurs.

5.  The `--data="<path-to-some-binary>"@<base-address-of-binary>` parameter is
    used to load images into Base FVP memory. The base addresses used should
    match the image base addresses used while linking the images. This parameter
    is used to load the Linux kernel image and device tree into DRAM.

6.  This and the following notes only apply when the firmware is built with
    the `RESET_TO_BL31` option.

    The `--data="<path-to><bl31|bl32|bl33-binary>"@<base-address-of-binary>`
    parameter is needed to load the individual bootloader images in memory.
    BL32 image is only needed if BL31 has been built to expect a Secure-EL1
    Payload.

7.  The `-C cluster<X>.cpu<Y>.RVBAR=@<base-address-of-bl31>` parameter, where
    X and Y are the cluster and CPU numbers respectively, is used to set the
    reset vector for each core.

8.  Changing the default value of `ARM_TSP_RAM_LOCATION` will also require
    changing the value of
    `--data="<path-to><bl32-binary>"@<base-address-of-bl32>` to the new value of
    `BL32_BASE`.


### Running on the AEMv8 Base FVP with reset to BL1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the ARM Trusted Firmware.

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

### Running on the Cortex-A57-A53 Base FVP with reset to BL1 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the ARM Trusted Firmware.

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

### Running on the AEMv8 Base FVP with reset to BL31 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_AEMv8A-AEMv8A` parameters should be used to boot Linux
with 8 CPUs using the ARM Trusted Firmware.

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

### Running on the Cortex-A57-A53 Base FVP with reset to BL31 entrypoint

Please read "Notes regarding Base FVP configuration options" section above for
information about some of the options to run the software.

The following `FVP_Base_Cortex-A57x4-A53x4` model parameters should be used to
boot Linux with 8 CPUs using the ARM Trusted Firmware.

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

### Configuring the GICv2 memory map

The Base FVP models support GICv2 with the default model parameters at the
following addresses. The Foundation FVP also supports these addresses when
configured for GICv3 in GICv2 emulation mode.

    GICv2 Distributor Interface     0x2f000000
    GICv2 CPU Interface             0x2c000000
    GICv2 Virtual CPU Interface     0x2c010000
    GICv2 Hypervisor Interface      0x2c02f000

The AEMv8 Base FVP can be configured to support GICv2 at addresses
corresponding to the legacy (Versatile Express) memory map as follows. These are
the default addresses when using the Foundation FVP in GICv2 mode.

    GICv2 Distributor Interface     0x2c001000
    GICv2 CPU Interface             0x2c002000
    GICv2 Virtual CPU Interface     0x2c004000
    GICv2 Hypervisor Interface      0x2c006000

The choice of memory map is reflected in the build variant field (bits[15:12])
in the `SYS_ID` register (Offset `0x0`) in the Versatile Express System
registers memory map (`0x1c010000`).

*   `SYS_ID.Build[15:12]`

    `0x1` corresponds to the presence of the Base GIC memory map. This is the
    default value on the Base FVPs.

*   `SYS_ID.Build[15:12]`

    `0x0` corresponds to the presence of the Legacy VE GIC memory map. This is
    the default value on the Foundation FVP.

This register can be configured as described in the following sections.

NOTE: If the legacy VE GIC memory map is used, then Trusted Firmware must be
compiled with the GICv2 only driver, and the corresponding FDT and BL33 images
should be used.

#### Configuring AEMv8 Foundation FVP GIC for legacy VE memory map

The following parameters configure the Foundation FVP to use GICv2 with the
legacy VE memory map:

    <path-to>/Foundation_Platform             \
    --cores=4                                 \
    --secure-memory                           \
    --visualization                           \
    --no-gicv3                                \
    --data="<path-to>/<bl1-binary>"@0x0       \
    --data="<path-to>/<FIP-binary>"@0x8000000 \
    --block-device="<path-to>/<file-system-image>"

Explicit configuration of the `SYS_ID` register is not required.

#### Configuring AEMv8 Base FVP GIC for legacy VE memory map

The following parameters configure the AEMv8 Base FVP to use GICv2 with the
legacy VE memory map. They must added to the parameters described in the
"Running on the AEMv8 Base FVP" section above:

    -C cluster0.gic.GICD-offset=0x1000                  \
    -C cluster0.gic.GICC-offset=0x2000                  \
    -C cluster0.gic.GICH-offset=0x4000                  \
    -C cluster0.gic.GICH-other-CPU-offset=0x5000        \
    -C cluster0.gic.GICV-offset=0x6000                  \
    -C cluster0.gic.PERIPH-size=0x8000                  \
    -C cluster1.gic.GICD-offset=0x1000                  \
    -C cluster1.gic.GICC-offset=0x2000                  \
    -C cluster1.gic.GICH-offset=0x4000                  \
    -C cluster1.gic.GICH-other-CPU-offset=0x5000        \
    -C cluster1.gic.GICV-offset=0x6000                  \
    -C cluster1.gic.PERIPH-size=0x8000                  \
    -C gic_distributor.GICD-alias=0x2c001000            \
    -C gicv3.gicv2-only=1                               \
    -C bp.variant=0x0

The `bp.variant` parameter corresponds to the build variant field of the
`SYS_ID` register.  Setting this to `0x0` allows the ARM Trusted Firmware to
detect the legacy VE memory map while configuring the GIC.

### Booting an EL3 payload on FVP

Booting an EL3 payload on FVP requires a couple of changes to the way the
model is normally invoked.

First of all, the EL3 payload image is not part of the FIP and is not loaded by
the Trusted Firmware. Therefore, it must be loaded in memory some other way.
There are 2 ways of doing that:

1.  It can be loaded over JTAG at the appropriate time. The infinite loop
    introduced in BL1 when compiling the Trusted Firmware with
    `SPIN_ON_BL1_EXIT=1` stops execution at the right moment for a debugger to
    take control of the target and load the payload.

2.  It can be pre-loaded in the FVP memory using the following model parameter:

        --data="<path-to-binary>"@<base-address-of-binary>

    The base address provided to the FVP must match the `EL3_PAYLOAD_BASE`
    address used when building the Trusted Firmware.

Secondly, the EL3 payloads boot flow requires the CPUs mailbox to be cleared
at reset for the secondary CPUs holding pen to work properly. Unfortunately,
its reset value is undefined on FVP. One way to clear it is to create an
8-byte file containing all zero bytes and pre-load it into the FVP memory at the
mailbox address (i.e. `0x04000000`) using the same `--data` FVP parameter as
described above.

The following command creates such a file called `mailbox.dat`:

    dd if=/dev/zero of=mailbox.dat bs=1 count=8


10.  Running the software on Juno
---------------------------------

This version of the ARM Trusted Firmware has been tested on Juno r0 and Juno r1.

To execute the software stack on Juno, the version of the Juno board recovery
image indicated in the [Linaro Release Notes] must be installed. If you have an
earlier version installed or are unsure which version is installed, please
re-install the recovery image by following the [Instructions for using Linaro's
deliverables on Juno][Juno Instructions].

### Preparing Trusted Firmware images

The Juno platform requires a SCP_BL1 and a SCP_BL2 image to boot up. The
SCP_BL1 image contains the ROM firmware that runs on the SCP (System Control
Processor), whereas the SCP_BL2 image contains the SCP Runtime firmware. Both
images are embedded within the Juno board recovery image, these are the files
`bl0.bin` and `bl30.bin`, respectively. Please note that these filenames still
use the old terminology.

The SCP_BL2 file must be part of the FIP image. Therefore, its path must be
supplied using the `SCP_BL2` variable on the command line when building the
FIP. Please refer to the section "Building the Trusted Firmware".

After building Trusted Firmware, the files `bl1.bin` and `fip.bin` need copying
to the `SOFTWARE/` directory of the Juno SD card.

### Other Juno software information

Please visit the [ARM Platforms Portal] to get support and obtain any other Juno
software information. Please also refer to the [Juno Getting Started Guide] to
get more detailed information about the Juno ARM development platform and how to
configure it.

### Testing SYSTEM SUSPEND on Juno

The SYSTEM SUSPEND is a PSCI API which can be used to implement system suspend
to RAM. For more details refer to section 5.16 of [PSCI]. The [Linaro Release
Notes] point to the required SCP and motherboard firmware binaries supporting
this feature on Juno. The mainline linux kernel does not yet have support for
this feature on Juno but it is queued to be merged in v4.4. Till that becomes
available, the feature can be tested by using a custom kernel built from the
following repository:

    git clone git://git.kernel.org/pub/scm/linux/kernel/git/lpieralisi/linux.git
    cd linux
    git checkout firmware/psci-1.0

Configure the linux kernel:

    export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu-
    make ARCH=arm64 defconfig

The feature is tested conveniently by using the RTC. Enable the RTC driver in
menuconfig

    make ARCH=arm64 menuconfig

The PL031 RTC driver can be enabled at the following location in menuconfig

    ARM AMBA PL031 RTC
      |   Location:
      |     -> Device Drivers
      |       -> Real Time Clock

Build the kernel

    make ARCH=arm64 Image -j8

Replace the kernel image in the `SOFTWARE/` directory of the Juno SD card with
the `Image` from `arch/arm64/boot/` of the linux directory.

Reset the board and wait for it to boot. At the shell prompt issue the
following command:

    echo +10 > /sys/class/rtc/rtc1/wakealarm
    echo -n mem > /sys/power/state

The Juno board should suspend to RAM and then wakeup after 10 seconds due to
wakeup interrupt from RTC.


11.  Changes required for booting Linux on FVP in GICv3 mode
------------------------------------------------------------

In case the TF FVP port is built with the build option
`FVP_USE_GIC_DRIVER=FVP_GICV3`, then the GICv3 hardware cannot be used in
GICv2 legacy mode. The default build of UEFI for FVP in
[latest tracking kernel][Linaro Release Notes] configures GICv3 in GICv2 legacy
mode. This can be changed by setting the build flag
`gArmTokenSpaceGuid.PcdArmGicV3WithV2Legacy` to FALSE in
`uefi/edk2/ArmPlatformPkg/ArmVExpressPkg/ArmVExpress-FVP-AArch64.dsc`.

Recompile UEFI as mentioned [here][FVP Instructions].

The GICv3 DTBs found in ARM Trusted Firmware source directory can be
used to test the GICv3 kernel on the respective FVP models.

- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved._


[Firmware Design]:             firmware-design.md
[ARM FVP website]:             http://www.arm.com/fvp
[Linaro Release Notes]:        https://community.arm.com/docs/DOC-10952#jive_content_id_Linaro_Release_1510
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
