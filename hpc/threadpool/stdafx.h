#pragma once

#ifdef _MSC_VER
#include <SDKDDKVer.h>
#define USE_BOOST_ATOMIC
#endif

#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <vector>
using std::vector;
#include <queue>
using std::queue;
#include <unordered_map>
using std::unordered_map;
#include <unordered_set>
using std::unordered_set;

#include <algorithm>
using std::pair;
using std::make_pair;

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>

using boost::shared_ptr;
using boost::make_shared;

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/time_duration.hpp>
namespace pt = boost::posix_time;

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/optional.hpp>
using boost::optional;

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


#if defined(USE_BOOST_ATOMIC)
#include <boost/atomic.hpp>
using boost::atomic_bool;
#else
#include <atomic>
using std::atomic_bool;
#endif

#include <stdexcept>

#define OUT_ARG(x) x

#define MY_ASSERT assert
