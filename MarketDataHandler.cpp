#include "MarketDataHandler.h"
#include "DPDKSetup.h"
#include <iostream>
#include <chrono>
#include <rte_ethdev.h>
#include "TCPIPStack.h"
#include "OrderProtocol.h"
#include <algorithm>
#include <numeric>

extern volatile bool force_quit;

/* Constructor with member initializations
 * Sets up initial state and random number generators for testing
 */
MarketDataHandler::MarketDataHandler()
        : start_time(std::chrono::high_resolution_clock::now()),
          rng(std::random_device{}()),
          price_dist(1000, 2000),  // Price range $10.00 to $20.00
          quantity_dist(1, 1000),  // Quantity range 1 to 1000
          buy_sell_dist(0.5)  // 50% chance of buy or sell
{
    latencies.reserve(10000);  // Reserve space for 10,000 latency measurements
}

/* Handle incoming market data messages
 * Calculates latency and adds message to the processing queue
 */
void MarketDataHandler::handleMessage(const MarketDataMessage& msg) {
    auto now = std::chrono::high_resolution_clock::now();
    auto msg_time = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds>(std::chrono::nanoseconds(msg.timestamp));
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(now - msg_time).count();
    total_latency.fetch_add(latency, std::memory_order_relaxed);
    message_count.fetch_add(1, std::memory_order_relaxed);
    message_queue.push(msg);
}

/* Process messages in the queue
 * Updates the order book and executes trading strategy
 */
void MarketDataHandler::processMessages() {
    MarketDataMessage msg;
    while (!force_quit && message_queue.pop(msg)) {
        auto start = std::chrono::high_resolution_clock::now();

        order_book.addOrder(msg.order_id, msg.price, msg.quantity, msg.symbol[0] == 'B');

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        latencies.push_back(duration.count());

        /* memory_order_relaxed enables atomic operations with no synchronization or ordering guarantees,
         * We don't need any ordering guarantees here so it speeds perf/makes everything easier to debug
         */
        processed_messages.fetch_add(1, std::memory_order_relaxed);
    }
}

/* Executes example trading strategy based on current market state
 * This is very simple and should be replaced with actual strategy
 */
void MarketDataHandler::executeTradingStrategy() {
    uint32_t best_bid = order_book.getBestBid();
    uint32_t best_ask = order_book.getBestAsk();
    if (best_bid > 0 && best_ask < std::numeric_limits<uint32_t>::max()) {
        if (best_ask - best_bid <= 2) {  // Tight spread, potential arbitrage
            // Simulate placing orders
            uint64_t new_order_id = last_order_id.fetch_add(2) + 1;
            Order buy_order{new_order_id, best_bid, 100, true};
            Order sell_order{new_order_id + 1, best_ask, 100, false};
            submit_order(buy_order);
            submit_order(sell_order);
        }
    }
}

/* Print statistics about the market data handler's performance
 * This includes message processing rate, latencies, and order book state
 */
void MarketDataHandler::printStats() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

    std::cout << "Processed messages: " << processed_messages.load(std::memory_order_relaxed) << std::endl;
    if (duration > 0) {
        std::cout << "Messages per second: " << processed_messages.load(std::memory_order_relaxed) / duration << std::endl;
    } else {
        std::cout << "Messages per second: N/A (duration too short)" << std::endl;
    }

    uint64_t message_count_val = message_count.load(std::memory_order_relaxed);
    if (message_count_val > 0) {
        std::cout << "Average latency (ns): " << total_latency.load(std::memory_order_relaxed) / message_count_val << std::endl;
    } else {
        std::cout << "Average latency (ns): N/A (no messages processed)" << std::endl;
    }

    if (!latencies.empty()) {
        auto max_latency = *std::max_element(latencies.begin(), latencies.end());
        std::cout << "Max latency (ns): " << max_latency << std::endl;

        std::sort(latencies.begin(), latencies.end());
        size_t percentile_99_index = latencies.size() * 0.99;
        std::cout << "99th percentile latency (ns): " << latencies[percentile_99_index] << std::endl;
    }

    std::cout << "Best Bid: " << order_book.getBestBid() << std::endl;
    std::cout << "Best Ask: " << order_book.getBestAsk() << std::endl;
}

/* Process a network packet
 * Extracts orders from TCP packets and adds them to the order book
 */
void MarketDataHandler::process_network_packet(const uint8_t* data, size_t len) {
    tcp_stack.process_packet(data, len);
    // Check for complete orders and process them
    while (!force_quit) {
        std::vector<uint8_t> order_data = tcp_stack.get_next_message();
        if (order_data.empty()) break;

        Order order = OrderProtocol::deserialize_order(order_data.data(), order_data.size());
        MarketDataMessage msg;
        msg.order_id = order.order_id;
        msg.price = order.price;
        msg.quantity = order.quantity;
        msg.symbol[0] = order.is_buy ? 'B' : 'S';
        msg.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        handleMessage(msg);
    }
}

// Simulate semi-realistic network delay (number is random i'm not sure what its like in actual prod but i'd imagine its not this large)
void MarketDataHandler::simulate_network_delay() {
    rte_delay_us(50);  // 50 μs network delay
}

/* Submit an order to the network
 * Creates a TCP packet for the order and processes it
 */
void MarketDataHandler::submit_order(const Order& order) {
    uint32_t dest_ip = 0x0A000001;  // Example: 10.0.0.1
    uint16_t dest_port = 12345;     // Example port

    std::vector<uint8_t> order_data = OrderProtocol::serialize_order(order);
    std::vector<uint8_t> packet = tcp_stack.create_packet(dest_ip, dest_port, order_data.data(), order_data.size());

    simulate_network_delay();

    process_network_packet(packet.data(), packet.size());

    std::cout << "Order submitted: ID " << order.order_id << ", Price " << order.price
              << ", Quantity " << order.quantity << ", Is Buy " << order.is_buy << std::endl;
}

/* Generate a random order
 * Used for simulating market activity
 */
Order MarketDataHandler::generate_random_order() {
    Order order;
    order.order_id = last_order_id.fetch_add(1, std::memory_order_relaxed);
    order.price = price_dist(rng);
    order.quantity = quantity_dist(rng);
    order.is_buy = buy_sell_dist(rng);
    return order;
}

/* Simulate market activity by generating and processing random orders
 * This function is used for testing and benchmarking
 */
void MarketDataHandler::simulate_market_activity(int num_orders) {
    std::cout << "Simulating market activity with " << num_orders << " orders..." << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_orders; ++i) {
        Order order = generate_random_order();

        std::vector<uint8_t> order_data = OrderProtocol::serialize_order(order);
        std::vector<uint8_t> packet = tcp_stack.create_packet(0x0A000001, 12345, order_data.data(), order_data.size());

        process_network_packet(packet.data(), packet.size());

        //delay to avoid overwhelming system
        //std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Simulated " << num_orders << " orders in " << duration.count() << " milliseconds" << std::endl;
    std::cout << "Average latency: " << (duration.count() * 1000.0 / num_orders) << " microseconds per order" << std::endl;

}

/* RX core function
 * Receives packets and handles market data messages
 */
int lcore_rx(void *arg) {
    MarketDataHandler* handler = static_cast<MarketDataHandler*>(arg);
    struct rte_mbuf *bufs[BURST_SIZE];

    while (!force_quit) {
        const uint16_t nb_rx = rte_eth_rx_burst(0, 0, bufs, BURST_SIZE);

        for (uint16_t i = 0; i < nb_rx; i++) {
            char* data = rte_pktmbuf_mtod(bufs[i], char*);
            uint16_t data_len = rte_pktmbuf_data_len(bufs[i]);

            // Process as network packet (for order submission)
            handler->process_network_packet(reinterpret_cast<uint8_t*>(data), data_len);

            rte_pktmbuf_free(bufs[i]);
        }
    }

    return 0;
}

/* Worker core function
 * Processes market data messages and executes trading strategy
 */
int lcore_worker(void *arg) {
    MarketDataHandler* handler = static_cast<MarketDataHandler*>(arg);

    while (!force_quit) {
        handler->processMessages();

        /* Optional delay to reduce CPU load and power consumption
         * Adjust this value based on performance testing
         * TO:DO: Adaptive/conditional delay duration on load of messages processed
         */
        //rte_delay_us(100);
    }

    return 0;
}
