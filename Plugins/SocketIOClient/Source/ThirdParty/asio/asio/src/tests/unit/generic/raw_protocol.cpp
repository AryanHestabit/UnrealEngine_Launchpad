//
// generic/raw_protocol.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
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
#include "asio/generic/raw_protocol.hpp"

#include <cstring>
#include "asio/io_context.hpp"
#include "asio/ip/icmp.hpp"
#include "../unit_test.hpp"

#if defined(__cplusplus_cli) || defined(__cplusplus_winrt)
# define generic cpp_generic
#endif

//------------------------------------------------------------------------------

// generic_raw_protocol_socket_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// generic::raw_socket::socket compile and link correctly. Runtime failures
// are ignored.

namespace generic_raw_protocol_socket_compile {

void connect_handler(const asio_sockio::error_code&)
{
}

void send_handler(const asio_sockio::error_code&, std::size_t)
{
}

void receive_handler(const asio_sockio::error_code&, std::size_t)
{
}

void test()
{
  using namespace asio_sockio;
  namespace generic = asio_sockio::generic;
  typedef generic::raw_protocol rp;

  const int af_inet = ASIO_OS_DEF(AF_INET);
  const int ipproto_icmp = ASIO_OS_DEF(IPPROTO_ICMP);
  const int sock_raw = ASIO_OS_DEF(SOCK_RAW);

  try
  {
    io_context ioc;
    char mutable_char_buffer[128] = "";
    const char const_char_buffer[128] = "";
    socket_base::message_flags in_flags = 0;
    socket_base::send_buffer_size socket_option;
    socket_base::bytes_readable io_control_command;
    asio_sockio::error_code ec;

    // basic_raw_socket constructors.

    rp::socket socket1(ioc);
    rp::socket socket2(ioc, rp(af_inet, ipproto_icmp));
    rp::socket socket3(ioc, rp::endpoint());
#if !defined(ASIO_WINDOWS_RUNTIME)
    rp::socket::native_handle_type native_socket1
      = ::socket(af_inet, sock_raw, 0);
    rp::socket socket4(ioc, rp(af_inet, ipproto_icmp), native_socket1);
#endif // !defined(ASIO_WINDOWS_RUNTIME)

#if defined(ASIO_HAS_MOVE)
    rp::socket socket5(std::move(socket4));
    asio_sockio::ip::icmp::socket icmp_socket(ioc);
    rp::socket socket6(std::move(icmp_socket));
#endif // defined(ASIO_HAS_MOVE)

    // basic_datagram_socket operators.

#if defined(ASIO_HAS_MOVE)
    socket1 = rp::socket(ioc);
    socket1 = std::move(socket2);
    socket1 = asio_sockio::ip::icmp::socket(ioc);
#endif // defined(ASIO_HAS_MOVE)

    // basic_io_object functions.

    rp::socket::executor_type ex = socket1.get_executor();
    (void)ex;

#if !defined(ASIO_NO_DEPRECATED)
    io_context& ioc_ref = socket1.get_io_context();
    (void)ioc_ref;

    io_context& ioc_ref2 = socket1.get_io_service();
    (void)ioc_ref2;
#endif // !defined(ASIO_NO_DEPRECATED)

    // basic_socket functions.

    rp::socket::lowest_layer_type& lowest_layer = socket1.lowest_layer();
    (void)lowest_layer;

    socket1.open(rp(af_inet, ipproto_icmp));
    socket1.open(rp(af_inet, ipproto_icmp), ec);

#if !defined(ASIO_WINDOWS_RUNTIME)
    rp::socket::native_handle_type native_socket2
      = ::socket(af_inet, sock_raw, 0);
    socket1.assign(rp(af_inet, ipproto_icmp), native_socket2);
    rp::socket::native_handle_type native_socket3
      = ::socket(af_inet, sock_raw, 0);
    socket1.assign(rp(af_inet, ipproto_icmp), native_socket3, ec);
#endif // !defined(ASIO_WINDOWS_RUNTIME)

    bool is_open = socket1.is_open();
    (void)is_open;

    socket1.close();
    socket1.close(ec);

    rp::socket::native_handle_type native_socket4 = socket1.native_handle();
    (void)native_socket4;

    socket1.cancel();
    socket1.cancel(ec);

    bool at_mark1 = socket1.at_mark();
    (void)at_mark1;
    bool at_mark2 = socket1.at_mark(ec);
    (void)at_mark2;

    std::size_t available1 = socket1.available();
    (void)available1;
    std::size_t available2 = socket1.available(ec);
    (void)available2;

    socket1.bind(rp::endpoint());
    socket1.bind(rp::endpoint(), ec);

    socket1.connect(rp::endpoint());
    socket1.connect(rp::endpoint(), ec);

    socket1.async_connect(rp::endpoint(), connect_handler);

    socket1.set_option(socket_option);
    socket1.set_option(socket_option, ec);

    socket1.get_option(socket_option);
    socket1.get_option(socket_option, ec);

    socket1.io_control(io_control_command);
    socket1.io_control(io_control_command, ec);

    rp::endpoint endpoint1 = socket1.local_endpoint();
    rp::endpoint endpoint2 = socket1.local_endpoint(ec);

    rp::endpoint endpoint3 = socket1.remote_endpoint();
    rp::endpoint endpoint4 = socket1.remote_endpoint(ec);

    socket1.shutdown(socket_base::shutdown_both);
    socket1.shutdown(socket_base::shutdown_both, ec);

    // basic_raw_socket functions.

    socket1.send(buffer(mutable_char_buffer));
    socket1.send(buffer(const_char_buffer));
    socket1.send(null_buffers());
    socket1.send(buffer(mutable_char_buffer), in_flags);
    socket1.send(buffer(const_char_buffer), in_flags);
    socket1.send(null_buffers(), in_flags);
    socket1.send(buffer(mutable_char_buffer), in_flags, ec);
    socket1.send(buffer(const_char_buffer), in_flags, ec);
    socket1.send(null_buffers(), in_flags, ec);

    socket1.async_send(buffer(mutable_char_buffer), send_handler);
    socket1.async_send(buffer(const_char_buffer), send_handler);
    socket1.async_send(null_buffers(), send_handler);
    socket1.async_send(buffer(mutable_char_buffer), in_flags, send_handler);
    socket1.async_send(buffer(const_char_buffer), in_flags, send_handler);
    socket1.async_send(null_buffers(), in_flags, send_handler);

    socket1.send_to(buffer(mutable_char_buffer),
        rp::endpoint());
    socket1.send_to(buffer(const_char_buffer),
        rp::endpoint());
    socket1.send_to(null_buffers(),
        rp::endpoint());
    socket1.send_to(buffer(mutable_char_buffer),
        rp::endpoint(), in_flags);
    socket1.send_to(buffer(const_char_buffer),
        rp::endpoint(), in_flags);
    socket1.send_to(null_buffers(),
        rp::endpoint(), in_flags);
    socket1.send_to(buffer(mutable_char_buffer),
        rp::endpoint(), in_flags, ec);
    socket1.send_to(buffer(const_char_buffer),
        rp::endpoint(), in_flags, ec);
    socket1.send_to(null_buffers(),
        rp::endpoint(), in_flags, ec);

    socket1.async_send_to(buffer(mutable_char_buffer),
        rp::endpoint(), send_handler);
    socket1.async_send_to(buffer(const_char_buffer),
        rp::endpoint(), send_handler);
    socket1.async_send_to(null_buffers(),
        rp::endpoint(), send_handler);
    socket1.async_send_to(buffer(mutable_char_buffer),
        rp::endpoint(), in_flags, send_handler);
    socket1.async_send_to(buffer(const_char_buffer),
        rp::endpoint(), in_flags, send_handler);
    socket1.async_send_to(null_buffers(),
        rp::endpoint(), in_flags, send_handler);

    socket1.receive(buffer(mutable_char_buffer));
    socket1.receive(null_buffers());
    socket1.receive(buffer(mutable_char_buffer), in_flags);
    socket1.receive(null_buffers(), in_flags);
    socket1.receive(buffer(mutable_char_buffer), in_flags, ec);
    socket1.receive(null_buffers(), in_flags, ec);

    socket1.async_receive(buffer(mutable_char_buffer), receive_handler);
    socket1.async_receive(null_buffers(), receive_handler);
    socket1.async_receive(buffer(mutable_char_buffer), in_flags,
        receive_handler);
    socket1.async_receive(null_buffers(), in_flags, receive_handler);

    rp::endpoint endpoint;
    socket1.receive_from(buffer(mutable_char_buffer), endpoint);
    socket1.receive_from(null_buffers(), endpoint);
    socket1.receive_from(buffer(mutable_char_buffer), endpoint, in_flags);
    socket1.receive_from(null_buffers(), endpoint, in_flags);
    socket1.receive_from(buffer(mutable_char_buffer), endpoint, in_flags, ec);
    socket1.receive_from(null_buffers(), endpoint, in_flags, ec);

    socket1.async_receive_from(buffer(mutable_char_buffer),
        endpoint, receive_handler);
    socket1.async_receive_from(null_buffers(),
        endpoint, receive_handler);
    socket1.async_receive_from(buffer(mutable_char_buffer),
        endpoint, in_flags, receive_handler);
    socket1.async_receive_from(null_buffers(),
        endpoint, in_flags, receive_handler);
  }
  catch (std::exception&)
  {
  }
}

} // namespace generic_raw_protocol_socket_compile

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "generic/raw_protocol",
  ASIO_TEST_CASE(generic_raw_protocol_socket_compile::test)
)
