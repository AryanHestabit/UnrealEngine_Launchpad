//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "asio.hpp"
#include <algorithm>
#include <boost/bind.hpp>
#include <iostream>
#include <list>
#include "handler_allocator.hpp"

class session
{
public:
  session(asio_sockio::io_context& ioc, size_t block_size)
    : io_context_(ioc),
      strand_(ioc),
      socket_(ioc),
      block_size_(block_size),
      read_data_(new char[block_size]),
      read_data_length_(0),
      write_data_(new char[block_size]),
      unsent_count_(0),
      op_count_(0)
  {
  }

  ~session()
  {
    delete[] read_data_;
    delete[] write_data_;
  }

  asio_sockio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    asio_sockio::error_code set_option_err;
    asio_sockio::ip::tcp::no_delay no_delay(true);
    socket_.set_option(no_delay, set_option_err);
    if (!set_option_err)
    {
      ++op_count_;
      socket_.async_read_some(asio_sockio::buffer(read_data_, block_size_),
          asio_sockio::bind_executor(strand_,
            make_custom_alloc_handler(read_allocator_,
              boost::bind(&session::handle_read, this,
                asio_sockio::placeholders::error,
                asio_sockio::placeholders::bytes_transferred))));
    }
    else
    {
      asio_sockio::post(io_context_, boost::bind(&session::destroy, this));
    }
  }

  void handle_read(const asio_sockio::error_code& err, size_t length)
  {
    --op_count_;

    if (!err)
    {
      read_data_length_ = length;
      ++unsent_count_;
      if (unsent_count_ == 1)
      {
        op_count_ += 2;
        std::swap(read_data_, write_data_);
        async_write(socket_, asio_sockio::buffer(write_data_, read_data_length_),
            asio_sockio::bind_executor(strand_,
              make_custom_alloc_handler(write_allocator_,
                boost::bind(&session::handle_write, this,
                  asio_sockio::placeholders::error))));
        socket_.async_read_some(asio_sockio::buffer(read_data_, block_size_),
            asio_sockio::bind_executor(strand_,
              make_custom_alloc_handler(read_allocator_,
                boost::bind(&session::handle_read, this,
                  asio_sockio::placeholders::error,
                  asio_sockio::placeholders::bytes_transferred))));
      }
    }

    if (op_count_ == 0)
      asio_sockio::post(io_context_, boost::bind(&session::destroy, this));
  }

  void handle_write(const asio_sockio::error_code& err)
  {
    --op_count_;

    if (!err)
    {
      --unsent_count_;
      if (unsent_count_ == 1)
      {
        op_count_ += 2;
        std::swap(read_data_, write_data_);
        async_write(socket_, asio_sockio::buffer(write_data_, read_data_length_),
            asio_sockio::bind_executor(strand_,
              make_custom_alloc_handler(write_allocator_,
                boost::bind(&session::handle_write, this,
                  asio_sockio::placeholders::error))));
        socket_.async_read_some(asio_sockio::buffer(read_data_, block_size_),
            asio_sockio::bind_executor(strand_,
              make_custom_alloc_handler(read_allocator_,
                boost::bind(&session::handle_read, this,
                  asio_sockio::placeholders::error,
                  asio_sockio::placeholders::bytes_transferred))));
      }
    }

    if (op_count_ == 0)
      asio_sockio::post(io_context_, boost::bind(&session::destroy, this));
  }

  static void destroy(session* s)
  {
    delete s;
  }

private:
  asio_sockio::io_context& io_context_;
  asio_sockio::io_context::strand strand_;
  asio_sockio::ip::tcp::socket socket_;
  size_t block_size_;
  char* read_data_;
  size_t read_data_length_;
  char* write_data_;
  int unsent_count_;
  int op_count_;
  handler_allocator read_allocator_;
  handler_allocator write_allocator_;
};

class server
{
public:
  server(asio_sockio::io_context& ioc, const asio_sockio::ip::tcp::endpoint& endpoint,
      size_t block_size)
    : io_context_(ioc),
      acceptor_(ioc),
      block_size_(block_size)
  {
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio_sockio::ip::tcp::acceptor::reuse_address(1));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    start_accept();
  }

  void start_accept()
  {
    session* new_session = new session(io_context_, block_size_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          asio_sockio::placeholders::error));
  }

  void handle_accept(session* new_session, const asio_sockio::error_code& err)
  {
    if (!err)
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
  size_t block_size_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 5)
    {
      std::cerr << "Usage: server <address> <port> <threads> <blocksize>\n";
      return 1;
    }

    using namespace std; // For atoi.
    asio_sockio::ip::address address = asio_sockio::ip::make_address(argv[1]);
    short port = atoi(argv[2]);
    int thread_count = atoi(argv[3]);
    size_t block_size = atoi(argv[4]);

    asio_sockio::io_context ioc;

    server s(ioc, asio_sockio::ip::tcp::endpoint(address, port), block_size);

    // Threads not currently supported in this test.
    std::list<asio_sockio::thread*> threads;
    while (--thread_count > 0)
    {
      asio_sockio::thread* new_thread = new asio_sockio::thread(
          boost::bind(&asio_sockio::io_context::run, &ioc));
      threads.push_back(new_thread);
    }

    ioc.run();

    while (!threads.empty())
    {
      threads.front()->join();
      delete threads.front();
      threads.pop_front();
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
