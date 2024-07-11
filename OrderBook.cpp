#include "OrderBook.h"
#include <limits>

/* Add a new order to the order book
 * Updates either the bid or ask side based on the order type
 * Realistic average O(1). Worst 0(N) due to very rare hash collisions
 */
void OrderBook::addOrder(uint64_t order_id, uint32_t price, uint32_t quantity, bool is_buy) {
    Order order = {order_id, quantity, price, is_buy};
    if (is_buy) {
        bids[price][order_id] = order;
        order_map[order_id] = &bids[price][order_id];
    } else {
        asks[price][order_id] = order;
        order_map[order_id] = &asks[price][order_id];
    }
}

/* Remove an order from the order book at specified price level
 * Cleans up empty price levels if necessary
 * Realistic average O(1). Worst 0(N) due to very rare hash collisions
 */
void OrderBook::removeOrder(uint64_t order_id) {
    auto it = order_map.find(order_id);
    if (it != order_map.end()) {
        Order* order = it->second; //retrieve order
        if (order->is_buy) {
            bids[order->price].erase(order_id);
            if (bids[order->price].empty()) {
                bids.erase(order->price);
            }
        } else {
            asks[order->price].erase(order_id);
            if (asks[order->price].empty()) {
                asks.erase(order->price);
            }
        }
        order_map.erase(it);
    }
}

/* Modify the quantity of an existing order
 * This function assumes the price remains unchanged
 */
void OrderBook::modifyOrder(uint64_t order_id, uint32_t new_quantity) {
    auto it = order_map.find(order_id);
    if (it != order_map.end()) {
        it->second->quantity = new_quantity;
    }
}

/* Get the best (highest) bid price
 * Returns 0 if there are no bids
 */
uint32_t OrderBook::getBestBid() const {
    return bids.empty() ? 0 : bids.begin()->first;
}

/* Get the best (lowest) ask price
 * Returns the maximum possible value if there are no asks
 */
uint32_t OrderBook::getBestAsk() const {
    return asks.empty() ? std::numeric_limits<uint32_t>::max() : asks.begin()->first;
}