//
// serial_port.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Rep Invariant Systems, Inc. (info@repinvariant.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/serial_port.hpp"

#include "archetypes/async_result.hpp"
#include "asio/io_context.hpp"
#include "unit_test.hpp"

//------------------------------------------------------------------------------

// serial_port_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// serial_port compile and link correctly. Runtime failures are ignored.

namespace serial_port_compile {

struct write_some_handler
{
  write_some_handler() {}
  void operator()(const asio_sockio::error_code&, std::size_t) {}
#if defined(ASIO_HAS_MOVE)
  write_some_handler(write_some_handler&&) {}
private:
  write_some_handler(const write_some_handler&);
#endif // defined(ASIO_HAS_MOVE)
};

struct read_some_handler
{
  read_some_handler() {}
  void operator()(const asio_sockio::error_code&, std::size_t) {}
#if defined(ASIO_HAS_MOVE)
  read_some_handler(read_some_handler&&) {}
private:
  read_some_handler(const read_some_handler&);
#endif // defined(ASIO_HAS_MOVE)
};

void test()
{
#if defined(ASIO_HAS_SERIAL_PORT)
  using namespace asio_sockio;

  try
  {
    io_context ioc;
    char mutable_char_buffer[128] = "";
    const char const_char_buffer[128] = "";
    serial_port::baud_rate serial_port_option;
    archetypes::lazy_handler lazy;
    asio_sockio::error_code ec;

    // basic_serial_port constructors.

    serial_port port1(ioc);
    serial_port port2(ioc, "null");
    serial_port::native_handle_type native_port1 = port1.native_handle();
    serial_port port3(ioc, native_port1);

#if defined(ASIO_HAS_MOVE)
    serial_port port4(std::move(port3));
#endif // defined(ASIO_HAS_MOVE)

    // basic_serial_port operators.

#if defined(ASIO_HAS_MOVE)
    port1 = serial_port(ioc);
    port1 = std::move(port2);
#endif // defined(ASIO_HAS_MOVE)

    // basic_io_object functions.

    serial_port::executor_type ex = port1.get_executor();
    (void)ex;

#if !defined(ASIO_NO_DEPRECATED)
    io_context& ioc_ref = port1.get_io_context();
    (void)ioc_ref;

    io_context& ioc_ref2 = port1.get_io_service();
    (void)ioc_ref2;
#endif // !defined(ASIO_NO_DEPRECATED)

    // basic_serial_port functions.

    serial_port::lowest_layer_type& lowest_layer = port1.lowest_layer();
    (void)lowest_layer;

    const serial_port& port5 = port1;
    const serial_port::lowest_layer_type& lowest_layer2 = port5.lowest_layer();
    (void)lowest_layer2;

    port1.open("null");
    port1.open("null", ec);

    serial_port::native_handle_type native_port2 = port1.native_handle();
    port1.assign(native_port2);
    serial_port::native_handle_type native_port3 = port1.native_handle();
    port1.assign(native_port3, ec);

    bool is_open = port1.is_open();
    (void)is_open;

    port1.close();
    port1.close(ec);

    serial_port::native_handle_type native_port4 = port1.native_handle();
    (void)native_port4;

    port1.cancel();
    port1.cancel(ec);

    port1.set_option(serial_port_option);
    port1.set_option(serial_port_option, ec);

    port1.get_option(serial_port_option);
    port1.get_option(serial_port_option, ec);

    port1.send_break();
    port1.send_break(ec);

    port1.write_some(buffer(mutable_char_buffer));
    port1.write_some(buffer(const_char_buffer));
    port1.write_some(buffer(mutable_char_buffer), ec);
    port1.write_some(buffer(const_char_buffer), ec);

    port1.async_write_some(buffer(mutable_char_buffer), write_some_handler());
    port1.async_write_some(buffer(const_char_buffer), write_some_handler());
    int i1 = port1.async_write_some(buffer(mutable_char_buffer), lazy);
    (void)i1;
    int i2 = port1.async_write_some(buffer(const_char_buffer), lazy);
    (void)i2;

    port1.read_some(buffer(mutable_char_buffer));
    port1.read_some(buffer(mutable_char_buffer), ec);

    port1.async_read_some(buffer(mutable_char_buffer), read_some_handler());
    int i3 = port1.async_read_some(buffer(mutable_char_buffer), lazy);
    (void)i3;
  }
  catch (std::exception&)
  {
  }
#endif // defined(ASIO_HAS_SERIAL_PORT)
}

} // namespace serial_port_compile

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "serial_port",
  ASIO_TEST_CASE(serial_port_compile::test)
)
