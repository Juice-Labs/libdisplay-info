#include <stdlib.h>

#include "edid.h"
#include "info.h"

struct di_info *
di_info_parse_edid(const void *data, size_t size)
{
	struct di_edid *edid;
	struct di_info *info;

	edid = di_edid_parse(data, size);
	if (!edid)
		return NULL;

	info = calloc(1, sizeof(*info));
	if (!info) {
		di_edid_destroy(edid);
		return NULL;
	}

	info->edid = edid;

	return info;
}

void
di_info_destroy(struct di_info *info)
{
	di_edid_destroy(info->edid);
	free(info);
}
