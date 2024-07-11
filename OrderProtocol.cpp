#include "OrderProtocol.h"
#include <cstring>

/* Serialize an Order object into a byte vector
 * This allows the order to be transmitted over the network
 */
std::vector<uint8_t> OrderProtocol::serialize_order(const Order& order) {
    std::vector<uint8_t> data(sizeof(Order));
    memcpy(data.data(), &order, sizeof(Order));
    return data;
}

/* Deserialize a byte array into an Order object
 * This allows received data to be converted back into an Order
 */
Order OrderProtocol::deserialize_order(const uint8_t* data, size_t len) {
    Order order;
    if (len >= sizeof(Order)) { //avoids buffer overflows & undefined behaviour so memcpy doesnt copy beyond bounds
        memcpy(&order, data, sizeof(Order));
    }
    return order;
}