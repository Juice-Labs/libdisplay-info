#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libdisplay-info/cta.h>
#include <libdisplay-info/edid.h>
#include <libdisplay-info/info.h>

static const char *
standard_timing_aspect_ratio_name(enum di_edid_standard_timing_aspect_ratio aspect_ratio)
{
	switch (aspect_ratio) {
	case DI_EDID_STANDARD_TIMING_16_10:
		return "16:10";
	case DI_EDID_STANDARD_TIMING_4_3:
		return "4:3";
	case DI_EDID_STANDARD_TIMING_5_4:
		return "5:4";
	case DI_EDID_STANDARD_TIMING_16_9:
		return "16:9";
	}
	abort();
}

static void
print_standard_timing(const struct di_edid_standard_timing *t)
{
	int32_t vert_video;
	uint8_t dmt_id;

	vert_video = di_edid_standard_timing_get_vert_video(t);
	dmt_id = di_edid_standard_timing_get_dmt_id(t);

	/* TODO: GTF and CVT timings */
	printf("    ");
	printf("DMT 0x%02x:", dmt_id);
	printf(" %5dx%-5d", t->horiz_video, vert_video);
	printf(" %10.6f Hz", (float) t->refresh_rate_hz);
	printf(" %s", standard_timing_aspect_ratio_name(t->aspect_ratio));
	printf("\n");
}

static int
gcd(int a, int b)
{
	int tmp;

	while (b) {
		tmp = b;
		b = a % b;
		a = tmp;
	}

	return a;
}

static void
compute_aspect_ratio(int width, int height, int *horiz_ratio, int *vert_ratio)
{
	int d;

	d = gcd(width, height);
	if (d == 0) {
		*horiz_ratio = *vert_ratio = 0;
	} else {
		*horiz_ratio = width / d;
		*vert_ratio = height / d;
	}
}

/**
 * Join a list of strings into a comma-separated string.
 *
 * The list must be NULL-terminated.
 */
static char *
join_str(const char *l[])
{
	char *out = NULL;
	size_t out_size = 0, i;
	FILE *f;

	f = open_memstream(&out, &out_size);
	if (!f) {
		return NULL;
	}

	for (i = 0; l[i] != NULL; i++) {
		if (i > 0) {
			fprintf(f, ", ");
		}
		fprintf(f, "%s", l[i]);
	}

	fclose(f);
	return out;
}

static const char *
detailed_timing_def_stereo_name(enum di_edid_detailed_timing_def_stereo stereo)
{
	switch (stereo) {
	case DI_EDID_DETAILED_TIMING_DEF_STEREO_NONE:
		return "none";
	case DI_EDID_DETAILED_TIMING_DEF_STEREO_FIELD_SEQ_RIGHT:
		return "field sequential L/R";
	case DI_EDID_DETAILED_TIMING_DEF_STEREO_FIELD_SEQ_LEFT:
		return "field sequential R/L";
	case DI_EDID_DETAILED_TIMING_DEF_STEREO_2_WAY_INTERLEAVED_RIGHT:
		return "interleaved right even";
	case DI_EDID_DETAILED_TIMING_DEF_STEREO_2_WAY_INTERLEAVED_LEFT:
		return "interleaved left even";
	case DI_EDID_DETAILED_TIMING_DEF_STEREO_4_WAY_INTERLEAVED:
		return "four way interleaved";
	case DI_EDID_DETAILED_TIMING_DEF_STEREO_SIDE_BY_SIDE_INTERLEAVED:
		return "side by side interleaved";
	}
	abort();
}

static void
print_detailed_timing_def(const struct di_edid_detailed_timing_def *def, size_t n)
{
	int hbl, vbl, horiz_total, vert_total;
	int horiz_back_porch, vert_back_porch;
	int horiz_ratio, vert_ratio;
	double refresh, horiz_freq_hz;
	const char *flags[32] = {0};
	char size_mm[64];
	size_t flags_len = 0;

	hbl = def->horiz_blank - 2 * def->horiz_border;
	vbl = def->vert_blank - 2 * def->vert_border;
	horiz_total = def->horiz_video + hbl;
	vert_total = def->vert_video + vbl;
	refresh = (double) def->pixel_clock_hz / (horiz_total * vert_total);
	horiz_freq_hz = (double) def->pixel_clock_hz / horiz_total;

	compute_aspect_ratio(def->horiz_video, def->vert_video,
			     &horiz_ratio, &vert_ratio);

	if (def->stereo != DI_EDID_DETAILED_TIMING_DEF_STEREO_NONE) {
		flags[flags_len++] = detailed_timing_def_stereo_name(def->stereo);
	}
	if (def->horiz_image_mm != 0 || def->vert_image_mm != 0) {
		snprintf(size_mm, sizeof(size_mm), "%d mm x %d mm",
			 def->horiz_image_mm, def->vert_image_mm);
		flags[flags_len++] = size_mm;
	}
	assert(flags_len < sizeof(flags) / sizeof(flags[0]));

	printf("    DTD %zu:", n);
	printf(" %5dx%-5d", def->horiz_video, def->vert_video);
	if (def->interlaced) {
		printf("i");
	}
	printf(" %10.6f Hz", refresh);
	printf(" %3u:%-3u", horiz_ratio, vert_ratio);
	printf(" %8.3f kHz %13.6f MHz", horiz_freq_hz / 1000,
	       (double) def->pixel_clock_hz / (1000 * 1000));
	if (flags_len > 0) {
		char *flags_str = join_str(flags);
		printf(" (%s)", flags_str);
		free(flags_str);
	}
	printf("\n");

	horiz_back_porch = hbl - def->horiz_sync_pulse - def->horiz_front_porch;
	printf("                 Hfront %4d Hsync %3d Hback %4d",
	       def->horiz_front_porch, def->horiz_sync_pulse, horiz_back_porch);
	if (def->horiz_border != 0) {
		printf(" Hborder %d", def->horiz_border);
	}
	printf("\n");

	vert_back_porch = vbl - def->vert_sync_pulse - def->vert_front_porch;
	printf("                 Vfront %4u Vsync %3u Vback %4d",
	       def->vert_front_porch, def->vert_sync_pulse, vert_back_porch);
	if (def->vert_border != 0) {
		printf(" Vborder %d", def->vert_border);
	}
	printf("\n");
}

static const char *
display_desc_tag_name(enum di_edid_display_descriptor_tag tag)
{
	switch (tag) {
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_SERIAL:
		return "Display Product Serial Number";
	case DI_EDID_DISPLAY_DESCRIPTOR_DATA_STRING:
		return "Alphanumeric Data String";
	case DI_EDID_DISPLAY_DESCRIPTOR_RANGE_LIMITS:
		return "Display Range Limits";
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME:
		return "Display Product Name";
	case DI_EDID_DISPLAY_DESCRIPTOR_COLOR_POINT:
		return "Color Point Data";
	case DI_EDID_DISPLAY_DESCRIPTOR_STD_TIMING_IDS:
		return "Standard Timing Identifications";
	case DI_EDID_DISPLAY_DESCRIPTOR_DCM_DATA:
		return "Display Color Management Data";
	case DI_EDID_DISPLAY_DESCRIPTOR_CVT_TIMING_CODES:
		return "CVT 3 Byte Timing Codes";
	case DI_EDID_DISPLAY_DESCRIPTOR_ESTABLISHED_TIMINGS_III:
		return "Established timings III";
	case DI_EDID_DISPLAY_DESCRIPTOR_DUMMY:
		return "Dummy Descriptor";
	}
	abort();
}

static void
print_display_desc(const struct di_edid_display_descriptor *desc)
{
	enum di_edid_display_descriptor_tag tag;
	const char *tag_name, *str;
	const struct di_edid_display_range_limits *range_limits;

	tag = di_edid_display_descriptor_get_tag(desc);
	tag_name = display_desc_tag_name(tag);

	printf("    %s:", tag_name);

	switch (tag) {
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_SERIAL:
	case DI_EDID_DISPLAY_DESCRIPTOR_DATA_STRING:
	case DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME:
		str = di_edid_display_descriptor_get_string(desc);
		printf(" '%s'", str);
		break;
	case DI_EDID_DISPLAY_DESCRIPTOR_RANGE_LIMITS:
		range_limits = di_edid_display_descriptor_get_range_limits(desc);
		printf("\n      Monitor ranges: %d-%d Hz V, %d-%d kHz H",
		       range_limits->min_vert_rate_hz,
		       range_limits->max_vert_rate_hz,
		       range_limits->min_horiz_rate_hz / 1000,
		       range_limits->max_horiz_rate_hz / 1000);
		if (range_limits->max_pixel_clock_hz != 0) {
			printf(", max dotclock %d MHz",
			       range_limits->max_pixel_clock_hz / (1000 * 1000));
		}
		break;
	default:
		break; /* TODO: print other tags */
	}

	printf("\n");
}

static const char *
ext_tag_name(enum di_edid_ext_tag tag)
{
	switch (tag) {
	case DI_EDID_EXT_CEA:
		return "CTA-861 Extension Block";
	case DI_EDID_EXT_VTB:
		return "Video Timing Extension Block";
	case DI_EDID_EXT_DI:
		return "Display Information Extension Block";
	case DI_EDID_EXT_LS:
		return "Localized String Extension Block";
	case DI_EDID_EXT_DPVL:
		return "Digital Packet Video Link Extension";
	case DI_EDID_EXT_BLOCK_MAP:
		return "Block Map Extension Block";
	case DI_EDID_EXT_VENDOR:
		return "Manufacturer-Specific Extension Block";
	}
	abort();
}

static const char *
digital_interface_name(enum di_edid_video_input_digital_interface interface)
{
	switch (interface) {
	case DI_EDID_VIDEO_INPUT_DIGITAL_UNDEFINED:
		return "Digital interface is not defined";
	case DI_EDID_VIDEO_INPUT_DIGITAL_DVI:
		return "DVI interface";
	case DI_EDID_VIDEO_INPUT_DIGITAL_HDMI_A:
		return "HDMI-a interface";
	case DI_EDID_VIDEO_INPUT_DIGITAL_HDMI_B:
		return "HDMI-b interface";
	case DI_EDID_VIDEO_INPUT_DIGITAL_MDDI:
		return "MDDI interface";
	case DI_EDID_VIDEO_INPUT_DIGITAL_DISPLAYPORT:
		return "DisplayPort interface";
	}
	abort();
}

static const char *
display_color_type_name(enum di_edid_display_color_type type)
{
	switch (type) {
	case DI_EDID_DISPLAY_COLOR_MONOCHROME:
		return "Monochrome or grayscale display";
	case DI_EDID_DISPLAY_COLOR_RGB:
		return "RGB color display";
	case DI_EDID_DISPLAY_COLOR_NON_RGB:
		return "Non-RGB color display";
	case DI_EDID_DISPLAY_COLOR_UNDEFINED:
		return "Undefined display color type";
	}
	abort();
}

static const char *
cta_data_block_tag_name(enum di_cta_data_block_tag tag)
{
	switch (tag) {
	case DI_CTA_DATA_BLOCK_AUDIO:
		return "Audio Data Block";
	case DI_CTA_DATA_BLOCK_VIDEO:
		return "Video Data Block";
	case DI_CTA_DATA_BLOCK_SPEAKER_ALLOC:
		return "Speaker Allocation Data Block";
	case DI_CTA_DATA_BLOCK_VESA_DISPLAY_TRANSFER_CHARACTERISTIC:
		return "VESA Display Transfer Characteristic Data Block";
	case DI_CTA_DATA_BLOCK_VIDEO_CAP:
		return "Video Capability Data Block";
	case DI_CTA_DATA_BLOCK_VESA_DISPLAY_DEVICE:
		return "VESA Display Device Data Block";
	case DI_CTA_DATA_BLOCK_COLORIMETRY:
		return "Colorimetry Data Block";
	case DI_CTA_DATA_BLOCK_HDR_STATIC_METADATA:
		return "HDR Static Metadata Data Block";
	case DI_CTA_DATA_BLOCK_HDR_DYNAMIC_METADATA:
		return "HDR Dynamic Metadata Data Block";
	case DI_CTA_DATA_BLOCK_VIDEO_FORMAT_PREF:
		return "Video Format Preference Data Block";
	case DI_CTA_DATA_BLOCK_YCBCR420:
		return "YCbCr 4:2:0 Video Data Block";
	case DI_CTA_DATA_BLOCK_YCBCR420_CAP_MAP:
		return "YCbCr 4:2:0 Capability Map Data Block";
	case DI_CTA_DATA_BLOCK_HDMI_AUDIO:
		return "HDMI Audio Data Block";
	case DI_CTA_DATA_BLOCK_ROOM_CONFIG:
		return "Room Configuration Data Block";
	case DI_CTA_DATA_BLOCK_SPEAKER_LOCATION:
		return "Speaker Location Data Block";
	case DI_CTA_DATA_BLOCK_INFOFRAME:
		return "InfoFrame Data Block";
	case DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VII:
		return "DisplayID Type VII Video Timing Data Block";
	case DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_VIII:
		return "DisplayID Type VIII Video Timing Data Block";
	case DI_CTA_DATA_BLOCK_DISPLAYID_VIDEO_TIMING_X:
		return "DisplayID Type X Video Timing Data Block";
	case DI_CTA_DATA_BLOCK_HDMI_EDID_EXT_OVERRIDE :
		return "HDMI Forum EDID Extension Override Data Block";
	case DI_CTA_DATA_BLOCK_HDMI_SINK_CAP:
		return "HDMI Forum Sink Capability Data Block";
	}
	return "Unknown CTA-861 Data Block";
}

static void
print_cta(const struct di_edid_cta *cta)
{
	const struct di_edid_cta_flags *cta_flags;
	const struct di_cta_data_block *const *data_blocks;
	const struct di_cta_data_block *data_block;
	enum di_cta_data_block_tag data_block_tag;
	size_t i;

	printf("  Revision: %d\n", di_edid_cta_get_revision(cta));

	cta_flags = di_edid_cta_get_flags(cta);
	if (cta_flags->underscan) {
		printf("  Underscans IT Video Formats by default\n");
	}
	if (cta_flags->basic_audio) {
		printf("  Basic audio support\n");
	}
	if (cta_flags->ycc444) {
		printf("  Supports YCbCr 4:4:4\n");
	}
	if (cta_flags->ycc422) {
		printf("  Supports YCbCr 4:2:2\n");
	}
	printf("  Native detailed modes: %d\n", cta_flags->native_dtds);

	data_blocks = di_edid_cta_get_data_blocks(cta);
	for (i = 0; data_blocks[i] != NULL; i++) {
		data_block = data_blocks[i];

		data_block_tag = di_cta_data_block_get_tag(data_block);
		printf("  %s:\n", cta_data_block_tag_name(data_block_tag));
	}
}

static void
print_ext(const struct di_edid_ext *ext, size_t ext_index)
{
	const char *tag_name;

	tag_name = ext_tag_name(di_edid_ext_get_tag(ext));
	printf("\n----------------\n\n");
	printf("Block %zu, %s:\n", ext_index + 1, tag_name);

	switch (di_edid_ext_get_tag(ext)) {
	case DI_EDID_EXT_CEA:
		print_cta(di_edid_ext_get_cta(ext));
		break;
	default:
		break; /* Ignore */
	}
}

static size_t
edid_checksum_index(size_t block_index)
{
	return 128 * (block_index + 1) - 1;
}

static float
truncate_chromaticity_coord(float coord)
{
	return floorf(coord * 10000) / 10000;
}

int
main(int argc, char *argv[])
{
	FILE *in;
	static uint8_t raw[32 * 1024];
	size_t size = 0;
	const struct di_edid *edid;
	struct di_info *info;
	const struct di_edid_vendor_product *vendor_product;
	const struct di_edid_video_input_digital *video_input_digital;
	const struct di_edid_screen_size *screen_size;
	float gamma;
	const struct di_edid_dpms *dpms;
	enum di_edid_display_color_type display_color_type;
	const struct di_edid_color_encoding_formats *color_encoding_formats;
	const struct di_edid_misc_features *misc_features;
	const struct di_edid_chromaticity_coords *chromaticity_coords;
	const struct di_edid_standard_timing *const *standard_timings;
	const struct di_edid_detailed_timing_def *const *detailed_timing_defs;
	const struct di_edid_display_descriptor *const *display_descs;
	const struct di_edid_ext *const *exts;
	size_t i;

	in = stdin;
	if (argc > 1) {
		in = fopen(argv[1], "r");
		if (!in) {
			perror("failed to open input file");
			return 1;
		}
	}

	while (!feof(in)) {
		size += fread(&raw[size], 1, sizeof(raw) - size, in);
		if (ferror(in)) {
			perror("fread failed");
			return 1;
		} else if (size >= sizeof(raw)) {
			fprintf(stderr, "input too large\n");
			return 1;
		}
	}

	fclose(in);

	info = di_info_parse_edid(raw, size);
	if (!info) {
		perror("di_edid_parse failed");
		return 1;
	}

	edid = di_info_get_edid(info);

	printf("Block 0, Base EDID:\n");
	printf("  EDID Structure Version & Revision: %d.%d\n",
	       di_edid_get_version(edid), di_edid_get_revision(edid));

	vendor_product = di_edid_get_vendor_product(edid);
	printf("  Vendor & Product Identification:\n");
	printf("    Manufacturer: %.3s\n", vendor_product->manufacturer);
	printf("    Model: %" PRIu16 "\n", vendor_product->product);
	if (vendor_product->serial != 0) {
		printf("    Serial Number: %" PRIu32 "\n", vendor_product->serial);
	}
	if (vendor_product->model_year != 0) {
		printf("    Model year: %d\n", vendor_product->model_year);
	} else {
		printf("    Made in: week %d of %d\n",
		       vendor_product->manufacture_week,
		       vendor_product->manufacture_year);
	}

	printf("  Basic Display Parameters & Features:\n");
	video_input_digital = di_edid_get_video_input_digital(edid);
	if (video_input_digital) {
		printf("    Digital display\n");
		if (di_edid_get_revision(edid) >= 4) {
			if (video_input_digital->color_bit_depth == 0) {
				printf("    Color depth is undefined\n");
			} else {
				printf("    Bits per primary color channel: %d\n",
				       video_input_digital->color_bit_depth);
			}
			printf("    %s\n",
			       digital_interface_name(video_input_digital->interface));
		}
	}
	screen_size = di_edid_get_screen_size(edid);
	if (screen_size->width_cm > 0) {
		printf("    Maximum image size: %d cm x %d cm\n",
		       screen_size->width_cm, screen_size->height_cm);
	} else if (screen_size->landscape_aspect_ratio > 0) {
		printf("    Aspect ratio: %.2f (landscape)\n",
		       screen_size->landscape_aspect_ratio);
	} else if (screen_size->portait_aspect_ratio > 0) {
		printf("    Aspect ratio: %.2f (portrait)\n",
		       screen_size->portait_aspect_ratio);
	} else {
		printf("    Image size is variable\n");
	}

	gamma = di_edid_get_basic_gamma(edid);
	if (gamma != 0) {
		printf("    Gamma: %.2f\n", gamma);
	} else {
		printf("    Gamma is defined in an extension block\n");
	}

	dpms = di_edid_get_dpms(edid);
	if (dpms->standby || dpms->suspend || dpms->off) {
		printf("    DPMS levels:");
		if (dpms->standby) {
			printf(" Standby");
		}
		if (dpms->suspend) {
			printf(" Suspend");
		}
		if (dpms->off) {
			printf(" Off");
		}
		printf("\n");
	}

	if (!video_input_digital || di_edid_get_revision(edid) < 4) {
		display_color_type = di_edid_get_display_color_type(edid);
		printf("    %s\n", display_color_type_name(display_color_type));
	}

	color_encoding_formats = di_edid_get_color_encoding_formats(edid);
	if (color_encoding_formats) {
		assert(color_encoding_formats->rgb444);
		printf("    Supported color formats: RGB 4:4:4");
		if (color_encoding_formats->ycrcb444) {
			printf(", YCrCb 4:4:4");
		}
		if (color_encoding_formats->ycrcb422) {
			printf(", YCrCb 4:2:2");
		}
		printf("\n");
	}

	misc_features = di_edid_get_misc_features(edid);
	if (misc_features->srgb_is_primary) {
		printf("    Default (sRGB) color space is primary color space\n");
	}
	if (di_edid_get_revision(edid) >= 4) {
		assert(misc_features->has_preferred_timing);
		if (misc_features->preferred_timing_is_native) {
			printf("    First detailed timing includes the native "
			       "pixel format and preferred refresh rate\n");
		} else {
			printf("    First detailed timing does not include the "
			       "native pixel format and preferred refresh rate\n");
		}
	} else {
		if (misc_features->has_preferred_timing) {
			printf("    First detailed timing is the preferred timing\n");
		}
	}
	if (misc_features->continuous_freq) {
		printf("    Display is continuous frequency\n");
	}
	if (misc_features->default_gtf) {
		printf("    Supports GTF timings within operating range\n");
	}

	/* edid-decode truncates the result, but %f rounds it */
	chromaticity_coords = di_edid_get_chromaticity_coords(edid);
	printf("  Color Characteristics:\n");
	printf("    Red  : %.4f, %.4f\n",
	       truncate_chromaticity_coord(chromaticity_coords->red_x),
	       truncate_chromaticity_coord(chromaticity_coords->red_y));
	printf("    Green: %.4f, %.4f\n",
	       truncate_chromaticity_coord(chromaticity_coords->green_x),
	       truncate_chromaticity_coord(chromaticity_coords->green_y));
	printf("    Blue : %.4f, %.4f\n",
	       truncate_chromaticity_coord(chromaticity_coords->blue_x),
	       truncate_chromaticity_coord(chromaticity_coords->blue_y));
	printf("    White: %.4f, %.4f\n",
	       truncate_chromaticity_coord(chromaticity_coords->white_x),
	       truncate_chromaticity_coord(chromaticity_coords->white_y));

	printf("  Standard Timings:");
	standard_timings = di_edid_get_standard_timings(edid);
	if (standard_timings[0] == NULL) {
		printf(" none");
	}
	printf("\n");
	for (i = 0; standard_timings[i] != NULL; i++) {
		print_standard_timing(standard_timings[i]);
	}

	printf("  Detailed Timing Descriptors:\n");
	detailed_timing_defs = di_edid_get_detailed_timing_defs(edid);
	for (i = 0; detailed_timing_defs[i] != NULL; i++) {
		print_detailed_timing_def(detailed_timing_defs[i], i + 1);
	}
	display_descs = di_edid_get_display_descriptors(edid);
	for (i = 0; display_descs[i] != NULL; i++) {
		print_display_desc(display_descs[i]);
	}

	exts = di_edid_get_extensions(edid);

	for (i = 0; exts[i] != NULL; i++);
	if (i > 0) {
		printf("  Extension blocks: %zu\n", i);
	}
	printf("Checksum: 0x%02hhx\n", raw[edid_checksum_index(0)]);

	for (i = 0; exts[i] != NULL; i++) {
		print_ext(exts[i], i);
		printf("Checksum: 0x%02hhx\n", raw[edid_checksum_index(i + 1)]);
	}

	di_info_destroy(info);
	return 0;
}
