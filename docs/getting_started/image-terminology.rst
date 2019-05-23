Image Terminology
=================

This page contains the current name, abbreviated name and purpose of the various
images referred to in the Trusted Firmware project.

General Notes
-------------

- Some of the names and abbreviated names have changed to accomodate new
  requirements. The changed names are as backward compatible as possible to
  minimize confusion. Where applicable, the previous names are indicated. Some
  code, documentation and build artefacts may still refer to the previous names;
  these will inevitably take time to catch up.

- The main name change is to prefix each image with the processor it corresponds
  to (for example ``AP_``, ``SCP_``, ...). In situations where there is no
  ambiguity (for example, within AP specific code/documentation), it is
  permitted to omit the processor prefix (for example, just BL1 instead of
  ``AP_BL1``).

- Previously, the format for 3rd level images had 2 forms; ``BL3`` was either
  suffixed with a dash ("-") followed by a number (for example, ``BL3-1``) or a
  subscript number, depending on whether rich text formatting was available.
  This was confusing and often the dash gets omitted in practice. Therefore the
  new form is to just omit the dash and not use subscript formatting.

- The names no longer contain dash ("-") characters at all. In some places (for
  example, function names) it's not possible to use this character. All dashes
  are either removed or replaced by underscores ("_").

- The abbreviation BL stands for BootLoader. This is a historical anomaly.
  Clearly, many of these images are not BootLoaders, they are simply firmware
  images. However, the BL abbreviation is now widely used and is retained for
  backwards compatibility.

- The image names are not case sensitive. For example, ``bl1`` is
  interchangeable with ``BL1``, although mixed case should be avoided.

Trusted Firmware Images
-----------------------

AP Boot ROM: ``AP_BL1``
~~~~~~~~~~~~~~~~~~~~~~~

Typically, this is the first code to execute on the AP and cannot be modified.
Its primary purpose is to perform the minimum intialization necessary to load
and authenticate an updateable AP firmware image into an executable RAM
location, then hand-off control to that image.

AP RAM Firmware: ``AP_BL2``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the 2nd stage AP firmware. It is currently also known as the "Trusted
Boot Firmware". Its primary purpose is to perform any additional initialization
required to load and authenticate all 3rd level firmware images into their
executable RAM locations, then hand-off control to the EL3 Runtime Firmware.

EL3 Runtime Firmware: ``AP_BL31``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Also known as "SoC AP firmware" or "EL3 monitor firmware". Its primary purpose
is to handle transitions between the normal and secure world.

Secure-EL1 Payload (SP): ``AP_BL32``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Typically this is a TEE or Trusted OS, providing runtime secure services to the
normal world. However, it may refer to a more abstract Secure-EL1 Payload (SP).
Note that this abbreviation should only be used in systems where there is a
single or primary image executing at Secure-EL1. In systems where there are
potentially multiple SPs and there is no concept of a primary SP, this
abbreviation should be avoided; use the recommended **Other AP 3rd level
images** abbreviation instead.

AP Normal World Firmware: ``AP_BL33``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For example, UEFI or uboot. Its primary purpose is to boot a normal world OS.

Other AP 3rd level images: ``AP_BL3_XXX``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The abbreviated names of the existing 3rd level images imply a load/execution
ordering (for example, ``AP_BL31 -> AP_BL32 -> AP_BL33``).  Some systems may
have additional images and/or a different load/execution ordering. The
abbreviated names of the existing images are retained for backward compatibility
but new 3rd level images should be suffixed with an underscore followed by text
identifier, not a number.

In systems where 3rd level images are provided by different vendors, the
abbreviated name should identify the vendor as well as the image
function. For example, ``AP_BL3_ARM_RAS``.

SCP Boot ROM: ``SCP_BL1`` (previously ``BL0``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Typically, this is the first code to execute on the SCP and cannot be modified.
Its primary purpose is to perform the minimum intialization necessary to load
and authenticate an updateable SCP firmware image into an executable RAM
location, then hand-off control to that image. This may be performed in
conjunction with other processor firmware (for example, ``AP_BL1`` and
``AP_BL2``).

This image was previously abbreviated as ``BL0`` but in some systems, the SCP
may directly load/authenticate its own firmware. In these systems, it doesn't
make sense to interleave the image terminology for AP and SCP; both AP and SCP
Boot ROMs are ``BL1`` from their own point of view.

SCP RAM Firmware: ``SCP_BL2`` (previously ``BL3-0``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the 2nd stage SCP firmware. It is currently also known as the "SCP
runtime firmware" but it could potentially be an intermediate firmware if the
SCP needs to load/authenticate multiple 3rd level images in future.

This image was previously abbreviated as BL3-0 but from the SCP's point of view,
this has always been the 2nd stage firmware. The previous name is too
AP-centric.

Firmware Update (FWU) Images
----------------------------

The terminology for these images has not been widely adopted yet but they have
to be considered in a production Trusted Board Boot solution.

AP Firmware Update Boot ROM: ``AP_NS_BL1U``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Typically, this is the first normal world code to execute on the AP during a
firmware update operation, and cannot be modified. Its primary purpose is to
load subequent firmware update images from an external interface and communicate
with ``AP_BL1`` to authenticate those images.

During firmware update, there are (potentially) multiple transitions between the
secure and normal world. The "level" of the BL image is relative to the world
it's in so it makes sense to encode "NS" in the normal world images. The absence
of "NS" implies a secure world image.

AP Firmware Update Config: ``AP_BL2U``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This image does the minimum necessary AP secure world configuration required to
complete the firmware update operation. It is potentially a subset of ``AP_BL2``
functionality.

SCP Firmware Update Config: ``SCP_BL2U`` (previously ``BL2-U0``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This image does the minimum necessary SCP secure world configuration required to
complete the firmware update operation. It is potentially a subset of
``SCP_BL2`` functionality.

AP Firmware Updater: ``AP_NS_BL2U`` (previously ``BL3-U``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the 2nd stage AP normal world firmware updater. Its primary purpose is
to load a new set of firmware images from an external interface and write them
into non-volatile storage.

Other Processor Firmware Images
-------------------------------

Some systems may have additional processors to the AP and SCP. For example, a
Management Control Processor (MCP). Images for these processors should follow
the same terminology, with the processor abbreviation prefix, followed by
underscore and the level of the firmware image.

For example,

MCP Boot ROM: ``MCP_BL1``
~~~~~~~~~~~~~~~~~~~~~~~~~

MCP RAM Firmware: ``MCP_BL2``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
