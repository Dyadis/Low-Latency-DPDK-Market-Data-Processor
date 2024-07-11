#pragma once

#include <rte_ethdev.h>

/* Define constants for DPDK setup
 * These values can be tuned based on your specific hardware and requirements
 */
#define RX_RING_SIZE 4096
#define TX_RING_SIZE 4096
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

// Declare external variables
extern struct rte_mempool* mbuf_pool;
extern volatile bool force_quit;

// Function declarations
int dpdk_init(int argc, char* argv[]);
void dpdk_cleanup();
int port_init(uint16_t port, struct rte_mempool* mbuf_pool);