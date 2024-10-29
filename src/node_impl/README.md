# Network Layer Routing

## Table of Contents

- [Overview](#overview)
- [Packet Types](#packet-types)
- [Protocol Components](#protocol-components)
  - [Header Structure](#header-structure)
  - [Routing Table](#routing-table)
  - [Expiry Table](#expiry-table)
- [Key Functions](#key-functions)
  - [Sending Data Segments](#sending-data-segments)
  - [Receiving Packets](#receiving-packets)
  - [Periodic Updates](#periodic-updates)
- [Code Structure](#code-structure)

## Overview

I have implemented the Distance Vector Protocol in rp.cc and rp.h

## Packet Types

There are two types of packets in RPRP:

1. **DATA (Type 0)**: Standard data packets containing a source IP, destination IP, and a segment of data. Nodes forward these packets based on their routing table.
2. **BROADCAST_TABLE (Type 1)**: Packets used to share routing information with neighbors. Nodes use these packets to update their routing tables with new routes or distances.

## Protocol Components

### Header Structure

The protocol uses a custom header structure defined as `RPPacketHeader`. This header contains the following fields:
- `src_ip`: Source IP address of the packet.
- `dest_ip`: Destination IP address of the packet.
- `type`: Packet type (`DATA` or `BROADCAST_TABLE`).
- `ttl`: Time-to-live (TTL), initialized to a maximum value (`TTL = 15`) and decremented each time the packet is forwarded. The packet is discarded if TTL reaches zero.

### Routing Table

Each node maintains a **routing table** that contains:
- **Next-hop MAC address** for each known destination.
- **Distance** (hop count) to the destination.

### Expiry Table

Nodes also maintain an **expiry table** to track the validity of only the neighbouring enteries.
Each entry has a timeout (default `MAX_EXPIRY = 10`) that decrements periodically, and entries are removed once the timeout reaches zero.

## Key Functions

### Sending Data Segments

The `send_segment` function creates a `DATA` packet for a given destination IP and segment of data. If the destination is found in the routing table, the packet is forwarded to the next hop. Otherwise, the packet is broadcast to all neighbors.

### Receiving Packets

The `receive_packet` function processes incoming packets based on their type:
- **DATA Packets**: If the node is the intended recipient, the data segment is extracted and processed. Otherwise, the packet is forwarded to the next hop or broadcast if no route exists.
- **BROADCAST_TABLE Packets**: The function updates the routing table with information from the broadcasted table, adding new entries or updating existing ones with better distances.
The updation:
We remove a neighbouring entry if we have not received any packet from it for MAX_EXPIRY times because neighbour continuosly keeps broadcasting its table, which updates it's expiry to MAX_EXPIRY, otherwise we delete this neighbour in the routing table, and all enteries in routing table which have next hop as this address.

### Periodic Updates

The `do_periodic` function is called periodically by each node. It:
1. Broadcasts the current routing table to all neighbors.
2. Checks the expiry table for expired neighbour entries and removes them and the enteries in routing table which have the next hop as this mac.

## Code Structure

- **`RPPacketHeader`**: Defines the structure of packet headers.
- **`RPNode::send_segment`**: Handles sending of data packets.
- **`RPNode::receive_packet`**: Processes incoming packets, updating routing tables as needed.
- **`RPNode::do_periodic`**: Broadcasts routing tables and cleans up expired entries.
- **`RPNode::erase_entries`**: Helper function to remove routing entries which have next hop as the provided mac address in the arguments.