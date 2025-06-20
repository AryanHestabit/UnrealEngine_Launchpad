//
// refactored_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio/experimental/co_spawn.hpp>
#include <asio/experimental/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>
#include <asio/write.hpp>
#include <cstdio>

using asio_sockio::ip::tcp;
using asio_sockio::experimental::co_spawn;
using asio_sockio::experimental::detached;
namespace this_coro = asio_sockio::experimental::this_coro;

template <typename T>
  using awaitable = asio_sockio::experimental::awaitable<
    T, asio_sockio::io_context::executor_type>;

awaitable<void> echo_once(tcp::socket& socket)
{
  auto token = co_await this_coro::token();

  char data[128];
  std::size_t n = co_await socket.async_read_some(asio_sockio::buffer(data), token);
  co_await async_write(socket, asio_sockio::buffer(data, n), token);
}

awaitable<void> echo(tcp::socket socket)
{
  try
  {
    for (;;)
    {
      // The asynchronous operations to echo a single chunk of data have been
      // refactored into a separate function. When this function is called, the
      // operations are still performed in the context of the current
      // coroutine, and the behaviour is functionally equivalent.
      co_await echo_once(socket);
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

awaitable<void> listener()
{
  auto executor = co_await this_coro::executor();
  auto token = co_await this_coro::token();

  tcp::acceptor acceptor(executor.context(), {tcp::v4(), 55555});
  for (;;)
  {
    tcp::socket socket = co_await acceptor.async_accept(token);
    co_spawn(executor,
        [socket = std::move(socket)]() mutable
        {
          return echo(std::move(socket));
        },
        detached);
  }
}

int main()
{
  try
  {
    asio_sockio::io_context io_context(1);

    asio_sockio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });

    co_spawn(io_context, listener, detached);

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::printf("Exception: %s\n", e.what());
  }
}
