//
// socket_base.cpp
// ~~~~~~~~~~~~~~~
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
#include "asio/socket_base.hpp"

#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ip/udp.hpp"
#include "unit_test.hpp"

//------------------------------------------------------------------------------

// socket_base_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all nested classes, enums and constants in
// socket_base compile and link correctly. Runtime failures are ignored.

namespace socket_base_compile {

void test()
{
  using namespace asio_sockio;
  namespace ip = asio_sockio::ip;

  try
  {
    io_context ioc;
    ip::tcp::socket sock(ioc);
    char buf[1024];

    // shutdown_type enumeration.

    sock.shutdown(socket_base::shutdown_receive);
    sock.shutdown(socket_base::shutdown_send);
    sock.shutdown(socket_base::shutdown_both);

    // message_flags constants.

    sock.receive(buffer(buf), socket_base::message_peek);
    sock.receive(buffer(buf), socket_base::message_out_of_band);
    sock.send(buffer(buf), socket_base::message_do_not_route);

    // broadcast class.

    socket_base::broadcast broadcast1(true);
    sock.set_option(broadcast1);
    socket_base::broadcast broadcast2;
    sock.get_option(broadcast2);
    broadcast1 = true;
    (void)static_cast<bool>(broadcast1);
    (void)static_cast<bool>(!broadcast1);
    (void)static_cast<bool>(broadcast1.value());

    // debug class.

    socket_base::debug debug1(true);
    sock.set_option(debug1);
    socket_base::debug debug2;
    sock.get_option(debug2);
    debug1 = true;
    (void)static_cast<bool>(debug1);
    (void)static_cast<bool>(!debug1);
    (void)static_cast<bool>(debug1.value());

    // do_not_route class.

    socket_base::do_not_route do_not_route1(true);
    sock.set_option(do_not_route1);
    socket_base::do_not_route do_not_route2;
    sock.get_option(do_not_route2);
    do_not_route1 = true;
    (void)static_cast<bool>(do_not_route1);
    (void)static_cast<bool>(!do_not_route1);
    (void)static_cast<bool>(do_not_route1.value());

    // keep_alive class.

    socket_base::keep_alive keep_alive1(true);
    sock.set_option(keep_alive1);
    socket_base::keep_alive keep_alive2;
    sock.get_option(keep_alive2);
    keep_alive1 = true;
    (void)static_cast<bool>(keep_alive1);
    (void)static_cast<bool>(!keep_alive1);
    (void)static_cast<bool>(keep_alive1.value());

    // send_buffer_size class.

    socket_base::send_buffer_size send_buffer_size1(1024);
    sock.set_option(send_buffer_size1);
    socket_base::send_buffer_size send_buffer_size2;
    sock.get_option(send_buffer_size2);
    send_buffer_size1 = 1;
    (void)static_cast<int>(send_buffer_size1.value());

    // send_low_watermark class.

    socket_base::send_low_watermark send_low_watermark1(128);
    sock.set_option(send_low_watermark1);
    socket_base::send_low_watermark send_low_watermark2;
    sock.get_option(send_low_watermark2);
    send_low_watermark1 = 1;
    (void)static_cast<int>(send_low_watermark1.value());

    // receive_buffer_size class.

    socket_base::receive_buffer_size receive_buffer_size1(1024);
    sock.set_option(receive_buffer_size1);
    socket_base::receive_buffer_size receive_buffer_size2;
    sock.get_option(receive_buffer_size2);
    receive_buffer_size1 = 1;
    (void)static_cast<int>(receive_buffer_size1.value());

    // receive_low_watermark class.

    socket_base::receive_low_watermark receive_low_watermark1(128);
    sock.set_option(receive_low_watermark1);
    socket_base::receive_low_watermark receive_low_watermark2;
    sock.get_option(receive_low_watermark2);
    receive_low_watermark1 = 1;
    (void)static_cast<int>(receive_low_watermark1.value());

    // reuse_address class.

    socket_base::reuse_address reuse_address1(true);
    sock.set_option(reuse_address1);
    socket_base::reuse_address reuse_address2;
    sock.get_option(reuse_address2);
    reuse_address1 = true;
    (void)static_cast<bool>(reuse_address1);
    (void)static_cast<bool>(!reuse_address1);
    (void)static_cast<bool>(reuse_address1.value());

    // linger class.

    socket_base::linger linger1(true, 30);
    sock.set_option(linger1);
    socket_base::linger linger2;
    sock.get_option(linger2);
    linger1.enabled(true);
    (void)static_cast<bool>(linger1.enabled());
    linger1.timeout(1);
    (void)static_cast<int>(linger1.timeout());

    // out_of_band_inline class.

    socket_base::out_of_band_inline out_of_band_inline1(true);
    sock.set_option(out_of_band_inline1);
    socket_base::out_of_band_inline out_of_band_inline2;
    sock.get_option(out_of_band_inline2);
    out_of_band_inline1 = true;
    (void)static_cast<bool>(out_of_band_inline1);
    (void)static_cast<bool>(!out_of_band_inline1);
    (void)static_cast<bool>(out_of_band_inline1.value());

    // enable_connection_aborted class.

    socket_base::enable_connection_aborted enable_connection_aborted1(true);
    sock.set_option(enable_connection_aborted1);
    socket_base::enable_connection_aborted enable_connection_aborted2;
    sock.get_option(enable_connection_aborted2);
    enable_connection_aborted1 = true;
    (void)static_cast<bool>(enable_connection_aborted1);
    (void)static_cast<bool>(!enable_connection_aborted1);
    (void)static_cast<bool>(enable_connection_aborted1.value());

    // bytes_readable class.

    socket_base::bytes_readable bytes_readable;
    sock.io_control(bytes_readable);
    std::size_t bytes = bytes_readable.get();
    (void)bytes;
  }
  catch (std::exception&)
  {
  }
}

} // namespace socket_base_compile

//------------------------------------------------------------------------------

// socket_base_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks the runtime operation of the socket options and I/O
// control commands defined in socket_base.

namespace socket_base_runtime {

void test()
{
  using namespace asio_sockio;
  namespace ip = asio_sockio::ip;

  io_context ioc;
  ip::udp::socket udp_sock(ioc, ip::udp::v4());
  ip::tcp::socket tcp_sock(ioc, ip::tcp::v4());
  ip::tcp::acceptor tcp_acceptor(ioc, ip::tcp::v4());
  asio_sockio::error_code ec;

  // broadcast class.

  socket_base::broadcast broadcast1(true);
  ASIO_CHECK(broadcast1.value());
  ASIO_CHECK(static_cast<bool>(broadcast1));
  ASIO_CHECK(!!broadcast1);
  udp_sock.set_option(broadcast1, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::broadcast broadcast2;
  udp_sock.get_option(broadcast2, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(broadcast2.value());
  ASIO_CHECK(static_cast<bool>(broadcast2));
  ASIO_CHECK(!!broadcast2);

  socket_base::broadcast broadcast3(false);
  ASIO_CHECK(!broadcast3.value());
  ASIO_CHECK(!static_cast<bool>(broadcast3));
  ASIO_CHECK(!broadcast3);
  udp_sock.set_option(broadcast3, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::broadcast broadcast4;
  udp_sock.get_option(broadcast4, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(!broadcast4.value());
  ASIO_CHECK(!static_cast<bool>(broadcast4));
  ASIO_CHECK(!broadcast4);

  // debug class.

  socket_base::debug debug1(true);
  ASIO_CHECK(debug1.value());
  ASIO_CHECK(static_cast<bool>(debug1));
  ASIO_CHECK(!!debug1);
  udp_sock.set_option(debug1, ec);
#if defined(__linux__)
  // On Linux, only root can set SO_DEBUG.
  bool not_root = (ec == asio_sockio::error::access_denied);
  ASIO_CHECK(!ec || not_root);
  ASIO_WARN_MESSAGE(!ec, "Must be root to set debug socket option");
#else // defined(__linux__)
# if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
# else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
# endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)
#endif // defined(__linux__)

  socket_base::debug debug2;
  udp_sock.get_option(debug2, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
# if defined(__linux__)
  ASIO_CHECK(debug2.value() || not_root);
  ASIO_CHECK(static_cast<bool>(debug2) || not_root);
  ASIO_CHECK(!!debug2 || not_root);
# else // defined(__linux__)
  ASIO_CHECK(debug2.value());
  ASIO_CHECK(static_cast<bool>(debug2));
  ASIO_CHECK(!!debug2);
# endif // defined(__linux__)
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  socket_base::debug debug3(false);
  ASIO_CHECK(!debug3.value());
  ASIO_CHECK(!static_cast<bool>(debug3));
  ASIO_CHECK(!debug3);
  udp_sock.set_option(debug3, ec);
#if defined(__linux__)
  ASIO_CHECK(!ec || not_root);
#else // defined(__linux__)
# if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
# else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
# endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)
#endif // defined(__linux__)

  socket_base::debug debug4;
  udp_sock.get_option(debug4, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
# if defined(__linux__)
  ASIO_CHECK(!debug4.value() || not_root);
  ASIO_CHECK(!static_cast<bool>(debug4) || not_root);
  ASIO_CHECK(!debug4 || not_root);
# else // defined(__linux__)
  ASIO_CHECK(!debug4.value());
  ASIO_CHECK(!static_cast<bool>(debug4));
  ASIO_CHECK(!debug4);
# endif // defined(__linux__)
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  // do_not_route class.

  socket_base::do_not_route do_not_route1(true);
  ASIO_CHECK(do_not_route1.value());
  ASIO_CHECK(static_cast<bool>(do_not_route1));
  ASIO_CHECK(!!do_not_route1);
  udp_sock.set_option(do_not_route1, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  socket_base::do_not_route do_not_route2;
  udp_sock.get_option(do_not_route2, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(do_not_route2.value());
  ASIO_CHECK(static_cast<bool>(do_not_route2));
  ASIO_CHECK(!!do_not_route2);
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  socket_base::do_not_route do_not_route3(false);
  ASIO_CHECK(!do_not_route3.value());
  ASIO_CHECK(!static_cast<bool>(do_not_route3));
  ASIO_CHECK(!do_not_route3);
  udp_sock.set_option(do_not_route3, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  socket_base::do_not_route do_not_route4;
  udp_sock.get_option(do_not_route4, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(!do_not_route4.value());
  ASIO_CHECK(!static_cast<bool>(do_not_route4));
  ASIO_CHECK(!do_not_route4);
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  // keep_alive class.

  socket_base::keep_alive keep_alive1(true);
  ASIO_CHECK(keep_alive1.value());
  ASIO_CHECK(static_cast<bool>(keep_alive1));
  ASIO_CHECK(!!keep_alive1);
  tcp_sock.set_option(keep_alive1, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::keep_alive keep_alive2;
  tcp_sock.get_option(keep_alive2, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(keep_alive2.value());
  ASIO_CHECK(static_cast<bool>(keep_alive2));
  ASIO_CHECK(!!keep_alive2);

  socket_base::keep_alive keep_alive3(false);
  ASIO_CHECK(!keep_alive3.value());
  ASIO_CHECK(!static_cast<bool>(keep_alive3));
  ASIO_CHECK(!keep_alive3);
  tcp_sock.set_option(keep_alive3, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::keep_alive keep_alive4;
  tcp_sock.get_option(keep_alive4, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(!keep_alive4.value());
  ASIO_CHECK(!static_cast<bool>(keep_alive4));
  ASIO_CHECK(!keep_alive4);

  // send_buffer_size class.

  socket_base::send_buffer_size send_buffer_size1(4096);
  ASIO_CHECK(send_buffer_size1.value() == 4096);
  tcp_sock.set_option(send_buffer_size1, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::send_buffer_size send_buffer_size2;
  tcp_sock.get_option(send_buffer_size2, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(send_buffer_size2.value() == 4096);

  socket_base::send_buffer_size send_buffer_size3(16384);
  ASIO_CHECK(send_buffer_size3.value() == 16384);
  tcp_sock.set_option(send_buffer_size3, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::send_buffer_size send_buffer_size4;
  tcp_sock.get_option(send_buffer_size4, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(send_buffer_size4.value() == 16384);

  // send_low_watermark class.

  socket_base::send_low_watermark send_low_watermark1(4096);
  ASIO_CHECK(send_low_watermark1.value() == 4096);
  tcp_sock.set_option(send_low_watermark1, ec);
#if defined(WIN32) || defined(__linux__) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows, Linux or Solaris.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif

  socket_base::send_low_watermark send_low_watermark2;
  tcp_sock.get_option(send_low_watermark2, ec);
#if defined(WIN32) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows or Solaris.
#elif defined(__linux__)
  ASIO_CHECK(!ec); // Not supported on Linux but can get value.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(send_low_watermark2.value() == 4096);
#endif

  socket_base::send_low_watermark send_low_watermark3(8192);
  ASIO_CHECK(send_low_watermark3.value() == 8192);
  tcp_sock.set_option(send_low_watermark3, ec);
#if defined(WIN32) || defined(__linux__) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows, Linux or Solaris.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif

  socket_base::send_low_watermark send_low_watermark4;
  tcp_sock.get_option(send_low_watermark4, ec);
#if defined(WIN32) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows or Solaris.
#elif defined(__linux__)
  ASIO_CHECK(!ec); // Not supported on Linux but can get value.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(send_low_watermark4.value() == 8192);
#endif

  // receive_buffer_size class.

  socket_base::receive_buffer_size receive_buffer_size1(4096);
  ASIO_CHECK(receive_buffer_size1.value() == 4096);
  tcp_sock.set_option(receive_buffer_size1, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  socket_base::receive_buffer_size receive_buffer_size2;
  tcp_sock.get_option(receive_buffer_size2, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK(!ec); // Not supported under Windows CE but can get value.
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(receive_buffer_size2.value() == 4096);
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  socket_base::receive_buffer_size receive_buffer_size3(16384);
  ASIO_CHECK(receive_buffer_size3.value() == 16384);
  tcp_sock.set_option(receive_buffer_size3, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Option is not supported under Windows CE.
  ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
      ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  socket_base::receive_buffer_size receive_buffer_size4;
  tcp_sock.get_option(receive_buffer_size4, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK(!ec); // Not supported under Windows CE but can get value.
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(receive_buffer_size4.value() == 16384);
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

  // receive_low_watermark class.

  socket_base::receive_low_watermark receive_low_watermark1(4096);
  ASIO_CHECK(receive_low_watermark1.value() == 4096);
  tcp_sock.set_option(receive_low_watermark1, ec);
#if defined(WIN32) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows or Solaris.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif

  socket_base::receive_low_watermark receive_low_watermark2;
  tcp_sock.get_option(receive_low_watermark2, ec);
#if defined(WIN32) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows or Solaris.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(receive_low_watermark2.value() == 4096);
#endif

  socket_base::receive_low_watermark receive_low_watermark3(8192);
  ASIO_CHECK(receive_low_watermark3.value() == 8192);
  tcp_sock.set_option(receive_low_watermark3, ec);
#if defined(WIN32) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows or Solaris.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif

  socket_base::receive_low_watermark receive_low_watermark4;
  tcp_sock.get_option(receive_low_watermark4, ec);
#if defined(WIN32) || defined(__sun)
  ASIO_CHECK(!!ec); // Not supported on Windows or Solaris.
#else
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(receive_low_watermark4.value() == 8192);
#endif

  // reuse_address class.

  socket_base::reuse_address reuse_address1(true);
  ASIO_CHECK(reuse_address1.value());
  ASIO_CHECK(static_cast<bool>(reuse_address1));
  ASIO_CHECK(!!reuse_address1);
  udp_sock.set_option(reuse_address1, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::reuse_address reuse_address2;
  udp_sock.get_option(reuse_address2, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(reuse_address2.value());
  ASIO_CHECK(static_cast<bool>(reuse_address2));
  ASIO_CHECK(!!reuse_address2);

  socket_base::reuse_address reuse_address3(false);
  ASIO_CHECK(!reuse_address3.value());
  ASIO_CHECK(!static_cast<bool>(reuse_address3));
  ASIO_CHECK(!reuse_address3);
  udp_sock.set_option(reuse_address3, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::reuse_address reuse_address4;
  udp_sock.get_option(reuse_address4, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(!reuse_address4.value());
  ASIO_CHECK(!static_cast<bool>(reuse_address4));
  ASIO_CHECK(!reuse_address4);

  // linger class.

  socket_base::linger linger1(true, 60);
  ASIO_CHECK(linger1.enabled());
  ASIO_CHECK(linger1.timeout() == 60);
  tcp_sock.set_option(linger1, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::linger linger2;
  tcp_sock.get_option(linger2, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(linger2.enabled());
  ASIO_CHECK(linger2.timeout() == 60);

  socket_base::linger linger3(false, 0);
  ASIO_CHECK(!linger3.enabled());
  ASIO_CHECK(linger3.timeout() == 0);
  tcp_sock.set_option(linger3, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::linger linger4;
  tcp_sock.get_option(linger4, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(!linger4.enabled());

  // enable_connection_aborted class.

  socket_base::enable_connection_aborted enable_connection_aborted1(true);
  ASIO_CHECK(enable_connection_aborted1.value());
  ASIO_CHECK(static_cast<bool>(enable_connection_aborted1));
  ASIO_CHECK(!!enable_connection_aborted1);
  tcp_acceptor.set_option(enable_connection_aborted1, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::enable_connection_aborted enable_connection_aborted2;
  tcp_acceptor.get_option(enable_connection_aborted2, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(enable_connection_aborted2.value());
  ASIO_CHECK(static_cast<bool>(enable_connection_aborted2));
  ASIO_CHECK(!!enable_connection_aborted2);

  socket_base::enable_connection_aborted enable_connection_aborted3(false);
  ASIO_CHECK(!enable_connection_aborted3.value());
  ASIO_CHECK(!static_cast<bool>(enable_connection_aborted3));
  ASIO_CHECK(!enable_connection_aborted3);
  tcp_acceptor.set_option(enable_connection_aborted3, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

  socket_base::enable_connection_aborted enable_connection_aborted4;
  tcp_acceptor.get_option(enable_connection_aborted4, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  ASIO_CHECK(!enable_connection_aborted4.value());
  ASIO_CHECK(!static_cast<bool>(enable_connection_aborted4));
  ASIO_CHECK(!enable_connection_aborted4);

  // bytes_readable class.

  socket_base::bytes_readable bytes_readable;
  udp_sock.io_control(bytes_readable, ec);
  ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
}

} // namespace socket_base_runtime

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "socket_base",
  ASIO_TEST_CASE(socket_base_compile::test)
  ASIO_TEST_CASE(socket_base_runtime::test)
)
