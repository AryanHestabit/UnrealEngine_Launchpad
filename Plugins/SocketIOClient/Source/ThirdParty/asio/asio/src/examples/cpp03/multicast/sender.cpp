//
// sender.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <sstream>
#include <string>
#include "asio.hpp"
#include "boost/bind.hpp"

const short multicast_port = 30001;
const int max_message_count = 10;

class sender
{
public:
  sender(asio_sockio::io_context& io_context,
      const asio_sockio::ip::address& multicast_address)
    : endpoint_(multicast_address, multicast_port),
      socket_(io_context, endpoint_.protocol()),
      timer_(io_context),
      message_count_(0)
  {
    std::ostringstream os;
    os << "Message " << message_count_++;
    message_ = os.str();

    socket_.async_send_to(
        asio_sockio::buffer(message_), endpoint_,
        boost::bind(&sender::handle_send_to, this,
          asio_sockio::placeholders::error));
  }

  void handle_send_to(const asio_sockio::error_code& error)
  {
    if (!error && message_count_ < max_message_count)
    {
      timer_.expires_after(asio_sockio::chrono::seconds(1));
      timer_.async_wait(
          boost::bind(&sender::handle_timeout, this,
            asio_sockio::placeholders::error));
    }
  }

  void handle_timeout(const asio_sockio::error_code& error)
  {
    if (!error)
    {
      std::ostringstream os;
      os << "Message " << message_count_++;
      message_ = os.str();

      socket_.async_send_to(
          asio_sockio::buffer(message_), endpoint_,
          boost::bind(&sender::handle_send_to, this,
            asio_sockio::placeholders::error));
    }
  }

private:
  asio_sockio::ip::udp::endpoint endpoint_;
  asio_sockio::ip::udp::socket socket_;
  asio_sockio::steady_timer timer_;
  int message_count_;
  std::string message_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: sender <multicast_address>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    sender 239.255.0.1\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    sender ff31::8000:1234\n";
      return 1;
    }

    asio_sockio::io_context io_context;
    sender s(io_context, asio_sockio::ip::make_address(argv[1]));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
