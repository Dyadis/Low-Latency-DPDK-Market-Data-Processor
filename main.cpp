#include <iostream>
#include <signal.h>
#include "DPDKSetup.h"
#include "MarketDataHandler.h"

#define RX_CORE 1
#define WORKER_CORE 2

/* Signal handler for graceful shutdown
 * This function is called when SIGINT or SIGTERM is received
 * printf instead of cout because printf is safer and more reliable in signal handlers, avoiding issues like thread safety, complexity, and potential deadlocks. it's asynchronous so can interrupt any time
 */
void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        printf("\nReceived signal %d, preparing to exit...\n", signum);
        force_quit = true;
    }
}

int main(int argc, char *argv[]) {
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "Starting DPDK initialization..." << std::endl;
    if (dpdk_init(argc, argv) < 0) {
        std::cerr << "DPDK initialization failed." << std::endl;
        return -1;
    }
    std::cout << "DPDK initialization completed." << std::endl;

    MarketDataHandler handler;

    /* Launch RX core
     * This core is responsible for receiving packets
     */
    std::cout << "Launching RX core..." << std::endl;
    if (rte_eal_remote_launch(lcore_rx, &handler, RX_CORE) != 0) {
        std::cerr << "Failed to launch RX core." << std::endl;
        return -1;
    }
    std::cout << "RX core launched." << std::endl;

    /* Launch worker core
     * This core processes the received market data
     */
    std::cout << "Launching worker core..." << std::endl;
    if (rte_eal_remote_launch(lcore_worker, &handler, WORKER_CORE) != 0) {
        std::cerr << "Failed to launch worker core." << std::endl;
        return -1;
    }
    std::cout << "Worker core launched." << std::endl;

    // Some time for the cores to initialize
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Simulate market activity
    handler.simulate_market_activity(10000);  // 10,000 orders

    // Allow processing to complete first
    std::this_thread::sleep_for(std::chrono::seconds(5));

    handler.printStats();

    force_quit = true;
    /* Wait for all cores to complete
     * This ensures orderly shutdown
     */
    std::cout << "Waiting for all cores to complete..." << std::endl;
    rte_eal_mp_wait_lcore();

    // Clean up DPDK resources
    dpdk_cleanup();
    std::cout << "DPDK cleanup completed." << std::endl;

    return 0;
}
