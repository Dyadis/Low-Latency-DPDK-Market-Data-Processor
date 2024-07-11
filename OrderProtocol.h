#pragma once

#include <cstdint>
#include <vector>

struct Order {
    uint64_t order_id;
    uint32_t price;
    uint32_t quantity;
    bool is_buy;
};

class OrderProtocol {
public:
    static std::vector<uint8_t> serialize_order(const Order& order);
    static Order deserialize_order(const uint8_t* data, size_t len);
};