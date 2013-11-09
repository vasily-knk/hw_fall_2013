#pragma once

#include "targetver.h"


#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::shared_ptr;

