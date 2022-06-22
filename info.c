#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edid.h"
#include "info.h"

struct di_info *
di_info_parse_edid(const void *data, size_t size)
{
	struct di_edid *edid;
	struct di_info *info;

	edid = _di_edid_parse(data, size);
	if (!edid)
		return NULL;

	info = calloc(1, sizeof(*info));
	if (!info) {
		_di_edid_destroy(edid);
		return NULL;
	}

	info->edid = edid;

	return info;
}

void
di_info_destroy(struct di_info *info)
{
	_di_edid_destroy(info->edid);
	free(info);
}

const struct di_edid *
di_info_get_edid(const struct di_info *info)
{
	return info->edid;
}

const char *
di_info_get_failure_msg(const struct di_info *info)
{
	return info->edid->failure_msg;
}

char *
di_info_get_product_name(const struct di_info *info)
{
	char name[64];
	const struct di_edid_vendor_product *edid_vendor_product;

	edid_vendor_product = di_edid_get_vendor_product(info->edid);

	/* TODO: use strings from Detailed Timing Descriptors, if any */
	snprintf(name, sizeof(name), "%.3s 0x%X" PRIu16 " 0x%X" PRIu32,
		 edid_vendor_product->manufacturer,
		 edid_vendor_product->product,
		 edid_vendor_product->serial);

	return strdup(name);
}
