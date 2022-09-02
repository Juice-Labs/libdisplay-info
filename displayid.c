#include <errno.h>

#include "bits.h"
#include "displayid.h"

/**
 * The size of the mandatory fields in a DisplayID section.
 */
#define DISPLAYID_MIN_SIZE 5
/**
 * The maximum size of a DisplayID section.
 */
#define DISPLAYID_MAX_SIZE 256

bool
_di_displayid_parse(struct di_displayid *displayid, const uint8_t *data,
		    size_t size, struct di_logger *logger)
{
	size_t section_size;

	if (size < 5) {
		errno = EINVAL;
		return false;
	}

	displayid->version = get_bit_range(data[0x00], 7, 4);
	displayid->revision = get_bit_range(data[0x00], 3, 0);
	if (displayid->version == 0 || displayid->version > 1) {
		errno = ENOTSUP;
		return false;
	}

	section_size = (size_t) data[0x01] + DISPLAYID_MIN_SIZE;
	if (section_size > DISPLAYID_MAX_SIZE || section_size > size) {
		errno = EINVAL;
		return false;
	}

	return true;
}

int
di_displayid_get_version(const struct di_displayid *displayid)
{
	return displayid->version;
}

int
di_displayid_get_revision(const struct di_displayid *displayid)
{
	return displayid->revision;
}
