#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return num_in_flight;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return num_retrans;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  uint64_t window_visual = max((uint64_t)1, window_size);
  uint64_t window_usable = (window_visual < num_in_flight ? 0 : window_visual - num_in_flight);
  do{
    if(this->is_fin) return;
    TCPSenderMessage msg_ = TCPSenderMessage();
    uint64_t payload_ = 
      (reader().bytes_buffered()>TCPConfig::MAX_PAYLOAD_SIZE ? TCPConfig::MAX_PAYLOAD_SIZE : reader().bytes_buffered());
    if(this->cur_seq == 0) payload_++;
    uint64_t seq_size = 
      (window_usable>payload_ ? payload_ : window_usable);
    payload_ = seq_size;
    if(this->cur_seq == 0){
      msg_.SYN = true;
      payload_--;
    }
    if(this->reader().has_error()){
      msg_.RST = true;
    }
    while(msg_.payload.size() < payload_){
      string_view p = this->reader().peek();
      uint64_t index = min(p.size(), payload_ - msg_.payload.size());
      msg_.payload += p.substr(0, index);
      this->input_.reader().pop(index);
    }
    if(reader().is_finished() && seq_size < window_usable){
      msg_.FIN = true;
      seq_size++;
      this->is_fin = true;
    }
    if(msg_.sequence_length() == 0) return;
    msg_.seqno = Wrap32::wrap(this->cur_seq, this->zero_point);
    this->cur_seq += msg_.sequence_length();
    this->num_in_flight += msg_.sequence_length();
    outstanding_message.push_back(msg_);
    transmit(msg_);
    if(this->expire_time == UINT64_MAX){
      this->expire_time = cur_time + rto;
    }
    window_usable = (window_visual < num_in_flight ? 0 : window_visual - num_in_flight);

  }while(window_usable > 0 && this->reader().bytes_buffered() > 0);
}


TCPSenderMessage TCPSender::make_empty_message() const
{
  return {Wrap32::wrap(cur_seq, zero_point), 0, string(), 0, this->reader().has_error()};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if(msg.ackno.has_value()){
    uint64_t ack_rcv = unwrap(msg.ackno.value());
    if(ack_rcv <= this->cur_seq && ack_rcv > this->ack_){
        this->ack_ = ack_rcv;
        this->rto = this->initial_RTO_ms_;
        this->expire_time = this->cur_time + this->rto;
        this->num_retrans = 0;
        while(!outstanding_message.empty()){
          TCPSenderMessage &p = outstanding_message.front();
          if(unwrap(p.seqno) + p.sequence_length() > this->ack_){
            break;
          }
          num_in_flight -= p.sequence_length();
          outstanding_message.pop_front();
        }
        if(outstanding_message.empty()){
          expire_time = UINT64_MAX;
        }
    }
  }
  this->window_size = msg.window_size;
  if(msg.RST){
    writer().set_error();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  cur_time += ms_since_last_tick;
  if(cur_time >= expire_time && expire_time != 0){
    transmit(outstanding_message.front());
    if(window_size != 0){
      num_retrans++;
      rto = 2*rto;
    }
    expire_time = cur_time + rto;
  }
}

uint64_t TCPSender::unwrap(const Wrap32 &p){
  return p.unwrap(this->zero_point, this->ack_);
}
