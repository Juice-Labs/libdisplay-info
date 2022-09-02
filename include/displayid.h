#ifndef DISPLAYID_H
#define DISPLAYID_H

/**
 * Private header for the low-level DisplayID API.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libdisplay-info/displayid.h>

#include "log.h"

struct di_displayid {
	int version, revision;
	enum di_displayid_product_type product_type;

	struct di_logger *logger;
};

bool
_di_displayid_parse(struct di_displayid *displayid, const uint8_t *data,
		    size_t size, struct di_logger *logger);

#endif
