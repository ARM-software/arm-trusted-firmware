Trusted Board Boot
==================

The `Trusted Board Boot` (TBB) feature prevents malicious firmware from running
on the platform by authenticating all firmware images up to and including the
normal world bootloader. It does this by establishing a `Chain of Trust` using
Public-Key-Cryptography Standards (PKCS).

This document describes the design of Trusted Firmware-A (TF-A) TBB, which is an
implementation of the `Trusted Board Boot Requirements (TBBR)`_ specification,
Arm DEN0006D. It should be used in conjunction with the :ref:`Firmware Update
(FWU)` design document, which implements a specific aspect of the TBBR.

Chain of Trust
--------------

A Chain of Trust (CoT) starts with a set of implicitly trusted components, which
are used to establish trust in the next layer of components, and so on, in a
`chained` manner.

The chain of trust depends on several factors, including:

-  The set of firmware images in use on this platform.
   Typically, most platforms share a common set of firmware images (BL1, BL2,
   BL31, BL33) but extra platform-specific images might be required.

-  The key provisioning scheme: which keys need to programmed into the device
   and at which stage during the platform's manufacturing lifecycle.

-  The key ownership model: who owns which key.

As these vary across platforms, chains of trust also vary across
platforms. Although each platform is free to define its own CoT based on its
needs, TF-A provides a set of "default" CoTs fitting some typical trust models,
which platforms may reuse. The rest of this section presents general concepts
which apply to all these default CoTs.

The implicitly trusted components forming the trust anchor are:

-  A Root of Trust Public Key (ROTPK), or a hash of it.

   On Arm development platforms, a hash of the ROTPK (hash algorithm selected by
   the ``HASH_ALG`` build option, with sha256 as default) is stored in the
   trusted root-key storage registers. Alternatively, a development ROTPK might
   be used and its hash embedded into the BL1 and BL2 images (only for
   development purposes).

-  The BL1 image, on the assumption that it resides in ROM so cannot be
   tampered with.

The remaining components in the CoT are either certificates or boot loader
images. The certificates follow the `X.509 v3`_ standard. This standard
enables adding custom extensions to the certificates, which are used to store
essential information to establish the CoT.

All certificates are self-signed. There is no need for a Certificate Authority
(CA) because the CoT is not established by verifying the validity of a
certificate's issuer but by the content of the certificate extensions. To sign
the certificates, different signature schemes are available, please refer to the
:ref:`Build Options` for more details.

The certificates are categorised as "Key" and "Content" certificates. Key
certificates are used to verify public keys which have been used to sign content
certificates. Content certificates are used to store the hash of a boot loader
image. An image can be authenticated by calculating its hash and matching it
with the hash extracted from the content certificate. Various hash algorithms
are supported to calculate all hashes, please refer to the :ref:`Build Options`
for more details. The public keys and hashes are included as non-standard
extension fields in the `X.509 v3`_ certificates.

The next sections now present specificities of each default CoT provided in
TF-A.

Default CoT #1: TBBR
~~~~~~~~~~~~~~~~~~~~

The `TBBR` CoT is named after the specification it follows to the letter.

In the TBBR CoT, all firmware binaries and certificates are (directly or
indirectly) linked to the Root of Trust Public Key (ROTPK). Typically, the same
vendor owns the ROTPK, the Trusted key and the Non-Trusted Key. Thus, this vendor
is involved in signing every BL3x Key Certificate.

The keys used to establish this CoT are:

-  **Root of trust key**

   The private part of this key is used to sign the trusted boot firmware
   certificate and the trusted key certificate. The public part is the ROTPK.

-  **Trusted world key**

   The private part is used to sign the key certificates corresponding to the
   secure world images (SCP_BL2, BL31 and BL32). The public part is stored in
   one of the extension fields in the trusted key certificate.

-  **Non-trusted world key**

   The private part is used to sign the key certificate corresponding to the
   non-secure world image (BL33). The public part is stored in one of the
   extension fields in the trusted key certificate.

-  **BL3X keys**

   For each of SCP_BL2, BL31, BL32 and BL33, the private part is used to
   sign the content certificate for the BL3X image. The public part is stored
   in one of the extension fields in the corresponding key certificate.

The following images are included in the CoT:

-  BL1
-  BL2
-  SCP_BL2 (optional)
-  BL31
-  BL33
-  BL32 (optional)

The following certificates are used to authenticate the images.

-  **Trusted boot firmware certificate**

   It is self-signed with the private part of the ROT key. It contains a hash of
   the BL2 image and hashes of various firmware configuration files
   (TB_FW_CONFIG, HW_CONFIG, FW_CONFIG).

-  **Trusted key certificate**

   It is self-signed with the private part of the ROT key. It contains the
   public part of the trusted world key and the public part of the non-trusted
   world key.

-  **SCP firmware key certificate**

   It is self-signed with the trusted world key. It contains the public part of
   the SCP_BL2 key.

-  **SCP firmware content certificate**

   It is self-signed with the SCP_BL2 key. It contains a hash of the SCP_BL2
   image.

-  **SoC firmware key certificate**

   It is self-signed with the trusted world key. It contains the public part of
   the BL31 key.

-  **SoC firmware content certificate**

   It is self-signed with the BL31 key. It contains hashes of the BL31 image and
   its configuration file (SOC_FW_CONFIG).

-  **Trusted OS key certificate**

   It is self-signed with the trusted world key. It contains the public part of
   the BL32 key.

-  **Trusted OS content certificate**

   It is self-signed with the BL32 key. It contains hashes of the BL32 image(s)
   and its configuration file(s) (TOS_FW_CONFIG).

-  **Non-trusted firmware key certificate**

   It is self-signed with the non-trusted world key. It contains the public
   part of the BL33 key.

-  **Non-trusted firmware content certificate**

   It is self-signed with the BL33 key. It contains hashes of the BL33 image and
   its configuration file (NT_FW_CONFIG).

The SCP firmware and Trusted OS certificates are optional, but they must be
present if the corresponding SCP_BL2 or BL32 images are present.

The following diagram summarizes the part of the TBBR CoT enforced by BL2. Some
images (SCP, debug certificates, secure partitions, configuration files) are not
shown here for conciseness:

.. image:: ../resources/diagrams/cot-tbbr.jpg

Default CoT #2: Dualroot
~~~~~~~~~~~~~~~~~~~~~~~~

The `dualroot` CoT is targeted at systems where the Normal World firmware is
owned by a different entity than the Secure World Firmware, and those 2 entities
do not wish to share any keys or have any dependency between each other when it
comes to signing their respective images. It establishes 2 separate signing
domains, each with its own Root of Trust key. In that sense, this CoT has 2
roots of trust, hence the `dualroot` name.

Although the dualroot CoT reuses some of the TBBR CoT components and concepts,
it differs on the BL33 image's chain of trust, which is rooted into a new key,
called `Platform ROTPK`, or `PROTPK` for short.

The following diagram summarizes the part of the dualroot CoT enforced by
BL2. Some images (SCP, debug certificates, secure partitions, configuration
files) are not shown here for conciseness:

.. image:: ../resources/diagrams/cot-dualroot.jpg

Default CoT #3: CCA
~~~~~~~~~~~~~~~~~~~

This CoT is targeted at Arm CCA systems. The Arm CCA security model recommends
making supply chains for the Arm CCA firmware, the secure world firmware and the
platform owner firmware, independent. Hence, this CoT has 3 roots of trust, one
for each supply chain.

Trusted Board Boot Sequence
---------------------------

The CoT is verified through the following sequence of steps. The system panics
if any of the steps fail.

-  BL1 loads and verifies the BL2 content certificate. The issuer public key is
   read from the verified certificate. A hash of that key is calculated and
   compared with the hash of the ROTPK read from the trusted root-key storage
   registers. If they match, the BL2 hash is read from the certificate.

   .. note::
      The matching operation is platform specific and is currently
      unimplemented on the Arm development platforms.

-  BL1 loads the BL2 image. Its hash is calculated and compared with the hash
   read from the certificate. Control is transferred to the BL2 image if all
   the comparisons succeed.

-  BL2 loads and verifies the trusted key certificate. The issuer public key is
   read from the verified certificate. A hash of that key is calculated and
   compared with the hash of the ROTPK read from the trusted root-key storage
   registers. If the comparison succeeds, BL2 reads and saves the trusted and
   non-trusted world public keys from the verified certificate.

The next two steps are executed for each of the SCP_BL2, BL31 & BL32 images.
The steps for the optional SCP_BL2 and BL32 images are skipped if these images
are not present.

-  BL2 loads and verifies the BL3x key certificate. The certificate signature
   is verified using the trusted world public key. If the signature
   verification succeeds, BL2 reads and saves the BL3x public key from the
   certificate.

-  BL2 loads and verifies the BL3x content certificate. The signature is
   verified using the BL3x public key. If the signature verification succeeds,
   BL2 reads and saves the BL3x image hash from the certificate.

The next two steps are executed only for the BL33 image.

-  BL2 loads and verifies the BL33 key certificate. If the signature
   verification succeeds, BL2 reads and saves the BL33 public key from the
   certificate.

-  BL2 loads and verifies the BL33 content certificate. If the signature
   verification succeeds, BL2 reads and saves the BL33 image hash from the
   certificate.

The next step is executed for all the boot loader images.

-  BL2 calculates the hash of each image. It compares it with the hash obtained
   from the corresponding content certificate. The image authentication succeeds
   if the hashes match.

The Trusted Board Boot implementation spans both generic and platform-specific
BL1 and BL2 code, and in tool code on the host build machine. The feature is
enabled through use of specific build flags as described in
:ref:`Build Options`.

On the host machine, a tool generates the certificates, which are included in
the FIP along with the boot loader images. These certificates are loaded in
Trusted SRAM using the IO storage framework. They are then verified by an
Authentication module included in TF-A.

The mechanism used for generating the FIP and the Authentication module are
described in the following sections.

Authentication Framework
------------------------

The authentication framework included in TF-A provides support to implement
the desired trusted boot sequence. Arm platforms use this framework to
implement the boot requirements specified in the
`Trusted Board Boot Requirements (TBBR)`_ document.

More information about the authentication framework can be found in the
:ref:`Authentication Framework & Chain of Trust` document.

Certificate Generation Tool
---------------------------

The ``cert_create`` tool is built and runs on the host machine as part of the
TF-A build process when ``GENERATE_COT=1``. It takes the boot loader images
and keys as inputs and generates the certificates (in DER format) required to
establish the CoT. The input keys must either be a file in PEM format or a
PKCS11 URI in case a HSM is used. New keys can be generated by the tool in
case they are not provided. The certificates are then passed as inputs to
the ``fiptool`` utility for creating the FIP.

The certificates are also stored individually in the output build directory.

The tool resides in the ``tools/cert_create`` directory. It uses the OpenSSL SSL
library version to generate the X.509 certificates. The specific version of the
library that is required is given in the :ref:`Prerequisites` document.

Instructions for building and using the tool can be found at
:ref:`tools_build_cert_create`.

Authenticated Encryption Framework
----------------------------------

The authenticated encryption framework included in TF-A provides support to
implement the optional firmware encryption feature. This feature can be
optionally enabled on platforms to implement the optional requirement:
R060_TBBR_FUNCTION as specified in the `Trusted Board Boot Requirements (TBBR)`_
document.

Firmware Encryption Tool
------------------------

The ``encrypt_fw`` tool is built and runs on the host machine as part of the
TF-A build process when ``DECRYPTION_SUPPORT != none``. It takes the plain
firmware image as input and generates the encrypted firmware image which can
then be passed as input to the ``fiptool`` utility for creating the FIP.

The encrypted firmwares are also stored individually in the output build
directory.

The tool resides in the ``tools/encrypt_fw`` directory. It uses OpenSSL SSL
library version 1.0.1 or later to do authenticated encryption operation.
Instructions for building and using the tool can be found in the
:ref:`tools_build_enctool`.

--------------

*Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.*

.. _X.509 v3: https://tools.ietf.org/rfc/rfc5280.txt
.. _Trusted Board Boot Requirements (TBBR): https://developer.arm.com/docs/den0006/latest
