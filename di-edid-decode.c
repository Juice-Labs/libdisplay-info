#include <inttypes.h>
#include <stdio.h>

#include <libdisplay-info/edid.h>
#include <libdisplay-info/info.h>

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

static void
print_ext(const struct di_edid_ext *ext, size_t ext_index)
{
	const char *tag_name = ext_tag_name(di_edid_ext_get_tag(ext));

	printf("\n----------------\n\n");
	printf("Block %zu, %s:\n", ext_index + 1, tag_name);
}

int
main(void)
{
	static char raw[32 * 1024];
	size_t size = 0;
	const struct di_edid *edid;
	struct di_info *info;
	const struct di_edid_vendor_product *vendor_product;
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

	exts = di_edid_get_extensions(edid);
	for (i = 0; exts[i] != NULL; i++) {
		print_ext(exts[i], i);
	}

	di_info_destroy(info);
	return 0;
}
