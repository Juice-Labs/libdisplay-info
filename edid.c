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
