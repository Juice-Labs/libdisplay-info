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

/**
 * The maximum number of data blocks in a DisplayID section.
 *
 * A DisplayID section has a payload size of 251 bytes, and each data block has
 * a minimum size of 3 bytes.
 */
#define DISPLAYID_MAX_DATA_BLOCKS 83

struct di_displayid {
	int version, revision;
	enum di_displayid_product_type product_type;

	struct di_displayid_data_block *data_blocks[DISPLAYID_MAX_DATA_BLOCKS + 1];
	size_t data_blocks_len;

	struct di_logger *logger;
};

struct di_displayid_data_block {
	enum di_displayid_data_block_tag tag;
};

bool
_di_displayid_parse(struct di_displayid *displayid, const uint8_t *data,
		    size_t size, struct di_logger *logger);

void
_di_displayid_finish(struct di_displayid *displayid);

#endif
