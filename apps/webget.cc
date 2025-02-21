#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
  // 解析主机名，建立 TCP 连接
  Address address( host, "http" );
  TCPSocket tcp_socket;
  tcp_socket.connect( address );

  // 发送 HTTP 请求
  string request = "GET " + path
                   + " HTTP/1.1\r\n"
                     "Host: "
                   + host
                   + "\r\n"
                     "Connection: close\r\n"
                     "\r\n";

  tcp_socket.write( request );    // 发送完整的 HTTP 请求
  tcp_socket.shutdown( SHUT_WR ); // 关闭写端，表示请求已完成

  // 读取服务器响应
  string buffer;
  while ( !tcp_socket.eof() ) {
    tcp_socket.read( buffer ); // 读取数据
    cout << buffer;            // 输出数据
  }
  tcp_socket.close();
  cerr << "Function called: get URL(" << host << "," << path << ").\n";
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
