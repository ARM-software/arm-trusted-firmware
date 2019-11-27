Trusted Board Boot
==================

The Trusted Board Boot (TBB) feature prevents malicious firmware from running on
the platform by authenticating all firmware images up to and including the
normal world bootloader. It does this by establishing a Chain of Trust using
Public-Key-Cryptography Standards (PKCS).

This document describes the design of Trusted Firmware-A (TF-A) TBB, which is an
implementation of the `Trusted Board Boot Requirements (TBBR)`_ specification,
Arm DEN0006D. It should be used in conjunction with the
:ref:`Firmware Update (FWU)` design document, which implements a specific aspect
of the TBBR.

Chain of Trust
--------------

A Chain of Trust (CoT) starts with a set of implicitly trusted components. On
the Arm development platforms, these components are:

-  A SHA-256 hash of the Root of Trust Public Key (ROTPK). It is stored in the
   trusted root-key storage registers.

-  The BL1 image, on the assumption that it resides in ROM so cannot be
   tampered with.

The remaining components in the CoT are either certificates or boot loader
images. The certificates follow the `X.509 v3`_ standard. This standard
enables adding custom extensions to the certificates, which are used to store
essential information to establish the CoT.

In the TBB CoT all certificates are self-signed. There is no need for a
Certificate Authority (CA) because the CoT is not established by verifying the
validity of a certificate's issuer but by the content of the certificate
extensions. To sign the certificates, the PKCS#1 SHA-256 with RSA Encryption
signature scheme is used with a RSA key length of 2048 bits. Future version of
TF-A will support additional cryptographic algorithms.

The certificates are categorised as "Key" and "Content" certificates. Key
certificates are used to verify public keys which have been used to sign content
certificates. Content certificates are used to store the hash of a boot loader
image. An image can be authenticated by calculating its hash and matching it
with the hash extracted from the content certificate. The SHA-256 function is
used to calculate all hashes. The public keys and hashes are included as
non-standard extension fields in the `X.509 v3`_ certificates.

The keys used to establish the CoT are:

-  **Root of trust key**

   The private part of this key is used to sign the BL2 content certificate and
   the trusted key certificate. The public part is the ROTPK.

-  **Trusted world key**

   The private part is used to sign the key certificates corresponding to the
   secure world images (SCP_BL2, BL31 and BL32). The public part is stored in
   one of the extension fields in the trusted world certificate.

-  **Non-trusted world key**

   The private part is used to sign the key certificate corresponding to the
   non secure world image (BL33). The public part is stored in one of the
   extension fields in the trusted world certificate.

-  **BL3-X keys**

   For each of SCP_BL2, BL31, BL32 and BL33, the private part is used to
   sign the content certificate for the BL3-X image. The public part is stored
   in one of the extension fields in the corresponding key certificate.

The following images are included in the CoT:

-  BL1
-  BL2
-  SCP_BL2 (optional)
-  BL31
-  BL33
-  BL32 (optional)

The following certificates are used to authenticate the images.

-  **BL2 content certificate**

   It is self-signed with the private part of the ROT key. It contains a hash
   of the BL2 image.

-  **Trusted key certificate**

   It is self-signed with the private part of the ROT key. It contains the
   public part of the trusted world key and the public part of the non-trusted
   world key.

-  **SCP_BL2 key certificate**

   It is self-signed with the trusted world key. It contains the public part of
   the SCP_BL2 key.

-  **SCP_BL2 content certificate**

   It is self-signed with the SCP_BL2 key. It contains a hash of the SCP_BL2
   image.

-  **BL31 key certificate**

   It is self-signed with the trusted world key. It contains the public part of
   the BL31 key.

-  **BL31 content certificate**

   It is self-signed with the BL31 key. It contains a hash of the BL31 image.

-  **BL32 key certificate**

   It is self-signed with the trusted world key. It contains the public part of
   the BL32 key.

-  **BL32 content certificate**

   It is self-signed with the BL32 key. It contains a hash of the BL32 image.

-  **BL33 key certificate**

   It is self-signed with the non-trusted world key. It contains the public
   part of the BL33 key.

-  **BL33 content certificate**

   It is self-signed with the BL33 key. It contains a hash of the BL33 image.

The SCP_BL2 and BL32 certificates are optional, but they must be present if the
corresponding SCP_BL2 or BL32 images are present.

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
and keys as inputs (keys must be in PEM format) and generates the
certificates (in DER format) required to establish the CoT. New keys can be
generated by the tool in case they are not provided. The certificates are then
passed as inputs to the ``fiptool`` utility for creating the FIP.

The certificates are also stored individually in the in the output build
directory.

The tool resides in the ``tools/cert_create`` directory. It uses the OpenSSL SSL
library version to generate the X.509 certificates. The specific version of the
library that is required is given in the :ref:`Prerequisites` document.

Instructions for building and using the tool can be found at
:ref:`tools_build_cert_create`.

--------------

*Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.*

.. _X.509 v3: https://tools.ietf.org/rfc/rfc5280.txt
.. _Trusted Board Boot Requirements (TBBR): https://developer.arm.com/docs/den0006/latest/trusted-board-boot-requirements-client-tbbr-client-armv8-a
