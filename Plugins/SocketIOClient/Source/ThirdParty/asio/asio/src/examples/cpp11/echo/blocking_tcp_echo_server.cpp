//
// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include "asio.hpp"

using asio_sockio::ip::tcp;

const int max_length = 1024;

void session(tcp::socket sock)
{
  try
  {
    for (;;)
    {
      char data[max_length];

      asio_sockio::error_code error;
      size_t length = sock.read_some(asio_sockio::buffer(data), error);
      if (error == asio_sockio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw asio_sockio::system_error(error); // Some other error.

      asio_sockio::write(sock, asio_sockio::buffer(data, length));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void server(asio_sockio::io_context& io_context, unsigned short port)
{
  tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
  for (;;)
  {
    std::thread(session, a.accept()).detach();
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
      return 1;
    }

    asio_sockio::io_context io_context;

    server(io_context, std::atoi(argv[1]));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
