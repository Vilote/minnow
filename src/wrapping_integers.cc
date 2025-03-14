#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{

  return zero_point + ( n & 0x00000000FFFFFFFF );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t offset = raw_value_ + -zero_point.raw_value_;
  uint32_t left = checkpoint % 0x1'0000'0000;
  uint64_t high32 = checkpoint - left;
  if ( offset < left ) {
    if ( left - offset <= 0x8000'0000 ) {
      return high32 + offset;
    } else {
      return high32 + offset + 0x1'0000'0000;
    }
  } else {
    if ( high32 == 0 || offset - left <= 0x8000'0000 ) {
      return high32 + offset;
    } else {
      return high32 + offset - 0x1'0000'0000;
    }
  }
}
