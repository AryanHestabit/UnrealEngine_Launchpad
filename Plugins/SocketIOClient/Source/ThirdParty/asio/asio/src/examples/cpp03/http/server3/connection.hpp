//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_CONNECTION_HPP
#define HTTP_SERVER3_CONNECTION_HPP

#include <asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace http {
namespace server3 {

/// Represents a single connection from a client.
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
public:
  /// Construct a connection with the given io_context.
  explicit connection(asio_sockio::io_context& io_context,
      request_handler& handler);

  /// Get the socket associated with the connection.
  asio_sockio::ip::tcp::socket& socket();

  /// Start the first asynchronous operation for the connection.
  void start();

private:
  /// Handle completion of a read operation.
  void handle_read(const asio_sockio::error_code& e,
      std::size_t bytes_transferred);

  /// Handle completion of a write operation.
  void handle_write(const asio_sockio::error_code& e);

  /// Strand to ensure the connection's handlers are not called concurrently.
  asio_sockio::io_context::strand strand_;

  /// Socket for the connection.
  asio_sockio::ip::tcp::socket socket_;

  /// The handler used to process the incoming request.
  request_handler& request_handler_;

  /// Buffer for incoming data.
  boost::array<char, 8192> buffer_;

  /// The incoming request.
  request request_;

  /// The parser for the incoming request.
  request_parser request_parser_;

  /// The reply to be sent back to the client.
  reply reply_;
};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP
