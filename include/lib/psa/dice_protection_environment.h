/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DICE_PROTECTION_ENVIRONMENT_H
#define DICE_PROTECTION_ENVIRONMENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <dice.h>

/* Additional defines for max size limit. These limits are set by DPE in RSE. */
#define DICE_AUTHORITY_DESCRIPTOR_MAX_SIZE	64
#define DICE_CONFIG_DESCRIPTOR_MAX_SIZE		64
#define DICE_CODE_DESCRIPTOR_MAX_SIZE		32

typedef int32_t dpe_error_t;

#define DPE_NO_ERROR			((dpe_error_t)0)
#define DPE_INTERNAL_ERROR		((dpe_error_t)1)
#define DPE_INVALID_COMMAND		((dpe_error_t)2)
#define DPE_INVALID_ARGUMENT		((dpe_error_t)3)
#define DPE_ARGUMENT_NOT_SUPPORTED	((dpe_error_t)4)
#define DPE_SESSION_EXHAUSTED		((dpe_error_t)5)

/* Custom values in RSE based DPE implementation */
#define DPE_INSUFFICIENT_MEMORY		((dpe_error_t)128)
#define DPE_ERR_CBOR_FORMATTING		((dpe_error_t)129)

/**
 * Client facing API. Parameters are according to the DPE spec version r0.9
 *
 * \brief Performs the DICE computation to derive a new context and optionally
 *        creates an intermediate certificate. Software component measurement
 *        must be provided in dice_inputs.
 *
 * \param[in]  context_handle              Input context handle for the DPE
 *                                         context.
 * \param[in]  cert_id                     Logical certificate id to which derived
 *                                         context belongs to.
 * \param[in]  retain_parent_context       Flag to indicate whether to retain the
 *                                         parent context. True only if a client
 *                                         will call further DPE commands on the
 *                                         same context.
 * \param[in]  allow_new_context_to_derive Flag to indicate whether derived context
 *                                         can derive further. True only if the
 *                                         new context will load further components.
 * \param[in]  create_certificate          Flag to indicate whether to create an
 *                                         intermediate certificate. True only if
 *                                         it is the last component in the layer.
 * \param[in]  dice_inputs                 DICE input values.
 * \param[in]  target_locality             Identifies the locality to which the
 *                                         derived context will be bound. Could be
 *                                         MHU id.
 * \param[in]  return_certificate          Indicates whether to return the generated
 *                                         certificate when create_certificate is true.
 * \param[in]  allow_new_context_to_export Indicates whether the DPE permits export of
 *                                         the CDI from the newly derived context.
 * \param[in]  export_cdi                  Indicates whether to export derived CDI.
 * \param[out] new_context_handle          New handle for the derived context.
 * \param[out] new_parent_context_handle   New handle for the parent context.
 * \param[out] new_certificate_buf         If create_certificate and return_certificate
 *                                         are both true, this argument holds the new
 *                                         certificate generated for the new context
 * \param[in]  new_certificate_buf_size    Size of the allocated buffer for
 *                                         new certificate.
 * \param[out] new_certificate_actual_size Actual size of the new certificate.
 * \param[out] exported_cdi_buf            If export_cdi is true, this is the
 *                                         exported CDI value.
 * \param[in]  exported_cdi_buf_size       Size of the allocated buffer for
 *                                         exported cdi.
 * \param[out] exported_cdi_actual_size    Actual size of the exported cdi.
 *
 * \return Returns error code of type dpe_error_t
 */
dpe_error_t dpe_derive_context(int      context_handle,
			       uint32_t cert_id,
			       bool     retain_parent_context,
			       bool     allow_new_context_to_derive,
			       bool     create_certificate,
			       const DiceInputValues *dice_inputs,
			       int32_t  target_locality,
			       bool     return_certificate,
			       bool     allow_new_context_to_export,
			       bool     export_cdi,
			       int     *new_context_handle,
			       int     *new_parent_context_handle,
			       uint8_t *new_certificate_buf,
			       size_t   new_certificate_buf_size,
			       size_t  *new_certificate_actual_size,
			       uint8_t *exported_cdi_buf,
			       size_t   exported_cdi_buf_size,
			       size_t  *exported_cdi_actual_size);

#endif /* DICE_PROTECTION_ENVIRONMENT_H */
