//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include "asio.hpp"
#include "asio/ssl.hpp"

typedef asio_sockio::ssl::stream<asio_sockio::ip::tcp::socket> ssl_socket;

class session
{
public:
  session(asio_sockio::io_context& io_context,
      asio_sockio::ssl::context& context)
    : socket_(io_context, context)
  {
  }

  ssl_socket::lowest_layer_type& socket()
  {
    return socket_.lowest_layer();
  }

  void start()
  {
    socket_.async_handshake(asio_sockio::ssl::stream_base::server,
        boost::bind(&session::handle_handshake, this,
          asio_sockio::placeholders::error));
  }

  void handle_handshake(const asio_sockio::error_code& error)
  {
    if (!error)
    {
      socket_.async_read_some(asio_sockio::buffer(data_, max_length),
          boost::bind(&session::handle_read, this,
            asio_sockio::placeholders::error,
            asio_sockio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }

  void handle_read(const asio_sockio::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      asio_sockio::async_write(socket_,
          asio_sockio::buffer(data_, bytes_transferred),
          boost::bind(&session::handle_write, this,
            asio_sockio::placeholders::error));
    }
    else
    {
      delete this;
    }
  }

  void handle_write(const asio_sockio::error_code& error)
  {
    if (!error)
    {
      socket_.async_read_some(asio_sockio::buffer(data_, max_length),
          boost::bind(&session::handle_read, this,
            asio_sockio::placeholders::error,
            asio_sockio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }

private:
  ssl_socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server
{
public:
  server(asio_sockio::io_context& io_context, unsigned short port)
    : io_context_(io_context),
      acceptor_(io_context,
          asio_sockio::ip::tcp::endpoint(asio_sockio::ip::tcp::v4(), port)),
      context_(asio_sockio::ssl::context::sslv23)
  {
    context_.set_options(
        asio_sockio::ssl::context::default_workarounds
        | asio_sockio::ssl::context::no_sslv2
        | asio_sockio::ssl::context::single_dh_use);
    context_.set_password_callback(boost::bind(&server::get_password, this));
    context_.use_certificate_chain_file("server.pem");
    context_.use_private_key_file("server.pem", asio_sockio::ssl::context::pem);
    context_.use_tmp_dh_file("dh2048.pem");

    start_accept();
  }

  std::string get_password() const
  {
    return "test";
  }

  void start_accept()
  {
    session* new_session = new session(io_context_, context_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          asio_sockio::placeholders::error));
  }

  void handle_accept(session* new_session,
      const asio_sockio::error_code& error)
  {
    if (!error)
    {
      new_session->start();
    }
    else
    {
      delete new_session;
    }

    start_accept();
  }

private:
  asio_sockio::io_context& io_context_;
  asio_sockio::ip::tcp::acceptor acceptor_;
  asio_sockio::ssl::context context_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: server <port>\n";
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
