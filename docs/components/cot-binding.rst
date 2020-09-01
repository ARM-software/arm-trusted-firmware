Chain of trust bindings
=======================

The device tree allows to describe the chain of trust with the help of
'cot' node which contain 'manifests' and 'images' as sub-nodes.
'manifests' and 'images' nodes contains number of sub-nodes (i.e. 'certificate'
and 'image' nodes) mentioning properties of the certificate and image respectively.

Also, device tree describes 'non-volatile-counters' node which contains number of
sub-nodes mentioning properties of all non-volatile-counters used in the chain of trust.

cot
------------------------------------------------------------------
This is root node which contains 'manifests' and 'images' as sub-nodes


Manifests and Certificate node bindings definition
----------------------------------------------------------------

- Manifests node
        Description: Container of certificate nodes.

        PROPERTIES

        - compatible:
                Usage: required

                Value type: <string>

                Definition: must be "arm, cert-descs"

- Certificate node
        Description:

        Describes certificate properties which are used
        during the authentication process.

        PROPERTIES

        - root-certificate
               Usage:

               Required for the certificate with no parent.
               In other words, certificates which are validated
               using root of trust public key.

               Value type: <boolean>

        - image-id
                Usage: Required for every certificate with unique id.

                Value type: <u32>

        - parent
                Usage:

                It refers to their parent image, which typically contains
                information to authenticate the certificate.
                This property is required for all non-root certificates.

                This property is not required for root-certificates
                as root-certificates are validated using root of trust
                public key provided by platform.

                Value type: <phandle>

        - signing-key
                Usage:

                This property is used to refer public key node present in
                parent certificate node and it is required property for all
                non-root certificates which are authenticated using public-key
                present in parent certificate.

                This property is not required for root-certificates
                as root-certificates are validated using root of trust
                public key provided by platform.

                Value type: <phandle>

        - antirollback-counter
                Usage:

                This property is used by all certificates which are
                protected against rollback attacks using a non-volatile
                counter and it is an optional property.

                This property is used to refer one of the non-volatile
                counter sub-node present in 'non-volatile counters' node.

                Value type: <phandle>


        SUBNODES
            - Description:

              Hash and public key information present in the certificate
              are shown by these nodes.

            - public key node
                  Description: Provide public key information in the certificate.

                  PROPERTIES

                  - oid
                     Usage:

                     This property provides the Object ID of public key
                     provided in the certificate which the help of which
                     public key information can be extracted.

                     Value type: <string>

            - hash node
                 Description: Provide the hash information in the certificate.

                 PROPERTIES

                 - oid
                     Usage:

                     This property provides the Object ID of hash provided in
                     the certificate which the help of which hash information
                     can be extracted.

                     Value type: <string>

Example:

.. code:: c

   cot {
      manifests {
         compatible = "arm, cert-descs”

         trusted-key-cert: trusted-key-cert {
            root-certificate;
            image-id = <TRUSTED_KEY_CERT_ID>;
            antirollback-counter = <&trusted_nv_counter>;

            trusted-world-pk: trusted-world-pk {
               oid = TRUSTED_WORLD_PK_OID;
            };
            non-trusted-world-pk: non-trusted-world-pk {
               oid = NON_TRUSTED_WORLD_PK_OID;
            };
         };

         scp_fw_key_cert: scp_fw_key_cert {
            image-id = <SCP_FW_KEY_CERT_ID>;
            parent = <&trusted-key-cert>;
            signing-key = <&trusted_world_pk>;
            antirollback-counter = <&trusted_nv_counter>;

            scp_fw_content_pk: scp_fw_content_pk {
               oid = SCP_FW_CONTENT_CERT_PK_OID;
            };
         };
         .
         .
         .

         next-certificate {

         };
      };
   };

Images and Image node bindings definition
-----------------------------------------

- Images node
        Description: Container of image nodes

        PROPERTIES

        - compatible:
                Usage: required

                Value type: <string>

                Definition: must be "arm, img-descs"

- Image node
        Description:

        Describes image properties which will be used during
        authentication process.

        PROPERTIES

        - image-id
                Usage: Required for every image with unique id.

                Value type: <u32>

        - parent
                Usage:

                Required for every image to provide a reference to
                its parent image, which contains the necessary information
                to authenticate it.

                Value type: <phandle>

        - hash
                Usage:

                Required for all images which are validated using
                hash method. This property is used to refer hash
                node present in parent certificate node.

                Value type: <phandle>

                Note:

                Currently, all images are validated using 'hash'
                method. In future, there may be multiple methods can
                be used to validate the image.

Example:

.. code:: c

   cot {
      images {
         compatible = "arm, img-descs";

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

                Definition:

                Must be set according to address size
                of non-volatile counter register

        - #size-cells
                Usage: required

                Value type: <u32>

                Definition: must be set to 0

        SUBNODE
            - counters node
                    Description: Contains various non-volatile counters present in the platform.

            PROPERTIES
                - id
                    Usage: Required for every nv-counter with unique id.

                    Value type: <u32>

                - reg
                    Usage:

                    Register base address of non-volatile counter and it is required
                    property.

                    Value type: <u32>

                - oid
                    Usage:

                    This property provides the Object ID of non-volatile counter
                    provided in the certificate and it is required property.

                    Value type: <string>

Example:
Below is non-volatile counters example for ARM platform

.. code:: c

   non_volatile_counters: non_volatile_counters {
        compatible = "arm, non-volatile-counter";
        #address-cells = <1>;
        #size-cells = <0>;

        trusted-nv-counter: trusted_nv_counter {
           id  = <TRUSTED_NV_CTR_ID>;
           reg = <TFW_NVCTR_BASE>;
           oid = TRUSTED_FW_NVCOUNTER_OID;
        };

        non_trusted_nv_counter: non_trusted_nv_counter {
           id  = <NON_TRUSTED_NV_CTR_ID>;
           reg = <NTFW_CTR_BASE>;
           oid = NON_TRUSTED_FW_NVCOUNTER_OID;
        };
   };

Future update to chain of trust binding
---------------------------------------

This binding document needs to be revisited to generalise some terminologies
which are currently specific to X.509 certificates for e.g. Object IDs.

*Copyright (c) 2020, Arm Limited. All rights reserved.*
