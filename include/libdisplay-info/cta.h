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

/**
 * CTA data block, defined in section 7.4.
 */
struct di_cta_data_block;

/**
 * Get CTA data blocks.
 *
 * The returned array is NULL-terminated.
 */
const struct di_cta_data_block *const *
di_edid_cta_get_data_blocks(const struct di_edid_cta *cta);

/**
 * CTA data block tag.
 *
 * Note, the enum values don't match the specification.
 */
enum di_cta_data_block_tag {
	/* Audio Data Block */
	DI_CTA_DATA_BLOCK_AUDIO = 1,
	/* Video Data Block */
	DI_CTA_DATA_BLOCK_VIDEO,
	/* Speaker Allocation Data Block */
	DI_CTA_DATA_BLOCK_SPEAKER_ALLOC,
	/* VESA Display Transfer Characteristic Data Block */
	DI_CTA_DATA_BLOCK_VESA_DISPLAY_TRANSFER_CHARACTERISTIC,

	/* Video Capability Data Block */
	DI_CTA_DATA_BLOCK_VIDEO_CAP,
	/* VESA Display Device Data Block */
	DI_CTA_DATA_BLOCK_VESA_DISPLAY_DEVICE,
	/* Colorimetry Data Block */
	DI_CTA_DATA_BLOCK_COLORIMETRY,
	/* HDR Static Metadata Data Block */
	DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA,
	/* HDR Dynamic Metadata Data Block */
	DI_CTA_DATA_BLOCK_HDR_DYNAMIC_METADATA,
	/* Video Format Preference Data Block */
	DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF,
	/* YCbCr 4:2:0 Video Data Block */
	DI_CTA_DATA_BLOCK_YCBCR420,
	/* YCbCr 4:2:0 Capability Map Data Block */
	DI_CTA_DATA_BLOCK_YCBCR420_CAP_MAP,
	/* HDMI Audio Data Block */
	DI_CTA_DATA_BLOCK_HDMI_AUDIO,
	/* Room Configuration Data Block */
	DI_CTA_DATA_BLOCK_ROOM_CONFIG,
	/* Speaker Location Data Block */
	DI_CTA_DATA_BLOCK_SPEAKER_LOCATION,
	/* InfoFrame Data Block */
	DI_CTA_DATA_BLOCK_INFOFRAME,
	/* DisplayID Type VII Video Timing Data Block */
	DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VII,
	/* DisplayID Type VIII Video Timing Data Block */
	DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VIII,
	/* DisplayID Type X Video Timing Data Block */
	DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_X,
	/* HDMI Forum EDID Extension Override Data Block */
	DI_CTA_DATA_BLOCK_HDMI_EDID_EXT_OVERRIDE,
	/* HDMI Forum Sink Capability Data Block */
	DI_CTA_DATA_BLOCK_HDMI_SINK_CAP,
};

/**
 * Get the tag of the CTA data block.
 */
enum di_cta_data_block_tag
di_cta_data_block_get_tag(const struct di_cta_data_block *block);

/**
 * CTA colorimetry data block, defined in section 7.5.5.
 */
struct di_cta_colorimetry_block {
	/* Standard Definition Colorimetry based on IEC 61966-2-4 */
	bool xvycc_601;
	/* High Definition Colorimetry based on IEC 61966-2-4 */
	bool xvycc_709;
	/* Colorimetry based on IEC 61966-2-1/Amendment 1 */
	bool sycc_601;
	/* Colorimetry based on IEC 61966-2-5, Annex A */
	bool opycc_601;
	/* Colorimetry based on IEC 61966-2-5 */
	bool oprgb;
	/* Colorimetry based on Rec. ITU-R BT.2020 Y'cC'bcC'rc */
	bool bt2020_cycc;
	/* Colorimetry based on Rec. ITU-R BT.2020 Y'C'bC'r */
	bool bt2020_ycc;
	/* Colorimetry based on Rec. ITU-R BT.2020 R'G'B' */
	bool bt2020_rgb;
	/* Colorimetry based on SMPTE ST 2113 R'G'B' */
	bool st2113_rgb;
	/* Colorimetry based on Rec. ITU-R BT.2100 ICtCp */
	bool ictcp;
};

/**
 * Get the colorimetry data from a CTA data block.
 *
 * Returns NULL if the data block tag is not DI_CTA_DATA_BLOCK_COLORIMETRY.
 */
const struct di_cta_colorimetry_block *
di_cta_data_block_get_colorimetry(const struct di_cta_data_block *block);

/**
 * Supported Electro-Optical Transfer Functions for a CTA HDR static metadata
 * block.
 */
struct di_cta_hdr_static_metadata_block_eotfs {
	/* Traditional gamma - SDR luminance range */
	bool traditional_sdr;
	/* Traditional gamma - HDR luminance range */
	bool traditional_hdr;
	/* Perceptual Quantization (PQ) based on SMPTE ST 2084 */
	bool pq;
	/* Hybrid Log-Gamma (HLG) based on Rec. ITU-R BT.2100 */
	bool hlg;
};

/**
 * Supported static metadata descriptors for a CTA HDR static metadata block.
 */
struct di_cta_hdr_static_metadata_block_descriptors {
	/* Static Metadata Type 1 */
	bool type1;
};

/**
 * CTA HDR static metadata block, defined in section 7.5.13.
 */
struct di_cta_hdr_static_metadata_block {
	/* Desired content max luminance (cd/m²), zero if unset */
	float desired_content_max_luminance;
	/* Desired content max frame-average luminance (cd/m²), zero if unset */
	float desired_content_max_frame_avg_luminance;
	/* Desired content min luminance (cd/m²), zero if unset */
	float desired_content_min_luminance;
	/* Supported EOFTs */
	const struct di_cta_hdr_static_metadata_block_eotfs *eotfs;
	/* Supported descriptors */
	const struct di_cta_hdr_static_metadata_block_descriptors *descriptors;
};

/**
 * Get the HDR static metadata from a CTA data block.
 *
 * Returns NULL if the data block tag is not
 * DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA.
 */
const struct di_cta_hdr_static_metadata_block *
di_cta_data_block_get_hdr_static_metadata(const struct di_cta_data_block *block);

/**
 * A Short Video Descriptor (SVD).
 */
struct di_cta_svd {
	/* Video Identification Code (VIC) */
	uint8_t vic;
	/* Whether this is a native video format */
	bool native;
};

/**
 * Get an array of short video descriptors from a CTA data block.
 *
 * Returns NULL if the data block tag is not DI_CTA_DATA_BLOCK_VIDEO.
 *
 * The returned array is NULL-terminated.
 */
const struct di_cta_svd *const *
di_cta_data_block_get_svds(const struct di_cta_data_block *block);

/**
 * Get a list of EDID detailed timing definitions.
 *
 * The returned array is NULL-terminated.
 */
const struct di_edid_detailed_timing_def *const *
di_edid_cta_get_detailed_timing_defs(const struct di_edid_cta *cta);

#endif
