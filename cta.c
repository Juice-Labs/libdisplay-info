#include <assert.h>

#include "cta.h"

bool
_di_edid_cta_parse(struct di_edid_cta *cta, const uint8_t *data, size_t size)
{
	assert(size == 128);
	assert(data[0] == 0x02);

	cta->revision = data[1];

	return true;
}

int
di_edid_cta_get_revision(const struct di_edid_cta *cta)
{
	return cta->revision;
}
