#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "edid.h"

/**
 * The size of an EDID block, defined in section 2.2.
 */
#define EDID_BLOCK_SIZE 128
/**
 * The size of an EDID byte descriptor, defined in section 3.10.
 */
#define EDID_BYTE_DESCRIPTOR_SIZE 18

/**
 * Fixed EDID header, defined in section 3.1.
 */
static const uint8_t header[] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };

/**
 * Check whether a byte has a bit set.
 */
static bool
has_bit(uint8_t val, size_t index)
{
	return val & (1 << index);
}

static void
parse_version_revision(const uint8_t data[static EDID_BLOCK_SIZE],
		       int *version, int *revision)
{
	*version = (int) data[0x12];
	*revision = (int) data[0x13];
}

static size_t
parse_ext_count(const uint8_t data[static EDID_BLOCK_SIZE])
{
	return data[0x7E];
}

static bool
validate_block_checksum(const uint8_t data[static EDID_BLOCK_SIZE])
{
	uint8_t sum = 0;
	size_t i;

	for (i = 0; i < EDID_BLOCK_SIZE; i++) {
		sum += data[i];
	}

	return sum == 0;
}

static void
parse_vendor_product(const uint8_t data[static EDID_BLOCK_SIZE],
		     struct di_edid_vendor_product *out)
{
	uint16_t man;
	int year = 0;

	/* The ASCII 3-letter manufacturer code is encoded in 5-bit codes. */
	man = (uint16_t) ((data[0x08] << 8) | data[0x09]);
	out->manufacturer[0] = ((man >> 10) & 0x1F) + '@';
	out->manufacturer[1] = ((man >> 5) & 0x1F) + '@';
	out->manufacturer[2] = ((man >> 0) & 0x1F) + '@';

	out->product = (uint16_t) (data[0x0A] | (data[0x0B] << 8));
	out->serial = (uint32_t) (data[0x0C] |
				  (data[0x0D] << 8) |
				  (data[0x0E] << 16) |
				  (data[0x0F] << 24));

	if (data[0x11] >= 0x10) {
		year = data[0x11] + 1990;
	}

	if (data[0x10] == 0xFF) {
		/* Special flag for model year */
		out->model_year = year;
	} else {
		out->manufacture_year = year;
		if (data[0x10] > 0 && data[0x10] <= 54) {
			out->manufacture_week = data[0x10];
		}
	}
}

static bool
parse_byte_descriptor(struct di_edid *edid,
		      const uint8_t data[static EDID_BYTE_DESCRIPTOR_SIZE])
{
	struct di_edid_display_descriptor *desc;
	uint8_t tag;
	char *newline;

	if (data[0] || data[1]) {
		if (edid->display_descriptors_len > 0) {
			/* A detailed timing descriptor is not allowed after a
			 * display descriptor per note 3 of table 3.20. */
			errno = EINVAL;
			return false;
		}

		/* TODO: parse detailed timing descriptor */
		return true;
	}

	/* TODO: check we got at least one detailed timing descriptor, per note
	 * 4 of table 3.20. */

	desc = calloc(1, sizeof(*desc));
	if (!desc) {
		return false;
	}

	tag = data[3];
	switch (tag) {
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_SERIAL:
	case DI_EDID_DISPLAY_DESCRIPTOR_DATA_STRING:
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME:
		memcpy(desc->str, &data[5], 13);

		/* A newline (if any) indicates the end of the string. */
		newline = strchr(desc->str, '\n');
		if (newline) {
			newline[0] = '\0';
		}
		break;
	case DI_EDID_DISPLAY_DESCRIPTOR_RANGE_LIMITS:
	case DI_EDID_DISPLAY_DESCRIPTOR_COLOR_POINT:
	case DI_EDID_DISPLAY_DESCRIPTOR_STD_TIMING_IDS:
	case DI_EDID_DISPLAY_DESCRIPTOR_DCM_DATA:
	case DI_EDID_DISPLAY_DESCRIPTOR_CVT_TIMING_CODES:
	case DI_EDID_DISPLAY_DESCRIPTOR_ESTABLISHED_TIMINGS_III:
	case DI_EDID_DISPLAY_DESCRIPTOR_DUMMY:
		break; /* Ignore */
	default:
		free(desc);
		if (tag <= 0x0F) {
			/* Manufacturer-specific */
			errno = ENOTSUP;
		} else {
			/* Reserved */
			if (edid->revision > 4) {
				errno = ENOTSUP;
			} else {
				errno = EINVAL;
			}
		}
		return false;
	}

	desc->tag = tag;
	edid->display_descriptors[edid->display_descriptors_len++] = desc;

	return true;
}

static bool
parse_ext(struct di_edid *edid, const uint8_t data[static EDID_BLOCK_SIZE])
{
	struct di_edid_ext *ext;
	uint8_t tag;

	if (!validate_block_checksum(data)) {
		errno = EINVAL;
		return false;
	}

	tag = data[0x00];
	switch (tag) {
	case DI_EDID_EXT_CEA:
	case DI_EDID_EXT_VTB:
	case DI_EDID_EXT_DI:
	case DI_EDID_EXT_LS:
	case DI_EDID_EXT_DPVL:
	case DI_EDID_EXT_BLOCK_MAP:
	case DI_EDID_EXT_VENDOR:
		/* Supported */
		break;
	default:
		/* Unsupported */
		errno = ENOTSUP;
		return false;
	}

	ext = calloc(1, sizeof(*ext));
	if (!ext) {
		return false;
	}
	ext->tag = tag;
	edid->exts[edid->exts_len++] = ext;

	return true;
}

struct di_edid *
di_edid_parse(const void *data, size_t size)
{
	struct di_edid *edid;
	int version, revision;
	size_t exts_len, i;
	const uint8_t *byte_desc_data, *ext_data;

	if (size < EDID_BLOCK_SIZE ||
	    size > EDID_MAX_BLOCK_COUNT * EDID_BLOCK_SIZE ||
	    size % EDID_BLOCK_SIZE != 0) {
		errno = EINVAL;
		return NULL;
	}

	if (memcmp(data, header, sizeof(header)) != 0) {
		errno = EINVAL;
		return NULL;
	}

	parse_version_revision(data, &version, &revision);
	if (version != 1) {
		/* Only EDID version 1 is supported -- as per section 2.1.7
		 * subsequent versions break the structure */
		errno = ENOTSUP;
		return NULL;
	}

	if (!validate_block_checksum(data)) {
		errno = EINVAL;
		return NULL;
	}

	exts_len = size / EDID_BLOCK_SIZE - 1;
	if (exts_len != parse_ext_count(data)) {
		errno = -EINVAL;
		return NULL;
	}

	edid = calloc(1, sizeof(*edid));
	if (!edid) {
		return NULL;
	}

	edid->version = version;
	edid->revision = revision;

	parse_vendor_product(data, &edid->vendor_product);

	for (i = 0; i < EDID_BYTE_DESCRIPTOR_COUNT; i++) {
		byte_desc_data = (const uint8_t *) data
			       + 0x36 + i * EDID_BYTE_DESCRIPTOR_SIZE;
		if (!parse_byte_descriptor(edid, byte_desc_data)
		    && errno != ENOTSUP) {
			di_edid_destroy(edid);
			return NULL;
		}
	}

	for (i = 0; i < exts_len; i++) {
		ext_data = (const uint8_t *) data + (i + 1) * EDID_BLOCK_SIZE;
		if (!parse_ext(edid, ext_data) && errno != ENOTSUP) {
			di_edid_destroy(edid);
			return NULL;
		}
	}

	return edid;
}

void
di_edid_destroy(struct di_edid *edid)
{
	size_t i;

	for (i = 0; i < edid->display_descriptors_len; i++) {
		free(edid->display_descriptors[i]);
	}

	for (i = 0; edid->exts[i] != NULL; i++) {
		free(edid->exts[i]);
	}

	free(edid);
}

int
di_edid_get_version(const struct di_edid *edid)
{
	return edid->version;
}

int
di_edid_get_revision(const struct di_edid *edid)
{
	return edid->revision;
}

const struct di_edid_vendor_product *
di_edid_get_vendor_product(const struct di_edid *edid)
{
	return &edid->vendor_product;
}

const struct di_edid_display_descriptor *const *
di_edid_get_display_descriptors(const struct di_edid *edid)
{
	return (const struct di_edid_display_descriptor *const *) &edid->display_descriptors;
}

enum di_edid_display_descriptor_tag
di_edid_display_descriptor_get_tag(const struct di_edid_display_descriptor *desc)
{
	return desc->tag;
}

const char *
di_edid_display_descriptor_get_string(const struct di_edid_display_descriptor *desc)
{
	switch (desc->tag) {
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_SERIAL:
	case DI_EDID_DISPLAY_DESCRIPTOR_DATA_STRING:
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME:
		return desc->str;
	default:
		return NULL;
	}
}

const struct di_edid_ext *const *
di_edid_get_extensions(const struct di_edid *edid)
{
	return (const struct di_edid_ext *const *) edid->exts;
}

enum di_edid_ext_tag
di_edid_ext_get_tag(const struct di_edid_ext *ext)
{
	return ext->tag;
}
