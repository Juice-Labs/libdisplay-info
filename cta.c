#include <assert.h>
#include <errno.h>

#include "bits.h"
#include "cta.h"

bool
_di_edid_cta_parse(struct di_edid_cta *cta, const uint8_t *data, size_t size)
{
	uint8_t flags;

	assert(size == 128);
	assert(data[0] == 0x02);

	cta->revision = data[1];

	flags = data[3];
	if (cta->revision >= 2) {
		cta->flags.underscan = has_bit(flags, 7);
		cta->flags.basic_audio = has_bit(flags, 6);
		cta->flags.ycc444 = has_bit(flags, 5);
		cta->flags.ycc422 = has_bit(flags, 4);
		cta->flags.native_dtds = get_bit_range(flags, 3, 0);
	} else if (flags != 0) {
		/* Reserved */
		errno = EINVAL;
		return false;
	}

	return true;
}

int
di_edid_cta_get_revision(const struct di_edid_cta *cta)
{
	return cta->revision;
}

const struct di_edid_cta_flags *
di_edid_cta_get_flags(const struct di_edid_cta *cta)
{
	return &cta->flags;
}
