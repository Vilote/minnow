#pragma once

#include "ipv4_datagram.hh"
#include "parser.hh"
#include "ref.hh"

#include <ranges>
#include <string>
#include <vector>

// Helper to serialize any object (without constructing a Serializer of the caller's own)
// example: ```ethernet_frame.payload = serialize( internet_datagram );```
template<class T>
std::vector<Ref<std::string>> serialize( const T& obj )
{
  Serializer s;
  obj.serialize( s );
  return s.finish();
}

// Helper to parse any object (without constructing a Parser of the caller's own). Returns true if successful.
// example:
//   ```
//   InternetDatagram internet_datagram;
//   if ( parse( internet_datagram, ethernet_frame.payload ) ) {
//     /* process internet_datagram */
//   }
//   ```
template<class T, typename... Targs>
[[nodiscard]] bool parse( T& obj, auto&& buffers, Targs&&... Fargs )
{
  Parser p { std::forward<decltype( buffers )>( buffers ) };
  obj.parse( p, std::forward<Targs>( Fargs )... );
  return not p.has_error();
}

// Concatenate a sequence of buffers into one string
std::string concat( std::ranges::range auto&& r )
{
  std::string ret;
  for ( const auto& x : r ) {
    ret.append( x );
  }
  return ret;
}

// Pretty-print a string (escaping unprintable characters, and with a maximum length)
std::string pretty_print( std::string_view str, size_t max_length = 32 );

inline InternetDatagram clone( const InternetDatagram& x )
{
  auto duplicate_payload = x.payload | std::views::transform( []( auto& i ) { return i.get(); } );
  return { x.header, { duplicate_payload.begin(), duplicate_payload.end() } };
}
