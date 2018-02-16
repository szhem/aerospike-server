/*
 * hlc.h
 *
 * Copyright (C) 2008-2016 Aerospike, Inc.
 *
 * Portions may be licensed to Aerospike, Inc. under one or more contributor
 * license agreements.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

/*
 * Hybrid logical clock as described in
 * http://www.cse.buffalo.edu/tech-reports/2014-04.pdf.
 *
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "citrusleaf/cf_clock.h"

#include "node.h"

/**
 * A hybrid logical clock timestamp.
 *
 * The most significant 48 bits represent the physical component of the hlc and
 * the least significant 16 bits represent the logical component.
 */
typedef uint64_t as_hlc_timestamp;

/**
 * Timestamp for a message receive event.
 */
typedef struct as_hlc_msg_timestamp_s
{
	/**
	 * The sender's HLC timestamp at time when the message was sent.
	 */
	as_hlc_timestamp send_ts;
	/**
	 * Local HLC timestamp at message receipt.
	 */
	as_hlc_timestamp recv_ts;
} as_hlc_msg_timestamp;

/**
 * Result of ordering two hlc timestamps.
 */
typedef enum as_hlc_timestamp_order_e {
	/**
	 * The event with first timestamp happened before.
	 */
	AS_HLC_HAPPENS_BEFORE,
	/**
	 * The event with first timestamp happened after.
	 */
	AS_HLC_HAPPENS_AFTER,
	/**
	 * The order of the timestamps is indeterminated.
	 */
	AS_HLC_ORDER_INDETERMINATE
} as_hlc_timestamp_order;

/*----------------------------------------------------------------------------
 * Public API.
 *----------------------------------------------------------------------------*/
/**
 * Initialize hybrid logical clock.
 */
void as_hlc_init();

/**
 * Return a hlc timestamp representing the hlc time "now".
 */
as_hlc_timestamp as_hlc_timestamp_now();

/**
 * Return the physical component of a hlc timstamp
 * @param hlc_ts the hybrid logical clock timestamp.
 */
cf_clock as_hlc_physical_ts_get(as_hlc_timestamp hlc_ts);

/**
 * Update the HLC on receipt of a remote message. The notion is to adjust this
 * node's hlc to ensure the receive hlc ts > the send hlc ts.
 *
 * @param source for debugging and tracking only.
 * @param send_timestamp the hlc timestamp when this message was sent.
 * @param recv_timestamp (output) the message receive timestamp which will be
 * populated. Can be NULL in which case it will be ignored.
 */
void as_hlc_timestamp_update(cf_node source, as_hlc_timestamp send_ts,
			     as_hlc_msg_timestamp* msg_ts);

/**
 * Return the difference in milliseconds between two hlc timestamps. Note this
 * difference may be greater than or equal to the physical wall call difference,
 * because HLC can have non linear jumps, whenever the clock is adjusted. The
 * difference should be used as an estimate rather than an absolute difference.
 * For e.g. use the difference to check that the time difference is at least
 * some number of milliseconds. However do not use this for interval statistics
 * or to check if the difference in time is at the most some number of
 * milliseconds.
 *
 * @param ts1 the first timestamp.
 * @param ts2 the seconds timestamp.
 * @return ts1 - ts2 in milliseconds.
 */
int64_t as_hlc_timestamp_diff_ms(as_hlc_timestamp ts1, as_hlc_timestamp ts2);

/**
 * Orders a local timestamp and remote message send timestamp.
 *
 * @param local_ts the local timestamp.
 * @param msg_ts message receive timestamp containing the remote send and the
 * local receive timestamp.
 * @return the order between the local and the message timestamp.
 */
as_hlc_timestamp_order as_hlc_send_timestamp_order(
  as_hlc_timestamp local_ts, as_hlc_msg_timestamp* msg_ts);

/**
 * Orders two timestamp generated by the same node / process.
 *
 * @param ts1 the first timestamp.
 * @param ts2 the second timestamp.
 * @return AS_HLC_HAPPENS_BEFORE if ts1 happens before ts2 else
 * AS_HLC_HAPPENS_AFTER if ts1 happens after ts2  else
 * AS_HLC_ORDER_INDETERMINATE.
 */
as_hlc_timestamp_order as_hlc_timestamp_order_get(as_hlc_timestamp ts1,
						  as_hlc_timestamp ts2);

/**
 * Subtract milliseconds worth of time from the timestamp.
 * @param timestamp the input timestamp.
 * @param ms the number of milliseconds to subtract.
 */
as_hlc_timestamp as_hlc_timestamp_subtract_ms(as_hlc_timestamp timestamp,
					       int ms);

/**
 * Dump some debugging information to the logs.
 */
void as_hlc_dump(bool verbose);
