#ifndef EDID_H
#define EDID_H

/**
 * Private header for the low-level EDID API.
 */

#include <stdbool.h>
#include <stdint.h>

#include <libdisplay-info/edid.h>

/**
 * The maximum number of EDID blocks (including the base block), defined in
 * section 2.2.1.
 */
#define EDID_MAX_BLOCK_COUNT 256
/**
 * The number of EDID byte descriptors, defined in section 3.10.
 */
#define EDID_BYTE_DESCRIPTOR_COUNT 4

struct di_edid {
	struct di_edid_vendor_product vendor_product;
	int version, revision;

	bool is_digital;
	struct di_edid_video_input_digital video_input_digital;
	struct di_edid_screen_size screen_size;
	float gamma;
	struct di_edid_dpms dpms;
	struct di_edid_color_encoding_formats color_encoding_formats;
	struct di_edid_misc_features misc_features;

	/* NULL-terminated */
	struct di_edid_detailed_timing_def *detailed_timing_defs[EDID_BYTE_DESCRIPTOR_COUNT + 1];
	size_t detailed_timing_defs_len;

	/* NULL-terminated */
	struct di_edid_display_descriptor *display_descriptors[EDID_BYTE_DESCRIPTOR_COUNT + 1];
	size_t display_descriptors_len;

	/* NULL-terminated, doesn't include the base block */
	struct di_edid_ext *exts[EDID_MAX_BLOCK_COUNT];
	size_t exts_len;
};

struct di_edid_display_descriptor {
	enum di_edid_display_descriptor_tag tag;
	/* Used for PRODUCT_SERIAL, DATA_STRING and PRODUCT_NAME,
	 * zero-terminated */
	char str[14];
	/* Used for RANGE_LIMITS */
	struct di_edid_display_range_limits range_limits;
};

struct di_edid_ext {
	enum di_edid_ext_tag tag;
};

/**
 * Create an EDID data structure.
 *
 * Callers do not need to keep the provided data pointer valid after calling
 * this function. Callers should destroy the returned pointer via
 * di_edid_destroy().
 */
struct di_edid *
_di_edid_parse(const void *data, size_t size);

/**
 * Destroy an EDID data structure.
 */
void
_di_edid_destroy(struct di_edid *edid);

#endif
