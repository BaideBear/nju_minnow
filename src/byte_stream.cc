#include "byte_stream.hh"
#include <assert.h>
#include <string>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  return this->finished_;
}

void Writer::push( string data )
{
  if(data.empty()) return;

  uint64_t len = data.length();
  if(len > available_capacity()){
    len = available_capacity();
  }
  if(len <= 0) return;
  // for (uint64_t i = 0; i < len; i++){
  //   buffer.push_back(data[i]);
  // }

  buffer_ += data.substr(0, len);

  this->used += len;
  this->wtotal += len;
}

void Writer::close()
{
  this->finished_ = true;
}

uint64_t Writer::available_capacity() const
{
  return this->capacity_ - this->used;
}

uint64_t Writer::bytes_pushed() const
{
  return this->wtotal;
}

bool Reader::is_finished() const
{
  return (this->used == 0) && (this->finished_);
  //return this->finished_;
}

uint64_t Reader::bytes_popped() const
{
  return this->rtotal;
}

string_view Reader::peek() const
{
  //return string{buffer.begin(), buffer.end()};
  return buffer_;
}

void Reader::pop( uint64_t len )
{
  if(len > this->buffer_.size()){
    len = this->buffer_.size();
  }
  // for(uint64_t i = 0; i < len; i++){
  //   buffer.pop_front();
  // }
  //buffer.erase(buffer.begin(), buffer.begin()+len);

  buffer_.erase(0, len);

  this->used = (used-len > 0)?used-len:0;
  this->rtotal += len;
}

uint64_t Reader::bytes_buffered() const
{
  return this->used;
}