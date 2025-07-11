//
// ip/impl/address.ipp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_IMPL_ADDRESS_IPP
#define ASIO_IP_IMPL_ADDRESS_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <typeinfo>
#include "asio/detail/throw_error.hpp"
#include "asio/detail/throw_exception.hpp"
#include "asio/error.hpp"
#include "asio/ip/address.hpp"
#include "asio/ip/bad_address_cast.hpp"
#include "asio/system_error.hpp"

#include "asio/detail/push_options.hpp"

namespace asio_sockio {
namespace ip {

address::address() ASIO_NOEXCEPT
  : type_(ipv4),
    ipv4_address_(),
    ipv6_address_()
{
}

address::address(
    const asio_sockio::ip::address_v4& ipv4_address) ASIO_NOEXCEPT
  : type_(ipv4),
    ipv4_address_(ipv4_address),
    ipv6_address_()
{
}

address::address(
    const asio_sockio::ip::address_v6& ipv6_address) ASIO_NOEXCEPT
  : type_(ipv6),
    ipv4_address_(),
    ipv6_address_(ipv6_address)
{
}

address::address(const address& other) ASIO_NOEXCEPT
  : type_(other.type_),
    ipv4_address_(other.ipv4_address_),
    ipv6_address_(other.ipv6_address_)
{
}

#if defined(ASIO_HAS_MOVE)
address::address(address&& other) ASIO_NOEXCEPT
  : type_(other.type_),
    ipv4_address_(other.ipv4_address_),
    ipv6_address_(other.ipv6_address_)
{
}
#endif // defined(ASIO_HAS_MOVE)

address& address::operator=(const address& other) ASIO_NOEXCEPT
{
  type_ = other.type_;
  ipv4_address_ = other.ipv4_address_;
  ipv6_address_ = other.ipv6_address_;
  return *this;
}

#if defined(ASIO_HAS_MOVE)
address& address::operator=(address&& other) ASIO_NOEXCEPT
{
  type_ = other.type_;
  ipv4_address_ = other.ipv4_address_;
  ipv6_address_ = other.ipv6_address_;
  return *this;
}
#endif // defined(ASIO_HAS_MOVE)

address& address::operator=(
    const asio_sockio::ip::address_v4& ipv4_address) ASIO_NOEXCEPT
{
  type_ = ipv4;
  ipv4_address_ = ipv4_address;
  ipv6_address_ = asio_sockio::ip::address_v6();
  return *this;
}

address& address::operator=(
    const asio_sockio::ip::address_v6& ipv6_address) ASIO_NOEXCEPT
{
  type_ = ipv6;
  ipv4_address_ = asio_sockio::ip::address_v4();
  ipv6_address_ = ipv6_address;
  return *this;
}

address make_address(const char* str)
{
  asio_sockio::error_code ec;
  address addr = make_address(str, ec);
  asio_sockio::detail::throw_error(ec);
  return addr;
}

address make_address(const char* str,
    asio_sockio::error_code& ec) ASIO_NOEXCEPT
{
  asio_sockio::ip::address_v6 ipv6_address =
    asio_sockio::ip::make_address_v6(str, ec);
  if (!ec)
    return address(ipv6_address);

  asio_sockio::ip::address_v4 ipv4_address =
    asio_sockio::ip::make_address_v4(str, ec);
  if (!ec)
    return address(ipv4_address);

  return address();
}

address make_address(const std::string& str)
{
  return make_address(str.c_str());
}

address make_address(const std::string& str,
    asio_sockio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address(str.c_str(), ec);
}

#if defined(ASIO_HAS_STRING_VIEW)

address make_address(string_view str)
{
  return make_address(static_cast<std::string>(str));
}

address make_address(string_view str,
    asio_sockio::error_code& ec) ASIO_NOEXCEPT
{
  return make_address(static_cast<std::string>(str), ec);
}

#endif // defined(ASIO_HAS_STRING_VIEW)

asio_sockio::ip::address_v4 address::to_v4() const
{
  if (type_ != ipv4)
  {
    bad_address_cast ex;
    asio_sockio::detail::throw_exception(ex);
  }
  return ipv4_address_;
}

asio_sockio::ip::address_v6 address::to_v6() const
{
  if (type_ != ipv6)
  {
    bad_address_cast ex;
    asio_sockio::detail::throw_exception(ex);
  }
  return ipv6_address_;
}

std::string address::to_string() const
{
  if (type_ == ipv6)
    return ipv6_address_.to_string();
  return ipv4_address_.to_string();
}

#if !defined(ASIO_NO_DEPRECATED)
std::string address::to_string(asio_sockio::error_code& ec) const
{
  if (type_ == ipv6)
    return ipv6_address_.to_string(ec);
  return ipv4_address_.to_string(ec);
}
#endif // !defined(ASIO_NO_DEPRECATED)

bool address::is_loopback() const ASIO_NOEXCEPT
{
  return (type_ == ipv4)
    ? ipv4_address_.is_loopback()
    : ipv6_address_.is_loopback();
}

bool address::is_unspecified() const ASIO_NOEXCEPT
{
  return (type_ == ipv4)
    ? ipv4_address_.is_unspecified()
    : ipv6_address_.is_unspecified();
}

bool address::is_multicast() const ASIO_NOEXCEPT
{
  return (type_ == ipv4)
    ? ipv4_address_.is_multicast()
    : ipv6_address_.is_multicast();
}

bool operator==(const address& a1, const address& a2) ASIO_NOEXCEPT
{
  if (a1.type_ != a2.type_)
    return false;
  if (a1.type_ == address::ipv6)
    return a1.ipv6_address_ == a2.ipv6_address_;
  return a1.ipv4_address_ == a2.ipv4_address_;
}

bool operator<(const address& a1, const address& a2) ASIO_NOEXCEPT
{
  if (a1.type_ < a2.type_)
    return true;
  if (a1.type_ > a2.type_)
    return false;
  if (a1.type_ == address::ipv6)
    return a1.ipv6_address_ < a2.ipv6_address_;
  return a1.ipv4_address_ < a2.ipv4_address_;
}

} // namespace ip
} // namespace asio_sockio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IP_IMPL_ADDRESS_IPP
