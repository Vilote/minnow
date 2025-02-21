#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// Writer类的push方法，用于将数据推入字节流
void Writer::push( string data )
{
  if ( is_closed() )
    return;
  if ( data.size() > available_capacity() )
    data.resize( available_capacity() );
  if ( !data.empty() ) {
    num_bytes_pushed_ += data.size();
    num_bytes_buffered_ += data.size();
    bytes_.emplace( move( data ) );
  }
  // 临界条件：pop 了所有字节导致队列为空且 view_wnd_ 为空
  if ( view_wnd_.empty() && !bytes_.empty() )
    view_wnd_ = bytes_.front();
}

void Writer::close()
{
  if ( !is_closed_ ) {
    is_closed_ = true;
    // 防止重复关闭，将表示文件结束的EOF字符作为一个字符串插入队列
    bytes_.emplace( string( 1, EOF ) );
  }
}

bool Writer::is_closed() const
{
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - num_bytes_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  return num_bytes_pushed_;
}

// Reader类的peek方法，用于查看当前可读取的数据
string_view Reader::peek() const
{
  return view_wnd_;
}

// Reader类的pop方法，用于从字节流中移除指定长度的数据
void Reader::pop( uint64_t len )
{
  auto remainder = len;
  while ( remainder >= view_wnd_.size() && remainder != 0 ) {
    // 减去查看窗口的长度
    remainder -= view_wnd_.size();
    bytes_.pop();
    // 如果队列为空，将查看窗口置为空字符串视图，否则将队列的第一个元素赋值给查看窗口
    view_wnd_ = bytes_.empty() ? ""sv : bytes_.front();
  }
  if ( !view_wnd_.empty() )
    view_wnd_.remove_prefix( remainder );

  num_bytes_buffered_ -= len;
  num_bytes_poped_ += len;
}

bool Reader::is_finished() const
{
  // 当且仅当写者关闭、存在队列中未 pop 的字节数为 0
  return is_closed_ && bytes_buffered() == 0;
}

uint64_t Reader::bytes_buffered() const
{
  return num_bytes_buffered_;
}

uint64_t Reader::bytes_popped() const
{
  return num_bytes_poped_;
}
