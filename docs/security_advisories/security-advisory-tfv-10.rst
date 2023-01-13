Advisory TFV-10 (CVE-2022-47630)
================================

+----------------+-------------------------------------------------------------+
| Title          | Incorrect validation of X.509 certificate extensions can    |
|                | result in an out-of-bounds read.                            |
+================+=============================================================+
| CVE ID         | `CVE-2022-47630`_                                           |
+----------------+-------------------------------------------------------------+
| Date           | Reported on 12 Dec 2022                                     |
+----------------+-------------------------------------------------------------+
| Versions       | v1.2 to v2.8                                                |
| Affected       |                                                             |
+----------------+-------------------------------------------------------------+
| Configurations | BL1 and BL2 with Trusted Boot enabled with custom,          |
| Affected       | downstream usages of ``get_ext()`` and/or ``auth_nvctr()``  |
|                | interfaces. Not exploitable in upstream TF-A code.          |
+----------------+-------------------------------------------------------------+
| Impact         | Out-of-bounds read.                                         |
+----------------+-------------------------------------------------------------+
| Fix Version    | - `fd37982a19a4a291`_ "fix(auth): forbid junk after         |
|                |   extensions"                                               |
|                |                                                             |
|                | - `72460f50e2437a85`_ "fix(auth): require at least one      |
|                |   extension to be present"                                  |
|                |                                                             |
|                | - `f5c51855d36e399e`_ "fix(auth): properly validate X.509   |
|                |   extensions"                                               |
|                |                                                             |
|                | - `abb8f936fd0ad085`_ "fix(auth): avoid out-of-bounds read  |
|                |   in auth_nvctr()"                                          |
|                |                                                             |
|                | Note that `72460f50e2437a85`_ is not fixing any             |
|                | vulnerability per se but it is required for                 |
|                | `f5c51855d36e399e`_ to apply cleanly.                       |
+----------------+-------------------------------------------------------------+
| Credit         | Demi Marie Obenour, Invisible Things Lab                    |
+----------------+-------------------------------------------------------------+

This security advisory describes a vulnerability in the X.509 parser used to
parse boot certificates in TF-A trusted boot: it is possible for a crafted
certificate to cause an out-of-bounds memory read.

Note that upstream platforms are **not** affected by this. Only downstream
platforms may be, if (and only if) the interfaces described below are used in a
different context than seen in upstream code. Details of such context is
described in the rest of this document.

To fully understand this security advisory, it is recommended to refer to the
following standards documents:

 - `RFC 5280`_, *Internet X.509 Public Key Infrastructure Certificate and
   Certificate Revocation List (CRL) Profile*.

 - `ITU-T X.690`_, *ASN.1 encoding rules: Specification of Basic Encoding Rules
   (BER), Canonical Encoding Rules (CER) and Distinguished Encoding Rules
   (DER).*

Bug 1: Insufficient certificate validation
------------------------------------------

The vulnerability lies in the following source file:
``drivers/auth/mbedtls/mbedtls_x509_parser.c``. By design, ``get_ext()`` does
not check the return value of the various ``mbedtls_*()`` functions, as
``cert_parse()`` is assumed to have guaranteed that they will always succeed.
However, it passes the end of an extension as the end pointer to these
functions, whereas ``cert_parse()`` passes the end of the ``TBSCertificate``.
Furthermore, ``cert_parse()`` does not check that the contents of the extension
have the same length as the extension itself. It also does not check that the
extension block extends to the end of the ``TBSCertificate``.

This is a problem, as ``mbedtls_asn1_get_tag()`` leaves ``*p`` and ``*len``
undefined on failure.  In practice, this results in ``get_ext()`` continuing to
parse at different offsets than were used (and validated) by ``cert_parse()``,
which means that the in-bounds guarantee provided by ``cert_parse()`` no longer
holds.  The result is that it is possible for ``get_ext()`` to read memory past
the end of the certificate.  This could potentially access memory with dangerous
read side effects, or leak microarchitectural state that could theoretically be
retrieved through some side-channel attacks as part of a more complex attack.

Bug 2: Missing bounds check in ``auth_nvctr()``
-----------------------------------------------
``auth_nvctr()`` does not check that the buffer provided is
long enough to hold an ``ASN.1 INTEGER``.  Since ``auth_nvctr()`` will only ever
read 6 bytes, it is possible to read up to 6 bytes past the end of the buffer.

Exploitability Analysis
-----------------------

Upstream TF-A Code
~~~~~~~~~~~~~~~~~~

In upstream TF-A code, the only caller of ``auth_nvctr()`` takes its input from
``get_ext()``, which means that the second bug is exploitable, so is the first.
Therefore, only the first bug need be considered.

All standard chains of trust provided in TF-A source tree (that is, under
``drivers/auth/``) require that the certificate's signature has already been
validated prior to calling ``get_ext()``, or any function that calls ``get_ext()``.
Platforms taking their chain of trust from a dynamic configuration file (such as
``fdts/cot_descriptors.dtsi``) are also safe, as signature verification will
always be done prior to any calls to ``get_ext()`` or ``auth_nvctr()`` in this
case, no matter the order of the properties in the file.  Therefore, it is not
possible to exploit this vulnerability pre-authentication in upstream TF-A.

Furthermore, the data read through ``get_ext()`` only
ever gets used by the authentication framework (``drivers/auth/auth_mod.c``),
which greatly reduces the range of inputs it will ever receive and thus the
impact this has. Specifically, the authentication framework uses ``get_ext()``
in three cases:

 1. Retrieving a hash from an X.509 certificate to check the integrity of a
    child certificate (see ``auth_hash()``).

 2. Retrieving the signature details from an X.509 certificate to check its
    authenticity and integrity (see ``auth_signature()``).

 3. Retrieving the security counter value from an X.509 certificate to protect
    it from unauthorized rollback to a previous version (see ``auth_nvctr()``).

None of these uses authentication framework write to the out-of-bounds memory,
so no memory corruption is possible.

In summary, there are 2 separate issues - one in ``get_ext()`` and another one
in ``auth_nvctr()`` - but neither of these can be exploited in the context of
TF-A upstream code.

Only in the following 2 cases do we expect this vulnerability to be triggerable
prior to authentication:

 - The platform uses a custom chain of trust which uses the non-volatile counter
   authentication method (``AUTH_METHOD_NV_CTR``) before the cryptographic
   authentication method (``AUTH_METHOD_SIG``).

 - The chain of trust uses a custom authentication method that calls
   ``get_ext()`` before cryptographic authentication.

Custom Image Parsers
~~~~~~~~~~~~~~~~~~~~

If the platform uses a custom image parser instead of the certificate parser,
the bug in the certificate parser is obviously not relevant.  The bug in
``auth_nvctr()`` *may* be relevant, but only if the returned data is:

- Taken from an untrusted source (meaning that it is read prior to
  authentication).

- Not already checked to be a primitively-encoded ASN.1 tag.

In particular, if the custom image parser implementation wraps a 32-bit integer
in an ASN.1 ``INTEGER``, it is not affected.

.. _CVE-2022-47630: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2022-47630
.. _fd37982a19a4a291: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=fd37982a19a4a291
.. _72460f50e2437a85: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=72460f50e2437a85
.. _f5c51855d36e399e: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=f5c51855d36e399e
.. _abb8f936fd0ad085: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/commit/?id=abb8f936fd0ad085
.. _RFC 5280: https://www.ietf.org/rfc/rfc5280.txt
.. _ITU-T X.690: https://www.itu.int/ITU-T/studygroups/com10/languages/X.690_1297.pdf
