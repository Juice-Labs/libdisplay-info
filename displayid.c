#include <errno.h>

#include "bits.h"
#include "displayid.h"

bool
_di_displayid_parse(struct di_displayid *displayid, const uint8_t *data,
		    size_t size, struct di_logger *logger)
{
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
