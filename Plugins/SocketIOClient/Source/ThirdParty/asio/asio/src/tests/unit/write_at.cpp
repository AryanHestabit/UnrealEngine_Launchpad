//
// write_at.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/write_at.hpp"

#include <cstring>
#include "archetypes/async_result.hpp"
#include "asio/io_context.hpp"
#include "asio/post.hpp"
#include "asio/streambuf.hpp"
#include "unit_test.hpp"

#if defined(ASIO_HAS_BOOST_BIND)
# include <boost/bind.hpp>
#else // defined(ASIO_HAS_BOOST_BIND)
# include <functional>
#endif // defined(ASIO_HAS_BOOST_BIND)

#if defined(ASIO_HAS_BOOST_ARRAY)
#include <boost/array.hpp>
#endif // defined(ASIO_HAS_BOOST_ARRAY)

#if defined(ASIO_HAS_STD_ARRAY)
# include <array>
#endif // defined(ASIO_HAS_STD_ARRAY)

using namespace std; // For memcmp, memcpy and memset.

class test_random_access_device
{
public:
  typedef asio_sockio::io_context::executor_type executor_type;

  test_random_access_device(asio_sockio::io_context& io_context)
    : io_context_(io_context),
      length_(max_length),
      next_write_length_(max_length)
  {
    memset(data_, 0, max_length);
  }

  executor_type get_executor() ASIO_NOEXCEPT
  {
    return io_context_.get_executor();
  }

  void reset()
  {
    memset(data_, 0, max_length);
    next_write_length_ = max_length;
  }

  void next_write_length(size_t length)
  {
    next_write_length_ = length;
  }

  template <typename Iterator>
  bool check_buffers(asio_sockio::uint64_t offset,
      Iterator begin, Iterator end, size_t length)
  {
    if (offset + length > max_length)
      return false;

    Iterator iter = begin;
    size_t checked_length = 0;
    for (; iter != end && checked_length < length; ++iter)
    {
      size_t buffer_length = asio_sockio::buffer_size(*iter);
      if (buffer_length > length - checked_length)
        buffer_length = length - checked_length;
      if (memcmp(data_ + offset + checked_length,
            iter->data(), buffer_length) != 0)
        return false;
      checked_length += buffer_length;
    }

    return true;
  }

  template <typename Const_Buffers>
  bool check_buffers(asio_sockio::uint64_t offset,
      const Const_Buffers& buffers, size_t length)
  {
    return check_buffers(offset, asio_sockio::buffer_sequence_begin(buffers),
        asio_sockio::buffer_sequence_end(buffers), length);
  }

  template <typename Const_Buffers>
  size_t write_some_at(asio_sockio::uint64_t offset,
      const Const_Buffers& buffers)
  {
    return asio_sockio::buffer_copy(
        asio_sockio::buffer(data_, length_) + offset,
        buffers, next_write_length_);
  }

  template <typename Const_Buffers>
  size_t write_some_at(asio_sockio::uint64_t offset,
      const Const_Buffers& buffers, asio_sockio::error_code& ec)
  {
    ec = asio_sockio::error_code();
    return write_some_at(offset, buffers);
  }

  template <typename Const_Buffers, typename Handler>
  void async_write_some_at(asio_sockio::uint64_t offset,
      const Const_Buffers& buffers, Handler handler)
  {
    size_t bytes_transferred = write_some_at(offset, buffers);
    asio_sockio::post(get_executor(),
        asio_sockio::detail::bind_handler(
          ASIO_MOVE_CAST(Handler)(handler),
          asio_sockio::error_code(), bytes_transferred));
  }

private:
  asio_sockio::io_context& io_context_;
  enum { max_length = 8192 };
  char data_[max_length];
  size_t length_;
  size_t next_write_length_;
};

static const char write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char mutable_write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void test_3_arg_const_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_3_arg_mutable_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::mutable_buffer buffers
    = asio_sockio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
}

void test_3_arg_vector_buffers_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<asio_sockio::const_buffer> buffers;
  buffers.push_back(asio_sockio::buffer(write_data, 32));
  buffers.push_back(asio_sockio::buffer(write_data) + 32);

  s.reset();
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_4_arg_nothrow_const_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  asio_sockio::error_code error;
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void test_4_arg_nothrow_mutable_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::mutable_buffer buffers
    = asio_sockio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  asio_sockio::error_code error;
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);
}

void test_4_arg_nothrow_vector_buffers_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<asio_sockio::const_buffer> buffers;
  buffers.push_back(asio_sockio::buffer(write_data, 32));
  buffers.push_back(asio_sockio::buffer(write_data) + 32);

  s.reset();
  asio_sockio::error_code error;
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

bool old_style_transfer_all(const asio_sockio::error_code& ec,
    size_t /*bytes_transferred*/)
{
  return !!ec;
}

size_t short_transfer(const asio_sockio::error_code& ec,
    size_t /*bytes_transferred*/)
{
  return !!ec ? 0 : 3;
}

void test_4_arg_const_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_4_arg_mutable_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::mutable_buffer buffers
    = asio_sockio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
}

void test_4_arg_vector_buffers_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<asio_sockio::const_buffer> buffers;
  buffers.push_back(asio_sockio::buffer(write_data, 32));
  buffers.push_back(asio_sockio::buffer(write_data) + 32);

  s.reset();
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 0, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers, short_transfer);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_5_arg_const_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  asio_sockio::error_code error;
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void test_5_arg_mutable_buffer_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::mutable_buffer buffers
    = asio_sockio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  asio_sockio::error_code error;
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);
}

void test_5_arg_vector_buffers_write_at()
{
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<asio_sockio::const_buffer> buffers;
  buffers.push_back(asio_sockio::buffer(write_data, 32));
  buffers.push_back(asio_sockio::buffer(write_data) + 32);

  s.reset();
  asio_sockio::error_code error;
  size_t bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 0, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio_sockio::error_code();
  bytes_transferred = asio_sockio::write_at(s, 1234, buffers,
      short_transfer, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void async_write_handler(const asio_sockio::error_code& e,
    size_t bytes_transferred, size_t expected_bytes_transferred, bool* called)
{
  *called = true;
  ASIO_CHECK(!e);
  ASIO_CHECK(bytes_transferred == expected_bytes_transferred);
}

void test_4_arg_const_buffer_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_4_arg_mutable_buffer_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::mutable_buffer buffers
    = asio_sockio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_4_arg_boost_array_buffers_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

#if defined(ASIO_HAS_BOOST_ARRAY)
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  boost::array<asio_sockio::const_buffer, 2> buffers = { {
    asio_sockio::buffer(write_data, 32),
    asio_sockio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
#endif // defined(ASIO_HAS_BOOST_ARRAY)
}

void test_4_arg_std_array_buffers_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

#if defined(ASIO_HAS_STD_ARRAY)
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::array<asio_sockio::const_buffer, 2> buffers = { {
    asio_sockio::buffer(write_data, 32),
    asio_sockio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
#endif // defined(ASIO_HAS_STD_ARRAY)
}

void test_4_arg_vector_buffers_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<asio_sockio::const_buffer> buffers;
  buffers.push_back(asio_sockio::buffer(write_data, 32));
  buffers.push_back(asio_sockio::buffer(write_data) + 32);

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_4_arg_streambuf_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::streambuf sb;
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  bool called = false;
  asio_sockio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  int i = asio_sockio::async_write_at(s, 0, sb,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_5_arg_const_buffer_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_5_arg_mutable_buffer_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::mutable_buffer buffers
    = asio_sockio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_5_arg_boost_array_buffers_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

#if defined(ASIO_HAS_BOOST_ARRAY)
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  boost::array<asio_sockio::const_buffer, 2> buffers = { {
    asio_sockio::buffer(write_data, 32),
    asio_sockio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
#endif // defined(ASIO_HAS_BOOST_ARRAY)
}

void test_5_arg_std_array_buffers_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

#if defined(ASIO_HAS_STD_ARRAY)
  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::array<asio_sockio::const_buffer, 2> buffers = { {
    asio_sockio::buffer(write_data, 32),
    asio_sockio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
#endif // defined(ASIO_HAS_STD_ARRAY)
}

void test_5_arg_vector_buffers_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<asio_sockio::const_buffer> buffers;
  buffers.push_back(asio_sockio::buffer(write_data, 32));
  buffers.push_back(asio_sockio::buffer(write_data) + 32);

  s.reset();
  bool called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, buffers, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  int i = asio_sockio::async_write_at(s, 0, buffers, short_transfer,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_5_arg_streambuf_async_write_at()
{
#if defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
  namespace bindns = std;
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  test_random_access_device s(ioc);
  asio_sockio::streambuf sb;
  asio_sockio::const_buffer buffers
    = asio_sockio::buffer(write_data, sizeof(write_data));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  bool called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb,
      asio_sockio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 0, sb, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  asio_sockio::async_write_at(s, 1234, sb, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 0, sb, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 0, sb, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  asio_sockio::async_write_at(s, 1234, sb, short_transfer,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  int i = asio_sockio::async_write_at(s, 0, sb, short_transfer,
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

ASIO_TEST_SUITE
(
  "write_at",
  ASIO_TEST_CASE(test_3_arg_const_buffer_write_at)
  ASIO_TEST_CASE(test_3_arg_mutable_buffer_write_at)
  ASIO_TEST_CASE(test_3_arg_vector_buffers_write_at)
  ASIO_TEST_CASE(test_4_arg_nothrow_const_buffer_write_at)
  ASIO_TEST_CASE(test_4_arg_nothrow_mutable_buffer_write_at)
  ASIO_TEST_CASE(test_4_arg_nothrow_vector_buffers_write_at)
  ASIO_TEST_CASE(test_4_arg_const_buffer_write_at)
  ASIO_TEST_CASE(test_4_arg_mutable_buffer_write_at)
  ASIO_TEST_CASE(test_4_arg_vector_buffers_write_at)
  ASIO_TEST_CASE(test_5_arg_const_buffer_write_at)
  ASIO_TEST_CASE(test_5_arg_mutable_buffer_write_at)
  ASIO_TEST_CASE(test_5_arg_vector_buffers_write_at)
  ASIO_TEST_CASE(test_4_arg_const_buffer_async_write_at)
  ASIO_TEST_CASE(test_4_arg_mutable_buffer_async_write_at)
  ASIO_TEST_CASE(test_4_arg_boost_array_buffers_async_write_at)
  ASIO_TEST_CASE(test_4_arg_std_array_buffers_async_write_at)
  ASIO_TEST_CASE(test_4_arg_vector_buffers_async_write_at)
  ASIO_TEST_CASE(test_4_arg_streambuf_async_write_at)
  ASIO_TEST_CASE(test_5_arg_const_buffer_async_write_at)
  ASIO_TEST_CASE(test_5_arg_mutable_buffer_async_write_at)
  ASIO_TEST_CASE(test_5_arg_boost_array_buffers_async_write_at)
  ASIO_TEST_CASE(test_5_arg_std_array_buffers_async_write_at)
  ASIO_TEST_CASE(test_5_arg_vector_buffers_async_write_at)
  ASIO_TEST_CASE(test_5_arg_streambuf_async_write_at)
)
