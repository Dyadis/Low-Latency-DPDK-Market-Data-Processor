#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <queue>

// Structure to hold IP header information
// Lots of comments for my personal study
struct IPHeader {
    uint32_t src_ip;     // Source IP address
    uint32_t dest_ip;    // Destination IP address
    uint8_t protocol;    // Protocol type (e.g., TCP)
};

// Structure to hold TCP header information
struct TCPHeader {
    uint16_t src_port;   // Source port number
    uint16_t dest_port;  // Destination port number
    uint32_t seq_num;    // Sequence number
    uint32_t ack_num;    // Acknowledgment number
};

// Class representing a TCP connection
class TCPConnection {
private:
    uint32_t remote_ip;          // Remote IP address
    uint16_t remote_port;        // Remote port number
    uint32_t local_ip;           // Local IP address
    uint16_t local_port;         // Local port number
    uint32_t next_seq_num;       // Next sequence number to use
    uint32_t next_ack_num;       // Next acknowledgment number to send
    std::queue<std::vector<uint8_t>> received_data; // Queue of received data packets

public:
    // Constructor to initialize the connection with IP addresses and ports
    TCPConnection(uint32_t rip, uint16_t rport, uint32_t lip, uint16_t lport);

    // Process an incoming TCP packet
    void process_packet(const TCPHeader& tcp_header, const uint8_t* data, size_t len);

    // Create a TCP packet to send data
    std::vector<uint8_t> create_packet(const uint8_t* data, size_t len);

    // Check if there is any received data available
    bool has_data() const { return !received_data.empty(); }

    // Get the next chunk of received data
    std::vector<uint8_t> get_data();
};

// Class representing the TCP/IP stack
class TCPIPStack {
private:
    std::unordered_map<uint64_t, TCPConnection> connections; // Map of active connections

    // Generate a unique key for a connection based on IP and port
    uint64_t get_connection_key(uint32_t ip, uint16_t port);

public:
    // Process an incoming network packet
    void process_packet(const uint8_t* data, size_t len);

    // Create a packet to send data to a remote host
    std::vector<uint8_t> create_packet(uint32_t dest_ip, uint16_t dest_port, const uint8_t* data, size_t len);

    // Retrieve the next available message from any connection
    std::vector<uint8_t> get_next_message();
};

/*
 * IPHeader
IPHeader: Holds basic information about an IP packet, including source and destination IP addresses and the protocol type.
TCPHeader
TCPHeader: Contains information about a TCP packet, such as source and destination ports, sequence number, and acknowledgment number.
TCPConnection
TCPConnection: Represents an individual TCP connection.
remote_ip: The IP address of the remote end of the connection.
remote_port: The port number of the remote end of the connection.
local_ip: The IP address of the local end of the connection.
local_port: The port number of the local end of the connection.
next_seq_num: The sequence number for the next packet to be sent.
next_ack_num: The acknowledgment number for the next packet to be acknowledged.
received_data: A queue to store received data packets.
TCPConnection: Initializes the connection with given remote and local IP addresses and ports.
process_packet: Handles incoming packets by updating acknowledgment numbers and storing received data.
create_packet: Constructs a TCP packet with the provided data and updates the sequence number.
has_data: Checks if there is any received data available.
get_data: Retrieves the next chunk of data that was received.
TCPIPStack
TCPIPStack: Manages multiple TCP connections and processes network packets.
connections: A map to store active connections, using a unique key based on IP and port.
get_connection_key: Generates a unique key for a connection based on IP and port.
process_packet: Processes incoming network packets by extracting headers, finding the corresponding connection, and delegating packet processing to the connection.
create_packet: Creates a TCP packet to send to a remote host, including both TCP and IP headers.
get_next_message: Retrieves the next available message from any connection, if there is data to be rea
 */