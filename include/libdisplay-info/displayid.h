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

/**
 * Product type identifier, defined in section 2.3.
 */
enum di_displayid_product_type {
	/* Extension section */
	DI_DISPLAYID_PRODUCT_TYPE_EXTENSION = 0x00,
	/* Test structure or equipment */
	DI_DISPLAYID_PRODUCT_TYPE_TEST = 0x01,
	/* Display panel or other transducer, LCD or PDP module, etc. */
	DI_DISPLAYID_PRODUCT_TYPE_DISPLAY_PANEL = 0x02,
	/* Standalone display device, desktop monitor, TV monitor, etc. */
	DI_DISPLAYID_PRODUCT_TYPE_STANDALONE_DISPLAY = 0x03,
	/* Television receiver */
	DI_DISPLAYID_PRODUCT_TYPE_TV_RECEIVER = 0x04,
	/* Repeater/translator */
	DI_DISPLAYID_PRODUCT_TYPE_REPEATER = 0x05,
	/* Direct drive monitor */
	DI_DISPLAYID_PRODUCT_TYPE_DIRECT_DRIVE = 0x06,
};

/**
 * Get the DisplayID product type.
 */
enum di_displayid_product_type
di_displayid_get_product_type(const struct di_displayid *displayid);

#endif
