#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

struct memory_stream {
	FILE *fp;
	char *str;
	size_t str_len;
};

static bool
memory_stream_open(struct memory_stream *m)
{
	*m = (struct memory_stream){ 0 };
	m->fp = open_memstream(&m->str, &m->str_len);

	return m->fp != NULL;
}

static char *
memory_stream_close(struct memory_stream *m)
{
	char *str;
	int ret;

	ret = fclose(m->fp);
	str = m->str;
	*m = (struct memory_stream){ 0 };

	if (ret != 0) {
		free(str);
		str = NULL;
	}

	return str;
}

static void
encode_ascii_byte(FILE *out, char ch)
{
	uint8_t c = (uint8_t)ch;

	/*
	 * Replace ASCII control codes and non-7-bit codes
	 * with an escape string. The result is guaranteed to be valid
	 * UTF-8.
	 */
	if (c < 0x20 || c >= 0x7f)
		fprintf(out, "\\x%02x", c);
	else
		fputc(c, out);
}

static void
encode_ascii_string(FILE *out, const char *str)
{
	size_t len = strlen(str);
	size_t i;

	for (i = 0; i < len; i++)
		encode_ascii_byte(out, str[i]);
}

char *
di_info_get_make(const struct di_info *info)
{
	const struct di_edid_vendor_product *evp;
	char pnp_id[(sizeof(evp->manufacturer)) + 1] = { 0, };
	struct memory_stream m;

	if (!info->edid)
		return NULL;

	if (!memory_stream_open(&m))
		return NULL;

	evp = di_edid_get_vendor_product(info->edid);
	memcpy(pnp_id, evp->manufacturer, sizeof(evp->manufacturer));

	/* TODO: use pnp.ids to convert to a company name */

	fputs("PNP(", m.fp);
	encode_ascii_string(m.fp, pnp_id);
	fputs(")", m.fp);

	return memory_stream_close(&m);
}

char *
di_info_get_model(const struct di_info *info)
{
	const struct di_edid_vendor_product *evp;
	const struct di_edid_display_descriptor *const *desc;
	struct memory_stream m;
	size_t i;

	if (!info->edid)
		return NULL;

	if (!memory_stream_open(&m))
		return NULL;

	desc = di_edid_get_display_descriptors(info->edid);
	for (i = 0; desc[i]; i++) {
		if (di_edid_display_descriptor_get_tag(desc[i]) == DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME) {
			encode_ascii_string(m.fp, di_edid_display_descriptor_get_string(desc[i]));
			return memory_stream_close(&m);
		}
	}

	evp = di_edid_get_vendor_product(info->edid);
	fprintf(m.fp, "0x%04" PRIX16, evp->product);

	return memory_stream_close(&m);
}

char *
di_info_get_serial(const struct di_info *info)
{
	const struct di_edid_display_descriptor *const *desc;
	const struct di_edid_vendor_product *evp;
	struct memory_stream m;
	size_t i;

	if (!info->edid)
		return NULL;

	if (!memory_stream_open(&m))
		return NULL;

	desc = di_edid_get_display_descriptors(info->edid);
	for (i = 0; desc[i]; i++) {
		if (di_edid_display_descriptor_get_tag(desc[i]) == DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_SERIAL) {
			encode_ascii_string(m.fp, di_edid_display_descriptor_get_string(desc[i]));
			return memory_stream_close(&m);
		}
	}

	evp = di_edid_get_vendor_product(info->edid);
	if (evp->serial != 0) {
		fprintf(m.fp, "0x%08" PRIX32, evp->serial);
		return memory_stream_close(&m);
	}

	free(memory_stream_close(&m));
	return NULL;
}
