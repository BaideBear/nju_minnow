#include "reassembler.hh"
#define MAGIC 0xfefefe

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t i, j, o_start, o_end, len;
  o_end = output_.writer().bytes_pushed();
  o_start = output_.reader().bytes_popped();
  len = first_index + data.length();
  len = len>o_start+capacity ? o_start+capacity : len;

  for (uint64_t p = last_read_index; p < o_start; p++){
    bytes.pop_front();
    bytes.push_back(0);
    state.pop_front();
    state.push_back(false);
  }
  last_read_index = o_start;
  if(first_index <= o_end){
    string s = "";
    for (i = o_end, j = o_end-o_start; i < len; i++, j++){
      bytes[j] = data[i-first_index];
      s += bytes[j];
      if(state[j]) bytes_cnt--;
      state[j] = false;
    }
    for (; i < capacity + o_start && state[j]; j++,i++){
      s += bytes[j];
      state[j] = false;
      bytes_cnt--;
    }
    output_.writer().push(s);
    if(i == last_input_index) output_.writer().close();
  }else{
    for (i = first_index,j = first_index-o_start; i < len; i++,j++){
      bytes[j] = data[i-first_index];
      if(!state[j]){
        state[j] = true;
        bytes_cnt++;
      }
    }
    if(is_last_substring) {
      last_input_index = first_index + data.length();
    }
  }

  if(is_last_substring && bytes_cnt == 0) {
    output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_cnt;
}
