#ifndef DI_DISPLAYID_H
#define DI_DISPLAYID_H

/**
 * libdisplay-info's low-level API for VESA Display Identification Data
 * (DisplayID).
 *
 * The library implements DisplayID version 1.3, available at:
 * https://vesa.org/vesa-standards/
 */

/**
 * DisplayID data structure.
 */
struct di_displayid;

/**
 * Get the DisplayID version.
 */
int
di_displayid_get_version(const struct di_displayid *displayid);

/**
 * Get the DisplayID revision.
 */
int
di_displayid_get_revision(const struct di_displayid *displayid);

#endif
