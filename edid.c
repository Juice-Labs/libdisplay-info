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
 * The maximum number of EDID blocks (including the base block), defined in
 * section 2.2.1.
 */
#define EDID_MAX_BLOCK_COUNT 256

/**
 * Fixed EDID header, defined in section 3.1.
 */
static const uint8_t header[] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };

static void
parse_version_revision(const uint8_t data[static EDID_BLOCK_SIZE],
		       int *version, int *revision)
{
	*version = (int) data[18];
	*revision = (int) data[19];
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
	man = (uint16_t) ((data[8] << 8) | data[9]);
	out->manufacturer[0] = ((man >> 10) & 0x1F) + '@';
	out->manufacturer[1] = ((man >> 5) & 0x1F) + '@';
	out->manufacturer[2] = ((man >> 0) & 0x1F) + '@';

	out->product = (uint16_t) (data[10] | (data[11] << 8));
	out->serial = (uint32_t) (data[12] |
				  (data[13] << 8) |
				  (data[14] << 16) |
				  (data[15] << 24));

	if (data[17] >= 0x10) {
		year = data[17] + 1990;
	}

	if (data[16] == 0xFF) {
		/* Special flag for model year */
		out->model_year = year;
	} else {
		out->manufacture_year = year;
		if (data[16] > 0 && data[16] <= 54) {
			out->manufacture_week = data[16];
		}
	}
}

struct di_edid *
di_edid_parse(const void *data, size_t size)
{
	struct di_edid *edid;
	int version, revision;

	if (size < EDID_BLOCK_SIZE ||
	    size > EDID_MAX_BLOCK_COUNT * EDID_BLOCK_SIZE) {
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

	edid = calloc(1, sizeof(*edid));
	if (!edid) {
		return NULL;
	}

	edid->version = version;
	edid->revision = revision;

	parse_vendor_product(data, &edid->vendor_product);

	return edid;
}

void
di_edid_destroy(struct di_edid *edid)
{
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
