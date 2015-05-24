/*
    Copyright 2015, Proteus Tech Co Ltd. http://www.kirit.com/Rask
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/main>

#include <rask/pool.hpp>


FSL_MAIN("rask", "Rask")(fostlib::ostream &out, fostlib::arguments &args) {
    rask::pool sockets(4);
    return 0;
}
