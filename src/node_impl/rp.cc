#include "rp.h"

#include <cassert>
#include <cstring>
#include <bits/stdc++.h>

#define DATA 0
#define BROADCAST_TABLE 1
#define TTL 15
#define MAX_EXPIRY 10
using namespace std;

// type 0 is data, 1 is broadcasting neighbours, 2 is broadcasting neighbours to all nodes

struct RPPacketHeader {
private:
    RPPacketHeader() = default;

public:
    IPAddress src_ip;
    IPAddress dest_ip;
    uint8_t type;
    uint8_t ttl = TTL;


    RPPacketHeader(IPAddress src_ip, IPAddress dest_ip, uint8_t type, uint8_t ttl)
        : src_ip(src_ip), dest_ip(dest_ip), type(type), ttl(ttl)  { }
    static RPPacketHeader from_bytes(uint8_t const* bytes)
    {
        RPPacketHeader ph;
        memcpy(&ph, bytes, sizeof(ph));
        return ph;
    }
};

void RPNode::send_segment(IPAddress dest_ip, std::vector<uint8_t> const& segment) const
{
    // assert(false && "Unimplemented");
    RPPacketHeader header(ip, dest_ip, DATA, TTL);
    std::vector<uint8_t> packet(sizeof(header) + segment.size());
    memcpy(packet.data(), &header, sizeof(header));
    memcpy(packet.data() + sizeof(header), segment.data(), segment.size());
    if(routing_table.find(dest_ip) == routing_table.end()){
        broadcast_packet_to_all_neighbors(packet, true);
    }
    else{
        send_packet(routing_table.at(dest_ip).next_hop, packet, true);
    }

}
void RPNode::receive_packet(MACAddress src_mac, std::vector<uint8_t> packet, size_t distance)
{
    expiry_table[src_mac] = MAX_EXPIRY;
    RPPacketHeader header = RPPacketHeader::from_bytes(packet.data());
    IPAddress src_ip =header.src_ip;

    if (header.type == DATA){
        IPAddress dest_ip = header.dest_ip;
        if (header.ttl == 0) {
        return;
        }
        if(dest_ip == ip){
            std::vector<uint8_t> segment(packet.begin() + sizeof(header), packet.end());
            receive_segment(src_ip, segment);
        }
        
        else{
            RPPacketHeader header2 = RPPacketHeader(ip, dest_ip, DATA, header.ttl - 1);
            if(routing_table.find(dest_ip) == routing_table.end()){
                memcpy(packet.data(), &header2, sizeof(header));
                broadcast_packet_to_all_neighbors(packet, true);
            }
            else{
                header.ttl--;
                memcpy(packet.data(), &header2, sizeof(header));
                send_packet(routing_table.at(dest_ip).next_hop, packet, true);
            }
        }
    }
    else if(header.type == BROADCAST_TABLE){
        // // log( "Received broadcast table from " + std::to_string(src_ip));
        std::unordered_map<IPAddress, TableEntry> received_table;
        size_t offset = sizeof(header);
        if(routing_table.find(src_ip) == routing_table.end()) routing_table[src_ip] = TableEntry(src_mac, distance);
        IPAddress ip_;        
        TableEntry entry;
        while(offset < packet.size()){
            // use memcpy instead of reinterpret_cast
            memcpy(&ip_, packet.data() + offset, sizeof(ip_));
            offset += sizeof(ip_);
            memcpy(&entry, packet.data() + offset, sizeof(entry));
            offset += sizeof(TableEntry);
            received_table[ip_] = entry;

            if(entry.next_hop == mac){
                continue;
            }
            if(routing_table.find(ip_) == routing_table.end()){
                routing_table[ip_].next_hop = src_mac;
                routing_table[ip_].distance = distance + entry.distance;
            }
            else if(routing_table[ip_].distance > entry.distance + distance){
                routing_table[ip_].next_hop = src_mac;
                routing_table[ip_].distance = entry.distance + distance;
            }
        }
        std::vector<IPAddress> to_erase;
        // log("Parsed the Received Table");
        for(auto it :routing_table){
            if(it.second.next_hop == src_mac){
                if(received_table.find(it.first) == received_table.end()){
                    to_erase.push_back(it.first);
                }    
                else{
                    routing_table[it.first].distance = received_table[it.first].distance + distance;
                }
            }  
        }
        for(size_t i = 0; i<to_erase.size(); i++){
            routing_table.erase(to_erase[i]);
        }
        
    }
    else{
        assert(false && "Invalid type");
    }

}

void RPNode::do_periodic()
{   
    routing_table[ip] = TableEntry(mac, 0);
    //1) Broadcast the routing table to all neighbours
    RPPacketHeader header(ip, -1, BROADCAST_TABLE, TTL);
    std::vector<uint8_t> packet(sizeof(header) + routing_table.size() * (sizeof(TableEntry) + sizeof(IPAddress)));
    memcpy(packet.data(), &header, sizeof(header));
    int offset = sizeof(header);
    for(auto it = routing_table.begin(); it!=routing_table.end(); it++){
        memcpy(packet.data() + offset, &it->first, sizeof(it->first));
        offset += sizeof(it->first);
        memcpy(packet.data() + offset, &it->second, sizeof(it->second));
        offset += sizeof(it->second);
    }
    broadcast_packet_to_all_neighbors(packet, false);

    //2) Check if any entry in the neighbour table has expired
    std::vector<MACAddress> to_erase;
    for(auto it: expiry_table){
        if(it.second == 0){
            to_erase.push_back(it.first);
        }
        else{
            expiry_table[it.first]--;
        }
    }
    for(size_t i = 0; i<to_erase.size(); i++){
        erase_enteries(to_erase[i]);
    }
}

//Function to erase all entries in the routing table that have next_hop as mac_
void RPNode::erase_enteries(MACAddress mac_){
    std::vector<IPAddress> to_erase;
    for(auto it: routing_table){
        if(it.second.next_hop == mac_){
            to_erase.push_back(it.first);
        }
    }   
    for(size_t i = 0; i<to_erase.size(); i++){
        routing_table.erase(to_erase[i]);
    } 
    expiry_table.erase(mac_);

}
