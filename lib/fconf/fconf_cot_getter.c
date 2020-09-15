/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <common/fdt_wrappers.h>
#include <drivers/auth/mbedtls/mbedtls_config.h>
#include <drivers/auth/auth_mod.h>
#include <lib/fconf/fconf.h>
#include <lib/object_pool.h>
#include <libfdt.h>

#include <tools_share/tbbr_oid.h>

/* static structures used during authentication process */
static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
			AUTH_PARAM_SIG, 0);
static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
			AUTH_PARAM_SIG_ALG, 0);
static auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(
			AUTH_PARAM_RAW_DATA, 0);

/* pointers to an array of CoT descriptors */
static const auth_img_desc_t *cot_desc[MAX_NUMBER_IDS];
/* array of CoT descriptors */
static auth_img_desc_t auth_img_descs[MAX_NUMBER_IDS];

/* array of authentication methods structures */
static auth_method_desc_t auth_methods[MAX_NUMBER_IDS * AUTH_METHOD_NUM];
static OBJECT_POOL_ARRAY(auth_methods_pool, auth_methods);

/* array of authentication params structures */
static auth_param_desc_t auth_params[MAX_NUMBER_IDS * COT_MAX_VERIFIED_PARAMS];
static OBJECT_POOL_ARRAY(auth_params_pool, auth_params);

/* array of authentication param type structures */
static auth_param_type_desc_t auth_param_type_descs[MAX_NUMBER_IDS];
static OBJECT_POOL_ARRAY(auth_param_type_descs_pool, auth_param_type_descs);

/*
 * array of OIDs
 * Object IDs are used to search hash, pk, counter values in certificate.
 * As per binding we have below 2 combinations:
 * 1. Certificates are validated using nv-cntr and pk
 * 2. Raw images are authenticated using hash
 * Hence in worst case, there are maximum 2 OIDs per image/certificate
 */
static unsigned char oids[(MAX_NUMBER_IDS * 2)][MAX_OID_NAME_LEN];
static OBJECT_POOL_ARRAY(oid_pool, oids);

/* An array of auth buffer which holds hashes and pk
 * ToDo: Size decided with the current number of images and
 * certificates which are available in CoT. Size of these buffers bound to
 * increase in the future on the addition of images/certificates.
 */
static unsigned char hash_auth_bufs[20][HASH_DER_LEN];
static OBJECT_POOL_ARRAY(hash_auth_buf_pool, hash_auth_bufs);
static unsigned char pk_auth_bufs[12][PK_DER_LEN];
static OBJECT_POOL_ARRAY(pk_auth_buf_pool, pk_auth_bufs);

/*******************************************************************************
 * update_parent_auth_data() - Update authentication data structure
 * @auth_desc[in]:	Pointer to the auth image descriptor
 * @type_desc[in]:	Pointer to authentication parameter
 * @auth_buf_size[in]:	Buffer size to hold pk or hash
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int update_parent_auth_data(const auth_img_desc_t *auth_desc,
				   auth_param_type_desc_t *type_desc,
				   unsigned int auth_buf_size)
{
	unsigned int i;
	auth_param_desc_t *auth_data = &auth_desc->authenticated_data[0];
	unsigned char *auth_buf;

	for (i = 0U; i < COT_MAX_VERIFIED_PARAMS; i++) {
		if (auth_data[i].type_desc == type_desc) {
			return 0;
		}
		if (auth_data[i].type_desc == NULL) {
			break;
		}
	}

	if (auth_buf_size == HASH_DER_LEN) {
		auth_buf = pool_alloc(&hash_auth_buf_pool);
	} else if (auth_buf_size == PK_DER_LEN) {
		auth_buf = pool_alloc(&pk_auth_buf_pool);
	} else {
		return -1;
	}

	if (i < COT_MAX_VERIFIED_PARAMS) {
		auth_data[i].type_desc = type_desc;
		auth_data[i].data.ptr = auth_buf;
		auth_data[i].data.len = auth_buf_size;
	} else {
		ERROR("Out of authentication data array\n");
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * get_auth_param_type_desc() - Get pointer of authentication parameter
 * @img_id[in]:		Image Id
 * @type_desc[out]:	Pointer to authentication parameter
 * @buf_size[out]:	Buffer size which hold hash/pk
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int get_auth_param_type_desc(unsigned int img_id,
				    auth_param_type_desc_t **type_desc,
				    unsigned int *buf_size)
{
	auth_method_desc_t *img_auth_method = NULL;
	img_type_t type = auth_img_descs[img_id].img_type;

	if (type == IMG_CERT) {
		img_auth_method =
		&auth_img_descs[img_id].img_auth_methods[AUTH_METHOD_SIG];
		*type_desc = img_auth_method->param.sig.pk;
		*buf_size = PK_DER_LEN;
	} else if (type == IMG_RAW) {
		img_auth_method =
		&auth_img_descs[img_id].img_auth_methods[AUTH_METHOD_HASH];
		*type_desc = img_auth_method->param.hash.hash;
		*buf_size = HASH_DER_LEN;
	} else {
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * set_auth_method() - Update global auth image descriptors with authentication
 *			method data
 * @auth_method_type[in]:	Type of authentication method
 * @oid[in]:			Object Idetifier for pk/hash search
 * @auth_method[in]:		Pointer to authentication method to set
 ******************************************************************************/
static void set_auth_method(auth_method_type_t auth_method_type, char *oid,
			    auth_method_desc_t *auth_method)
{
	auth_param_type_t auth_param_type = AUTH_PARAM_NONE;
	auth_param_type_desc_t *auth_param_type_desc;

	assert(auth_method != NULL);

	auth_param_type_desc = pool_alloc(&auth_param_type_descs_pool);
	auth_method->type = auth_method_type;

	if (auth_method_type == AUTH_METHOD_SIG) {
		auth_param_type = AUTH_PARAM_PUB_KEY;
		auth_method->param.sig.sig = &sig;
		auth_method->param.sig.alg = &sig_alg;
		auth_method->param.sig.data = &raw_data;
		auth_method->param.sig.pk = auth_param_type_desc;
	} else if (auth_method_type == AUTH_METHOD_HASH) {
		auth_param_type = AUTH_PARAM_HASH;
		auth_method->param.hash.data = &raw_data;
		auth_method->param.hash.hash = auth_param_type_desc;
	} else if (auth_method_type == AUTH_METHOD_NV_CTR) {
		auth_param_type = AUTH_PARAM_NV_CTR;
		auth_method->param.nv_ctr.cert_nv_ctr = auth_param_type_desc;
		auth_method->param.nv_ctr.plat_nv_ctr = auth_param_type_desc;
	}

	auth_param_type_desc->type = auth_param_type;
	auth_param_type_desc->cookie = (void *)oid;
}

/*******************************************************************************
 * get_oid() -	get object identifier from device tree
 * @dtb[in]:	Pointer to the device tree blob in memory
 * @node[in]:	Offset of the node
 * @prop[in]:	Property to read from the given node
 * @oid[out]:	Object Indentifier of key/hash/nv-counter in certificate
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int get_oid(const void *dtb, int node, const char *prop, char **oid)
{
	uint32_t phandle;
	int rc;

	rc = fdt_read_uint32(dtb, node, prop, &phandle);
	if (rc < 0) {
		return rc;
	}

	node = fdt_node_offset_by_phandle(dtb, phandle);
	if (node < 0) {
		return node;
	}

	*oid = pool_alloc(&oid_pool);
	rc = fdtw_read_string(dtb, node, "oid", *oid, MAX_OID_NAME_LEN);

	return rc;
}

/*******************************************************************************
 * populate_and_set_auth_methods() -  Populate auth method parameters from
 *			device tree and set authentication method
 *			structure.
 * @dtb[in]:		Pointer to the device tree blob in memory
 * @node[in]:		Offset of the node
 * @img_id[in]:		Image identifier
 * @type[in]:		Type of image
 * @root_certificate[in]:Root certificate (authenticated by ROTPK)
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int populate_and_set_auth_methods(const void *dtb, int node,
					 unsigned int img_id, img_type_t type,
					 bool root_certificate)
{
	auth_method_type_t auth_method_type = AUTH_METHOD_NONE;
	int rc;
	char *oid = NULL;

	auth_method_desc_t *auth_method = pool_alloc_n(&auth_methods_pool,
					AUTH_METHOD_NUM);

	/*
	 * This is as per binding document where certificates are
	 * verified by signature and images are verified by hash.
	 */
	if (type == IMG_CERT) {
		if (root_certificate) {
			oid = NULL;
		} else {
			rc = get_oid(dtb, node, "signing-key", &oid);
			if (rc < 0) {
				ERROR("FCONF: Can't read %s property\n",
					"signing-key");
				return rc;
			}
		}
		auth_method_type = AUTH_METHOD_SIG;
	} else if (type == IMG_RAW) {
		rc = get_oid(dtb, node, "hash", &oid);
		if (rc < 0) {
			ERROR("FCONF: Can't read %s property\n",
				"hash");
			return rc;
		}
		auth_method_type = AUTH_METHOD_HASH;
	} else {
		return -1;
	}

	set_auth_method(auth_method_type, oid,
			&auth_method[auth_method_type]);

	/* Retrieve the optional property */
	rc = get_oid(dtb, node, "antirollback-counter", &oid);
	if (rc == 0) {
		auth_method_type = AUTH_METHOD_NV_CTR;
		set_auth_method(auth_method_type, oid,
				&auth_method[auth_method_type]);
	}

	auth_img_descs[img_id].img_auth_methods = &auth_method[0];

	return 0;
}

/*******************************************************************************
 * get_parent_img_id() - Get parent image id for given child node
 * @dtb[in]:		Pointer to the device tree blob in memory
 * @node[in]:		Offset of the child node
 * @parent_img_id[out]:	Image id of parent
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int get_parent_img_id(const void *dtb, int node,
			     unsigned int *parent_img_id)
{
	uint32_t phandle;
	int err;

	err = fdt_read_uint32(dtb, node, "parent", &phandle);
	if (err < 0) {
		ERROR("FCONF: Could not read %s property in node\n",
			"parent");
		return err;
	}

	node = fdt_node_offset_by_phandle(dtb, phandle);
	if (node < 0) {
		ERROR("FCONF: Failed to locate node using its phandle\n");
		return node;
	}

	err = fdt_read_uint32(dtb, node, "image-id", parent_img_id);
	if (err < 0) {
		ERROR("FCONF: Could not read %s property in node\n",
			"image-id");
	}

	return err;
}

/*******************************************************************************
 * set_desc_data() - Update data in descriptor's structure
 * @dtb[in]:	Pointer to the device tree blob in memory
 * @node[in]:	Offset of the node
 * @type[in]:	Type of image (RAW/CERT)
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int set_desc_data(const void *dtb, int node, img_type_t type)
{
	int rc;
	bool root_certificate = false;
	unsigned int img_id, parent_img_id;

	rc = fdt_read_uint32(dtb, node, "image-id", &img_id);
	if (rc < 0) {
		ERROR("FCONF: Can't find property %s in node\n",
			"image-id");
		return rc;
	}

	if (fdt_getprop(dtb, node, "root-certificate",
					NULL) != NULL) {
		root_certificate = true;
	}

	if (!root_certificate) {
		rc = get_parent_img_id(dtb, node, &parent_img_id);
		if (rc < 0) {
			return rc;
		}
		auth_img_descs[img_id].parent = &auth_img_descs[parent_img_id];
	}

	auth_img_descs[img_id].img_id = img_id;
	auth_img_descs[img_id].img_type = type;

	rc = populate_and_set_auth_methods(dtb, node, img_id, type,
				root_certificate);
	if (rc < 0) {
		return rc;
	}

	if (type == IMG_CERT) {
		auth_param_desc_t *auth_param =
			pool_alloc_n(&auth_params_pool,
					COT_MAX_VERIFIED_PARAMS);
		auth_img_descs[img_id].authenticated_data = &auth_param[0];
	}

	cot_desc[img_id] = &auth_img_descs[img_id];

	return rc;
}

/*******************************************************************************
 * populate_manifest_descs() - Populate CoT descriptors and update global
 *			       certificate structures
 * @dtb[in]:	Pointer to the device tree blob in memory
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int populate_manifest_descs(const void *dtb)
{
	int node, child;
	int rc;

	/*
	 * Assert the node offset points to "arm, cert-descs"
	 * compatible property
	 */
	const char *compatible_str = "arm, cert-descs";

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in node\n",
			compatible_str);
		return node;
	}

	fdt_for_each_subnode(child, dtb, node) {
		rc = set_desc_data(dtb, child, IMG_CERT);
		if (rc < 0) {
			return rc;
		}
	}

	return 0;
}

/*******************************************************************************
 * populate_image_descs() - Populate CoT descriptors and update global
 *			    image descriptor structures.
 * @dtb[in]:	Pointer to the device tree blob in memory
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int populate_image_descs(const void *dtb)
{
	int node, child;
	int rc;

	/*
	 * Assert the node offset points to "arm, img-descs"
	 * compatible property
	 */
	const char *compatible_str = "arm, img-descs";

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in node\n",
			compatible_str);
		return node;
	}

	fdt_for_each_subnode(child, dtb, node) {
		rc = set_desc_data(dtb, child, IMG_RAW);
		if (rc < 0) {
			return rc;
		}
	}

	return 0;
}

/*******************************************************************************
 * fconf_populate_cot_descs() - Populate CoT descriptors and update global
 *				structures
 * @config[in]:	Pointer to the device tree blob in memory
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int fconf_populate_cot_descs(uintptr_t config)
{
	auth_param_type_desc_t *type_desc = NULL;
	unsigned int auth_buf_size = 0U;
	int rc;

	/* As libfdt uses void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/* populate manifest descs information */
	rc = populate_manifest_descs(dtb);
	if (rc < 0) {
		ERROR("FCONF: population of %s descs failed %d\n",
			"manifest", rc);
		return rc;
	}

	/* populate image descs information */
	rc = populate_image_descs(dtb);
	if (rc < 0) {
		ERROR("FCONF: population of %s descs failed %d\n",
			"images", rc);
		return rc;
	}

	/* update parent's authentication data */
	for (unsigned int i = 0U; i < MAX_NUMBER_IDS; i++) {
		if (auth_img_descs[i].parent != NULL) {
			rc = get_auth_param_type_desc(i,
						&type_desc,
						&auth_buf_size);
			if (rc < 0) {
				ERROR("FCONF: failed to get auth data %d\n",
					rc);
				return rc;
			}

			rc = update_parent_auth_data(auth_img_descs[i].parent,
						type_desc,
						auth_buf_size);
			if (rc < 0) {
				ERROR("FCONF: auth data update failed %d\n",
					rc);
				return rc;
			}
		}
	}

	return rc;
}

FCONF_REGISTER_POPULATOR(TB_FW, cot_desc, fconf_populate_cot_descs);
REGISTER_COT(cot_desc);
