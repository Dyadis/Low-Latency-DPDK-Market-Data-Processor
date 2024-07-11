#include <cstdint>
#include <iostream>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include "DPDKSetup.h"

// Global variables
struct rte_mempool* mbuf_pool = nullptr;
volatile bool force_quit = false;

int dpdk_init(int argc, char* argv[]) {
    int ret;

    /* DPDK Environment Abstraction Layer (EAL) arguments
     * These settings configure DPDK's runtime environment
     */
    char* dpdk_args[] = {
            argv[0],
            "--file-prefix", "unique_prefix3",  // Unique prefix to avoid conflicts. Not really needed but good to have
            "--socket-mem", "1024",             // Allocate 1GB of memory
            "--huge-dir", "/mnt/huge",          // Directory for hugepages
            NULL
    };

    /* Initialize the Environment Abstraction Layer (EAL)
     * This sets up DPDK's core functionality
     */
    ret = rte_eal_init(sizeof(dpdk_args) / sizeof(dpdk_args[0]) - 1, dpdk_args);
    if (ret < 0) {
        std::cerr << "Error with EAL initialization" << std::endl;
        return -1;
    }

    /* Create a memory pool for packet buffers
     * This pre-allocates memory for packet handling
     */
    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS,
                                        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    if (mbuf_pool == nullptr) {
        std::cerr << "Cannot create mbuf pool" << std::endl;
        return -1;
    }

    // Initialize port 0
    if (port_init(0, mbuf_pool) != 0) {
        std::cerr << "Cannot init port 0" << std::endl;
        return -1;
    }

    return 0;
}

void dpdk_cleanup() {
    // Clean up the EAL resources
    rte_eal_cleanup();
}

int port_init(uint16_t port, struct rte_mempool* mbuf_pool) {
    std::cout << "Initializing port " << port << "..." << std::endl;

    struct rte_eth_conf port_conf = {};
    const uint16_t rx_rings = 1, tx_rings = 1;
    uint16_t nb_rxd = RX_RING_SIZE;
    uint16_t nb_txd = TX_RING_SIZE;
    int retval;
    uint16_t q;

    /* Check if the port is available
     * This ensures we're not trying to use a non-existent port
     */
    if (port >= rte_eth_dev_count_avail()) {
        std::cerr << "Port " << port << " is not available." << std::endl;
        return -1;
    }

    /* Configure the Ethernet device
     * This sets up the basic parameters for the port
     */
    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    if (retval != 0) {
        std::cerr << "Failed to configure port " << port << "." << std::endl;
        return retval;
    }

    /* Adjust the number of RX and TX descriptors
     * This ensures we have the correct number of descriptors for our setup
     */
    retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
    if (retval != 0) {
        std::cerr << "Failed to adjust RX/TX descriptors for port " << port << "." << std::endl;
        return retval;
    }

    /* Set up RX queues
     * This configures the receive queues for the port
     */
    for (q = 0; q < rx_rings; q++) {
        retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
                                        rte_eth_dev_socket_id(port), nullptr, mbuf_pool);
        if (retval < 0) {
            std::cerr << "Failed to setup RX queue for port " << port << "." << std::endl;
            return retval;
        }
    }

    /* Set up TX queues
     * This configures the transmit queues for the port
     */
    for (q = 0; q < tx_rings; q++) {
        retval = rte_eth_tx_queue_setup(port, q, nb_txd,
                                        rte_eth_dev_socket_id(port), nullptr);
        if (retval < 0) {
            std::cerr << "Failed to setup TX queue for port " << port << "." << std::endl;
            return retval;
        }
    }

    /* Start the Ethernet port
     * This activates the port for packet processing
     */
    retval = rte_eth_dev_start(port);
    if (retval < 0) {
        std::cerr << "Failed to start port " << port << "." << std::endl;
        return retval;
    }

    /* Enable promiscuous mode (i couldn't believe the name at first )
     * This allows the port to receive all packets, regardless of MAC address
     */
    rte_eth_promiscuous_enable(port);

    std::cout << "Port " << port << " initialized successfully." << std::endl;
    return 0;
}