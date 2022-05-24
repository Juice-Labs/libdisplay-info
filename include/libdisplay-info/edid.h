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

/**
 * EDID display descriptor tag, defined in section 3.10.3.
 */
enum di_edid_display_descriptor_tag {
	/* Display Product Serial Number */
	DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_SERIAL = 0xFF,
	/* Alphanumeric Data String (ASCII) */
	DI_EDID_DISPLAY_DESCRIPTOR_DATA_STRING = 0xFE,
	/* Display Range Limits */
	DI_EDID_DISPLAY_DESCRIPTOR_RANGE_LIMITS = 0xFD,
	/* Display Product Name */
	DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME = 0xFC,
	/* Color Point Data */
	DI_EDID_DISPLAY_DESCRIPTOR_COLOR_POINT = 0xFB,
	/* Standard Timing Identifications */
	DI_EDID_DISPLAY_DESCRIPTOR_STD_TIMING_IDS = 0xFA,
	/* Display Color Management (DCM) Data */
	DI_EDID_DISPLAY_DESCRIPTOR_DCM_DATA = 0xF9,
	/* CVT 3 Byte Timing Codes */
	DI_EDID_DISPLAY_DESCRIPTOR_CVT_TIMING_CODES = 0xF8,
	/* Established Timings III */
	DI_EDID_DISPLAY_DESCRIPTOR_ESTABLISHED_TIMINGS_III = 0xF7,
	/* Dummy Descriptor */
	DI_EDID_DISPLAY_DESCRIPTOR_DUMMY = 0x10,
};

/**
 * Get a list of EDID display descriptors.
 *
 * The returned array is NULL-terminated.
 */
const struct di_edid_display_descriptor *const *
di_edid_get_display_descriptors(const struct di_edid *edid);

/**
 * EDID display descriptor.
 */
struct di_edid_display_descriptor;

/**
 * Get the tag of an EDID display descriptor.
 */
enum di_edid_display_descriptor_tag
di_edid_display_descriptor_get_tag(const struct di_edid_display_descriptor *desc);

/**
 * Get the contents of a product serial number, a data string, or a product name
 * display descriptor.
 *
 * Returns NULL if the display descriptor tag isn't either
 * DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_SERIAL_NUMBER,
 * DI_EDID_DISPLAY_DESCRIPTOR_DATA_STRING or
 * DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME.
 */
const char *
di_edid_display_descriptor_get_string(const struct di_edid_display_descriptor *desc);

/**
 * Get a list of EDID extensions.
 *
 * The returned array is NULL-terminated.
 */
const struct di_edid_ext *const *
di_edid_get_extensions(const struct di_edid *edid);

/**
 * EDID extension block tags, defined in section 2.2.4.
 */
enum di_edid_ext_tag {
	/* CEA 861 Series Extension */
	DI_EDID_EXT_CEA = 0x02,
	/* Video Timing Block Extension */
	DI_EDID_EXT_VTB = 0x10,
	/* Display Information Extension */
	DI_EDID_EXT_DI = 0x40,
	/* Localized String Extension */
	DI_EDID_EXT_LS = 0x50,
	/* Digital Packet Video Link Extension */
	DI_EDID_EXT_DPVL = 0x60,
	/* Extension Block Map */
	DI_EDID_EXT_BLOCK_MAP = 0xF0,
	/* Extension defined by the display manufacturer */
	DI_EDID_EXT_VENDOR = 0xFF,
};

/**
 * EDID extension block.
 */
struct di_edid_ext;

/**
 * Get the tag of an EDID extension block.
 */
enum di_edid_ext_tag
di_edid_ext_get_tag(const struct di_edid_ext *ext);

#endif
