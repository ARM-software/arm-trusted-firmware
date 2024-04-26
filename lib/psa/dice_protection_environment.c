/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <qcbor/qcbor_decode.h>
#include <qcbor/qcbor_encode.h>
#include <qcbor/qcbor_spiffy_decode.h>

#include <common/debug.h>
#include <dice.h>
#include <dice_protection_environment.h>
#include <psa/client.h>
#include <psa_manifest/sid.h>

enum dpe_command_id_t {
	/* Standard commands */
	DPE_GET_PROFILE = 1,
	DPE_OPEN_SESSION = 2,
	DPE_CLOSE_SESSION = 3,
	DPE_SYNC_SESSION = 4,
	DPE_EXPORT_SESSION = 5,
	DPE_IMPORT_SESSION = 6,
	DPE_INITIALIZE_CONTEXT = 7,
	DPE_DERIVE_CONTEXT = 8,
	DPE_CERTIFY_KEY = 9,
	DPE_SIGN = 10,
	DPE_SEAL = 11,
	DPE_UNSEAL = 12,
	DPE_DERIVE_SEALING_PUBLIC_KEY = 13,
	DPE_ROTATE_CONTEXT_HANDLE = 14,
	DPE_DESTROY_CONTEXT = 15,
};

enum dice_input_labels_t {
	DICE_CODE_HASH = 1,
	DICE_CODE_DESCRIPTOR = 2,
	DICE_CONFIG_TYPE = 3,
	DICE_CONFIG_VALUE = 4,
	DICE_CONFIG_DESCRIPTOR = 5,
	DICE_AUTHORITY_HASH = 6,
	DICE_AUTHORITY_DESCRIPTOR = 7,
	DICE_MODE = 8,
	DICE_HIDDEN = 9,
};

enum dpe_derive_context_input_labels_t {
	DPE_DERIVE_CONTEXT_CONTEXT_HANDLE = 1,
	DPE_DERIVE_CONTEXT_RETAIN_PARENT_CONTEXT = 2,
	DPE_DERIVE_CONTEXT_ALLOW_NEW_CONTEXT_TO_DERIVE = 3,
	DPE_DERIVE_CONTEXT_CREATE_CERTIFICATE = 4,
	DPE_DERIVE_CONTEXT_NEW_SESSION_INITIATOR_HANDSHAKE = 5,
	DPE_DERIVE_CONTEXT_INPUT_DATA = 6,
	DPE_DERIVE_CONTEXT_INTERNAL_INPUTS = 7,
	DPE_DERIVE_CONTEXT_TARGET_LOCALITY = 8,
	DPE_DERIVE_CONTEXT_RETURN_CERTIFICATE = 9,
	DPE_DERIVE_CONTEXT_ALLOW_NEW_CONTEXT_TO_EXPORT = 10,
	DPE_DERIVE_CONTEXT_EXPORT_CDI = 11,
	/* enum values 256 and onwards are reserved for custom arguments */
	DPE_DERIVE_CONTEXT_CERT_ID = 256,
};

enum dpe_derive_context_output_labels_t {
	DPE_DERIVE_CONTEXT_NEW_CONTEXT_HANDLE = 1,
	DPE_DERIVE_CONTEXT_NEW_SESSION_RESPONDER_HANDSHAKE = 2,
	DPE_DERIVE_CONTEXT_PARENT_CONTEXT_HANDLE = 3,
	DPE_DERIVE_CONTEXT_NEW_CERTIFICATE = 4,
	DPE_DERIVE_CONTEXT_EXPORTED_CDI = 5,
};

struct derive_context_input_t {
	int context_handle;
	uint32_t cert_id;
	bool retain_parent_context;
	bool allow_new_context_to_derive;
	bool create_certificate;
	const DiceInputValues *dice_inputs;
	int32_t target_locality;
	bool return_certificate;
	bool allow_new_context_to_export;
	bool export_cdi;
};

struct derive_context_output_t {
	int new_context_handle;
	int new_parent_context_handle;
	const uint8_t *new_certificate;
	size_t new_certificate_size;
	const uint8_t *exported_cdi;
	size_t exported_cdi_size;
};

static void encode_dice_inputs(QCBOREncodeContext *encode_ctx,
			       const DiceInputValues *input)
{
	/* Wrap the DICE inputs into a byte string */
	QCBOREncode_BstrWrapInMapN(encode_ctx, DPE_DERIVE_CONTEXT_INPUT_DATA);

	/* Inside the byte string the DICE inputs are encoded as a map */
	QCBOREncode_OpenMap(encode_ctx);

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CODE_HASH,
				  (UsefulBufC) { input->code_hash,
						 sizeof(input->code_hash) });

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CODE_DESCRIPTOR,
				   (UsefulBufC) { input->code_descriptor,
						  input->code_descriptor_size });

	QCBOREncode_AddInt64ToMapN(encode_ctx, DICE_CONFIG_TYPE,
				   input->config_type);

	if (input->config_type == kDiceConfigTypeInline) {
		QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CONFIG_VALUE,
					   (UsefulBufC) { input->config_value,
							  sizeof(input->config_value) });
	} else {
		QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CONFIG_DESCRIPTOR,
					   (UsefulBufC) { input->config_descriptor,
							  input->config_descriptor_size });
	}

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_AUTHORITY_HASH,
				   (UsefulBufC) { input->authority_hash,
						  sizeof(input->authority_hash) });

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_AUTHORITY_DESCRIPTOR,
				   (UsefulBufC) { input->authority_descriptor,
						  input->authority_descriptor_size });

	QCBOREncode_AddInt64ToMapN(encode_ctx, DICE_MODE, input->mode);

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_HIDDEN,
				   (UsefulBufC) { input->hidden,
						  sizeof(input->hidden) });

	QCBOREncode_CloseMap(encode_ctx);
	QCBOREncode_CloseBstrWrap2(encode_ctx, true, NULL);
}

static QCBORError encode_derive_context(const struct derive_context_input_t *args,
					UsefulBuf buf,
					UsefulBufC *encoded_buf)
{
	QCBOREncodeContext encode_ctx;

	QCBOREncode_Init(&encode_ctx, buf);

	QCBOREncode_OpenArray(&encode_ctx);
	QCBOREncode_AddUInt64(&encode_ctx, DPE_DERIVE_CONTEXT);

	/* Encode DeriveContext command */
	QCBOREncode_OpenMap(&encode_ctx);
	QCBOREncode_AddBytesToMapN(&encode_ctx,
				   DPE_DERIVE_CONTEXT_CONTEXT_HANDLE,
				   (UsefulBufC) { &args->context_handle,
						  sizeof(args->context_handle) });
	QCBOREncode_AddUInt64ToMapN(&encode_ctx,
				    DPE_DERIVE_CONTEXT_CERT_ID,
				    args->cert_id);
	QCBOREncode_AddBoolToMapN(&encode_ctx,
				  DPE_DERIVE_CONTEXT_RETAIN_PARENT_CONTEXT,
				  args->retain_parent_context);
	QCBOREncode_AddBoolToMapN(&encode_ctx,
				  DPE_DERIVE_CONTEXT_ALLOW_NEW_CONTEXT_TO_DERIVE,
				  args->allow_new_context_to_derive);
	QCBOREncode_AddBoolToMapN(&encode_ctx,
				  DPE_DERIVE_CONTEXT_CREATE_CERTIFICATE,
				  args->create_certificate);
	encode_dice_inputs(&encode_ctx, args->dice_inputs);
	QCBOREncode_AddBytesToMapN(&encode_ctx,
				   DPE_DERIVE_CONTEXT_TARGET_LOCALITY,
				   (UsefulBufC) { &args->target_locality,
						  sizeof(args->target_locality) });
	QCBOREncode_AddBoolToMapN(&encode_ctx,
				  DPE_DERIVE_CONTEXT_RETURN_CERTIFICATE,
				  args->return_certificate);
	QCBOREncode_AddBoolToMapN(&encode_ctx,
				  DPE_DERIVE_CONTEXT_ALLOW_NEW_CONTEXT_TO_EXPORT,
				  args->allow_new_context_to_export);
	QCBOREncode_AddBoolToMapN(&encode_ctx,
				  DPE_DERIVE_CONTEXT_EXPORT_CDI,
				  args->export_cdi);
	QCBOREncode_CloseMap(&encode_ctx);

	QCBOREncode_CloseArray(&encode_ctx);

	return QCBOREncode_Finish(&encode_ctx, encoded_buf);
}

static QCBORError decode_derive_context_response(UsefulBufC encoded_buf,
						 struct derive_context_output_t *args,
						 dpe_error_t *dpe_err)
{
	QCBORDecodeContext decode_ctx;
	UsefulBufC out;
	int64_t response_dpe_err;

	QCBORDecode_Init(&decode_ctx, encoded_buf, QCBOR_DECODE_MODE_NORMAL);

	QCBORDecode_EnterArray(&decode_ctx, NULL);

	/* Get the error code from the response. DPE returns int32_t */
	QCBORDecode_GetInt64(&decode_ctx, &response_dpe_err);
	*dpe_err = (dpe_error_t)response_dpe_err;

	/* Decode DeriveContext response if successful */
	if (*dpe_err == DPE_NO_ERROR) {
		QCBORDecode_EnterMap(&decode_ctx, NULL);

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_DERIVE_CONTEXT_NEW_CONTEXT_HANDLE,
						&out);
		if (out.len != sizeof(args->new_context_handle)) {
			return QCBORDecode_Finish(&decode_ctx);
		}
		memcpy(&args->new_context_handle, out.ptr, out.len);

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_DERIVE_CONTEXT_PARENT_CONTEXT_HANDLE,
						&out);
		if (out.len != sizeof(args->new_parent_context_handle)) {
			return QCBORDecode_Finish(&decode_ctx);
		}
		memcpy(&args->new_parent_context_handle, out.ptr, out.len);

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_DERIVE_CONTEXT_NEW_CERTIFICATE,
						&out);
		args->new_certificate = out.ptr;
		args->new_certificate_size = out.len;

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_DERIVE_CONTEXT_EXPORTED_CDI,
						&out);
		args->exported_cdi = out.ptr;
		args->exported_cdi_size = out.len;

		QCBORDecode_ExitMap(&decode_ctx);
	}

	QCBORDecode_ExitArray(&decode_ctx);

	return QCBORDecode_Finish(&decode_ctx);
}

static int32_t dpe_client_call(const char *cmd_input, size_t cmd_input_size,
			       char *cmd_output, size_t *cmd_output_size)
{
	int32_t err;

	psa_invec in_vec[] = {
		{ cmd_input, cmd_input_size },
	};
	psa_outvec out_vec[] = {
		{ cmd_output, *cmd_output_size },
	};

	err = psa_call(RSE_DPE_SERVICE_HANDLE, 0,
			in_vec, IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));

	if (err == PSA_SUCCESS) {
		*cmd_output_size = out_vec[0].len;
	}

	return err;
}

dpe_error_t dpe_derive_context(int context_handle,
			       uint32_t cert_id,
			       bool retain_parent_context,
			       bool allow_new_context_to_derive,
			       bool create_certificate,
			       const DiceInputValues *dice_inputs,
			       int32_t target_locality,
			       bool return_certificate,
			       bool allow_new_context_to_export,
			       bool export_cdi,
			       int *new_context_handle,
			       int *new_parent_context_handle,
			       uint8_t *new_certificate_buf,
			       size_t new_certificate_buf_size,
			       size_t *new_certificate_actual_size,
			       uint8_t *exported_cdi_buf,
			       size_t exported_cdi_buf_size,
			       size_t *exported_cdi_actual_size)
{
	int32_t service_err;
	dpe_error_t dpe_err;
	QCBORError qcbor_err;
	UsefulBufC encoded_buf;
	UsefulBuf_MAKE_STACK_UB(cmd_buf, 612);

	const struct derive_context_input_t in_args = {
		context_handle,
		cert_id,
		retain_parent_context,
		allow_new_context_to_derive,
		create_certificate,
		dice_inputs,
		target_locality,
		return_certificate,
		allow_new_context_to_export,
		export_cdi,
	};
	struct derive_context_output_t out_args;

	/*
	 * Validate the output params here because they are not sent to the
	 * service. Input params are validated by the DPE service.
	 */
	if ((new_context_handle == NULL) ||
	    (retain_parent_context == true && new_parent_context_handle == NULL) ||
	    (return_certificate == true &&
		(new_certificate_buf == NULL || new_certificate_actual_size == NULL)) ||
	    (export_cdi == true &&
		(exported_cdi_buf == NULL || exported_cdi_actual_size == NULL))) {
		return DPE_INVALID_ARGUMENT;
	}

	qcbor_err = encode_derive_context(&in_args, cmd_buf, &encoded_buf);
	if (qcbor_err != QCBOR_SUCCESS) {
		return DPE_INTERNAL_ERROR;
	}

	service_err = dpe_client_call(encoded_buf.ptr, encoded_buf.len,
				      cmd_buf.ptr, &cmd_buf.len);
	if (service_err != 0) {
		return DPE_INTERNAL_ERROR;
	}

	qcbor_err = decode_derive_context_response(UsefulBuf_Const(cmd_buf),
						   &out_args, &dpe_err);
	if (qcbor_err != QCBOR_SUCCESS) {
		return DPE_INTERNAL_ERROR;
	} else if (dpe_err != DPE_NO_ERROR) {
		return dpe_err;
	}

	/* Copy returned values into caller's memory */
	*new_context_handle = out_args.new_context_handle;

	if (retain_parent_context == true) {
		*new_parent_context_handle = out_args.new_parent_context_handle;
	}

	if (return_certificate == true) {
		if (out_args.new_certificate_size > new_certificate_buf_size) {
			return DPE_INVALID_ARGUMENT;
		}

		memcpy(new_certificate_buf, out_args.new_certificate,
			out_args.new_certificate_size);
		*new_certificate_actual_size = out_args.new_certificate_size;
	}

	if (export_cdi == true) {
		if (out_args.exported_cdi_size > exported_cdi_buf_size) {
			return DPE_INVALID_ARGUMENT;
		}

		memcpy(exported_cdi_buf, out_args.exported_cdi,
			out_args.exported_cdi_size);
		*exported_cdi_actual_size = out_args.exported_cdi_size;
	}

	return DPE_NO_ERROR;
}
