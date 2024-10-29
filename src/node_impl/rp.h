#ifndef RP_H
#define RP_H

#include "../node.h"
#include <unordered_map>
#include <set>
#include <vector>

struct TableEntry {
private:
public:
    TableEntry() = default;
    
    MACAddress next_hop;
    size_t distance;

    TableEntry(MACAddress next_hop, size_t distance) : next_hop(next_hop), distance(distance) { }
};

class RPNode : public Node {
    /*
     * XXX
     * Add any fields, helper functions etc here
     */
    std::unordered_map<MACAddress, uint8_t> expiry_table;
    std::unordered_map<IPAddress, TableEntry> routing_table;
public:
    /*
     * NOTE You may not modify the constructor of this class
     */
    RPNode(Simulation* simul, MACAddress mac, IPAddress ip) : Node(simul, mac, ip) { }
    void erase_enteries(IPAddress ip);
    void send_segment(IPAddress dest_ip, std::vector<uint8_t> const& segment) const override;
    void receive_packet(MACAddress src_mac, std::vector<uint8_t> packet, size_t distance) override;
    void do_periodic() override;
};

#endif
