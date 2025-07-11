//
// high_resolution_timer.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_HIGH_RESOLUTION_TIMER_HPP
#define ASIO_HIGH_RESOLUTION_TIMER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_CHRONO) || defined(GENERATING_DOCUMENTATION)

#include "asio/basic_waitable_timer.hpp"
#include "asio/detail/chrono.hpp"

namespace asio_sockio {

/// Typedef for a timer based on the high resolution clock.
/**
 * This typedef uses the C++11 @c &lt;chrono&gt; standard library facility, if
 * available. Otherwise, it may use the Boost.Chrono library. To explicitly
 * utilise Boost.Chrono, use the basic_waitable_timer template directly:
 * @code
 * typedef basic_waitable_timer<boost::chrono::high_resolution_clock> timer;
 * @endcode
 */
typedef basic_waitable_timer<
    chrono::high_resolution_clock>
  high_resolution_timer;

} // namespace asio_sockio

#endif // defined(ASIO_HAS_CHRONO) || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_HIGH_RESOLUTION_TIMER_HPP
