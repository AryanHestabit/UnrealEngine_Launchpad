//
// buffer.cpp
// ~~~~~~~~~~
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
#include "asio/buffer.hpp"

#include "unit_test.hpp"

#if defined(ASIO_HAS_BOOST_ARRAY)
# include <boost/array.hpp>
#endif // defined(ASIO_HAS_BOOST_ARRAY)

#if defined(ASIO_HAS_STD_ARRAY)
# include <array>
#endif // defined(ASIO_HAS_STD_ARRAY)

//------------------------------------------------------------------------------

// buffer_compile test
// ~~~~~~~~~~~~~~~~~~~
// The following test checks that all overloads of the buffer function compile
// and link correctly. Runtime failures are ignored.

namespace buffer_compile {

using namespace asio_sockio;

void test()
{
  try
  {
    char raw_data[1024];
    const char const_raw_data[1024] = "";
    void* void_ptr_data = raw_data;
    const void* const_void_ptr_data = const_raw_data;
#if defined(ASIO_HAS_BOOST_ARRAY)
    boost::array<char, 1024> array_data;
    const boost::array<char, 1024>& const_array_data_1 = array_data;
    boost::array<const char, 1024> const_array_data_2 = { { 0 } };
#endif // defined(ASIO_HAS_BOOST_ARRAY)
#if defined(ASIO_HAS_STD_ARRAY)
    std::array<char, 1024> std_array_data;
    const std::array<char, 1024>& const_std_array_data_1 = std_array_data;
    std::array<const char, 1024> const_std_array_data_2 = { { 0 } };
#endif // defined(ASIO_HAS_STD_ARRAY)
    std::vector<char> vector_data(1024);
    const std::vector<char>& const_vector_data = vector_data;
    std::string string_data(1024, ' ');
    const std::string const_string_data(1024, ' ');
    std::vector<mutable_buffer> mutable_buffer_sequence;
    std::vector<const_buffer> const_buffer_sequence;
#if defined(ASIO_HAS_STD_STRING_VIEW)
    std::string_view string_view_data(string_data);
#elif defined(ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    std::experimental::string_view string_view_data(string_data);
#endif // defined(ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)

    // mutable_buffer constructors.

    mutable_buffer mb1;
    mutable_buffer mb2(void_ptr_data, 1024);
    mutable_buffer mb3(mb1);

    // mutable_buffer functions.

    void* ptr1 = mb1.data();
    (void)ptr1;

    std::size_t n1 = mb1.size();
    (void)n1;

    // mutable_buffer operators.

    mb1 += 128;
    mb1 = mb2 + 128;
    mb1 = 128 + mb2;

#if !defined(ASIO_NO_DEPRECATED)

    // mutable_buffers_1 constructors.

    mutable_buffers_1 mbc1(mb1);
    mutable_buffers_1 mbc2(mbc1);

    // mutable_buffers_1 functions.

    mutable_buffers_1::const_iterator iter1 = mbc1.begin();
    (void)iter1;
    mutable_buffers_1::const_iterator iter2 = mbc1.end();
    (void)iter2;

#endif // !defined(ASIO_NO_DEPRECATED)

    // const_buffer constructors.

    const_buffer cb1;
    const_buffer cb2(const_void_ptr_data, 1024);
    const_buffer cb3(cb1);
    const_buffer cb4(mb1);

    // const_buffer functions.

    const void* ptr2 = cb1.data();
    (void)ptr2;

    std::size_t n2 = cb1.size();
    (void)n2;

    // const_buffer operators.

    cb1 += 128;
    cb1 = cb2 + 128;
    cb1 = 128 + cb2;

#if !defined(ASIO_NO_DEPRECATED)

    // const_buffers_1 constructors.

    const_buffers_1 cbc1(cb1);
    const_buffers_1 cbc2(cbc1);

    // const_buffers_1 functions.

    const_buffers_1::const_iterator iter3 = cbc1.begin();
    (void)iter3;
    const_buffers_1::const_iterator iter4 = cbc1.end();
    (void)iter4;

#endif // !defined(ASIO_NO_DEPRECATED)

    // buffer_size function overloads.

    std::size_t size1 = buffer_size(mb1);
    (void)size1;
    std::size_t size2 = buffer_size(cb1);
    (void)size2;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size3 = buffer_size(mbc1);
    (void)size3;
    std::size_t size4 = buffer_size(cbc1);
    (void)size4;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size5 = buffer_size(mutable_buffer_sequence);
    (void)size5;
    std::size_t size6 = buffer_size(const_buffer_sequence);
    (void)size6;

    // buffer_cast function overloads.

#if !defined(ASIO_NO_DEPRECATED)
    void* ptr3 = buffer_cast<void*>(mb1);
    (void)ptr3;
    const void* ptr4 = buffer_cast<const void*>(cb1);
    (void)ptr4;
#endif // !defined(ASIO_NO_DEPRECATED)

    // buffer function overloads.

    mb1 = buffer(mb2);
    mb1 = buffer(mb2, 128);
    cb1 = buffer(cb2);
    cb1 = buffer(cb2, 128);
    mb1 = buffer(void_ptr_data, 1024);
    cb1 = buffer(const_void_ptr_data, 1024);
    mb1 = buffer(raw_data);
    mb1 = buffer(raw_data, 1024);
    cb1 = buffer(const_raw_data);
    cb1 = buffer(const_raw_data, 1024);
#if defined(ASIO_HAS_BOOST_ARRAY)
    mb1 = buffer(array_data);
    mb1 = buffer(array_data, 1024);
    cb1 = buffer(const_array_data_1);
    cb1 = buffer(const_array_data_1, 1024);
    cb1 = buffer(const_array_data_2);
    cb1 = buffer(const_array_data_2, 1024);
#endif // defined(ASIO_HAS_BOOST_ARRAY)
#if defined(ASIO_HAS_STD_ARRAY)
    mb1 = buffer(std_array_data);
    mb1 = buffer(std_array_data, 1024);
    cb1 = buffer(const_std_array_data_1);
    cb1 = buffer(const_std_array_data_1, 1024);
    cb1 = buffer(const_std_array_data_2);
    cb1 = buffer(const_std_array_data_2, 1024);
#endif // defined(ASIO_HAS_STD_ARRAY)
    mb1 = buffer(vector_data);
    mb1 = buffer(vector_data, 1024);
    cb1 = buffer(const_vector_data);
    cb1 = buffer(const_vector_data, 1024);
    mb1 = buffer(string_data);
    mb1 = buffer(string_data, 1024);
    cb1 = buffer(const_string_data);
    cb1 = buffer(const_string_data, 1024);
#if defined(ASIO_HAS_STRING_VIEW)
    cb1 = buffer(string_view_data);
    cb1 = buffer(string_view_data, 1024);
#endif // defined(ASIO_HAS_STRING_VIEW)

    // buffer_copy function overloads.

    std::size_t size7 = buffer_copy(mb1, cb2);
    (void)size7;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size8 = buffer_copy(mb1, cbc2);
    (void)size8;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size9 = buffer_copy(mb1, mb2);
    (void)size9;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size10 = buffer_copy(mb1, mbc2);
    (void)size10;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size11 = buffer_copy(mb1, const_buffer_sequence);
    (void)size11;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size12 = buffer_copy(mbc1, cb2);
    (void)size12;
    std::size_t size13 = buffer_copy(mbc1, cbc2);
    (void)size13;
    std::size_t size14 = buffer_copy(mbc1, mb2);
    (void)size14;
    std::size_t size15 = buffer_copy(mbc1, mbc2);
    (void)size15;
    std::size_t size16 = buffer_copy(mbc1, const_buffer_sequence);
    (void)size16;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size17 = buffer_copy(mutable_buffer_sequence, cb2);
    (void)size17;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size18 = buffer_copy(mutable_buffer_sequence, cbc2);
    (void)size18;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size19 = buffer_copy(mutable_buffer_sequence, mb2);
    (void)size19;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size20 = buffer_copy(mutable_buffer_sequence, mbc2);
    (void)size20;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size21 = buffer_copy(
        mutable_buffer_sequence, const_buffer_sequence);
    (void)size21;
    std::size_t size22 = buffer_copy(mb1, cb2, 128);
    (void)size22;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size23 = buffer_copy(mb1, cbc2, 128);
    (void)size23;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size24 = buffer_copy(mb1, mb2, 128);
    (void)size24;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size25 = buffer_copy(mb1, mbc2, 128);
    (void)size25;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size26 = buffer_copy(mb1, const_buffer_sequence, 128);
    (void)size26;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size27 = buffer_copy(mbc1, cb2, 128);
    (void)size27;
    std::size_t size28 = buffer_copy(mbc1, cbc2, 128);
    (void)size28;
    std::size_t size29 = buffer_copy(mbc1, mb2, 128);
    (void)size29;
    std::size_t size30 = buffer_copy(mbc1, mbc2, 128);
    (void)size30;
    std::size_t size31 = buffer_copy(mbc1, const_buffer_sequence, 128);
    (void)size31;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size32 = buffer_copy(mutable_buffer_sequence, cb2, 128);
    (void)size32;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size33 = buffer_copy(mutable_buffer_sequence, cbc2, 128);
    (void)size33;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size34 = buffer_copy(mutable_buffer_sequence, mb2, 128);
    (void)size34;
#if !defined(ASIO_NO_DEPRECATED)
    std::size_t size35 = buffer_copy(mutable_buffer_sequence, mbc2, 128);
    (void)size35;
#endif // !defined(ASIO_NO_DEPRECATED)
    std::size_t size36 = buffer_copy(
        mutable_buffer_sequence, const_buffer_sequence, 128);
    (void)size36;

    // dynamic_buffer function overloads.

    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type> db1 = dynamic_buffer(string_data);
    (void)db1;
    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type> db2 = dynamic_buffer(string_data, 1024);
    (void)db2;
    dynamic_vector_buffer<char, std::allocator<char> >
      db3 = dynamic_buffer(vector_data);
    (void)db3;
    dynamic_vector_buffer<char, std::allocator<char> >
      db4 = dynamic_buffer(vector_data, 1024);
    (void)db4;

    // dynamic_buffer member functions.

    std::size_t size37 = db1.size();
    (void)size37;
    std::size_t size38 = db3.size();
    (void)size38;

    std::size_t size39 = db1.max_size();
    (void)size39;
    std::size_t size40 = db3.max_size();
    (void)size40;

    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type>::const_buffers_type
        cb5 = db1.data();
    (void)cb5;
    dynamic_vector_buffer<char, std::allocator<char> >::const_buffers_type
      cb6 = db3.data();
    (void)cb6;

    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type>::mutable_buffers_type mb5
        = db1.prepare(1024);
    (void)mb5;
    dynamic_vector_buffer<char, std::allocator<char> >::mutable_buffers_type
      mb6 = db3.prepare(1024);
    (void)mb6;

    db1.commit(1024);
    db3.commit(1024);

    db1.consume(1024);
    db3.consume(1024);
  }
  catch (std::exception&)
  {
  }
}

} // namespace buffer_compile

//------------------------------------------------------------------------------

namespace buffer_copy_runtime {

using namespace asio_sockio;
using namespace std;

void test()
{
  char dest_data[256];
  char source_data[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  memset(dest_data, 0, sizeof(dest_data));
  mutable_buffer mb1 = asio_sockio::buffer(dest_data);
  mutable_buffer mb2 = asio_sockio::buffer(source_data);
  std::size_t n = buffer_copy(mb1, mb2);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  const_buffer cb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mb1, cb1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

#if !defined(ASIO_NO_DEPRECATED)
  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  mutable_buffers_1 mbc1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mb1, mbc1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  const_buffers_1 cbc1 = const_buffers_1(asio_sockio::buffer(source_data));
  n = buffer_copy(mb1, cbc1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  mb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mbc1, mb1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  cb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mbc1, cb1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  mutable_buffers_1 mbc2 = asio_sockio::buffer(source_data);
  n = buffer_copy(mbc1, mbc2);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  cbc1 = const_buffers_1(asio_sockio::buffer(source_data));
  n = buffer_copy(mbc1, cbc1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);
#endif // !defined(ASIO_NO_DEPRECATED)

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  std::vector<mutable_buffer> mv1;
  mv1.push_back(asio_sockio::buffer(source_data, 5));
  mv1.push_back(asio_sockio::buffer(source_data) + 5);
  n = buffer_copy(mb1, mv1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  std::vector<const_buffer> cv1;
  cv1.push_back(asio_sockio::buffer(source_data, 6));
  cv1.push_back(asio_sockio::buffer(source_data) + 6);
  n = buffer_copy(mb1, cv1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio_sockio::buffer(dest_data, 7));
  mv1.push_back(asio_sockio::buffer(dest_data) + 7);
  cb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mv1, cb1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio_sockio::buffer(dest_data, 7));
  mv1.push_back(asio_sockio::buffer(dest_data) + 7);
  cv1.clear();
  cv1.push_back(asio_sockio::buffer(source_data, 8));
  cv1.push_back(asio_sockio::buffer(source_data) + 8);
  n = buffer_copy(mv1, cv1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  mb2 = asio_sockio::buffer(source_data);
  n = buffer_copy(mb1, mb2, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  cb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mb1, cb1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

#if !defined(ASIO_NO_DEPRECATED)
  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  mbc1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mb1, mbc1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  cbc1 = const_buffers_1(asio_sockio::buffer(source_data));
  n = buffer_copy(mb1, cbc1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  mb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mbc1, mb1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  cb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mbc1, cb1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  mbc2 = asio_sockio::buffer(source_data);
  n = buffer_copy(mbc1, mbc2, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mbc1 = asio_sockio::buffer(dest_data);
  cbc1 = const_buffers_1(asio_sockio::buffer(source_data));
  n = buffer_copy(mbc1, cbc1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);
#endif // !defined(ASIO_NO_DEPRECATED)

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  mv1.clear();
  mv1.push_back(asio_sockio::buffer(source_data, 5));
  mv1.push_back(asio_sockio::buffer(source_data) + 5);
  n = buffer_copy(mb1, mv1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio_sockio::buffer(dest_data);
  cv1.clear();
  cv1.push_back(asio_sockio::buffer(source_data, 6));
  cv1.push_back(asio_sockio::buffer(source_data) + 6);
  n = buffer_copy(mb1, cv1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio_sockio::buffer(dest_data, 7));
  mv1.push_back(asio_sockio::buffer(dest_data) + 7);
  cb1 = asio_sockio::buffer(source_data);
  n = buffer_copy(mv1, cb1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio_sockio::buffer(dest_data, 7));
  mv1.push_back(asio_sockio::buffer(dest_data) + 7);
  cv1.clear();
  cv1.push_back(asio_sockio::buffer(source_data, 8));
  cv1.push_back(asio_sockio::buffer(source_data) + 8);
  n = buffer_copy(mv1, cv1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);
}

} // namespace buffer_copy_runtime

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "buffer",
  ASIO_COMPILE_TEST_CASE(buffer_compile::test)
  ASIO_TEST_CASE(buffer_copy_runtime::test)
)
