#ifndef CTA_H
#define CTA_H

/**
 * Private header for the low-level CTA API.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libdisplay-info/cta.h>

struct di_edid_cta {
	int revision;
};

bool
_di_edid_cta_parse(struct di_edid_cta *cta, const uint8_t *data, size_t size);

#endif
