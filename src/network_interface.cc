#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  EthernetFrame msg = EthernetFrame();
  uint32_t ip_des = next_hop.ipv4_numeric();
  msg.payload = serialize(dgram);
  msg.header.src = this->ethernet_address_;
  msg.header.type = EthernetHeader::TYPE_IPv4;
  auto p = arp_.find(ip_des);
  if(p == arp_.end() || arp_[ip_des].second < cur_time){
    frame_[ip_des].first.push(std::move(msg));
    EthernetFrame arp_frame;
    auto q = frame_.find(ip_des);
    if(q != frame_.end() && frame_[ip_des].second.has_value() && frame_[ip_des].second >= cur_time){
      return;
    } 
    arp_frame.header.type = EthernetHeader::TYPE_ARP;
    arp_frame.header.src = this->ethernet_address_;
    arp_frame.header.dst = ETHERNET_BROADCAST;

    ARPMessage arp_msg = ARPMessage();
    arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
    arp_msg.sender_ip_address = this->ip_address_.ipv4_numeric();
    arp_msg.sender_ethernet_address = this->ethernet_address_;
    arp_msg.target_ip_address = ip_des;

    arp_frame.payload = serialize(arp_msg);
    transmit(arp_frame);
    frame_[ip_des].second = cur_time + 5000;
  }
  else{
    msg.header.dst = arp_[ip_des].first;
    transmit(msg);
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.
  if(frame.header.dst != this->ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST)
    return;
  if(frame.header.type == EthernetHeader::TYPE_ARP){
    ARPMessage msg = ARPMessage();
    if(parse(msg, frame.payload) && msg.target_ip_address == this->ip_address_.ipv4_numeric()){
      arp_[msg.sender_ip_address] = make_pair(msg.sender_ethernet_address, cur_time + 30000);
      if(msg.opcode == ARPMessage::OPCODE_REQUEST){
        EthernetFrame rps = EthernetFrame();
        rps.header.type = EthernetHeader::TYPE_ARP;
        rps.header.src = this->ethernet_address_;
        rps.header.dst = msg.sender_ethernet_address;

        ARPMessage arp_rps = ARPMessage();
        arp_rps.opcode = ARPMessage::OPCODE_REPLY;
        arp_rps.sender_ip_address = this->ip_address_.ipv4_numeric();
        arp_rps.sender_ethernet_address = this->ethernet_address_;
        arp_rps.target_ip_address = msg.sender_ip_address;
        arp_rps.target_ethernet_address = msg.sender_ethernet_address;
        
        rps.payload = serialize(arp_rps);
        transmit(rps);
      }
      else{
        std::queue<EthernetFrame> &ip = frame_[msg.sender_ip_address].first;
        while(!ip.empty()){
          ip.front().header.dst = msg.sender_ethernet_address;
          transmit(ip.front());
          ip.pop();
        }
      }
    }
  }
  else if(frame.header.type == EthernetHeader::TYPE_IPv4){
    InternetDatagram msg = InternetDatagram();
    if(parse(msg, frame.payload)) datagrams_received_.emplace(std::move(msg));
  }

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  this->cur_time += ms_since_last_tick;
}
