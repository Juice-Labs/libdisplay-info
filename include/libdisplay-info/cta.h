#ifndef DI_CTA_H
#define DI_CTA_H

/**
 * libdisplay-info's low-level API for Consumer Technology Association
 * standards.
 *
 * The library implements CTA-861-H, available at:
 * https://shop.cta.tech/collections/standards/products/a-dtv-profile-for-uncompressed-high-speed-digital-interfaces-cta-861-h
 */

#include <stdbool.h>

/**
 * EDID CTA-861 extension block.
 */
struct di_edid_cta;

/**
 * Get the CTA extension revision (also referred to as "version" by the
 * specification).
 */
int
di_edid_cta_get_revision(const struct di_edid_cta *cta);

/**
 * Miscellaneous EDID CTA flags, defined in section 7.3.3.
 *
 * For CTA revision 1, all of the fields are zero.
 */
struct di_edid_cta_flags {
	/* Sink underscans IT Video Formats by default */
	bool underscan;
	/* Sink supports Basic Audio */
	bool basic_audio;
	/* Sink supports YCbCr 4:4:4 in addition to RGB */
	bool ycc444;
	/* Sink supports YCbCr 4:2:2 in addition to RGB */
	bool ycc422;
	/* Total number of native detailed timing descriptors */
	int native_dtds;
};

/**
 * Get miscellaneous CTA flags.
 */
const struct di_edid_cta_flags *
di_edid_cta_get_flags(const struct di_edid_cta *cta);

#endif
