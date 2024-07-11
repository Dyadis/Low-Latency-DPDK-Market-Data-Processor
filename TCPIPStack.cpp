#include "TCPIPStack.h"
#include <algorithm>

// Constructor for TCPConnection initializes remote and local IP and ports and sets sequence numbers to 0
TCPConnection::TCPConnection(uint32_t rip, uint16_t rport, uint32_t lip, uint16_t lport)
        : remote_ip(rip), remote_port(rport), local_ip(lip), local_port(lport), next_seq_num(0), next_ack_num(0) {}

// Process an incoming packet for this connection
void TCPConnection::process_packet(const TCPHeader& tcp_header, const uint8_t* data, size_t len) {
    // Update the acknowledgment number to the next expected sequence number
    next_ack_num = tcp_header.seq_num + len;
    // If the packet contains data, store it
    if (len > 0) {
        received_data.push(std::vector<uint8_t>(data, data + len));
    }
}

// Create a TCP packet with the given data
std::vector<uint8_t> TCPConnection::create_packet(const uint8_t* data, size_t len) {
    // Allocate space for the TCP header and the data
    std::vector<uint8_t> packet(sizeof(TCPHeader) + len);
    // Pointer to the TCP header portion of the packet
    TCPHeader* header = reinterpret_cast<TCPHeader*>(packet.data());
    // Fill in the TCP header fields
    header->src_port = local_port;
    header->dest_port = remote_port;
    header->seq_num = next_seq_num;
    header->ack_num = next_ack_num;

    // Copy the data into the packet after the header
    std::copy(data, data + len, packet.begin() + sizeof(TCPHeader));
    // Update the sequence number for the next packet to send
    next_seq_num += len;
    return packet;
}

// Retrieve the next chunk of received data
std::vector<uint8_t> TCPConnection::get_data() {
    if (received_data.empty()) return {};
    auto data = std::move(received_data.front());
    received_data.pop();
    return data;
}

// Generate a unique key for a connection based on the IP and port
uint64_t TCPIPStack::get_connection_key(uint32_t ip, uint16_t port) {
    return (static_cast<uint64_t>(ip) << 16) | port;
}

// Process an incoming network packet
void TCPIPStack::process_packet(const uint8_t* data, size_t len) {
    if (len < sizeof(IPHeader) + sizeof(TCPHeader)) return; // Ignore packets that are too small

    // Interpret the beginning of the data as IP and TCP headers
    const IPHeader* ip_header = reinterpret_cast<const IPHeader*>(data);
    const TCPHeader* tcp_header = reinterpret_cast<const TCPHeader*>(data + sizeof(IPHeader));

    // Generate a key for the connection based on the source IP and port
    uint64_t conn_key = get_connection_key(ip_header->src_ip, tcp_header->src_port);
    auto it = connections.find(conn_key);
    if (it == connections.end()) {
        // If the connection does not exist, create a new one
        connections.emplace(std::piecewise_construct,
                            std::forward_as_tuple(conn_key),
                            std::forward_as_tuple(ip_header->src_ip, tcp_header->src_port, ip_header->dest_ip, tcp_header->dest_port));
        it = connections.find(conn_key);
    }

    // Process the TCP packet using the appropriate connection
    it->second.process_packet(*tcp_header, data + sizeof(IPHeader) + sizeof(TCPHeader),
                              len - sizeof(IPHeader) - sizeof(TCPHeader));
}

// Create a TCP packet to send to a remote host
std::vector<uint8_t> TCPIPStack::create_packet(uint32_t dest_ip, uint16_t dest_port, const uint8_t* data, size_t len) {
    // Generate a key for the connection based on the destination IP and port
    uint64_t conn_key = get_connection_key(dest_ip, dest_port);
    auto it = connections.find(conn_key);
    if (it == connections.end()) {
        // If the connection does not exist, create a new one
        connections.emplace(std::piecewise_construct,
                            std::forward_as_tuple(conn_key),
                            std::forward_as_tuple(dest_ip, dest_port, 0, 0));  // Use appropriate local IP and port
        it = connections.find(conn_key);
    }

    // Create a TCP packet using the connection
    std::vector<uint8_t> tcp_packet = it->second.create_packet(data, len);
    // Allocate space for the IP header and the TCP packet
    std::vector<uint8_t> ip_packet(sizeof(IPHeader) + tcp_packet.size());

    // Pointer to the IP header portion of the packet
    IPHeader* ip_header = reinterpret_cast<IPHeader*>(ip_packet.data());
    // Fill in the IP header fields
    ip_header->src_ip = 0;  // Use appropriate local IP
    ip_header->dest_ip = dest_ip;
    ip_header->protocol = 6;  // TCP
    // Set other IP header fields as needed

    // Copy the TCP packet into the IP packet after the header
    std::copy(tcp_packet.begin(), tcp_packet.end(), ip_packet.begin() + sizeof(IPHeader));
    return ip_packet;
}

// Retrieve the next available message from any connection
std::vector<uint8_t> TCPIPStack::get_next_message() {
    for (auto& conn : connections) {
        if (conn.second.has_data()) {
            return conn.second.get_data();
        }
    }
    return {};
}


/*
 * TCPConnection
Constructor: Initializes a TCP connection with given remote and local IP addresses and ports.
process_packet: Handles incoming packets by updating acknowledgment numbers and storing any received data.
create_packet: Constructs a TCP packet with the provided data and updates the sequence number.
get_data: Retrieves the next chunk of data that was received.
TCPIPStack
get_connection_key: Generates a unique key for a connection based on the IP address and port, used to identify connections.
process_packet: Processes incoming network packets by extracting headers, finding the corresponding connection, and delegating packet processing to the connection.
create_packet: Creates a TCP packet to send to a remote host, including both TCP and IP headers.
get_next_message: Retrieves the next available message from any connection, if there is data to be read.

 */