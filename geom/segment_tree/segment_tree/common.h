#pragma once

#ifdef _MSC_VER
#include <SDKDDKVer.h>
#endif

#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <map>
using std::map;
#include <set>
using std::set;
#include <vector>
using std::vector;

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/make_shared.hpp>

using boost::shared_ptr;
using boost::make_shared;

namespace pt = boost::posix_time;

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/optional.hpp>
using boost::optional;

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/range/algorithm.hpp>

#include <stdexcept>

#ifdef WIN32
#include <Windows.h>
#endif

