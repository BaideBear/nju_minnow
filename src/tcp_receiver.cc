#include "tcp_receiver.hh"
#include <iostream>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.RST){
    this->rst = true;
    this->reassembler_.set_error();
  }
  if(message.SYN){
    this->has_isn = true;
    this->isn = Wrap32(message.seqno);
    message.seqno = message.seqno + 1;
  }
  if(message.FIN){
    this->eof = true;
    this->fin = Wrap32(message.seqno + message.payload.size());
  }
  if(this->has_isn){
    this->reassembler_.insert(
      message.seqno.unwrap(isn, this->writer().bytes_pushed()) - 1,
      message.payload,
      eof
    );
  }
  if(this->reassembler_.has_error()){
    message.RST = true;
    this->rst = true;
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage p;
  //p.RST = this->rst;
  if(this->reassembler_.has_error()){
    p.RST = true;
  }
  Wrap32 ackno = Wrap32::wrap(this->writer().bytes_pushed()+1, isn);
  if(this->has_isn){
    p.ackno = (fin==ackno)?ackno+1:ackno;
  }
  p.window_size = this->writer().available_capacity() > UINT16_MAX ? UINT16_MAX : this->writer().available_capacity();
  return p;
}
