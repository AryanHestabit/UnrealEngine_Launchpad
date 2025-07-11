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
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <ctime>

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
  std::shared_ptr<std::vector<char> > data_;
  asio_sockio::const_buffer buffer_;
};

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    do_write();
  }

private:
  void do_write()
  {
    std::time_t now = std::time(0);
    shared_const_buffer buffer(std::ctime(&now));

    auto self(shared_from_this());
    asio_sockio::async_write(socket_, buffer,
        [this, self](std::error_code /*ec*/, std::size_t /*length*/)
        {
        });
  }

  // The socket used to communicate with the client.
  tcp::socket socket_;
};

class server
{
public:
  server(asio_sockio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](std::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket))->start();
          }

          do_accept();
        });
  }

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

    server s(io_context, std::atoi(argv[1]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
