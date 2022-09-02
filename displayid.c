#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

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
/**
 * The size of a DisplayID data block header (tag, revision and size).
 */
#define DISPLAYID_DATA_BLOCK_HEADER_SIZE 3

static void
add_failure(struct di_displayid *displayid, const char fmt[], ...)
{
	va_list args;

	va_start(args, fmt);
	_di_logger_va_add_failure(displayid->logger, fmt, args);
	va_end(args);
}

static ssize_t
parse_data_block(struct di_displayid *displayid, const uint8_t *data,
		 size_t size)
{
	uint8_t tag;
	size_t data_block_size;
	struct di_displayid_data_block *data_block;

	assert(size >= DISPLAYID_DATA_BLOCK_HEADER_SIZE);

	tag = data[0x00];
	data_block_size = (size_t) data[0x02] + DISPLAYID_DATA_BLOCK_HEADER_SIZE;
	if (data_block_size > size) {
		add_failure(displayid,
			    "The length of this DisplayID data block (%d) exceeds the number of bytes remaining (%zu)",
			    data_block_size, size);
		goto skip;
	}

	switch (tag) {
	case DI_DISPLAYID_DATA_BLOCK_PRODUCT_ID:
	case DI_DISPLAYID_DATA_BLOCK_DISPLAY_PARAMS:
	case DI_DISPLAYID_DATA_BLOCK_COLOR_CHARACT:
	case DI_DISPLAYID_DATA_BLOCK_TYPE_I_TIMING:
	case DI_DISPLAYID_DATA_BLOCK_TYPE_II_TIMING:
	case DI_DISPLAYID_DATA_BLOCK_TYPE_III_TIMING:
	case DI_DISPLAYID_DATA_BLOCK_TYPE_IV_TIMING:
	case DI_DISPLAYID_DATA_BLOCK_VESA_TIMING:
	case DI_DISPLAYID_DATA_BLOCK_CEA_TIMING:
	case DI_DISPLAYID_DATA_BLOCK_TIMING_RANGE_LIMITS:
	case DI_DISPLAYID_DATA_BLOCK_PRODUCT_SERIAL:
	case DI_DISPLAYID_DATA_BLOCK_ASCII_STRING:
	case DI_DISPLAYID_DATA_BLOCK_DISPLAY_DEVICE_DATA:
	case DI_DISPLAYID_DATA_BLOCK_INTERFACE_POWER_SEQ:
	case DI_DISPLAYID_DATA_BLOCK_TRANSFER_CHARACT:
	case DI_DISPLAYID_DATA_BLOCK_DISPLAY_INTERFACE:
	case DI_DISPLAYID_DATA_BLOCK_STEREO_DISPLAY_INTERFACE:
	case DI_DISPLAYID_DATA_BLOCK_TYPE_V_TIMING:
	case DI_DISPLAYID_DATA_BLOCK_TILED_DISPLAY_TOPO:
	case DI_DISPLAYID_DATA_BLOCK_TYPE_VI_TIMING:
		break; /* Supported */
	case 0x7F:
		goto skip; /* Vendor-specific */
	default:
		add_failure(displayid,
			    "Unknown DisplayID Data Block (0x%" PRIx8 ", length %" PRIu8 ")",
			    tag, data_block_size - DISPLAYID_DATA_BLOCK_HEADER_SIZE);
		goto skip;
	}

	data_block = calloc(1, sizeof(*data_block));
	if (!data_block)
		return -1;

	data_block->tag = tag;

	assert(displayid->data_blocks_len < DISPLAYID_MAX_DATA_BLOCKS);
	displayid->data_blocks[displayid->data_blocks_len++] = data_block;

skip:
	return (ssize_t) data_block_size;
}

static bool
is_all_zeroes(const uint8_t *data, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++) {
		if (data[i] != 0)
			return false;
	}

	return true;
}

static bool
is_data_block_end(const uint8_t *data, size_t size)
{
	if (size < DISPLAYID_DATA_BLOCK_HEADER_SIZE)
		return true;
	return is_all_zeroes(data, DISPLAYID_DATA_BLOCK_HEADER_SIZE);
}

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
	size_t section_size, i, max_data_block_size;
	ssize_t data_block_size;
	uint8_t product_type;

	if (size < DISPLAYID_MIN_SIZE) {
		errno = EINVAL;
		return false;
	}

	displayid->logger = logger;

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

	i = DISPLAYID_MIN_SIZE - 1;
	max_data_block_size = 0;
	while (i < section_size - 1) {
		max_data_block_size = section_size - 1 - i;
		if (is_data_block_end(&data[i], max_data_block_size))
			break;
		data_block_size = parse_data_block(displayid, &data[i],
						   max_data_block_size);
		if (data_block_size < 0)
			return false;
		assert(data_block_size > 0);
		i += (size_t) data_block_size;
	}
	if (!is_all_zeroes(&data[i], max_data_block_size)) {
		if (max_data_block_size < DISPLAYID_DATA_BLOCK_HEADER_SIZE)
			add_failure(displayid,
				    "Not enough bytes remain (%zu) for a DisplayID data block and the DisplayID filler is non-0.",
				    max_data_block_size);
		else
			add_failure(displayid, "Padding: Contains non-zero bytes.");
	}

	displayid->logger = NULL;
	return true;
}

void
_di_displayid_finish(struct di_displayid *displayid)
{
	size_t i;

	for (i = 0; i < displayid->data_blocks_len; i++)
		free(displayid->data_blocks[i]);
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

enum di_displayid_data_block_tag
di_displayid_data_block_get_tag(const struct di_displayid_data_block *data_block)
{
	return data_block->tag;
}

const struct di_displayid_data_block *const *
di_displayid_get_data_blocks(const struct di_displayid *displayid)
{
	return (const struct di_displayid_data_block *const *) displayid->data_blocks;
}
