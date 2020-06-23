Chain of trust bindings
=======================

The device tree allows to describes the chain of trust with the help of
certificates and images nodes, which in turn contains number of sub-nodes
(i.e. certificate and image) mentioning properties for every certificate
and image respectively.
Also, this binding allows to describe OID of non-volatile counters, memory
mapped address and size of non-volatile counter register.

Convention used in this document
--------------------------------

This document follows the conventions described in the Device-tree
Specification

certificates, certificate and extension node bindings definition
----------------------------------------------------------------

- Certificates node
        Description: Container of certificate nodes.

        PROPERTIES

        - compatible:
                Usage: required

                Value type: <string>

                Definition: must be "arm, certificate-descriptors"

- Certificate node
        Description: Describes certificate properties which are used
                     during the authentication process.

        PROPERTIES

        - root-certificate
                Usage: Required for the certificate with no parent.
                       In other words, Certificates which are validated
                       using root of trust public key.

                Value type: <boolean>

        - image-id
                Usage: Required for every certificate with unique id.

                Value type: <u32>

        - parent
                Usage: It refers to their parent image, which typically contains
                       information to authenticate the certificate.
                       This property is required for all non-root certificates.

                       This property is not required for root-certificates
                       as it is validated using root of trust public key
                       provided by platform.

                Value type: <phandle>

        - signing-key
                Usage: This property is used to refer extension node present in
                       parent certificate and it is required property for all non-
                       root certificates which are authenticated using public-key
                       present in parent certificate.

                       This property is not required for root-certificates
                       as root-certificates are validated using root of trust
                       public key provided by platform.

                Value type: <phandle>

        - antirollback-counter
                Usage: This property is used by all certificates which are protected
                       against rollback attacks using a non-volatile counter and it
                       is optional property.

                       This property is used to refer trusted or non-trusted
                       non-volatile counter node.

                Value type: <phandle>

        SUBNODES

        - extensions node
                Description: This is sub-node of certificate node.
                             Describes OIDs present in the certificate which will
                             be used during authentication process to extract
                             hash/public key information from this certificate.
                             OIDs in extension node are represented using number of
                             sub-nodes which contains 'oid' as property

                PROPERTIES

                - oid
                        Usage: This property provides the Object ID of an extension
                               provided in the certificate.

                        Value type: <string>

Example:

.. code:: c

   certificates {
         compatible = "arm, certificate-descriptors”

         trusted-key-cert: trusted-key-cert {
                  root-certificate;
                  image-id = <TRUSTED_KEY_CERT_ID>;
                  antirollback-counter = <&trusted_nv_counter>;
                  extensions {
                        trusted-world-pk: trusted-world-pk {
                              oid = TRUSTED_WORLD_PK_OID;
                        };
                        non-trusted-world-pk: non-trusted-world-pk {
                              oid = NON_TRUSTED_WORLD_PK_OID;
                        };
                };
        };

        scp_fw_key_cert: scp_fw_key_cert {
                image-id = <SCP_FW_KEY_CERT_ID>;
                parent = <&trusted-key-cert>;
                signing-key = <&trusted_world_pk>;
                antirollback-counter = <&trusted_nv_counter>;
                extensions {
                        scp_fw_content_pk: scp_fw_content_pk {
                              oid = SCP_FW_CONTENT_CERT_PK_OID;
                        };
                };
        };

        .
        .
        .

        next-cert {

        };
   };

Images and image node bindings definition
-----------------------------------------

- Images node
        Description: Container of image nodes

        PROPERTIES

        - compatible:
                Usage: required

                Value type: <string>

                Definition: must be "arm, image-descriptors"

- Image node
        Description: Describes image properties which will be used during
                     authentication process.

        PROPERTIES

        - image-id
                Usage: Required for every image with unique id.

                Value type: <u32>

        - parent
                Usage: Required for every image to provide a reference to
                       it's parent image, which contains the necessary information
                       to authenticate it.

                Value type: <phandle>

        - hash
                Usage: Required for all images which are validated using
                       hash method. This property is used to refer extension
                       node present in parent certificate and it is required
                       property for all images.

                Value type: <phandle>

                Note: Currently, all images are validated using "hash"
                      method. In future, there may be multiple methods can
                      be used to validate the image.

Example:

.. code:: c

   images {
         compatible = "arm, imgage-descriptors";

         scp_bl2_image {
               image-id = <SCP_BL2_IMAGE_ID>;
               parent = <&scp_fw_content_cert>;
               hash = <&scp_fw_hash>;
         };

         .
         .
         .

         next-img {
         };
   };

non-volatile counter node binding definition
--------------------------------------------

- non-volatile counters node
        Description: Contains properties for non-volatile counters.

        PROPERTIES

        - compatible:
                Usage: required

                Value type: <string>

                Definition: must be "arm, non-volatile-counter"

        - #address-cells
                Usage: required

                Value type: <u32>

                Definition: Must be set according to address size
                            of non-volatile counter register

        - #size-cells
                Usage: required

                Value type: <u32>

                Definition: must be set to 0

        SUBNODE
            - counters node
                    Description: Contains various non-volatile counters present in the platform.

            PROPERTIES

                - reg
                    Usage: Register base address of non-volatile counter and it is required
                           property.

                    Value type: <u32>

                - oid
                    Usage: This property provides the Object ID of non-volatile counter
                           provided in the certificate and it is required property.

                    Value type: <string>

Example:
Below is non-volatile counters example for ARM platform

.. code:: c

   non-volatile-counters {
        compatible = "arm, non-volatile-counter";
        #address-cells = <1>;
        #size-cells = <0>;

        counters {
            trusted-nv-counter: trusted_nv_counter {
                reg = <TFW_NVCTR_BASE>;
                oid = TRUSTED_FW_NVCOUNTER_OID;
            };
            non_trusted_nv_counter: non_trusted_nv_counter {
                reg = <NTFW_CTR_BASE>;
                oid = NON_TRUSTED_FW_NVCOUNTER_OID;

            };
        };
   };

Future update to chain of trust binding
---------------------------------------

This binding document need to be revisited to generalise some terminologies
like Object IDs, extensions etc which are currently specific to X.509
certificates.

*Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.*
