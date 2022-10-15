#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edid.h"
#include "info.h"
#include "log.h"

struct di_info *
di_info_parse_edid(const void *data, size_t size)
{
	struct di_edid *edid;
	struct di_info *info;
	char *failure_msg = NULL;
	size_t failure_msg_size;
	FILE *failure_msg_file;

	failure_msg_file = open_memstream(&failure_msg,
					  &failure_msg_size);
	if (!failure_msg_file)
		return NULL;

	edid = _di_edid_parse(data, size, failure_msg_file);
	if (!edid)
		goto err_failure_msg_file;

	info = calloc(1, sizeof(*info));
	if (!info)
		goto err_edid;

	info->edid = edid;

	if (fflush(failure_msg_file) != 0)
		goto err_info;
	fclose(failure_msg_file);
	if (failure_msg && failure_msg[0] != '\0')
		info->failure_msg = failure_msg;
	else
		free(failure_msg);

	return info;

err_info:
	free(info);
err_edid:
	_di_edid_destroy(edid);
err_failure_msg_file:
	fclose(failure_msg_file);
	free(failure_msg);
	return NULL;
}

void
di_info_destroy(struct di_info *info)
{
	_di_edid_destroy(info->edid);
	free(info->failure_msg);
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
	return info->failure_msg;
}
