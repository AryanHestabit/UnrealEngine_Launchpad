//
// reference_counted.cpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <vector>

using asio_sockio::ip::tcp;

// A reference-counted non-modifiable buffer class.
class shared_const_buffer
{
public:
  // Construct from a std::string.
  explicit shared_const_buffer(const std::string& data)
    : data_(new std::vector<char>(data.begin(), data.end())),
      buffer_(asio_sockio::buffer(*data_))
  {
  }

  // Implement the ConstBufferSequence requirements.
  typedef asio_sockio::const_buffer value_type;
  typedef const asio_sockio::const_buffer* const_iterator;
  const asio_sockio::const_buffer* begin() const { return &buffer_; }
  const asio_sockio::const_buffer* end() const { return &buffer_ + 1; }

private:
  boost::shared_ptr<std::vector<char> > data_;
  asio_sockio::const_buffer buffer_;
};

class session
  : public boost::enable_shared_from_this<session>
{
public:
  session(asio_sockio::io_context& io_context)
    : socket_(io_context)
  {
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    using namespace std; // For time_t, time and ctime.
    time_t now = time(0);
    shared_const_buffer buffer(ctime(&now));
    asio_sockio::async_write(socket_, buffer,
        boost::bind(&session::handle_write, shared_from_this()));
  }

  void handle_write()
  {
  }

private:
  // The socket used to communicate with the client.
  tcp::socket socket_;
};

typedef boost::shared_ptr<session> session_ptr;

class server
{
public:
  server(asio_sockio::io_context& io_context, short port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    session_ptr new_session(new session(io_context_));
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          asio_sockio::placeholders::error));
  }

  void handle_accept(session_ptr new_session,
      const asio_sockio::error_code& error)
  {
    if (!error)
    {
      new_session->start();
    }

    new_session.reset(new session(io_context_));
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          asio_sockio::placeholders::error));
  }

private:
  asio_sockio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: reference_counted <port>\n";
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
