#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // Your code here.
  routing_table.emplace_back(route_prefix, prefix_length, next_hop, interface_num);
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // Your code here.
  for (auto &interface: _interfaces){
    auto &queue = interface->datagrams_received();
    while(!queue.empty()){
      InternetDatagram &dat = queue.front();
      if(dat.header.ttl == 0 || dat.header.ttl == 1){
        queue.pop();
        continue;
      }
      dat.header.ttl -= 1;
      dat.header.compute_checksum();
      uint32_t ip_dst = dat.header.dst;
      optional<routing_node> match;
      for (auto &node: routing_table){
        if(node.route_prefix_ == (ip_dst & node.mask_)){
          if(!match.has_value() || match->mask_ < node.mask_){
            match = node;
          }
        }
      }
      if(match.has_value()){
        auto &next_interface = _interfaces.at(match->interface_num_);
        if(match->next_hop_.has_value()){
          next_interface->send_datagram(dat, match->next_hop_.value());
        }
        else{
          next_interface->send_datagram(dat, Address::from_ipv4_numeric(dat.header.dst));
        }
      }
      queue.pop();
    }
  }
}
