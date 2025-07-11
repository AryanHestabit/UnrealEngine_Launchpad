//
// third_party_lib.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

using asio_sockio::ip::tcp;

namespace third_party_lib {

// Simulation of a third party library that wants to perform read and write
// operations directly on a socket. It needs to be polled to determine whether
// it requires a read or write operation, and notified when the socket is ready
// for reading or writing.
class session
{
public:
  session(tcp::socket& socket)
    : socket_(socket),
      state_(reading)
  {
  }

  // Returns true if the third party library wants to be notified when the
  // socket is ready for reading.
  bool want_read() const
  {
    return state_ == reading;
  }

  // Notify that third party library that it should perform its read operation.
  void do_read(asio_sockio::error_code& ec)
  {
    if (std::size_t len = socket_.read_some(asio_sockio::buffer(data_), ec))
    {
      write_buffer_ = asio_sockio::buffer(data_, len);
      state_ = writing;
    }
  }

  // Returns true if the third party library wants to be notified when the
  // socket is ready for writing.
  bool want_write() const
  {
    return state_ == writing;
  }

  // Notify that third party library that it should perform its write operation.
  void do_write(asio_sockio::error_code& ec)
  {
    if (std::size_t len = socket_.write_some(
          asio_sockio::buffer(write_buffer_), ec))
    {
      write_buffer_ = write_buffer_ + len;
      state_ = asio_sockio::buffer_size(write_buffer_) > 0 ? writing : reading;
    }
  }

private:
  tcp::socket& socket_;
  enum { reading, writing } state_;
  boost::array<char, 128> data_;
  asio_sockio::const_buffer write_buffer_;
};

} // namespace third_party_lib

// The glue between asio's sockets and the third party library.
class connection
  : public boost::enable_shared_from_this<connection>
{
public:
  typedef boost::shared_ptr<connection> pointer;

  static pointer create(asio_sockio::io_context& io_context)
  {
    return pointer(new connection(io_context));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    // Put the socket into non-blocking mode.
    socket_.non_blocking(true);

    start_operations();
  }

private:
  connection(asio_sockio::io_context& io_context)
    : socket_(io_context),
      session_impl_(socket_),
      read_in_progress_(false),
      write_in_progress_(false)
  {
  }

  void start_operations()
  {
    // Start a read operation if the third party library wants one.
    if (session_impl_.want_read() && !read_in_progress_)
    {
      read_in_progress_ = true;
      socket_.async_wait(tcp::socket::wait_read,
          boost::bind(&connection::handle_read,
            shared_from_this(),
            asio_sockio::placeholders::error));
    }

    // Start a write operation if the third party library wants one.
    if (session_impl_.want_write() && !write_in_progress_)
    {
      write_in_progress_ = true;
      socket_.async_wait(tcp::socket::wait_write,
          boost::bind(&connection::handle_write,
            shared_from_this(),
            asio_sockio::placeholders::error));
    }
  }

  void handle_read(asio_sockio::error_code ec)
  {
    read_in_progress_ = false;

    // Notify third party library that it can perform a read.
    if (!ec)
      session_impl_.do_read(ec);

    // The third party library successfully performed a read on the socket.
    // Start new read or write operations based on what it now wants.
    if (!ec || ec == asio_sockio::error::would_block)
      start_operations();

    // Otherwise, an error occurred. Closing the socket cancels any outstanding
    // asynchronous read or write operations. The connection object will be
    // destroyed automatically once those outstanding operations complete.
    else
      socket_.close();
  }

  void handle_write(asio_sockio::error_code ec)
  {
    write_in_progress_ = false;

    // Notify third party library that it can perform a write.
    if (!ec)
      session_impl_.do_write(ec);

    // The third party library successfully performed a write on the socket.
    // Start new read or write operations based on what it now wants.
    if (!ec || ec == asio_sockio::error::would_block)
      start_operations();

    // Otherwise, an error occurred. Closing the socket cancels any outstanding
    // asynchronous read or write operations. The connection object will be
    // destroyed automatically once those outstanding operations complete.
    else
      socket_.close();
  }

private:
  tcp::socket socket_;
  third_party_lib::session session_impl_;
  bool read_in_progress_;
  bool write_in_progress_;
};

class server
{
public:
  server(asio_sockio::io_context& io_context, unsigned short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    connection::pointer new_connection =
      connection::create(acceptor_.get_executor().context());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&server::handle_accept, this, new_connection,
          asio_sockio::placeholders::error));
  }

  void handle_accept(connection::pointer new_connection,
      const asio_sockio::error_code& error)
  {
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: third_party_lib <port>\n";
      return 1;
    }

    asio_sockio::io_context io_context;

    using namespace std; // For atoi.
    server s(io_context, atoi(argv[1]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
