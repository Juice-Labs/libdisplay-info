#ifndef DI_INFO_H
#define DI_INFO_H

/**
 * libdisplay-info's high-level API.
 */

/**
 * Information about a display device.
 *
 * This includes at least one EDID or DisplayID blob.
 *
 * Use di_info_parse_edid() to create a struct di_info from an EDID blob.
 * DisplayID blobs are not yet supported.
 */
struct di_info;

/**
 * Parse an EDID blob.
 *
 * Callers do not need to keep the provided data pointer valid after calling
 * this function. Callers should destroy the returned pointer via
 * di_info_destroy().
 */
struct di_info *
di_info_parse_edid(const void *data, size_t size);

/**
 * Destroy a display device information structure.
 */
void
di_info_destroy(struct di_info *info);

#endif
