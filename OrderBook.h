#pragma once

#include <map>
#include <unordered_map>
#include <cstdint>


class OrderBook {
private:
    struct Order {

        /* uint64_t is used for order_id to ensure a vast range for unique identifiers
         * uint32_t is used for price and quantity to balance range, memory efficiency, and performance (better cache utilization)
         */
        uint64_t order_id;
        uint32_t quantity;
        uint32_t price;
        bool is_buy;
    };

    // Bids are sorted in descending order, asks in ascending order. Time O(log K). Map is balancy binary search tree. K = price level
    std::map<uint32_t, std::unordered_map<uint64_t, Order>, std::greater<uint32_t>> bids;
    std::map<uint32_t, std::unordered_map<uint64_t, Order>, std::less<uint32_t>> asks;
    // Map for quick lookup. Will look into combining with stable vectors in the near future
    std::unordered_map<uint64_t, Order*> order_map;

public:
    void addOrder(uint64_t order_id, uint32_t price, uint32_t quantity, bool is_buy);
    void removeOrder(uint64_t order_id);
    void modifyOrder(uint64_t order_id, uint32_t new_quantity);
    uint32_t getBestBid() const;
    uint32_t getBestAsk() const;
};