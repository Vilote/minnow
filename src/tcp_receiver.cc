#include "tcp_receiver.hh"
using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) {
    this->reader().set_error();
    return;
  }

  if ( this->synsent_ == false ) {
    if ( !message.SYN ) {
      return;
    }
    this->synsent_ = true;
    this->zero_pointer = message.seqno;
  }

  if ( message.FIN ) {
    this->finsent_ = true;
  }

  uint64_t curr_abs_seqno
    = message.seqno.unwrap( this->zero_pointer, this->reassembler().writer().bytes_pushed() + 1 );

  int64_t stream_index = 0;
  if ( !message.SYN )
    stream_index = curr_abs_seqno - 1;

  // boder check
  if ( stream_index < 0 )
    return;
  this->reassembler_.insert( stream_index, message.payload, message.FIN );
}

optional<Wrap32> TCPReceiver::ackno() const
{
  // 获取发出的 ack （FIN, SYN 之类的长度也算在内）
  Wrap32 ret = Wrap32( 0 );
  if ( this->synsent_ == false )
    return nullopt;

  ret = ret.wrap( this->reassembler().writer().bytes_pushed() + 1, this->zero_pointer );

  if ( this->reassembler().writer().is_closed() )
    ret = ret.wrap( 1, ret );

  return optional<Wrap32>( ret );
}

size_t TCPReceiver::window_size() const
{
  uint32_t res = this->reassembler().writer().available_capacity();
  if ( res >= 65535 )
    return 65535;
  return res;
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage ret;
  ret.ackno = this->ackno();
  ret.window_size = this->window_size();
  ret.RST = this->reader().has_error();

  return ret;
}