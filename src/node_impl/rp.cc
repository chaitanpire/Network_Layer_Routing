#include "rp.h"

#include <cassert>
#include <cstring>

#define DATA 0
#define BROADCAST_NEIGHBOURS 1
#define BROADCAST_NEIGHBOURS_TO_ALL 2
// type 0 is data, 1 is broadcasting neighbours, 2 is broadcasting neighbours to all nodes

struct RPPacketHeader {
private:
    RPPacketHeader() = default;

public:
    IPAddress src_ip;
    IPAddress dest_ip;
    uint8_t type;

    RPPacketHeader(IPAddress src_ip, IPAddress dest_ip, uint8_t type)
        : src_ip(src_ip), dest_ip(dest_ip), type(type) { }
    static RPPacketHeader from_bytes(uint8_t const* bytes)
    {
        RPPacketHeader ph;
        memcpy(&ph, bytes, sizeof(ph));
        return ph;
    }
};

void RPNode::send_segment(IPAddress dest_ip, std::vector<uint8_t> const& segment) const
{
    /*
     * XXX
     * Implement this function
     */
    // This function will be called only when we have to send data
    RPPacketHeader ph(ip, dest_ip, DATA);
    std::vector<uint8_t> packet(sizeof(ph) + segment.size());
    memcpy(&packet[0], &ph, sizeof(ph));
    memcpy(&packet[sizeof(ph)], &segment[0], segment.size());
    broadcast_packet_to_all_neighbors(packet);

    // assert(false && "Unimplemented");
}

void RPNode::receive_packet(MACAddress src_mac, std::vector<uint8_t> packet, size_t distance)
{
    /*
     * XXX
     * Implement this function
     */

    RPPacketHeader ph = RPPacketHeader::from_bytes(&packet[0]);
    if(ph.type == DATA)
    {
        if(ph.dest_ip == ip)
        {
            std::vector<uint8_t> segment(packet.begin() + sizeof(ph), packet.end());
            receive_segment(ph.src_ip, segment);
        }
        else
        {
            // Forward the packet to the next hop
            IPAddress next_hop_ip = next_hop[ph.dest_ip];
            send_segment(next_hop_ip, packet);
        }
    }
    else if(ph.type == BROADCAST_NEIGHBOURS)
    {
        // Add the neighbour to the neighbour table
        neighbours[ph.src_ip] = distance;
    }
    else if(ph.type == BROADCAST_NEIGHBOURS_TO_ALL)
    {
        // Need to iterate over the packet information and update the adjacency list
        std::pair <IPAddress, int> neighbour;
        for(size_t i = sizeof(ph); i < packet.size(); i += sizeof(neighbour))
        {
            memcpy(&neighbour, &packet[i], sizeof(neighbour));
            adjacency_list[ph.src_ip][neighbour.first] = neighbour.second;
        }
        

    }
    else
    {
        assert(false && "Invalid packet type");
    }

    /*This function will be called when we 
    1)receive data, 
    2) broadcast from neighbours, 
    3) broadcast from other nodes, 
    */ 
    assert(false && "Unimplemented");
}

void RPNode::do_periodic()
{
    /*
     * XXX
     * Implement this function
     */

    //  Broadcast to all neighbours, to create neighbour table
    RPPacketHeader ph(ip, 0, BROADCAST_NEIGHBOURS);
    std::vector<uint8_t> packet(sizeof(ph));
    memcpy(&packet[0], &ph, sizeof(ph));
    broadcast_packet_to_all_neighbors(packet);
    //  add delay


    //  Blast to all nodes about the neighbours
    ph.type = BROADCAST_NEIGHBOURS_TO_ALL;
    std::vector<uint8_t> packet2(sizeof(ph));
    memcpy(&packet2[0], &ph, sizeof(ph));
    //memcpy the neighbours to the packet
    for(auto neighbour : neighbours)
    {
        memcpy(&packet2[sizeof(ph)], &neighbour, sizeof(neighbour));
    }    
    broadcast_packet_to_all_neighbors(packet2);
    //  add delay

    // Perform Djikstra and update the distance table and the next hop table
    
    // Initialize the distance and next hop table
    for(auto neighbour : neighbours)
    {
        distance[neighbour.first] = neighbour.second;
        next_hop[neighbour.first] = neighbour.first;
    }
    


    assert(false && "Unimplemented");
}
