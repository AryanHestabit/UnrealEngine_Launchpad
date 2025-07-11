//
// async_tcp_client.cpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "asio/buffer.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read_until.hpp"
#include "asio/steady_timer.hpp"
#include "asio/write.hpp"
#include <boost/bind.hpp>
#include <iostream>
#include <string>

using asio_sockio::steady_timer;
using asio_sockio::ip::tcp;

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by an "actor" that persists for the lifetime of the
// client object:
//
//  +----------------+
//  |                |
//  | check_deadline |<---+
//  |                |    |
//  +----------------+    | async_wait()
//              |         |
//              +---------+
//
// If the deadline actor determines that the deadline has expired, the socket
// is closed and any outstanding operations are consequently cancelled.
//
// Connection establishment involves trying each endpoint in turn until a
// connection is successful, or the available endpoints are exhausted. If the
// deadline actor closes the socket, the connect actor is woken up and moves to
// the next endpoint.
//
//  +---------------+
//  |               |
//  | start_connect |<---+
//  |               |    |
//  +---------------+    |
//           |           |
//  async_-  |    +----------------+
// connect() |    |                |
//           +--->| handle_connect |
//                |                |
//                +----------------+
//                          :
// Once a connection is     :
// made, the connect        :
// actor forks in two -     :
//                          :
// an actor for reading     :       and an actor for
// inbound messages:        :       sending heartbeats:
//                          :
//  +------------+          :          +-------------+
//  |            |<- - - - -+- - - - ->|             |
//  | start_read |                     | start_write |<---+
//  |            |<---+                |             |    |
//  +------------+    |                +-------------+    | async_wait()
//          |         |                        |          |
//  async_- |    +-------------+       async_- |    +--------------+
//   read_- |    |             |       write() |    |              |
//  until() +--->| handle_read |               +--->| handle_write |
//               |             |                    |              |
//               +-------------+                    +--------------+
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character. The deadline for a complete message is 30 seconds.
//
// The heartbeat actor sends a heartbeat (a message that consists of a single
// newline character) every 10 seconds. In this example, no deadline is applied
// to message sending.
//
class client
{
public:
  client(asio_sockio::io_context& io_context)
    : stopped_(false),
      socket_(io_context),
      deadline_(io_context),
      heartbeat_timer_(io_context)
  {
  }

  // Called by the user of the client class to initiate the connection process.
  // The endpoints will have been obtained using a tcp::resolver.
  void start(tcp::resolver::results_type endpoints)
  {
    // Start the connect actor.
    endpoints_ = endpoints;
    start_connect(endpoints_.begin());

    // Start the deadline actor. You will note that we're not setting any
    // particular deadline here. Instead, the connect and input actors will
    // update the deadline prior to each asynchronous operation.
    deadline_.async_wait(boost::bind(&client::check_deadline, this));
  }

  // This function terminates all the actors to shut down the connection. It
  // may be called by the user of the client class, or by the class itself in
  // response to graceful termination or an unrecoverable error.
  void stop()
  {
    stopped_ = true;
    asio_sockio::error_code ignored_ec;
    socket_.close(ignored_ec);
    deadline_.cancel();
    heartbeat_timer_.cancel();
  }

private:
  void start_connect(tcp::resolver::results_type::iterator endpoint_iter)
  {
    if (endpoint_iter != endpoints_.end())
    {
      std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

      // Set a deadline for the connect operation.
      deadline_.expires_after(asio_sockio::chrono::seconds(60));

      // Start the asynchronous connect operation.
      socket_.async_connect(endpoint_iter->endpoint(),
          boost::bind(&client::handle_connect,
            this, _1, endpoint_iter));
    }
    else
    {
      // There are no more endpoints to try. Shut down the client.
      stop();
    }
  }

  void handle_connect(const asio_sockio::error_code& ec,
      tcp::resolver::results_type::iterator endpoint_iter)
  {
    if (stopped_)
      return;

    // The async_connect() function automatically opens the socket at the start
    // of the asynchronous operation. If the socket is closed at this time then
    // the timeout handler must have run first.
    if (!socket_.is_open())
    {
      std::cout << "Connect timed out\n";

      // Try the next available endpoint.
      start_connect(++endpoint_iter);
    }

    // Check if the connect operation failed before the deadline expired.
    else if (ec)
    {
      std::cout << "Connect error: " << ec.message() << "\n";

      // We need to close the socket used in the previous connection attempt
      // before starting a new one.
      socket_.close();

      // Try the next available endpoint.
      start_connect(++endpoint_iter);
    }

    // Otherwise we have successfully established a connection.
    else
    {
      std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";

      // Start the input actor.
      start_read();

      // Start the heartbeat actor.
      start_write();
    }
  }

  void start_read()
  {
    // Set a deadline for the read operation.
    deadline_.expires_after(asio_sockio::chrono::seconds(30));

    // Start an asynchronous operation to read a newline-delimited message.
    asio_sockio::async_read_until(socket_,
        asio_sockio::dynamic_buffer(input_buffer_), '\n',
        boost::bind(&client::handle_read, this, _1, _2));
  }

  void handle_read(const asio_sockio::error_code& ec, std::size_t n)
  {
    if (stopped_)
      return;

    if (!ec)
    {
      // Extract the newline-delimited message from the buffer.
      std::string line(input_buffer_.substr(0, n - 1));
      input_buffer_.erase(0, n);

      // Empty messages are heartbeats and so ignored.
      if (!line.empty())
      {
        std::cout << "Received: " << line << "\n";
      }

      start_read();
    }
    else
    {
      std::cout << "Error on receive: " << ec.message() << "\n";

      stop();
    }
  }

  void start_write()
  {
    if (stopped_)
      return;

    // Start an asynchronous operation to send a heartbeat message.
    asio_sockio::async_write(socket_, asio_sockio::buffer("\n", 1),
        boost::bind(&client::handle_write, this, _1));
  }

  void handle_write(const asio_sockio::error_code& ec)
  {
    if (stopped_)
      return;

    if (!ec)
    {
      // Wait 10 seconds before sending the next heartbeat.
      heartbeat_timer_.expires_after(asio_sockio::chrono::seconds(10));
      heartbeat_timer_.async_wait(boost::bind(&client::start_write, this));
    }
    else
    {
      std::cout << "Error on heartbeat: " << ec.message() << "\n";

      stop();
    }
  }

  void check_deadline()
  {
    if (stopped_)
      return;

    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline_.expiry() <= steady_timer::clock_type::now())
    {
      // The deadline has passed. The socket is closed so that any outstanding
      // asynchronous operations are cancelled.
      socket_.close();

      // There is no longer an active deadline. The expiry is set to the
      // maximum time point so that the actor takes no action until a new
      // deadline is set.
      deadline_.expires_at(steady_timer::time_point::max());
    }

    // Put the actor back to sleep.
    deadline_.async_wait(boost::bind(&client::check_deadline, this));
  }

private:
  bool stopped_;
  tcp::resolver::results_type endpoints_;
  tcp::socket socket_;
  std::string input_buffer_;
  steady_timer deadline_;
  steady_timer heartbeat_timer_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: client <host> <port>\n";
      return 1;
    }

    asio_sockio::io_context io_context;
    tcp::resolver r(io_context);
    client c(io_context);

    c.start(r.resolve(argv[1], argv[2]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
