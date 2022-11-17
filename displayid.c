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
/**
 * The size of a DisplayID type I timing.
 */
#define DISPLAYID_TYPE_I_TIMING_SIZE 20

static void
add_failure(struct di_displayid *displayid, const char fmt[], ...)
{
	va_list args;

	va_start(args, fmt);
	_di_logger_va_add_failure(displayid->logger, fmt, args);
	va_end(args);
}

static void
check_data_block_revision(struct di_displayid *displayid,
			  const uint8_t data[static_array DISPLAYID_DATA_BLOCK_HEADER_SIZE],
			  const char *block_name, uint8_t max_revision)
{
	uint8_t revision, flags;

	flags = get_bit_range(data[0x01], 7, 3);
	revision = get_bit_range(data[0x01], 2, 0);

	if (revision > max_revision) {
		add_failure(displayid, "%s: Unexpected revision (%u != %u).",
			    block_name, revision, max_revision);
	}
	if (flags != 0) {
		add_failure(displayid, "%s: Unexpected flags (0x%02x).",
			    block_name, flags);
	}
}

static bool
parse_display_params_block(struct di_displayid *displayid,
			   struct di_displayid_display_params_priv *priv,
			   const uint8_t *data, size_t size)
{
	struct di_displayid_display_params *params = &priv->base;
	uint8_t raw_features;

	check_data_block_revision(displayid, data,
				  "Display Parameters Data Block",
				  0);

	if (size != 0x0F) {
		add_failure(displayid, "Display Parameters Data Block: DisplayID payload length is different than expected (%zu != %zu)", size, 0x0F);
		return false;
	}

	params->horiz_image_mm = 0.1f * (float)(data[0x03] | (data[0x04] << 8));
	params->vert_image_mm = 0.1f * (float)(data[0x05] | (data[0x06] << 8));

	params->horiz_pixels = data[0x07] | (data[0x08] << 8);
	params->vert_pixels = data[0x09] | (data[0x0A] << 8);

	raw_features = data[0x0B];
	params->features = &priv->features;
	priv->features.audio = has_bit(raw_features, 7);
	priv->features.separate_audio_inputs = has_bit(raw_features, 6);
	priv->features.audio_input_override = has_bit(raw_features, 5);
	priv->features.power_management = has_bit(raw_features, 4);
	priv->features.fixed_timing = has_bit(raw_features, 3);
	priv->features.fixed_pixel_format = has_bit(raw_features, 2);
	priv->features.ai = has_bit(raw_features, 1);
	priv->features.deinterlacing = has_bit(raw_features, 0);

	if (data[0x0C] != 0xFF)
		params->gamma = (float)data[0x0C] / 100 + 1;
	params->aspect_ratio = (float)data[0x0D] / 100 + 1;
	params->bits_per_color_overall = get_bit_range(data[0x0E], 7, 4) + 1;
	params->bits_per_color_native = get_bit_range(data[0x0E], 3, 0) + 1;

	return true;
}

static bool
parse_type_i_timing(struct di_displayid *displayid,
		    struct di_displayid_data_block *data_block,
		    const uint8_t data[static_array DISPLAYID_TYPE_I_TIMING_SIZE])
{
	int raw_pixel_clock;
	uint8_t stereo_3d, aspect_ratio;

	struct di_displayid_type_i_timing *t = calloc(1, sizeof(*t));
	if (t == NULL) {
		return false;
	}

	raw_pixel_clock = data[0] | (data[1] << 8) | (data[2] << 16);
	t->pixel_clock_mhz = (double)(1 + raw_pixel_clock) * 0.01;

	t->preferred = has_bit(data[3], 7);
	t->interlaced = has_bit(data[3], 4);

	stereo_3d = get_bit_range(data[3], 6, 5);
	switch (stereo_3d) {
	case DI_DISPLAYID_TYPE_I_TIMING_STEREO_3D_NEVER:
	case DI_DISPLAYID_TYPE_I_TIMING_STEREO_3D_ALWAYS:
	case DI_DISPLAYID_TYPE_I_TIMING_STEREO_3D_USER:
		t->stereo_3d = stereo_3d;
		break;
	default:
		add_failure(displayid,
			    "Video Timing Modes Type 1 - Detailed Timings Data Block: Reserved stereo 0x%02x.",
			    stereo_3d);
		break;
	}

	aspect_ratio = get_bit_range(data[3], 3, 0);
	switch (aspect_ratio) {
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_1_1:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_5_4:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_4_3:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_15_9:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_16_9:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_16_10:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_64_27:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_256_135:
	case DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_UNDEFINED:
		t->aspect_ratio = aspect_ratio;
		break;
	default:
		t->aspect_ratio = DI_DISPLAYID_TYPE_I_TIMING_ASPECT_RATIO_UNDEFINED;
		add_failure(displayid,
			    "Video Timing Modes Type 1 - Detailed Timings Data Block: Unknown aspect 0x%02x.",
			    aspect_ratio);
		break;
	}

	t->horiz_active = 1 + (data[4] | (data[5] << 8));
	t->horiz_blank = 1 + (data[6] | (data[7] << 8));
	t->horiz_offset = 1 + (data[8] | (get_bit_range(data[9], 6, 0) << 8));
	t->horiz_sync_polarity = has_bit(data[9], 7);
	t->horiz_sync_width = 1 + (data[10] | (data[11] << 8));
	t->vert_active = 1 + (data[12] | (data[13] << 8));
	t->vert_blank = 1 + (data[14] | (data[15] << 8));
	t->vert_offset = 1 + (data[16] | (get_bit_range(data[17], 6, 0) << 8));
	t->vert_sync_polarity = has_bit(data[17], 7);
	t->vert_sync_width = 1 + (data[18] | (data[19] << 8));

	assert(data_block->type_i_timings_len < DISPLAYID_MAX_TYPE_I_TIMINGS);
	data_block->type_i_timings[data_block->type_i_timings_len++] = t;
	return true;
}

static bool
parse_type_i_timing_block(struct di_displayid *displayid,
			  struct di_displayid_data_block *data_block,
			  const uint8_t *data, size_t size)
{
	size_t i;

	check_data_block_revision(displayid, data,
				  "Video Timing Modes Type 1 - Detailed Timings Data Block",
				  1);

	if ((size - DISPLAYID_DATA_BLOCK_HEADER_SIZE) % DISPLAYID_TYPE_I_TIMING_SIZE != 0) {
		add_failure(displayid,
			    "Video Timing Modes Type 1 - Detailed Timings Data Block: payload size not divisible by element size.");
	}

	for (i = DISPLAYID_DATA_BLOCK_HEADER_SIZE;
	     i + DISPLAYID_TYPE_I_TIMING_SIZE <= size;
	     i += DISPLAYID_TYPE_I_TIMING_SIZE) {
		if (!parse_type_i_timing(displayid, data_block, &data[i])) {
			return false;
		}
	}

	return true;
}

static ssize_t
parse_data_block(struct di_displayid *displayid, const uint8_t *data,
		 size_t size)
{
	uint8_t tag;
	size_t data_block_size;
	struct di_displayid_data_block *data_block = NULL;

	assert(size >= DISPLAYID_DATA_BLOCK_HEADER_SIZE);

	tag = data[0x00];
	data_block_size = (size_t) data[0x02] + DISPLAYID_DATA_BLOCK_HEADER_SIZE;
	if (data_block_size > size) {
		add_failure(displayid,
			    "The length of this DisplayID data block (%d) exceeds the number of bytes remaining (%zu)",
			    data_block_size, size);
		goto skip;
	}

	data_block = calloc(1, sizeof(*data_block));
	if (!data_block)
		goto error;

	switch (tag) {
	case DI_DISPLAYID_DATA_BLOCK_DISPLAY_PARAMS:
		if (!parse_display_params_block(displayid,
						&data_block->display_params,
						data, data_block_size))
			goto error;
		break;
	case DI_DISPLAYID_DATA_BLOCK_TYPE_I_TIMING:
		if (!parse_type_i_timing_block(displayid, data_block, data, data_block_size))
			goto error;
		break;
	case DI_DISPLAYID_DATA_BLOCK_PRODUCT_ID:
	case DI_DISPLAYID_DATA_BLOCK_COLOR_CHARACT:
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

	data_block->tag = tag;

	assert(displayid->data_blocks_len < DISPLAYID_MAX_DATA_BLOCKS);
	displayid->data_blocks[displayid->data_blocks_len++] = data_block;
	return (ssize_t) data_block_size;

skip:
	free(data_block);
	return (ssize_t) data_block_size;

error:
	free(data_block);
	return -1;
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

static void
destroy_data_block(struct di_displayid_data_block *data_block)
{
	size_t i;

	switch (data_block->tag) {
	case DI_DISPLAYID_DATA_BLOCK_TYPE_I_TIMING:
		for (i = 0; i < data_block->type_i_timings_len; i++)
			free(data_block->type_i_timings[i]);
		break;
	default:
		break; /* Nothing to do */
	}

	free(data_block);
}

void
_di_displayid_finish(struct di_displayid *displayid)
{
	size_t i;

	for (i = 0; i < displayid->data_blocks_len; i++)
		destroy_data_block(displayid->data_blocks[i]);
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

const struct di_displayid_display_params *
di_displayid_data_block_get_display_params(const struct di_displayid_data_block *data_block)
{
	if (data_block->tag != DI_DISPLAYID_DATA_BLOCK_DISPLAY_PARAMS) {
		return NULL;
	}
	return &data_block->display_params.base;
}

const struct di_displayid_type_i_timing *const *
di_displayid_data_block_get_type_i_timings(const struct di_displayid_data_block *data_block)
{
	if (data_block->tag != DI_DISPLAYID_DATA_BLOCK_TYPE_I_TIMING) {
		return NULL;
	}
	return (const struct di_displayid_type_i_timing *const *) data_block->type_i_timings;
}

const struct di_displayid_data_block *const *
di_displayid_get_data_blocks(const struct di_displayid *displayid)
{
	return (const struct di_displayid_data_block *const *) displayid->data_blocks;
}
