#pragma once

#include "byte_stream.hh"
#include <cstdint>
#include <map>
#include <set>
#include <vector>

struct reassembler_item
{
  std::string data;
  uint64_t first_index;
  uint64_t last_index; // 左闭右开
  bool is_last;

  bool operator<( const reassembler_item& x ) const { return first_index < x.first_index; }

  reassembler_item( std::string data1, uint64_t first_index1, uint64_t last_index1, bool is_last1 )
    : data( std::move( data1 ) ), first_index( first_index1 ), last_index( last_index1 ), is_last( is_last1 )
  {}
};

class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ) {}

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
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  // This function is for testing only; don't add extra state to support it.
  uint64_t count_bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  uint64_t current_index_ {};
  uint64_t pending_size_ {};
  std::vector<reassembler_item> buffer_ {};
  ByteStream output_;
};
