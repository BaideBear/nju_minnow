#pragma once

#include "byte_stream.hh"
#include <iostream>
#include <set>
#include <vector>
#include <deque>
#include <string>
using namespace std;

class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ) {
    capacity = output_.writer().available_capacity();
    for(uint64_t i = 0; i < capacity; i++){
      bytes.push_back(0);
      state.push_back(false);
    }
  }

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  uint64_t capacity = 0;
  std::deque<char> bytes = {};
  std::deque<bool> state = {};
  uint64_t bytes_cnt = 0;
  uint64_t last_input_index = -1;
  uint64_t last_read_index = 0;



  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

  bool has_error() const {return output_.has_error(); }
  void set_error() { output_.set_error(); }

private:
  ByteStream output_; // the Reassembler writes to this ByteStream
};
