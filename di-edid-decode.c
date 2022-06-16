#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include <libdisplay-info/edid.h>
#include <libdisplay-info/info.h>

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

static void
print_detailed_timing_def(const struct di_edid_detailed_timing_def *def, size_t n)
{
	int hbl, vbl, horiz_total, vert_total;
	int horiz_back_porch, vert_back_porch;
	int horiz_ratio, vert_ratio;
	double refresh, horiz_freq_hz;

	hbl = def->horiz_blank - 2 * def->horiz_border;
	vbl = def->vert_blank - 2 * def->vert_border;
	horiz_total = def->horiz_video + hbl;
	vert_total = def->vert_video + vbl;
	refresh = (double) def->pixel_clock_hz / (horiz_total * vert_total);
	horiz_freq_hz = (double) def->pixel_clock_hz / horiz_total;

	compute_aspect_ratio(def->horiz_video, def->vert_video,
			     &horiz_ratio, &vert_ratio);

	printf("    DTD %zu:", n);
	printf(" %5dx%-5d", def->horiz_video, def->vert_video);
	printf(" %10.6f Hz", refresh);
	printf(" %3u:%-3u", horiz_ratio, vert_ratio);
	printf(" %8.3f kHz %13.6f MHz", horiz_freq_hz / 1000,
	       (double) def->pixel_clock_hz / (1000 * 1000));
	printf(" (%d mm x %d mm)", def->horiz_image_mm, def->vert_image_mm);
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
	static char name[256];
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
	default:
		snprintf(name, sizeof(name), "%s Display Descriptor (0x%02hhx)",
			 tag <= 0x0F ? "Manufacturer-Specified" : "Unknown",
			 tag);
		return name;
	}
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
	static char name[256];
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
	default:
		snprintf(name, sizeof(name),
			 "Unknown EDID Extension Block 0x%02x", tag);
		return name;
	}
}

static const char *
digital_interface_name(enum di_edid_video_input_digital_interface interface)
{
	static char name[256];
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
	default:
		snprintf(name, sizeof(name), "Unknown interface: 0x%02x",
			 interface);
		return name;
	}
}

static void
print_ext(const struct di_edid_ext *ext, size_t ext_index)
{
	const char *tag_name = ext_tag_name(di_edid_ext_get_tag(ext));

	printf("\n----------------\n\n");
	printf("Block %zu, %s:\n", ext_index + 1, tag_name);
}

static size_t
edid_checksum_index(size_t block_index)
{
	return 128 * (block_index + 1) - 1;
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
	const struct di_edid_color_encoding_formats *color_encoding_formats;
	const struct di_edid_misc_features *misc_features;
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
