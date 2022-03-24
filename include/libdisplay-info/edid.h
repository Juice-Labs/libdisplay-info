#ifndef DI_EDID_H
#define DI_EDID_H

/**
 * libdisplay-info's low-level API for Extended Display Identification
 * Data (EDID).
 *
 * EDID 1.4 is defined in VESA Enhanced Extended Display Identification Data
 * Standard release A revision 2.
 */

#include <stddef.h>
#include <stdint.h>

/**
 * EDID data structure.
 */
struct di_edid;

/**
 * Get the EDID version.
 */
int
di_edid_get_version(const struct di_edid *edid);

/**
 * Get the EDID revision.
 */
int
di_edid_get_revision(const struct di_edid *edid);

/**
 * EDID vendor & product identification.
 */
struct di_edid_vendor_product {
	char manufacturer[3];
	uint16_t product;
	uint32_t serial;

	/* These fields are zero if unset */
	int manufacture_week;
	int manufacture_year;
	int model_year;
};

const struct di_edid_vendor_product *
di_edid_get_vendor_product(const struct di_edid *edid);

#endif
