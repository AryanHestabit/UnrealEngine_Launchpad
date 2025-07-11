//
// ip/basic_endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_BASIC_ENDPOINT_HPP
#define ASIO_IP_BASIC_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/ip/address.hpp"
#include "asio/ip/detail/endpoint.hpp"

#if !defined(ASIO_NO_IOSTREAM)
# include <iosfwd>
#endif // !defined(ASIO_NO_IOSTREAM)

#include "asio/detail/push_options.hpp"

namespace asio_sockio {
namespace ip {

/// Describes an endpoint for a version-independent IP socket.
/**
 * The asio_sockio::ip::basic_endpoint class template describes an endpoint that
 * may be associated with a particular socket.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Concepts:
 * Endpoint.
 */
template <typename InternetProtocol>
class basic_endpoint
{
public:
  /// The protocol type associated with the endpoint.
  typedef InternetProtocol protocol_type;

  /// The type of the endpoint structure. This type is dependent on the
  /// underlying implementation of the socket layer.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined data_type;
#else
  typedef asio_sockio::detail::socket_addr_type data_type;
#endif

  /// Default constructor.
  basic_endpoint() ASIO_NOEXCEPT
    : impl_()
  {
  }

  /// Construct an endpoint using a port number, specified in the host's byte
  /// order. The IP address will be the any address (i.e. INADDR_ANY or
  /// in6addr_any). This constructor would typically be used for accepting new
  /// connections.
  /**
   * @par Examples
   * To initialise an IPv4 TCP endpoint for port 1234, use:
   * @code
   * asio_sockio::ip::tcp::endpoint ep(asio_sockio::ip::tcp::v4(), 1234);
   * @endcode
   *
   * To specify an IPv6 UDP endpoint for port 9876, use:
   * @code
   * asio_sockio::ip::udp::endpoint ep(asio_sockio::ip::udp::v6(), 9876);
   * @endcode
   */
  basic_endpoint(const InternetProtocol& internet_protocol,
      unsigned short port_num) ASIO_NOEXCEPT
    : impl_(internet_protocol.family(), port_num)
  {
  }

  /// Construct an endpoint using a port number and an IP address. This
  /// constructor may be used for accepting connections on a specific interface
  /// or for making a connection to a remote endpoint.
  basic_endpoint(const asio_sockio::ip::address& addr,
      unsigned short port_num) ASIO_NOEXCEPT
    : impl_(addr, port_num)
  {
  }

  /// Copy constructor.
  basic_endpoint(const basic_endpoint& other) ASIO_NOEXCEPT
    : impl_(other.impl_)
  {
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move constructor.
  basic_endpoint(basic_endpoint&& other) ASIO_NOEXCEPT
    : impl_(other.impl_)
  {
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Assign from another endpoint.
  basic_endpoint& operator=(const basic_endpoint& other) ASIO_NOEXCEPT
  {
    impl_ = other.impl_;
    return *this;
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move-assign from another endpoint.
  basic_endpoint& operator=(basic_endpoint&& other) ASIO_NOEXCEPT
  {
    impl_ = other.impl_;
    return *this;
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// The protocol associated with the endpoint.
  protocol_type protocol() const ASIO_NOEXCEPT
  {
    if (impl_.is_v4())
      return InternetProtocol::v4();
    return InternetProtocol::v6();
  }

  /// Get the underlying endpoint in the native type.
  data_type* data() ASIO_NOEXCEPT
  {
    return impl_.data();
  }

  /// Get the underlying endpoint in the native type.
  const data_type* data() const ASIO_NOEXCEPT
  {
    return impl_.data();
  }

  /// Get the underlying size of the endpoint in the native type.
  std::size_t size() const ASIO_NOEXCEPT
  {
    return impl_.size();
  }

  /// Set the underlying size of the endpoint in the native type.
  void resize(std::size_t new_size)
  {
    impl_.resize(new_size);
  }

  /// Get the capacity of the endpoint in the native type.
  std::size_t capacity() const ASIO_NOEXCEPT
  {
    return impl_.capacity();
  }

  /// Get the port associated with the endpoint. The port number is always in
  /// the host's byte order.
  unsigned short port() const ASIO_NOEXCEPT
  {
    return impl_.port();
  }

  /// Set the port associated with the endpoint. The port number is always in
  /// the host's byte order.
  void port(unsigned short port_num) ASIO_NOEXCEPT
  {
    impl_.port(port_num);
  }

  /// Get the IP address associated with the endpoint.
  asio_sockio::ip::address address() const ASIO_NOEXCEPT
  {
    return impl_.address();
  }

  /// Set the IP address associated with the endpoint.
  void address(const asio_sockio::ip::address& addr) ASIO_NOEXCEPT
  {
    impl_.address(addr);
  }

  /// Compare two endpoints for equality.
  friend bool operator==(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2) ASIO_NOEXCEPT
  {
    return e1.impl_ == e2.impl_;
  }

  /// Compare two endpoints for inequality.
  friend bool operator!=(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2) ASIO_NOEXCEPT
  {
    return !(e1 == e2);
  }

  /// Compare endpoints for ordering.
  friend bool operator<(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2) ASIO_NOEXCEPT
  {
    return e1.impl_ < e2.impl_;
  }

  /// Compare endpoints for ordering.
  friend bool operator>(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2) ASIO_NOEXCEPT
  {
    return e2.impl_ < e1.impl_;
  }

  /// Compare endpoints for ordering.
  friend bool operator<=(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2) ASIO_NOEXCEPT
  {
    return !(e2 < e1);
  }

  /// Compare endpoints for ordering.
  friend bool operator>=(const basic_endpoint<InternetProtocol>& e1,
      const basic_endpoint<InternetProtocol>& e2) ASIO_NOEXCEPT
  {
    return !(e1 < e2);
  }

private:
  // The underlying IP endpoint.
  asio_sockio::ip::detail::endpoint impl_;
};

#if !defined(ASIO_NO_IOSTREAM)

/// Output an endpoint as a string.
/**
 * Used to output a human-readable string for a specified endpoint.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param endpoint The endpoint to be written.
 *
 * @return The output stream.
 *
 * @relates asio_sockio::ip::basic_endpoint
 */
template <typename Elem, typename Traits, typename InternetProtocol>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os,
    const basic_endpoint<InternetProtocol>& endpoint);

#endif // !defined(ASIO_NO_IOSTREAM)

} // namespace ip
} // namespace asio_sockio

#include "asio/detail/pop_options.hpp"

#include "asio/ip/impl/basic_endpoint.hpp"

#endif // ASIO_IP_BASIC_ENDPOINT_HPP
