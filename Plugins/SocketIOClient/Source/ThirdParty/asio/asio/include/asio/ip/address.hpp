//
// ip/address.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_ADDRESS_HPP
#define ASIO_IP_ADDRESS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <string>
#include "asio/detail/throw_exception.hpp"
#include "asio/detail/string_view.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/error_code.hpp"
#include "asio/ip/address_v4.hpp"
#include "asio/ip/address_v6.hpp"
#include "asio/ip/bad_address_cast.hpp"

#if !defined(ASIO_NO_IOSTREAM)
# include <iosfwd>
#endif // !defined(ASIO_NO_IOSTREAM)

#include "asio/detail/push_options.hpp"

namespace asio_sockio {
namespace ip {

/// Implements version-independent IP addresses.
/**
 * The asio_sockio::ip::address class provides the ability to use either IP
 * version 4 or version 6 addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
class address
{
public:
  /// Default constructor.
  ASIO_DECL address() ASIO_NOEXCEPT;

  /// Construct an address from an IPv4 address.
  ASIO_DECL address(
      const asio_sockio::ip::address_v4& ipv4_address) ASIO_NOEXCEPT;

  /// Construct an address from an IPv6 address.
  ASIO_DECL address(
      const asio_sockio::ip::address_v6& ipv6_address) ASIO_NOEXCEPT;

  /// Copy constructor.
  ASIO_DECL address(const address& other) ASIO_NOEXCEPT;

#if defined(ASIO_HAS_MOVE)
  /// Move constructor.
  ASIO_DECL address(address&& other) ASIO_NOEXCEPT;
#endif // defined(ASIO_HAS_MOVE)

  /// Assign from another address.
  ASIO_DECL address& operator=(const address& other) ASIO_NOEXCEPT;

#if defined(ASIO_HAS_MOVE)
  /// Move-assign from another address.
  ASIO_DECL address& operator=(address&& other) ASIO_NOEXCEPT;
#endif // defined(ASIO_HAS_MOVE)

  /// Assign from an IPv4 address.
  ASIO_DECL address& operator=(
      const asio_sockio::ip::address_v4& ipv4_address) ASIO_NOEXCEPT;

  /// Assign from an IPv6 address.
  ASIO_DECL address& operator=(
      const asio_sockio::ip::address_v6& ipv6_address) ASIO_NOEXCEPT;

  /// Get whether the address is an IP version 4 address.
  bool is_v4() const ASIO_NOEXCEPT
  {
    return type_ == ipv4;
  }

  /// Get whether the address is an IP version 6 address.
  bool is_v6() const ASIO_NOEXCEPT
  {
    return type_ == ipv6;
  }

  /// Get the address as an IP version 4 address.
  ASIO_DECL asio_sockio::ip::address_v4 to_v4() const;

  /// Get the address as an IP version 6 address.
  ASIO_DECL asio_sockio::ip::address_v6 to_v6() const;

  /// Get the address as a string.
  ASIO_DECL std::string to_string() const;

#if !defined(ASIO_NO_DEPRECATED)
  /// (Deprecated: Use other overload.) Get the address as a string.
  ASIO_DECL std::string to_string(asio_sockio::error_code& ec) const;

  /// (Deprecated: Use make_address().) Create an address from an IPv4 address
  /// string in dotted decimal form, or from an IPv6 address in hexadecimal
  /// notation.
  static address from_string(const char* str);

  /// (Deprecated: Use make_address().) Create an address from an IPv4 address
  /// string in dotted decimal form, or from an IPv6 address in hexadecimal
  /// notation.
  static address from_string(const char* str, asio_sockio::error_code& ec);

  /// (Deprecated: Use make_address().) Create an address from an IPv4 address
  /// string in dotted decimal form, or from an IPv6 address in hexadecimal
  /// notation.
  static address from_string(const std::string& str);

  /// (Deprecated: Use make_address().) Create an address from an IPv4 address
  /// string in dotted decimal form, or from an IPv6 address in hexadecimal
  /// notation.
  static address from_string(
      const std::string& str, asio_sockio::error_code& ec);
#endif // !defined(ASIO_NO_DEPRECATED)

  /// Determine whether the address is a loopback address.
  ASIO_DECL bool is_loopback() const ASIO_NOEXCEPT;

  /// Determine whether the address is unspecified.
  ASIO_DECL bool is_unspecified() const ASIO_NOEXCEPT;

  /// Determine whether the address is a multicast address.
  ASIO_DECL bool is_multicast() const ASIO_NOEXCEPT;

  /// Compare two addresses for equality.
  ASIO_DECL friend bool operator==(const address& a1,
      const address& a2) ASIO_NOEXCEPT;

  /// Compare two addresses for inequality.
  friend bool operator!=(const address& a1,
      const address& a2) ASIO_NOEXCEPT
  {
    return !(a1 == a2);
  }

  /// Compare addresses for ordering.
  ASIO_DECL friend bool operator<(const address& a1,
      const address& a2) ASIO_NOEXCEPT;

  /// Compare addresses for ordering.
  friend bool operator>(const address& a1,
      const address& a2) ASIO_NOEXCEPT
  {
    return a2 < a1;
  }

  /// Compare addresses for ordering.
  friend bool operator<=(const address& a1,
      const address& a2) ASIO_NOEXCEPT
  {
    return !(a2 < a1);
  }

  /// Compare addresses for ordering.
  friend bool operator>=(const address& a1,
      const address& a2) ASIO_NOEXCEPT
  {
    return !(a1 < a2);
  }

private:
  // The type of the address.
  enum { ipv4, ipv6 } type_;

  // The underlying IPv4 address.
  asio_sockio::ip::address_v4 ipv4_address_;

  // The underlying IPv6 address.
  asio_sockio::ip::address_v6 ipv6_address_;
};

/// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
/**
 * @relates address
 */
ASIO_DECL address make_address(const char* str);

/// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
/**
 * @relates address
 */
ASIO_DECL address make_address(const char* str,
    asio_sockio::error_code& ec) ASIO_NOEXCEPT;

/// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
/**
 * @relates address
 */
ASIO_DECL address make_address(const std::string& str);

/// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
/**
 * @relates address
 */
ASIO_DECL address make_address(const std::string& str,
    asio_sockio::error_code& ec) ASIO_NOEXCEPT;

#if defined(ASIO_HAS_STRING_VIEW) \
  || defined(GENERATING_DOCUMENTATION)

/// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
/**
 * @relates address
 */
ASIO_DECL address make_address(string_view str);

/// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
/**
 * @relates address
 */
ASIO_DECL address make_address(string_view str,
    asio_sockio::error_code& ec) ASIO_NOEXCEPT;

#endif // defined(ASIO_HAS_STRING_VIEW)
       //  || defined(GENERATING_DOCUMENTATION)

#if !defined(ASIO_NO_IOSTREAM)

/// Output an address as a string.
/**
 * Used to output a human-readable string for a specified address.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param addr The address to be written.
 *
 * @return The output stream.
 *
 * @relates asio_sockio::ip::address
 */
template <typename Elem, typename Traits>
std::basic_ostream<Elem, Traits>& operator<<(
    std::basic_ostream<Elem, Traits>& os, const address& addr);

#endif // !defined(ASIO_NO_IOSTREAM)

} // namespace ip
} // namespace asio_sockio

#include "asio/detail/pop_options.hpp"

#include "asio/ip/impl/address.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/ip/impl/address.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_IP_ADDRESS_HPP
