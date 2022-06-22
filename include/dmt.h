#ifndef DMT_H
#define DMT_H

/**
 * Private header for VESA Display Monitor Timing.
 */

#include <stdint.h>
#include <sys/types.h>

struct di_dmt_timing {
	/* DMT ID */
	uint8_t dmt_id;
	/* EDID standard timing 2-byte code, zero if unset */
	uint16_t edid_std_id;
	/* CVT 3-byte code, zero if unset */
	uint32_t cvt_id;
	/* Addressable pixels */
	int32_t horiz_video, vert_video;
	/* Field Refresh Rate in Hz */
	float refresh_rate_hz;
};

extern const struct di_dmt_timing _di_dmt_timings[];
extern const size_t _di_dmt_timings_len;

#endif
