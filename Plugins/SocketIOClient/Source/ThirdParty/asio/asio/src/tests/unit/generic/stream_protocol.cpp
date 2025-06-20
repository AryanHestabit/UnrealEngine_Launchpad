//
// generic/stream_protocol.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#include "asio/generic/stream_protocol.hpp"

#include <cstring>
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "../unit_test.hpp"

#if defined(__cplusplus_cli) || defined(__cplusplus_winrt)
# define generic cpp_generic
#endif

//------------------------------------------------------------------------------

// generic_stream_protocol_socket_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// generic::stream_protocol::socket compile and link correctly. Runtime
// failures are ignored.

namespace generic_stream_protocol_socket_compile {

void connect_handler(const asio_sockio::error_code&)
{
}

void send_handler(const asio_sockio::error_code&, std::size_t)
{
}

void receive_handler(const asio_sockio::error_code&, std::size_t)
{
}

void write_some_handler(const asio_sockio::error_code&, std::size_t)
{
}

void read_some_handler(const asio_sockio::error_code&, std::size_t)
{
}

void test()
{
  using namespace asio_sockio;
  namespace generic = asio_sockio::generic;
  typedef generic::stream_protocol sp;

  const int af_inet = ASIO_OS_DEF(AF_INET);
  const int ipproto_tcp = ASIO_OS_DEF(IPPROTO_TCP);
  const int sock_stream = ASIO_OS_DEF(SOCK_STREAM);

  try
  {
    io_context ioc;
    char mutable_char_buffer[128] = "";
    const char const_char_buffer[128] = "";
    socket_base::message_flags in_flags = 0;
    socket_base::keep_alive socket_option;
    socket_base::bytes_readable io_control_command;
    asio_sockio::error_code ec;

    // basic_stream_socket constructors.

    sp::socket socket1(ioc);
    sp::socket socket2(ioc, sp(af_inet, ipproto_tcp));
    sp::socket socket3(ioc, sp::endpoint());
#if defined(ASIO_WINDOWS_RUNTIME)
    Windows::Networking::Sockets::StreamSocket^ native_socket1 = nullptr;
#else // defined(ASIO_WINDOWS_RUNTIME)
    sp::socket::native_handle_type native_socket1
      = ::socket(af_inet, sock_stream, 0);
#endif // defined(ASIO_WINDOWS_RUNTIME)
    sp::socket socket4(ioc, sp(af_inet, ipproto_tcp), native_socket1);

#if defined(ASIO_HAS_MOVE)
    sp::socket socket5(std::move(socket4));
    asio_sockio::ip::tcp::socket tcp_socket(ioc);
    sp::socket socket6(std::move(tcp_socket));
#endif // defined(ASIO_HAS_MOVE)

    // basic_stream_socket operators.

#if defined(ASIO_HAS_MOVE)
    socket1 = sp::socket(ioc);
    socket1 = std::move(socket2);
    socket1 = asio_sockio::ip::tcp::socket(ioc);
#endif // defined(ASIO_HAS_MOVE)

    // basic_io_object functions.

    sp::socket::executor_type ex = socket1.get_executor();
    (void)ex;

#if !defined(ASIO_NO_DEPRECATED)
    io_context& ioc_ref = socket1.get_io_context();
    (void)ioc_ref;

    io_context& ioc_ref2 = socket1.get_io_service();
    (void)ioc_ref2;
#endif // !defined(ASIO_NO_DEPRECATED)

    // basic_socket functions.

    sp::socket::lowest_layer_type& lowest_layer = socket1.lowest_layer();
    (void)lowest_layer;

    socket1.open(sp(af_inet, ipproto_tcp));
    socket1.open(sp(af_inet, ipproto_tcp), ec);

#if defined(ASIO_WINDOWS_RUNTIME)
    Windows::Networking::Sockets::StreamSocket^ native_socket2 = nullptr;
#else // defined(ASIO_WINDOWS_RUNTIME)
    sp::socket::native_handle_type native_socket2
      = ::socket(af_inet, sock_stream, 0);
#endif // defined(ASIO_WINDOWS_RUNTIME)
    socket1.assign(sp(af_inet, ipproto_tcp), native_socket2);
#if defined(ASIO_WINDOWS_RUNTIME)
    Windows::Networking::Sockets::StreamSocket^ native_socket3 = nullptr;
#else // defined(ASIO_WINDOWS_RUNTIME)
    sp::socket::native_handle_type native_socket3
      = ::socket(af_inet, sock_stream, 0);
#endif // defined(ASIO_WINDOWS_RUNTIME)
    socket1.assign(sp(af_inet, ipproto_tcp), native_socket3, ec);

    bool is_open = socket1.is_open();
    (void)is_open;

    socket1.close();
    socket1.close(ec);

    sp::socket::native_handle_type native_socket4 = socket1.native_handle();
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

    socket1.bind(sp::endpoint());
    socket1.bind(sp::endpoint(), ec);

    socket1.connect(sp::endpoint());
    socket1.connect(sp::endpoint(), ec);

    socket1.async_connect(sp::endpoint(), connect_handler);

    socket1.set_option(socket_option);
    socket1.set_option(socket_option, ec);

    socket1.get_option(socket_option);
    socket1.get_option(socket_option, ec);

    socket1.io_control(io_control_command);
    socket1.io_control(io_control_command, ec);

    sp::endpoint endpoint1 = socket1.local_endpoint();
    sp::endpoint endpoint2 = socket1.local_endpoint(ec);

    sp::endpoint endpoint3 = socket1.remote_endpoint();
    sp::endpoint endpoint4 = socket1.remote_endpoint(ec);

    socket1.shutdown(socket_base::shutdown_both);
    socket1.shutdown(socket_base::shutdown_both, ec);

    // basic_stream_socket functions.

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

    socket1.write_some(buffer(mutable_char_buffer));
    socket1.write_some(buffer(const_char_buffer));
    socket1.write_some(null_buffers());
    socket1.write_some(buffer(mutable_char_buffer), ec);
    socket1.write_some(buffer(const_char_buffer), ec);
    socket1.write_some(null_buffers(), ec);

    socket1.async_write_some(buffer(mutable_char_buffer), write_some_handler);
    socket1.async_write_some(buffer(const_char_buffer), write_some_handler);
    socket1.async_write_some(null_buffers(), write_some_handler);

    socket1.read_some(buffer(mutable_char_buffer));
    socket1.read_some(buffer(mutable_char_buffer), ec);
    socket1.read_some(null_buffers(), ec);

    socket1.async_read_some(buffer(mutable_char_buffer), read_some_handler);
    socket1.async_read_some(null_buffers(), read_some_handler);
  }
  catch (std::exception&)
  {
  }
}

} // namespace generic_stream_protocol_socket_compile

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "generic/stream_protocol",
  ASIO_TEST_CASE(generic_stream_protocol_socket_compile::test)
)
