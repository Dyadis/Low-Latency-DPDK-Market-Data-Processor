#pragma once

#include <cstdint>
#include <cstring>
#include <x86intrin.h>

// Structure to hold market data message details
// Using tons of comments because I looked at it the morning after I wrote it and couldn't understand what the hell I was thinking
struct MarketDataMessage {
    uint64_t timestamp;      // Timestamp of the message - 8 bytes
    uint32_t sequence_number;// Sequence number of the message - 4 bytes
    char message_type;       // Type of the message (single character) - 1 byte
    char symbol[8];          // Symbol associated with the order (8 characters) - 8 bytes
    uint64_t order_id;       // Unique order identifier - 8 bytes
    uint32_t price;          // Price of the order - 4 bytes
    uint32_t quantity;       // Quantity of the order - 4 bytes
};

// Parser class for converting raw data into a MarketDataMessage
class SIMDMessageParser {
public:
    // Static method to parse raw data into a MarketDataMessage
    static MarketDataMessage parse(const char* data) {
        MarketDataMessage msg; // Declare a message object to hold the parsed data

        // Extract timestamp from the first 8 bytes
        msg.timestamp = *reinterpret_cast<const uint64_t*>(data);
        // Extract sequence number from the next 4 bytes
        msg.sequence_number = *reinterpret_cast<const uint32_t*>(data + 8);
        // Extract message type from the next byte
        msg.message_type = data[12];
        // Copy the next 8 bytes into the symbol array
        std::memcpy(msg.symbol, data + 13, 8);
        // Extract order ID from the next 8 bytes
        msg.order_id = *reinterpret_cast<const uint64_t*>(data + 21);
        // Extract price from the next 4 bytes
        msg.price = *reinterpret_cast<const uint32_t*>(data + 29);
        // Extract quantity from the next 4 bytes
        msg.quantity = *reinterpret_cast<const uint32_t*>(data + 33);

        // Return the populated message object
        return msg;

        /*
        Alternative implementation using AVX2 instructions for faster processing (commented out)

        __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data));

        // Extract timestamp (64 bits) from the first 8 bytes
        msg.timestamp = _mm256_extract_epi64(chunk, 0);
        // Extract sequence number (32 bits) from bytes 8-11
        msg.sequence_number = _mm256_extract_epi32(chunk, 2);
        // Extract message type (8 bits) from byte 12
        msg.message_type = static_cast<char>(_mm256_extract_epi8(chunk, 12));

        // Load 128 bits (16 bytes) of data for the symbol starting at byte 13
        __m128i symbol_chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 13));
        // Store the symbol data (8 bytes) into the symbol array
        _mm_storeu_si128(reinterpret_cast<__m128i*>(msg.symbol), symbol_chunk);

        // Extract order ID (64 bits) from bytes 21-28
        msg.order_id = _mm256_extract_epi64(chunk, 3);
        // Extract price (32 bits) from bytes 29-32
        msg.price = _mm256_extract_epi32(chunk, 6);
        // Extract quantity (32 bits) from bytes 33-36
        msg.quantity = _mm256_extract_epi32(chunk, 7);

        // Return the populated message object
        return msg;
        */
    }
};



