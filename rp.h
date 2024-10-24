#ifndef RP_H
#define RP_H

#include "../node.h"
#include <map>
#include <set>
#include <vector>

class RPNode : public Node {
    /*
     * XXX
     * Add any fields, helper functions etc here
     */


public:
    
    map<IPAddress, int> neighbours;
    map<IPAddress, map<IPAddress, int>> adjacency_list;
    /*
     * NOTE You may not modify the constructor of this class
     */
    RPNode(Simulation* simul, MACAddress mac, IPAddress ip) : Node(simul, mac, ip) { }

    void send_segment(IPAddress dest_ip, std::vector<uint8_t> const& segment) const override;
    void receive_packet(MACAddress src_mac, std::vector<uint8_t> packet, size_t distance) override;
    void do_periodic() override;
};

#endif
