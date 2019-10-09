Authentication Framework & Chain of Trust
=========================================

The aim of this document is to describe the authentication framework
implemented in Trusted Firmware-A (TF-A). This framework fulfills the
following requirements:

#. It should be possible for a platform port to specify the Chain of Trust in
   terms of certificate hierarchy and the mechanisms used to verify a
   particular image/certificate.

#. The framework should distinguish between:

   -  The mechanism used to encode and transport information, e.g. DER encoded
      X.509v3 certificates to ferry Subject Public Keys, hashes and non-volatile
      counters.

   -  The mechanism used to verify the transported information i.e. the
      cryptographic libraries.

The framework has been designed following a modular approach illustrated in the
next diagram:

::

        +---------------+---------------+------------+
        | Trusted       | Trusted       | Trusted    |
        | Firmware      | Firmware      | Firmware   |
        | Generic       | IO Framework  | Platform   |
        | Code i.e.     | (IO)          | Port       |
        | BL1/BL2 (GEN) |               | (PP)       |
        +---------------+---------------+------------+
               ^               ^               ^
               |               |               |
               v               v               v
         +-----------+   +-----------+   +-----------+
         |           |   |           |   | Image     |
         | Crypto    |   | Auth      |   | Parser    |
         | Module    |<->| Module    |<->| Module    |
         | (CM)      |   | (AM)      |   | (IPM)     |
         |           |   |           |   |           |
         +-----------+   +-----------+   +-----------+
               ^                               ^
               |                               |
               v                               v
        +----------------+             +-----------------+
        | Cryptographic  |             | Image Parser    |
        | Libraries (CL) |             | Libraries (IPL) |
        +----------------+             +-----------------+
                      |                |
                      |                |
                      |                |
                      v                v
                     +-----------------+
                     | Misc. Libs e.g. |
                     | ASN.1 decoder   |
                     |                 |
                     +-----------------+

        DIAGRAM 1.

This document describes the inner details of the authentication framework and
the abstraction mechanisms available to specify a Chain of Trust.

Framework design
----------------

This section describes some aspects of the framework design and the rationale
behind them. These aspects are key to verify a Chain of Trust.

Chain of Trust
~~~~~~~~~~~~~~

A CoT is basically a sequence of authentication images which usually starts with
a root of trust and culminates in a single data image. The following diagram
illustrates how this maps to a CoT for the BL31 image described in the
`TBBR-Client specification`_.

::

        +------------------+       +-------------------+
        | ROTPK/ROTPK Hash |------>| Trusted Key       |
        +------------------+       | Certificate       |
                                   | (Auth Image)      |
                                  /+-------------------+
                                 /            |
                                /             |
                               /              |
                              /               |
                             L                v
        +------------------+       +-------------------+
        | Trusted World    |------>| BL31 Key          |
        | Public Key       |       | Certificate       |
        +------------------+       | (Auth Image)      |
                                   +-------------------+
                                  /           |
                                 /            |
                                /             |
                               /              |
                              /               v
        +------------------+ L     +-------------------+
        | BL31 Content     |------>| BL31 Content      |
        | Certificate PK   |       | Certificate       |
        +------------------+       | (Auth Image)      |
                                   +-------------------+
                                  /           |
                                 /            |
                                /             |
                               /              |
                              /               v
        +------------------+ L     +-------------------+
        | BL31 Hash        |------>| BL31 Image        |
        |                  |       | (Data Image)      |
        +------------------+       |                   |
                                   +-------------------+

        DIAGRAM 2.

The root of trust is usually a public key (ROTPK) that has been burnt in the
platform and cannot be modified.

Image types
~~~~~~~~~~~

Images in a CoT are categorised as authentication and data images. An
authentication image contains information to authenticate a data image or
another authentication image. A data image is usually a boot loader binary, but
it could be any other data that requires authentication.

Component responsibilities
~~~~~~~~~~~~~~~~~~~~~~~~~~

For every image in a Chain of Trust, the following high level operations are
performed to verify it:

#. Allocate memory for the image either statically or at runtime.

#. Identify the image and load it in the allocated memory.

#. Check the integrity of the image as per its type.

#. Authenticate the image as per the cryptographic algorithms used.

#. If the image is an authentication image, extract the information that will
   be used to authenticate the next image in the CoT.

In Diagram 1, each component is responsible for one or more of these operations.
The responsibilities are briefly described below.

TF-A Generic code and IO framework (GEN/IO)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These components are responsible for initiating the authentication process for a
particular image in BL1 or BL2. For each BL image that requires authentication,
the Generic code asks recursively the Authentication module what is the parent
image until either an authenticated image or the ROT is reached. Then the
Generic code calls the IO framework to load the image and calls the
Authentication module to authenticate it, following the CoT from ROT to Image.

TF-A Platform Port (PP)
^^^^^^^^^^^^^^^^^^^^^^^

The platform is responsible for:

#. Specifying the CoT for each image that needs to be authenticated. Details of
   how a CoT can be specified by the platform are explained later. The platform
   also specifies the authentication methods and the parsing method used for
   each image.

#. Statically allocating memory for each parameter in each image which is
   used for verifying the CoT, e.g. memory for public keys, hashes etc.

#. Providing the ROTPK or a hash of it.

#. Providing additional information to the IPM to enable it to identify and
   extract authentication parameters contained in an image, e.g. if the
   parameters are stored as X509v3 extensions, the corresponding OID must be
   provided.

#. Fulfill any other memory requirements of the IPM and the CM (not currently
   described in this document).

#. Export functions to verify an image which uses an authentication method that
   cannot be interpreted by the CM, e.g. if an image has to be verified using a
   NV counter, then the value of the counter to compare with can only be
   provided by the platform.

#. Export a custom IPM if a proprietary image format is being used (described
   later).

Authentication Module (AM)
^^^^^^^^^^^^^^^^^^^^^^^^^^

It is responsible for:

#. Providing the necessary abstraction mechanisms to describe a CoT. Amongst
   other things, the authentication and image parsing methods must be specified
   by the PP in the CoT.

#. Verifying the CoT passed by GEN by utilising functionality exported by the
   PP, IPM and CM.

#. Tracking which images have been verified. In case an image is a part of
   multiple CoTs then it should be verified only once e.g. the Trusted World
   Key Certificate in the TBBR-Client spec. contains information to verify
   SCP_BL2, BL31, BL32 each of which have a separate CoT. (This
   responsibility has not been described in this document but should be
   trivial to implement).

#. Reusing memory meant for a data image to verify authentication images e.g.
   in the CoT described in Diagram 2, each certificate can be loaded and
   verified in the memory reserved by the platform for the BL31 image. By the
   time BL31 (the data image) is loaded, all information to authenticate it
   will have been extracted from the parent image i.e. BL31 content
   certificate. It is assumed that the size of an authentication image will
   never exceed the size of a data image. It should be possible to verify this
   at build time using asserts.

Cryptographic Module (CM)
^^^^^^^^^^^^^^^^^^^^^^^^^

The CM is responsible for providing an API to:

#. Verify a digital signature.
#. Verify a hash.

The CM does not include any cryptography related code, but it relies on an
external library to perform the cryptographic operations. A Crypto-Library (CL)
linking the CM and the external library must be implemented. The following
functions must be provided by the CL:

.. code:: c

    void (*init)(void);
    int (*verify_signature)(void *data_ptr, unsigned int data_len,
                            void *sig_ptr, unsigned int sig_len,
                            void *sig_alg, unsigned int sig_alg_len,
                            void *pk_ptr, unsigned int pk_len);
    int (*verify_hash)(void *data_ptr, unsigned int data_len,
                       void *digest_info_ptr, unsigned int digest_info_len);

These functions are registered in the CM using the macro:

.. code:: c

    REGISTER_CRYPTO_LIB(_name, _init, _verify_signature, _verify_hash);

``_name`` must be a string containing the name of the CL. This name is used for
debugging purposes.

Image Parser Module (IPM)
^^^^^^^^^^^^^^^^^^^^^^^^^

The IPM is responsible for:

#. Checking the integrity of each image loaded by the IO framework.
#. Extracting parameters used for authenticating an image based upon a
   description provided by the platform in the CoT descriptor.

Images may have different formats (for example, authentication images could be
x509v3 certificates, signed ELF files or any other platform specific format).
The IPM allows to register an Image Parser Library (IPL) for every image format
used in the CoT. This library must implement the specific methods to parse the
image. The IPM obtains the image format from the CoT and calls the right IPL to
check the image integrity and extract the authentication parameters.

See Section "Describing the image parsing methods" for more details about the
mechanism the IPM provides to define and register IPLs.

Authentication methods
~~~~~~~~~~~~~~~~~~~~~~

The AM supports the following authentication methods:

#. Hash
#. Digital signature

The platform may specify these methods in the CoT in case it decides to define
a custom CoT instead of reusing a predefined one.

If a data image uses multiple methods, then all the methods must be a part of
the same CoT. The number and type of parameters are method specific. These
parameters should be obtained from the parent image using the IPM.

#. Hash

   Parameters:

   #. A pointer to data to hash
   #. Length of the data
   #. A pointer to the hash
   #. Length of the hash

   The hash will be represented by the DER encoding of the following ASN.1
   type:

   ::

       DigestInfo ::= SEQUENCE {
           digestAlgorithm  DigestAlgorithmIdentifier,
           digest           Digest
       }

   This ASN.1 structure makes it possible to remove any assumption about the
   type of hash algorithm used as this information accompanies the hash. This
   should allow the Cryptography Library (CL) to support multiple hash
   algorithm implementations.

#. Digital Signature

   Parameters:

   #. A pointer to data to sign
   #. Length of the data
   #. Public Key Algorithm
   #. Public Key value
   #. Digital Signature Algorithm
   #. Digital Signature value

   The Public Key parameters will be represented by the DER encoding of the
   following ASN.1 type:

   ::

       SubjectPublicKeyInfo  ::=  SEQUENCE  {
           algorithm         AlgorithmIdentifier{PUBLIC-KEY,{PublicKeyAlgorithms}},
           subjectPublicKey  BIT STRING  }

   The Digital Signature Algorithm will be represented by the DER encoding of
   the following ASN.1 types.

   ::

       AlgorithmIdentifier {ALGORITHM:IOSet } ::= SEQUENCE {
           algorithm         ALGORITHM.&id({IOSet}),
           parameters        ALGORITHM.&Type({IOSet}{@algorithm}) OPTIONAL
       }

   The digital signature will be represented by:

   ::

       signature  ::=  BIT STRING

The authentication framework will use the image descriptor to extract all the
information related to authentication.

Specifying a Chain of Trust
---------------------------

A CoT can be described as a set of image descriptors linked together in a
particular order. The order dictates the sequence in which they must be
verified. Each image has a set of properties which allow the AM to verify it.
These properties are described below.

The PP is responsible for defining a single or multiple CoTs for a data image.
Unless otherwise specified, the data structures described in the following
sections are populated by the PP statically.

Describing the image parsing methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parsing method refers to the format of a particular image. For example, an
authentication image that represents a certificate could be in the X.509v3
format. A data image that represents a boot loader stage could be in raw binary
or ELF format. The IPM supports three parsing methods. An image has to use one
of the three methods described below. An IPL is responsible for interpreting a
single parsing method. There has to be one IPL for every method used by the
platform.

#. Raw format: This format is effectively a nop as an image using this method
   is treated as being in raw binary format e.g. boot loader images used by
   TF-A. This method should only be used by data images.

#. X509V3 method: This method uses industry standards like X.509 to represent
   PKI certificates (authentication images). It is expected that open source
   libraries will be available which can be used to parse an image represented
   by this method. Such libraries can be used to write the corresponding IPL
   e.g. the X.509 parsing library code in mbed TLS.

#. Platform defined method: This method caters for platform specific
   proprietary standards to represent authentication or data images. For
   example, The signature of a data image could be appended to the data image
   raw binary. A header could be prepended to the combined blob to specify the
   extents of each component. The platform will have to implement the
   corresponding IPL to interpret such a format.

The following enum can be used to define these three methods.

.. code:: c

    typedef enum img_type_enum {
        IMG_RAW,            /* Binary image */
        IMG_PLAT,           /* Platform specific format */
        IMG_CERT,           /* X509v3 certificate */
        IMG_MAX_TYPES,
    } img_type_t;

An IPL must provide functions with the following prototypes:

.. code:: c

    void init(void);
    int check_integrity(void *img, unsigned int img_len);
    int get_auth_param(const auth_param_type_desc_t *type_desc,
                          void *img, unsigned int img_len,
                          void **param, unsigned int *param_len);

An IPL for each type must be registered using the following macro:

.. code:: c

    REGISTER_IMG_PARSER_LIB(_type, _name, _init, _check_int, _get_param)

-  ``_type``: one of the types described above.
-  ``_name``: a string containing the IPL name for debugging purposes.
-  ``_init``: initialization function pointer.
-  ``_check_int``: check image integrity function pointer.
-  ``_get_param``: extract authentication parameter function pointer.

The ``init()`` function will be used to initialize the IPL.

The ``check_integrity()`` function is passed a pointer to the memory where the
image has been loaded by the IO framework and the image length. It should ensure
that the image is in the format corresponding to the parsing method and has not
been tampered with. For example, RFC-2459 describes a validation sequence for an
X.509 certificate.

The ``get_auth_param()`` function is passed a parameter descriptor containing
information about the parameter (``type_desc`` and ``cookie``) to identify and
extract the data corresponding to that parameter from an image. This data will
be used to verify either the current or the next image in the CoT sequence.

Each image in the CoT will specify the parsing method it uses. This information
will be used by the IPM to find the right parser descriptor for the image.

Describing the authentication method(s)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As part of the CoT, each image has to specify one or more authentication methods
which will be used to verify it. As described in the Section "Authentication
methods", there are three methods supported by the AM.

.. code:: c

    typedef enum {
        AUTH_METHOD_NONE,
        AUTH_METHOD_HASH,
        AUTH_METHOD_SIG,
        AUTH_METHOD_NUM
    } auth_method_type_t;

The AM defines the type of each parameter used by an authentication method. It
uses this information to:

#. Specify to the ``get_auth_param()`` function exported by the IPM, which
   parameter should be extracted from an image.

#. Correctly marshall the parameters while calling the verification function
   exported by the CM and PP.

#. Extract authentication parameters from a parent image in order to verify a
   child image e.g. to verify the certificate image, the public key has to be
   obtained from the parent image.

.. code:: c

    typedef enum {
        AUTH_PARAM_NONE,
        AUTH_PARAM_RAW_DATA,        /* Raw image data */
        AUTH_PARAM_SIG,         /* The image signature */
        AUTH_PARAM_SIG_ALG,     /* The image signature algorithm */
        AUTH_PARAM_HASH,        /* A hash (including the algorithm) */
        AUTH_PARAM_PUB_KEY,     /* A public key */
    } auth_param_type_t;

The AM defines the following structure to identify an authentication parameter
required to verify an image.

.. code:: c

    typedef struct auth_param_type_desc_s {
        auth_param_type_t type;
        void *cookie;
    } auth_param_type_desc_t;

``cookie`` is used by the platform to specify additional information to the IPM
which enables it to uniquely identify the parameter that should be extracted
from an image. For example, the hash of a BL3x image in its corresponding
content certificate is stored in an X509v3 custom extension field. An extension
field can only be identified using an OID. In this case, the ``cookie`` could
contain the pointer to the OID defined by the platform for the hash extension
field while the ``type`` field could be set to ``AUTH_PARAM_HASH``. A value of 0 for
the ``cookie`` field means that it is not used.

For each method, the AM defines a structure with the parameters required to
verify the image.

.. code:: c

    /*
     * Parameters for authentication by hash matching
     */
    typedef struct auth_method_param_hash_s {
        auth_param_type_desc_t *data;   /* Data to hash */
        auth_param_type_desc_t *hash;   /* Hash to match with */
    } auth_method_param_hash_t;

    /*
     * Parameters for authentication by signature
     */
    typedef struct auth_method_param_sig_s {
        auth_param_type_desc_t *pk; /* Public key */
        auth_param_type_desc_t *sig;    /* Signature to check */
        auth_param_type_desc_t *alg;    /* Signature algorithm */
        auth_param_type_desc_t *tbs;    /* Data signed */
    } auth_method_param_sig_t;

The AM defines the following structure to describe an authentication method for
verifying an image

.. code:: c

    /*
     * Authentication method descriptor
     */
    typedef struct auth_method_desc_s {
        auth_method_type_t type;
        union {
            auth_method_param_hash_t hash;
            auth_method_param_sig_t sig;
        } param;
    } auth_method_desc_t;

Using the method type specified in the ``type`` field, the AM finds out what field
needs to access within the ``param`` union.

Storing Authentication parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A parameter described by ``auth_param_type_desc_t`` to verify an image could be
obtained from either the image itself or its parent image. The memory allocated
for loading the parent image will be reused for loading the child image. Hence
parameters which are obtained from the parent for verifying a child image need
to have memory allocated for them separately where they can be stored. This
memory must be statically allocated by the platform port.

The AM defines the following structure to store the data corresponding to an
authentication parameter.

.. code:: c

    typedef struct auth_param_data_desc_s {
        void *auth_param_ptr;
        unsigned int auth_param_len;
    } auth_param_data_desc_t;

The ``auth_param_ptr`` field is initialized by the platform. The ``auth_param_len``
field is used to specify the length of the data in the memory.

For parameters that can be obtained from the child image itself, the IPM is
responsible for populating the ``auth_param_ptr`` and ``auth_param_len`` fields
while executing the ``img_get_auth_param()`` function.

The AM defines the following structure to enable an image to describe the
parameters that should be extracted from it and used to verify the next image
(child) in a CoT.

.. code:: c

    typedef struct auth_param_desc_s {
        auth_param_type_desc_t type_desc;
        auth_param_data_desc_t data;
    } auth_param_desc_t;

Describing an image in a CoT
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An image in a CoT is a consolidation of the following aspects of a CoT described
above.

#. A unique identifier specified by the platform which allows the IO framework
   to locate the image in a FIP and load it in the memory reserved for the data
   image in the CoT.

#. A parsing method which is used by the AM to find the appropriate IPM.

#. Authentication methods and their parameters as described in the previous
   section. These are used to verify the current image.

#. Parameters which are used to verify the next image in the current CoT. These
   parameters are specified only by authentication images and can be extracted
   from the current image once it has been verified.

The following data structure describes an image in a CoT.

.. code:: c

    typedef struct auth_img_desc_s {
        unsigned int img_id;
        const struct auth_img_desc_s *parent;
        img_type_t img_type;
        const auth_method_desc_t *const img_auth_methods;
        const auth_param_desc_t *const authenticated_data;
    } auth_img_desc_t;

A CoT is defined as an array of pointers to ``auth_image_desc_t`` structures
linked together by the ``parent`` field. Those nodes with no parent must be
authenticated using the ROTPK stored in the platform.

Implementation example
----------------------

This section is a detailed guide explaining a trusted boot implementation using
the authentication framework. This example corresponds to the Applicative
Functional Mode (AFM) as specified in the TBBR-Client document. It is
recommended to read this guide along with the source code.

The TBBR CoT
~~~~~~~~~~~~

The CoT can be found in ``drivers/auth/tbbr/tbbr_cot.c``. This CoT consists of
an array of pointers to image descriptors and it is registered in the framework
using the macro ``REGISTER_COT(cot_desc)``, where 'cot_desc' must be the name
of the array (passing a pointer or any other type of indirection will cause the
registration process to fail).

The number of images participating in the boot process depends on the CoT.
There is, however, a minimum set of images that are mandatory in TF-A and thus
all CoTs must present:

-  ``BL2``
-  ``SCP_BL2`` (platform specific)
-  ``BL31``
-  ``BL32`` (optional)
-  ``BL33``

The TBBR specifies the additional certificates that must accompany these images
for a proper authentication. Details about the TBBR CoT may be found in the
:ref:`Trusted Board Boot` document.

Following the :ref:`Porting Guide`, a platform must provide unique
identifiers for all the images and certificates that will be loaded during the
boot process. If a platform is using the TBBR as a reference for trusted boot,
these identifiers can be obtained from ``include/common/tbbr/tbbr_img_def.h``.
Arm platforms include this file in ``include/plat/arm/common/arm_def.h``. Other
platforms may also include this file or provide their own identifiers.

**Important**: the authentication module uses these identifiers to index the
CoT array, so the descriptors location in the array must match the identifiers.

Each image descriptor must specify:

-  ``img_id``: the corresponding image unique identifier defined by the platform.
-  ``img_type``: the image parser module uses the image type to call the proper
   parsing library to check the image integrity and extract the required
   authentication parameters. Three types of images are currently supported:

   -  ``IMG_RAW``: image is a raw binary. No parsing functions are available,
      other than reading the whole image.
   -  ``IMG_PLAT``: image format is platform specific. The platform may use this
      type for custom images not directly supported by the authentication
      framework.
   -  ``IMG_CERT``: image is an x509v3 certificate.

-  ``parent``: pointer to the parent image descriptor. The parent will contain
   the information required to authenticate the current image. If the parent
   is NULL, the authentication parameters will be obtained from the platform
   (i.e. the BL2 and Trusted Key certificates are signed with the ROT private
   key, whose public part is stored in the platform).
-  ``img_auth_methods``: this points to an array which defines the
   authentication methods that must be checked to consider an image
   authenticated. Each method consists of a type and a list of parameter
   descriptors. A parameter descriptor consists of a type and a cookie which
   will point to specific information required to extract that parameter from
   the image (i.e. if the parameter is stored in an x509v3 extension, the
   cookie will point to the extension OID). Depending on the method type, a
   different number of parameters must be specified. This pointer should not be
   NULL.
   Supported methods are:

   -  ``AUTH_METHOD_HASH``: the hash of the image must match the hash extracted
      from the parent image. The following parameter descriptors must be
      specified:

      -  ``data``: data to be hashed (obtained from current image)
      -  ``hash``: reference hash (obtained from parent image)

   -  ``AUTH_METHOD_SIG``: the image (usually a certificate) must be signed with
      the private key whose public part is extracted from the parent image (or
      the platform if the parent is NULL). The following parameter descriptors
      must be specified:

      -  ``pk``: the public key (obtained from parent image)
      -  ``sig``: the digital signature (obtained from current image)
      -  ``alg``: the signature algorithm used (obtained from current image)
      -  ``data``: the data to be signed (obtained from current image)

-  ``authenticated_data``: this array pointer indicates what authentication
   parameters must be extracted from an image once it has been authenticated.
   Each parameter consists of a parameter descriptor and the buffer
   address/size to store the parameter. The CoT is responsible for allocating
   the required memory to store the parameters. This pointer may be NULL.

In the ``tbbr_cot.c`` file, a set of buffers are allocated to store the parameters
extracted from the certificates. In the case of the TBBR CoT, these parameters
are hashes and public keys. In DER format, an RSA-4096 public key requires 550
bytes, and a hash requires 51 bytes. Depending on the CoT and the authentication
process, some of the buffers may be reused at different stages during the boot.

Next in that file, the parameter descriptors are defined. These descriptors will
be used to extract the parameter data from the corresponding image.

Example: the BL31 Chain of Trust
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Four image descriptors form the BL31 Chain of Trust:

.. code:: c

    static const auth_img_desc_t trusted_key_cert = {
            .img_id = TRUSTED_KEY_CERT_ID,
            .img_type = IMG_CERT,
            .parent = NULL,
            .img_auth_methods =  (const auth_method_desc_t[AUTH_METHOD_NUM]) {
                    [0] = {
                            .type = AUTH_METHOD_SIG,
                            .param.sig = {
                                    .pk = &subject_pk,
                                    .sig = &sig,
                                    .alg = &sig_alg,
                                    .data = &raw_data
                            }
                    },
                    [1] = {
                            .type = AUTH_METHOD_NV_CTR,
                            .param.nv_ctr = {
                                    .cert_nv_ctr = &trusted_nv_ctr,
                                    .plat_nv_ctr = &trusted_nv_ctr
                            }
                    }
            },
            .authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
                    [0] = {
                            .type_desc = &trusted_world_pk,
                            .data = {
                                    .ptr = (void *)trusted_world_pk_buf,
                                    .len = (unsigned int)PK_DER_LEN
                            }
                    },
                    [1] = {
                            .type_desc = &non_trusted_world_pk,
                            .data = {
                                    .ptr = (void *)non_trusted_world_pk_buf,
                                    .len = (unsigned int)PK_DER_LEN
                            }
                    }
            }
    };
    static const auth_img_desc_t soc_fw_key_cert = {
            .img_id = SOC_FW_KEY_CERT_ID,
            .img_type = IMG_CERT,
            .parent = &trusted_key_cert,
            .img_auth_methods =  (const auth_method_desc_t[AUTH_METHOD_NUM]) {
                    [0] = {
                            .type = AUTH_METHOD_SIG,
                            .param.sig = {
                                    .pk = &trusted_world_pk,
                                    .sig = &sig,
                                    .alg = &sig_alg,
                                    .data = &raw_data
                            }
                    },
                    [1] = {
                            .type = AUTH_METHOD_NV_CTR,
                            .param.nv_ctr = {
                                    .cert_nv_ctr = &trusted_nv_ctr,
                                    .plat_nv_ctr = &trusted_nv_ctr
                            }
                    }
            },
            .authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
                    [0] = {
                            .type_desc = &soc_fw_content_pk,
                            .data = {
                                    .ptr = (void *)content_pk_buf,
                                    .len = (unsigned int)PK_DER_LEN
                            }
                    }
            }
    };
    static const auth_img_desc_t soc_fw_content_cert = {
            .img_id = SOC_FW_CONTENT_CERT_ID,
            .img_type = IMG_CERT,
            .parent = &soc_fw_key_cert,
            .img_auth_methods =  (const auth_method_desc_t[AUTH_METHOD_NUM]) {
                    [0] = {
                            .type = AUTH_METHOD_SIG,
                            .param.sig = {
                                    .pk = &soc_fw_content_pk,
                                    .sig = &sig,
                                    .alg = &sig_alg,
                                    .data = &raw_data
                            }
                    },
                    [1] = {
                            .type = AUTH_METHOD_NV_CTR,
                            .param.nv_ctr = {
                                    .cert_nv_ctr = &trusted_nv_ctr,
                                    .plat_nv_ctr = &trusted_nv_ctr
                            }
                    }
            },
            .authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
                    [0] = {
                            .type_desc = &soc_fw_hash,
                            .data = {
                                    .ptr = (void *)soc_fw_hash_buf,
                                    .len = (unsigned int)HASH_DER_LEN
                            }
                    },
                    [1] = {
                            .type_desc = &soc_fw_config_hash,
                            .data = {
                                    .ptr = (void *)soc_fw_config_hash_buf,
                                    .len = (unsigned int)HASH_DER_LEN
                            }
                    }
            }
    };
    static const auth_img_desc_t bl31_image = {
            .img_id = BL31_IMAGE_ID,
            .img_type = IMG_RAW,
            .parent = &soc_fw_content_cert,
            .img_auth_methods =  (const auth_method_desc_t[AUTH_METHOD_NUM]) {
                    [0] = {
                            .type = AUTH_METHOD_HASH,
                            .param.hash = {
                                    .data = &raw_data,
                                    .hash = &soc_fw_hash
                            }
                    }
            }
    };

The **Trusted Key certificate** is signed with the ROT private key and contains
the Trusted World public key and the Non-Trusted World public key as x509v3
extensions. This must be specified in the image descriptor using the
``img_auth_methods`` and ``authenticated_data`` arrays, respectively.

The Trusted Key certificate is authenticated by checking its digital signature
using the ROTPK. Four parameters are required to check a signature: the public
key, the algorithm, the signature and the data that has been signed. Therefore,
four parameter descriptors must be specified with the authentication method:

-  ``subject_pk``: parameter descriptor of type ``AUTH_PARAM_PUB_KEY``. This type
   is used to extract a public key from the parent image. If the cookie is an
   OID, the key is extracted from the corresponding x509v3 extension. If the
   cookie is NULL, the subject public key is retrieved. In this case, because
   the parent image is NULL, the public key is obtained from the platform
   (this key will be the ROTPK).
-  ``sig``: parameter descriptor of type ``AUTH_PARAM_SIG``. It is used to extract
   the signature from the certificate.
-  ``sig_alg``: parameter descriptor of type ``AUTH_PARAM_SIG``. It is used to
   extract the signature algorithm from the certificate.
-  ``raw_data``: parameter descriptor of type ``AUTH_PARAM_RAW_DATA``. It is used
   to extract the data to be signed from the certificate.

Once the signature has been checked and the certificate authenticated, the
Trusted World public key needs to be extracted from the certificate. A new entry
is created in the ``authenticated_data`` array for that purpose. In that entry,
the corresponding parameter descriptor must be specified along with the buffer
address to store the parameter value. In this case, the ``tz_world_pk`` descriptor
is used to extract the public key from an x509v3 extension with OID
``TRUSTED_WORLD_PK_OID``. The BL31 key certificate will use this descriptor as
parameter in the signature authentication method. The key is stored in the
``plat_tz_world_pk_buf`` buffer.

The **BL31 Key certificate** is authenticated by checking its digital signature
using the Trusted World public key obtained previously from the Trusted Key
certificate. In the image descriptor, we specify a single authentication method
by signature whose public key is the ``tz_world_pk``. Once this certificate has
been authenticated, we have to extract the BL31 public key, stored in the
extension specified by ``bl31_content_pk``. This key will be copied to the
``plat_content_pk`` buffer.

The **BL31 certificate** is authenticated by checking its digital signature
using the BL31 public key obtained previously from the BL31 Key certificate.
We specify the authentication method using ``bl31_content_pk`` as public key.
After authentication, we need to extract the BL31 hash, stored in the extension
specified by ``bl31_hash``. This hash will be copied to the ``plat_bl31_hash_buf``
buffer.

The **BL31 image** is authenticated by calculating its hash and matching it
with the hash obtained from the BL31 certificate. The image descriptor contains
a single authentication method by hash. The parameters to the hash method are
the reference hash, ``bl31_hash``, and the data to be hashed. In this case, it is
the whole image, so we specify ``raw_data``.

The image parser library
~~~~~~~~~~~~~~~~~~~~~~~~

The image parser module relies on libraries to check the image integrity and
extract the authentication parameters. The number and type of parser libraries
depend on the images used in the CoT. Raw images do not need a library, so
only an x509v3 library is required for the TBBR CoT.

Arm platforms will use an x509v3 library based on mbed TLS. This library may be
found in ``drivers/auth/mbedtls/mbedtls_x509_parser.c``. It exports three
functions:

.. code:: c

    void init(void);
    int check_integrity(void *img, unsigned int img_len);
    int get_auth_param(const auth_param_type_desc_t *type_desc,
                       void *img, unsigned int img_len,
                       void **param, unsigned int *param_len);

The library is registered in the framework using the macro
``REGISTER_IMG_PARSER_LIB()``. Each time the image parser module needs to access
an image of type ``IMG_CERT``, it will call the corresponding function exported
in this file.

The build system must be updated to include the corresponding library and
mbed TLS sources. Arm platforms use the ``arm_common.mk`` file to pull the
sources.

The cryptographic library
~~~~~~~~~~~~~~~~~~~~~~~~~

The cryptographic module relies on a library to perform the required operations,
i.e. verify a hash or a digital signature. Arm platforms will use a library
based on mbed TLS, which can be found in
``drivers/auth/mbedtls/mbedtls_crypto.c``. This library is registered in the
authentication framework using the macro ``REGISTER_CRYPTO_LIB()`` and exports
three functions:

.. code:: c

    void init(void);
    int verify_signature(void *data_ptr, unsigned int data_len,
                         void *sig_ptr, unsigned int sig_len,
                         void *sig_alg, unsigned int sig_alg_len,
                         void *pk_ptr, unsigned int pk_len);
    int verify_hash(void *data_ptr, unsigned int data_len,
                    void *digest_info_ptr, unsigned int digest_info_len);

The mbedTLS library algorithm support is configured by both the
``TF_MBEDTLS_KEY_ALG`` and ``TF_MBEDTLS_KEY_SIZE`` variables.

-  ``TF_MBEDTLS_KEY_ALG`` can take in 3 values: `rsa`, `ecdsa` or `rsa+ecdsa`.
   This variable allows the Makefile to include the corresponding sources in
   the build for the various algorithms. Setting the variable to `rsa+ecdsa`
   enables support for both rsa and ecdsa algorithms in the mbedTLS library.

-  ``TF_MBEDTLS_KEY_SIZE`` sets the supported RSA key size for TFA. Valid values
   include 1024, 2048, 3072 and 4096.

.. note::
   If code size is a concern, the build option ``MBEDTLS_SHA256_SMALLER`` can
   be defined in the platform Makefile. It will make mbed TLS use an
   implementation of SHA-256 with smaller memory footprint (~1.5 KB less) but
   slower (~30%).

--------------

*Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.*

.. _TBBR-Client specification: https://developer.arm.com/docs/den0006/latest/trusted-board-boot-requirements-client-tbbr-client-armv8-a
