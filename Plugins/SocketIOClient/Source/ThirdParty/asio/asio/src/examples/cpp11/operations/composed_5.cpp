//
// composed_5.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_future.hpp>
#include <asio/write.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

using asio_sockio::ip::tcp;

//------------------------------------------------------------------------------

// This composed operation shows composition of multiple underlying operations.
// It automatically serialises a message, using its I/O streams insertion
// operator, before sending it N times on the socket. To do this, it must
// allocate a buffer for the encoded message and ensure this buffer's validity
// until all underlying async_write operation complete. A one second delay is
// inserted prior to each write operation, using a steady_timer.
template <typename T, typename CompletionToken>
auto async_write_messages(tcp::socket& socket,
    const T& message, std::size_t repeat_count,
    CompletionToken&& token)
  // The return type of the initiating function is deduced from the combination
  // of CompletionToken type and the completion handler's signature. When the
  // completion token is a simple callback, the return type is always void.
  // In this example, when the completion token is asio_sockio::yield_context
  // (used for stackful coroutines) the return type would be also be void, as
  // there is no non-error argument to the completion handler. When the
  // completion token is asio_sockio::use_future it would be std::future<void>.
  -> typename asio_sockio::async_result<
    typename std::decay<CompletionToken>::type,
    void(std::error_code)>::return_type
{
  // Define a type alias for the concrete completion handler, as we will use
  // the type in several places in the implementation below.
  using completion_handler_type =
    typename asio_sockio::async_completion<CompletionToken,
      void(std::error_code)>::completion_handler_type;

  // In this example, the composed operation's intermediate completion handler
  // is implemented as a hand-crafted function object.
  struct intermediate_completion_handler
  {
    // The intermediate completion handler holds a reference to the socket as
    // it is used for multiple async_write operations, as well as for obtaining
    // the I/O executor (see get_executor below).
    tcp::socket& socket_;

    // The allocated buffer for the encoded message. The std::unique_ptr smart
    // pointer is move-only, and as a consequence our intermediate completion
    // handler is also move-only.
    std::unique_ptr<std::string> encoded_message_;

    // The repeat count remaining.
    std::size_t repeat_count_;

    // A steady timer used for introducing a delay.
    std::unique_ptr<asio_sockio::steady_timer> delay_timer_;

    // To manage the cycle between the multiple underlying asychronous
    // operations, our intermediate completion handler is implemented as a
    // state machine.
    enum { starting, waiting, writing } state_;

    // As our composed operation performs multiple underlying I/O operations,
    // we should maintain a work object against the I/O executor. This tells
    // the I/O executor that there is still more work to come in the future.
    asio_sockio::executor_work_guard<tcp::socket::executor_type> io_work_;

    // The user-supplied completion handler, called once only on completion of
    // the entire composed operation.
    completion_handler_type handler_;

    // By having a default value for the second argument, this function call
    // operator matches the completion signature of both the async_write and
    // steady_timer::async_wait operations.
    void operator()(const std::error_code& error, std::size_t = 0)
    {
      if (!error)
      {
        switch (state_)
        {
        case starting:
        case writing:
          if (repeat_count_ > 0)
          {
            --repeat_count_;
            state_ = waiting;
            delay_timer_->expires_after(std::chrono::seconds(1));
            delay_timer_->async_wait(std::move(*this));
            return; // Composed operation not yet complete.
          }
          break; // Composed operation complete, continue below.
        case waiting:
          state_ = writing;
          asio_sockio::async_write(socket_,
              asio_sockio::buffer(*encoded_message_), std::move(*this));
          return; // Composed operation not yet complete.
        }
      }

      // This point is reached only on completion of the entire composed
      // operation.

      // We no longer have any future work coming for the I/O executor.
      io_work_.reset();

      // Deallocate the encoded message before calling the user-supplied
      // completion handler.
      encoded_message_.reset();

      // Call the user-supplied handler with the result of the operation.
      handler_(error);
    }

    // It is essential to the correctness of our composed operation that we
    // preserve the executor of the user-supplied completion handler. With a
    // hand-crafted function object we can do this by defining a nested type
    // executor_type and member function get_executor. These obtain the
    // completion handler's associated executor, and default to the I/O
    // executor - in this case the executor of the socket - if the completion
    // handler does not have its own.
    using executor_type = asio_sockio::associated_executor_t<
      completion_handler_type, tcp::socket::executor_type>;

    executor_type get_executor() const noexcept
    {
      return asio_sockio::get_associated_executor(
          handler_, socket_.get_executor());
    }

    // Although not necessary for correctness, we may also preserve the
    // allocator of the user-supplied completion handler. This is achieved by
    // defining a nested type allocator_type and member function get_allocator.
    // These obtain the completion handler's associated allocator, and default
    // to std::allocator<void> if the completion handler does not have its own.
    using allocator_type = asio_sockio::associated_allocator_t<
      completion_handler_type, std::allocator<void>>;

    allocator_type get_allocator() const noexcept
    {
      return asio_sockio::get_associated_allocator(
          handler_, std::allocator<void>{});
    }
  };

  // The asio_sockio::async_completion object takes the completion token and
  // from it creates:
  //
  // - completion.completion_handler:
  //     A completion handler (i.e. a callback) with the specified signature.
  //
  // - completion.result:
  //     An object from which we obtain the result of the initiating function.
  asio_sockio::async_completion<CompletionToken,
    void(std::error_code)> completion(token);

  // Encode the message and copy it into an allocated buffer. The buffer will
  // be maintained for the lifetime of the composed asynchronous operation.
  std::ostringstream os;
  os << message;
  std::unique_ptr<std::string> encoded_message(new std::string(os.str()));

  // Create a steady_timer to be used for the delay between messages.
  std::unique_ptr<asio_sockio::steady_timer> delay_timer(
      new asio_sockio::steady_timer(socket.get_executor().context()));

  // Initiate the underlying operations by explicitly calling our intermediate
  // completion handler's function call operator.
  intermediate_completion_handler{
      socket, std::move(encoded_message),
      repeat_count, std::move(delay_timer),
      intermediate_completion_handler::starting,
      asio_sockio::make_work_guard(socket.get_executor()),
      std::move(completion.completion_handler)
    }({});

  // Finally, we return the result of the initiating function.
  return completion.result.get();
}

//------------------------------------------------------------------------------

void test_callback()
{
  asio_sockio::io_context io_context;

  tcp::acceptor acceptor(io_context, {tcp::v4(), 55555});
  tcp::socket socket = acceptor.accept();

  // Test our asynchronous operation using a lambda as a callback.
  async_write_messages(socket, "Testing callback\r\n", 5,
      [](const std::error_code& error)
      {
        if (!error)
        {
          std::cout << "Messages sent\n";
        }
        else
        {
          std::cout << "Error: " << error.message() << "\n";
        }
      });

  io_context.run();
}

//------------------------------------------------------------------------------

void test_future()
{
  asio_sockio::io_context io_context;

  tcp::acceptor acceptor(io_context, {tcp::v4(), 55555});
  tcp::socket socket = acceptor.accept();

  // Test our asynchronous operation using the use_future completion token.
  // This token causes the operation's initiating function to return a future,
  // which may be used to synchronously wait for the result of the operation.
  std::future<void> f = async_write_messages(
      socket, "Testing future\r\n", 5, asio_sockio::use_future);

  io_context.run();

  try
  {
    // Get the result of the operation.
    f.get();
    std::cout << "Messages sent\n";
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << "\n";
  }
}

//------------------------------------------------------------------------------

int main()
{
  test_callback();
  test_future();
}
