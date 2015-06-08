ARM Trusted Firmware User Guide
===============================

Contents :

1.  [Introduction](#1--introduction)
2.  [Host machine requirements](#2--host-machine-requirements)
3.  [Tools](#3--tools)
4.  [Getting the Trusted Firmware source code](#4--getting-the-trusted-firmware-source-code)
5.  [Building the Trusted Firmware](#5--building-the-trusted-firmware)
6.  [Building the rest of the software stack](#6--building-the-rest-of-the-software-stack)
7.  [Preparing the images to run on FVP](#7--preparing-the-images-to-run-on-fvp)
8.  [Running the software on FVP](#8--running-the-software-on-fvp)
9.  [Running the software on Juno](#9--running-the-software-on-juno)


1.  Introduction
----------------

This document describes how to build ARM Trusted Firmware and run it with a
tested set of other software components using defined configurations on the Juno
ARM development platform and ARM Fixed Virtual Platform (FVP) models. It is
possible to use other software components, configurations and platforms but that
is outside the scope of this document.

This document should be used in conjunction with the [Firmware Design] and the
[Linaro release notes][Linaro releases].


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

In addition to the prerequisite tools listed on the
[Linaro release notes][Linaro releases], the following tools are needed to use
the ARM Trusted Firmware:

*   `device-tree-compiler` package for building the Flattened Device Tree (FDT)
    source files (`.dts` files) provided with this software.

*   `libssl-dev` package to build the certificate generation tool when support
    for Trusted Board Boot is needed.

*   (Optional) For debugging, ARM [Development Studio 5 (DS-5)][DS-5] v5.21.


4.  Getting the Trusted Firmware source code
--------------------------------------------

The Trusted Firmware source code can be obtained as part of the standard Linaro
releases, which provide a full software stack, including the Trusted Firmware,
normal world firmware, Linux kernel and device tree, file system as well as any
additional micro-controller firmware required by the platform. Please follow the
instructions on the [Linaro release notes][Linaro releases], section 2.2
"Downloading the software sources" and section 2.3 "Downloading the filesystem
binaries".

Note: Both the LSK kernel or the latest tracking kernel can be used along the
ARM Trusted Firmware, choose the one that best suits your needs.

The Trusted Firmware source code can then be found in the `arm-tf/` directory.
This is the full git repository cloned from Github. The revision checked out by
the `repo` tool is indicated by the manifest file. Depending on the manifest
file you're using, this might not be the latest development version. To
synchronize your copy of the repository and get the latest updates, use the
following commands:

    # Change to the Trusted Firmware directory.
    cd arm-tf

    # Download the latest code from Github.
    git fetch github

    # Update your working copy to the latest master.
    # This command will create a local branch master that tracks the remote
    # branch master from Github.
    git checkout --track github/master


Alternatively, the Trusted Firmware source code can be fetched on its own
from GitHub:

    git clone https://github.com/ARM-software/arm-trusted-firmware.git

However, the rest of this document assumes that you got the Trusted Firmware
as part of the Linaro release.


5.  Building the Trusted Firmware
---------------------------------

To build the Trusted Firmware images, change to the root directory of the
Trusted Firmware source tree and follow these steps:

1.  Set the compiler path, specify a Non-trusted Firmware image (BL3-3) and
    a valid platform, and then build:

        CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu- \
        BL33=<path-to>/<bl33_image>                                \
        make PLAT=<platform> all fip

    If `PLAT` is not specified, `fvp` is assumed by default. See the "Summary of
    build options" for more information on available build options.

    The BL3-3 image corresponds to the software that is executed after switching
    to the non-secure world. UEFI can be used as the BL3-3 image. Refer to the
    "Building the rest of the software stack" section below.

    The TSP (Test Secure Payload), corresponding to the BL3-2 image, is not
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
    either `debug` or `release`. A Firmare Image Package (FIP) will be created
    as part of the build. It contains all boot loader images except for
    `bl1.bin`.

    *   `build/<platform>/<build-type>/fip.bin`

    For more information on FIPs, see the "Firmware Image Package" section in
    the [Firmware Design].

2.  (Optional) Some platforms may require a BL3-0 image to boot. This image can
    be included in the FIP when building the Trusted Firmware by specifying the
    `BL30` build option:

        BL30=<path-to>/<bl30_image>

3.  Output binary files `bl1.bin` and `fip.bin` are both required to boot the
    system. How these files are used is platform specific. Refer to the
    platform documentation on how to use the firmware images.

4.  (Optional) Build products for a specific build variant can be removed using:

        make DEBUG=<D> PLAT=<platform> clean

    ... where `<D>` is `0` or `1`, as specified when building.

    The build tree can be removed completely using:

        make realclean

5.  (Optional) Path to binary for certain BL stages (BL2, BL3-1 and BL3-2) can be
    provided by specifying the BLx=<path-to>/<blx_image> where BLx is the BL stage.
    This will bypass the build of the BL component from source, but will include
    the specified binary in the final FIP image. Please note that BL3-2 will be
    included in the build, only if the `SPD` build option is specified.

    For example, specifying BL2=<path-to>/<bl2_image> in the build option, will
    skip compilation of BL2 source in trusted firmware, but include the BL2
    binary specified in the final FIP image.

### Summary of build options

ARM Trusted Firmware build system supports the following build options. Unless
mentioned otherwise, these options are expected to be specified at the build
command line and are not to be modified in any component makefiles. Note that
the build system doesn't track dependency for build options. Therefore, if any
of the build options are changed from a previous build, a clean build must be
performed.

#### Common build options

*   `BL30`: Path to BL3-0 image in the host file system. This image is optional.
    If a BL3-0 image is present then this option must be passed for the `fip`
    target.

*   `BL33`: Path to BL3-3 image in the host file system. This is mandatory for
    `fip` target in case the BL2 from ARM Trusted Firmware is used.

*   `BL2`: This is an optional build option which specifies the path to BL2
    image for the `fip` target. In this case, the BL2 in the ARM Trusted
    Firmware will not be built.

*   `BL31`:  This is an optional build option which specifies the path to
    BL3-1 image for the `fip` target. In this case, the BL3-1 in the ARM
    Trusted Firmware will not be built.

*   `BL32`:  This is an optional build option which specifies the path to
    BL3-2 image for the `fip` target. In this case, the BL3-2 in the ARM
    Trusted Firmware will not be built.

*   `FIP_NAME`: This is an optional build option which specifies the FIP
    filename for the `fip` target. Default is `fip.bin`.

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

*   `ARM_GIC_ARCH`: Choice of ARM GIC architecture version used by the ARM GIC
    driver for implementing the platform GIC API. This API is used
    by the interrupt management framework. Default is 2 (that is, version 2.0).

*   `ARM_CCI_PRODUCT_ID`: Choice of ARM CCI product used by the platform. This
    is used to determine the number of valid slave interfaces available in the
    ARM CCI driver. Default is 400 (that is, CCI-400).

*   `IMF_READ_INTERRUPT_ID`: Boolean flag used by the interrupt management
    framework to enable passing of the interrupt id to its handler. The id is
    read using a platform GIC API. `INTR_ID_UNAVAILABLE` is passed instead if
    this option set to 0. Default is 0.

*   `RESET_TO_BL31`: Enable BL3-1 entrypoint as the CPU reset vector instead
    of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
    entrypoint) or 1 (CPU reset to BL3-1 entrypoint).
    The default value is 0.

*   `CRASH_REPORTING`: A non-zero value enables a console dump of processor
    register state when an unexpected exception occurs during execution of
    BL3-1. This option defaults to the value of `DEBUG` - i.e. by default
    this is only enabled for a debug build of the firmware.

*   `ASM_ASSERTION`: This flag determines whether the assertion checks within
    assembly source files are enabled or not. This option defaults to the
    value of `DEBUG` - that is, by default this is only enabled for a debug
    build of the firmware.

*   `TSP_INIT_ASYNC`: Choose BL3-2 initialization method as asynchronous or
    synchronous, (see "Initializing a BL3-2 Image" section in [Firmware
    Design]). It can take the value 0 (BL3-2 is initialized using
    synchronous method) or 1 (BL3-2 is initialized using asynchronous method).
    Default is 0.

*   `USE_COHERENT_MEM`: This flag determines whether to include the coherent
    memory region in the BL memory map or not (see "Use of Coherent memory in
    Trusted Firmware" section in [Firmware Design]). It can take the value 1
    (Coherent memory region is included) or 0 (Coherent memory region is
    excluded). Default is 1.

*   `TSPD_ROUTE_IRQ_TO_EL3`: A non zero value enables the routing model
    for non-secure interrupts in which they are routed to EL3 (TSPD). The
    default model (when the value is 0) is to route non-secure interrupts
    to S-EL1 (TSP).

*   `TRUSTED_BOARD_BOOT`: Boolean flag to include support for the Trusted Board
    Boot feature. When set to '1', BL1 and BL2 images include support to load
    and verify the certificates and images in a FIP. The default value is '0'.
    Generation and inclusion of certificates in the FIP depends upon the value
    of the `GENERATE_COT` option.

*   `GENERATE_COT`: Boolean flag used to build and execute the `cert_create`
    tool to create certificates as per the Chain of Trust described in
    [Trusted Board Boot].  The build system then calls the `fip_create` tool to
    include the certificates in the FIP. Default value is '0'.

    Specify `TRUSTED_BOARD_BOOT=1` and `GENERATE_COT=1` to include support for
    the Trusted Board Boot Sequence in the BL1 and BL2 images and the FIP.

    Note that if `TRUSTED_BOARD_BOOT=0` and `GENERATE_COT=1`, the BL1 and BL2
    images will not include support for Trusted Board Boot. The FIP will still
    include the key and content certificates. This FIP can be used to verify the
    Chain of Trust on the host machine through other mechanisms.

    Note that if `TRUSTED_BOARD_BOOT=1` and `GENERATE_COT=0`, the BL1 and BL2
    images will include support for Trusted Board Boot, but the FIP will not
    include the key and content certificates, causing a boot failure.

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

*   `BL30_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-0 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `BL31_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-1 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `BL32_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-2 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `BL33_KEY`: This option is used when `GENERATE_COT=1`. It specifies the
    file that contains the BL3-3 private key in PEM format. If `SAVE_KEYS=1`,
    this file name will be used to save the key.

*   `PROGRAMMABLE_RESET_ADDRESS`: This option indicates whether the reset
    vector address can be programmed or is fixed on the platform. It can take
    either 0 (fixed) or 1 (programmable). Default is 0. If the platform has a
    programmable reset address, it is expected that a CPU will start executing
    code directly at the right address, both on a cold and warm reset. In this
    case, there is no need to identify the entrypoint on boot and this has
    implication for `plat_get_my_entrypoint()` platform porting interface.
    (see the [Porting Guide] for details)

*   `PSCI_EXTENDED_STATE_ID`: As per PSCI1.0 Specification, there are 2 formats
    possible for the PSCI power-state parameter viz original and extended
    State-ID formats. This flag if set to 1, configures the generic PSCI layer
    to use the extended format. The default value of this flag is 0, which
    means by default the original power-state format is used by the PSCI
    implementation. This flag should be specified by the platform makefile
    and it governs the return value of PSCI_FEATURES API for CPU_SUSPEND
    smc function id.

*   `WARN_DEPRECATED`: This option decides whether to warn the usage of
    deprecated platform APIs and context management helpers within Trusted
    Firmware. It can take the value 1 (warn the use of deprecated APIs) or
    0. The default is 0.

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

#### ARM CSS platform specific build options

*   `CSS_DETECT_PRE_1_7_0_SCP`: Boolean flag to detect SCP version
    incompatibility. Version 1.7.0 of the SCP firmware made a non-backwards
    compatible change to the MTL protocol, used for AP/SCP communication.
    Trusted Firmware no longer supports earlier SCP versions. If this option is
    set to 1 then Trusted Firmware will detect if an earlier version is in use.
    Default is 1.


### Creating a Firmware Image Package

FIPs are automatically created as part of the build instructions described in
the previous section. It is also possible to independently build the FIP
creation tool and FIPs if required. To do this, follow these steps:

Build the tool:

    make -C tools/fip_create

It is recommended to remove the build artifacts before rebuilding:

    make -C tools/fip_create clean

Create a Firmware package that contains existing BL2 and BL3-1 images:

    # fip_create --help to print usage information
    # fip_create <fip_name> <images to add> [--dump to show result]
    ./tools/fip_create/fip_create fip.bin --dump \
       --bl2 build/<platform>/debug/bl2.bin --bl31 build/<platform>/debug/bl31.bin

     Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x81E8
      file: 'build/<platform>/debug/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x8270, size=0xC218
      file: 'build/<platform>/debug/bl31.bin'
    ---------------------------
    Creating "fip.bin"

View the contents of an existing Firmware package:

    ./tools/fip_create/fip_create fip.bin --dump

     Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x81E8
    - EL3 Runtime Firmware BL3-1: offset=0x8270, size=0xC218
    ---------------------------

Existing package entries can be individially updated:

    # Change the BL2 from Debug to Release version
    ./tools/fip_create/fip_create fip.bin --dump \
      --bl2 build/<platform>/release/bl2.bin

    Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0x88, size=0x7240
      file: 'build/<platform>/release/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x72C8, size=0xC218
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


### Building the Test Secure Payload

The TSP is coupled with a companion runtime service in the BL3-1 firmware,
called the TSPD. Therefore, if you intend to use the TSP, the BL3-1 image
must be recompiled as well. For more information on SPs and SPDs, see the
"Secure-EL1 Payloads and Dispatchers" section in the [Firmware Design].

First clean the Trusted Firmware build directory to get rid of any previous
BL3-1 binary. Then to build the TSP image and include it into the FIP use:

    CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-linux-gnu- \
    BL33=<path-to>/<bl33_image>                                \
    make PLAT=<platform> SPD=tspd all fip

An additional boot loader binary file is created in the `build` directory:

*   `build/<platform>/<build-type>/bl32.bin`

The FIP will now contain the additional BL3-2 image. Here is an example
output from an FVP build in release mode including BL3-2 and using
FVP_AARCH64_EFI.fd as BL3-3 image:

    Firmware Image Package ToC:
    ---------------------------
    - Trusted Boot Firmware BL2: offset=0xD8, size=0x6000
      file: './build/fvp/release/bl2.bin'
    - EL3 Runtime Firmware BL3-1: offset=0x60D8, size=0x9000
      file: './build/fvp/release/bl31.bin'
    - Secure Payload BL3-2 (Trusted OS): offset=0xF0D8, size=0x3000
      file: './build/fvp/release/bl32.bin'
    - Non-Trusted Firmware BL3-3: offset=0x120D8, size=0x280000
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


### Building a FIP image with support for Trusted Board Boot

The Trusted Board Boot feature is described in [Trusted Board Boot]. The
following steps should be followed to build a FIP image with support for this
feature.

1.  Fulfill the dependencies of the `mbedtls` cryptographic and image parser
    modules by checking out the tag `mbedtls-1.3.11` from the
    [mbedTLS Repository].

    The `drivers/auth/mbedtls/mbedtls_*.mk` files contain the list of mbedTLS
    source files the modules depend upon.
    `include/drivers/auth/mbedtls/mbedtls_config.h` contains the configuration
    options required to build the mbedTLS sources.

    Note that the mbedTLS library is licensed under the GNU GPL version 2
    or later license. Using mbedTLS source code will affect the licensing of
    Trusted Firmware binaries that are built using this library.

2.  Ensure that the following command line variables are set while invoking
    `make` to build Trusted Firmware:

    *   `MBEDTLS_DIR=<path of the directory containing mbedTLS sources>`
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
        MBEDTLS_DIR=<path of the directory containing mbedTLS sources>  \
        make PLAT=<platform> TRUSTED_BOARD_BOOT=1 GENERATE_COT=1        \
        ARM_ROTPK_LOCATION=devel_rsa                                    \
        ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem        \
        all fip

    The result of this build will be the bl1.bin and the fip.bin binaries, with
    the difference that the FIP will include the certificates corresponding to
    the Chain of Trust described in the TBBR-client document. These certificates
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


7.  Preparing the images to run on FVP
--------------------------------------

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


8.  Running the software on FVP
-------------------------------

This version of the ARM Trusted Firmware has been tested on the following ARM
FVPs (64-bit versions only).

*   `Foundation_Platform` (Version 9.1, Build 9.1.33)
*   `FVP_Base_AEMv8A-AEMv8A` (Version 6.2, Build 0.8.6202)
*   `FVP_Base_Cortex-A57x4-A53x4` (Version 6.2, Build 0.8.6202)
*   `FVP_Base_Cortex-A57x1-A53x1` (Version 6.2, Build 0.8.6202)
*   `FVP_Base_Cortex-A57x2-A53x4` (Version 6.2, Build 0.8.6202)

NOTE: The build numbers quoted above are those reported by launching the FVP
with the `--version` parameter.

NOTE: The software will not work on Version 1.0 of the Foundation FVP.
The commands below would report an `unhandled argument` error in this case.

NOTE: The Foundation FVP does not provide a debugger interface.

Please refer to the FVP documentation for a detailed description of the model
parameter options. A brief description of the important ones that affect the
ARM Trusted Firmware and normal world software behavior is provided below.

The Foundation FVP is a cut down version of the AArch64 Base FVP. It can be
downloaded for free from [ARM's website][ARM FVP website].


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

8.  Changing the default value of `FVP_SHARED_DATA_LOCATION` will also require
    changing the value of
    `--data="<path-to><bl31-binary>"@<base-address-of-bl31>` and
    `-C cluster<X>.cpu<X>.RVBAR=@<base-address-of-bl31>`, to the new value of
    `BL31_BASE`.

9.  Changing the default value of `FVP_TSP_RAM_LOCATION` will also require
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

### Running on the AEMv8 Base FVP with reset to BL3-1 entrypoint

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

### Running on the Cortex-A57-A53 Base FVP with reset to BL3-1 entrypoint

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

NOTE: If the legacy VE GIC memory map is used, then the corresponding FDT and
BL3-3 images should be used.

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


9.  Running the software on Juno
--------------------------------

This version of the ARM Trusted Firmware has been tested on Juno r0 and Juno r1.

To execute the versions of software components on Juno referred to in this
document, the latest Juno board recovery image must be installed. If you
have an earlier version installed or are unsure which version is installed,
follow the recovery image update instructions in the [Juno Software Guide]
on the [ARM Connected Community] website. The latest Juno board recovery image
can be obtained from [Linaro releases], see section 2.7 "Using prebuilt
binaries".

### Preparing Trusted Firmware images

The Juno platform requires a BL0 and a BL30 image to boot up. The BL0 image
contains the ROM firmware that runs on the SCP (System Control Processor),
whereas the BL30 image contains the SCP Runtime firmware. Both images are
embedded within the Juno board recovery image, these are the files `bl0.bin`
and `bl30.bin`.

The BL30 file must be part of the FIP image. Therefore, its path must be
supplied using the `BL30` variable on the command line when building the
FIP. Please refer to the section "Building the Trusted Firmware".

After building Trusted Firmware, the files `bl1.bin` and `fip.bin` need copying
to the `SOFTWARE/` directory as explained in the [Juno Software Guide].

### Other Juno software information

Please refer to the [Juno Software Guide] to:

*   Install and run the Juno binaries on the board
*   Obtain any other Juno software information


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved._


[Firmware Design]:             firmware-design.md
[Linaro releases]:             http://releases.linaro.org/15.06/members/arm/platforms
[ARM FVP website]:             http://www.arm.com/fvp
[ARM Connected Community]:     http://community.arm.com
[Juno Software Guide]:         http://community.arm.com/docs/DOC-8396
[DS-5]:                        http://www.arm.com/products/tools/software-tools/ds-5/index.php
[mbedTLS Repository]:          https://github.com/ARMmbed/mbedtls.git
[Porting Guide]:               ./porting-guide.md
[Trusted Board Boot]:          trusted-board-boot.md
