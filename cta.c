#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "bits.h"
#include "cta.h"

/**
 * Number of bytes in the CTA header (tag + revision + DTD offset + flags).
 */
#define CTA_HEADER_SIZE 4

static bool
parse_data_block(struct di_edid_cta *cta, uint8_t raw_tag, const uint8_t *data, size_t size)
{
	enum di_cta_data_block_tag tag;
	uint8_t extended_tag;
	struct di_cta_data_block *data_block;

	switch (raw_tag) {
	case 1:
		tag = DI_CTA_DATA_BLOCK_AUDIO;
		break;
	case 2:
		tag = DI_CTA_DATA_BLOCK_VIDEO;
		break;
	case 3:
		/* Vendor-Specific Data Block */
		errno = ENOTSUP;
		return false;
	case 4:
		tag = DI_CTA_DATA_BLOCK_SPEAKER_ALLOC;
		break;
	case 5:
		tag = DI_CTA_DATA_BLOCK_VESA_DISPLAY_TRANSFER_CHARACTERISTIC;
		break;
	case 7:
		/* Use Extended Tag */
		if (size < 1) {
			errno = EINVAL;
			return false;
		}

		extended_tag = data[0];
		data = &data[1];
		size--;

		switch (extended_tag) {
		case 0:
			tag = DI_CTA_DATA_BLOCK_VIDEO_CAP;
			break;
		case 2:
			tag = DI_CTA_DATA_BLOCK_VESA_DISPLAY_DEVICE;
			break;
		case 5:
			tag = DI_CTA_DATA_BLOCK_COLORIMETRY;
			break;
		case 6:
			tag = DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA;
			break;
		case 7:
			tag = DI_CTA_DATA_BLOCK_HDR_DYNAMIC_METADATA;
			break;
		case 13:
			tag = DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF;
			break;
		case 14:
			tag = DI_CTA_DATA_BLOCK_YCBCR420;
			break;
		case 15:
			tag = DI_CTA_DATA_BLOCK_YCBCR420_CAP_MAP;
			break;
		case 18:
			tag = DI_CTA_DATA_BLOCK_HDMI_AUDIO;
			break;
		case 19:
			tag = DI_CTA_DATA_BLOCK_ROOM_CONFIG;
			break;
		case 20:
			tag = DI_CTA_DATA_BLOCK_SPEAKER_LOCATION;
			break;
		case 32:
			tag = DI_CTA_DATA_BLOCK_INFOFRAME;
			break;
		case 34:
			tag = DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VII;
			break;
		case 35:
			tag = DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VIII;
			break;
		case 42:
			tag = DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_X;
			break;
		case 120:
			tag = DI_CTA_DATA_BLOCK_HDMI_EDID_EXT_OVERRIDE;
			break;
		case 121:
			tag = DI_CTA_DATA_BLOCK_HDMI_SINK_CAP;
			break;
		case 1: /* Vendor-Specific Video Data Block */
		case 17: /* Vendor-Specific Audio Data Block */
			errno = ENOTSUP;
			return false;
		default:
			/* Reserved */
			if (cta->revision <= 3) {
				errno = EINVAL;
			} else {
				errno = ENOTSUP;
			}
			return false;
		}
		break;
	default:
		/* Reserved */
		if (cta->revision <= 3) {
			errno = EINVAL;
		} else {
			errno = ENOTSUP;
		}
		return false;
	}

	data_block = calloc(1, sizeof(*data_block));
	if (!data_block) {
		return false;
	}

	data_block->tag = tag;
	cta->data_blocks[cta->data_blocks_len++] = data_block;
	return true;
}

bool
_di_edid_cta_parse(struct di_edid_cta *cta, const uint8_t *data, size_t size)
{
	uint8_t flags, dtd_start;
	uint8_t data_block_header, data_block_tag, data_block_size;
	size_t i;

	assert(size == 128);
	assert(data[0] == 0x02);

	cta->revision = data[1];
	dtd_start = data[2];

	flags = data[3];
	if (cta->revision >= 2) {
		cta->flags.underscan = has_bit(flags, 7);
		cta->flags.basic_audio = has_bit(flags, 6);
		cta->flags.ycc444 = has_bit(flags, 5);
		cta->flags.ycc422 = has_bit(flags, 4);
		cta->flags.native_dtds = get_bit_range(flags, 3, 0);
	} else if (flags != 0) {
		/* Reserved */
		errno = EINVAL;
		return false;
	}

	if (dtd_start == 0) {
		return true;
	} else if (dtd_start < CTA_HEADER_SIZE || dtd_start >= size) {
		errno = EINVAL;
		return false;
	}

	i = CTA_HEADER_SIZE;
	while (i < dtd_start) {
		data_block_header = data[i];
		data_block_tag = get_bit_range(data_block_header, 7, 5);
		data_block_size = get_bit_range(data_block_header, 4, 0);

		if (i + 1 + data_block_size > dtd_start) {
			_di_edid_cta_finish(cta);
			errno = EINVAL;
			return false;
		}

		if (!parse_data_block(cta, data_block_tag,
				      &data[i + 1], data_block_size)
		    && errno != ENOTSUP) {
			_di_edid_cta_finish(cta);
			return false;
		}

		i += 1 + data_block_size;
	}

	if (i != dtd_start) {
		_di_edid_cta_finish(cta);
		errno = EINVAL;
		return false;
	}

	return true;
}

void
_di_edid_cta_finish(struct di_edid_cta *cta)
{
	size_t i;

	for (i = 0; i < cta->data_blocks_len; i++) {
		free(cta->data_blocks[i]);
	}
}

int
di_edid_cta_get_revision(const struct di_edid_cta *cta)
{
	return cta->revision;
}

const struct di_edid_cta_flags *
di_edid_cta_get_flags(const struct di_edid_cta *cta)
{
	return &cta->flags;
}

const struct di_cta_data_block *const *
di_edid_cta_get_data_blocks(const struct di_edid_cta *cta)
{
	return (const struct di_cta_data_block *const *) cta->data_blocks;
}

enum di_cta_data_block_tag
di_cta_data_block_get_tag(const struct di_cta_data_block *block)
{
	return block->tag;
}
