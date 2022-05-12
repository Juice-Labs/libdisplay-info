#include <inttypes.h>
#include <stdio.h>

#include <libdisplay-info/edid.h>
#include <libdisplay-info/info.h>

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
main(void)
{
	static uint8_t raw[32 * 1024];
	size_t size = 0;
	const struct di_edid *edid;
	struct di_info *info;
	const struct di_edid_vendor_product *vendor_product;
	const struct di_edid_video_input_digital *video_input_digital;
	const struct di_edid_screen_size *screen_size;
	float gamma;
	const struct di_edid_display_descriptor *const *display_descs;
	const struct di_edid_ext *const *exts;
	size_t i;

	while (!feof(stdin)) {
		size += fread(&raw[size], 1, sizeof(raw) - size, stdin);
		if (ferror(stdin)) {
			perror("fread failed");
			return 1;
		}
	}

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
	printf("    Serial Number: %" PRIu32 "\n", vendor_product->serial);
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

	printf("  Detailed Timing Descriptors:\n");
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
