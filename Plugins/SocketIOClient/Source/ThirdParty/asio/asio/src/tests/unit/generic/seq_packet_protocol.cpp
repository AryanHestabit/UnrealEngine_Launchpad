//
// generic/seq_packet_protocol.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#include "asio/generic/seq_packet_protocol.hpp"

#include <cstring>
#include "asio/io_context.hpp"
#include "../unit_test.hpp"

#if defined(__cplusplus_cli) || defined(__cplusplus_winrt)
# define generic cpp_generic
#endif

//------------------------------------------------------------------------------

// generic_seq_packet_protocol_socket_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// generic::seq_packet_socket::socket compile and link correctly. Runtime
// failures are ignored.

namespace generic_seq_packet_protocol_socket_compile {

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
  typedef generic::seq_packet_protocol spp;

  const int af_inet = ASIO_OS_DEF(AF_INET);
  const int sock_seqpacket = ASIO_OS_DEF(SOCK_SEQPACKET);

  try
  {
    io_context ioc;
    char mutable_char_buffer[128] = "";
    const char const_char_buffer[128] = "";
    const socket_base::message_flags in_flags = 0;
    socket_base::message_flags out_flags = 0;
    socket_base::send_buffer_size socket_option;
    socket_base::bytes_readable io_control_command;
    asio_sockio::error_code ec;

    // basic_seq_packet_socket constructors.

    spp::socket socket1(ioc);
    spp::socket socket2(ioc, spp(af_inet, 0));
    spp::socket socket3(ioc, spp::endpoint());
#if !defined(ASIO_WINDOWS_RUNTIME)
    spp::socket::native_handle_type native_socket1
      = ::socket(af_inet, sock_seqpacket, 0);
    spp::socket socket4(ioc, spp(af_inet, 0), native_socket1);
#endif // !defined(ASIO_WINDOWS_RUNTIME)

#if defined(ASIO_HAS_MOVE)
    spp::socket socket5(std::move(socket4));
#endif // defined(ASIO_HAS_MOVE)

    // basic_seq_packet_socket operators.

#if defined(ASIO_HAS_MOVE)
    socket1 = spp::socket(ioc);
    socket1 = std::move(socket2);
#endif // defined(ASIO_HAS_MOVE)

    // basic_io_object functions.

    spp::socket::executor_type ex = socket1.get_executor();
    (void)ex;

#if !defined(ASIO_NO_DEPRECATED)
    io_context& ioc_ref = socket1.get_io_context();
    (void)ioc_ref;

    io_context& ioc_ref2 = socket1.get_io_service();
    (void)ioc_ref2;
#endif // !defined(ASIO_NO_DEPRECATED)

    // basic_socket functions.

    spp::socket::lowest_layer_type& lowest_layer = socket1.lowest_layer();
    (void)lowest_layer;

    socket1.open(spp(af_inet, 0));
    socket1.open(spp(af_inet, 0), ec);

#if !defined(ASIO_WINDOWS_RUNTIME)
    spp::socket::native_handle_type native_socket2
      = ::socket(af_inet, sock_seqpacket, 0);
    socket1.assign(spp(af_inet, 0), native_socket2);
    spp::socket::native_handle_type native_socket3
      = ::socket(af_inet, sock_seqpacket, 0);
    socket1.assign(spp(af_inet, 0), native_socket3, ec);
#endif // !defined(ASIO_WINDOWS_RUNTIME)

    bool is_open = socket1.is_open();
    (void)is_open;

    socket1.close();
    socket1.close(ec);

    spp::socket::native_handle_type native_socket4 = socket1.native_handle();
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

    socket1.bind(spp::endpoint());
    socket1.bind(spp::endpoint(), ec);

    socket1.connect(spp::endpoint());
    socket1.connect(spp::endpoint(), ec);

    socket1.async_connect(spp::endpoint(), connect_handler);

    socket1.set_option(socket_option);
    socket1.set_option(socket_option, ec);

    socket1.get_option(socket_option);
    socket1.get_option(socket_option, ec);

    socket1.io_control(io_control_command);
    socket1.io_control(io_control_command, ec);

    spp::endpoint endpoint1 = socket1.local_endpoint();
    spp::endpoint endpoint2 = socket1.local_endpoint(ec);

    spp::endpoint endpoint3 = socket1.remote_endpoint();
    spp::endpoint endpoint4 = socket1.remote_endpoint(ec);

    socket1.shutdown(socket_base::shutdown_both);
    socket1.shutdown(socket_base::shutdown_both, ec);

    // basic_seq_packet_socket functions.

    socket1.send(buffer(mutable_char_buffer), in_flags);
    socket1.send(buffer(const_char_buffer), in_flags);
    socket1.send(null_buffers(), in_flags);
    socket1.send(buffer(mutable_char_buffer), in_flags, ec);
    socket1.send(buffer(const_char_buffer), in_flags, ec);
    socket1.send(null_buffers(), in_flags, ec);

    socket1.async_send(buffer(mutable_char_buffer), in_flags, send_handler);
    socket1.async_send(buffer(const_char_buffer), in_flags, send_handler);
    socket1.async_send(null_buffers(), in_flags, send_handler);

    socket1.receive(buffer(mutable_char_buffer), out_flags);
    socket1.receive(null_buffers(), out_flags);
    socket1.receive(buffer(mutable_char_buffer), in_flags, out_flags);
    socket1.receive(null_buffers(), in_flags, out_flags);
    socket1.receive(buffer(mutable_char_buffer), in_flags, out_flags, ec);
    socket1.receive(null_buffers(), in_flags, out_flags, ec);

    socket1.async_receive(buffer(mutable_char_buffer), out_flags,
        receive_handler);
    socket1.async_receive(null_buffers(), out_flags, receive_handler);
    socket1.async_receive(buffer(mutable_char_buffer), in_flags,
        out_flags, receive_handler);
    socket1.async_receive(null_buffers(), in_flags, out_flags, receive_handler);
  }
  catch (std::exception&)
  {
  }
}

} // namespace generic_seq_packet_protocol_socket_compile

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "generic/seq_packet_protocol",
  ASIO_TEST_CASE(generic_seq_packet_protocol_socket_compile::test)
)
