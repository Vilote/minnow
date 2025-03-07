#include "helpers.hh"

#include <iomanip>

using namespace std;

string pretty_print( string_view str, size_t max_length )
{
  ostringstream ss;
  bool truncated = false;
  for ( const uint8_t ch : str ) {
    if ( ss.str().size() >= max_length ) {
      truncated = true;
      break;
    }

    if ( isprint( ch ) and ch != '"' ) {
      ss << ch;
    } else {
      ss << "\\x" << fixed << setw( 2 ) << setfill( '0' ) << hex << static_cast<size_t>( ch );
    }
  }
  string ret = ss.str();
  if ( truncated ) {
    if ( ret.size() >= 3 ) {
      ret.replace( ret.size() - 3, 3, "..." );
    } else {
      ret += "...";
    }
  }
  return ret;
}
