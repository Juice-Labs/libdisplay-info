#include <inttypes.h>
#include <stdio.h>

#include <libdisplay-info/edid.h>
#include <libdisplay-info/info.h>

int
main(void)
{
	static char raw[32 * 1024];
	size_t size = 0;
	const struct di_edid *edid;
	struct di_info *info;
	const struct di_edid_vendor_product *vendor_product;

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

	di_info_destroy(info);
	return 0;
}
