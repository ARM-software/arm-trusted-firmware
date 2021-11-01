/*
 *  TLS 1.3 key schedule
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 ( the "License" ); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#if !defined(MBEDTLS_SSL_TLS1_3_KEYS_H)
#define MBEDTLS_SSL_TLS1_3_KEYS_H

/* This requires MBEDTLS_SSL_TLS1_3_LABEL( idx, name, string ) to be defined at
 * the point of use. See e.g. the definition of mbedtls_ssl_tls1_3_labels_union
 * below. */
#define MBEDTLS_SSL_TLS1_3_LABEL_LIST                               \
    MBEDTLS_SSL_TLS1_3_LABEL( finished    , "finished"     ) \
    MBEDTLS_SSL_TLS1_3_LABEL( resumption  , "resumption"   ) \
    MBEDTLS_SSL_TLS1_3_LABEL( traffic_upd , "traffic upd"  ) \
    MBEDTLS_SSL_TLS1_3_LABEL( exporter    , "exporter"     ) \
    MBEDTLS_SSL_TLS1_3_LABEL( key         , "key"          ) \
    MBEDTLS_SSL_TLS1_3_LABEL( iv          , "iv"           ) \
    MBEDTLS_SSL_TLS1_3_LABEL( c_hs_traffic, "c hs traffic" ) \
    MBEDTLS_SSL_TLS1_3_LABEL( c_ap_traffic, "c ap traffic" ) \
    MBEDTLS_SSL_TLS1_3_LABEL( c_e_traffic , "c e traffic"  ) \
    MBEDTLS_SSL_TLS1_3_LABEL( s_hs_traffic, "s hs traffic" ) \
    MBEDTLS_SSL_TLS1_3_LABEL( s_ap_traffic, "s ap traffic" ) \
    MBEDTLS_SSL_TLS1_3_LABEL( s_e_traffic , "s e traffic"  ) \
    MBEDTLS_SSL_TLS1_3_LABEL( e_exp_master, "e exp master" ) \
    MBEDTLS_SSL_TLS1_3_LABEL( res_master  , "res master"   ) \
    MBEDTLS_SSL_TLS1_3_LABEL( exp_master  , "exp master"   ) \
    MBEDTLS_SSL_TLS1_3_LABEL( ext_binder  , "ext binder"   ) \
    MBEDTLS_SSL_TLS1_3_LABEL( res_binder  , "res binder"   ) \
    MBEDTLS_SSL_TLS1_3_LABEL( derived     , "derived"      )

#define MBEDTLS_SSL_TLS1_3_LABEL( name, string )       \
    const unsigned char name    [ sizeof(string) - 1 ];

union mbedtls_ssl_tls1_3_labels_union
{
    MBEDTLS_SSL_TLS1_3_LABEL_LIST
};
struct mbedtls_ssl_tls1_3_labels_struct
{
    MBEDTLS_SSL_TLS1_3_LABEL_LIST
};
#undef MBEDTLS_SSL_TLS1_3_LABEL

extern const struct mbedtls_ssl_tls1_3_labels_struct mbedtls_ssl_tls1_3_labels;

#define MBEDTLS_SSL_TLS1_3_LBL_WITH_LEN( LABEL )  \
    mbedtls_ssl_tls1_3_labels.LABEL,              \
    sizeof(mbedtls_ssl_tls1_3_labels.LABEL)

#define MBEDTLS_SSL_TLS1_3_KEY_SCHEDULE_MAX_LABEL_LEN  \
    sizeof( union mbedtls_ssl_tls1_3_labels_union )

/* The maximum length of HKDF contexts used in the TLS 1.3 standard.
 * Since contexts are always hashes of message transcripts, this can
 * be approximated from above by the maximum hash size. */
#define MBEDTLS_SSL_TLS1_3_KEY_SCHEDULE_MAX_CONTEXT_LEN  \
    MBEDTLS_MD_MAX_SIZE

/* Maximum desired length for expanded key material generated
 * by HKDF-Expand-Label.
 *
 * Warning: If this ever needs to be increased, the implementation
 * ssl_tls1_3_hkdf_encode_label() in ssl_tls13_keys.c needs to be
 * adjusted since it currently assumes that HKDF key expansion
 * is never used with more than 255 Bytes of output. */
#define MBEDTLS_SSL_TLS1_3_KEY_SCHEDULE_MAX_EXPANSION_LEN 255

/**
 * \brief           The \c HKDF-Expand-Label function from
 *                  the TLS 1.3 standard RFC 8446.
 *
 * <tt>
 *                  HKDF-Expand-Label( Secret, Label, Context, Length ) =
 *                       HKDF-Expand( Secret, HkdfLabel, Length )
 * </tt>
 *
 * \param hash_alg  The identifier for the hash algorithm to use.
 * \param secret    The \c Secret argument to \c HKDF-Expand-Label.
 *                  This must be a readable buffer of length \p slen Bytes.
 * \param slen      The length of \p secret in Bytes.
 * \param label     The \c Label argument to \c HKDF-Expand-Label.
 *                  This must be a readable buffer of length \p llen Bytes.
 * \param llen      The length of \p label in Bytes.
 * \param ctx       The \c Context argument to \c HKDF-Expand-Label.
 *                  This must be a readable buffer of length \p clen Bytes.
 * \param clen      The length of \p context in Bytes.
 * \param buf       The destination buffer to hold the expanded secret.
 *                  This must be a writable buffer of length \p blen Bytes.
 * \param blen      The desired size of the expanded secret in Bytes.
 *
 * \returns         \c 0 on success.
 * \return          A negative error code on failure.
 */

int mbedtls_ssl_tls1_3_hkdf_expand_label(
                     mbedtls_md_type_t hash_alg,
                     const unsigned char *secret, size_t slen,
                     const unsigned char *label, size_t llen,
                     const unsigned char *ctx, size_t clen,
                     unsigned char *buf, size_t blen );

/**
 * \brief           This function is part of the TLS 1.3 key schedule.
 *                  It extracts key and IV for the actual client/server traffic
 *                  from the client/server traffic secrets.
 *
 * From RFC 8446:
 *
 * <tt>
 *   [sender]_write_key = HKDF-Expand-Label(Secret, "key", "", key_length)
 *   [sender]_write_iv  = HKDF-Expand-Label(Secret, "iv", "", iv_length)*
 * </tt>
 *
 * \param hash_alg      The identifier for the hash algorithm to be used
 *                      for the HKDF-based expansion of the secret.
 * \param client_secret The client traffic secret.
 *                      This must be a readable buffer of size \p slen Bytes
 * \param server_secret The server traffic secret.
 *                      This must be a readable buffer of size \p slen Bytes
 * \param slen          Length of the secrets \p client_secret and
 *                      \p server_secret in Bytes.
 * \param key_len       The desired length of the key to be extracted in Bytes.
 * \param iv_len        The desired length of the IV to be extracted in Bytes.
 * \param keys          The address of the structure holding the generated
 *                      keys and IVs.
 *
 * \returns             \c 0 on success.
 * \returns             A negative error code on failure.
 */

int mbedtls_ssl_tls1_3_make_traffic_keys(
                     mbedtls_md_type_t hash_alg,
                     const unsigned char *client_secret,
                     const unsigned char *server_secret,
                     size_t slen, size_t key_len, size_t iv_len,
                     mbedtls_ssl_key_set *keys );


#define MBEDTLS_SSL_TLS1_3_CONTEXT_UNHASHED 0
#define MBEDTLS_SSL_TLS1_3_CONTEXT_HASHED   1

/**
 * \brief The \c Derive-Secret function from the TLS 1.3 standard RFC 8446.
 *
 * <tt>
 *   Derive-Secret( Secret, Label, Messages ) =
 *      HKDF-Expand-Label( Secret, Label,
 *                         Hash( Messages ),
 *                         Hash.Length ) )
 * </tt>
 *
 * \param hash_alg   The identifier for the hash function used for the
 *                   applications of HKDF.
 * \param secret     The \c Secret argument to the \c Derive-Secret function.
 *                   This must be a readable buffer of length \p slen Bytes.
 * \param slen       The length of \p secret in Bytes.
 * \param label      The \c Label argument to the \c Derive-Secret function.
 *                   This must be a readable buffer of length \p llen Bytes.
 * \param llen       The length of \p label in Bytes.
 * \param ctx        The hash of the \c Messages argument to the
 *                   \c Derive-Secret function, or the \c Messages argument
 *                   itself, depending on \p context_already_hashed.
 * \param clen       The length of \p hash.
 * \param ctx_hashed This indicates whether the \p ctx contains the hash of
 *                   the \c Messages argument in the application of the
 *                   \c Derive-Secret function
 *                   (value MBEDTLS_SSL_TLS1_3_CONTEXT_HASHED), or whether
 *                   it is the content of \c Messages itself, in which case
 *                   the function takes care of the hashing
 *                   (value MBEDTLS_SSL_TLS1_3_CONTEXT_UNHASHED).
 * \param dstbuf     The target buffer to write the output of
 *                   \c Derive-Secret to. This must be a writable buffer of
 *                   size \p buflen Bytes.
 * \param buflen     The length of \p dstbuf in Bytes.
 *
 * \returns        \c 0 on success.
 * \returns        A negative error code on failure.
 */
int mbedtls_ssl_tls1_3_derive_secret(
                   mbedtls_md_type_t hash_alg,
                   const unsigned char *secret, size_t slen,
                   const unsigned char *label, size_t llen,
                   const unsigned char *ctx, size_t clen,
                   int ctx_hashed,
                   unsigned char *dstbuf, size_t buflen );

/**
 * \brief Derive TLS 1.3 early data key material from early secret.
 *
 *        This is a small wrapper invoking mbedtls_ssl_tls1_3_derive_secret()
 *        with the appropriate labels.
 *
 * <tt>
 *        Early Secret
 *             |
 *             +-----> Derive-Secret(., "c e traffic", ClientHello)
 *             |                      = client_early_traffic_secret
 *             |
 *             +-----> Derive-Secret(., "e exp master", ClientHello)
 *             .                      = early_exporter_master_secret
 *             .
 *             .
 * </tt>
 *
 * \note  To obtain the actual key and IV for the early data traffic,
 *        the client secret derived by this function need to be
 *        further processed by mbedtls_ssl_tls1_3_make_traffic_keys().
 *
 * \note  The binder key, which is also generated from the early secret,
 *        is omitted here. Its calculation is part of the separate routine
 *        mbedtls_ssl_tls1_3_create_psk_binder().
 *
 * \param md_type      The hash algorithm associated with the PSK for which
 *                     early data key material is being derived.
 * \param early_secret The early secret from which the early data key material
 *                     should be derived. This must be a readable buffer whose
 *                     length is the digest size of the hash algorithm
 *                     represented by \p md_size.
 * \param transcript   The transcript of the handshake so far, calculated with
 *                     respect to \p md_type. This must be a readable buffer
 *                     whose length is the digest size of the hash algorithm
 *                     represented by \p md_size.
 * \param derived      The address of the structure in which to store
 *                     the early data key material.
 *
 * \returns        \c 0 on success.
 * \returns        A negative error code on failure.
 */
int mbedtls_ssl_tls1_3_derive_early_secrets(
          mbedtls_md_type_t md_type,
          unsigned char const *early_secret,
          unsigned char const *transcript, size_t transcript_len,
          mbedtls_ssl_tls1_3_early_secrets *derived );

/**
 * \brief Derive TLS 1.3 handshake key material from the handshake secret.
 *
 *        This is a small wrapper invoking mbedtls_ssl_tls1_3_derive_secret()
 *        with the appropriate labels from the standard.
 *
 * <tt>
 *        Handshake Secret
 *              |
 *              +-----> Derive-Secret( ., "c hs traffic",
 *              |                      ClientHello...ServerHello )
 *              |                      = client_handshake_traffic_secret
 *              |
 *              +-----> Derive-Secret( ., "s hs traffic",
 *              .                      ClientHello...ServerHello )
 *              .                      = server_handshake_traffic_secret
 *              .
 * </tt>
 *
 * \note  To obtain the actual key and IV for the encrypted handshake traffic,
 *        the client and server secret derived by this function need to be
 *        further processed by mbedtls_ssl_tls1_3_make_traffic_keys().
 *
 * \param md_type           The hash algorithm associated with the ciphersuite
 *                          that's being used for the connection.
 * \param handshake_secret  The handshake secret from which the handshake key
 *                          material should be derived. This must be a readable
 *                          buffer whose length is the digest size of the hash
 *                          algorithm represented by \p md_size.
 * \param transcript        The transcript of the handshake so far, calculated
 *                          with respect to \p md_type. This must be a readable
 *                          buffer whose length is the digest size of the hash
 *                          algorithm represented by \p md_size.
 * \param derived           The address of the structure in which to
 *                          store the handshake key material.
 *
 * \returns        \c 0 on success.
 * \returns        A negative error code on failure.
 */
int mbedtls_ssl_tls1_3_derive_handshake_secrets(
          mbedtls_md_type_t md_type,
          unsigned char const *handshake_secret,
          unsigned char const *transcript, size_t transcript_len,
          mbedtls_ssl_tls1_3_handshake_secrets *derived );

/**
 * \brief Derive TLS 1.3 application key material from the master secret.
 *
 *        This is a small wrapper invoking mbedtls_ssl_tls1_3_derive_secret()
 *        with the appropriate labels from the standard.
 *
 * <tt>
 *        Master Secret
 *              |
 *              +-----> Derive-Secret( ., "c ap traffic",
 *              |                      ClientHello...server Finished )
 *              |                      = client_application_traffic_secret_0
 *              |
 *              +-----> Derive-Secret( ., "s ap traffic",
 *              |                      ClientHello...Server Finished )
 *              |                      = server_application_traffic_secret_0
 *              |
 *              +-----> Derive-Secret( ., "exp master",
 *              .                      ClientHello...server Finished)
 *              .                      = exporter_master_secret
 *              .
 * </tt>
 *
 * \note  To obtain the actual key and IV for the (0-th) application traffic,
 *        the client and server secret derived by this function need to be
 *        further processed by mbedtls_ssl_tls1_3_make_traffic_keys().
 *
 * \param md_type           The hash algorithm associated with the ciphersuite
 *                          that's being used for the connection.
 * \param master_secret     The master secret from which the application key
 *                          material should be derived. This must be a readable
 *                          buffer whose length is the digest size of the hash
 *                          algorithm represented by \p md_size.
 * \param transcript        The transcript of the handshake up to and including
 *                          the ServerFinished message, calculated with respect
 *                          to \p md_type. This must be a readable buffer whose
 *                          length is the digest size of the hash algorithm
 *                          represented by \p md_type.
 * \param derived           The address of the structure in which to
 *                          store the application key material.
 *
 * \returns        \c 0 on success.
 * \returns        A negative error code on failure.
 */
int mbedtls_ssl_tls1_3_derive_application_secrets(
          mbedtls_md_type_t md_type,
          unsigned char const *master_secret,
          unsigned char const *transcript, size_t transcript_len,
          mbedtls_ssl_tls1_3_application_secrets *derived );

/**
 * \brief Derive TLS 1.3 resumption master secret from the master secret.
 *
 *        This is a small wrapper invoking mbedtls_ssl_tls1_3_derive_secret()
 *        with the appropriate labels from the standard.
 *
 * \param md_type           The hash algorithm used in the application for which
 *                          key material is being derived.
 * \param application_secret The application secret from which the resumption master
 *                          secret should be derived. This must be a readable
 *                          buffer whose length is the digest size of the hash
 *                          algorithm represented by \p md_size.
 * \param transcript        The transcript of the handshake up to and including
 *                          the ClientFinished message, calculated with respect
 *                          to \p md_type. This must be a readable buffer whose
 *                          length is the digest size of the hash algorithm
 *                          represented by \p md_type.
 * \param transcript_len    The length of \p transcript in Bytes.
 * \param derived           The address of the structure in which to
 *                          store the resumption master secret.
 *
 * \returns        \c 0 on success.
 * \returns        A negative error code on failure.
 */
int mbedtls_ssl_tls1_3_derive_resumption_master_secret(
          mbedtls_md_type_t md_type,
          unsigned char const *application_secret,
          unsigned char const *transcript, size_t transcript_len,
          mbedtls_ssl_tls1_3_application_secrets *derived );

/**
 * \brief Compute the next secret in the TLS 1.3 key schedule
 *
 * The TLS 1.3 key schedule proceeds as follows to compute
 * the three main secrets during the handshake: The early
 * secret for early data, the handshake secret for all
 * other encrypted handshake messages, and the master
 * secret for all application traffic.
 *
 * <tt>
 *                    0
 *                    |
 *                    v
 *     PSK ->  HKDF-Extract = Early Secret
 *                    |
 *                    v
 *     Derive-Secret( ., "derived", "" )
 *                    |
 *                    v
 *  (EC)DHE -> HKDF-Extract = Handshake Secret
 *                    |
 *                    v
 *     Derive-Secret( ., "derived", "" )
 *                    |
 *                    v
 *     0 -> HKDF-Extract = Master Secret
 * </tt>
 *
 * Each of the three secrets in turn is the basis for further
 * key derivations, such as the derivation of traffic keys and IVs;
 * see e.g. mbedtls_ssl_tls1_3_make_traffic_keys().
 *
 * This function implements one step in this evolution of secrets:
 *
 * <tt>
 *                old_secret
 *                    |
 *                    v
 *     Derive-Secret( ., "derived", "" )
 *                    |
 *                    v
 *     input -> HKDF-Extract = new_secret
 * </tt>
 *
 * \param hash_alg    The identifier for the hash function used for the
 *                    applications of HKDF.
 * \param secret_old  The address of the buffer holding the old secret
 *                    on function entry. If not \c NULL, this must be a
 *                    readable buffer whose size matches the output size
 *                    of the hash function represented by \p hash_alg.
 *                    If \c NULL, an all \c 0 array will be used instead.
 * \param input       The address of the buffer holding the additional
 *                    input for the key derivation (e.g., the PSK or the
 *                    ephemeral (EC)DH secret). If not \c NULL, this must be
 *                    a readable buffer whose size \p input_len Bytes.
 *                    If \c NULL, an all \c 0 array will be used instead.
 * \param input_len   The length of \p input in Bytes.
 * \param secret_new  The address of the buffer holding the new secret
 *                    on function exit. This must be a writable buffer
 *                    whose size matches the output size of the hash
 *                    function represented by \p hash_alg.
 *                    This may be the same as \p secret_old.
 *
 * \returns           \c 0 on success.
 * \returns           A negative error code on failure.
 */

int mbedtls_ssl_tls1_3_evolve_secret(
                   mbedtls_md_type_t hash_alg,
                   const unsigned char *secret_old,
                   const unsigned char *input, size_t input_len,
                   unsigned char *secret_new );

#define MBEDTLS_SSL_TLS1_3_PSK_EXTERNAL   0
#define MBEDTLS_SSL_TLS1_3_PSK_RESUMPTION 1

/**
 * \brief             Calculate a TLS 1.3 PSK binder.
 *
 * \param ssl         The SSL context. This is used for debugging only and may
 *                    be \c NULL if MBEDTLS_DEBUG_C is disabled.
 * \param md_type     The hash algorithm associated to the PSK \p psk.
 * \param psk         The buffer holding the PSK for which to create a binder.
 * \param psk_len     The size of \p psk in bytes.
 * \param psk_type    This indicates whether the PSK \p psk is externally
 *                    provisioned (#MBEDTLS_SSL_TLS1_3_PSK_EXTERNAL) or a
 *                    resumption PSK (#MBEDTLS_SSL_TLS1_3_PSK_RESUMPTION).
 * \param transcript  The handshake transcript up to the point where the
 *                    PSK binder calculation happens. This must be readable,
 *                    and its size must be equal to the digest size of
 *                    the hash algorithm represented by \p md_type.
 * \param result      The address at which to store the PSK binder on success.
 *                    This must be writable, and its size must be equal to the
 *                    digest size of  the hash algorithm represented by
 *                    \p md_type.
 *
 * \returns           \c 0 on success.
 * \returns           A negative error code on failure.
 */
int mbedtls_ssl_tls1_3_create_psk_binder( mbedtls_ssl_context *ssl,
                               const mbedtls_md_type_t md_type,
                               unsigned char const *psk, size_t psk_len,
                               int psk_type,
                               unsigned char const *transcript,
                               unsigned char *result );

/**
 * \bref Setup an SSL transform structure representing the
 *       record protection mechanism used by TLS 1.3
 *
 * \param transform    The SSL transform structure to be created. This must have
 *                     been initialized through mbedtls_ssl_transform_init() and
 *                     not used in any other way prior to calling this function.
 *                     In particular, this function does not clean up the
 *                     transform structure prior to installing the new keys.
 * \param endpoint     Indicates whether the transform is for the client
 *                     (value #MBEDTLS_SSL_IS_CLIENT) or the server
 *                     (value #MBEDTLS_SSL_IS_SERVER).
 * \param ciphersuite  The numerical identifier for the ciphersuite to use.
 *                     This must be one of the identifiers listed in
 *                     ssl_ciphersuites.h.
 * \param traffic_keys The key material to use. No reference is stored in
 *                     the SSL transform being generated, and the caller
 *                     should destroy the key material afterwards.
 * \param ssl          (Debug-only) The SSL context to use for debug output
 *                     in case of failure. This parameter is only needed if
 *                     #MBEDTLS_DEBUG_C is set, and is ignored otherwise.
 *
 * \return             \c 0 on success. In this case, \p transform is ready to
 *                     be used with mbedtls_ssl_transform_decrypt() and
 *                     mbedtls_ssl_transform_encrypt().
 * \return             A negative error code on failure.
 */
int mbedtls_ssl_tls13_populate_transform( mbedtls_ssl_transform *transform,
                                          int endpoint,
                                          int ciphersuite,
                                          mbedtls_ssl_key_set const *traffic_keys,
                                          mbedtls_ssl_context *ssl );

/*
 * TLS 1.3 key schedule evolutions
 *
 *   Early -> Handshake -> Application
 *
 * Small wrappers around mbedtls_ssl_tls1_3_evolve_secret().
 */

/**
 * \brief Begin TLS 1.3 key schedule by calculating early secret.
 *
 *        The TLS 1.3 key schedule can be viewed as a simple state machine
 *        with states Initial -> Early -> Handshake -> Application, and
 *        this function represents the Initial -> Early transition.
 *
 * \param ssl  The SSL context to operate on.
 *
 * \returns    \c 0 on success.
 * \returns    A negative error code on failure.
 */
int mbedtls_ssl_tls1_3_key_schedule_stage_early( mbedtls_ssl_context *ssl );

/**
 * \brief Transition into handshake stage of TLS 1.3 key schedule.
 *
 *        The TLS 1.3 key schedule can be viewed as a simple state machine
 *        with states Initial -> Early -> Handshake -> Application, and
 *        this function represents the Early -> Handshake transition.
 *
 *        In the handshake stage, mbedtls_ssl_tls13_generate_handshake_keys()
 *        can be used to derive the handshake traffic keys.
 *
 * \param ssl  The SSL context to operate on. This must be in key schedule
 *             stage \c Early.
 *
 * \returns    \c 0 on success.
 * \returns    A negative error code on failure.
 */
int mbedtls_ssl_tls13_key_schedule_stage_handshake( mbedtls_ssl_context *ssl );

/**
 * \brief Compute TLS 1.3 handshake traffic keys.
 *
 * \param ssl  The SSL context to operate on. This must be in
 *             key schedule stage \c Handshake, see
 *             mbedtls_ssl_tls13_key_schedule_stage_handshake().
 * \param traffic_keys The address at which to store the handshake traffic key
 *                     keys. This must be writable but may be uninitialized.
 *
 * \returns    \c 0 on success.
 * \returns    A negative error code on failure.
 */
int mbedtls_ssl_tls13_generate_handshake_keys( mbedtls_ssl_context *ssl,
                                               mbedtls_ssl_key_set *traffic_keys );

#endif /* MBEDTLS_SSL_TLS1_3_KEYS_H */
