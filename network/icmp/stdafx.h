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
#include <array>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pt = boost::posix_time;
namespace asio = boost::asio;

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::shared_ptr;
using boost::shared_array;
using boost::make_shared;

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/optional.hpp>
using boost::optional;

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/range/algorithm.hpp>

#include <stdexcept>


