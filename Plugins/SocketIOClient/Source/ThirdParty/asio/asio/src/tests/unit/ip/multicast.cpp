//
// multicast.cpp
// ~~~~~~~~~~~~~
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
#include "asio/ip/multicast.hpp"

#include "asio/io_context.hpp"
#include "asio/ip/udp.hpp"
#include "../unit_test.hpp"

//------------------------------------------------------------------------------

// ip_multicast_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all nested classes, enums and constants in
// ip::multicast compile and link correctly. Runtime failures are ignored.

namespace ip_multicast_compile {

void test()
{
  using namespace asio_sockio;
  namespace ip = asio_sockio::ip;

  try
  {
    io_context ioc;
    ip::udp::socket sock(ioc);
    const ip::address address;
    const ip::address_v4 address_v4;
    const ip::address_v6 address_v6;

    // join_group class.

    ip::multicast::join_group join_group1;
    ip::multicast::join_group join_group2(address);
    ip::multicast::join_group join_group3(address_v4);
    ip::multicast::join_group join_group4(address_v4, address_v4);
    ip::multicast::join_group join_group5(address_v6);
    ip::multicast::join_group join_group6(address_v6, 1);
    sock.set_option(join_group6);

    // leave_group class.

    ip::multicast::leave_group leave_group1;
    ip::multicast::leave_group leave_group2(address);
    ip::multicast::leave_group leave_group3(address_v4);
    ip::multicast::leave_group leave_group4(address_v4, address_v4);
    ip::multicast::leave_group leave_group5(address_v6);
    ip::multicast::leave_group leave_group6(address_v6, 1);
    sock.set_option(leave_group6);

    // outbound_interface class.

    ip::multicast::outbound_interface outbound_interface1;
    ip::multicast::outbound_interface outbound_interface2(address_v4);
    ip::multicast::outbound_interface outbound_interface3(1);
    sock.set_option(outbound_interface3);

    // hops class.

    ip::multicast::hops hops1(1024);
    sock.set_option(hops1);
    ip::multicast::hops hops2;
    sock.get_option(hops2);
    hops1 = 1;
    (void)static_cast<int>(hops1.value());

    // enable_loopback class.

    ip::multicast::enable_loopback enable_loopback1(true);
    sock.set_option(enable_loopback1);
    ip::multicast::enable_loopback enable_loopback2;
    sock.get_option(enable_loopback2);
    enable_loopback1 = true;
    (void)static_cast<bool>(enable_loopback1);
    (void)static_cast<bool>(!enable_loopback1);
    (void)static_cast<bool>(enable_loopback1.value());
  }
  catch (std::exception&)
  {
  }
}

} // namespace ip_multicast_compile

//------------------------------------------------------------------------------

// ip_multicast_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks the runtime operation of the socket options defined
// in the ip::multicast namespace.

namespace ip_multicast_runtime {

#if defined(__hpux)
// HP-UX doesn't declare this function extern "C", so it is declared again here
// to avoid a linker error about an undefined symbol.
extern "C" unsigned int if_nametoindex(const char*);
#endif // defined(__hpux)

void test()
{
  using namespace asio_sockio;
  namespace ip = asio_sockio::ip;

  io_context ioc;
  asio_sockio::error_code ec;

  ip::udp::endpoint ep_v4(ip::address_v4::loopback(), 0);
  ip::udp::socket sock_v4(ioc);
  sock_v4.open(ep_v4.protocol(), ec);
  sock_v4.bind(ep_v4, ec);
  bool have_v4 = !ec;

  ip::udp::endpoint ep_v6(ip::address_v6::loopback(), 0);
  ip::udp::socket sock_v6(ioc);
  sock_v6.open(ep_v6.protocol(), ec);
  sock_v6.bind(ep_v6, ec);
  bool have_v6 = !ec;

  ASIO_CHECK(have_v4 || have_v6);

#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
  // Windows CE seems to have problems with some multicast group addresses.
  // The following address works on CE, but as it is not a private multicast
  // address it will not be used on other platforms.
  const ip::address multicast_address_v4 = ip::make_address("239.0.0.4", ec);
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  const ip::address multicast_address_v4 = ip::make_address("239.255.0.1", ec);
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  ASIO_CHECK(!have_v4 || !ec);

#if (defined(__MACH__) && defined(__APPLE__)) \
  || defined(__FreeBSD__) \
  || defined(__NetBSD__) \
  || defined(__OpenBSD__)
  const ip::address multicast_address_v6 = ip::make_address("ff02::1%lo0", ec);
#else // (defined(__MACH__) && defined(__APPLE__))
      //   || defined(__FreeBSD__)
      //   || defined(__NetBSD__)
      //   || defined(__OpenBSD__)
  const ip::address multicast_address_v6 = ip::make_address("ff01::1", ec);
#endif // (defined(__MACH__) && defined(__APPLE__))
       //   || defined(__FreeBSD__)
       //   || defined(__NetBSD__)
       //   || defined(__OpenBSD__)
  ASIO_CHECK(!have_v6 || !ec);

  // join_group class.

  if (have_v4)
  {
    ip::multicast::join_group join_group(multicast_address_v4);
    sock_v4.set_option(join_group, ec);
    ASIO_CHECK_MESSAGE(!ec || ec == error::no_such_device,
                       ec.value() << ", " << ec.message());

    if (!ec)
    {
      // leave_group class.

      ip::multicast::leave_group leave_group(multicast_address_v4);
      sock_v4.set_option(leave_group, ec);
      ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    }
  }

  if (have_v6)
  {
    ip::multicast::join_group join_group(multicast_address_v6);
    sock_v6.set_option(join_group, ec);
    ASIO_CHECK_MESSAGE(!ec || ec == error::no_such_device,
                       ec.value() << ", " << ec.message());

    if (!ec)
    {
      // leave_group class.

      ip::multicast::leave_group leave_group(multicast_address_v6);
      sock_v6.set_option(leave_group, ec);
      ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    }
  }

  // outbound_interface class.

  if (have_v4)
  {
    ip::multicast::outbound_interface outbound_interface(
        ip::address_v4::loopback());
    sock_v4.set_option(outbound_interface, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  }

  if (have_v6)
  {
#if defined(__hpux)
    ip::multicast::outbound_interface outbound_interface(if_nametoindex("lo0"));
#else
    ip::multicast::outbound_interface outbound_interface(1);
#endif
    sock_v6.set_option(outbound_interface, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
  }

  // hops class.

  if (have_v4)
  {
    ip::multicast::hops hops1(1);
    ASIO_CHECK(hops1.value() == 1);
    sock_v4.set_option(hops1, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

    ip::multicast::hops hops2;
    sock_v4.get_option(hops2, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(hops2.value() == 1);

    ip::multicast::hops hops3(0);
    ASIO_CHECK(hops3.value() == 0);
    sock_v4.set_option(hops3, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

    ip::multicast::hops hops4;
    sock_v4.get_option(hops4, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(hops4.value() == 0);
  }

  if (have_v6)
  {
    ip::multicast::hops hops1(1);
    ASIO_CHECK(hops1.value() == 1);
    sock_v6.set_option(hops1, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

    ip::multicast::hops hops2;
    sock_v6.get_option(hops2, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(hops2.value() == 1);

    ip::multicast::hops hops3(0);
    ASIO_CHECK(hops3.value() == 0);
    sock_v6.set_option(hops3, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

    ip::multicast::hops hops4;
    sock_v6.get_option(hops4, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(hops4.value() == 0);
  }

  // enable_loopback class.

  if (have_v4)
  {
    ip::multicast::enable_loopback enable_loopback1(true);
    ASIO_CHECK(enable_loopback1.value());
    ASIO_CHECK(static_cast<bool>(enable_loopback1));
    ASIO_CHECK(!!enable_loopback1);
    sock_v4.set_option(enable_loopback1, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
    // Option is not supported under Windows CE.
    ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
        ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

    ip::multicast::enable_loopback enable_loopback2;
    sock_v4.get_option(enable_loopback2, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
    // Not supported under Windows CE but can get value.
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(enable_loopback2.value());
    ASIO_CHECK(static_cast<bool>(enable_loopback2));
    ASIO_CHECK(!!enable_loopback2);
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

    ip::multicast::enable_loopback enable_loopback3(false);
    ASIO_CHECK(!enable_loopback3.value());
    ASIO_CHECK(!static_cast<bool>(enable_loopback3));
    ASIO_CHECK(!enable_loopback3);
    sock_v4.set_option(enable_loopback3, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
    // Option is not supported under Windows CE.
    ASIO_CHECK_MESSAGE(ec == asio_sockio::error::no_protocol_option,
        ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)

    ip::multicast::enable_loopback enable_loopback4;
    sock_v4.get_option(enable_loopback4, ec);
#if defined(ASIO_WINDOWS) && defined(UNDER_CE)
    // Not supported under Windows CE but can get value.
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
#else // defined(ASIO_WINDOWS) && defined(UNDER_CE)
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(!enable_loopback4.value());
    ASIO_CHECK(!static_cast<bool>(enable_loopback4));
    ASIO_CHECK(!enable_loopback4);
#endif // defined(ASIO_WINDOWS) && defined(UNDER_CE)
  }

  if (have_v6)
  {
    ip::multicast::enable_loopback enable_loopback1(true);
    ASIO_CHECK(enable_loopback1.value());
    ASIO_CHECK(static_cast<bool>(enable_loopback1));
    ASIO_CHECK(!!enable_loopback1);
    sock_v6.set_option(enable_loopback1, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

    ip::multicast::enable_loopback enable_loopback2;
    sock_v6.get_option(enable_loopback2, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(enable_loopback2.value());
    ASIO_CHECK(static_cast<bool>(enable_loopback2));
    ASIO_CHECK(!!enable_loopback2);

    ip::multicast::enable_loopback enable_loopback3(false);
    ASIO_CHECK(!enable_loopback3.value());
    ASIO_CHECK(!static_cast<bool>(enable_loopback3));
    ASIO_CHECK(!enable_loopback3);
    sock_v6.set_option(enable_loopback3, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());

    ip::multicast::enable_loopback enable_loopback4;
    sock_v6.get_option(enable_loopback4, ec);
    ASIO_CHECK_MESSAGE(!ec, ec.value() << ", " << ec.message());
    ASIO_CHECK(!enable_loopback4.value());
    ASIO_CHECK(!static_cast<bool>(enable_loopback4));
    ASIO_CHECK(!enable_loopback4);
  }
}

} // namespace ip_multicast_runtime

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "ip/multicast",
  ASIO_TEST_CASE(ip_multicast_compile::test)
  ASIO_TEST_CASE(ip_multicast_runtime::test)
)
