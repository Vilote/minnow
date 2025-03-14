#include "tcp_sender.hh"
#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <ctime>
using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return in_flight_cnt_;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  return retrans_cnt_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  bool window_zero = window_size_ == 0;
  uint64_t available_window
    = ( window_size_ + window_zero ) < in_flight_cnt_ ? 0 : window_size_ + window_zero - in_flight_cnt_;
  do {

    if ( is_fin_sent )
      return;
    // 最大可交付的字节占用的seq
    uint64_t pay_load_size = min( reader().bytes_buffered(), TCPConfig::MAX_PAYLOAD_SIZE );
    // 实际交付数量
    uint64_t seq_size = min( available_window, pay_load_size + ( current_seq_ == 0 ) );
    pay_load_size = seq_size;

    TCPSenderMessage msg = TCPSenderMessage();
    if ( current_seq_ == 0 ) {
      msg.SYN = true;
      pay_load_size--;
    }
    if ( reader().has_error() ) {
      msg.RST = true;
    }

    // 逐步填充数据包，当可用窗口大于上层应用交付的数据时，为提高效率需要进行等待

    while ( msg.payload.size() < pay_load_size ) {
      string_view front_view = reader().peek();
      uint64_t bytes_to_read = min( front_view.size(), pay_load_size - msg.payload.size() );
      msg.payload += front_view.substr( 0, bytes_to_read );
      input_.reader().pop( bytes_to_read );
    }

    if ( reader().is_finished() && seq_size < available_window ) {
      msg.FIN = true;
      seq_size++;
      is_fin_sent = true;
    }
    if ( msg.sequence_length() == 0 )
      return;
    if ( msg.SYN )
      rto_ = initial_RTO_ms_;
    msg.seqno = Wrap32::wrap( current_seq_, isn_ );
    current_seq_ += msg.sequence_length();
    in_flight_cnt_ += msg.sequence_length();
    outstanding_msg_.push_back( msg );
    transmit( msg );
    if ( expire_time_ == UINT64_MAX )
      expire_time_ = current_time_ + rto_;
    available_window
      = ( window_size_ + window_zero ) < in_flight_cnt_ ? 0 : window_size_ + window_zero - in_flight_cnt_;
  } while ( reader().bytes_buffered() != 0 && available_window != 0 );
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage sen;
  sen.seqno = Wrap32::wrap( current_seq_, isn_ );
  sen.SYN = false;
  sen.payload = string();
  sen.FIN = false;
  sen.RST = reader().has_error();
  return sen;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if ( msg.RST ) {
    reader().set_error();
    outstanding_msg_.clear();
    in_flight_cnt_ = 0;
    return;
  }
  window_size_ = msg.window_size;
  if ( msg.ackno.has_value() ) {
    auto ack_from_recv = msg.ackno->unwrap( isn_, current_seq_ );
    if ( ack_from_recv > ack_ && ack_from_recv <= current_seq_ ) {
      ack_ = ack_from_recv;
      rto_ = initial_RTO_ms_;
      expire_time_ = current_time_ + rto_;
      retrans_cnt_ = 0;
      while ( !outstanding_msg_.empty() ) {
        auto& front_msg = outstanding_msg_.front();
        if ( front_msg.seqno.unwrap( isn_, current_seq_ ) + front_msg.sequence_length() > ack_ ) {
          break;
        }
        in_flight_cnt_ -= front_msg.sequence_length();
        outstanding_msg_.pop_front();
      }
      if ( outstanding_msg_.empty() ) {
        expire_time_ = UINT64_MAX;
      }
    }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if ( outstanding_msg_.empty() )
    return;
  current_time_ += ms_since_last_tick;
  if ( current_time_ >= expire_time_ ) {
    transmit( outstanding_msg_.front() );
    if ( window_size_ != 0 ) {
      rto_ *= 2;
      retrans_cnt_++;
    }
    expire_time_ = current_time_ + rto_;
  }
}
