//
// timer.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <asio.hpp>
#include <boost/bind.hpp>

class printer
{
public:
  printer(asio_sockio::io_context& io)
    : strand_(io),
      timer1_(io, asio_sockio::chrono::seconds(1)),
      timer2_(io, asio_sockio::chrono::seconds(1)),
      count_(0)
  {
    timer1_.async_wait(asio_sockio::bind_executor(strand_,
          boost::bind(&printer::print1, this)));

    timer2_.async_wait(asio_sockio::bind_executor(strand_,
          boost::bind(&printer::print2, this)));
  }

  ~printer()
  {
    std::cout << "Final count is " << count_ << std::endl;
  }

  void print1()
  {
    if (count_ < 10)
    {
      std::cout << "Timer 1: " << count_ << std::endl;
      ++count_;

      timer1_.expires_at(timer1_.expiry() + asio_sockio::chrono::seconds(1));

      timer1_.async_wait(asio_sockio::bind_executor(strand_,
            boost::bind(&printer::print1, this)));
    }
  }

  void print2()
  {
    if (count_ < 10)
    {
      std::cout << "Timer 2: " << count_ << std::endl;
      ++count_;

      timer2_.expires_at(timer2_.expiry() + asio_sockio::chrono::seconds(1));

      timer2_.async_wait(asio_sockio::bind_executor(strand_,
            boost::bind(&printer::print2, this)));
    }
  }

private:
  asio_sockio::io_context::strand strand_;
  asio_sockio::steady_timer timer1_;
  asio_sockio::steady_timer timer2_;
  int count_;
};

int main()
{
  asio_sockio::io_context io;
  printer p(io);
  asio_sockio::thread t(boost::bind(&asio_sockio::io_context::run, &io));
  io.run();
  t.join();

  return 0;
}
