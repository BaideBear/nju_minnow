#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32(static_cast<uint32_t>(n) + zero_point.raw_value_);
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t quotient = checkpoint>>32;
  uint64_t remainder = (checkpoint<<32) >> 32;
  uint64_t middle = 1UL<<31;
  if(remainder >= middle) quotient++;
  uint64_t abs = static_cast<uint64_t>(this->raw_value_ - zero_point.raw_value_);
  uint64_t abs_1 = abs + (quotient << 32);
  uint64_t abs_2 = abs + ((quotient==0?0:quotient-1) << 32);
  if(checkpoint < (abs_1 + abs_2)/2) abs = abs_2;
  else abs = abs_1;
  
  return abs;
}
