#ifndef EDID_H
#define EDID_H

/**
 * Private header for the low-level EDID API.
 */

#include <stdint.h>

#include <libdisplay-info/edid.h>

/**
 * The maximum number of EDID blocks (including the base block), defined in
 * section 2.2.1.
 */
#define EDID_MAX_BLOCK_COUNT 256

struct di_edid {
	struct di_edid_vendor_product vendor_product;
	int version, revision;
	/* NULL-terminated, doesn't include the base block */
	struct di_edid_ext *exts[EDID_MAX_BLOCK_COUNT];
	size_t exts_len;
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
di_edid_parse(const void *data, size_t size);

/**
 * Destroy an EDID data structure.
 */
void
di_edid_destroy(struct di_edid *edid);

#endif
