#pragma once

#include <atomic>
#include <chrono>
#include <random>
#include <thread>
#include "OrderBook.h"
#include "LockFreeRingBuffer.h"
#include "SIMDMessageParser.h"
#include "TCPIPStack.h"
#include "OrderProtocol.h"

class MarketDataHandler {
private:
    LockFreeRingBuffer<MarketDataMessage, 1024> message_queue;
    OrderBook order_book;
    std::atomic<uint64_t> processed_messages{0};
    std::chrono::high_resolution_clock::time_point start_time;
    std::atomic<uint64_t> total_latency{0};
    std::atomic<uint64_t> message_count{0};
    std::vector<uint64_t> latencies;
    TCPIPStack tcp_stack;
    std::atomic<uint64_t> last_order_id{0};


    std::mt19937 rng;
    std::uniform_int_distribution<> price_dist;
    std::uniform_int_distribution<> quantity_dist;
    std::bernoulli_distribution buy_sell_dist;

    void executeTradingStrategy();
    void simulate_network_delay();

public:
    MarketDataHandler();
    void handleMessage(const MarketDataMessage& msg);
    void processMessages();
    void printStats();
    void process_network_packet(const uint8_t* data, size_t len);
    void submit_order(const Order& order);
    Order generate_random_order();
    void simulate_market_activity(int num_orders);
};

int lcore_rx(void *arg);
int lcore_worker(void *arg);