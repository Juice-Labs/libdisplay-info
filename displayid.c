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

static bool
validate_checksum(const uint8_t *data, size_t size)
{
	uint8_t sum = 0;
	size_t i;

	for (i = 0; i < size; i++) {
		sum += data[i];
	}

	return sum == 0;
}

bool
_di_displayid_parse(struct di_displayid *displayid, const uint8_t *data,
		    size_t size, struct di_logger *logger)
{
	size_t section_size;
	uint8_t product_type;

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

	if (!validate_checksum(data, section_size)) {
		errno = EINVAL;
		return false;
	}

	product_type = data[0x02];
	switch (product_type) {
	case DI_DISPLAYID_PRODUCT_TYPE_EXTENSION:
	case DI_DISPLAYID_PRODUCT_TYPE_TEST:
	case DI_DISPLAYID_PRODUCT_TYPE_DISPLAY_PANEL:
	case DI_DISPLAYID_PRODUCT_TYPE_STANDALONE_DISPLAY:
	case DI_DISPLAYID_PRODUCT_TYPE_TV_RECEIVER:
	case DI_DISPLAYID_PRODUCT_TYPE_REPEATER:
	case DI_DISPLAYID_PRODUCT_TYPE_DIRECT_DRIVE:
		displayid->product_type = product_type;
		break;
	default:
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

enum di_displayid_product_type
di_displayid_get_product_type(const struct di_displayid *displayid)
{
	return displayid->product_type;
}
