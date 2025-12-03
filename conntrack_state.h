/*
  ISC License

  Copyright (c) 2025, Connection Tracking State for nlbwmon

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
  REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
  INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
  OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
  PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef __CONNTRACK_STATE_H__
#define __CONNTRACK_STATE_H__

#include <stdint.h>
#include <netinet/in.h>
#include <libubox/avl.h>

/*
 * Connection state tracker for implementing incremental counting.
 * 
 * This module maintains the last reported counter values for each
 * connection, allowing us to calculate traffic deltas instead of
 * incorrectly accumulating absolute counter values from conntrack.
 */

/* Connection state key */
struct conn_key {
	uint8_t family;
	uint8_t proto;
	uint16_t port;
	union {
		struct in6_addr in6;
		struct in_addr in;
	} addr;
};

/* Connection state tracking */
struct conn_state {
	struct conn_key key;
	uint64_t last_in_pkts;
	uint64_t last_in_bytes;
	uint64_t last_out_pkts;
	uint64_t last_out_bytes;
	struct avl_node node;
};

/* Initialize the connection state tracker */
void conntrack_state_init(void);

/* Clean up the connection state tracker */
void conntrack_state_free(void);

/*
 * Calculate traffic delta for a connection.
 * 
 * This function:
 * 1. Looks up the last reported values for this connection
 * 2. Calculates the delta (current - last)
 * 3. Updates the stored last values
 * 4. Handles counter resets (when current < last)
 * 
 * Parameters:
 *   family: AF_INET or AF_INET6
 *   proto: Protocol number
 *   port: Destination port
 *   addr: Source address
 *   curr_in_pkts, curr_in_bytes: Current incoming counters from conntrack
 *   curr_out_pkts, curr_out_bytes: Current outgoing counters from conntrack
 *   delta_in_pkts, delta_in_bytes: [OUT] Calculated incoming deltas
 *   delta_out_pkts, delta_out_bytes: [OUT] Calculated outgoing deltas
 */
void conntrack_state_update(
	uint8_t family,
	uint8_t proto,
	uint16_t port,
	const void *addr,
	uint64_t curr_in_pkts,
	uint64_t curr_in_bytes,
	uint64_t curr_out_pkts,
	uint64_t curr_out_bytes,
	uint64_t *delta_in_pkts,
	uint64_t *delta_in_bytes,
	uint64_t *delta_out_pkts,
	uint64_t *delta_out_bytes
);

/* Clear all connection states (e.g., on database reset) */
void conntrack_state_clear(void);

#endif /* __CONNTRACK_STATE_H__ */
