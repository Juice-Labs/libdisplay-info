#ifndef CTA_H
#define CTA_H

/**
 * Private header for the low-level CTA API.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libdisplay-info/cta.h>

/**
 * The maximum number of detailed timing definitions included in an EDID CTA
 * block.
 *
 * The CTA extension leaves at most 122 bytes for timings, and each timing takes
 * 18 bytes.
 */
#define EDID_CTA_MAX_DETAILED_TIMING_DEFS 6

struct di_edid_cta {
	int revision;
	struct di_edid_cta_flags flags;

	/* NULL-terminated */
	struct di_cta_data_block *data_blocks[128];
	size_t data_blocks_len;

	/* NULL-terminated */
	struct di_edid_detailed_timing_def_priv *detailed_timing_defs[EDID_CTA_MAX_DETAILED_TIMING_DEFS + 1];
	size_t detailed_timing_defs_len;

	struct di_logger *logger;
};

struct di_cta_hdr_static_metadata_block_priv {
	struct di_cta_hdr_static_metadata_block base;
	struct di_cta_hdr_static_metadata_block_eotfs eotfs;
	struct di_cta_hdr_static_metadata_block_descriptors descriptors;
};

struct di_cta_data_block {
	enum di_cta_data_block_tag tag;

	/* Used for DI_CTA_DATA_BLOCK_COLORIMETRY */
	struct di_cta_colorimetry_block colorimetry;
	/* Used for DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA */
	struct di_cta_hdr_static_metadata_block_priv hdr_static_metadata;
};

bool
_di_edid_cta_parse(struct di_edid_cta *cta, const uint8_t *data, size_t size,
		   struct di_logger *logger);

void
_di_edid_cta_finish(struct di_edid_cta *cta);

#endif
