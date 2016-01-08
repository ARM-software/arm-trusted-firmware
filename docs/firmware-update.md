ARM Trusted Firmware - Firmware Update Design Guide
===================================================

Contents :

1.  [Introduction](#1--introduction)
2.  [FWU Overview](#2--fwu-overview)
3.  [Image Identification](#3--image-identification)
4.  [FWU State Machine](#4--fwu-state-machine)
5.  [BL1 SMC Interface](#5--bl1-smc-interface)

- - - - - - - - - - - - - - - - - -

1.  Introduction
----------------

This document describes the design of the Firmware Update (FWU) feature, which
enables authenticated firmware to update firmware images from external
interfaces such as USB, UART, SD-eMMC, NAND, NOR or Ethernet to SoC Non-Volatile
memories such as NAND Flash, LPPDR2-NVM or any memory determined by the
platform. This feature functions even when the current firmware in the system
is corrupt or missing; it therefore may be used as a recovery mode. It may also
be complemented by other, higher level firmware update software.

FWU implements a specific part of the Trusted Board Boot Requirements (TBBR)
specification, ARM DEN0006C-1. It should be used in conjunction with the
[Trusted Board Boot] design document, which describes the image authentication
parts of the Trusted Firmware (TF) TBBR implementation.

### Scope

This document describes the secure world FWU design. It is beyond its scope to
describe how normal world FWU images should operate. To implement normal world
FWU images, please refer to the "Non-Trusted Firmware Updater" requirements in
the TBBR.


2.  FWU Overview
----------------

The FWU boot flow is primarily mediated by BL1. Since BL1 executes in ROM, and
it is usually desirable to minimize the amount of ROM code, the design allows
some parts of FWU to be implemented in other secure and normal world images.
Platform code may choose which parts are implemented in which images but the
general expectation is:

*   BL1 handles:
    *   Detection and initiation of the FWU boot flow.
    *   Copying images from non-secure to secure memory
    *   FWU image authentication
    *   Context switching between the normal and secure world during the FWU
        process.
*   Other secure world FWU images handle platform initialization required by
    the FWU process.
*   Normal world FWU images handle loading of firmware images from external
    interfaces to non-secure memory.

The primary requirements of the FWU feature are:

1.  Export a BL1 SMC interface to interoperate with other FWU images executing
    at other Exception Levels.
2.  Export a platform interface to provide FWU common code with the information
    it needs, and to enable platform specific FWU functionality. See the
    [Porting Guide] for details of this interface.

TF uses abbreviated image terminology for FWU images like for other TF images.
An overview of this terminology can be found [here][TF Image Terminology].

The following diagram shows the FWU boot flow for ARM development platforms.
ARM CSS platforms like Juno have a System Control Processor (SCP), and these
use all defined FWU images. Other platforms may use a subset of these.

![Flow Diagram](diagrams/fwu_flow.png?raw=true)


3.  Image Identification
------------------------

Each FWU image and certificate is identified by a unique ID, defined by the
platform, which BL1 uses to fetch an image descriptor (`image_desc_t`) via a
call to `bl1_plat_get_image_desc()`. The same ID is also used to prepare the
Chain of Trust (Refer to the [Authentication Framework Design][Auth Framework]
for more information).

The image descriptor includes the following information:

*   Executable or non-executable image. This indicates whether the normal world
    is permitted to request execution of a secure world FWU image (after
    authentication). Secure world certificates and non-AP images are examples
    of non-executable images.
*   Secure or non-secure image. This indicates whether the image is
    authenticated/executed in secure or non-secure memory.
*   Image base address and size.
*   Image entry point configuration (an `entry_point_info_t`).
*   FWU image state.

BL1 uses the FWU image descriptors to:

*   Validate the arguments of FWU SMCs
*   Manage the state of the FWU process
*   Initialize the execution state of the next FWU image.


4.  FWU State Machine
---------------------

BL1 maintains state for each FWU image during FWU execution. FWU images at lower
Exception Levels raise SMCs to invoke FWU functionality in BL1, which causes
BL1 to update its FWU image state. The BL1 image states and valid state
transitions are shown in the diagram below. Note that secure images have a more
complex state machine than non-secure images.

![FWU state machine](diagrams/fwu_states.png?raw=true)

The following is a brief description of the supported states:

*   RESET:         This is the initial state of every image at the start of FWU.
                   Authentication failure also leads to this state. A secure
                   image may yield to this state if it has completed execution.

*   COPYING:       This is the state of a secure image while BL1 is copying it
                   in blocks from non-secure to secure memory.

*   COPIED:        This is the state of a secure image when BL1 has completed
                   copying it to secure memory.

*   AUTHENTICATED: This is the state of an image when BL1 has successfully
                   authenticated it.

*   EXECUTED:      This is the state of a secure, executable image when BL1 has
                   passed execution control to it.

*   INTERRUPTED:   This is the state of a secure, executable image after it has
                   requested BL1 to resume normal world execution.


5.  BL1 SMC Interface
---------------------

### BL1_SMC_CALL_COUNT

    Arguments:
        uint32_t function ID : 0x0

    Return:
        uint32_t

This SMC returns the number of SMCs supported by BL1.

### BL1_SMC_UID

    Arguments:
        uint32_t function ID : 0x1

    Return:
        UUID : 32 bits in each of w0-w3 (or r0-r3 for AArch32 callers)

This SMC returns the 128-bit [Universally Unique Identifier][UUID] for the
BL1 SMC service.

### BL1_SMC_VERSION

    Argument:
        uint32_t function ID : 0x3

    Return:
        uint32_t : Bits [31:16] Major Version
                   Bits [15:0] Minor Version

This SMC returns the current version of the BL1 SMC service.

### BL1_SMC_RUN_IMAGE

    Arguments:
        uint32_t           function ID : 0x4
        entry_point_info_t *ep_info

    Return:
        void

    Pre-conditions:
        if (normal world caller) synchronous exception
        if (ep_info not EL3) synchronous exception

This SMC passes execution control to an EL3 image described by the provided
`entry_point_info_t` structure. In the normal TF boot flow, BL2 invokes this SMC
for BL1 to pass execution control to BL31.


### FWU_SMC_IMAGE_COPY

    Arguments:
        uint32_t     function ID : 0x10
        unsigned int image_id
        uintptr_t    image_addr
        unsigned int block_size
        unsigned int image_size

    Return:
        int : 0 (Success)
            : -ENOMEM
            : -EPERM

    Pre-conditions:
        if (image_id is invalid) return -EPERM
        if (image_id is non-secure image) return -EPERM
        if (image_id state is not (RESET or COPYING)) return -EPERM
        if (secure world caller) return -EPERM
        if (source block is in secure memory) return -ENOMEM
        if (source block is not mapped into BL1) return -ENOMEM
        if (image_size > free secure memory) return -ENOMEM

This SMC copies the secure image indicated by `image_id` into secure memory. The
image may be copied in a single block or multiple blocks. In either case, the
total size of the image must be provided in `image_size` when invoking this SMC
the first time for each image. The `image_addr` and `block_size` specify the
source memory block to copy from. If `block_size` >= the size of the remaining
image to copy, then BL1 completes the copy operation and sets the image state
to COPIED. If there is still more to copy, BL1 sets the image state to COPYING.
When using multiple blocks, the source blocks do not necessarily need to be in
contiguous memory.

BL1 returns from exception to the normal world caller.


### FWU_SMC_IMAGE_AUTH

    Arguments:
        uint32_t     function ID : 0x11
        unsigned int image_id
        uintptr_t    image_addr
        unsigned int image_size

    Return:
        int : 0 (Success)
            : -ENOMEM
            : -EPERM
            : -EAUTH

    Pre-conditions:
        if (image_id is invalid) return -EPERM
        if (secure world caller)
            if (image_id state is not RESET) return -EPERM
            if (image_addr/image_size is not mappped into BL1) return -ENOMEM
        else // normal world caller
            if (image_id is secure image)
                if (image_id state is not COPIED) return -EPERM
            else // image_id is non-secure image
                if (image_id state is not RESET) return -EPERM
                if (image_addr/image_size is in secure memory) return -ENOMEM
                if (image_addr/image_size not mappped into BL1) return -ENOMEM

This SMC authenticates the image specified by `image_id`. If the image is in the
 RESET state, BL1 authenticates the image in place using the provided
`image_addr` and `image_size`. If the image is a secure image in the COPIED
state, BL1 authenticates the image from the secure memory that BL1 previously
copied the image into.

BL1 returns from exception to the caller. If authentication succeeds then BL1
sets the image state to AUTHENTICATED. If authentication fails then BL1 returns
the -EAUTH error and sets the image state back to RESET.


### FWU_SMC_IMAGE_EXECUTE

    Arguments:
        uint32_t     function ID : 0x12
        unsigned int image_id

    Return:
        int : 0 (Success)
            : -EPERM

    Pre-conditions:
        if (image_id is invalid) return -EPERM
        if (secure world caller) return -EPERM
        if (image_id is non-secure image) return -EPERM
        if (image_id is non-executable image) return -EPERM
        if (image_id state is not AUTHENTICATED) return -EPERM

This SMC initiates execution of a previously authenticated image specified by
`image_id`, in the other security world to the caller. The current
implementation only supports normal world callers initiating execution of a
secure world image.

BL1 saves the normal world caller's context, sets the secure image state to
EXECUTED, and returns from exception to the secure image.


### FWU_SMC_IMAGE_RESUME

    Arguments:
        uint32_t   function ID : 0x13
        register_t image_param

    Return:
        register_t : image_param (Success)
                   : -EPERM

    Pre-conditions:
        if (normal world caller and no INTERRUPTED secure image) return -EPERM

This SMC resumes execution in the other security world while there is a secure
image in the EXECUTED/INTERRUPTED state.

For normal world callers, BL1 sets the previously interrupted secure image state
to EXECUTED. For secure world callers, BL1 sets the previously executing secure
image state to INTERRUPTED. In either case, BL1 saves the calling world's
context, restores the resuming world's context and returns from exception into
the resuming world. If the call is successful then the caller provided
`image_param` is returned to the resumed world, otherwise an error code is
returned to the caller.


### FWU_SMC_SEC_IMAGE_DONE

    Arguments:
        uint32_t function ID : 0x14

    Return:
        int : 0 (Success)
            : -EPERM

    Pre-conditions:
        if (normal world caller) return -EPERM

This SMC indicates completion of a previously executing secure image.

BL1 sets the previously executing secure image state to the RESET state,
restores the normal world context and returns from exception into the normal
world.


### FWU_SMC_UPDATE_DONE

    Arguments:
        uint32_t   function ID : 0x15
        register_t client_cookie

    Return:
        N/A

This SMC completes the firmware update process. BL1 calls the platform specific
function `bl1_plat_fwu_done`, passing the optional argument `client_cookie` as
a `void *`. The SMC does not return.


- - - - - - - - - - - - - - - - - - - - - - - - - -

_Copyright (c) 2015, ARM Limited and Contributors. All rights reserved._


[Porting Guide]:        ./porting-guide.md
[Auth Framework]:       ./auth-framework.md
[Trusted Board Boot]:   ./trusted-board-boot.md
[TF Image Terminology]: https://github.com/ARM-software/arm-trusted-firmware/wiki/Trusted-Firmware-Image-Terminology
[UUID]:                 https://tools.ietf.org/rfc/rfc4122.txt "A Universally Unique IDentifier (UUID) URN Namespace"
