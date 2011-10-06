//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//  Adaptation to Boost of the libcxx
//  Copyright 2010 Vicente J. Botet Escriba
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <iostream>
#include <boost/type_traits/is_same.hpp>
#include <boost/chrono/stopwatches/simple_stopwatch.hpp>
#include <libs/chrono/test/cycle_count.hpp>
#include <boost/detail/lightweight_test.hpp>

#if !defined(BOOST_NO_STATIC_ASSERT)
#define NOTHING ""
#endif


template <typename Stopwatch>
void check_invariants()
{
    BOOST_CHRONO_STATIC_ASSERT((boost::is_same<typename Stopwatch::rep, typename Stopwatch::clock::duration::rep>::value), NOTHING, ());
    BOOST_CHRONO_STATIC_ASSERT((boost::is_same<typename Stopwatch::period, typename Stopwatch::clock::duration::period>::value), NOTHING, ());
    BOOST_CHRONO_STATIC_ASSERT((boost::is_same<typename Stopwatch::duration, typename Stopwatch::clock::time_point::duration>::value), NOTHING, ());
    BOOST_CHRONO_STATIC_ASSERT(Stopwatch::is_steady == Stopwatch::clock::is_steady, NOTHING, ());
}

template <typename Stopwatch>
void check_default_constructor()
{
  Stopwatch _;
}

template <typename Stopwatch>
void check_constructor_ec()
{
  boost::system::error_code ec;
  Stopwatch _(ec);
  BOOST_TEST(ec.value()==0);
}

template <typename Stopwatch>
void check_constructor_throws()
{
  Stopwatch _(boost::throws());
}


template <typename Stopwatch>
void check_elapsed(bool check=true)
{
  Stopwatch sw;
  ex::sleep_for<typename Stopwatch::clock>(boost::chrono::milliseconds(100));
  typename Stopwatch::duration d=sw.elapsed();
  if (check)
    BOOST_TEST(d >= boost::chrono::milliseconds(100));
}

template <typename Clock>
void check_all(bool check=true)
{
  typedef boost::chrono::simple_stopwatch<Clock> Stopwatch;
  check_invariants<Stopwatch>();
  check_default_constructor<Stopwatch>();
  check_constructor_ec<Stopwatch>();
  check_constructor_throws<Stopwatch>();
  check_elapsed<Stopwatch>(check);
}


int main()
{
  std::cout << "cycle_count=";
  check_all<ex::cycle_count<1500> >(true);

  return boost::report_errors();
}