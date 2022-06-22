#ifndef DI_EDID_H
#define DI_EDID_H

/**
 * libdisplay-info's low-level API for Extended Display Identification
 * Data (EDID).
 *
 * EDID 1.4 is defined in VESA Enhanced Extended Display Identification Data
 * Standard release A revision 2.
 */

#include <stdbool.h>
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
	uint32_t serial; /* zero if unset */

	/* These fields are zero if unset */
	int manufacture_week;
	int manufacture_year;
	int model_year;
};

const struct di_edid_vendor_product *
di_edid_get_vendor_product(const struct di_edid *edid);

/**
 * Digital video input interface standard.
 */
enum di_edid_video_input_digital_interface {
	DI_EDID_VIDEO_INPUT_DIGITAL_UNDEFINED = 0x00,
	DI_EDID_VIDEO_INPUT_DIGITAL_DVI = 0x01,
	DI_EDID_VIDEO_INPUT_DIGITAL_HDMI_A = 0x02,
	DI_EDID_VIDEO_INPUT_DIGITAL_HDMI_B = 0x03,
	DI_EDID_VIDEO_INPUT_DIGITAL_MDDI = 0x04,
	DI_EDID_VIDEO_INPUT_DIGITAL_DISPLAYPORT = 0x05,
};

/**
 * EDID digital video input basic information, defined in section 3.6.1.
 */
struct di_edid_video_input_digital {
	int color_bit_depth; /* zero if undefined */
	enum di_edid_video_input_digital_interface interface;
};

/**
 * Get the digital video input basic information.
 *
 * Returns NULL if this isn't a digital display.
 */
const struct di_edid_video_input_digital *
di_edid_get_video_input_digital(const struct di_edid *edid);

struct di_edid_screen_size {
	/* Physical size in centimeters, zero if unset */
	int width_cm, height_cm;
	/* Aspect ratio rounded to the hundredth decimal place, zero if unset. */
	float landscape_aspect_ratio, portait_aspect_ratio;
};

/**
 * Get the screen size.
 */
const struct di_edid_screen_size *
di_edid_get_screen_size(const struct di_edid *edid);

/**
 * Get the display transfer characteristics from the basic EDID parameters, also
 * known as "gamma".
 *
 * Returns 0 if unset (ie, stored in an extension block).
 */
float
di_edid_get_basic_gamma(const struct di_edid *edid);

/**
 * Supported legacy Display Power Management Signaling (DPMS) states, defined in
 * section 3.6.4.
 *
 * Display Power Management (DPM) compliant displays only support "off".
 */
struct di_edid_dpms {
	bool standby;
	bool suspend;
	bool off;
};

/**
 * Get the set of supported legacy DPMS states.
 */
const struct di_edid_dpms *
di_edid_get_dpms(const struct di_edid *edid);

/**
 * Display color type.
 */
enum di_edid_display_color_type {
	/* Monochrome or grayscale display */
	DI_EDID_DISPLAY_COLOR_MONOCHROME = 0x00,
	/* RGB color display */
	DI_EDID_DISPLAY_COLOR_RGB = 0x01,
	/* Non-RGB color display */
	DI_EDID_DISPLAY_COLOR_NON_RGB = 0x02,
	/* Undefined */
	DI_EDID_DISPLAY_COLOR_UNDEFINED = 0x03,
};

/**
 * Get the display color type.
 *
 * For digital displays using EDID 1.4 and later, DI_EDID_DISPLAY_COLOR_UNDEFINED
 * is always returned.
 */
enum di_edid_display_color_type
di_edid_get_display_color_type(const struct di_edid *edid);

/**
 * Basic color encoding formats, defined in section 3.6.4.
 */
struct di_edid_color_encoding_formats {
	bool rgb444; /* RGB 4:4:4 */
	bool ycrcb444; /* YCrCb 4:4:4 */
	bool ycrcb422; /* YCrCb 4:2:2 */
};

/**
 * Get the set of supported color encoding formats.
 *
 * Returns NULL if the display is analog or if the color encoding formats are
 * not specified.
 */
const struct di_edid_color_encoding_formats *
di_edid_get_color_encoding_formats(const struct di_edid *edid);

/**
 * Miscellaneous basic features, defined in section 3.6.4.
 *
 * Note, the enum values don't match the specification.
 */
struct di_edid_misc_features {
	/**
	 * First detailed timing is the preferred timing.
	 *
	 * Always set for EDID 1.4 and later.
	 */
	bool has_preferred_timing;
	/**
	 * GTF using the default parameters is supported.
	 *
	 * Never set for EDID 1.4 and later.
	 */
	bool default_gtf;
	/**
	 * sRGB standard default color space is primary color space.
	 */
	bool srgb_is_primary;
	/**
	 * Preferred timing mode includes native pixel format and rate.
	 *
	 * Never set for EDID 1.3 and earlier.
	 */
	bool preferred_timing_is_native;
	/**
	 * GTF or CVT generated timings within the display's range limits are
	 * accepted.
	 *
	 * Never set for EDID 1.3 and earlier.
	 */
	bool continuous_freq;
};

/**
 * Get the set of miscellaneous basic features.
 */
const struct di_edid_misc_features *
di_edid_get_misc_features(const struct di_edid *edid);

/**
 * EDID display chromaticity coordinates, defined in section 3.7.
 *
 * The values are accurate to the thousandth place. The red, green and blue
 * values are zero for monochrome displays.
 */
struct di_edid_chromaticity_coords {
	float red_x, red_y;
	float green_x, green_y;
	float blue_x, blue_y;
	float white_x, white_y;
};

/**
 * Get chromaticity coordinates.
 */
const struct di_edid_chromaticity_coords *
di_edid_get_chromaticity_coords(const struct di_edid *edid);

/**
 * Aspect ratio for an EDID standard timing.
 */
enum di_edid_standard_timing_aspect_ratio {
	DI_EDID_STANDARD_TIMING_16_10 = 0, /* 16:10 */
	DI_EDID_STANDARD_TIMING_4_3 = 1, /* 4:3 */
	DI_EDID_STANDARD_TIMING_5_4 = 2, /* 5:4 */
	DI_EDID_STANDARD_TIMING_16_9 = 3, /* 16:9 */
};

/**
 * EDID standard timing, defined in section 3.9.
 */
struct di_edid_standard_timing {
	/* Horizontal addressable pixels */
	int32_t horiz_video;
	/* Aspect ratio */
	enum di_edid_standard_timing_aspect_ratio aspect_ratio;
	/* Field Refresh Rate in Hz */
	int32_t refresh_rate_hz;
};

/**
 * Get the vertical addressable line count of an EDID standard timing.
 */
int32_t
di_edid_standard_timing_get_vert_video(const struct di_edid_standard_timing *t);

/**
 * Get the VESA Display Monitor Timing (DMT) ID, if any.
 *
 * Zero is returned if the standard timing doesn't have a DMT ID.
 *
 * TODO: replace with a function which returns a struct describing the DMT.
 */
uint8_t
di_edid_standard_timing_get_dmt_id(const struct di_edid_standard_timing *t);

/**
 * Get a lits of EDID standard timings.
 *
 * The returned array is NULL-terminated.
 */
const struct di_edid_standard_timing *const *
di_edid_get_standard_timings(const struct di_edid *edid);

/**
 * Stereo viewing support.
 */
enum di_edid_detailed_timing_def_stereo {
	/* Normal Display – No Stereo */
	DI_EDID_DETAILED_TIMING_DEF_STEREO_NONE,
	/* Field sequential stereo, right image when stereo sync signal = 1 */
	DI_EDID_DETAILED_TIMING_DEF_STEREO_FIELD_SEQ_RIGHT,
	/* Field sequential stereo, left image when stereo sync signal = 1 */
	DI_EDID_DETAILED_TIMING_DEF_STEREO_FIELD_SEQ_LEFT,
	/* 2-way interleaved stereo, right image on even lines */
	DI_EDID_DETAILED_TIMING_DEF_STEREO_2_WAY_INTERLEAVED_RIGHT,
	/* 2-way interleaved stereo, left image on even lines */
	DI_EDID_DETAILED_TIMING_DEF_STEREO_2_WAY_INTERLEAVED_LEFT,
	/* 4-way interleaved stereo */
	DI_EDID_DETAILED_TIMING_DEF_STEREO_4_WAY_INTERLEAVED,
	/* Side-by-Side interleaved stereo */
	DI_EDID_DETAILED_TIMING_DEF_STEREO_SIDE_BY_SIDE_INTERLEAVED,
};

/**
 * Signal definitions for EDID detailed timings, defined in notes for table 3.22.
 */
enum di_edid_detailed_timing_def_signal_type {
	/* Analog composite */
	DI_EDID_DETAILED_TIMING_DEF_SIGNAL_ANALOG_COMPOSITE = 0x00,
	/* Bipolar analog composite */
	DI_EDID_DETAILED_TIMING_DEF_SIGNAL_BIPOLAR_ANALOG_COMPOSITE = 0x01,
	/* Digital composite */
	DI_EDID_DETAILED_TIMING_DEF_SIGNAL_DIGITAL_COMPOSITE = 0x02,
	/* Digital separate */
	DI_EDID_DETAILED_TIMING_DEF_SIGNAL_DIGITAL_SEPARATE = 0x03,
};

/**
 * EDID detailed timing definition, defined in section 3.10.2.
 */
struct di_edid_detailed_timing_def {
	/* Pixel clock */
	int32_t pixel_clock_hz;
	/* Horizontal/Vertical Addressable Video in pixels/lines */
	int32_t horiz_video, vert_video;
	/* Horizontal/Vertical Blanking in pixels/lines */
	int32_t horiz_blank, vert_blank;
	/* Horizontal/Vertical Front Porch in pixels/lines */
	int32_t horiz_front_porch, vert_front_porch;
	/* Horizontal/Vertical Sync Pulse Width in pixels/lines */
	int32_t horiz_sync_pulse, vert_sync_pulse;
	/* Horizontal/Vertical Addressable Video Image Size in mm, zero if unset */
	int32_t horiz_image_mm, vert_image_mm;
	/* Horizontal/Vertical Border in pixels/lines */
	int32_t horiz_border, vert_border;
	/* Interlaced signal */
	bool interlaced;
	/* Stereo Viewing Support */
	enum di_edid_detailed_timing_def_stereo stereo;
	/* Signal type */
	enum di_edid_detailed_timing_def_signal_type signal_type;
};

/**
 * Get a list of EDID detailed timing definitions.
 *
 * The returned array is NULL-terminated.
 */
const struct di_edid_detailed_timing_def *const *
di_edid_get_detailed_timing_defs(const struct di_edid *edid);

/**
 * EDID display descriptor.
 */
struct di_edid_display_descriptor;

/**
 * Get a list of EDID display descriptors.
 *
 * The returned array is NULL-terminated.
 */
const struct di_edid_display_descriptor *const *
di_edid_get_display_descriptors(const struct di_edid *edid);

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
 * EDID display range limits type.
 *
 * The values do not match the EDID specification.
 *
 * The CVT entry was introduced in EDID 1.4.
 */
enum di_edid_display_range_limits_type {
	/* Range Limits Only - no additional timing information is provided */
	DI_EDID_DISPLAY_RANGE_LIMITS_BARE,
	/* Default GTF supported */
	DI_EDID_DISPLAY_RANGE_LIMITS_DEFAULT_GTF,
	/* Secondary GTF supported */
	DI_EDID_DISPLAY_RANGE_LIMITS_SECONDARY_GTF,
	/* CVT supported */
	DI_EDID_DISPLAY_RANGE_LIMITS_CVT,
};

/**
 * EDID display range limits, defined in section 3.10.3.3.1.
 */
struct di_edid_display_range_limits {
	/* Vertical rate limits in Hz, from 1 Hz to 510 Hz */
	int32_t min_vert_rate_hz, max_vert_rate_hz;
	/* Horizontal rate limits in Hz, from 1 KHz to 510 KHz, rounded to the
	 * nearest multiple of 1 KHz */
	int32_t min_horiz_rate_hz, max_horiz_rate_hz;

	/* Maximum pixel clock in Hz, zero if unset, rounded to the nearest
	 * multiple of 10 MHz */
	int32_t max_pixel_clock_hz;

	enum di_edid_display_range_limits_type type;
};

/**
 * Get the contents of a display range limits descriptor.
 *
 * Returns NULL if the display descriptor tag isn't
 * DI_EDID_DISPLAY_DESCRIPTOR_RANGE_LIMITS.
 */
const struct di_edid_display_range_limits *
di_edid_display_descriptor_get_range_limits(const struct di_edid_display_descriptor *desc);

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

/* See <libdisplay-info/cta.h> */
struct di_edid_cta;

/**
 * Get a CTA-861 extension block.
 *
 * Returns NULL if the extension block tag is not DI_EDID_EXT_CEA.
 */
const struct di_edid_cta *
di_edid_ext_get_cta(const struct di_edid_ext *ext);

#endif
