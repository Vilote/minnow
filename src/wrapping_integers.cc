#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{

  return zero_point + ( n & 0x00000000FFFFFFFF );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  auto base = checkpoint & 0xffffffff00000000;
  auto candidate_n = raw_value_ - zero_point.raw_value_ + base;
  if ( checkpoint != 0 ) {
    if ( raw_value_ < zero_point.raw_value_ )
      candidate_n = ( 1ULL << 32 ) - ( zero_point.raw_value_ - raw_value_ ) + base;
    if ( candidate_n < checkpoint && checkpoint - candidate_n > ( 1ULL << 31 ) ) {
      candidate_n += ( 1ULL << 32 );
    } else if ( candidate_n > checkpoint && candidate_n - checkpoint > ( 1ULL << 31 ) ) {
      candidate_n -= ( 1ULL << 32 );
    }
  }
  return candidate_n;
}
