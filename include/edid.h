#ifndef EDID_H
#define EDID_H

/**
 * Private header for the low-level EDID API.
 */

#include <stdint.h>

#include <libdisplay-info/edid.h>

struct di_edid {
	int version, revision;
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
