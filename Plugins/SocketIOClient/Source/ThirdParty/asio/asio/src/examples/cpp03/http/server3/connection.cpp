//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"

namespace http {
namespace server3 {

connection::connection(asio_sockio::io_context& io_context,
    request_handler& handler)
  : strand_(io_context),
    socket_(io_context),
    request_handler_(handler)
{
}

asio_sockio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

void connection::start()
{
  socket_.async_read_some(asio_sockio::buffer(buffer_),
      asio_sockio::bind_executor(strand_,
        boost::bind(&connection::handle_read, shared_from_this(),
          asio_sockio::placeholders::error,
          asio_sockio::placeholders::bytes_transferred)));
}

void connection::handle_read(const asio_sockio::error_code& e,
    std::size_t bytes_transferred)
{
  if (!e)
  {
    boost::tribool result;
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred);

    if (result)
    {
      request_handler_.handle_request(request_, reply_);
      asio_sockio::async_write(socket_, reply_.to_buffers(),
          asio_sockio::bind_executor(strand_,
            boost::bind(&connection::handle_write, shared_from_this(),
              asio_sockio::placeholders::error)));
    }
    else if (!result)
    {
      reply_ = reply::stock_reply(reply::bad_request);
      asio_sockio::async_write(socket_, reply_.to_buffers(),
          asio_sockio::bind_executor(strand_,
            boost::bind(&connection::handle_write, shared_from_this(),
              asio_sockio::placeholders::error)));
    }
    else
    {
      socket_.async_read_some(asio_sockio::buffer(buffer_),
          asio_sockio::bind_executor(strand_,
            boost::bind(&connection::handle_read, shared_from_this(),
              asio_sockio::placeholders::error,
              asio_sockio::placeholders::bytes_transferred)));
    }
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

void connection::handle_write(const asio_sockio::error_code& e)
{
  if (!e)
  {
    // Initiate graceful connection closure.
    asio_sockio::error_code ignored_ec;
    socket_.shutdown(asio_sockio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}

} // namespace server3
} // namespace http
