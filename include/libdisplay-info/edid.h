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

#endif
