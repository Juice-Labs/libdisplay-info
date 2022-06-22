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

/**
 * Returns the EDID the display device information was constructed with.
 *
 * The returned struct di_edid can be used to query low-level EDID information,
 * see <libdisplay-info/edid.h>. Users should prefer the high-level API if
 * possible.
 *
 * NULL is returned if the struct di_info doesn't contain an EDID. The returned
 * struct di_edid is valid until di_info_destroy().
 */
const struct di_edid *
di_info_get_edid(const struct di_info *info);

/**
 * Get the failure messages for this blob.
 *
 * NULL is returned if the blob conforms to the relevant specifications.
 */
const char *
di_info_get_failure_msg(const struct di_info *info);

/**
 * Get the product name of the display device.
 *
 * The product name typically contains the vendor, model and serial number.
 *
 * The caller is responsible for free'ing the returned string.
 */
char *
di_info_get_product_name(const struct di_info *info);

#endif
