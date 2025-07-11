//
// tcp_server.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using asio_sockio::ip::tcp;

#include <asio/yield.hpp>

class tcp_server : asio_sockio::coroutine
{
public:
  tcp_server(tcp::acceptor& acceptor, std::size_t buf_size) :
    acceptor_(acceptor),
    socket_(acceptor_.get_executor().context()),
    buffer_(buf_size)
  {
  }

  void operator()(asio_sockio::error_code ec, std::size_t n = 0)
  {
    reenter (this) for (;;)
    {
      yield acceptor_.async_accept(socket_, ref(this));

      while (!ec)
      {
        yield asio_sockio::async_read(socket_,
            asio_sockio::buffer(buffer_), ref(this));

        if (!ec)
        {
          for (std::size_t i = 0; i < n; ++i) buffer_[i] = ~buffer_[i];

          yield asio_sockio::async_write(socket_,
              asio_sockio::buffer(buffer_), ref(this));
        }
      }

      socket_.close();
    }
  }

  struct ref
  {
    explicit ref(tcp_server* p)
      : p_(p)
    {
    }

    void operator()(asio_sockio::error_code ec, std::size_t n = 0)
    {
      (*p_)(ec, n);
    }

  private:
    tcp_server* p_;
  };

private:
  tcp::acceptor& acceptor_;
  tcp::socket socket_;
  std::vector<unsigned char> buffer_;
  tcp::endpoint sender_;
};

#include <asio/unyield.hpp>

int main(int argc, char* argv[])
{
  if (argc != 5)
  {
    std::fprintf(stderr,
        "Usage: tcp_server <port> <nconns> "
        "<bufsize> {spin|block}\n");
    return 1;
  }

  unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
  int max_connections = std::atoi(argv[2]);
  std::size_t buf_size = std::atoi(argv[3]);
  bool spin = (std::strcmp(argv[4], "spin") == 0);

  asio_sockio::io_context io_context(1);
  tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
  std::vector<boost::shared_ptr<tcp_server> > servers;

  for (int i = 0; i < max_connections; ++i)
  {
    boost::shared_ptr<tcp_server> s(new tcp_server(acceptor, buf_size));
    servers.push_back(s);
    (*s)(asio_sockio::error_code());
  }

  if (spin)
    for (;;) io_context.poll();
  else
    io_context.run();
}
