//
// system_timer.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Prevent link dependency on the Boost.System library.
#if !defined(BOOST_SYSTEM_NO_DEPRECATED)
#define BOOST_SYSTEM_NO_DEPRECATED
#endif // !defined(BOOST_SYSTEM_NO_DEPRECATED)

// Test that header file is self-contained.
#include "asio/system_timer.hpp"

#include "unit_test.hpp"

#if defined(ASIO_HAS_STD_CHRONO)

#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/thread.hpp"

#if defined(ASIO_HAS_BOOST_BIND)
# include <boost/bind.hpp>
#else // defined(ASIO_HAS_BOOST_BIND)
# include <functional>
#endif // defined(ASIO_HAS_BOOST_BIND)

#if defined(ASIO_HAS_BOOST_BIND)
namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
namespace bindns = std;
#endif // defined(ASIO_HAS_BOOST_BIND)

void increment(int* count)
{
  ++(*count);
}

void decrement_to_zero(asio_sockio::system_timer* t, int* count)
{
  if (*count > 0)
  {
    --(*count);

    int before_value = *count;

    t->expires_at(t->expiry() + asio_sockio::chrono::seconds(1));
    t->async_wait(bindns::bind(decrement_to_zero, t, count));

    // Completion cannot nest, so count value should remain unchanged.
    ASIO_CHECK(*count == before_value);
  }
}

void increment_if_not_cancelled(int* count,
    const asio_sockio::error_code& ec)
{
  if (!ec)
    ++(*count);
}

void cancel_timer(asio_sockio::system_timer* t)
{
  std::size_t num_cancelled = t->cancel();
  ASIO_CHECK(num_cancelled == 1);
}

void cancel_one_timer(asio_sockio::system_timer* t)
{
  std::size_t num_cancelled = t->cancel_one();
  ASIO_CHECK(num_cancelled == 1);
}

asio_sockio::system_timer::time_point now()
{
  return asio_sockio::system_timer::clock_type::now();
}

void system_timer_test()
{
  using asio_sockio::chrono::seconds;
  using asio_sockio::chrono::microseconds;
#if !defined(ASIO_HAS_BOOST_BIND)
  using std::placeholders::_1;
  using std::placeholders::_2;
#endif // !defined(ASIO_HAS_BOOST_BIND)

  asio_sockio::io_context ioc;
  int count = 0;

  asio_sockio::system_timer::time_point start = now();

  asio_sockio::system_timer t1(ioc, seconds(1));
  t1.wait();

  // The timer must block until after its expiry time.
  asio_sockio::system_timer::time_point end = now();
  asio_sockio::system_timer::time_point expected_end = start + seconds(1);
  ASIO_CHECK(expected_end < end || expected_end == end);

  start = now();

  asio_sockio::system_timer t2(ioc, seconds(1) + microseconds(500000));
  t2.wait();

  // The timer must block until after its expiry time.
  end = now();
  expected_end = start + seconds(1) + microseconds(500000);
  ASIO_CHECK(expected_end < end || expected_end == end);

  t2.expires_at(t2.expiry() + seconds(1));
  t2.wait();

  // The timer must block until after its expiry time.
  end = now();
  expected_end += seconds(1);
  ASIO_CHECK(expected_end < end || expected_end == end);

  start = now();

  t2.expires_after(seconds(1) + microseconds(200000));
  t2.wait();

  // The timer must block until after its expiry time.
  end = now();
  expected_end = start + seconds(1) + microseconds(200000);
  ASIO_CHECK(expected_end < end || expected_end == end);

  start = now();

  asio_sockio::system_timer t3(ioc, seconds(5));
  t3.async_wait(bindns::bind(increment, &count));

  // No completions can be delivered until run() is called.
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all operations have finished, and
  // this should not be until after the timer's expiry time.
  ASIO_CHECK(count == 1);
  end = now();
  expected_end = start + seconds(1);
  ASIO_CHECK(expected_end < end || expected_end == end);

  count = 3;
  start = now();

  asio_sockio::system_timer t4(ioc, seconds(1));
  t4.async_wait(bindns::bind(decrement_to_zero, &t4, &count));

  // No completions can be delivered until run() is called.
  ASIO_CHECK(count == 3);

  ioc.restart();
  ioc.run();

  // The run() call will not return until all operations have finished, and
  // this should not be until after the timer's final expiry time.
  ASIO_CHECK(count == 0);
  end = now();
  expected_end = start + seconds(3);
  ASIO_CHECK(expected_end < end || expected_end == end);

  count = 0;
  start = now();

  asio_sockio::system_timer t5(ioc, seconds(10));
  t5.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));
  asio_sockio::system_timer t6(ioc, seconds(1));
  t6.async_wait(bindns::bind(cancel_timer, &t5));

  // No completions can be delivered until run() is called.
  ASIO_CHECK(count == 0);

  ioc.restart();
  ioc.run();

  // The timer should have been cancelled, so count should not have changed.
  // The total run time should not have been much more than 1 second (and
  // certainly far less than 10 seconds).
  ASIO_CHECK(count == 0);
  end = now();
  expected_end = start + seconds(2);
  ASIO_CHECK(end < expected_end);

  // Wait on the timer again without cancelling it. This time the asynchronous
  // wait should run to completion and increment the counter.
  t5.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));

  ioc.restart();
  ioc.run();

  // The timer should not have been cancelled, so count should have changed.
  // The total time since the timer was created should be more than 10 seconds.
  ASIO_CHECK(count == 1);
  end = now();
  expected_end = start + seconds(10);
  ASIO_CHECK(expected_end < end || expected_end == end);

  count = 0;
  start = now();

  // Start two waits on a timer, one of which will be cancelled. The one
  // which is not cancelled should still run to completion and increment the
  // counter.
  asio_sockio::system_timer t7(ioc, seconds(3));
  t7.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));
  t7.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));
  asio_sockio::system_timer t8(ioc, seconds(1));
  t8.async_wait(bindns::bind(cancel_one_timer, &t7));

  ioc.restart();
  ioc.run();

  // One of the waits should not have been cancelled, so count should have
  // changed. The total time since the timer was created should be more than 3
  // seconds.
  ASIO_CHECK(count == 1);
  end = now();
  expected_end = start + seconds(3);
  ASIO_CHECK(expected_end < end || expected_end == end);
}

struct timer_handler
{
  timer_handler() {}
  void operator()(const asio_sockio::error_code&) {}
#if defined(ASIO_HAS_MOVE)
  timer_handler(timer_handler&&) {}
private:
  timer_handler(const timer_handler&);
#endif // defined(ASIO_HAS_MOVE)
};

void system_timer_cancel_test()
{
  static asio_sockio::io_context io_context;
  struct timer
  {
    asio_sockio::system_timer t;
    timer() : t(io_context)
    {
      t.expires_at((asio_sockio::system_timer::time_point::max)());
    }
  } timers[50];

  timers[2].t.async_wait(timer_handler());
  timers[41].t.async_wait(timer_handler());
  for (int i = 10; i < 20; ++i)
    timers[i].t.async_wait(timer_handler());

  ASIO_CHECK(timers[2].t.cancel() == 1);
  ASIO_CHECK(timers[41].t.cancel() == 1);
  for (int i = 10; i < 20; ++i)
    ASIO_CHECK(timers[i].t.cancel() == 1);
}

struct custom_allocation_timer_handler
{
  custom_allocation_timer_handler(int* count) : count_(count) {}
  void operator()(const asio_sockio::error_code&) {}
  int* count_;
};

void* asio_handler_allocate(std::size_t size,
    custom_allocation_timer_handler* handler)
{
  ++(*handler->count_);
  return ::operator new(size);
}

void asio_handler_deallocate(void* pointer, std::size_t,
    custom_allocation_timer_handler* handler)
{
  --(*handler->count_);
  ::operator delete(pointer);
}

void system_timer_custom_allocation_test()
{
  static asio_sockio::io_context io_context;
  struct timer
  {
    asio_sockio::system_timer t;
    timer() : t(io_context) {}
  } timers[100];

  int allocation_count = 0;

  for (int i = 0; i < 50; ++i)
  {
    timers[i].t.expires_at((asio_sockio::system_timer::time_point::max)());
    timers[i].t.async_wait(custom_allocation_timer_handler(&allocation_count));
  }

  for (int i = 50; i < 100; ++i)
  {
    timers[i].t.expires_at((asio_sockio::system_timer::time_point::min)());
    timers[i].t.async_wait(custom_allocation_timer_handler(&allocation_count));
  }

  for (int i = 0; i < 50; ++i)
    timers[i].t.cancel();

  io_context.run();

  ASIO_CHECK(allocation_count == 0);
}

void io_context_run(asio_sockio::io_context* ioc)
{
  ioc->run();
}

void system_timer_thread_test()
{
  asio_sockio::io_context ioc;
  asio_sockio::executor_work_guard<asio_sockio::io_context::executor_type> work
    = asio_sockio::make_work_guard(ioc);
  asio_sockio::system_timer t1(ioc);
  asio_sockio::system_timer t2(ioc);
  int count = 0;

  asio_sockio::thread th(bindns::bind(io_context_run, &ioc));

  t2.expires_after(asio_sockio::chrono::seconds(2));
  t2.wait();

  t1.expires_after(asio_sockio::chrono::seconds(2));
  t1.async_wait(bindns::bind(increment, &count));

  t2.expires_after(asio_sockio::chrono::seconds(4));
  t2.wait();

  ioc.stop();
  th.join();

  ASIO_CHECK(count == 1);
}

#if defined(ASIO_HAS_MOVE)
asio_sockio::system_timer make_timer(asio_sockio::io_context& ioc, int* count)
{
  asio_sockio::system_timer t(ioc);
  t.expires_after(asio_sockio::chrono::seconds(1));
  t.async_wait(bindns::bind(increment, count));
  return t;
}
#endif

void system_timer_move_test()
{
#if defined(ASIO_HAS_MOVE)
  asio_sockio::io_context io_context1;
  asio_sockio::io_context io_context2;
  int count = 0;

  asio_sockio::system_timer t1 = make_timer(io_context1, &count);
  asio_sockio::system_timer t2 = make_timer(io_context2, &count);
  asio_sockio::system_timer t3 = std::move(t1);

  t2 = std::move(t1);

  io_context2.run();

  ASIO_CHECK(count == 1);

  io_context1.run();

  ASIO_CHECK(count == 2);
#endif // defined(ASIO_HAS_MOVE)
}

ASIO_TEST_SUITE
(
  "system_timer",
  ASIO_TEST_CASE(system_timer_test)
  ASIO_TEST_CASE(system_timer_cancel_test)
  ASIO_TEST_CASE(system_timer_custom_allocation_test)
  ASIO_TEST_CASE(system_timer_thread_test)
  ASIO_TEST_CASE(system_timer_move_test)
)
#else // defined(ASIO_HAS_STD_CHRONO)
ASIO_TEST_SUITE
(
  "system_timer",
  ASIO_TEST_CASE(null_test)
)
#endif // defined(ASIO_HAS_STD_CHRONO)
