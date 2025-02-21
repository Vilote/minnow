#pragma once

#include <cstdint>
#include <queue>
#include <string>
#include <string_view>

class Reader;
class Writer;

// 字节流类
class ByteStream
{
public:
  // 显式构造函数，接受一个表示字节流容量的64位无符号整数
  explicit ByteStream( uint64_t capacity );

  // 辅助函数，用于访问ByteStream的Reader和Writer接口
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;

  // 标记流遇到了错误
  void set_error() { error_ = true; };
  // 检查流是否遇到了错误
  bool has_error() const { return error_; };

protected:
  // 请在这里为ByteStream添加任何额外的状态，而不是在Writer和Reader接口中添加
  std::queue<std::string> bytes_ {}; // 用于存储字节流中的数据块
  std::string_view view_wnd_ {};     // 用于查看当前可读取的数据
  uint64_t capacity_ {};             // 字节流的总容量
  uint64_t num_bytes_pushed_ {};     // 累计推入字节流的字节数
  uint64_t num_bytes_poped_ {};      // 累计从字节流中弹出的字节数
  uint64_t num_bytes_buffered_ {};   // 当前缓冲区中缓冲的字节数
  bool is_closed_ {};
  // uint64_t capacity_;
  bool error_ {};
};

// 写入器类，继承自ByteStream
class Writer : public ByteStream
{
public:
  // 将数据推送到流中，但仅推送不超过可用容量的数据
  void push( std::string data );
  // 标记流已到达结尾，不再写入任何数据
  void close();

  // 检查流是否已关闭
  bool is_closed() const;
  // 检查当前可以推送到流中的字节数
  uint64_t available_capacity() const;
  // 累计推送到流中的字节总数
  uint64_t bytes_pushed() const;
};

// 读取器类，继承自ByteStream
class Reader : public ByteStream
{
public:
  // 查看缓冲区中的下一批字节
  std::string_view peek() const;
  // 从缓冲区中移除 `len` 个字节
  void pop( uint64_t len );

  // 检查流是否已结束（已关闭且所有数据都已被弹出）
  bool is_finished() const;
  // 当前缓冲区中缓冲的字节数（已推送但未弹出）
  uint64_t bytes_buffered() const;
  // 累计从流中弹出的字节总数
  uint64_t bytes_popped() const;
};

/*
 * read: 一个（已提供）辅助函数，从ByteStream Reader中查看并弹出最多 `max_len` 个字节到一个字符串中
 */
void read( Reader& reader, uint64_t max_len, std::string& out );
