#pragma once

#include <memory>
#include <optional>

#include "exception.hh"
#include "network_interface.hh"


struct routing_node{
  routing_node( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num ){
                    this->route_prefix_ = route_prefix;
                    this->prefix_length_ = prefix_length;
                    this->mask_ = prefix_length!=0 ? ~((1 << (32-prefix_length)) - 1) : 0;
                    this->interface_num_ = interface_num;
                    this->next_hop_ = next_hop;
                  }
  
  uint32_t route_prefix_ = 0;
  uint8_t prefix_length_ = 0;
  uint32_t mask_ = 0;
  std::optional<Address> next_hop_ = {};
  size_t interface_num_ = 0;
};

// \brief A router that has multiple network interfaces and
// performs longest-prefix-match routing between them.
class Router
{
public:
  // Add an interface to the router
  // \param[in] interface an already-constructed network interface
  // \returns The index of the interface after it has been added to the router
  size_t add_interface( std::shared_ptr<NetworkInterface> interface )
  {
    _interfaces.push_back( notnull( "add_interface", std::move( interface ) ) );
    return _interfaces.size() - 1;
  }

  // Access an interface by index
  std::shared_ptr<NetworkInterface> interface( const size_t N ) { return _interfaces.at( N ); }

  // Add a route (a forwarding rule)
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // Route packets between the interfaces
  void route();

private:
  // The router's collection of network interfaces
  std::vector<routing_node> routing_table {};
  std::vector<std::shared_ptr<NetworkInterface>> _interfaces {};
};
