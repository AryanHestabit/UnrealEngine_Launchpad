//
// windows/random_access_handle_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_WINDOWS_RANDOM_ACCESS_HANDLE_SERVICE_HPP
#define ASIO_WINDOWS_RANDOM_ACCESS_HANDLE_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_ENABLE_OLD_SERVICES)

#if defined(ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE) \
  || defined(GENERATING_DOCUMENTATION)

#include <cstddef>
#include "asio/async_result.hpp"
#include "asio/detail/cstdint.hpp"
#include "asio/detail/win_iocp_handle_service.hpp"
#include "asio/error.hpp"
#include "asio/io_context.hpp"

#include "asio/detail/push_options.hpp"

namespace asio_sockio {
namespace windows {

/// Default service implementation for a random-access handle.
class random_access_handle_service
#if defined(GENERATING_DOCUMENTATION)
  : public asio_sockio::io_context::service
#else
  : public asio_sockio::detail::service_base<random_access_handle_service>
#endif
{
public:
#if defined(GENERATING_DOCUMENTATION)
  /// The unique service identifier.
  static asio_sockio::io_context::id id;
#endif

private:
  // The type of the platform-specific implementation.
  typedef detail::win_iocp_handle_service service_impl_type;

public:
  /// The type of a random-access handle implementation.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined implementation_type;
#else
  typedef service_impl_type::implementation_type implementation_type;
#endif

  /// The native handle type.
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined native_handle_type;
#else
  typedef service_impl_type::native_handle_type native_handle_type;
#endif

  /// Construct a new random-access handle service for the specified io_context.
  explicit random_access_handle_service(asio_sockio::io_context& io_context)
    : asio_sockio::detail::service_base<
        random_access_handle_service>(io_context),
      service_impl_(io_context)
  {
  }

  /// Construct a new random-access handle implementation.
  void construct(implementation_type& impl)
  {
    service_impl_.construct(impl);
  }

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move-construct a new random-access handle implementation.
  void move_construct(implementation_type& impl,
      implementation_type& other_impl)
  {
    service_impl_.move_construct(impl, other_impl);
  }

  /// Move-assign from another random-access handle implementation.
  void move_assign(implementation_type& impl,
      random_access_handle_service& other_service,
      implementation_type& other_impl)
  {
    service_impl_.move_assign(impl, other_service.service_impl_, other_impl);
  }
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Destroy a random-access handle implementation.
  void destroy(implementation_type& impl)
  {
    service_impl_.destroy(impl);
  }

  /// Assign an existing native handle to a random-access handle.
  ASIO_SYNC_OP_VOID assign(implementation_type& impl,
      const native_handle_type& handle, asio_sockio::error_code& ec)
  {
    service_impl_.assign(impl, handle, ec);
    ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Determine whether the handle is open.
  bool is_open(const implementation_type& impl) const
  {
    return service_impl_.is_open(impl);
  }

  /// Close a random-access handle implementation.
  ASIO_SYNC_OP_VOID close(implementation_type& impl,
      asio_sockio::error_code& ec)
  {
    service_impl_.close(impl, ec);
    ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Get the native handle implementation.
  native_handle_type native_handle(implementation_type& impl)
  {
    return service_impl_.native_handle(impl);
  }

  /// Cancel all asynchronous operations associated with the handle.
  ASIO_SYNC_OP_VOID cancel(implementation_type& impl,
      asio_sockio::error_code& ec)
  {
    service_impl_.cancel(impl, ec);
    ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Write the given data at the specified offset.
  template <typename ConstBufferSequence>
  std::size_t write_some_at(implementation_type& impl, uint64_t offset,
      const ConstBufferSequence& buffers, asio_sockio::error_code& ec)
  {
    return service_impl_.write_some_at(impl, offset, buffers, ec);
  }

  /// Start an asynchronous write at the specified offset.
  template <typename ConstBufferSequence, typename WriteHandler>
  ASIO_INITFN_RESULT_TYPE(WriteHandler,
      void (asio_sockio::error_code, std::size_t))
  async_write_some_at(implementation_type& impl,
      uint64_t offset, const ConstBufferSequence& buffers,
      ASIO_MOVE_ARG(WriteHandler) handler)
  {
    asio_sockio::async_completion<WriteHandler,
      void (asio_sockio::error_code, std::size_t)> init(handler);

    service_impl_.async_write_some_at(impl,
        offset, buffers, init.completion_handler);

    return init.result.get();
  }

  /// Read some data from the specified offset.
  template <typename MutableBufferSequence>
  std::size_t read_some_at(implementation_type& impl, uint64_t offset,
      const MutableBufferSequence& buffers, asio_sockio::error_code& ec)
  {
    return service_impl_.read_some_at(impl, offset, buffers, ec);
  }

  /// Start an asynchronous read at the specified offset.
  template <typename MutableBufferSequence, typename ReadHandler>
  ASIO_INITFN_RESULT_TYPE(ReadHandler,
      void (asio_sockio::error_code, std::size_t))
  async_read_some_at(implementation_type& impl,
      uint64_t offset, const MutableBufferSequence& buffers,
      ASIO_MOVE_ARG(ReadHandler) handler)
  {
    asio_sockio::async_completion<ReadHandler,
      void (asio_sockio::error_code, std::size_t)> init(handler);

    service_impl_.async_read_some_at(impl,
        offset, buffers, init.completion_handler);

    return init.result.get();
  }

private:
  // Destroy all user-defined handler objects owned by the service.
  void shutdown()
  {
    service_impl_.shutdown();
  }

  // The platform-specific implementation.
  service_impl_type service_impl_;
};

} // namespace windows
} // namespace asio_sockio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // defined(ASIO_ENABLE_OLD_SERVICES)

#endif // ASIO_WINDOWS_RANDOM_ACCESS_HANDLE_SERVICE_HPP
